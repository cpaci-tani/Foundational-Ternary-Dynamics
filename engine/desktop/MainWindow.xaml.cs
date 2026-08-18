using System.Diagnostics;
using System.Text;
using System.Text.Json;
using System.Windows;
using System.Windows.Media;
using System.Windows.Threading;
using Microsoft.Web.WebView2.Core;

namespace FtdDesktop;

internal sealed record DashboardRuntimeInfo(
    bool BridgeReady,
    bool NativeGpu,
    int LatticeSize,
    bool WebGlAvailable,
    bool ContextLost,
    bool HardwareIdentityAvailable,
    string WebGlRenderer,
    string WebGlVersion);

public partial class MainWindow : Window
{
    private static readonly JsonSerializerOptions DashboardJsonOptions = new()
    {
        PropertyNameCaseInsensitive = true,
    };

    private const string DashboardProbeScript = """
        (() => {
            const ctx = window.__ftdCtx || null;
            const bridge = ctx?.bridge || null;
            const renderer = ctx?.viewport?.renderer || null;
            let gl = null;
            let gpuRenderer = '';
            let webGlVersion = '';
            let hardwareIdentityAvailable = false;
            try {
                gl = renderer?.getContext?.() || null;
                if (gl) {
                    const debugInfo = gl.getExtension('WEBGL_debug_renderer_info');
                    hardwareIdentityAvailable = !!debugInfo;
                    gpuRenderer = debugInfo
                        ? String(gl.getParameter(debugInfo.UNMASKED_RENDERER_WEBGL) || '')
                        : String(gl.getParameter(gl.RENDERER) || '');
                    webGlVersion = String(gl.getParameter(gl.VERSION) || '');
                }
            } catch {
                gl = null;
            }
            return {
                bridgeReady: bridge?.ready === true,
                nativeGpu: bridge?.isNativeGPU === true,
                latticeSize: Number(bridge?.latticeSize) || 0,
                webGlAvailable: !!gl,
                contextLost: !!gl?.isContextLost?.(),
                hardwareIdentityAvailable,
                webGlRenderer: gpuRenderer || 'renderer details unavailable',
                webGlVersion: webGlVersion || 'version unavailable'
            };
        })()
        """;

    private readonly DesktopOptions _options;
    private readonly DesktopPaths _paths;
    private readonly DashboardServer _dashboardServer = new();
    private readonly CancellationTokenSource _lifetime = new();
    private readonly StringBuilder _logBuffer = new();
    private readonly StringBuilder _pendingUiLog = new();
    private readonly object _logFileLock = new();
    private readonly object _uiLogLock = new();
    private readonly string _sessionLogPath;
    private readonly DispatcherTimer _logFlushTimer;
    private readonly DispatcherTimer _dashboardWatchdog;
    private StreamWriter? _logWriter;
    private EngineHost? _engineHost;
    private bool _dashboardStarted;
    private bool _isBusy;
    private bool _isClosing;
    private bool _watchdogBusy;
    private bool _smokeExitRequested;
    private int _watchdogFailures;
    private int _droppedUiLogLines;
    private int _activeLatticeSize;
    private string _engineVersion = "unknown";
    private EngineInfo? _lastEngineInfo;

    public MainWindow(DesktopOptions options, DesktopPaths paths)
    {
        _options = options;
        _paths = paths;
        _sessionLogPath = Path.Combine(
            _paths.LogDirectory,
            $"ftd-desktop-{DateTime.Now:yyyyMMdd-HHmmss}.log");
        InitializeComponent();
        try
        {
            _logWriter = new StreamWriter(
                new FileStream(
                    _sessionLogPath,
                    FileMode.Append,
                    FileAccess.Write,
                    FileShare.ReadWrite,
                    16 * 1024),
                new UTF8Encoding(encoderShouldEmitUTF8Identifier: false))
            {
                AutoFlush = true,
            };
        }
        catch
        {
            // The bounded in-window buffer remains available.
        }
        _logFlushTimer = new DispatcherTimer(DispatcherPriority.Background)
        {
            Interval = TimeSpan.FromMilliseconds(200),
        };
        _logFlushTimer.Tick += (_, _) => FlushPendingLog();
        _logFlushTimer.Start();
        _dashboardWatchdog = new DispatcherTimer(DispatcherPriority.Background)
        {
            Interval = TimeSpan.FromSeconds(3),
        };
        _dashboardWatchdog.Tick += DashboardWatchdog_Tick;
        Loaded += MainWindow_Loaded;
        Closing += MainWindow_Closing;
        AppendLog($"Repository: {_paths.RepositoryRoot}");
        AppendLog($"WSL2 distro: {_options.WslDistribution}");
        AppendLog($"Persistent log: {_sessionLogPath}");
    }

