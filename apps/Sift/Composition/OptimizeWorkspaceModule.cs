using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Views;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public sealed class OptimizeWorkspaceModule : IWorkspaceModule
{
    private readonly ITweakExecutor _executor;
    private readonly IElevationBroker _elevation;
    private readonly IOptimizeMutationWorkflow _workflow;
    private readonly AppSettings _settings;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly IWorkspaceNavigator _navigator;
    private readonly IReadOnlyList<Tweak> _catalog = TweakCatalog.Create();
    private readonly OptimizeWorkspaceView _view = new();
    private const string MutationOperation = "workspace.optimize.mutate";
    private bool _active;

    public OptimizeWorkspaceModule(ITweakExecutor executor, IElevationBroker elevation, IOptimizeMutationWorkflow workflow,
        AppSettings settings, OperationCoordinator operations, ActivityHub activity, IWorkspaceNavigator navigator)
    {
        _executor = executor;
        _elevation = elevation;
        _workflow = workflow;
        _settings = settings;
        _operations = operations;
        _activity = activity;
        _navigator = navigator;
        _view.Bind(_catalog);
        _view.RunRequested += View_RunRequested;
        _view.OpenBackupsRequested += View_OpenBackupsRequested;
    }

    public string Key => "Optimize";
    public string Title => "Optimize";
    public Control View => _view;
    public Task ActivateAsync(CancellationToken cancellationToken = default)
    {
        _active = true;
        return RefreshAsync(cancellationToken);
    }
    public void FocusPrimarySearch() => _view.FocusSearch();

    public async Task RefreshAsync(CancellationToken cancellationToken = default)
    {
        _view.SetBusy(true, "Checking current setting state…");
        var outcome = await _operations.RunLatestAsync("workspace.optimize.refresh", Key, "setting-state refresh",
            token => Task.Run(() => _catalog.ToDictionary(x => x.Id, _executor.IsApplied), token), cancellationToken);
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Refresh failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }
        foreach (var tweak in _catalog) tweak.IsApplied = outcome.Value[tweak.Id];
        _view.RefreshRows();
        _view.SetBusy(false, $"{_catalog.Count:N0} settings available · selected changes are checked before confirmation");
    }

    public void Deactivate()
    {
        _active = false;
        _operations.Cancel("workspace.optimize.refresh");
        _operations.Cancel(MutationOperation);
    }

    public void Dispose()
    {
        Deactivate();
        _view.RunRequested -= View_RunRequested;
        _view.OpenBackupsRequested -= View_OpenBackupsRequested;
    }

    private async void View_RunRequested(object? sender, EventArgs e)
    {
        var selection = _view.Selected;
        if (selection.Count == 0) return;
        _view.SetBusy(true, $"Checking {selection.Count:N0} selected change(s)…");

        var phases = new OptimizeMutationPhases(_executor, _elevation, selection, progress: (phase, count) =>
            _view.SetBusy(true, phase switch
            {
                OptimizeMutationPhaseProgress.RequestingAdministratorPermission =>
                    $"Waiting for Windows administrator permission for {count:N0} change(s)…",
                _ => $"Applying {count:N0} current-user change(s)…"
            }));
        var outcome = await _operations.RunCommittedAsync(MutationOperation, Key, "Optimize mutation workflow",
            token => _workflow.RunAsync(selection, _settings.OfferSystemRestorePoint, _view, phases, token));
        if (outcome.Cancelled)
        {
            if (_active) _view.SetBusy(false, "Optimize operation cancelled.");
            return;
        }
        if (!outcome.Succeeded || outcome.Value is null)
        {
            PublishLog(phases.CombinedLog);
            _view.SetBusy(false, $"Operation failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }

        var result = outcome.Value;
        if (result.Cancelled)
        {
            PublishLog(phases.CombinedLog);
            _activity.Info(Key, "Optimize changes cancelled", result.Summary);
            _view.SetBusy(false, result.Summary);
            return;
        }
        if (!result.Succeeded)
        {
            PublishLog(phases.CombinedLog);
            _activity.Warning(Key, "Optimize changes incomplete", result.Summary, persist: true);
            _view.SetBusy(false, result.Summary);
            return;
        }

        PublishResult("Apply complete", phases.CombinedLog, result.Summary);
        await RefreshAsync();
    }

    private void View_OpenBackupsRequested(object? sender, EventArgs e) => _navigator.NavigateTo("Recovery");

    private void PublishResult(string summary, IReadOnlyList<string> log, string detail)
    {
        _activity.Info(Key, summary, detail, persist: true);
        PublishLog(log);
        _view.SetBusy(false, $"{summary} · {detail}");
    }

    private void PublishLog(IReadOnlyList<string> log)
    {
        foreach (var line in log)
            _activity.Publish(ActivityEvent.Create(Key, line, ActivitySeverity.Trace));
    }
}
