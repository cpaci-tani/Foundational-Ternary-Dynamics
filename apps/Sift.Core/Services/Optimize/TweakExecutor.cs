using System.Diagnostics;
using System.IO;
using System.Text.Json;
using Sift.Infrastructure.Persistence;
using Sift.Models;
using Microsoft.Win32;

namespace Sift.Services;

public sealed class TweakExecutor : ITweakExecutor
{
    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };
    private static readonly string SystemDirectory = Environment.SystemDirectory;
    private static readonly string WindowsDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Windows);
    private static readonly TimeSpan PowerConfigurationTimeout = TimeSpan.FromSeconds(30);
    private static readonly TimeSpan PowerShellTimeout = TimeSpan.FromMinutes(2);
    private static readonly TimeSpan RepairCommandTimeout = TimeSpan.FromMinutes(90);
    private const int MaximumCapturedProcessOutputCharacters = 2 * 1024 * 1024;
    private readonly IReadOnlyDictionary<string, Tweak> _allowedTweaks;
    public string BackupDirectory { get; }

    public TweakExecutor(string? backupDirectory = null, IEnumerable<Tweak>? allowedTweaks = null)
    {
        BackupDirectory = backupDirectory ?? ProductPaths.BackupDirectory;
        _allowedTweaks = (allowedTweaks ?? TweakCatalog.Create()).ToDictionary(x => x.Id, StringComparer.OrdinalIgnoreCase);
    }

    public bool IsApplied(Tweak tweak)
    {
        try
        {
            return tweak.Kind switch
            {
                TweakKind.Registry => Equals(ReadRegistryValue(tweak)?.ToString(), tweak.DesiredValue?.ToString()),
                TweakKind.AppPackage => !PackageExists(tweak.Target),
                TweakKind.Command => IsCommandApplied(tweak),
                _ => false
            };
        }
        catch { return false; }
    }

    private static bool IsCommandApplied(Tweak tweak)
    {
        if (tweak.Id == "power.hibernate")
            return !IsHibernationAvailable();
        return false;
    }

    private static bool IsHibernationAvailable()
    {
        try
        {
            var output = RunProcess(
                CreateTrustedProcessStartInfo("powercfg.exe", ["/a"]),
                PowerConfigurationTimeout);
            // When hibernation is off, powercfg /a typically lists Hibernate under unavailable.
            if (output.Contains("Hibernation has not been enabled", StringComparison.OrdinalIgnoreCase)) return false;
            if (output.Contains("Hibernate", StringComparison.OrdinalIgnoreCase) &&
                output.Contains("The following sleep states are available", StringComparison.OrdinalIgnoreCase) &&
                !output.Contains("Hibernate (disabled", StringComparison.OrdinalIgnoreCase))
            {
                var availableSection = output.Split("The following sleep states are not available", 2)[0];
                return availableSection.Contains("Hibernate", StringComparison.OrdinalIgnoreCase);
            }
            var hiberfil = Path.Combine(Path.GetPathRoot(Environment.SystemDirectory) ?? "C:\\", "hiberfil.sys");
            return File.Exists(hiberfil);
        }
        catch
        {
            return false;
        }
    }

    public async Task<ApplyResult> ApplyAsync(IEnumerable<Tweak> selection, bool dryRun,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var selected = ValidateSelection(selection);
        var log = new List<string>();
        var succeeded = 0;
        var failed = 0;
        var previewed = 0;

        if (dryRun)
        {
            foreach (var tweak in selected)
            {
                cancellationToken.ThrowIfCancellationRequested();
                previewed++;
                log.Add($"PREFLIGHT  {tweak.Title} → {DescribeTarget(tweak)}");
            }
            return new ApplyResult { BackupPath = "", Log = log, Previewed = previewed };
        }

        if (selected.Any(ElevatedOperationPolicy.IsElevatedOptimizeTweak))
        {
            if (!ElevationHelper.IsElevated())
                throw new UnauthorizedAccessException("Machine-wide Sift actions require administrator permission.");
            if (!IsTrustedSignedMutationHost())
                throw new UnauthorizedAccessException(
                    "Machine-wide actions require a trusted, signed Sift executable. This build is not trusted for protected changes.");
        }

        cancellationToken.ThrowIfCancellationRequested();
        Directory.CreateDirectory(BackupDirectory);
        var backup = new Backup();
        foreach (var tweak in selected)
        {
            cancellationToken.ThrowIfCancellationRequested();
            backup.Entries.Add(Capture(tweak));
        }
        var path = Path.Combine(BackupDirectory,
            $"backup-{DateTime.UtcNow:yyyyMMdd-HHmmss-fff}-{backup.OperationId[..8]}.json");
        await WriteBackupAsync(path, backup);

        for (var i = 0; i < selected.Count; i++)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var tweak = selected[i];
            var entry = backup.Entries[i];
            try
            {
                entry.State = BackupEntryStates.Applying;
                await WriteBackupAsync(path, backup);
                await ApplyOneAsync(tweak, cancellationToken);
                entry.AppliedSuccessfully = true;
                entry.State = BackupEntryStates.Applied;
                entry.AppliedUtc = DateTime.UtcNow;
                succeeded++;
                log.Add($"APPLIED  {tweak.Title}");
                await WriteBackupAsync(path, backup);
            }
            catch (OperationCanceledException)
            {
                throw;
            }
            catch (Exception ex)
            {
                failed++;
                entry.FailureDetail = ex.Message;
                // Keep Applying: a failed command can still have partially mutated state,
                // so recovery must conservatively offer its undo path.
                log.Add($"FAILED   {tweak.Title}: {ex.Message}");
                await WriteBackupAsync(path, backup);
            }
        }

        return new ApplyResult
        {
            BackupPath = dryRun ? "" : path,
            Log = log,
            Succeeded = succeeded,
            Failed = failed,
            Previewed = previewed
        };
    }

    public async Task<RestoreResult> RestoreFromAsync(string path, IReadOnlyDictionary<string, Tweak> catalog,
        RestoreScope scope = RestoreScope.All)
    {
        path = ValidateBackupPath(path);
        if (!File.Exists(path))
            throw new InvalidOperationException("The selected backup file was not found.");

        var backup = JsonSerializer.Deserialize<Backup>(await File.ReadAllTextAsync(path), JsonOptions)
            ?? throw new InvalidOperationException("The backup could not be read.");
        var log = new List<string>();
        var restored = 0;
        var skipped = 0;
        var failed = 0;

        foreach (var entry in backup.Entries.Where(ShouldRestore).Reverse())
        {
            if (entry.RegistryTree is not null)
            {
                var machineTree = string.Equals(entry.RegistryHive, "HKLM", StringComparison.OrdinalIgnoreCase);
                if (!IncludedInScope(scope, machineTree)) continue;
                if (!MaintenancePolicy.IsAllowedUninstallKey(entry.RegistryHive, entry.RegistrySubKey))
                {
                    skipped++;
                    log.Add($"SKIPPED  {entry.TweakId} (the registry location is not supported for restore)");
                    continue;
                }
                try
                {
                    var hive = entry.RegistryHive == "HKCU" ? Registry.CurrentUser : Registry.LocalMachine;
                    RegistrySnapshotCodec.RestoreTree(hive, entry.RegistrySubKey!, entry.RegistryTree);
                    entry.State = BackupEntryStates.Restored;
                    entry.RestoredUtc = DateTime.UtcNow;
                    await WriteBackupAsync(path, backup);
                    restored++;
                    log.Add($"RESTORED {entry.TweakId}");
                }
                catch (Exception ex)
                {
                    failed++;
                    log.Add($"FAILED   {entry.TweakId}: {ex.Message}");
                }
                continue;
            }

            if (!_allowedTweaks.TryGetValue(entry.TweakId, out var tweak) ||
                !catalog.TryGetValue(entry.TweakId, out var supplied) || !SameDefinition(tweak, supplied))
            {
                skipped++;
                log.Add($"SKIPPED  {entry.TweakId} (no longer in catalog)");
                continue;
            }

            var machineAction = tweak.Kind == TweakKind.Command ||
                tweak.Kind == TweakKind.Registry && tweak.Target.StartsWith("HKLM\\", StringComparison.OrdinalIgnoreCase);
            if (!IncludedInScope(scope, machineAction)) continue;

            if (!tweak.Reversible)
            {
                skipped++;
                log.Add($"SKIPPED  {tweak.Title} (manual reinstall or recovery required)");
                continue;
            }

            try
            {
                if (tweak.Kind == TweakKind.Registry) RestoreRegistry(tweak, entry);
                else if (tweak.UndoCommand is not null)
                    await RunCommandAsync(tweak.UndoCommand, CancellationToken.None);
                entry.State = BackupEntryStates.Restored;
                entry.RestoredUtc = DateTime.UtcNow;
                await WriteBackupAsync(path, backup);
                restored++;
                log.Add($"RESTORED {tweak.Title}");
            }
            catch (Exception ex)
            {
                failed++;
                log.Add($"FAILED   {tweak.Title}: {ex.Message}");
            }
        }

        return new RestoreResult
        {
            BackupPath = path,
            Log = log,
            Restored = restored,
            Skipped = skipped,
            Failed = failed
        };
    }

    public IReadOnlyList<BackupInfo> ListBackups()
    {
        if (!Directory.Exists(BackupDirectory)) return [];
        var results = new List<BackupInfo>();
        foreach (var path in Directory.GetFiles(BackupDirectory, "backup-*.json")
                     .Where(path => !Path.GetFileName(path).StartsWith("backup-registry-", StringComparison.OrdinalIgnoreCase))
                     .OrderDescending())
        {
            try
            {
                var backup = JsonSerializer.Deserialize<Backup>(File.ReadAllText(path), JsonOptions);
                if (backup is null) continue;
                results.Add(new BackupInfo
                {
                    Path = path,
                    CreatedUtc = backup.CreatedUtc,
                    EntryCount = backup.Entries.Count,
                    SuccessCount = backup.Entries.Count(x => x.AppliedSuccessfully || x.State == BackupEntryStates.Applied)
                });
            }
            catch { /* skip unreadable backups */ }
        }
        return results;
    }

    private static Task WriteBackupAsync(string path, Backup backup)
    {
        AtomicFile.WriteAllText(path, JsonSerializer.Serialize(backup, JsonOptions));
        return Task.CompletedTask;
    }

    private static BackupEntry Capture(Tweak tweak)
    {
        if (tweak.Kind != TweakKind.Registry) return new() { TweakId = tweak.Id };
        var (hive, path) = ParseTarget(tweak.Target);
        var captured = RegistrySnapshotCodec.CaptureValue(hive, path, tweak.ValueName!);
        return new()
        {
            TweakId = tweak.Id,
            KeyExisted = captured.KeyExisted,
            Existed = captured.ValueExisted,
            RegistryValue = captured.Snapshot,
            Value = captured.Snapshot?.Data,
            RegistryKind = captured.Snapshot?.Kind
        };
    }

    private static Task ApplyOneAsync(Tweak tweak, CancellationToken cancellationToken)
    {
        if (tweak.Kind == TweakKind.Registry)
        {
            var (hive, path) = ParseTarget(tweak.Target);
            using var key = hive.CreateSubKey(path, true);
            key.SetValue(tweak.ValueName!, tweak.DesiredValue!, RegistryValueKind.DWord);
            return Task.CompletedTask;
        }
        if (tweak.Kind == TweakKind.Command) return RunCommandAsync(tweak.ApplyCommand!, cancellationToken);
        return RunPowerShellAsync(
            $"Get-AppxPackage -Name '{tweak.Target}' | Remove-AppxPackage -ErrorAction Stop",
            cancellationToken);
    }

    private static void RestoreRegistry(Tweak tweak, BackupEntry entry)
    {
        var (hive, path) = ParseTarget(tweak.Target);
        RegistrySnapshotCodec.RestoreValue(hive, path, tweak.ValueName!, entry);
    }

    private static (RegistryKey hive, string path) ParseTarget(string target)
    {
        var split = target.Split('\\', 2);
        if (split.Length != 2 || string.IsNullOrWhiteSpace(split[1]))
            throw new InvalidOperationException($"Invalid registry target '{target}'.");
        return split[0].ToUpperInvariant() switch
        {
            "HKCU" => (Registry.CurrentUser, split[1]),
            "HKLM" => (Registry.LocalMachine, split[1]),
            _ => throw new InvalidOperationException($"Unsupported registry hive '{split[0]}'.")
        };
    }

    private static object? ReadRegistryValue(Tweak tweak)
    {
        var (hive, path) = ParseTarget(tweak.Target);
        using var key = hive.OpenSubKey(path);
        return key?.GetValue(tweak.ValueName!, null, RegistryValueOptions.DoNotExpandEnvironmentNames);
    }

    private static readonly object PackageCacheGate = new();
    private static HashSet<string>? _installedPackageNames;
    private static long _packageCacheTimestamp;

    private static bool PackageExists(string name)
    {
        if (string.IsNullOrWhiteSpace(name)) return false;
        EnsurePackageCache();
        return _installedPackageNames!.Contains(name) ||
               _installedPackageNames.Any(installed =>
                   installed.StartsWith(name + "_", StringComparison.OrdinalIgnoreCase) ||
                   installed.Equals(name, StringComparison.OrdinalIgnoreCase));
    }

    private static void EnsurePackageCache()
    {
        lock (PackageCacheGate)
        {
            var now = Environment.TickCount64;
            if (_installedPackageNames is not null && now - _packageCacheTimestamp < 60_000) return;
            var output = RunProcess(
                CreatePowerShellStartInfo("Get-AppxPackage | Select-Object -ExpandProperty Name"),
                PowerShellTimeout);
            _installedPackageNames = new HashSet<string>(
                output.Split(['\r', '\n'], StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries),
                StringComparer.OrdinalIgnoreCase);
            _packageCacheTimestamp = now;
        }
    }

    private static Task RunPowerShellAsync(string command, CancellationToken cancellationToken) =>
        RunProcessForSuccessAsync(CreatePowerShellStartInfo(command), PowerShellTimeout, cancellationToken);

    private static Task RunCommandAsync(string command, CancellationToken cancellationToken)
    {
        var (startInfo, timeout) = CreateCatalogCommand(command);
        return RunProcessForSuccessAsync(startInfo, timeout, cancellationToken);
    }

    internal static ProcessStartInfo CreateCatalogCommandStartInfo(string command) =>
        CreateCatalogCommand(command).StartInfo;

    internal static ProcessStartInfo CreatePowerShellStartInfo(string command) =>
        CreateTrustedProcessStartInfo("powershell.exe",
        [
            "-NoLogo", "-NoProfile", "-NonInteractive", "-ExecutionPolicy", "RemoteSigned",
            "-Command", command
        ]);

    internal static ProcessStartInfo CreateTrustedProcessStartInfo(
        string executableName,
        IReadOnlyList<string> arguments)
    {
        var executable = ResolveTrustedExecutable(executableName);
        var info = new ProcessStartInfo(executable)
        {
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardError = true,
            RedirectStandardOutput = true,
            WorkingDirectory = SystemDirectory
        };
        foreach (var argument in arguments) info.ArgumentList.Add(argument);
        info.Environment.Clear();
        AddEnvironment(info, "SystemRoot", WindowsDirectory);
        AddEnvironment(info, "windir", WindowsDirectory);
        AddEnvironment(info, "SystemDrive", Path.GetPathRoot(WindowsDirectory) ?? "C:\\");
        AddEnvironment(info, "ComSpec", Path.Combine(SystemDirectory, "cmd.exe"));
        AddEnvironment(info, "PATH", string.Join(Path.PathSeparator,
            SystemDirectory,
            Path.Combine(SystemDirectory, "Wbem"),
            Path.Combine(SystemDirectory, "WindowsPowerShell", "v1.0")));
        AddEnvironment(info, "PATHEXT", ".COM;.EXE;.BAT;.CMD");
        AddEnvironment(info, "TEMP", Path.GetTempPath().TrimEnd(Path.DirectorySeparatorChar));
        AddEnvironment(info, "TMP", Path.GetTempPath().TrimEnd(Path.DirectorySeparatorChar));
        AddEnvironment(info, "USERPROFILE", Environment.GetFolderPath(Environment.SpecialFolder.UserProfile));
        AddEnvironment(info, "LOCALAPPDATA", Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));
        AddEnvironment(info, "APPDATA", Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData));
        AddEnvironment(info, "PSModulePath", string.Join(Path.PathSeparator,
            Path.Combine(SystemDirectory, "WindowsPowerShell", "v1.0", "Modules"),
            Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles),
                "WindowsPowerShell", "Modules")));
        AddEnvironment(info, "POWERSHELL_TELEMETRY_OPTOUT", "1");
        return info;
    }

    private static (ProcessStartInfo StartInfo, TimeSpan Timeout) CreateCatalogCommand(string command) =>
        command switch
        {
            "powercfg.exe /hibernate off" =>
                (CreateTrustedProcessStartInfo("powercfg.exe", ["/hibernate", "off"]), PowerConfigurationTimeout),
            "powercfg.exe /hibernate on" =>
                (CreateTrustedProcessStartInfo("powercfg.exe", ["/hibernate", "on"]), PowerConfigurationTimeout),
            "DISM.exe /Online /Cleanup-Image /StartComponentCleanup" =>
                (CreateTrustedProcessStartInfo("dism.exe", ["/Online", "/Cleanup-Image", "/StartComponentCleanup"]),
                    RepairCommandTimeout),
            "sfc.exe /scannow" =>
                (CreateTrustedProcessStartInfo("sfc.exe", ["/scannow"]), RepairCommandTimeout),
            _ => throw new InvalidOperationException("This command is not in Sift's trusted executable map.")
        };

    private static string ResolveTrustedExecutable(string executableName) =>
        executableName.ToLowerInvariant() switch
        {
            "powercfg.exe" => Path.Combine(SystemDirectory, "powercfg.exe"),
            "dism.exe" => Path.Combine(SystemDirectory, "dism.exe"),
            "sfc.exe" => Path.Combine(SystemDirectory, "sfc.exe"),
            "schtasks.exe" => Path.Combine(SystemDirectory, "schtasks.exe"),
            "powershell.exe" => Path.Combine(SystemDirectory, "WindowsPowerShell", "v1.0", "powershell.exe"),
            _ => throw new InvalidOperationException($"The executable '{executableName}' is not trusted by Sift.")
        };

    internal static bool IsTrustedSignedMutationHost()
    {
        var processPath = Environment.ProcessPath;
        return BinaryTrustPolicy.IsTrusted(processPath);
    }

    private static void AddEnvironment(ProcessStartInfo info, string name, string value)
    {
        if (!string.IsNullOrWhiteSpace(value)) info.Environment[name] = value;
    }

    internal static async Task RunProcessForSuccessAsync(
        ProcessStartInfo startInfo,
        TimeSpan timeout,
        CancellationToken cancellationToken)
    {
        _ = await RunProcessAsync(startInfo, timeout, cancellationToken);
    }

    private static string RunProcess(ProcessStartInfo startInfo, TimeSpan timeout) =>
        RunProcessAsync(startInfo, timeout, CancellationToken.None).GetAwaiter().GetResult().StandardOutput;

    internal static async Task<TrustedProcessResult> RunProcessAsync(
        ProcessStartInfo startInfo,
        TimeSpan timeout,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        using var process = new Process { StartInfo = startInfo };
        if (!process.Start()) throw new InvalidOperationException($"Could not start {startInfo.FileName}.");
        var outputTask = DrainBoundedAsync(process.StandardOutput, MaximumCapturedProcessOutputCharacters);
        var errorTask = DrainBoundedAsync(process.StandardError, MaximumCapturedProcessOutputCharacters);
        using var deadline = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        deadline.CancelAfter(timeout);
        try
        {
            await process.WaitForExitAsync(deadline.Token);
        }
        catch (OperationCanceledException)
        {
            await TerminateProcessTreeAsync(process);
            try { await Task.WhenAll(outputTask, errorTask).WaitAsync(TimeSpan.FromSeconds(5)); }
            catch { /* Process termination is authoritative; drain cleanup is best effort. */ }
            if (cancellationToken.IsCancellationRequested) cancellationToken.ThrowIfCancellationRequested();
            throw new TimeoutException($"{Path.GetFileName(startInfo.FileName)} exceeded its {timeout:g} execution limit.");
        }

        var output = await outputTask;
        var error = await errorTask;
        if (process.ExitCode != 0)
            throw new InvalidOperationException(string.IsNullOrWhiteSpace(error)
                ? $"{Path.GetFileName(startInfo.FileName)} exited with code {process.ExitCode}."
                : error.Trim());
        return new TrustedProcessResult(output, error);
    }

    private static async Task<string> DrainBoundedAsync(StreamReader reader, int maximumCharacters)
    {
        var buffer = new char[8_192];
        var captured = new System.Text.StringBuilder(Math.Min(maximumCharacters, 16_384));
        while (true)
        {
            var read = await reader.ReadAsync(buffer);
            if (read == 0) break;
            if (captured.Length >= maximumCharacters) continue;
            captured.Append(buffer, 0, Math.Min(read, maximumCharacters - captured.Length));
        }
        return captured.ToString();
    }

    private static async Task TerminateProcessTreeAsync(Process process)
    {
        if (!process.HasExited)
        {
            try { process.Kill(entireProcessTree: true); }
            catch (InvalidOperationException) when (process.HasExited) { }
        }
        try { await process.WaitForExitAsync(CancellationToken.None).WaitAsync(TimeSpan.FromSeconds(5)); }
        catch (TimeoutException exception)
        {
            throw new InvalidOperationException("The trusted command process tree did not exit within five seconds.", exception);
        }
    }

    internal sealed record TrustedProcessResult(string StandardOutput, string StandardError);

    private static string DescribeTarget(Tweak t) => t.Kind switch
    {
        TweakKind.Registry => $"{t.Target}\\{t.ValueName} = {t.DesiredValue}",
        TweakKind.AppPackage => $"remove package {t.Target}",
        _ => t.Target
    };

    private List<Tweak> ValidateSelection(IEnumerable<Tweak> selection)
    {
        var selected = selection.ToList();
        if (selected.Select(x => x.Id).Distinct(StringComparer.OrdinalIgnoreCase).Count() != selected.Count)
            throw new InvalidOperationException("A tweak may only appear once in an operation.");
        foreach (var tweak in selected)
            if (!_allowedTweaks.TryGetValue(tweak.Id, out var allowed) || !SameDefinition(tweak, allowed))
                throw new InvalidOperationException($"Setting '{tweak.Id}' is not present in the Sift catalog.");
        return selected;
    }

    private static bool SameDefinition(Tweak left, Tweak right) =>
        left.Kind == right.Kind && left.Target == right.Target && left.ValueName == right.ValueName &&
        Equals(left.DesiredValue, right.DesiredValue) && left.ApplyCommand == right.ApplyCommand &&
        left.UndoCommand == right.UndoCommand && left.Reversible == right.Reversible && left.Risk == right.Risk &&
        left.RequiresElevation == right.RequiresElevation;

    private static bool ShouldRestore(BackupEntry entry) =>
        entry.State != BackupEntryStates.Restored &&
        (entry.AppliedSuccessfully || entry.State is BackupEntryStates.Applying or BackupEntryStates.Applied);

    private static bool IncludedInScope(RestoreScope scope, bool machineAction) => scope switch
    {
        RestoreScope.All => true,
        RestoreScope.CurrentUser => !machineAction,
        RestoreScope.ElevatedMachine => machineAction,
        _ => false
    };

    private string ValidateBackupPath(string path)
    {
        var full = Path.GetFullPath(path);
        var root = Path.GetFullPath(BackupDirectory).TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar;
        if (!full.StartsWith(root, StringComparison.OrdinalIgnoreCase) ||
            !Path.GetFileName(full).StartsWith("backup-", StringComparison.OrdinalIgnoreCase) ||
            !full.EndsWith(".json", StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("Restore is limited to Sift backup files.");
        return full;
    }
}
