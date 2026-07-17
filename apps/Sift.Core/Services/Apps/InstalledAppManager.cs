using System.Diagnostics;
using System.Globalization;
using System.Collections.Concurrent;
using System.Text.Json;
using Sift.Infrastructure.Persistence;
using Sift.Models;
using Microsoft.Win32;

namespace Sift.Services;

public interface IInstalledAppInventory
{
    IReadOnlyList<InstalledApp> Enumerate(CancellationToken cancellationToken = default);
    InstalledApp? FindExact(InstalledAppRegistryLocation location);
}

public interface IInstalledAppManager
{
    Task<InstalledAppActionResult> UninstallAsync(InstalledApp app, bool preview, CancellationToken cancellationToken = default);
    Task<InstalledAppUninstallCompletion> WaitForUninstallCompletionAsync(InstalledApp app, string sessionId,
        CancellationToken cancellationToken = default);
    Task<InstalledAppUninstallCompletion> CheckUninstallCompletionAsync(InstalledApp app, string sessionId,
        CancellationToken cancellationToken = default);
    Task<InstalledAppActionResult> CleanupRegistrationAsync(InstalledApp app, bool preview, CancellationToken cancellationToken = default);
}

public interface IInstalledAppLauncher
{
    IInstalledAppLaunchHandle? Launch(InstalledAppLaunchPlan plan);
}

public interface IInstalledAppLaunchHandle : IDisposable
{
    int? ProcessId { get; }
    Task WaitForExitAsync(CancellationToken cancellationToken = default);
}

public sealed class InstalledAppLauncher : IInstalledAppLauncher
{
    public IInstalledAppLaunchHandle? Launch(InstalledAppLaunchPlan plan)
    {
        var process = Process.Start(new ProcessStartInfo(plan.FileName, plan.Arguments)
        {
            UseShellExecute = true,
            WorkingDirectory = Path.GetDirectoryName(plan.FileName) ?? Environment.CurrentDirectory
        });
        return process is null ? null : new InstalledAppLaunchHandle(process);
    }
}

internal sealed class InstalledAppLaunchHandle(Process process) : IInstalledAppLaunchHandle
{
    public int? ProcessId
    {
        get
        {
            try { return process.Id; }
            catch (InvalidOperationException) { return null; }
        }
    }

    public Task WaitForExitAsync(CancellationToken cancellationToken = default) => process.WaitForExitAsync(cancellationToken);
    public void Dispose() => process.Dispose();
}

public sealed class InstalledAppInventory : IInstalledAppInventory
{
    private const string UninstallRoot = @"Software\Microsoft\Windows\CurrentVersion\Uninstall";

    private static readonly (RegistryHive Hive, RegistryView View, string Source)[] Sources =
    [
        (RegistryHive.CurrentUser, RegistryView.Registry64, "Current user"),
        (RegistryHive.LocalMachine, RegistryView.Registry64, "64-bit machine"),
        (RegistryHive.LocalMachine, RegistryView.Registry32, "32-bit machine")
    ];

    public IReadOnlyList<InstalledApp> Enumerate(CancellationToken cancellationToken = default)
    {
        var apps = new List<InstalledApp>();
        var elevated = ElevationHelper.IsElevated();
        foreach (var source in Sources)
        {
            cancellationToken.ThrowIfCancellationRequested();
            try
            {
                using var baseKey = RegistryKey.OpenBaseKey(source.Hive, source.View);
                using var root = baseKey.OpenSubKey(UninstallRoot, writable: false);
                if (root is null) continue;
                foreach (var subKeyName in root.GetSubKeyNames())
                {
                    cancellationToken.ThrowIfCancellationRequested();
                    using var entry = root.OpenSubKey(subKeyName, writable: false);
                    if (entry is null) continue;
                    var app = ReadEntry(entry, source.Hive, source.View, subKeyName, source.Source, elevated);
                    if (app is not null) apps.Add(app);
                }
            }
            catch (Exception exception) when (exception is UnauthorizedAccessException or System.Security.SecurityException or IOException)
            {
                // A protected or transient entry must not prevent the rest of the inventory from loading.
            }
        }

        return apps
            .GroupBy(app => $"{app.DisplayName}\u001f{app.DisplayVersion}\u001f{app.Publisher}", StringComparer.OrdinalIgnoreCase)
            .Select(group => group.OrderByDescending(app => app.CanUninstall)
                .ThenByDescending(app => app.IsOrphanedRegistration)
                .ThenBy(app => app.Source).First())
            .OrderBy(app => app.DisplayName, StringComparer.CurrentCultureIgnoreCase)
            .ToList();
    }

