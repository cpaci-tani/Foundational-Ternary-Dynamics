using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Views;
using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public sealed class HealthWorkspaceModule : IWorkspaceModule
{
    private readonly IHealthWorkspaceOrchestrator _orchestrator;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly HealthWorkspaceView _view = new();
    private readonly DispatcherQueue _dispatcher = DispatcherQueue.GetForCurrentThread();
    private const string RefreshOperation = "workspace.health.refresh";

    public HealthWorkspaceModule(IHealthWorkspaceOrchestrator orchestrator, OperationCoordinator operations,
        ActivityHub activity)
    {
        _orchestrator = orchestrator;
        _operations = operations;
        _activity = activity;
        _view.RefreshRequested += View_RefreshRequested;
    }

    public string Key => "Health";
    public string Title => "Health";
    public Control View => _view;
    public Task ActivateAsync(CancellationToken cancellationToken = default) => RefreshAsync(cancellationToken);
    public void FocusPrimarySearch() => _view.FocusSearch();

    public async Task RefreshAsync(CancellationToken cancellationToken = default)
    {
        _view.SetLoading();
        var outcome = await _operations.RunLatestAsync(RefreshOperation, Key, "health refresh",
            token => _orchestrator.RefreshAsync(token), cancellationToken);
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetStatus($"Could not load health: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }

        var result = outcome.Value;
        if (result.Cancelled || result.Stale) return;

        await _dispatcher.EnqueueAsync(() =>
        {
            _view.BindChecks(result.Checks);
            _view.BindHistory(result.History);
            if (result.Warnings.Any(warning => warning.StartsWith("Could not load checks", StringComparison.OrdinalIgnoreCase)))
                _view.SetChecksError("Could not load checks");
            if (result.Warnings.Any(warning => warning.StartsWith("Could not load history", StringComparison.OrdinalIgnoreCase)))
                _view.SetHistoryError("Could not load history");
            var attention = result.Checks.Count(check =>
                check.Status is HealthStatus.Warning or HealthStatus.Critical);
            _view.SetStatus($"{result.Checks.Count:N0} checks · {attention:N0} need attention");
            return Task.CompletedTask;
        });
    }

    public void Deactivate()
    {
        _operations.Cancel(RefreshOperation);
        _orchestrator.Deactivate();
    }

    public void Dispose()
    {
        Deactivate();
        _view.RefreshRequested -= View_RefreshRequested;
    }

    private async void View_RefreshRequested(object? sender, EventArgs e)
    {
        _activity.Info(Key, "Refreshing health workspace", "Reloading checks and activity history.");
        await RefreshAsync();
    }
}

internal static class DispatcherQueueExtensions
{
    public static Task EnqueueAsync(this DispatcherQueue queue, Func<Task> action)
    {
        var completion = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        if (!queue.TryEnqueue(async () =>
            {
                try
                {
                    await action();
                    completion.TrySetResult();
                }
                catch (Exception exception)
                {
                    completion.TrySetException(exception);
                }
            }))
            completion.TrySetException(new InvalidOperationException("The UI dispatcher rejected the health update."));
        return completion.Task;
    }
}
