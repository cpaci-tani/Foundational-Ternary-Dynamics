using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Infrastructure.Settings;
using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class InfrastructureTests
{
    [Fact]
    public void ActivityHub_FansOutTypedEventExactlyOnce()
    {
        var sink = new RecordingSink();
        var hub = new ActivityHub(sink);
        ActivityEvent? published = null;
        hub.Published += (_, activity) => published = activity;

        hub.Warning("Storage", "Scan cancelled", operationId: "abc123");

        var recorded = Assert.Single(sink.Events);
        Assert.Same(recorded, published);
        Assert.Equal(ActivitySeverity.Warning, recorded.Severity);
        Assert.Equal("Storage", recorded.Category);
        Assert.Equal("abc123", recorded.OperationId);
    }

    [Fact]
    public async Task OperationCoordinator_LatestRequestCancelsPreviousRequest()
    {
        var hub = new ActivityHub();
        using var operations = new OperationCoordinator(hub);
        var entered = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        var first = operations.RunLatestAsync<int>("inventory", "Test", "first", async token =>
        {
            entered.SetResult();
            await Task.Delay(Timeout.InfiniteTimeSpan, token);
            return 1;
        }, TestContext.Current.CancellationToken);
        await entered.Task.WaitAsync(TestContext.Current.CancellationToken);

        var second = operations.RunLatestAsync("inventory", "Test", "second", _ => Task.FromResult(2), TestContext.Current.CancellationToken);
        var secondResult = await second;
        var firstResult = await first;

        Assert.True(secondResult.Succeeded);
        Assert.Equal(2, secondResult.Value);
        Assert.True(firstResult.Cancelled);
    }

    [Fact]
    public async Task OperationCoordinator_DropsStaleResultWhenDelegateIgnoresCancellation()
    {
        var hub = new ActivityHub();
        using var operations = new OperationCoordinator(hub);
        var entered = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        var release = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        var first = operations.RunLatestAsync<int>("inventory", "Test", "first", async _ =>
        {
            entered.SetResult();
            await release.Task;
            return 1;
        }, TestContext.Current.CancellationToken);
        await entered.Task.WaitAsync(TestContext.Current.CancellationToken);

        var second = await operations.RunLatestAsync("inventory", "Test", "second", _ => Task.FromResult(2), TestContext.Current.CancellationToken);
        release.SetResult();
        var stale = await first;

        Assert.True(second.Succeeded);
        Assert.True(stale.Cancelled);
    }

    [Fact]
    public async Task OperationCoordinator_CommittedMutationIgnoresNavigationCancelAndRejectsDuplicate()
    {
        var sink = new RecordingSink();
        using var operations = new OperationCoordinator(new ActivityHub(sink));
        var entered = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        var release = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        var committed = operations.RunCommittedAsync<int>("mutation", "Test", "change", async token =>
        {
            Assert.False(token.CanBeCanceled);
            entered.SetResult();
            await release.Task;
            return 7;
        }, TestContext.Current.CancellationToken);
        await entered.Task.WaitAsync(TestContext.Current.CancellationToken);

        operations.Cancel("mutation");
        var duplicateStarted = false;
        var duplicate = await operations.RunCommittedAsync("mutation", "Test", "duplicate", _ =>
        {
            duplicateStarted = true;
            return Task.FromResult(9);
        }, TestContext.Current.CancellationToken);
        release.SetResult();
        var result = await committed;

        Assert.True(result.Succeeded);
        Assert.Equal(7, result.Value);
        Assert.False(duplicate.Succeeded);
        Assert.IsType<InvalidOperationException>(duplicate.Error);
        Assert.False(duplicateStarted);
        Assert.Contains(sink.Events, activity => activity.Persist &&
            activity.Summary.StartsWith("Completed committed", StringComparison.Ordinal));
    }

    [Fact]
    public async Task SettingsPersistence_DebouncesRepeatedSchedules()
    {
        var store = new RecordingSettingsStore();
        using var persistence = new SettingsPersistenceCoordinator(store, TimeSpan.FromMilliseconds(30));
        var settings = new AppSettings();

        persistence.Schedule(settings);
        persistence.Schedule(settings);
        persistence.Schedule(settings);
        await Task.Delay(100, TestContext.Current.CancellationToken);

        Assert.Equal(1, store.SaveCount);
    }

    [Fact]
    public async Task SettingsPersistence_SaveNowOverridesPendingSchedule()
    {
        var store = new RecordingSettingsStore();
        using var persistence = new SettingsPersistenceCoordinator(store, TimeSpan.FromMilliseconds(200));
        var older = new AppSettings { LastMaintenanceScanUtc = "2020-01-01T00:00:00.0000000Z" };
        var newer = new AppSettings { LastMaintenanceScanUtc = "2026-07-14T12:00:00.0000000Z" };

        persistence.Schedule(older);
        persistence.SaveNow(newer);
        await Task.Delay(300, TestContext.Current.CancellationToken);

        Assert.Equal(1, store.SaveCount);
        Assert.Equal(newer.LastMaintenanceScanUtc, store.LastSaved?.LastMaintenanceScanUtc);
    }

    [Fact]
    public void ActivityStore_UsesRecoverableLocalRoundTrip()
    {
        var directory = Path.Combine(Path.GetTempPath(), "SiftUnit-" + Guid.NewGuid().ToString("N"));
        try
        {
            var store = new ActivityStore(directory);
            store.Append("Test", "Completed", "detail");
            var entry = Assert.Single(store.Load());
            Assert.Equal("Test", entry.Category);
            Assert.Equal("Completed", entry.Summary);
        }
        finally
        {
            try { Directory.Delete(directory, recursive: true); } catch { }
        }
    }

    private sealed class RecordingSink : IActivitySink
    {
        public List<ActivityEvent> Events { get; } = [];
        public void Publish(ActivityEvent activity) => Events.Add(activity);
    }

    private sealed class RecordingSettingsStore : ISettingsStore
    {
        public int SaveCount => _saveCount;
        public string SettingsPath => "memory";
        public AppSettings? LastSaved { get; private set; }
        public AppSettings Load() => new();
        public void Save(AppSettings settings)
        {
            LastSaved = settings;
            Interlocked.Increment(ref _saveCount);
        }
        private int _saveCount;
    }
}
