using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Settings;
using Sift.Models;
using Sift.WinUI.Views;
using Microsoft.UI.Xaml.Controls;
using Sift.Services;
using Sift.WinUI.Infrastructure.Monitoring;

namespace Sift.WinUI.Composition;

public sealed class SettingsWorkspaceModule : IWorkspaceModule, IShellSettingsChangeSource
{
    private readonly AppSettings _settings;
    private readonly SettingsPersistenceCoordinator _persistence;
    private readonly ActivityHub _activity;
    private readonly IDashboardMonitorController _monitor;
    private readonly IDashboardHistoryStore _dashboardHistory;
    private readonly SettingsWorkspaceView _view = new();

    public SettingsWorkspaceModule(
        AppSettings settings,
        SettingsPersistenceCoordinator persistence,
        ActivityHub activity,
        IDashboardMonitorController monitor,
        IDashboardHistoryStore dashboardHistory)
    {
        _settings = settings;
        _persistence = persistence;
        _activity = activity;
        _monitor = monitor;
        _dashboardHistory = dashboardHistory;
        _view.Bind(settings);
        _view.SettingChanged += View_SettingChanged;
        _view.MonitorCommandRequested += View_MonitorCommandRequested;
        _view.ClearDashboardHistoryRequested += View_ClearDashboardHistoryRequested;
    }

    public string Key => "Settings";
    public string Title => "Settings";
    public Control View => _view;
    public event EventHandler? ShellSettingsChanged;

    public async Task ActivateAsync(CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        _view.Bind(_settings);
        if (_settings.Dashboard.MonitorWhenClosed) await _monitor.EnsureRunningAsync(cancellationToken);
        await UpdateMonitorStatusAsync(cancellationToken);
    }

    public Task RefreshAsync(CancellationToken cancellationToken = default) => ActivateAsync(cancellationToken);
    public void Deactivate() { }
    public void FocusPrimarySearch() => _view.FocusPrimaryControl();

    public void Dispose()
    {
        _view.SettingChanged -= View_SettingChanged;
        _view.MonitorCommandRequested -= View_MonitorCommandRequested;
        _view.ClearDashboardHistoryRequested -= View_ClearDashboardHistoryRequested;
    }

    private async void View_SettingChanged(object? sender, SettingChangedEventArgs e)
    {
        if (e.Name == "background monitoring") _persistence.SaveNow(_settings);
        else _persistence.Schedule(_settings);
        _activity.Publish(ActivityEvent.Create("Settings", $"Changed {e.Name}", ActivitySeverity.Trace));
        ShellSettingsChanged?.Invoke(this, EventArgs.Empty);
        try
        {
            if (e.Name == "background monitoring")
                await _monitor.SetStartupEnabledAsync(_settings.Dashboard.MonitorWhenClosed);
            else if (e.Name.StartsWith("dashboard", StringComparison.OrdinalIgnoreCase) ||
                     e.Name == "background hardware sensors")
            {
                _persistence.SaveNow(_settings);
                await _monitor.ReloadPreferencesAsync();
            }
            await UpdateMonitorStatusAsync();
        }
        catch (Exception exception)
        {
            _settings.Dashboard.MonitorWhenClosed = false;
            _persistence.SaveNow(_settings);
            _view.Bind(_settings);
            _view.SetMonitorState(exception.Message);
            _activity.Error(Key, "Could not update lifecycle monitor", exception.Message);
        }
    }

    private async void View_MonitorCommandRequested(object? sender, MonitorCommandEventArgs e)
    {
        try
        {
            if (e.Command == "pause") await _monitor.PauseAsync();
            else await _monitor.ResumeAsync();
            await UpdateMonitorStatusAsync();
        }
        catch (Exception exception) { _view.SetMonitorState(exception.Message); }
    }

    private async void View_ClearDashboardHistoryRequested(object? sender, EventArgs e)
    {
        try
        {
            if (!await _monitor.ClearHistoryAsync()) await _dashboardHistory.ClearAsync();
            _view.SetMonitorState("Local dashboard history and alerts were cleared.");
            _activity.Info(Key, "Cleared dashboard history");
        }
        catch (Exception exception) { _view.SetMonitorState($"Could not clear history: {exception.Message}"); }
    }

    private async Task UpdateMonitorStatusAsync(CancellationToken cancellationToken = default)
    {
        var state = await _monitor.GetStateAsync(cancellationToken);
        var mode = state.Packaged ? "packaged startup task" : "current-user Startup entry";
        var running = state.Running ? (state.Paused ? "paused" : "running") : "stopped";
        _view.SetMonitorState($"{running} · startup {(state.StartupEnabled ? "enabled" : "disabled")} via {mode}");
    }
}
