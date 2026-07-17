using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Services;
using Sift.WinUI.Views;
using Sift.WinUI.Infrastructure.Interop;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public sealed class StartupWorkspaceModule : IWorkspaceModule
{
    private readonly IStartupInventory _inventory;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly IWindowsShellLauncher _shellLauncher;
    private readonly StartupWorkspaceView _view = new();

    public StartupWorkspaceModule(IStartupInventory inventory, OperationCoordinator operations,
        ActivityHub activity, IWindowsShellLauncher shellLauncher)
    {
        _inventory = inventory;
        _operations = operations;
        _activity = activity;
        _shellLauncher = shellLauncher;
        _view.RefreshRequested += View_RefreshRequested;
        _view.OpenSettingsRequested += View_OpenSettingsRequested;
    }

    public string Key => "Startup";
    public string Title => "Startup apps";
    public Control View => _view;

    public void FocusPrimarySearch() => _view.FocusSearch();
    public Task ActivateAsync(CancellationToken cancellationToken = default) => RefreshAsync(cancellationToken);
    public void Deactivate() => _operations.Cancel("workspace.startup");
    public void Dispose()
    {
        Deactivate();
        _view.RefreshRequested -= View_RefreshRequested;
        _view.OpenSettingsRequested -= View_OpenSettingsRequested;
    }

    public async Task RefreshAsync(CancellationToken cancellationToken = default)
    {
        _view.SetBusy(true, "Loading startup apps…");
        var outcome = await _operations.RunLatestAsync(
            "workspace.startup",
            Key,
            "startup inventory refresh",
            token => Task.Run(_inventory.Enumerate, token),
            cancellationToken);

        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Refresh failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }

        _view.Bind(outcome.Value, $"{outcome.Value.Count:N0} startup entries · updated {DateTime.Now:T}");
        _view.SetBusy(false, $"{outcome.Value.Count:N0} startup entries · updated {DateTime.Now:T}");
    }

    private void OpenSettings()
    {
        try
        {
            _shellLauncher.OpenSettings(WindowsSettingsPage.StartupApps);
            _activity.Info(Key, "Opened Windows Startup settings");
        }
        catch (Exception exception)
        {
            _activity.Error(Key, $"Could not open Windows Startup settings: {exception.Message}");
        }
    }

    private async void View_RefreshRequested(object? sender, EventArgs e) => await RefreshAsync();
    private void View_OpenSettingsRequested(object? sender, EventArgs e) => OpenSettings();
}
