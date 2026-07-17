using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Infrastructure.Settings;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Views;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public sealed class MaintenanceWorkspaceModule : IWorkspaceModule
{
    private readonly IMaintenanceScanner _scanner;
    private readonly IMaintenanceCleaner _cleaner;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly AppSettings _settings;
    private readonly SettingsPersistenceCoordinator _persistence;
    private readonly MaintenanceWorkspaceView _view = new();

    public MaintenanceWorkspaceModule(IMaintenanceScanner scanner, IMaintenanceCleaner cleaner,
        OperationCoordinator operations, ActivityHub activity, AppSettings settings,
        SettingsPersistenceCoordinator persistence)
    {
        _scanner = scanner;
        _cleaner = cleaner;
        _operations = operations;
        _activity = activity;
        _settings = settings;
        _persistence = persistence;
        _view.ScanRequested += View_ScanRequested;
        _view.CleanRequested += View_CleanRequested;
    }

    public string Key => "Maintenance";
    public string Title => "Maintenance";
    public Control View => _view;
    public Task ActivateAsync(CancellationToken cancellationToken = default) => RefreshAsync(cancellationToken);
    public void FocusPrimarySearch() => _view.FocusSearch();

    public async Task RefreshAsync(CancellationToken cancellationToken = default)
    {
        _view.SetBusy(true, "Scanning maintenance locations…");
        var outcome = await _operations.RunLatestAsync("workspace.maintenance.scan", Key, "maintenance scan",
            token => Task.Run(() => _scanner.Scan(), token), cancellationToken);
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Scan failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }
        _view.Bind(outcome.Value);
        _settings.LastMaintenanceScanUtc = DateTime.UtcNow.ToString("O");
        _persistence.SaveNow(_settings);
        var administratorNote = _scanner.DeliveryOptimizationSkippedForElevation || _scanner.PrefetchSkippedForElevation
            ? " · administrator-only locations are not included"
            : string.Empty;
        _view.SetBusy(false, $"{outcome.Value.Count:N0} item(s){administratorNote}");
    }

    public void Deactivate()
    {
        _operations.Cancel("workspace.maintenance.scan");
    }

    public void Dispose()
    {
        Deactivate();
        _view.ScanRequested -= View_ScanRequested;
        _view.CleanRequested -= View_CleanRequested;
    }

    private async void View_ScanRequested(object? sender, EventArgs e) => await RefreshAsync();

    private async void View_CleanRequested(object? sender, EventArgs e)
    {
        var selection = _view.Selected;
        if (selection.Count == 0) return;
        _view.SetBusy(true, $"Checking {selection.Count:N0} selected item(s)…");
        var reviewOutcome = await _operations.RunLatestAsync("workspace.maintenance.clean", Key,
            "maintenance review", token => _cleaner.ReviewAsync(selection, token));
        if (reviewOutcome.Cancelled) return;
        if (!reviewOutcome.Succeeded || reviewOutcome.Value is null)
        {
            _view.SetBusy(false, $"Could not check the selection: {reviewOutcome.Error?.Message ?? "unknown error"}");
            return;
        }
        var review = reviewOutcome.Value;
        var checkedSelection = review.Result;
        foreach (var line in checkedSelection.Log) _activity.Publish(ActivityEvent.Create(Key, line, ActivitySeverity.Trace));
        if (!review.CanExecute || checkedSelection.Previewed != selection.Count ||
            checkedSelection.Skipped > 0 || checkedSelection.Failed > 0)
        {
            var blocked = $"Could not clean this selection: checked {checkedSelection.Previewed:N0} of {selection.Count:N0}; " +
                          $"skipped {checkedSelection.Skipped:N0}; failed {checkedSelection.Failed:N0}.";
            _view.SetBusy(false, blocked);
            _activity.Warning(Key, "Maintenance selection unavailable", blocked);
            return;
        }
        _view.SetBusy(false, $"Checked {checkedSelection.Previewed:N0} item(s)");
        if (!await _view.ConfirmCleanAsync(selection, checkedSelection))
        {
            _cleaner.Discard(review.TicketId!);
            _view.SetBusy(false, "Cleanup cancelled");
            _activity.Info(Key, "Maintenance cleanup cancelled", "The confirmation dialog was cancelled.");
            return;
        }

        _view.SetBusy(true, "Checking the selected contents again…");
        var outcome = await _operations.RunCommittedAsync("workspace.maintenance.clean", Key,
            "maintenance cleanup", token => _cleaner.ExecuteAsync(review.TicketId!, token));
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Cleanup failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }
        if (outcome.Value.Status == MaintenanceCleanupStatus.Invalidated)
        {
            var changed = outcome.Value.Log.FirstOrDefault() ?? "The selected contents changed. Scan and review them again.";
            _view.SetBusy(false, changed.Replace("CHANGED  ", string.Empty, StringComparison.Ordinal));
            _activity.Warning(Key, "Maintenance selection changed", changed);
            return;
        }
        var detail = $"Cleaned {outcome.Value.Cleaned}; skipped {outcome.Value.Skipped}; failed {outcome.Value.Failed}";
        _activity.Info(Key, "Cleanup complete", detail, persist: true);
        foreach (var line in outcome.Value.Log) _activity.Publish(ActivityEvent.Create(Key, line, ActivitySeverity.Trace));
        await RefreshAsync();
    }
}
