using System.IO;
using System.Text.Json;
using Sift.Models;

namespace Sift.Services;

public interface IHistoryService
{
    Task<HistorySnapshot> LoadAsync(CancellationToken cancellationToken = default);
}

public sealed class HistoryService(
    ITweakExecutor executor,
    IActivityStore activityStore,
    int maximumRows = 300) : IHistoryService
{
    private static readonly JsonSerializerOptions JsonOptions = new();

    public async Task<HistorySnapshot> LoadAsync(CancellationToken cancellationToken = default)
    {
        if (maximumRows is < 1 or > 300)
            throw new ArgumentOutOfRangeException(nameof(maximumRows));

        var warnings = new List<string>();
        var rows = new List<HistoryRow>();

        await Task.Run(() =>
        {
            try
            {
                cancellationToken.ThrowIfCancellationRequested();
                rows.AddRange(BuildOptimizeBackups(executor));
            }
            catch (OperationCanceledException) { throw; }
            catch (Exception exception)
            {
                warnings.Add($"Optimize backup history is unavailable: {exception.Message}");
            }

            try
            {
                cancellationToken.ThrowIfCancellationRequested();
                rows.AddRange(BuildRegistryBackups(executor.BackupDirectory, warnings, cancellationToken));
            }
            catch (OperationCanceledException) { throw; }
            catch (Exception exception)
            {
                warnings.Add($"Registry backup history is unavailable: {exception.Message}");
            }

            try
            {
                cancellationToken.ThrowIfCancellationRequested();
                rows.AddRange(BuildActivityRows(activityStore));
            }
            catch (OperationCanceledException) { throw; }
            catch (Exception exception)
            {
                warnings.Add($"Activity history is unavailable: {exception.Message}");
            }
        }, cancellationToken);

        return new HistorySnapshot(
            rows.OrderByDescending(row => row.TimestampUtc).Take(maximumRows).ToList(),
            warnings);
    }

    private static IEnumerable<HistoryRow> BuildOptimizeBackups(ITweakExecutor executor)
    {
        foreach (var backup in executor.ListBackups())
        {
            yield return new HistoryRow
            {
                TimestampUtc = backup.CreatedUtc,
                Category = "Optimize backup",
                Title = $"{backup.SuccessCount}/{backup.EntryCount} registry changes applied",
                Detail = Path.GetFileName(backup.Path),
                Path = backup.Path,
                CanRestoreOptimize = backup.SuccessCount > 0
            };
        }
    }

    private static IEnumerable<HistoryRow> BuildRegistryBackups(
        string backupDir,
        ICollection<string> warnings,
        CancellationToken cancellationToken)
    {
        if (!Directory.Exists(backupDir)) yield break;
        foreach (var path in Directory.GetFiles(backupDir, "backup-registry-*.json").OrderDescending())
        {
            cancellationToken.ThrowIfCancellationRequested();
            DateTime created = File.GetCreationTimeUtc(path);
            var entryCount = 0;
            var canRestore = false;
            try
            {
                var backup = JsonSerializer.Deserialize<Backup>(File.ReadAllText(path), JsonOptions);
                entryCount = backup?.Entries.Count ?? 0;
                if (backup?.CreatedUtc is { } utc) created = utc;
                canRestore = backup?.Entries.Any(entry => entry.RegistryTree is not null &&
                    entry.State != BackupEntryStates.Restored &&
                    (entry.AppliedSuccessfully || entry.State is BackupEntryStates.Applying or BackupEntryStates.Applied)) == true;
            }
            catch (Exception exception)
            {
                warnings.Add($"Unreadable registry backup {Path.GetFileName(path)}: {exception.Message}");
            }

            yield return new HistoryRow
            {
                TimestampUtc = created,
                Category = "Registry backup",
                Title = "Orphan uninstall key snapshot",
                Detail = $"{Path.GetFileName(path)} · {entryCount} key(s)",
                Path = path,
                CanRestoreOptimize = canRestore
            };
        }
    }

    private static IEnumerable<HistoryRow> BuildActivityRows(IActivityStore activityStore)
    {
        foreach (var entry in activityStore.Load())
        {
            yield return new HistoryRow
            {
                TimestampUtc = entry.CreatedUtc,
                Category = entry.Category,
                Title = entry.Summary,
                Detail = entry.Detail ?? "",
                Path = entry.RelatedPath,
                CanRestoreOptimize = false
            };
        }
    }
}
