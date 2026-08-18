using System.Diagnostics;
using System.Net.Sockets;
using System.Net.WebSockets;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

namespace FtdDesktop;

public sealed record EngineInfo(
    int LatticeSize,
    bool Gpu,
    string Backend,
    string Version,
    string GpuDescription)
{
    public bool InteractiveGpuMode { get; init; }
    public int MaxLatticeSize { get; init; }
    public bool OwnsProcess { get; init; }
    public int RequestedLatticeSize { get; init; }
    public string? StartupWarning { get; init; }
}

public sealed class EngineHost : IAsyncDisposable
{
    private readonly DesktopOptions _options;
    private readonly DesktopPaths _paths;
    private readonly Action<string> _log;
    private readonly object _stateGate = new();
    private Process? _process;
    private int? _linuxPid;
    private bool _ownsProcess;
    private long _nextRequestId;

    public event Action<int>? UnexpectedExit;

    public EngineHost(DesktopOptions options, DesktopPaths paths, Action<string> log)
    {
        _options = options;
        _paths = paths;
        _log = log;
    }

    public async Task<EngineInfo> StartAsync(
        IProgress<string>? progress,
        CancellationToken cancellationToken)
    {
        DisposeExitedProcess();
        progress?.Report("Checking WSL2 and the NVIDIA runtime...");
        string gpuDescription = await PreflightAsync(cancellationToken);

        if (_process is not null && !_process.HasExited)
        {
            EngineInfo active = await ProbeRequiredAsync("active engine", cancellationToken);
            ValidateCudaEngine(active, "active engine");
            active = await ReconcileRequestedLatticeAsync(active, progress, cancellationToken);
            return active with
            {
                GpuDescription = gpuDescription,
                OwnsProcess = true,
            };
        }

        EngineInfo? existing = await TryProbeAsync(cancellationToken);
        if (existing is not null)
        {
            ValidateCudaEngine(existing, $"engine on port {_options.EnginePort}");
            await ValidateExistingWslServerAsync(cancellationToken);
            lock (_stateGate)
                _ownsProcess = false;
            _log($"Using an existing CUDA engine on port {_options.EnginePort}.");
            existing = await ReconcileRequestedLatticeAsync(existing, progress, cancellationToken);
            return existing with
            {
                GpuDescription = gpuDescription,
                OwnsProcess = false,
            };
        }

        if (await IsTcpPortOccupiedAsync(cancellationToken))
        {
            throw new InvalidOperationException(
                $"Port {_options.EnginePort} is already occupied by a service that is not " +
                "an idle compatible FTD CUDA server. Stop that service, close its current " +
                "dashboard client, or choose another --engine-port.");
        }

        if (!_options.SkipEngineBuild)
        {
            progress?.Report("Building the WSL2 CUDA server (incremental)...");
            await BuildServerAsync(cancellationToken);
        }

        progress?.Report("Starting the WSL2 CUDA engine...");
        int bootstrapSize = Math.Min(_options.LatticeSize, 64);
        if (bootstrapSize != _options.LatticeSize)
        {
            _log(
                $"Bootstrapping CUDA at L={bootstrapSize}; requested L={_options.LatticeSize} " +
                "will be memory-preflighted and resized transactionally.");
        }

        StartServerProcess(bootstrapSize);
        try
        {
            await WaitForOwnershipPidAsync(cancellationToken);
            EngineInfo info = await WaitUntilReadyAsync(cancellationToken);
            ValidateCudaEngine(info, "newly started engine");
            info = await ReconcileRequestedLatticeAsync(info, progress, cancellationToken);
            return info with
            {
                GpuDescription = gpuDescription,
                OwnsProcess = true,
            };
        }
        catch
        {
            await StopAsync();
            throw;
        }
    }

    private void DisposeExitedProcess()
    {
        Process? stale = null;
        lock (_stateGate)
        {
            if (_process is not null && _process.HasExited)
            {
                stale = _process;
                _process = null;
                _linuxPid = null;
                _ownsProcess = false;
            }
        }
        stale?.Dispose();
    }

