using Sift.Models;

namespace Sift.Services;

public sealed record HealthWorkspaceResult(
    long Generation,
    IReadOnlyList<HealthCheckRow> Checks,
    HistorySnapshot History,
    IReadOnlyList<string> Warnings,
    bool Cancelled,
    bool Stale);

public interface IHealthWorkspaceOrchestrator
{
    Task<HealthWorkspaceResult> RefreshAsync(CancellationToken cancellationToken = default);
    void Deactivate();
}

public sealed class HealthWorkspaceOrchestrator(IHealthInventory health, IHistoryService history)
    : IHealthWorkspaceOrchestrator
{
    private readonly object _sync = new();
    private long _generation;
    private CancellationTokenSource? _active;

    public async Task<HealthWorkspaceResult> RefreshAsync(CancellationToken cancellationToken = default)
    {
        CancellationTokenSource linked;
        long generation;
        lock (_sync)
        {
            _active?.Cancel();
            _active?.Dispose();
            _active = new CancellationTokenSource();
            linked = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken, _active.Token);
            generation = ++_generation;
        }

        using (linked)
        {
            var warnings = new List<string>();
            IReadOnlyList<HealthCheckRow> checks = [];
            HistorySnapshot historySnapshot = new([], []);
            try
            {
                checks = await Task.Run(health.Scan, linked.Token);
            }
            catch (OperationCanceledException) when (linked.Token.IsCancellationRequested)
            {
                return StaleResult(generation, true);
            }
            catch (Exception exception)
            {
                warnings.Add($"Could not load checks: {exception.Message}");
            }

            try
            {
                historySnapshot = await history.LoadAsync(linked.Token);
                warnings.AddRange(historySnapshot.Warnings);
            }
            catch (OperationCanceledException) when (linked.Token.IsCancellationRequested)
            {
                return StaleResult(generation, true);
            }
            catch (Exception exception)
            {
                warnings.Add($"Could not load history: {exception.Message}");
            }

            return CurrentResult(generation, checks, historySnapshot, warnings);
        }
    }

    public void Deactivate()
    {
        lock (_sync)
        {
            _generation++;
            _active?.Cancel();
            _active?.Dispose();
            _active = null;
        }
    }

    private HealthWorkspaceResult CurrentResult(
        long generation,
        IReadOnlyList<HealthCheckRow> checks,
        HistorySnapshot history,
        IReadOnlyList<string> warnings)
    {
        lock (_sync)
        {
            if (generation != _generation)
                return StaleResult(generation, false);
            return new HealthWorkspaceResult(generation, checks, history, warnings, false, false);
        }
    }

    private HealthWorkspaceResult StaleResult(long generation, bool cancelled) =>
        new(generation, [], new HistorySnapshot([], []), [], cancelled, true);
}
