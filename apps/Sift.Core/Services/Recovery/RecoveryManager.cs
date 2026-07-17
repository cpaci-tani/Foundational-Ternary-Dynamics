using System.Text.Json;
using Sift.Models;

namespace Sift.Services;

public interface IRecoveryManager
{
    IReadOnlyList<RecoveryBackupInfo> ListBackups();
    RecoveryBackupInfo InspectExact(string path);
    Task<RecoveryRestoreResult> RestoreAsync(string path, CancellationToken cancellationToken = default);
}

public sealed class RecoveryManager(ITweakExecutor executor, IElevationBroker elevation,
    Func<bool>? elevationProbe = null) : IRecoveryManager
{
    private const long MaximumBackupBytes = 4 * 1024 * 1024;
    private static readonly JsonSerializerOptions JsonOptions = new() { MaxDepth = 48 };
    private readonly IReadOnlyDictionary<string, Tweak> _catalog = TweakCatalog.Create()
        .ToDictionary(tweak => tweak.Id, StringComparer.OrdinalIgnoreCase);
    private bool IsElevated => (elevationProbe ?? ElevationHelper.IsElevated)();

    public IReadOnlyList<RecoveryBackupInfo> ListBackups()
    {
        if (!Directory.Exists(executor.BackupDirectory)) return [];
        if (File.GetAttributes(executor.BackupDirectory).HasFlag(FileAttributes.ReparsePoint)) return [];
        var results = new List<RecoveryBackupInfo>();
        foreach (var path in Directory.GetFiles(executor.BackupDirectory, "backup-*.json", SearchOption.TopDirectoryOnly)
                     .OrderDescending().Take(300))
        {
            try { results.Add(InspectExact(path)); }
            catch (Exception exception)
            {
                results.Add(new RecoveryBackupInfo(path, Path.GetFileName(path), File.GetCreationTimeUtc(path),
                    "Unreadable backup", string.Empty, 0, 0, 0, 1, false, false, false,
                    "Blocked", exception.Message));
            }
        }
        return results.OrderByDescending(item => item.CreatedUtc).ToList();
    }

    public RecoveryBackupInfo InspectExact(string path)
    {
        path = ValidatePath(path);
        var info = new FileInfo(path);
        if (!info.Exists) throw new InvalidOperationException("The selected recovery backup no longer exists.");
        if (info.Length is <= 0 or > MaximumBackupBytes)
            throw new InvalidDataException("The selected recovery backup is empty or exceeds the 4 MB limit.");
        var backup = JsonSerializer.Deserialize<Backup>(File.ReadAllText(path), JsonOptions)
            ?? throw new InvalidDataException("The selected recovery backup is malformed.");
        if (backup.SchemaVersion is < 1 or > 2 || backup.Entries.Count is 0 or > 256)
            throw new InvalidDataException("The selected recovery backup has an unsupported schema or entry count.");

        var pending = backup.Entries.Where(ShouldRestore).ToList();
        var restored = backup.Entries.Count(entry => entry.State == BackupEntryStates.Restored);
        var failed = backup.Entries.Count(entry => !string.IsNullOrWhiteSpace(entry.FailureDetail));
        var requiresElevation = false;
        var unsupportedMachineTree = false;
        var reversiblePending = 0;
        foreach (var entry in pending)
        {
            if (entry.RegistryTree is not null)
            {
                if (!MaintenancePolicy.IsAllowedUninstallKey(entry.RegistryHive, entry.RegistrySubKey)) continue;
                reversiblePending++;
                if (string.Equals(entry.RegistryHive, "HKLM", StringComparison.OrdinalIgnoreCase))
                {
                    requiresElevation = true;
                    unsupportedMachineTree = true;
                }
                continue;
            }
            if (!_catalog.TryGetValue(entry.TweakId, out var tweak) || !tweak.Reversible) continue;
            reversiblePending++;
            if (tweak.Kind == TweakKind.Command || tweak.Kind == TweakKind.Registry &&
                tweak.Target.StartsWith("HKLM\\", StringComparison.OrdinalIgnoreCase))
                requiresElevation = true;
        }

        var sameMachine = string.Equals(backup.MachineName, Environment.MachineName,
            StringComparison.OrdinalIgnoreCase);
        var canRestore = pending.Count > 0 && reversiblePending > 0 && sameMachine &&
            (!unsupportedMachineTree || IsElevated);
        var status = pending.Count == 0 ? "Complete" : canRestore ? "Ready" : "Blocked";
        var source = Path.GetFileName(path).StartsWith("backup-registry-", StringComparison.OrdinalIgnoreCase)
            ? "Registration cleanup" : "Optimize";
        var detail = !sameMachine
            ? $"This backup belongs to {backup.MachineName}; cross-machine restore is blocked."
            : unsupportedMachineTree && !IsElevated
                ? "This HKLM registration-tree snapshot cannot cross the elevation boundary without a signed backup envelope."
                : pending.Count == 0 ? "Every recoverable entry is already restored."
                : reversiblePending == 0 ? "The remaining entries do not have an automatic undo path."
                : $"{reversiblePending:N0} entr{(reversiblePending == 1 ? "y is" : "ies are")} ready to restore.";
        return new RecoveryBackupInfo(path, Path.GetFileName(path), backup.CreatedUtc, source,
            backup.MachineName, backup.Entries.Count, pending.Count, restored, failed, requiresElevation,
            unsupportedMachineTree, canRestore, status, detail);
    }

    public async Task<RecoveryRestoreResult> RestoreAsync(string path, CancellationToken cancellationToken = default)
    {
        var backup = InspectExact(path);
        if (!backup.CanRestore) throw new InvalidOperationException(backup.Detail);
        cancellationToken.ThrowIfCancellationRequested();
        var logs = new List<string>();
        var restored = 0;
        var skipped = 0;
        var failed = 0;

        if (IsElevated)
        {
            var direct = await executor.RestoreFromAsync(backup.Path, _catalog, RestoreScope.All);
            return FromDirect(direct);
        }

        if (backup.RequiresElevation)
        {
            var elevated = await elevation.RestoreMachineBackupAsync(backup.Path, cancellationToken);
            if (elevated.Cancelled)
                return new RecoveryRestoreResult(false, true, elevated.Message, 0, 0, 0, elevated.Log);
            if (!elevated.Succeeded)
                return new RecoveryRestoreResult(false, false, elevated.Message, elevated.Applied, 0,
                    Math.Max(1, elevated.Failed), elevated.Log);
            restored += elevated.Applied;
            failed += elevated.Failed;
            logs.AddRange(elevated.Log);
        }

        // Once a protected phase has started, complete the paired current-user phase even if the
        // workspace is deactivated; otherwise navigation could leave a misleading partial restore.
        var local = await executor.RestoreFromAsync(backup.Path, _catalog, RestoreScope.CurrentUser);
        restored += local.Restored;
        skipped += local.Skipped;
        failed += local.Failed;
        logs.AddRange(local.Log);
        return new RecoveryRestoreResult(failed == 0, false,
            failed == 0 ? $"Restored {restored:N0} backup entr{(restored == 1 ? "y" : "ies")}." :
                $"Restore completed with {failed:N0} failure(s).", restored, skipped, failed, logs);
    }

    private static RecoveryRestoreResult FromDirect(RestoreResult result) => new(result.Failed == 0, false,
        result.Failed == 0 ? $"Restored {result.Restored:N0} backup entries." :
            $"Restore completed with {result.Failed:N0} failure(s).", result.Restored, result.Skipped,
        result.Failed, result.Log);

    private string ValidatePath(string path)
    {
        var full = Path.GetFullPath(path);
        var root = Path.GetFullPath(executor.BackupDirectory).TrimEnd(Path.DirectorySeparatorChar);
        if (!string.Equals(Path.GetDirectoryName(full)?.TrimEnd(Path.DirectorySeparatorChar), root,
                StringComparison.OrdinalIgnoreCase) ||
            !Path.GetFileName(full).StartsWith("backup-", StringComparison.OrdinalIgnoreCase) ||
            !full.EndsWith(".json", StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("Recovery is limited to exact Sift backup files.");
        if (File.Exists(full) && File.GetAttributes(full).HasFlag(FileAttributes.ReparsePoint))
            throw new InvalidOperationException("Recovery backup reparse points are blocked.");
        return full;
    }

    private static bool ShouldRestore(BackupEntry entry) => entry.State != BackupEntryStates.Restored &&
        (entry.AppliedSuccessfully || entry.State is BackupEntryStates.Applying or BackupEntryStates.Applied);
}
