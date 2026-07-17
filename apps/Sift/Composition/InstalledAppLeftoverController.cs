using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Views;

namespace Sift.WinUI.Composition;

/// <summary>Owns exact AppData candidate review and Recycle Bin execution.</summary>
internal sealed class InstalledAppLeftoverController : IDisposable
{
    private const string ScanOperation = "workspace.apps.leftovers.scan";
    private const string DeleteOperation = "workspace.apps.leftovers.delete";
    private const string Title = "Installed apps";
    private readonly IAppLeftoverManager _leftovers;
    private readonly InstalledAppUninstallController _uninstall;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly InstalledAppsWorkspaceView _view;
    private bool _disposed;

    public InstalledAppLeftoverController(IAppLeftoverManager leftovers, InstalledAppUninstallController uninstall,
        OperationCoordinator operations, ActivityHub activity, InstalledAppsWorkspaceView view)
    {
        _leftovers = leftovers;
        _uninstall = uninstall;
        _operations = operations;
        _activity = activity;
        _view = view;
        _view.ScanFileLeftoversRequested += View_ScanFileLeftoversRequested;
        _view.DeleteFileLeftoversRequested += View_DeleteFileLeftoversRequested;
    }

    public void Deactivate()
    {
        _operations.Cancel(ScanOperation);
        _operations.Cancel(DeleteOperation);
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        Deactivate();
        _view.ScanFileLeftoversRequested -= View_ScanFileLeftoversRequested;
        _view.DeleteFileLeftoversRequested -= View_DeleteFileLeftoversRequested;
    }

    private async void View_ScanFileLeftoversRequested(object? sender, EventArgs e)
    {
        var app = _view.LeftoverTarget;
        if (app is null || !await _uninstall.EnsureCleanupAuthorizationAsync(app)) return;
        var continuation = _uninstall.ContinuationFor(app);
        _view.SetBusy(true, $"Checking AppData folders for {app.DisplayName}…");
        var outcome = await _operations.RunLatestAsync(ScanOperation, Title, "app-leftover scan",
            token => Task.Run(() => _leftovers.ScanLeftovers(app, continuation, token), token));
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Leftover scan failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }
        var scan = outcome.Value;
        _view.ShowFileLeftovers(app, scan.Candidates, scan.Message);
        if (scan.Blocked) _activity.Warning(Title, "App-leftover scan blocked", scan.Message);
        else _activity.Info(Title, "App-leftover scan complete", scan.Message);
        _view.SetBusy(false, scan.Message);
    }

    private async void View_DeleteFileLeftoversRequested(object? sender, EventArgs e)
    {
        var app = _view.DisplayedLeftoverTarget;
        var selection = _view.SelectedFileLeftovers;
        if (app is null || selection.Count == 0) return;
        var continuation = _uninstall.ContinuationFor(app);
        _view.SetBusy(true, "Checking selected app leftovers…");
        var previewOutcome = await _operations.RunLatestAsync(DeleteOperation, Title, "app-leftover cleanup check",
            token => Task.Run(() => _leftovers.DeleteLeftovers(app, continuation, selection, preview: true, token), token));
        if (previewOutcome.Cancelled) return;
        if (!previewOutcome.Succeeded || previewOutcome.Value is null)
        {
            _view.SetBusy(false, $"Could not check the selected leftovers: {previewOutcome.Error?.Message ?? "unknown error"}");
            return;
        }
        var preview = previewOutcome.Value;
        foreach (var line in preview.Log) _activity.Publish(ActivityEvent.Create(Title, line, ActivitySeverity.Trace));
        if (preview.Previewed != selection.Count || preview.Skipped > 0 || preview.Failed > 0)
        {
            _view.SetFileLeftoverStatus(preview.Summary + " The selection changed; scan again before continuing.");
            _view.SetBusy(false, preview.Summary);
            _activity.Warning(Title, "App-leftover cleanup unavailable", preview.Summary);
            return;
        }
        _view.SetBusy(false, preview.Summary);
        if (!await _view.ConfirmFileLeftoverDeletionAsync(app, selection, preview))
        {
            _activity.Info(Title, "App-leftover cleanup cancelled", app.DisplayName);
            return;
        }

        _view.SetBusy(true, "Moving selected leftovers to the Recycle Bin…");
        var outcome = await _operations.RunCommittedAsync(DeleteOperation, Title, "app-leftover Recycle Bin cleanup",
            token => Task.Run(() => _leftovers.DeleteLeftovers(app, continuation, selection, preview: false, token), token));
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"File cleanup failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }
        var result = outcome.Value;
        _activity.Info(Title, "App leftovers moved to Recycle Bin", result.Summary, persist: true);
        foreach (var line in result.Log) _activity.Publish(ActivityEvent.Create(Title, line, ActivitySeverity.Trace));
        if (result.Deleted > 0)
        {
            var scan = _leftovers.ScanLeftovers(app, continuation, CancellationToken.None);
            _view.ShowFileLeftovers(app, scan.Candidates, result.Summary + " " + scan.Message);
        }
        else _view.SetFileLeftoverStatus(result.Summary);
        _view.SetBusy(false, result.Summary);
    }
}