    public InstalledApp? FindExact(InstalledAppRegistryLocation location)
    {
        if (!TryResolveLocation(location, out var hive, out var view, out var subKeyName, out var source)) return null;
        try
        {
            using var baseKey = RegistryKey.OpenBaseKey(hive, view);
            using var root = baseKey.OpenSubKey(UninstallRoot, writable: false);
            using var entry = root?.OpenSubKey(subKeyName, writable: false);
            return entry is null ? null : ReadEntry(entry, hive, view, subKeyName, source, ElevationHelper.IsElevated());
        }
        catch (Exception exception) when (exception is UnauthorizedAccessException or System.Security.SecurityException or IOException)
        {
            return null;
        }
    }

    private static InstalledApp? ReadEntry(RegistryKey entry, RegistryHive hive, RegistryView view, string subKeyName, string source, bool elevated)
    {
        var values = new InstalledAppRegistryValues(
            ReadString(entry, "DisplayName"),
            ReadString(entry, "Publisher"),
            ReadString(entry, "DisplayVersion"),
            ReadString(entry, "InstallLocation"),
            FormatInstallDate(ReadString(entry, "InstallDate")),
            ReadEstimatedSize(entry),
            ReadString(entry, "UninstallString"),
            ReadInt32(entry, "WindowsInstaller") == 1,
            ReadInt32(entry, "SystemComponent") == 1,
            ReadString(entry, "ReleaseType"),
            ReadString(entry, "ParentKeyName"));
        if (!IsUsableDisplayName(values.DisplayName)) return null;

        var evaluation = InstalledAppPolicy.Evaluate(values);
        var location = new InstalledAppRegistryLocation(HiveName(hive), ViewName(view), $@"{UninstallRoot}\{subKeyName}");
        var orphaned = InstalledAppPolicy.IsConservativeOrphan(values, out var orphanEvidence);
        return new InstalledApp(
            location,
            values.DisplayName,
            values.Publisher,
            values.DisplayVersion,
            values.InstallLocation,
            values.InstallDate,
            values.EstimatedSizeBytes,
            values.UninstallString,
            source,
            evaluation.Allowed,
            evaluation.Reason)
        {
            IsOrphanedRegistration = orphaned,
            CanCleanRegistration = orphaned && (location.Hive == "HKCU" || elevated),
            OrphanEvidence = orphanEvidence,
            IconPng = AppIconExtractor.TryExtractPng(ReadString(entry, "DisplayIcon"))
        };
    }

    private static bool TryResolveLocation(InstalledAppRegistryLocation location, out RegistryHive hive, out RegistryView view,
        out string subKeyName, out string source)
    {
        hive = location.Hive.Equals("HKCU", StringComparison.OrdinalIgnoreCase) ? RegistryHive.CurrentUser : RegistryHive.LocalMachine;
        view = location.View.Equals("32-bit", StringComparison.OrdinalIgnoreCase) ? RegistryView.Registry32 : RegistryView.Registry64;
        source = hive == RegistryHive.CurrentUser ? "Current user" : view == RegistryView.Registry32 ? "32-bit machine" : "64-bit machine";
        subKeyName = string.Empty;

        if (!location.Hive.Equals("HKCU", StringComparison.OrdinalIgnoreCase) &&
            !location.Hive.Equals("HKLM", StringComparison.OrdinalIgnoreCase)) return false;
        var prefix = UninstallRoot + "\\";
        if (!location.SubKeyName.StartsWith(prefix, StringComparison.OrdinalIgnoreCase)) return false;
        subKeyName = location.SubKeyName[prefix.Length..];
        return !string.IsNullOrWhiteSpace(subKeyName) && !subKeyName.Contains('\\');
    }

    private static string HiveName(RegistryHive hive) => hive == RegistryHive.CurrentUser ? "HKCU" : "HKLM";
    private static string ViewName(RegistryView view) => view == RegistryView.Registry32 ? "32-bit" : "64-bit";
    private static string ReadString(RegistryKey key, string name) => key.GetValue(name)?.ToString()?.Trim() ?? string.Empty;

    private static int ReadInt32(RegistryKey key, string name) => key.GetValue(name) switch
    {
        int value => value,
        long value => checked((int)value),
        string value when int.TryParse(value, out var parsed) => parsed,
        _ => 0
    };

