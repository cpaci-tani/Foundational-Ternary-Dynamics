using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Views;

namespace Sift.WinUI.Composition;

/// <summary>Owns uninstall/registration actions and the verified uninstall continuation lifetime.</summary>
internal sealed class InstalledAppUninstallController : IDisposable
{
    private const string UninstallOperation = "workspace.apps.uninstall";
    private const string UninstallMonitorOperation = "workspace.apps.uninstall.monitor";
    private const string UninstallCheckOperation = "workspace.apps.uninstall.check";
    private const string Title = "Installed apps";
    private readonly IInstalledAppManager _manager;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly InstalledAppsWorkspaceView _view;
    private readonly Func<CancellationToken, Task> _refreshInventory;
    private readonly InstalledAppUninstallState _state;
    private bool _disposed;

    public InstalledAppUninstallController(IInstalledAppManager manager, OperationCoordinator operations,
        ActivityHub activity, InstalledAppsWorkspaceView view, InstalledAppUninstallState state,
        Func<CancellationToken, Task> refreshInventory)
    {
        _manager = manager;
        _operations = operations;
        _activity = activity;
        _view = view;
        _state = state;
        _refreshInventory = refreshInventory;
        _view.ActionRequested += View_ActionRequested;
    }

    public void Activate()
    {
        if (_state.HasPendingSession)
            _ = MonitorUninstallAsync(_state.Target!, _state.SessionId!);
    }

    public string? ContinuationFor(InstalledApp app) => _state.ContinuationFor(app);

    public async Task<bool> EnsureCleanupAuthorizationAsync(InstalledApp app)
    {
        if (!_state.Matches(app) || _state.CleanupAuthorized) return true;
        var sessionId = _state.SessionId;
        if (string.IsNullOrWhiteSpace(sessionId))
        {
            _view.SetStatus("Uninstall tracking is unavailable. Refresh and start the uninstall again.");
            return false;
        }

        var check = await _operations.RunLatestAsync(UninstallCheckOperation, Title, "uninstall completion check",
            token => _manager.CheckUninstallCompletionAsync(app, sessionId, token));
        if (check.Cancelled) return false;
        if (!check.Succeeded || check.Value is null)
        {
            _view.SetStatus($"Uninstall check failed: {check.Error?.Message ?? "unknown error"}");
            return false;
        }
        await ApplyUninstallCompletionAsync(app, check.Value);
        return check.Value.Completed && !check.Value.Blocked;
    }

    public void Deactivate()
    {
        _operations.Cancel(UninstallOperation);
        _operations.Cancel(UninstallMonitorOperation);
        _operations.Cancel(UninstallCheckOperation);
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        Deactivate();
        _view.ActionRequested -= View_ActionRequested;
    }

    private async void View_ActionRequested(object? sender, EventArgs e)
    {
        var app = _view.SelectedApp;
        if (app is null) return;
        var cleanup = app.IsOrphanedRegistration;
        if (cleanup ? !app.CanCleanRegistration : !app.CanUninstall) return;

        _view.SetBusy(true, $"Checking {app.DisplayName}…");
        var previewOutcome = await _operations.RunLatestAsync(UninstallOperation, Title,
            cleanup ? "leftover-registration check" : "uninstall check",
            token => cleanup
                ? _manager.CleanupRegistrationAsync(app, preview: true, token)
                : _manager.UninstallAsync(app, preview: true, token));
        if (previewOutcome.Cancelled) return;
        if (!previewOutcome.Succeeded || previewOutcome.Value is null)
        {
            _view.SetBusy(false, $"Could not check {app.DisplayName}: {previewOutcome.Error?.Message ?? "unknown error"}");
            return;
        }
        var preview = previewOutcome.Value;
        if (preview.Blocked || !preview.Previewed)
        {
            _view.SetBusy(false, preview.Message);
            _activity.Warning(Title, cleanup ? "Registration cleanup unavailable" : "Uninstall unavailable", preview.Message, persist: true);
            return;
        }
        _view.SetBusy(false, preview.Message);
        var confirmed = cleanup
            ? await _view.ConfirmRegistrationCleanupAsync(app, preview.Message)
            : await _view.ConfirmUninstallAsync(app, preview.Message);
        if (!confirmed)
        {
            _activity.Info(Title, cleanup ? "Registration cleanup cancelled" : "Uninstall cancelled", app.DisplayName);
            return;
        }

        _view.SetBusy(true, $"Checking {app.DisplayName} again…");
        var outcome = await _operations.RunCommittedAsync(UninstallOperation, Title,
            cleanup ? "leftover-registration cleanup" : "registered uninstaller launch",
            token => cleanup
                ? _manager.CleanupRegistrationAsync(app, preview: false, token)
                : _manager.UninstallAsync(app, preview: false, token));
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Action failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }

        var result = outcome.Value;
        if (result.Blocked)
            _activity.Warning(Title, cleanup ? "Leftover cleanup blocked" : "Uninstall handoff blocked", result.Message, persist: true);
        else
            _activity.Info(Title, cleanup ? "Leftover registration removed" : "Registered uninstaller opened", result.Message, persist: true);

        var continuationToken = result.ContinuationToken;
        var uninstallSessionId = result.UninstallSessionId;
        if (cleanup && result.Executed && !string.IsNullOrWhiteSpace(continuationToken))
        {
            _state.AuthorizeCleanup(app, continuationToken, result.Message);
            _view.SetRecentUninstall(app, _state.Status, cleanupAuthorized: true);
        }
        else if (!cleanup && result.Executed && !string.IsNullOrWhiteSpace(uninstallSessionId))
        {
            _state.TrackUninstaller(app, uninstallSessionId, result.Message);
            _view.SetRecentUninstall(app, _state.Status, cleanupAuthorized: false);
            _ = MonitorUninstallAsync(app, uninstallSessionId);
        }

        if (cleanup && !result.Blocked) await _refreshInventory(CancellationToken.None);
        else _view.SetBusy(false, result.Message);
    }

    private async Task MonitorUninstallAsync(InstalledApp app, string sessionId)
    {
        var outcome = await _operations.RunLatestAsync(UninstallMonitorOperation, Title, "uninstaller completion monitor",
            token => _manager.WaitForUninstallCompletionAsync(app, sessionId, token));
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _state.UpdateStatus($"Could not monitor the uninstaller: {outcome.Error?.Message ?? "unknown error"}. Use Check uninstall status.");
            _view.SetRecentUninstall(app, _state.Status, cleanupAuthorized: false);
            _activity.Warning(Title, "Uninstaller monitor failed", _state.Status);
            return;
        }
        await ApplyUninstallCompletionAsync(app, outcome.Value);
    }

    private async Task ApplyUninstallCompletionAsync(InstalledApp app, InstalledAppUninstallCompletion completion)
    {
        _state.UpdateStatus(completion.Message);
        if (completion.Blocked)
        {
            _activity.Warning(Title, "Uninstall verification blocked", completion.Message, persist: true);
            _state.Clear();
            _view.SetRecentUninstall(null);
            _view.SetStatus(completion.Message);
            return;
        }

        if (!completion.Completed)
        {
            _view.SetRecentUninstall(app, completion.Message, cleanupAuthorized: false);
            _view.SetStatus(completion.Message);
            _activity.Info(Title, "Uninstall not completed", completion.Message);
            return;
        }

        var continuationToken = completion.ContinuationToken;
        if (string.IsNullOrWhiteSpace(continuationToken))
        {
            _state.Clear();
            _view.SetRecentUninstall(null);
            _view.SetStatus("The app was removed, but file cleanup is unavailable. Refresh and scan again.");
            return;
        }

        _state.AuthorizeCleanup(app, continuationToken, completion.Message);
        _view.SetRecentUninstall(app, completion.Message, cleanupAuthorized: true);
        _activity.Info(Title, "Uninstall removal verified", completion.Message, persist: true);
        await _refreshInventory(CancellationToken.None);
    }
}