    private async Task<string> PreflightAsync(CancellationToken cancellationToken)
    {
        string command =
            "set -e; grep -qi 'microsoft-standard-WSL2' /proc/sys/kernel/osrelease; " +
            $"test -d {BashQuote(_paths.WslRepositoryRoot)}; " +
            "command -v nvidia-smi >/dev/null; " +
            "nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv,noheader | head -n 1";
        ProcessResult result;
        try
        {
            result = await RunWslAsync(
                command,
                cancellationToken,
                logOutput: false);
        }
        catch (Exception ex) when (
            ex is not OperationCanceledException ||
            !cancellationToken.IsCancellationRequested)
        {
            throw new InvalidOperationException(
                $"Could not launch WSL2 distribution '{_options.WslDistribution}'. " +
                "Verify WSL2 is installed and the distribution is registered.",
                ex);
        }
        if (result.ExitCode != 0)
        {
            string detail = FirstNonEmpty(
                result.StandardError,
                result.StandardOutput);
            throw new InvalidOperationException(
                $"WSL2 CUDA preflight failed for '{_options.WslDistribution}'. {detail}".Trim());
        }

        string description = result.StandardOutput
            .Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries)
            .LastOrDefault()?.Trim() ?? "NVIDIA GPU";
        _log($"GPU: {description}");
        return description;
    }

    private async Task ValidateExistingWslServerAsync(
        CancellationToken cancellationToken)
    {
        ProcessResult listener = await RunWslAsync(
            $"ss -H -ltnp 'sport = :{_options.EnginePort}'",
            cancellationToken,
            logOutput: false);
        Match pidMatch = Regex.Match(
            listener.StandardOutput,
            @"\bpid=(\d+)\b",
            RegexOptions.CultureInvariant);
        if (listener.ExitCode != 0 || !pidMatch.Success ||
            !int.TryParse(pidMatch.Groups[1].Value, out int pid))
        {
            string detail = FirstNonEmpty(
                listener.StandardError,
                listener.StandardOutput);
            throw new InvalidOperationException(
                $"Port {_options.EnginePort} reports an FTD CUDA service, but it " +
                "is not the configured WSL2 engine/build_wsl/ws_server process. " +
                "Stop that service or choose another --engine-port. " +
                detail);
        }

        ProcessResult executable = await RunWslAsync(
            $"readlink /proc/{pid}/exe",
            cancellationToken,
            logOutput: false);
        string actualPath = executable.StandardOutput.Trim();
        string expectedPath = _paths.WslServerPath;
        if (executable.ExitCode != 0 ||
            !(string.Equals(actualPath, expectedPath, StringComparison.Ordinal) ||
              string.Equals(
                  actualPath,
                  expectedPath + " (deleted)",
                  StringComparison.Ordinal)))
        {
            string detail = FirstNonEmpty(
                executable.StandardError,
                actualPath.Length > 0
                    ? $"Unexpected WSL2 executable: {actualPath}"
                    : listener.StandardOutput);
            throw new InvalidOperationException(
                $"Port {_options.EnginePort} reports an FTD CUDA service, but it " +
                "is not the configured WSL2 engine/build_wsl/ws_server process. " +
                "Stop that service or choose another --engine-port. " +
                detail);
        }

        ProcessResult gpuGuard = await RunWslAsync(
            $"grep -zqx 'FTD_FORCE_GPU=1' /proc/{pid}/environ",
            cancellationToken,
            logOutput: false);
        if (gpuGuard.ExitCode != 0)
        {
            throw new InvalidOperationException(
                $"The existing WSL2 engine on port {_options.EnginePort} was not " +
                "launched with FTD_FORCE_GPU=1. Restart it through FTD Desktop " +
                "or choose another --engine-port; unguarded reuse could silently " +
                "switch the backend to CPU.");
        }

        _log($"Existing WSL2 engine PID: {pid}");
    }

    private async Task BuildServerAsync(CancellationToken cancellationToken)
    {
        string command =
            $"cd {BashQuote(_paths.WslRepositoryRoot)} && " +
            "test -f engine/build_wsl/CMakeCache.txt && " +
            "grep -Eq '^FTD_ENABLE_CUDA:[^=]*=ON$' engine/build_wsl/CMakeCache.txt && " +
            "cmake --build engine/build_wsl --target ws_server --parallel 32";
        ProcessResult result = await RunWslAsync(command, cancellationToken);
        if (result.ExitCode != 0)
        {
            string detail = FirstNonEmpty(result.StandardError, result.StandardOutput);
            throw new InvalidOperationException($"WSL2 CUDA server build failed. {detail}".Trim());
        }
    }

    private void StartServerProcess(int latticeSize)
    {
        string command =
            $"cd {BashQuote(_paths.WslRepositoryRoot)} && " +
            $"test -x {BashQuote(_paths.WslServerPath)} && " +
            "echo FTD_DESKTOP_PID=$BASHPID && " +
            $"exec env FTD_FORCE_GPU=1 {BashQuote(_paths.WslServerPath)} " +
            $"{latticeSize} {_options.EnginePort} --bind 127.0.0.1";

        var startInfo = new ProcessStartInfo
        {
            FileName = "wsl.exe",
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        startInfo.ArgumentList.Add("-d");
        startInfo.ArgumentList.Add(_options.WslDistribution);
        startInfo.ArgumentList.Add("--");
        startInfo.ArgumentList.Add("bash");
        startInfo.ArgumentList.Add("-lc");
        startInfo.ArgumentList.Add(command);

        var process = new Process { StartInfo = startInfo, EnableRaisingEvents = true };
        process.OutputDataReceived += (_, e) => HandleServerOutput(process, e.Data);
        process.ErrorDataReceived += (_, e) => HandleServerOutput(process, e.Data);
        process.Exited += (_, _) =>
        {
            bool unexpected;
            lock (_stateGate)
            {
                unexpected = _ownsProcess && ReferenceEquals(_process, process);
                if (unexpected)
                {
                    _ownsProcess = false;
                    _linuxPid = null;
                }
            }
            if (unexpected)
            {
                int exitCode;
                try { exitCode = process.ExitCode; }
                catch { exitCode = -1; }
                _log($"GPU server exited with code {exitCode}.");
                UnexpectedExit?.Invoke(exitCode);
            }
        };

        lock (_stateGate)
        {
            _process = process;
            _linuxPid = null;
            _ownsProcess = true;
        }

        try
        {
            if (!process.Start())
                throw new InvalidOperationException("Failed to start the WSL2 CUDA server.");
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
        }
        catch
        {
            lock (_stateGate)
            {
                if (ReferenceEquals(_process, process))
                {
                    _process = null;
                    _linuxPid = null;
                    _ownsProcess = false;
                }
            }
            process.Dispose();
            throw;
        }
    }

    private void HandleServerOutput(Process source, string? line)
    {
        if (string.IsNullOrWhiteSpace(line))
            return;

        const string pidPrefix = "FTD_DESKTOP_PID=";
        if (line.StartsWith(pidPrefix, StringComparison.Ordinal) &&
            int.TryParse(line[pidPrefix.Length..], out int pid))
        {
            lock (_stateGate)
            {
                if (ReferenceEquals(_process, source) && _ownsProcess)
                    _linuxPid = pid;
            }
            _log($"WSL2 engine PID: {pid}");
            return;
        }

        _log(line);
    }

    private async Task WaitForOwnershipPidAsync(CancellationToken cancellationToken)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(TimeSpan.FromSeconds(10));
        while (!timeout.IsCancellationRequested)
        {
            lock (_stateGate)
            {
                if (_linuxPid is not null)
                    return;
                if (_process is null || _process.HasExited)
                {
                    throw new InvalidOperationException(
                        "The WSL2 CUDA server exited before its owned Linux PID was captured.");
                }
            }

            try
            {
                await Task.Delay(50, timeout.Token);
            }
            catch (OperationCanceledException) when (!cancellationToken.IsCancellationRequested)
            {
                break;
            }
        }

        cancellationToken.ThrowIfCancellationRequested();
        throw new TimeoutException(
            "The WSL2 CUDA server started without reporting its owned Linux PID. " +
            "Startup was aborted to avoid leaving an orphaned engine process.");
    }

    private async Task<EngineInfo> WaitUntilReadyAsync(CancellationToken cancellationToken)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(TimeSpan.FromSeconds(90));

        Exception? lastError = null;
        while (!timeout.IsCancellationRequested)
        {
            if (_process is { HasExited: true })
                throw new InvalidOperationException(
                    $"The WSL2 GPU server exited during startup (code {_process.ExitCode}).");

            try
            {
                EngineInfo? info = await TryProbeAsync(timeout.Token);
                if (info is not null)
                    return info;
            }
            catch (Exception ex) when (ex is not OperationCanceledException)
            {
                lastError = ex;
            }

            try
            {
                await Task.Delay(250, timeout.Token);
            }
            catch (OperationCanceledException) when (!cancellationToken.IsCancellationRequested)
            {
                break;
            }
        }

        cancellationToken.ThrowIfCancellationRequested();

        throw new TimeoutException(
            $"The WSL2 GPU server did not become ready on port {_options.EnginePort}.",
            lastError);
    }

    private async Task<EngineInfo> ProbeRequiredAsync(string context, CancellationToken cancellationToken)
    {
        EngineInfo? info = await TryProbeAsync(cancellationToken);
        return info ?? throw new InvalidOperationException($"Could not probe the {context}.");
    }

    private async Task<EngineInfo?> TryProbeAsync(CancellationToken cancellationToken)
    {
        try
        {
            JsonElement root = await RequestJsonAsync(
                new Dictionary<string, object?> { ["cmd"] = "info" },
                TimeSpan.FromSeconds(2),
                cancellationToken);
            return ParseEngineInfo(root);
        }
        catch (Exception ex) when (
            ex is WebSocketException or HttpRequestException or SocketException ||
            ex is OperationCanceledException && !cancellationToken.IsCancellationRequested)
        {
            return null;
        }
    }

    private async Task<EngineInfo> ReconcileRequestedLatticeAsync(
        EngineInfo current,
        IProgress<string>? progress,
        CancellationToken cancellationToken)
    {
        int requested = _options.LatticeSize;
        if (current.LatticeSize == requested)
        {
            return current with
            {
                RequestedLatticeSize = requested,
                StartupWarning = null,
            };
        }

        progress?.Report($"Checking memory for L={requested}...");
        JsonElement preflight = await RequestJsonAsync(
            new Dictionary<string, object?>
            {
                ["cmd"] = "preflight_resize",
                ["size"] = requested,
            },
            TimeSpan.FromSeconds(5),
            cancellationToken);
        ThrowIfEngineError(preflight, "native resize preflight");

        if (!preflight.TryGetProperty("accepted", out JsonElement acceptedElement) ||
            acceptedElement.ValueKind is not (JsonValueKind.True or JsonValueKind.False))
        {
            throw new InvalidDataException(
                "The native engine does not expose the required transactional resize preflight. " +
                "Rebuild engine/build_wsl/ws_server or remove --skip-engine-build.");
        }

        if (!acceptedElement.GetBoolean())
        {
            string warning = FormatResizeRejection(preflight, current.LatticeSize, requested);
            _log(warning);
            progress?.Report($"CUDA active at safe L={current.LatticeSize}.");
            return current with
            {
                RequestedLatticeSize = requested,
                StartupWarning = warning,
            };
        }

        progress?.Report($"Allocating L={requested} transactionally...");
        JsonElement resize;
        try
        {
            resize = await RequestJsonAsync(
                new Dictionary<string, object?>
                {
                    ["cmd"] = "resize",
                    ["size"] = requested,
                },
                TimeSpan.FromMinutes(3),
                cancellationToken);
        }
        catch (Exception ex) when (
            ex is not OperationCanceledException ||
            !cancellationToken.IsCancellationRequested)
        {
            EngineInfo? fallback = await TryRecoverAfterResizeFailureAsync(cancellationToken);
            if (fallback is null)
            {
                throw new InvalidOperationException(
                    $"Requested L={requested} failed and the prior CUDA lattice " +
                    "could not be re-verified.",
                    ex);
            }

            string warning =
                $"Requested L={requested} did not commit ({ex.Message}). " +
                $"The prior CUDA lattice was re-verified at L={fallback.LatticeSize}.";
            _log(warning);
            return fallback with
            {
                RequestedLatticeSize = requested,
                StartupWarning = warning,
            };
        }

        if (resize.TryGetProperty("error", out JsonElement errorElement))
        {
            EngineInfo fallback = await ProbeRequiredAsync(
                "preserved engine after resize failure",
                cancellationToken);
            ValidateCudaEngine(fallback, "preserved engine after resize failure");
            string detail = errorElement.GetString() ?? "unknown native resize failure";
            string warning =
                $"Requested L={requested} could not be allocated ({detail}). " +
                $"The prior CUDA lattice remains active at L={fallback.LatticeSize}.";
            _log(warning);
            return fallback with
            {
                RequestedLatticeSize = requested,
                StartupWarning = warning,
            };
        }

        EngineInfo resized = await ProbeRequiredAsync("resized engine", cancellationToken);
        ValidateCudaEngine(resized, "resized engine");
        if (resized.LatticeSize != requested)
        {
            throw new InvalidOperationException(
                $"The engine acknowledged resize to L={requested} but reports L={resized.LatticeSize}.");
        }

        _log($"Transactional CUDA resize committed at L={requested}.");
        return resized with
        {
            RequestedLatticeSize = requested,
            StartupWarning = null,
        };
    }

    private async Task<EngineInfo?> TryRecoverAfterResizeFailureAsync(
        CancellationToken cancellationToken)
    {
        try
        {
            using var recovery = CancellationTokenSource.CreateLinkedTokenSource(
                cancellationToken);
            recovery.CancelAfter(TimeSpan.FromSeconds(10));
            while (!recovery.IsCancellationRequested)
            {
                EngineInfo? fallback = await TryProbeAsync(recovery.Token);
                if (fallback is not null)
                {
                    ValidateCudaEngine(
                        fallback,
                        "preserved engine after interrupted resize");
                    return fallback;
                }
                await Task.Delay(250, recovery.Token);
            }
            return null;
        }
        catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
        {
            throw;
        }
        catch (OperationCanceledException) when (!cancellationToken.IsCancellationRequested)
        {
            return null;
        }
        catch (Exception ex)
        {
            _log($"Could not verify the preserved engine after resize failure: {ex.Message}");
            return null;
        }
    }

    private async Task<JsonElement> RequestJsonAsync(
        IReadOnlyDictionary<string, object?> request,
        TimeSpan requestTimeout,
        CancellationToken cancellationToken)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(requestTimeout);
        using var socket = new ClientWebSocket();
        await socket.ConnectAsync(
            new Uri($"ws://127.0.0.1:{_options.EnginePort}"),
            timeout.Token);

        long requestId = Interlocked.Increment(ref _nextRequestId);
        var payloadObject = new Dictionary<string, object?>(request)
        {
            ["_requestId"] = requestId,
        };
        byte[] requestBytes = Encoding.UTF8.GetBytes(JsonSerializer.Serialize(payloadObject));
        await socket.SendAsync(
            requestBytes,
            WebSocketMessageType.Text,
            endOfMessage: true,
            timeout.Token);

        var buffer = new byte[16 * 1024];
        try
        {
            while (true)
            {
                using var payload = new MemoryStream();
                WebSocketReceiveResult receive;
                do
                {
                    receive = await socket.ReceiveAsync(buffer, timeout.Token);
                    if (receive.MessageType == WebSocketMessageType.Close)
                        throw new WebSocketException("The native engine closed the readiness connection.");
                    if (receive.MessageType != WebSocketMessageType.Text)
                        throw new InvalidDataException("The native engine returned a non-JSON control response.");
                    payload.Write(buffer, 0, receive.Count);
                    if (payload.Length > 1024 * 1024)
                        throw new InvalidDataException("The native engine control response exceeded 1 MiB.");
                }
                while (!receive.EndOfMessage);

                using JsonDocument document = JsonDocument.Parse(payload.ToArray());
                JsonElement root = document.RootElement;
                if (root.TryGetProperty("type", out JsonElement typeElement) &&
                    typeElement.GetString() == "operation_progress")
                {
                    string operation = root.TryGetProperty("operation", out JsonElement operationElement)
                        ? operationElement.GetString() ?? "operation"
                        : "operation";
                    string phase = root.TryGetProperty("phase", out JsonElement phaseElement)
                        ? phaseElement.GetString() ?? "working"
                        : "working";
                    int size = root.TryGetProperty("size", out JsonElement sizeElement)
                        ? sizeElement.GetInt32()
                        : 0;
                    _log($"{operation}: {phase} (L={size})");
                    continue;
                }

                if (root.TryGetProperty("_requestId", out JsonElement idElement) &&
                    idElement.TryGetInt64(out long responseId) && responseId == requestId)
                {
                    return root.Clone();
                }
            }
        }
        finally
        {
            if (socket.State == WebSocketState.Open)
            {
                try
                {
                    using var closeTimeout = new CancellationTokenSource(TimeSpan.FromSeconds(1));
                    await socket.CloseAsync(
                        WebSocketCloseStatus.NormalClosure,
                        "desktop control request complete",
                        closeTimeout.Token);
                }
                catch
                {
                    socket.Abort();
                }
            }
        }
    }

    private static EngineInfo ParseEngineInfo(JsonElement root)
    {
        ThrowIfEngineError(root, "engine info probe");
        if (!root.TryGetProperty("latticeSize", out JsonElement latticeElement) ||
            !latticeElement.TryGetInt32(out int latticeSize) || latticeSize < 4 ||
            !root.TryGetProperty("gpu", out JsonElement gpuElement) ||
            gpuElement.ValueKind is not (JsonValueKind.True or JsonValueKind.False) ||
            !root.TryGetProperty("backend", out JsonElement backendElement) ||
            !root.TryGetProperty("version", out JsonElement versionElement) ||
            !root.TryGetProperty("interactiveGpuMode", out JsonElement interactiveElement))
        {
            throw new InvalidDataException(
                "The service on the engine port is not a compatible FTD native server.");
        }

        bool gpu = gpuElement.GetBoolean();
        string backend = backendElement.GetString() ?? string.Empty;
        string version = versionElement.GetString() ?? string.Empty;
        bool interactive = interactiveElement.ValueKind == JsonValueKind.True;
        int maxLatticeSize = root.TryGetProperty("maxLatticeSize", out JsonElement maxElement) &&
            maxElement.TryGetInt32(out int parsedMax)
            ? parsedMax
            : 0;
        if (string.IsNullOrWhiteSpace(backend) || string.IsNullOrWhiteSpace(version) ||
            maxLatticeSize < latticeSize)
        {
            throw new InvalidDataException(
                "The service on the engine port returned an incomplete FTD protocol descriptor.");
        }

        return new EngineInfo(latticeSize, gpu, backend, version, "NVIDIA GPU")
        {
            InteractiveGpuMode = interactive,
            MaxLatticeSize = maxLatticeSize,
            RequestedLatticeSize = latticeSize,
        };
    }

    private static void ValidateCudaEngine(EngineInfo info, string context)
    {
        if (!info.Gpu || !string.Equals(info.Backend, "cuda", StringComparison.OrdinalIgnoreCase))
        {
            throw new InvalidOperationException(
                $"The {context} reports backend '{info.Backend}', not CUDA. " +
                "FTD Desktop will not silently downgrade to CPU.");
        }
        if (!info.InteractiveGpuMode)
        {
            throw new InvalidOperationException(
                $"The {context} is not running in guarded interactive GPU mode. " +
                "Rebuild the WSL2 ws_server before using the desktop application.");
        }
    }

    private static void ThrowIfEngineError(JsonElement root, string context)
    {
        if (root.TryGetProperty("error", out JsonElement errorElement))
        {
            throw new InvalidOperationException(
                $"The {context} failed: {errorElement.GetString() ?? "unknown native error"}");
        }
    }

    private static string FormatResizeRejection(
        JsonElement preflight,
        int fallbackSize,
        int requestedSize)
    {
        static double GiB(JsonElement root, string name) =>
            root.TryGetProperty(name, out JsonElement value) && value.TryGetUInt64(out ulong bytes)
                ? bytes / 1073741824.0
                : 0.0;

        return
            $"Requested L={requestedSize} was rejected by native memory preflight " +
            $"(host {GiB(preflight, "estimatedHostBytes"):F2}/{GiB(preflight, "availableHostBytes"):F2} GiB, " +
            $"GPU {GiB(preflight, "estimatedGpuBytes"):F2}/{GiB(preflight, "availableGpuBytes"):F2} GiB). " +
            $"CUDA remains active at safe L={fallbackSize}.";
    }

    private async Task<bool> IsTcpPortOccupiedAsync(CancellationToken cancellationToken)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(TimeSpan.FromMilliseconds(750));
        using var client = new TcpClient(AddressFamily.InterNetwork);
        try
        {
            await client.ConnectAsync("127.0.0.1", _options.EnginePort, timeout.Token);
            return true;
        }
        catch (OperationCanceledException) when (!cancellationToken.IsCancellationRequested)
        {
            return false;
        }
        catch (SocketException)
        {
            return false;
        }
    }

    public async Task StopAsync()
    {
        Process? process;
        int? linuxPid;
        bool ownsProcess;
        lock (_stateGate)
        {
            process = _process;
            linuxPid = _linuxPid;
            ownsProcess = _ownsProcess;
            _ownsProcess = false;
            _linuxPid = null;
        }

        if (ownsProcess && linuxPid is int pid)
        {
            try
            {
                using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(3));
                await RunWslAsync($"kill -TERM {pid} 2>/dev/null || true", timeout.Token);
            }
            catch
            {
                // The Windows-side process-tree termination below is the fallback.
            }
        }

        if (process is not null)
        {
            try
            {
                if (ownsProcess && !process.HasExited)
                {
                    try
                    {
                        using var exitTimeout =
                            new CancellationTokenSource(TimeSpan.FromMilliseconds(1500));
                        await process.WaitForExitAsync(exitTimeout.Token);
                    }
                    catch (OperationCanceledException)
                    {
                        process.Kill(entireProcessTree: true);
                        try
                        {
                            using var killTimeout =
                                new CancellationTokenSource(TimeSpan.FromSeconds(1));
                            await process.WaitForExitAsync(killTimeout.Token);
                        }
                        catch
                        {
                            // Disposal below releases the Windows-side handle.
                        }
                    }
                }
            }
            catch
            {
                // Process may have exited between the checks.
            }
            finally
            {
                process.Dispose();
                lock (_stateGate)
                {
                    if (ReferenceEquals(_process, process))
                        _process = null;
                }
            }
        }
    }

    private Task<ProcessResult> RunWslAsync(
        string command,
        CancellationToken cancellationToken,
        bool logOutput = true) =>
        ProcessRunner.RunAsync(
            "wsl.exe",
            new[] { "-d", _options.WslDistribution, "--", "bash", "-lc", command },
            logOutput ? _log : null,
            cancellationToken);

    private static string BashQuote(string value) =>
        "'" + value.Replace("'", "'\"'\"'") + "'";

    private static string FirstNonEmpty(params string[] values) =>
        values.Select(value => value.Trim())
            .FirstOrDefault(value => value.Length > 0) ?? string.Empty;

    public async ValueTask DisposeAsync() => await StopAsync();
}
