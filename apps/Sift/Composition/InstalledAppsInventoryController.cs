using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure.Interop;
using Sift.WinUI.Views;

namespace Sift.WinUI.Composition;

/// <summary>Owns installed-app inventory, selected-item trust inspection, and Settings handoff.</summary>
internal sealed class InstalledAppsInventoryController : IDisposable
{
    private const string RefreshOperation = "workspace.apps.refresh";
    private const string TrustOperation = "workspace.apps.trust";
    private const string Title = "Installed apps";
    private readonly IInstalledAppInventory _inventory;
    private readonly IInstalledAppTrustInspector _trust;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly IWindowsShellLauncher _shellLauncher;
    private readonly InstalledAppsWorkspaceView _view;
    private readonly InstalledAppUninstallState _uninstallState;
    private bool _disposed;

    public InstalledAppsInventoryController(IInstalledAppInventory inventory, IInstalledAppTrustInspector trust,
        OperationCoordinator operations, ActivityHub activity, IWindowsShellLauncher shellLauncher,
        InstalledAppsWorkspaceView view, InstalledAppUninstallState uninstallState)
    {
        _inventory = inventory;
        _trust = trust;
        _operations = operations;
        _activity = activity;
        _shellLauncher = shellLauncher;
        _view = view;
        _uninstallState = uninstallState;
        _view.RefreshRequested += View_RefreshRequested;
        _view.OpenSettingsRequested += View_OpenSettingsRequested;
        _view.SelectedAppChanged += View_SelectedAppChanged;
    }

    public async Task RefreshAsync(CancellationToken cancellationToken = default)
    {
        _view.SetBusy(true, "Reading registered desktop applications…");
        var outcome = await _operations.RunLatestAsync(RefreshOperation, Title, "installed-app inventory refresh",
            token => Task.Run(() => _inventory.Enumerate(token), token), cancellationToken);
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Refresh failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }

        var eligible = outcome.Value.Count(app => app.CanUninstall);
        var leftovers = outcome.Value.Count(app => app.IsOrphanedRegistration);
        var status = $"{outcome.Value.Count:N0} registered desktop apps · {eligible:N0} uninstallable · {leftovers:N0} potential leftover registration(s)";
        _view.Bind(outcome.Value, status);
        _view.SetRecentUninstall(_uninstallState.Target, _uninstallState.Status, _uninstallState.CleanupAuthorized);
        _view.SetBusy(false, status);
    }

    public void Deactivate()
    {
        _operations.Cancel(RefreshOperation);
        _operations.Cancel(TrustOperation);
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        Deactivate();
        _view.RefreshRequested -= View_RefreshRequested;
        _view.OpenSettingsRequested -= View_OpenSettingsRequested;
        _view.SelectedAppChanged -= View_SelectedAppChanged;
    }

    private async void View_RefreshRequested(object? sender, EventArgs e) => await RefreshAsync();

    private async void View_SelectedAppChanged(object? sender, EventArgs e)
    {
        var app = _view.SelectedApp;
        if (app is null || app.IsOrphanedRegistration) return;
        _view.SetTrustLoading(app);
        var outcome = await _operations.RunLatestAsync(TrustOperation, Title, "selected uninstaller trust inspection",
            token => Task.Run(() => _trust.Inspect(app, token), token));
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetTrustReport(app, new InstalledAppTrustReport(InstalledAppSignatureStatus.Error,
                "Signature inspection failed", outcome.Error?.Message ?? "Unknown inspection error.",
                string.Empty, string.Empty, string.Empty, string.Empty, string.Empty,
                InstalledAppPublisherMatch.NotAvailable, string.Empty));
            return;
        }
        _view.SetTrustReport(app, outcome.Value);
    }

    private void View_OpenSettingsRequested(object? sender, EventArgs e)
    {
        try
        {
            _shellLauncher.OpenSettings(WindowsSettingsPage.InstalledApps);
            _activity.Info(Title, "Opened Windows Installed Apps");
        }
        catch (Exception exception)
        {
            _activity.Error(Title, "Could not open Windows Installed Apps", exception.Message);
            _view.SetStatus($"Could not open Windows Installed Apps: {exception.Message}");
        }
    }
}
