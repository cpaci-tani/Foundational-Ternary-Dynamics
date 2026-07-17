using System.Text.Json;
using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class HistoryServiceTests
{
    [Fact]
    public async Task Load_merges_activity_optimize_and_registry_backup_rows_newest_first()
    {
        var root = TempRoot();
        try
        {
            var executor = new FakeExecutor(root,
            [
                new BackupInfo { Path = Path.Combine(root, "backup-optimize.json"), CreatedUtc = DateTime.UtcNow.AddHours(-1), EntryCount = 2, SuccessCount = 2 }
            ]);
            File.WriteAllText(Path.Combine(root, "backup-registry-old.json"),
                JsonSerializer.Serialize(new Backup { CreatedUtc = DateTime.UtcNow.AddHours(-3), Entries = [] }));
            File.WriteAllText(Path.Combine(root, "backup-registry-new.json"),
                JsonSerializer.Serialize(new Backup { CreatedUtc = DateTime.UtcNow.AddHours(-2), Entries = [] }));
            var activity = new FakeActivityStore(
            [
                new ActivityEntry { CreatedUtc = DateTime.UtcNow.AddMinutes(-30), Category = "Optimize", Summary = "Applied", Detail = "detail" }
            ]);

            var snapshot = await new HistoryService(executor, activity).LoadAsync(TestContext.Current.CancellationToken);

            Assert.Equal(4, snapshot.Rows.Count);
            Assert.Equal("Optimize", snapshot.Rows[0].Category);
            Assert.Equal("Optimize backup", snapshot.Rows[1].Category);
            Assert.Equal("Registry backup", snapshot.Rows[2].Category);
        }
        finally { Directory.Delete(root, true); }
    }

    [Fact]
    public async Task Load_is_bounded_to_300_rows()
    {
        var activity = new FakeActivityStore(Enumerable.Range(0, 350).Select(index =>
            new ActivityEntry
            {
                CreatedUtc = DateTime.UtcNow.AddMinutes(-index),
                Category = "Test",
                Summary = $"entry-{index}",
                Detail = "detail"
            }).ToList());
        var snapshot = await new HistoryService(new FakeExecutor(TempRoot()), activity, 300)
            .LoadAsync(TestContext.Current.CancellationToken);
        Assert.Equal(300, snapshot.Rows.Count);
    }

    [Fact]
    public async Task Load_reports_activity_failure_as_partial_with_backup_rows()
    {
        var root = TempRoot();
        try
        {
            var executor = new FakeExecutor(root,
            [
                new BackupInfo { Path = Path.Combine(root, "backup-optimize.json"), CreatedUtc = DateTime.UtcNow, EntryCount = 1, SuccessCount = 1 }
            ]);
            var snapshot = await new HistoryService(executor, new ThrowingActivityStore())
                .LoadAsync(TestContext.Current.CancellationToken);
            Assert.Single(snapshot.Rows);
            Assert.Contains(snapshot.Warnings, warning => warning.Contains("Activity history is unavailable"));
        }
        finally { Directory.Delete(root, true); }
    }

    [Fact]
    public async Task Load_reports_backup_failure_as_partial_with_activity_rows()
    {
        var activity = new FakeActivityStore(
        [
            new ActivityEntry { CreatedUtc = DateTime.UtcNow, Category = "Test", Summary = "ok", Detail = "" }
        ]);
        var snapshot = await new HistoryService(new ThrowingExecutor(), activity)
            .LoadAsync(TestContext.Current.CancellationToken);
        Assert.Single(snapshot.Rows);
        Assert.Contains(snapshot.Warnings, warning => warning.Contains("Optimize backup history is unavailable"));
    }

    [Fact]
    public async Task Load_reports_unreadable_registry_backup_as_a_warning()
    {
        var root = TempRoot();
        try
        {
            File.WriteAllText(Path.Combine(root, "backup-registry-bad.json"), "{ not-json");
            var snapshot = await new HistoryService(new FakeExecutor(root), new FakeActivityStore([]))
                .LoadAsync(TestContext.Current.CancellationToken);
            Assert.Contains(snapshot.Warnings, warning => warning.Contains("Unreadable registry backup"));
        }
        finally { Directory.Delete(root, true); }
    }

    [Fact]
    public async Task Load_honors_cancellation()
    {
        using var cancellation = new CancellationTokenSource();
        var service = new HistoryService(new SlowExecutor(cancellation), new FakeActivityStore([]));
        await Assert.ThrowsAsync<OperationCanceledException>(() => service.LoadAsync(cancellation.Token));
    }

    private static string TempRoot()
    {
        var root = Path.Combine(Path.GetTempPath(), "sift-history-tests", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        return root;
    }

    private sealed class FakeExecutor(string root, IReadOnlyList<BackupInfo>? backups = null) : ITweakExecutor
    {
        public string BackupDirectory => root;
        public bool IsApplied(Tweak tweak) => false;
        public Task<ApplyResult> ApplyAsync(IEnumerable<Tweak> selection, bool dryRun, CancellationToken cancellationToken = default) => throw new NotSupportedException();
        public Task<RestoreResult> RestoreFromAsync(string path, IReadOnlyDictionary<string, Tweak> catalog,
            RestoreScope scope = RestoreScope.All) => throw new NotSupportedException();
        public IReadOnlyList<BackupInfo> ListBackups() => backups ?? [];
    }

    private sealed class ThrowingExecutor : ITweakExecutor
    {
        public string BackupDirectory => throw new InvalidOperationException("backup unavailable");
        public bool IsApplied(Tweak tweak) => false;
        public Task<ApplyResult> ApplyAsync(IEnumerable<Tweak> selection, bool dryRun, CancellationToken cancellationToken = default) => throw new NotSupportedException();
        public Task<RestoreResult> RestoreFromAsync(string path, IReadOnlyDictionary<string, Tweak> catalog,
            RestoreScope scope = RestoreScope.All) => throw new NotSupportedException();
        public IReadOnlyList<BackupInfo> ListBackups() => throw new InvalidOperationException("backup unavailable");
    }

    private sealed class SlowExecutor(CancellationTokenSource cancellation) : ITweakExecutor
    {
        public string BackupDirectory
        {
            get
            {
                cancellation.Cancel();
                throw new OperationCanceledException(cancellation.Token);
            }
        }
        public bool IsApplied(Tweak tweak) => false;
        public Task<ApplyResult> ApplyAsync(IEnumerable<Tweak> selection, bool dryRun, CancellationToken cancellationToken = default) => throw new NotSupportedException();
        public Task<RestoreResult> RestoreFromAsync(string path, IReadOnlyDictionary<string, Tweak> catalog,
            RestoreScope scope = RestoreScope.All) => throw new NotSupportedException();
        public IReadOnlyList<BackupInfo> ListBackups() => [];
    }

    private sealed class FakeActivityStore(IReadOnlyList<ActivityEntry> entries) : IActivityStore
    {
        public IReadOnlyList<ActivityEntry> Load() => entries;
        public void Append(string category, string summary, string? detail = null, string? relatedPath = null) { }
    }

    private sealed class ThrowingActivityStore : IActivityStore
    {
        public IReadOnlyList<ActivityEntry> Load() => throw new InvalidOperationException("activity unavailable");
        public void Append(string category, string summary, string? detail = null, string? relatedPath = null) { }
    }
}