    private static long ReadEstimatedSize(RegistryKey key)
    {
        var kilobytes = key.GetValue("EstimatedSize") switch
        {
            int value when value > 0 => (long)(uint)value,
            long value when value > 0 => value,
            string value when long.TryParse(value, out var parsed) && parsed > 0 => parsed,
            _ => 0
        };
        return kilobytes > long.MaxValue / 1024 ? 0 : kilobytes * 1024;
    }

    private static bool IsUsableDisplayName(string value) =>
        !string.IsNullOrWhiteSpace(value) &&
        !value.StartsWith("@{", StringComparison.Ordinal) &&
        !value.StartsWith("${{", StringComparison.Ordinal) &&
        !value.Contains("ms-resource:", StringComparison.OrdinalIgnoreCase);

    private static string FormatInstallDate(string raw) =>
        DateTime.TryParseExact(raw, "yyyyMMdd", CultureInfo.InvariantCulture, DateTimeStyles.None, out var date)
            ? date.ToString("d", CultureInfo.CurrentCulture)
            : raw;
}

public sealed class InstalledAppManager : IInstalledAppManager, IAppLeftoverManager
{
    private sealed class TrackedUninstall(
        InstalledApp app,
        IInstalledAppLaunchHandle handle,
        DateTime createdUtc,
        DateTime expiresUtc)
    {
        public InstalledApp App { get; } = app;
        public IInstalledAppLaunchHandle Handle { get; } = handle;
        public DateTime CreatedUtc { get; } = createdUtc;
        public DateTime ExpiresUtc { get; } = expiresUtc;
        public object SyncRoot { get; } = new();
        public string? ContinuationToken { get; set; }
    }

    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };
    private readonly IInstalledAppInventory _inventory;
    private readonly string _backupDirectory;
    private readonly IInstalledAppLauncher _launcher;
    private readonly TimeSpan _uninstallSettleWindow;
    private readonly ConcurrentDictionary<string, LeftoverAuthorization> _cleanupAuthorizations = new(StringComparer.Ordinal);
    private readonly ConcurrentDictionary<string, TrackedUninstall> _uninstallSessions = new(StringComparer.Ordinal);
    private readonly AppLeftoverManager _leftovers;

    public InstalledAppManager(IInstalledAppInventory inventory, string? backupDirectory = null,
        IStorageDeleter? storageDeleter = null, IInstalledAppLauncher? launcher = null,
        TimeSpan? uninstallSettleWindow = null)
    {
        _inventory = inventory;
        _backupDirectory = backupDirectory ?? ProductPaths.BackupDirectory;
        _launcher = launcher ?? new InstalledAppLauncher();
        _uninstallSettleWindow = uninstallSettleWindow ?? TimeSpan.FromSeconds(10);
        _leftovers = new AppLeftoverManager(inventory, storageDeleter ?? new StorageDeleter(), _cleanupAuthorizations);
    }

    public Task<InstalledAppActionResult> UninstallAsync(InstalledApp app, bool preview, CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var current = _inventory.FindExact(app.RegistryLocation);
        if (current is null)
            return Task.FromResult(new InstalledAppActionResult(preview, false, true, "The registered app entry no longer exists."));
        if (!string.Equals(current.DisplayName, app.DisplayName, StringComparison.Ordinal) ||
            !string.Equals(current.UninstallString, app.UninstallString, StringComparison.Ordinal))
            return Task.FromResult(new InstalledAppActionResult(preview, false, true, "The registered app entry changed. Refresh before continuing."));
        if (!current.CanUninstall)
            return Task.FromResult(new InstalledAppActionResult(preview, false, true, current.PolicyReason));
        if (!InstalledAppPolicy.TryParseUninstallCommand(current.UninstallString, out var plan, out var reason) || plan is null)
            return Task.FromResult(new InstalledAppActionResult(preview, false, true, reason));

        if (preview)
            return Task.FromResult(new InstalledAppActionResult(true, false, false,
                $"Preflight passed: Windows can open the registered interactive uninstaller for {current.DisplayName}. No changes were made."));

        cancellationToken.ThrowIfCancellationRequested();
        var handle = _launcher.Launch(plan);
        if (handle is null)
            return Task.FromResult(
                new InstalledAppActionResult(false, false, true, "Windows did not start the registered uninstaller."));
        var sessionId = TrackUninstall(current, handle);
        return Task.FromResult(
            new InstalledAppActionResult(false, true, false,
                $"Opened the registered uninstaller for {current.DisplayName}. Sift is waiting for it to close and will verify that the exact registration was removed.")
            {
                UninstallSessionId = sessionId,
                ProcessId = handle.ProcessId
            });
    }

    public async Task<InstalledAppUninstallCompletion> WaitForUninstallCompletionAsync(InstalledApp app, string sessionId,
        CancellationToken cancellationToken = default)
    {
        if (!TryGetUninstallSession(app, sessionId, out var session, out var blocked)) return blocked;
        await session!.Handle.WaitForExitAsync(cancellationToken);
        var deadline = DateTime.UtcNow + _uninstallSettleWindow;
        while (true)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var completion = VerifyUninstallCompletion(app, session);
            if (completion.Completed || completion.Blocked || DateTime.UtcNow >= deadline) return completion;
            var remaining = deadline - DateTime.UtcNow;
            await Task.Delay(remaining < TimeSpan.FromMilliseconds(500) ? remaining : TimeSpan.FromMilliseconds(500),
                cancellationToken);
        }
    }

    public Task<InstalledAppUninstallCompletion> CheckUninstallCompletionAsync(InstalledApp app, string sessionId,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        return Task.FromResult(TryGetUninstallSession(app, sessionId, out var session, out var blocked)
            ? VerifyUninstallCompletion(app, session!)
            : blocked);
    }

    public Task<InstalledAppActionResult> CleanupRegistrationAsync(InstalledApp app, bool preview,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var current = _inventory.FindExact(app.RegistryLocation);
        if (current is null)
            return Task.FromResult(new InstalledAppActionResult(preview, false, true, "The registered app entry no longer exists."));
        if (!MatchesExactRegistration(current, app))
            return Task.FromResult(new InstalledAppActionResult(preview, false, true, "The registered app entry changed. Refresh before continuing."));
        if (!current.IsOrphanedRegistration)
            return Task.FromResult(new InstalledAppActionResult(preview, false, true,
                "The entry no longer meets the two-signal leftover policy. Nothing was removed."));
        if (!current.CanCleanRegistration)
            return Task.FromResult(new InstalledAppActionResult(preview, false, true,
                "This machine-wide registration requires an administrator session."));
        if (preview)
            return Task.FromResult(new InstalledAppActionResult(true, false, false,
                $"Preflight passed: Sift can back up and remove the leftover registration for {current.DisplayName}. No changes were made."));

        cancellationToken.ThrowIfCancellationRequested();
        var registrationRemoved = false;
        string? writtenBackupPath = null;
        try
        {
            var physical = ResolvePhysicalLocation(current.RegistryLocation);
            using var existing = physical.Hive.OpenSubKey(physical.SubKeyName, writable: false);
            if (existing is null)
                return Task.FromResult(new InstalledAppActionResult(false, false, true, "The registry entry disappeared before backup."));
            if (!OpenedEntryIsSameOrphan(existing, current))
                return Task.FromResult(new InstalledAppActionResult(false, false, true,
                    "The app registration changed. Nothing was removed."));
            var tree = RegistrySnapshotCodec.CaptureTree(existing);
            var backup = new Backup
            {
                Entries =
                [
                    new BackupEntry
                    {
                        TweakId = $"installed-app.orphan.{Sanitize(current.DisplayName)}",
                        State = BackupEntryStates.Applying,
                        Existed = true,
                        KeyExisted = true,
                        RegistryHive = current.RegistryLocation.Hive,
                        RegistrySubKey = physical.SubKeyName,
                        RegistryTree = tree
                    }
                ]
            };
            Directory.CreateDirectory(_backupDirectory);
            var backupPath = Path.Combine(_backupDirectory,
                $"backup-registry-{DateTime.UtcNow:yyyyMMdd-HHmmss-fff}-{backup.OperationId[..8]}-orphan-{Sanitize(current.DisplayName)}.json");
            WriteBackup(backupPath, backup);
            writtenBackupPath = backupPath;

            var slash = physical.SubKeyName.LastIndexOf('\\');
            if (slash <= 0) throw new InvalidOperationException("The registry identity is not a child entry.");
            using var parent = physical.Hive.OpenSubKey(physical.SubKeyName[..slash], writable: true)
                ?? throw new InvalidOperationException("The uninstall registry parent is unavailable.");
            cancellationToken.ThrowIfCancellationRequested();
            if (!OpenedEntryIsSameOrphan(existing, current))
                return Task.FromResult(new InstalledAppActionResult(false, false, true,
                    "The registry entry changed after backup. The backup was retained; nothing was removed."));
            parent.DeleteSubKeyTree(physical.SubKeyName[(slash + 1)..], throwOnMissingSubKey: false);
            registrationRemoved = true;

            var entry = backup.Entries[0];
            entry.AppliedSuccessfully = true;
            entry.State = BackupEntryStates.Applied;
            entry.AppliedUtc = DateTime.UtcNow;
            WriteBackup(backupPath, backup);
            return Task.FromResult(new InstalledAppActionResult(false, true, false,
                $"Removed the leftover registration for {current.DisplayName}. No files were deleted. Backup: {Path.GetFileName(backupPath)}")
            {
                ContinuationToken = AuthorizeLeftoverContinuation(current)
            });
        }
        catch (OperationCanceledException)
        {
            throw;
        }
        catch (Exception exception)
        {
            if (registrationRemoved)
                return Task.FromResult(new InstalledAppActionResult(false, true, false,
                    $"Removed the leftover registration for {current.DisplayName}, but could not finalize backup status: {exception.Message}. Recovery backup: {Path.GetFileName(writtenBackupPath)}")
                {
                    ContinuationToken = AuthorizeLeftoverContinuation(current)
                });
            return Task.FromResult(new InstalledAppActionResult(false, false, true,
                $"Could not remove the leftover registration: {exception.Message}"));
        }
    }

    public AppLeftoverScanResult ScanLeftovers(InstalledApp app, string? continuationToken,
        CancellationToken cancellationToken = default) =>
        _leftovers.ScanLeftovers(app, continuationToken, cancellationToken);

    public AppLeftoverDeleteResult DeleteLeftovers(InstalledApp app, string? continuationToken,
        IEnumerable<AppLeftoverCandidate> selection, bool preview, CancellationToken cancellationToken = default) =>
        _leftovers.DeleteLeftovers(app, continuationToken, selection, preview, cancellationToken);

    private string AuthorizeLeftoverContinuation(InstalledApp app)
    {
        while (_cleanupAuthorizations.Count >= 8)
        {
            var oldest = _cleanupAuthorizations.Keys.FirstOrDefault();
            if (oldest is null || !_cleanupAuthorizations.TryRemove(oldest, out _)) break;
        }
        var token = Guid.NewGuid().ToString("N");
        _cleanupAuthorizations[token] = new LeftoverAuthorization(
            AppLeftoverManager.Fingerprint(app), DateTime.UtcNow.AddMinutes(30));
        return token;
    }

    private string TrackUninstall(InstalledApp app, IInstalledAppLaunchHandle handle)
    {
        PruneUninstallSessions();
        while (_uninstallSessions.Count >= 8)
        {
            var oldest = _uninstallSessions.OrderBy(pair => pair.Value.CreatedUtc).FirstOrDefault();
            if (string.IsNullOrWhiteSpace(oldest.Key) || !_uninstallSessions.TryRemove(oldest.Key, out var removed)) break;
            removed.Handle.Dispose();
        }

        var sessionId = Guid.NewGuid().ToString("N");
        _uninstallSessions[sessionId] = new TrackedUninstall(app, handle, DateTime.UtcNow, DateTime.UtcNow.AddHours(2));
        return sessionId;
    }

    private bool TryGetUninstallSession(InstalledApp app, string sessionId, out TrackedUninstall? session,
        out InstalledAppUninstallCompletion blocked)
    {
        PruneUninstallSessions();
        session = null;
        if (string.IsNullOrWhiteSpace(sessionId) || !_uninstallSessions.TryGetValue(sessionId, out var found))
        {
            blocked = new InstalledAppUninstallCompletion(false, true,
                "The uninstall tracking session is unavailable or expired. Start the uninstall again before scanning leftovers.");
            return false;
        }
        if (!string.Equals(AppLeftoverManager.Fingerprint(found.App), AppLeftoverManager.Fingerprint(app), StringComparison.Ordinal))
        {
            blocked = new InstalledAppUninstallCompletion(false, true,
                "The uninstall tracking session belongs to a different app. Nothing was authorized.");
            return false;
        }
        session = found;
        blocked = new InstalledAppUninstallCompletion(false, false, string.Empty);
        return true;
    }

    private InstalledAppUninstallCompletion VerifyUninstallCompletion(InstalledApp app, TrackedUninstall session)
    {
        var current = _inventory.FindExact(app.RegistryLocation);
        if (current is not null)
        {
            if (!MatchesExactRegistration(current, app))
                return new InstalledAppUninstallCompletion(false, true,
                    "The uninstall registry identity was replaced or changed. Refresh before taking another action.");
            return new InstalledAppUninstallCompletion(false, false,
                $"{app.DisplayName} is still registered. The uninstaller may still be open or may have been cancelled; leftover cleanup remains locked.");
        }

        lock (session.SyncRoot)
        {
            session.ContinuationToken ??= AuthorizeLeftoverContinuation(app);
            return new InstalledAppUninstallCompletion(true, false,
                $"Confirmed that the exact registration for {app.DisplayName} was removed. Exact AppData leftover review is now available.")
            {
                ContinuationToken = session.ContinuationToken
            };
        }
    }

    private void PruneUninstallSessions()
    {
        var now = DateTime.UtcNow;
        foreach (var pair in _uninstallSessions.Where(pair => pair.Value.ExpiresUtc <= now).ToList())
        {
            if (_uninstallSessions.TryRemove(pair.Key, out var removed)) removed.Handle.Dispose();
        }
    }

    private static bool MatchesExactRegistration(InstalledApp current, InstalledApp requested) =>
        string.Equals(current.DisplayName, requested.DisplayName, StringComparison.Ordinal) &&
        string.Equals(current.UninstallString, requested.UninstallString, StringComparison.Ordinal) &&
        string.Equals(current.InstallLocation, requested.InstallLocation, StringComparison.Ordinal);

    private static bool OpenedEntryIsSameOrphan(RegistryKey key, InstalledApp expected)
    {
        var displayName = key.GetValue("DisplayName")?.ToString()?.Trim() ?? string.Empty;
        var publisher = key.GetValue("Publisher")?.ToString()?.Trim() ?? string.Empty;
        var version = key.GetValue("DisplayVersion")?.ToString()?.Trim() ?? string.Empty;
        var installLocation = key.GetValue("InstallLocation")?.ToString()?.Trim() ?? string.Empty;
        var uninstall = key.GetValue("UninstallString")?.ToString()?.Trim() ?? string.Empty;
        if (!string.Equals(displayName, expected.DisplayName, StringComparison.Ordinal) ||
            !string.Equals(installLocation, expected.InstallLocation, StringComparison.Ordinal) ||
            !string.Equals(uninstall, expected.UninstallString, StringComparison.Ordinal)) return false;
        var values = new InstalledAppRegistryValues(
            displayName, publisher, version, installLocation, string.Empty, 0, uninstall,
            ReadRegistryFlag(key, "WindowsInstaller"),
            ReadRegistryFlag(key, "SystemComponent"),
            key.GetValue("ReleaseType")?.ToString()?.Trim() ?? string.Empty,
            key.GetValue("ParentKeyName")?.ToString()?.Trim() ?? string.Empty);
        return InstalledAppPolicy.IsConservativeOrphan(values, out _);
    }

    private static bool ReadRegistryFlag(RegistryKey key, string name) => key.GetValue(name) switch
    {
        int value => value == 1,
        long value => value == 1,
        string value => value == "1",
        _ => false
    };

    private static (RegistryKey Hive, string SubKeyName) ResolvePhysicalLocation(InstalledAppRegistryLocation location)
    {
        var logicalPrefix = @"Software\Microsoft\Windows\CurrentVersion\Uninstall\";
        if (!location.SubKeyName.StartsWith(logicalPrefix, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("The registry identity is outside the uninstall roots.");
        var child = location.SubKeyName[logicalPrefix.Length..];
        if (string.IsNullOrWhiteSpace(child) || child.Contains('\\'))
            throw new InvalidOperationException("The registry identity is not an exact uninstall child.");
        if (location.Hive == "HKCU")
            return (Registry.CurrentUser, logicalPrefix + child);
        if (location.Hive == "HKLM" && location.View == "32-bit")
            return (Registry.LocalMachine, @"SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\" + child);
        if (location.Hive == "HKLM" && location.View == "64-bit")
            return (Registry.LocalMachine, @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\" + child);
        throw new InvalidOperationException("The registry hive or view is unsupported.");
    }

    private static void WriteBackup(string path, Backup backup) =>
        AtomicFile.WriteAllText(path, JsonSerializer.Serialize(backup, JsonOptions));

    private static string Sanitize(string value)
    {
        var chars = value.Where(char.IsLetterOrDigit).Take(20).ToArray();
        return chars.Length == 0 ? "item" : new string(chars);
    }
}
