using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class HealthWorkspaceOrchestratorTests
{
    [Fact]
    public async Task Latest_refresh_suppresses_noncooperative_stale_result()
    {
        var health = new BarrierHealthInventory();
        var orchestrator = new HealthWorkspaceOrchestrator(health, new ImmediateHistoryService());
        var first = orchestrator.RefreshAsync(TestContext.Current.CancellationToken);
        await health.Entered.Task.WaitAsync(TestContext.Current.CancellationToken);
        var second = await orchestrator.RefreshAsync(TestContext.Current.CancellationToken);
        health.Release.SetResult();
        var stale = await first;
        Assert.True(stale.Stale || stale.Cancelled);
        Assert.False(second.Stale);
        Assert.False(second.Cancelled);
    }

    [Fact]
    public async Task Deactivate_cancels_owned_refresh()
    {
        var health = new BarrierHealthInventory();
        var orchestrator = new HealthWorkspaceOrchestrator(health, new ImmediateHistoryService());
        var refresh = orchestrator.RefreshAsync(TestContext.Current.CancellationToken);
        await health.Entered.Task.WaitAsync(TestContext.Current.CancellationToken);
        orchestrator.Deactivate();
        health.Release.SetResult();
        var result = await refresh;
        Assert.True(result.Cancelled || result.Stale);
    }

    [Fact]
    public async Task Checks_failure_retains_history_rows_and_warning()
    {
        var orchestrator = new HealthWorkspaceOrchestrator(new ThrowingHealthInventory(),
            new ImmediateHistoryService([new HistoryRow
            {
                TimestampUtc = DateTime.UtcNow, Category = "Test", Title = "row", Detail = "detail"
            }]));
        var result = await orchestrator.RefreshAsync(TestContext.Current.CancellationToken);
        Assert.Single(result.History.Rows);
        Assert.Contains(result.Warnings, warning => warning.StartsWith("Could not load checks"));
    }

    [Fact]
    public async Task History_failure_retains_checks_rows_and_warning()
    {
        var orchestrator = new HealthWorkspaceOrchestrator(new ImmediateHealthInventory(),
            new ThrowingHistoryService());
        var result = await orchestrator.RefreshAsync(TestContext.Current.CancellationToken);
        Assert.NotEmpty(result.Checks);
        Assert.Contains(result.Warnings, warning => warning.StartsWith("Could not load history"));
    }

    [Fact]
    public async Task History_partial_result_retains_rows_and_source_warnings()
    {
        var orchestrator = new HealthWorkspaceOrchestrator(new ImmediateHealthInventory(),
            new ImmediateHistoryService(
            [
                new HistoryRow { TimestampUtc = DateTime.UtcNow, Category = "Registry backup", Title = "row", Detail = "detail" }
            ],
            ["Unreadable registry backup fixture.json: bad json"]));
        var result = await orchestrator.RefreshAsync(TestContext.Current.CancellationToken);
        Assert.Single(result.History.Rows);
        Assert.True(result.History.IsPartial);
        Assert.Contains(result.Warnings, warning => warning.Contains("Unreadable registry backup"));
    }

    private sealed class ImmediateHealthInventory : IHealthInventory
    {
        public IReadOnlyList<HealthCheckRow> Scan() =>
        [
            new()
            {
                Id = "memory", Title = "Memory", Status = HealthStatus.Ok, Detail = "ok", Recommendation = "none"
            }
        ];
    }

    private sealed class ThrowingHealthInventory : IHealthInventory
    {
        public IReadOnlyList<HealthCheckRow> Scan() => throw new InvalidOperationException("checks unavailable");
    }

    private sealed class BarrierHealthInventory : IHealthInventory
    {
        public TaskCompletionSource Entered { get; } = new(TaskCreationOptions.RunContinuationsAsynchronously);
        public TaskCompletionSource Release { get; } = new(TaskCreationOptions.RunContinuationsAsynchronously);

        public IReadOnlyList<HealthCheckRow> Scan()
        {
            Entered.SetResult();
            Release.Task.GetAwaiter().GetResult();
            return
            [
                new()
                {
                    Id = "memory", Title = "Memory", Status = HealthStatus.Ok, Detail = "ok", Recommendation = "none"
                }
            ];
        }
    }

    private sealed class ImmediateHistoryService(
        IReadOnlyList<HistoryRow>? rows = null,
        IReadOnlyList<string>? warnings = null) : IHistoryService
    {
        public Task<HistorySnapshot> LoadAsync(CancellationToken cancellationToken = default) =>
            Task.FromResult(new HistorySnapshot(rows ?? [], warnings ?? []));
    }

    private sealed class ThrowingHistoryService : IHistoryService
    {
        public Task<HistorySnapshot> LoadAsync(CancellationToken cancellationToken = default) =>
            throw new InvalidOperationException("history unavailable");
    }
}
