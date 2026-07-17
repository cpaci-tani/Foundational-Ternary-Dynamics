namespace Sift.Models;

public sealed record RecoveryBackupInfo(
    string Path,
    string FileName,
    DateTime CreatedUtc,
    string Source,
    string MachineName,
    int EntryCount,
    int PendingCount,
    int RestoredCount,
    int FailedCount,
    bool RequiresElevation,
    bool HasUnsupportedMachineTree,
    bool CanRestore,
    string Status,
    string Detail)
{
    public string CreatedDisplay => CreatedUtc.ToLocalTime().ToString("yyyy-MM-dd HH:mm:ss");
    public string EntryDisplay => $"{PendingCount:N0} pending · {RestoredCount:N0} restored";
    public string ScopeDisplay => HasUnsupportedMachineTree
        ? "Machine registry tree"
        : RequiresElevation ? "Administrator + current user" : "Current user";
    public string RestoreDisplay => CanRestore ? "Yes" : "No";
}

public sealed record RecoveryRestoreResult(
    bool Succeeded,
    bool Cancelled,
    string Message,
    int Restored,
    int Skipped,
    int Failed,
    IReadOnlyList<string> Log);
