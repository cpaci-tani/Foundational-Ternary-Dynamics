using Microsoft.UI.Xaml.Controls;
using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure.Interop;
using Sift.WinUI.Views;

namespace Sift.WinUI.Composition;

/// <summary>
/// Lifetime boundary for the Installed Apps vertical slice. Inventory presentation,
/// uninstall tracking, and leftover cleanup are owned by dedicated controllers.
/// </summary>
public sealed class InstalledAppsWorkspaceModule : IWorkspaceModule
{
    private readonly InstalledAppsWorkspaceView _view = new();
    private readonly InstalledAppsInventoryController _inventory;
    private readonly InstalledAppUninstallController _uninstall;
    private readonly InstalledAppLeftoverController _leftovers;
    private bool _disposed;

    public InstalledAppsWorkspaceModule(IInstalledAppInventory inventory, IInstalledAppManager manager,
        IAppLeftoverManager leftovers, IInstalledAppTrustInspector trust,
        OperationCoordinator operations, ActivityHub activity, IWindowsShellLauncher shellLauncher)
    {
        var uninstallState = new InstalledAppUninstallState();
        _inventory = new InstalledAppsInventoryController(
            inventory, trust, operations, activity, shellLauncher, _view, uninstallState);
        _uninstall = new InstalledAppUninstallController(
            manager, operations, activity, _view, uninstallState, _inventory.RefreshAsync);
        _leftovers = new InstalledAppLeftoverController(
            leftovers, _uninstall, operations, activity, _view);
    }

    public string Key => "Apps";
    public string Title => "Installed apps";
    public Control View => _view;

    public async Task ActivateAsync(CancellationToken cancellationToken = default)
    {
        await RefreshAsync(cancellationToken);
        _uninstall.Activate();
    }

    public Task RefreshAsync(CancellationToken cancellationToken = default) =>
        _inventory.RefreshAsync(cancellationToken);

    public void FocusPrimarySearch() => _view.FocusSearch();

    public void Deactivate()
    {
        _inventory.Deactivate();
        _uninstall.Deactivate();
        _leftovers.Deactivate();
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        _leftovers.Dispose();
        _uninstall.Dispose();
        _inventory.Dispose();
        _view.ReleaseSubscriptions();
    }
}
