using System.Collections.Concurrent;
using System.IO;
using System.Security.Cryptography;
using System.Security.Principal;
using System.Text;
using System.Text.Json;
using Sift.Infrastructure.Persistence;
using Sift.Models;
using Microsoft.Win32;

namespace Sift.Services;

public sealed class MaintenanceCleaner : IMaintenanceCleaner
{
    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };
    private readonly string _backupDirectory;
    private readonly Func<bool> _isElevated;
    private readonly ConcurrentDictionary<string, CleanupTicket> _tickets = new(StringComparer.Ordinal);

    public MaintenanceCleaner(string? backupDirectory = null, Func<bool>? isElevated = null)
    {
        _backupDirectory = backupDirectory ?? ProductPaths.BackupDirectory;
        _isElevated = isElevated ?? ElevationHelper.IsElevated;
    }

    public Task<MaintenanceCleanupReview> ReviewAsync(IEnumerable<MaintenanceFinding> selection,
        CancellationToken cancellationToken = default)
    {
        foreach (var expired in _tickets.Where(pair => pair.Value.ExpiresUtc <= DateTimeOffset.UtcNow).Select(pair => pair.Key))
            _tickets.TryRemove(expired, out _);
        var selected = selection.Where(item => item.CanClean).ToList();
        var reviewed = new List<ReviewedFinding>();
        var log = new List<string>();
        var skipped = 0;

        foreach (var item in selected)
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (!MaintenancePolicy.IsAllowed(item, out var policyReason))
            {
                skipped++;
                log.Add($"BLOCKED  {item.Title} · {policyReason}");
                continue;
            }
            if (RequiresAdministrator(item) && !_isElevated())
            {
                skipped++;
                log.Add($"BLOCKED  {item.Title} · administrator permission is required");
                continue;
            }
            if (item.Category == MaintenanceCategory.OrphanUninstall &&
                !ValidateCurrentOrphan(item, out var orphanReason))
            {
                skipped++;
                log.Add($"BLOCKED  {item.Title} · {orphanReason}");
                continue;
            }

            try
            {
                reviewed.Add(new ReviewedFinding(item, CaptureContentIdentity(item)));
                log.Add($"REVIEWED  {item.Title} · {item.SizeLabel} · {item.Path}");
            }
            catch (Exception exception)
            {
                skipped++;
                log.Add($"BLOCKED  {item.Title} · could not inspect current contents: {exception.Message}");
            }
        }

        string? ticketId = null;
        var status = MaintenanceCleanupStatus.Rejected;
        if (selected.Count > 0 && reviewed.Count == selected.Count)
        {
            ticketId = Guid.NewGuid().ToString("N");
            _tickets[ticketId] = new CleanupTicket(reviewed, DateTimeOffset.UtcNow.AddMinutes(10));
            status = MaintenanceCleanupStatus.Reviewed;
        }

        var result = new CleanResult
        {
            Log = log,
            Previewed = reviewed.Count,
            Skipped = skipped,
            Status = status
        };
        return Task.FromResult(new MaintenanceCleanupReview(ticketId, result));
    }

    public async Task<CleanResult> ExecuteAsync(string ticketId, CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrWhiteSpace(ticketId) || !_tickets.TryRemove(ticketId, out var ticket))
            return Invalidated("The reviewed cleanup is no longer available. Review the selection again.");
        if (ticket.ExpiresUtc <= DateTimeOffset.UtcNow)
            return Invalidated("The reviewed cleanup expired. Review the selection again.");

        foreach (var reviewed in ticket.Findings)
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (!MaintenancePolicy.IsAllowed(reviewed.Finding, out var policyReason))
                return Invalidated($"{reviewed.Finding.Title} is no longer available: {policyReason}");
            if (RequiresAdministrator(reviewed.Finding) && !_isElevated())
                return Invalidated($"{reviewed.Finding.Title} requires administrator permission.");
            if (reviewed.Finding.Category == MaintenanceCategory.OrphanUninstall &&
                !ValidateCurrentOrphan(reviewed.Finding, out var orphanReason))
                return Invalidated($"{reviewed.Finding.Title} changed after review: {orphanReason}");

            string currentIdentity;
            try { currentIdentity = CaptureContentIdentity(reviewed.Finding); }
            catch (Exception exception)
            {
                return Invalidated($"Could not check {reviewed.Finding.Title} again: {exception.Message}");
            }
            if (!CryptographicOperations.FixedTimeEquals(
                    Convert.FromHexString(reviewed.ContentIdentity), Convert.FromHexString(currentIdentity)))
                return Invalidated($"{reviewed.Finding.Title} changed after review. Nothing was cleaned.");
        }

        return await CleanReviewedAsync(ticket.Findings.Select(item => item.Finding), cancellationToken);
    }

    public void Discard(string ticketId)
    {
        if (!string.IsNullOrWhiteSpace(ticketId)) _tickets.TryRemove(ticketId, out _);
    }

    private async Task<CleanResult> CleanReviewedAsync(IEnumerable<MaintenanceFinding> selection,
        CancellationToken cancellationToken)
    {
        var log = new List<string>();
        var cleaned = 0;
        var skipped = 0;
        var failed = 0;

        foreach (var item in selection.Where(x => x.CanClean))
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (!MaintenancePolicy.IsAllowed(item, out var policyReason))
            {
                skipped++;
                log.Add($"BLOCKED  {item.Title} · {policyReason}");
                continue;
            }

            if (item.Category == MaintenanceCategory.OrphanUninstall && !ValidateCurrentOrphan(item, out var orphanReason))
            {
                skipped++;
                log.Add($"BLOCKED  {item.Title} · {orphanReason}");
                continue;
            }

            try
            {
                if (item.Category == MaintenanceCategory.OrphanUninstall)
                {
                    if (DeleteOrphanUninstall(item, log)) cleaned++;
                    else skipped++;
                    continue;
                }

                if (item.Id == "maintenance.recycle")
                {
                    await RunPowerShellAsync("Clear-RecycleBin -Force -ErrorAction Stop", cancellationToken);
                    cleaned++;
                    log.Add($"CLEANED  Recycle Bin · {item.SizeLabel}");
                    continue;
                }

                if (item.Category is MaintenanceCategory.ThumbnailCache or MaintenanceCategory.CrashDumps or MaintenanceCategory.Prefetch)
                {
                    var (ok, skip, fail) = CleanMatchingFiles(item);
                    cleaned += ok;
                    skipped += skip;
                    failed += fail;
                    if (ok > 0) log.Add($"CLEANED  {item.Title} · {ok} file(s)");
                    if (skip > 0) log.Add($"SKIPPED  {item.Title} · {skip} locked/missing");
                    if (fail > 0) log.Add($"FAILED   {item.Title} · {fail} error(s)");
                    continue;
                }

                if (Directory.Exists(item.Path))
                {
                    var (ok, skip, fail) = CleanDirectoryContents(item.Path);
                    if (ok > 0 && skip == 0 && fail == 0)
                    {
                        cleaned++;
                        log.Add($"CLEANED  {item.Title} · {item.SizeLabel}");
                    }
                    else if (ok > 0)
                    {
                        cleaned++;
                        skipped += skip;
                        failed += fail;
                        log.Add($"CLEANED  {item.Title} · partial ({ok} removed, {skip} skipped, {fail} failed)");
                    }
                    else if (skip > 0 || fail > 0)
                    {
                        skipped += skip;
                        failed += fail;
                        log.Add($"SKIPPED  {item.Title} · nothing removed ({skip} locked, {fail} failed)");
                    }
                    else
                    {
                        skipped++;
                        log.Add($"SKIPPED  {item.Title} (empty)");
                    }
                }
                else
                {
                    skipped++;
                    log.Add($"SKIPPED  {item.Title} (path not found)");
                }
            }
            catch (Exception ex)
            {
                failed++;
                log.Add($"FAILED   {item.Title}: {ex.Message}");
            }
        }

        return new CleanResult
        {
            Log = log,
            Cleaned = cleaned,
            Skipped = skipped,
            Failed = failed,
            Status = MaintenanceCleanupStatus.Completed
        };
    }

    private static bool RequiresAdministrator(MaintenanceFinding item) =>
        item.RequiresElevation ||
        string.Equals(item.RegistryHive, "HKLM", StringComparison.OrdinalIgnoreCase);

    private static CleanResult Invalidated(string reason) => new()
    {
        Status = MaintenanceCleanupStatus.Invalidated,
        Skipped = 1,
        Log = [$"CHANGED  {reason}"]
    };

    private bool DeleteOrphanUninstall(MaintenanceFinding item, List<string> log)
    {
        if (string.IsNullOrWhiteSpace(item.RegistryHive) || string.IsNullOrWhiteSpace(item.RegistrySubKey))
        {
            log.Add($"SKIPPED  {item.Title} (missing registry path)");
            return false;
        }

        try
        {
            var hive = item.RegistryHive == "HKCU" ? Registry.CurrentUser : Registry.LocalMachine;
            var lastSlash = item.RegistrySubKey.LastIndexOf('\\');
            if (lastSlash < 0)
            {
                log.Add($"SKIPPED  {item.Title} (invalid key)");
                return false;
            }
            var parent = item.RegistrySubKey[..lastSlash];
            var name = item.RegistrySubKey[(lastSlash + 1)..];
            RegistryKeySnapshot tree;
            using (var existing = hive.OpenSubKey(item.RegistrySubKey))
            {
                if (existing is null)
                {
                    log.Add($"SKIPPED  {item.Title} (key missing)");
                    return false;
                }
                if (!ValidateCurrentOrphan(item, existing, out var orphanReason))
                {
                    log.Add($"BLOCKED  {item.Title} · {orphanReason}");
                    return false;
                }
                tree = RegistrySnapshotCodec.CaptureTree(existing);
            }
            var (backup, entry, backupPath) = PrepareRegistryBackup(item, tree);

            using var key = hive.OpenSubKey(parent, writable: true);
            if (key is null)
            {
                log.Add($"SKIPPED  {item.Title} (parent key missing)");
                return false;
            }
            using (var current = hive.OpenSubKey(item.RegistrySubKey, writable: false))
            {
                if (current is null)
                {
                    log.Add($"BLOCKED  {item.Title} · the registry entry disappeared after backup");
                    return false;
                }
                if (!ValidateCurrentOrphan(item, current, out var changedReason))
                {
                    log.Add($"BLOCKED  {item.Title} · {changedReason}");
                    return false;
                }
            }
            key.DeleteSubKeyTree(name, throwOnMissingSubKey: false);
            entry.AppliedSuccessfully = true;
            entry.State = BackupEntryStates.Applied;
            entry.AppliedUtc = DateTime.UtcNow;
            WriteRegistryBackup(backupPath, backup);
            log.Add($"CLEANED  {item.Title} · registry key removed · backup {Path.GetFileName(backupPath)}");
            return true;
        }
        catch (Exception ex)
        {
            log.Add($"FAILED   {item.Title}: {ex.Message}");
            return false;
        }
    }

    private static bool ValidateCurrentOrphan(MaintenanceFinding item, out string reason)
    {
        reason = string.Empty;
        if (string.IsNullOrWhiteSpace(item.RegistryHive) || string.IsNullOrWhiteSpace(item.RegistrySubKey))
        {
            reason = "the exact registry identity is missing";
            return false;
        }
        try
        {
            var hive = item.RegistryHive == "HKCU" ? Registry.CurrentUser : Registry.LocalMachine;
            using var key = hive.OpenSubKey(item.RegistrySubKey, writable: false);
            if (key is null)
            {
                reason = "the registry entry no longer exists";
                return false;
            }
            return ValidateCurrentOrphan(item, key, out reason);
        }
        catch (Exception exception)
        {
            reason = exception.Message;
            return false;
        }
    }

    private static bool ValidateCurrentOrphan(MaintenanceFinding item, RegistryKey key, out string reason)
    {
        reason = string.Empty;
        if (item.RegistryValues is null)
        {
            reason = "scan-time registry evidence is missing";
            return false;
        }

        string Read(string name) => key.GetValue(name)?.ToString()?.Trim() ?? string.Empty;
        foreach (var name in new[] { "DisplayName", "InstallLocation", "UninstallString" })
        {
            if (!item.RegistryValues.TryGetValue(name, out var expected) ||
                !string.Equals(Read(name), expected?.Trim() ?? string.Empty, StringComparison.Ordinal))
            {
                reason = $"the {name} value changed after scanning";
                return false;
            }
        }

        bool Flag(string name) => key.GetValue(name) switch
        {
            int value => value == 1,
            long value => value == 1,
            string value => value == "1",
            _ => false
        };
        var values = new InstalledAppRegistryValues(
            Read("DisplayName"), Read("Publisher"), Read("DisplayVersion"), Read("InstallLocation"), Read("InstallDate"), 0,
            Read("UninstallString"), Flag("WindowsInstaller"), Flag("SystemComponent"),
            Read("ReleaseType"), Read("ParentKeyName"));
        if (InstalledAppPolicy.IsConservativeOrphan(values, out _)) return true;
        reason = "the entry no longer meets the two-signal leftover policy";
        return false;
    }

    private (Backup Backup, BackupEntry Entry, string Path) PrepareRegistryBackup(
        MaintenanceFinding item,
        RegistryKeySnapshot tree)
    {
        var dir = _backupDirectory;
        Directory.CreateDirectory(dir);
        var entry = new BackupEntry
        {
            TweakId = item.Id,
            State = BackupEntryStates.Applying,
            Existed = true,
            KeyExisted = true,
            RegistryHive = item.RegistryHive,
            RegistrySubKey = item.RegistrySubKey,
            RegistryTree = tree
        };
        var backup = new Backup
        {
            Entries = [entry]
        };
        var path = Path.Combine(dir,
            $"backup-registry-{DateTime.UtcNow:yyyyMMdd-HHmmss-fff}-{backup.OperationId[..8]}-{Sanitize(item.Id)}.json");
        WriteRegistryBackup(path, backup);
        return (backup, entry, path);
    }

    private static void WriteRegistryBackup(string path, Backup backup) =>
        AtomicFile.WriteAllText(path, JsonSerializer.Serialize(backup, JsonOptions));

    private static (int ok, int skip, int fail) CleanMatchingFiles(MaintenanceFinding item)
    {
        var ok = 0; var skip = 0; var fail = 0;
        var dir = Path.GetDirectoryName(item.Path);
        var pattern = Path.GetFileName(item.Path);
        if (string.IsNullOrWhiteSpace(dir) || !Directory.Exists(dir)) return (0, 1, 0);
        if (pattern.Contains('*'))
        {
            foreach (var file in Directory.EnumerateFiles(dir, pattern))
            {
                try { File.Delete(file); ok++; }
                catch (IOException) { skip++; }
                catch (UnauthorizedAccessException) { skip++; }
                catch { fail++; }
            }
        }
        else if (File.Exists(item.Path))
        {
            try { File.Delete(item.Path); ok++; }
            catch (IOException) { skip++; }
            catch (UnauthorizedAccessException) { skip++; }
            catch { fail++; }
        }
        else if (Directory.Exists(item.Path))
            return CleanDirectoryContents(item.Path);
        else skip++;
        return (ok, skip, fail);
    }

    private static (int ok, int skip, int fail) CleanDirectoryContents(string path)
    {
        var ok = 0; var skip = 0; var fail = 0;
        foreach (var entry in Directory.EnumerateFileSystemEntries(path))
        {
            try
            {
                if (Directory.Exists(entry)) Directory.Delete(entry, true);
                else File.Delete(entry);
                ok++;
            }
            catch (IOException) { skip++; }
            catch (UnauthorizedAccessException) { skip++; }
            catch { fail++; }
        }
        return (ok, skip, fail);
    }

    private static string Sanitize(string value)
    {
        var chars = value.Where(char.IsLetterOrDigit).Take(20).ToArray();
        return chars.Length == 0 ? "item" : new string(chars);
    }

    private static string CaptureContentIdentity(MaintenanceFinding item)
    {
        if (item.Category == MaintenanceCategory.OrphanUninstall)
        {
            var hive = string.Equals(item.RegistryHive, "HKCU", StringComparison.OrdinalIgnoreCase)
                ? Registry.CurrentUser
                : Registry.LocalMachine;
            using var key = hive.OpenSubKey(item.RegistrySubKey!, writable: false)
                ?? throw new InvalidOperationException("The registry entry no longer exists.");
            var lines = new List<string>();
            AppendRegistrySnapshot(lines, "", RegistrySnapshotCodec.CaptureTree(key));
            return Hash(lines);
        }

        if (item.Category == MaintenanceCategory.RecycleBin)
        {
            var sid = WindowsIdentity.GetCurrent().User?.Value
                ?? throw new InvalidOperationException("The current Windows account could not be identified.");
            var roots = DriveInfo.GetDrives()
                .Where(drive => drive.IsReady && drive.DriveType == DriveType.Fixed)
                .Select(drive => Path.Combine(drive.RootDirectory.FullName, "$Recycle.Bin", sid))
                .Where(Directory.Exists)
                .ToList();
            return Hash(CaptureFileSystemEntries(roots, recursive: true));
        }

        var directory = Path.GetDirectoryName(item.Path);
        var pattern = Path.GetFileName(item.Path);
        if (!string.IsNullOrWhiteSpace(directory) && pattern.Contains('*'))
        {
            if (!Directory.Exists(directory)) return Hash(["missing"]);
            return Hash(CaptureFileSystemEntries(
                Directory.EnumerateFiles(directory, pattern, SearchOption.TopDirectoryOnly), recursive: false));
        }

        if (Directory.Exists(item.Path))
            return Hash(CaptureFileSystemEntries([item.Path], recursive: true));
        if (File.Exists(item.Path))
            return Hash(CaptureFileSystemEntries([item.Path], recursive: false));
        return Hash(["missing"]);
    }

    private static IReadOnlyList<string> CaptureFileSystemEntries(IEnumerable<string> roots, bool recursive)
    {
        const int maxEntries = 100_000;
        var lines = new List<string>();
        foreach (var root in roots.Select(Path.GetFullPath).OrderBy(path => path, StringComparer.OrdinalIgnoreCase))
        {
            if (lines.Count >= maxEntries)
                throw new InvalidOperationException($"The selected contents exceed {maxEntries:N0} entries.");
            if (File.Exists(root))
            {
                AppendFile(lines, root);
                continue;
            }
            if (!Directory.Exists(root)) continue;

            var rootInfo = new DirectoryInfo(root);
            if (rootInfo.Attributes.HasFlag(FileAttributes.ReparsePoint))
                throw new InvalidOperationException("The selected location is a reparse point.");
            lines.Add($"D|{rootInfo.FullName}|{rootInfo.LastWriteTimeUtc.Ticks}|{(int)rootInfo.Attributes}");
            var queue = new Queue<DirectoryInfo>();
            queue.Enqueue(rootInfo);
            while (queue.Count > 0)
            {
                var directory = queue.Dequeue();
                foreach (var entry in directory.EnumerateFileSystemInfos()
                             .OrderBy(entry => entry.FullName, StringComparer.OrdinalIgnoreCase))
                {
                    if (lines.Count >= maxEntries)
                        throw new InvalidOperationException($"The selected contents exceed {maxEntries:N0} entries.");
                    if (entry.Attributes.HasFlag(FileAttributes.ReparsePoint))
                        throw new InvalidOperationException($"The selected contents include a reparse point: {entry.FullName}");
                    if (entry is DirectoryInfo child)
                    {
                        lines.Add($"D|{child.FullName}|{child.LastWriteTimeUtc.Ticks}|{(int)child.Attributes}");
                        if (recursive) queue.Enqueue(child);
                    }
                    else if (entry is FileInfo file)
                        AppendFile(lines, file);
                }
                if (!recursive) queue.Clear();
            }
        }
        return lines;
    }

    private static void AppendFile(List<string> lines, string path)
    {
        AppendFile(lines, new FileInfo(path));
    }

    private static void AppendFile(List<string> lines, FileInfo info) =>
        lines.Add($"F|{info.FullName}|{info.Length}|{info.LastWriteTimeUtc.Ticks}|{(int)info.Attributes}");

    private static void AppendRegistrySnapshot(List<string> lines, string prefix, RegistryKeySnapshot snapshot)
    {
        foreach (var value in snapshot.Values.OrderBy(value => value.Name, StringComparer.OrdinalIgnoreCase))
            lines.Add($"V|{prefix}|{value.Name}|{value.Kind}|{value.Encoding}|{value.Data}");
        foreach (var child in snapshot.SubKeys.OrderBy(child => child.Key, StringComparer.OrdinalIgnoreCase))
            AppendRegistrySnapshot(lines, $"{prefix}\\{child.Key}", child.Value);
    }

    private static string Hash(IEnumerable<string> lines)
    {
        var canonical = string.Join('\n', lines);
        return Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(canonical)));
    }

    private static Task RunPowerShellAsync(string command, CancellationToken cancellationToken) =>
        TweakExecutor.RunProcessForSuccessAsync(
            TweakExecutor.CreatePowerShellStartInfo(command),
            TimeSpan.FromMinutes(2),
            cancellationToken);

    private sealed record ReviewedFinding(MaintenanceFinding Finding, string ContentIdentity);
    private sealed record CleanupTicket(IReadOnlyList<ReviewedFinding> Findings, DateTimeOffset ExpiresUtc);
}