    private async void MainWindow_Loaded(object sender, RoutedEventArgs e)
    {
        Loaded -= MainWindow_Loaded;
        await InitializeAsync();
    }

    private async Task InitializeAsync()
    {
        if (_isBusy || _isClosing)
            return;

        _isBusy = true;
        RetryButton.Visibility = Visibility.Collapsed;
        StartupProgress.Visibility = Visibility.Visible;
        StartupOverlay.Visibility = Visibility.Visible;
        SetBackendStatus("Starting", "#F59E0B");
        SetControlsEnabled(false);

        try
        {
            if (!_dashboardStarted)
            {
                StartupStatusText.Text = "Starting the native dashboard host...";
                await _dashboardServer.StartAsync(
                    _paths.WebRoot,
                    _options.DashboardPort,
                    _lifetime.Token);
                _dashboardStarted = true;
                AppendLog($"Dashboard: http://127.0.0.1:{_options.DashboardPort}");
            }

            _engineHost ??= CreateEngineHost();
            var progress = new Progress<string>(message =>
            {
                StartupStatusText.Text = message;
                FooterStatusText.Text = message;
            });
            EngineInfo info = await _engineHost.StartAsync(progress, _lifetime.Token);

            StartupStatusText.Text = "Verifying the native dashboard and hardware renderer...";
            DashboardRuntimeInfo runtime = await InitializeWebViewAsync(
                info.LatticeSize,
                _lifetime.Token);
            ApplyReadyState(info, runtime);
        }
        catch (OperationCanceledException) when (_lifetime.IsCancellationRequested)
        {
            // Normal shutdown.
        }
        catch (Exception ex)
        {
            AppendLog($"ERROR: {ex}");
            StartupStatusText.Text = ex.Message;
            FooterStatusText.Text = "Startup failed — open Logs for details";
            StartupProgress.Visibility = Visibility.Collapsed;
            RetryButton.Visibility = Visibility.Visible;
            SetBackendStatus("Offline", "#EF4444");
            LogsButton.IsEnabled = true;
            RequestSmokeExit(1, $"SMOKE_TEST_FAIL: {ex.Message}");
        }
        finally
        {
            _isBusy = false;
        }
    }

    private async Task<DashboardRuntimeInfo> InitializeWebViewAsync(
        int latticeSize,
        CancellationToken cancellationToken)
    {
        if (DashboardView.CoreWebView2 is null)
        {
            string userDataFolder = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                "FTD",
                "Desktop",
                "WebView2");
            CoreWebView2Environment environment = await CoreWebView2Environment.CreateAsync(
                browserExecutableFolder: null,
                userDataFolder: userDataFolder);
            await DashboardView.EnsureCoreWebView2Async(environment);

            CoreWebView2 core = DashboardView.CoreWebView2!;
            core.Settings.AreDevToolsEnabled = true;
            core.Settings.AreDefaultContextMenusEnabled = true;
            core.Settings.IsWebMessageEnabled = true;
            core.ProcessFailed += (_, args) => Dispatcher.BeginInvoke(() =>
            {
                if (_isClosing)
                    return;
                AppendLog($"WebView2 process failure: {args.ProcessFailedKind}");
                ShowRecoveryState(
                    "The dashboard renderer exited. The CUDA engine log was preserved; retry or restart the engine.",
                    "Renderer exited");
            });
            core.WebMessageReceived += (_, args) =>
            {
                try
                {
                    using JsonDocument document = JsonDocument.Parse(args.WebMessageAsJson);
                    JsonElement root = document.RootElement;
                    string type = root.TryGetProperty("type", out JsonElement typeElement)
                        ? typeElement.GetString() ?? string.Empty
                        : string.Empty;
                    if (type == "engine-progress")
                    {
                        string operation = root.GetProperty("operation").GetString() ?? "operation";
                        string phase = root.GetProperty("phase").GetString() ?? "working";
                        int size = root.GetProperty("size").GetInt32();
                        string message = $"{operation}: {phase} (L={size})";
                        FooterStatusText.Text = message;
                        AppendLog(message);
                    }
                    else if (type == "engine-error")
                    {
                        string message = root.GetProperty("message").GetString()
                            ?? "Native engine command failed.";
                        FooterStatusText.Text = message;
                        AppendLog($"Native engine error: {message}");
                        bool restartRequired = root.TryGetProperty("restartRequired", out JsonElement restartElement)
                            && restartElement.ValueKind == JsonValueKind.True;
                        if (restartRequired)
                        {
                            ShowRecoveryState(
                                $"The CUDA engine must be restarted before it can safely continue. {message}",
                                "Restart required");
                        }
                    }
                }
                catch (Exception ex)
                {
                    AppendLog($"Invalid dashboard host message: {ex.Message}");
                }
            };
            DashboardView.NavigationCompleted += (_, args) =>
            {
                if (!args.IsSuccess)
                    AppendLog($"Dashboard navigation failed: {args.WebErrorStatus}");
            };
        }

