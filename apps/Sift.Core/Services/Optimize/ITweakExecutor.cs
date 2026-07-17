using Sift.Models;

namespace Sift.Services;

public enum RestoreScope
{
    All,
    CurrentUser,
    ElevatedMachine
}

public interface ITweakExecutor
{
    string BackupDirectory { get; }
    bool IsApplied(Tweak tweak);
    Task<ApplyResult> ApplyAsync(IEnumerable<Tweak> selection, bool dryRun,
        CancellationToken cancellationToken = default);
    Task<RestoreResult> RestoreFromAsync(string path, IReadOnlyDictionary<string, Tweak> catalog,
        RestoreScope scope = RestoreScope.All);
    IReadOnlyList<BackupInfo> ListBackups();
}
