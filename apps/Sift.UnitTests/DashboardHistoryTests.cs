using Microsoft.Data.Sqlite;
using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class DashboardHistoryTests
{
    [Fact]
    public async Task History_aggregates_minute_samples_without_process_identity()
    {
        var root = TempDirectory();
        try
        {
            using var store = new DashboardHistoryStore(root);
            var now = DateTimeOffset.FromUnixTimeSeconds(DateTimeOffset.UtcNow.ToUnixTimeSeconds() / 60 * 60 + 5);
            await store.AppendAsync(
            [
                new DashboardMetricSample("cpu.percent", 10, "%", now),
                new DashboardMetricSample("cpu.percent", 30, "%", now.AddSeconds(20)),
                new DashboardMetricSample("process.chrome.cpu", 99, "%", now)
            ], TestContext.Current.CancellationToken);

            var rows = await store.QueryAsync("cpu.percent", now.AddMinutes(-1), now.AddMinutes(1),
                TestContext.Current.CancellationToken);
            var row = Assert.Single(rows);
            Assert.Equal(10, row.Minimum);
            Assert.Equal(30, row.Maximum);
            Assert.Equal(20, row.Average);
            Assert.Equal(2, row.SampleCount);
            Assert.False(DashboardMetricPolicy.IsPersistable("process.chrome.cpu"));
            await Assert.ThrowsAsync<ArgumentException>(() => store.QueryAsync(
                "process.chrome.cpu", now.AddMinutes(-1), now.AddMinutes(1), TestContext.Current.CancellationToken));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public async Task History_compacts_old_minutes_and_prunes_beyond_retention()
    {
        var root = TempDirectory();
        try
        {
            using var store = new DashboardHistoryStore(root);
            var now = DateTimeOffset.UtcNow;
            await store.AppendAsync(
            [
                new DashboardMetricSample("memory.percent", 40, "%", now.AddDays(-8)),
                new DashboardMetricSample("memory.percent", 50, "%", now.AddDays(-91)),
                new DashboardMetricSample("memory.percent", 60, "%", now.AddHours(-1))
            ], TestContext.Current.CancellationToken);

            await store.CompactAsync(now, 90, TestContext.Current.CancellationToken);
            var rows = await store.QueryAsync("memory.percent", now.AddDays(-100), now,
                TestContext.Current.CancellationToken);

            Assert.DoesNotContain(rows, row => Math.Abs(row.Average - 50) < 0.001);
            Assert.Contains(rows, row => row.Resolution == TimeSpan.FromMinutes(15) && Math.Abs(row.Average - 40) < 0.001);
            Assert.Contains(rows, row => row.Resolution == TimeSpan.FromMinutes(1) && Math.Abs(row.Average - 60) < 0.001);
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public async Task Clear_removes_metric_history()
    {
        var root = TempDirectory();
        try
        {
            using var store = new DashboardHistoryStore(root);
            var now = DateTimeOffset.UtcNow;
            await store.AppendAsync([new DashboardMetricSample("cpu.percent", 10, "%", now)],
                TestContext.Current.CancellationToken);
            await store.ClearAsync(TestContext.Current.CancellationToken);
            Assert.Empty(await store.QueryAsync("cpu.percent", now.AddDays(-1), now.AddDays(1),
                TestContext.Current.CancellationToken));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public async Task Cancelled_flush_requeues_pending_aggregates()
    {
        var root = TempDirectory();
        try
        {
            using var store = new DashboardHistoryStore(root);
            var now = DateTimeOffset.UtcNow;
            await store.AppendAsync([new DashboardMetricSample("cpu.percent", 42, "%", now)],
                TestContext.Current.CancellationToken);
            using var cancelled = new CancellationTokenSource();
            cancelled.Cancel();

            await Assert.ThrowsAnyAsync<OperationCanceledException>(() =>
                store.QueryAsync("cpu.percent", now.AddMinutes(-1), now.AddMinutes(1), cancelled.Token));

            var recovered = await store.QueryAsync("cpu.percent", now.AddMinutes(-1), now.AddMinutes(1),
                TestContext.Current.CancellationToken);
            Assert.Equal(42, Assert.Single(recovered).Average);
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public async Task Alert_storage_prunes_rows_beyond_bounded_retention()
    {
        var root = TempDirectory();
        try
        {
            using var store = new DashboardHistoryStore(root);
            var now = DateTimeOffset.UtcNow;
            for (var index = 0; index < 505; index++)
            {
                await store.UpsertAlertAsync(new DashboardAlert($"alert-{index}", "rule", "memory.percent",
                    "Alert", "detail", "Warning", now.AddSeconds(index), null, null, null),
                    TestContext.Current.CancellationToken);
            }

            await using var connection = new SqliteConnection($"Data Source={store.DatabasePath}");
            await connection.OpenAsync(TestContext.Current.CancellationToken);
            await using var command = connection.CreateCommand();
            command.CommandText = "SELECT COUNT(*) FROM alerts;";
            Assert.Equal(500L, Convert.ToInt64(await command.ExecuteScalarAsync(TestContext.Current.CancellationToken)));
        }
        finally { DeleteDirectory(root); }
    }

    private static string TempDirectory() => Path.Combine(Path.GetTempPath(), "Sift-Dashboard-History-" + Guid.NewGuid().ToString("N"));
    private static void DeleteDirectory(string path)
    {
        try { if (Directory.Exists(path)) Directory.Delete(path, recursive: true); }
        catch { }
    }
}
