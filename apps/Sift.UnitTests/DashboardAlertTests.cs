using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class DashboardAlertTests
{
    [Fact]
    public async Task Alert_requires_duration_then_clears_with_hysteresis()
    {
        var root = TempDirectory();
        try
        {
            using var store = new DashboardHistoryStore(root);
            var engine = new DashboardAlertEngine(store);
            var preferences = new DashboardPreferences
            {
                AlertRules =
                [
                    new DashboardAlertRule
                    {
                        Id = "memory",
                        MetricKey = "memory.percent",
                        Title = "Memory high",
                        Threshold = 90,
                        RequiredDuration = TimeSpan.FromMinutes(2),
                        Hysteresis = 5,
                        Cooldown = TimeSpan.FromMinutes(30)
                    }
                ]
            };
            var start = DateTimeOffset.UtcNow;

            Assert.Empty(await engine.EvaluateAsync(Snapshot(start, 95), preferences, TestContext.Current.CancellationToken));
            Assert.Empty(await engine.EvaluateAsync(Snapshot(start.AddMinutes(1), 96), preferences, TestContext.Current.CancellationToken));
            var raised = Assert.Single(await engine.EvaluateAsync(
                Snapshot(start.AddMinutes(2), 97), preferences, TestContext.Current.CancellationToken));
            Assert.Null(raised.ClearedUtc);
            Assert.Empty(await engine.EvaluateAsync(Snapshot(start.AddMinutes(3), 88), preferences, TestContext.Current.CancellationToken));
            var cleared = Assert.Single(await engine.EvaluateAsync(
                Snapshot(start.AddMinutes(4), 84), preferences, TestContext.Current.CancellationToken));
            Assert.NotNull(cleared.ClearedUtc);
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public async Task Acknowledge_and_snooze_are_persisted()
    {
        var root = TempDirectory();
        try
        {
            using var store = new DashboardHistoryStore(root);
            var engine = new DashboardAlertEngine(store);
            var preferences = new DashboardPreferences
            {
                AlertRules =
                [
                    new DashboardAlertRule
                    {
                        Id = "storage",
                        MetricKey = "storage.lowest_free_percent",
                        Title = "Storage low",
                        Threshold = 10,
                        TriggerWhenBelow = true,
                        RequiredDuration = TimeSpan.Zero,
                        Hysteresis = 2
                    }
                ]
            };
            var now = DateTimeOffset.UtcNow;
            var alert = Assert.Single(await engine.EvaluateAsync(
                Snapshot(now, 5, "storage.lowest_free_percent"), preferences, TestContext.Current.CancellationToken));

            await engine.AcknowledgeAsync(alert.Id, now.AddMinutes(1), TestContext.Current.CancellationToken);
            await engine.SnoozeAsync(alert.Id, now.AddHours(1), TestContext.Current.CancellationToken);
            var persisted = Assert.Single(await store.LoadAlertsAsync(TestContext.Current.CancellationToken));
            Assert.NotNull(persisted.AcknowledgedUtc);
            Assert.NotNull(persisted.SnoozedUntilUtc);
        }
        finally { DeleteDirectory(root); }
    }

    [Theory]
    [InlineData(23, false)]
    [InlineData(7, false)]
    [InlineData(12, true)]
    public void Notification_policy_honors_disabled_default_toast_and_overnight_quiet_hours(int hour, bool expected)
    {
        var preferences = new DashboardPreferences
        {
            NotificationsEnabled = true,
            QuietHoursStart = new TimeOnly(22, 0),
            QuietHoursEnd = new TimeOnly(8, 0)
        };
        var rule = new DashboardAlertRule { Id = "test", ToastEnabled = true };

        Assert.Equal(expected, DashboardNotificationPolicy.ShouldDeliver(preferences, rule, new TimeOnly(hour, 0)));
        preferences.NotificationsEnabled = false;
        Assert.False(DashboardNotificationPolicy.ShouldDeliver(preferences, rule, new TimeOnly(12, 0)));
    }

    [Fact]
    public async Task Alert_engine_ignores_cached_metric_that_was_not_changed()
    {
        var root = TempDirectory();
        try
        {
            using var store = new DashboardHistoryStore(root);
            var engine = new DashboardAlertEngine(store);
            var preferences = new DashboardPreferences
            {
                AlertRules =
                [
                    new DashboardAlertRule
                    {
                        Id = "memory",
                        MetricKey = "memory.percent",
                        Title = "Memory high",
                        Threshold = 90,
                        RequiredDuration = TimeSpan.Zero
                    }
                ]
            };
            var now = DateTimeOffset.UtcNow;
            var cached = Snapshot(now, 99) with { ChangedMetricKeys = [] };

            Assert.Empty(await engine.EvaluateAsync(cached, preferences, TestContext.Current.CancellationToken));
            Assert.Empty(engine.Alerts);
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public async Task Alert_engine_synchronizes_and_resets_in_memory_state()
    {
        var root = TempDirectory();
        try
        {
            using var store = new DashboardHistoryStore(root);
            var engine = new DashboardAlertEngine(store);
            var now = DateTimeOffset.UtcNow;
            var alert = new DashboardAlert("remote", "rule", "memory.percent", "Remote alert", "detail",
                "Warning", now, null, now, null);

            await engine.SynchronizeAsync([alert], TestContext.Current.CancellationToken);
            Assert.Equal("remote", Assert.Single(engine.Alerts).Id);
            Assert.NotNull(engine.Alerts[0].AcknowledgedUtc);

            await engine.ResetAsync(TestContext.Current.CancellationToken);
            Assert.Empty(engine.Alerts);
        }
        finally { DeleteDirectory(root); }
    }

    private static DashboardSnapshotDelta Snapshot(
        DateTimeOffset timestamp,
        double value,
        string key = "memory.percent") => new(
        timestamp.ToUnixTimeSeconds(), timestamp,
        new Dictionary<string, DashboardMetricSample>(StringComparer.OrdinalIgnoreCase)
        {
            [key] = new DashboardMetricSample(key, value, "%", timestamp)
        }, [], []);

    private static string TempDirectory() => Path.Combine(Path.GetTempPath(), "Sift-Dashboard-Alerts-" + Guid.NewGuid().ToString("N"));
    private static void DeleteDirectory(string path)
    {
        try { if (Directory.Exists(path)) Directory.Delete(path, recursive: true); }
        catch { }
    }
}