        return await NavigateAndVerifyDashboardAsync(latticeSize, cancellationToken);
    }

    private Uri DashboardUri(int latticeSize) => new(
        $"http://127.0.0.1:{_options.DashboardPort}/" +
        $"?engine=native&wsPort={_options.EnginePort}&lattice={latticeSize}");

    private async Task<DashboardRuntimeInfo> NavigateAndVerifyDashboardAsync(
        int latticeSize,
        CancellationToken cancellationToken)
    {
        CoreWebView2 core = DashboardView.CoreWebView2
            ?? throw new InvalidOperationException("WebView2 was not initialized.");
        Uri destination = DashboardUri(latticeSize);
        var navigation = new TaskCompletionSource<(bool Success, string Error)>(
            TaskCreationOptions.RunContinuationsAsynchronously);
        EventHandler<CoreWebView2NavigationCompletedEventArgs>? handler = null;
        handler = (_, args) =>
        {
            navigation.TrySetResult((args.IsSuccess, args.WebErrorStatus.ToString()));
        };
        core.NavigationCompleted += handler;

        try
        {
            using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
            timeout.CancelAfter(TimeSpan.FromSeconds(45));
            core.Navigate(destination.AbsoluteUri);
            (bool success, string error) = await navigation.Task.WaitAsync(timeout.Token);
            if (!success)
            {
                throw new InvalidOperationException(
                    $"The embedded dashboard could not load ({error}).");
            }
        }
        catch (OperationCanceledException) when (!cancellationToken.IsCancellationRequested)
        {
            throw new TimeoutException(
                $"The embedded dashboard did not load from 127.0.0.1:{_options.DashboardPort}.");
        }
        finally
        {
            core.NavigationCompleted -= handler;
        }

        return await WaitForDashboardRuntimeAsync(latticeSize, cancellationToken);
    }

    private async Task<DashboardRuntimeInfo> WaitForDashboardRuntimeAsync(
        int expectedLatticeSize,
        CancellationToken cancellationToken)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(TimeSpan.FromSeconds(90));
        Exception? lastError = null;

        while (!timeout.IsCancellationRequested)
        {
            try
            {
                DashboardRuntimeInfo runtime = await ReadDashboardRuntimeAsync(timeout.Token);
                if (runtime.BridgeReady)
                {
                    ValidateDashboardRuntime(runtime, expectedLatticeSize);
                    return runtime;
                }
            }
            catch (InvalidOperationException)
            {
                // A ready CPU bridge, wrong lattice, software renderer, or lost
                // context is authoritative. Never wait and relabel it as CUDA.
                throw;
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
            "The dashboard loaded, but did not establish a verified native CUDA " +
            "bridge and hardware WebGL renderer.",
            lastError);
    }

    private async Task<DashboardRuntimeInfo> ReadDashboardRuntimeAsync(
        CancellationToken cancellationToken)
    {
        CoreWebView2 core = DashboardView.CoreWebView2
            ?? throw new InvalidOperationException("WebView2 is unavailable.");
        string json = await core.ExecuteScriptAsync(DashboardProbeScript)
            .WaitAsync(cancellationToken);
        return JsonSerializer.Deserialize<DashboardRuntimeInfo>(
            json,
            DashboardJsonOptions)
            ?? throw new InvalidDataException(
                "The dashboard returned an empty runtime descriptor.");
    }

    private static void ValidateDashboardRuntime(
        DashboardRuntimeInfo runtime,
        int? expectedLatticeSize = null)
    {
        if (!runtime.BridgeReady)
            throw new InvalidOperationException("The native dashboard bridge is not ready.");
        if (!runtime.NativeGpu)
        {
            throw new InvalidOperationException(
                "The dashboard fell back to a WASM/CPU bridge. FTD Desktop will " +
                "not silently present that session as CUDA.");
        }
        if (runtime.LatticeSize < 4)
            throw new InvalidOperationException("The dashboard reported an invalid lattice size.");
        if (expectedLatticeSize is int expected && runtime.LatticeSize != expected)
        {
            throw new InvalidOperationException(
                $"The dashboard connected at L={runtime.LatticeSize}, but the " +
                $"verified CUDA engine is at L={expected}.");
        }
        if (!runtime.WebGlAvailable)
            throw new InvalidOperationException("WebView2 did not create a WebGL renderer.");
        if (runtime.ContextLost)
            throw new InvalidOperationException("The WebView2 WebGL context is lost.");
        if (!runtime.HardwareIdentityAvailable)
        {
            throw new InvalidOperationException(
                "WebView2 created a WebGL context but did not expose a hardware " +
                "renderer identity, so GPU rendering could not be verified.");
        }
        if (IsSoftwareRenderer(runtime.WebGlRenderer))
        {
            throw new InvalidOperationException(
                $"WebView2 selected a software renderer ({runtime.WebGlRenderer}). " +
                "Enable hardware acceleration before using FTD Desktop.");
        }
    }

    private static bool IsSoftwareRenderer(string renderer)
    {
        string normalized = renderer.ToLowerInvariant();
        return normalized.Contains("swiftshader", StringComparison.Ordinal) ||
               normalized.Contains("llvmpipe", StringComparison.Ordinal) ||
               normalized.Contains("microsoft basic render", StringComparison.Ordinal) ||
               normalized.Contains("software raster", StringComparison.Ordinal) ||
               normalized.Contains("warp", StringComparison.Ordinal);
    }

    private void ApplyReadyState(EngineInfo info, DashboardRuntimeInfo runtime)
    {
        ValidateDashboardRuntime(runtime, info.LatticeSize);
        _activeLatticeSize = runtime.LatticeSize;
        _engineVersion = info.Version;
        _lastEngineInfo = info;
        _watchdogFailures = 0;

        GpuNameText.Text = $"{info.GpuDescription} · WebGL: {runtime.WebGlRenderer}";
        VersionText.Text =
            $"Engine {info.Version} · L={runtime.LatticeSize} · ws:{_options.EnginePort}";
        RestartButton.Content = info.OwnsProcess ? "Restart engine" : "Reconnect engine";
        StartupProgress.Visibility = Visibility.Visible;

        if (!string.IsNullOrWhiteSpace(info.StartupWarning))
        {
            FooterStatusText.Text = info.StartupWarning;
            SetBackendStatus($"CUDA + WebGL · safe L={runtime.LatticeSize}", "#F59E0B");
            AppendLog(info.StartupWarning);
        }
        else
        {
            FooterStatusText.Text = "Native CUDA bridge and hardware WebGL renderer verified";
            SetBackendStatus("CUDA + WebGL active", "#22C55E");
        }

        AppendLog(
            $"Dashboard verified: native CUDA L={runtime.LatticeSize}; " +
            $"WebGL={runtime.WebGlRenderer}; {runtime.WebGlVersion}.");
        StartupOverlay.Visibility = Visibility.Collapsed;
        SetControlsEnabled(true);
        _dashboardWatchdog.Start();
        RequestSmokeExit(
            0,
            $"SMOKE_TEST_PASS: CUDA L={runtime.LatticeSize}; WebGL={runtime.WebGlRenderer}");
    }

    private async Task NavigateToBlankAsync(CancellationToken cancellationToken)
    {
        CoreWebView2? core = DashboardView.CoreWebView2;
        if (core is null)
            return;

        var navigation = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);
        EventHandler<CoreWebView2NavigationCompletedEventArgs>? handler = null;
        handler = (_, _) => navigation.TrySetResult(true);
        core.NavigationCompleted += handler;
        try
        {
            using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
            timeout.CancelAfter(TimeSpan.FromSeconds(5));
            core.Navigate("about:blank");
            await navigation.Task.WaitAsync(timeout.Token);
            // Navigation tears down the page. Give the loopback WebSocket close
            // a short turn before probing or terminating the single-client server.
            await Task.Delay(100, timeout.Token);
        }
        catch (OperationCanceledException) when (!cancellationToken.IsCancellationRequested)
        {
            AppendLog("Timed out waiting for the dashboard WebSocket to disconnect.");
            core.Stop();
        }
        finally
        {
            core.NavigationCompleted -= handler;
        }
    }

    private void AppendLog(string message)
    {
        string line = $"[{DateTime.Now:HH:mm:ss}] {message}";
        try
        {
            lock (_logFileLock)
                _logWriter?.WriteLine(line);
        }
        catch
        {
            // The in-window log remains available if persistent storage fails.
        }

        lock (_uiLogLock)
        {
            const int maxPendingCharacters = 500_000;
            if (_pendingUiLog.Length + line.Length + Environment.NewLine.Length <=
                maxPendingCharacters)
                _pendingUiLog.AppendLine(line);
            else
                _droppedUiLogLines++;
        }
    }

    private void FlushPendingLog()
    {
        string pending;
        lock (_uiLogLock)
        {
            if (_pendingUiLog.Length == 0 && _droppedUiLogLines == 0)
                return;
            pending = _pendingUiLog.ToString();
            _pendingUiLog.Clear();
            if (_droppedUiLogLines > 0)
            {
                pending +=
                    $"[desktop] {_droppedUiLogLines} UI log lines omitted; " +
                    $"the persistent log remains complete.{Environment.NewLine}";
                _droppedUiLogLines = 0;
            }
        }

        _logBuffer.Append(pending);
        const int maxCharacters = 250_000;
        if (_logBuffer.Length > maxCharacters)
            _logBuffer.Remove(0, _logBuffer.Length - maxCharacters);

        // Keep the persistent log hot, but avoid laying out a hidden 250 KB
        // TextBox on every timer tick.
        if (LogColumn.Width.Value > 0)
        {
            LogTextBox.Text = _logBuffer.ToString();
            LogTextBox.ScrollToEnd();
        }
    }

    private void ShowRecoveryState(string message, string backendStatus)
    {
        _dashboardWatchdog.Stop();
        _watchdogFailures = 0;
        if (_isClosing)
        {
            AppendLog(message);
            return;
        }
        StartupStatusText.Text = message;
        StartupProgress.Visibility = Visibility.Collapsed;
        RetryButton.Visibility = Visibility.Visible;
        StartupOverlay.Visibility = Visibility.Visible;
        FooterStatusText.Text = $"{message} Log: {_sessionLogPath}";
        SetBackendStatus(backendStatus, "#EF4444");
        SetControlsEnabled(false);
        LogsButton.IsEnabled = true;
        RequestSmokeExit(1, $"SMOKE_TEST_FAIL: {message}");
    }

    public void ReportUnhandledException(Exception exception)
    {
        AppendLog($"Unhandled desktop exception: {exception}");
        ShowRecoveryState(
            "The Windows shell encountered an unexpected error. The engine log was preserved; retry or restart the engine.",
            "Shell error");
    }

    private void SetBackendStatus(string text, string color)
    {
        BackendStatusText.Text = text;
        BackendStatusDot.Fill = (Brush)new BrushConverter().ConvertFromString(color)!;
    }

    private void SetControlsEnabled(bool enabled)
    {
        ReloadButton.IsEnabled = enabled;
        RestartButton.IsEnabled = enabled;
        LogsButton.IsEnabled = true;
    }

    private async void ReloadButton_Click(object sender, RoutedEventArgs e)
    {
        if (_isBusy || _isClosing || _lastEngineInfo is null)
            return;

        _isBusy = true;
        _dashboardWatchdog.Stop();
        SetControlsEnabled(false);
        StartupOverlay.Visibility = Visibility.Visible;
        StartupProgress.Visibility = Visibility.Visible;
        RetryButton.Visibility = Visibility.Collapsed;
        StartupStatusText.Text = "Reloading and re-verifying the native dashboard...";
        SetBackendStatus("Verifying", "#F59E0B");

        try
        {
            await NavigateToBlankAsync(_lifetime.Token);
            DashboardRuntimeInfo runtime = await NavigateAndVerifyDashboardAsync(
                _activeLatticeSize,
                _lifetime.Token);
            EngineInfo info = _lastEngineInfo with
            {
                LatticeSize = runtime.LatticeSize,
            };
            ApplyReadyState(info, runtime);
        }
        catch (OperationCanceledException) when (_lifetime.IsCancellationRequested)
        {
            // Normal shutdown.
        }
        catch (Exception ex)
        {
            AppendLog($"Dashboard reload failed: {ex}");
            ShowRecoveryState(
                $"Dashboard reload failed: {ex.Message}",
                "Dashboard offline");
        }
        finally
        {
            _isBusy = false;
        }
    }

    private async void RestartButton_Click(object sender, RoutedEventArgs e)
    {
        if (_isBusy || _isClosing)
            return;

        _isBusy = true;
        _dashboardWatchdog.Stop();
        SetControlsEnabled(false);
        StartupOverlay.Visibility = Visibility.Visible;
        StartupProgress.Visibility = Visibility.Visible;
        RetryButton.Visibility = Visibility.Collapsed;
        StartupStatusText.Text = "Restarting the WSL2 CUDA engine...";
        SetBackendStatus("Restarting", "#F59E0B");

        try
        {
            await NavigateToBlankAsync(_lifetime.Token);
            await (_engineHost?.StopAsync() ?? Task.CompletedTask);
            _engineHost = CreateEngineHost(
                _activeLatticeSize >= 4 ? _activeLatticeSize : null);
            var progress = new Progress<string>(message => StartupStatusText.Text = message);
            EngineInfo info = await _engineHost.StartAsync(progress, _lifetime.Token);
            DashboardRuntimeInfo runtime = await NavigateAndVerifyDashboardAsync(
                info.LatticeSize,
                _lifetime.Token);
            ApplyReadyState(info, runtime);
        }
        catch (OperationCanceledException) when (_lifetime.IsCancellationRequested)
        {
            // Normal shutdown.
        }
        catch (Exception ex)
        {
            AppendLog($"ERROR: {ex}");
            ShowRecoveryState(
                $"Engine restart failed: {ex.Message}",
                "Offline");
        }
        finally
        {
            _isBusy = false;
        }
    }

    private void LogsButton_Click(object sender, RoutedEventArgs e)
    {
        if (LogColumn.Width.Value > 0)
        {
            LogColumn.Width = new GridLength(0);
            return;
        }

        FlushPendingLog();
        LogColumn.Width = new GridLength(440);
        LogTextBox.Text = _logBuffer.ToString();
        LogTextBox.ScrollToEnd();
    }

    private async void RetryButton_Click(object sender, RoutedEventArgs e)
    {
        if (_isBusy || _isClosing)
            return;

        _isBusy = true;
        RetryButton.IsEnabled = false;
        _dashboardWatchdog.Stop();
        try
        {
            await NavigateToBlankAsync(_lifetime.Token);
            if (_engineHost is not null)
                await _engineHost.StopAsync();
            _engineHost = null;
        }
        catch (OperationCanceledException) when (_lifetime.IsCancellationRequested)
        {
            return;
        }
        catch (Exception ex)
        {
            AppendLog($"Retry cleanup warning: {ex.Message}");
        }
        finally
        {
            _isBusy = false;
            RetryButton.IsEnabled = true;
        }

        await InitializeAsync();
    }

    private EngineHost CreateEngineHost(int? latticeSize = null)
    {
        DesktopOptions options = latticeSize is int size
            ? _options with { LatticeSize = size }
            : _options;
        var host = new EngineHost(options, _paths, AppendLog);
        host.UnexpectedExit += exitCode => Dispatcher.BeginInvoke(() =>
        {
            if (_isClosing)
                return;
            AppendLog($"WSL2 CUDA engine exited unexpectedly with code {exitCode}.");
            ShowRecoveryState(
                $"The WSL2 CUDA engine exited with code {exitCode}. Retry will start a clean engine.",
                "Engine exited");
        });
        return host;
    }

    private async void DashboardWatchdog_Tick(object? sender, EventArgs e)
    {
        if (_watchdogBusy || _isBusy || _isClosing ||
            DashboardView.CoreWebView2 is null ||
            StartupOverlay.Visibility == Visibility.Visible)
        {
            return;
        }

        _watchdogBusy = true;
        try
        {
            using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(2));
            DashboardRuntimeInfo runtime = await ReadDashboardRuntimeAsync(timeout.Token);
            ValidateDashboardRuntime(runtime);
            _watchdogFailures = 0;

            if (_activeLatticeSize != runtime.LatticeSize)
            {
                int previous = _activeLatticeSize;
                _activeLatticeSize = runtime.LatticeSize;
                if (_lastEngineInfo is not null)
                {
                    _lastEngineInfo = _lastEngineInfo with
                    {
                        LatticeSize = runtime.LatticeSize,
                        RequestedLatticeSize = runtime.LatticeSize,
                        StartupWarning = null,
                    };
                }
                VersionText.Text =
                    $"Engine {_engineVersion} · L={runtime.LatticeSize} · ws:{_options.EnginePort}";
                AppendLog(
                    $"Dashboard lattice changed from L={previous} to L={runtime.LatticeSize}.");
                FooterStatusText.Text =
                    "Native CUDA bridge and hardware WebGL renderer verified";
                SetBackendStatus("CUDA + WebGL active", "#22C55E");
            }
        }
        catch (Exception ex)
        {
            if (_isClosing)
                return;
            _watchdogFailures++;
            if (_watchdogFailures == 1)
                AppendLog($"Dashboard health check failed: {ex.Message}");
            if (_watchdogFailures >= 3)
            {
                AppendLog($"Dashboard health check failed 3 times: {ex}");
                ShowRecoveryState(
                    "The dashboard lost its verified native CUDA bridge or hardware " +
                    "WebGL context. Reload or restart to recover.",
                    "GPU link lost");
            }
        }
        finally
        {
            _watchdogBusy = false;
        }
    }

    private void RequestSmokeExit(int exitCode, string message)
    {
        if (!_options.SmokeTest || _smokeExitRequested)
            return;

        _smokeExitRequested = true;
        Environment.ExitCode = exitCode;
        AppendLog(message);
        Dispatcher.BeginInvoke(Close, DispatcherPriority.Background);
    }

    private async Task WaitForActiveOperationAsync()
    {
        using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(15));
        try
        {
            while (_isBusy)
                await Task.Delay(50, timeout.Token);
        }
        catch (OperationCanceledException)
        {
            AppendLog(
                "Timed out waiting for the active desktop operation to observe shutdown.");
        }
    }

    private async void MainWindow_Closing(object? sender, System.ComponentModel.CancelEventArgs e)
    {
        if (_isClosing)
            return;

        e.Cancel = true;
        _isClosing = true;
        _dashboardWatchdog.Stop();
        SetBackendStatus("Stopping", "#F59E0B");
        _lifetime.Cancel();

        try
        {
            try
            {
                using var disconnectTimeout =
                    new CancellationTokenSource(TimeSpan.FromSeconds(6));
                await NavigateToBlankAsync(disconnectTimeout.Token);
            }
            catch (Exception ex)
            {
                AppendLog($"Dashboard disconnect during shutdown failed: {ex.Message}");
            }

            await WaitForActiveOperationAsync();
            try
            {
                if (_engineHost is not null)
                    await _engineHost.DisposeAsync();
            }
            finally
            {
                await _dashboardServer.DisposeAsync();
            }
        }
        catch (Exception ex)
        {
            AppendLog($"Desktop shutdown cleanup failed: {ex}");
            if (_options.SmokeTest)
                Environment.ExitCode = 1;
        }
        finally
        {
            _lifetime.Cancel();
            _lifetime.Dispose();
            _logFlushTimer.Stop();
            FlushPendingLog();
            lock (_logFileLock)
            {
                _logWriter?.Dispose();
                _logWriter = null;
            }
            Closing -= MainWindow_Closing;
            Close();
        }
    }
}
