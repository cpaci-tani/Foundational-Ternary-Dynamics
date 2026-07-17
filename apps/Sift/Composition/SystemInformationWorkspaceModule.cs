using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Services;
using Sift.WinUI.Views;
using Microsoft.UI.Xaml.Controls;
using Sift.WinUI.Infrastructure.Interop;

namespace Sift.WinUI.Composition;

public sealed class SystemInformationWorkspaceModule : IWorkspaceModule
{
    private const string OperationKey = "workspace.system-information";
    private readonly ISystemInformationService _systemInformation;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly IWindowsShellLauncher _shellLauncher;
    private readonly SystemInformationWorkspaceView _view;

    public SystemInformationWorkspaceModule(
        ISystemInformationService systemInformation,
        OperationCoordinator operations,
        ActivityHub activity,
        IClipboardService clipboard,
        IWindowsShellLauncher shellLauncher)
    {
        _systemInformation = systemInformation;
        _operations = operations;
        _activity = activity;
        _shellLauncher = shellLauncher;
        _view = new SystemInformationWorkspaceView(clipboard);
        _view.RefreshRequested += View_RefreshRequested;
        _view.OpenMsInfoRequested += View_OpenMsInfoRequested;
        _view.ReportCopied += View_ReportCopied;
        _view.PropertyCopied += View_PropertyCopied;
    }

    public string Key => "SystemInfo";
    public string Title => "System information";
    public Control View => _view;
    public Task ActivateAsync(CancellationToken cancellationToken = default) => RefreshAsync(cancellationToken);
    public void FocusPrimarySearch() => _view.FocusSearch();
    public void Deactivate() => _operations.Cancel(OperationKey);

    public async Task RefreshAsync(CancellationToken cancellationToken = default)
    {
        _view.SetBusy(true, "Preparing local Windows inventory…");
        var progress = new Progress<string>(_view.SetProgress);
        var outcome = await _operations.RunLatestAsync(OperationKey, Title, "system information refresh",
            token => Task.Run(() => _systemInformation.Collect(progress, token), token), cancellationToken);
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            var failure = $"Refresh failed: {outcome.Error?.Message ?? "unknown error"}";
            _view.SetBusy(false, failure);
            _activity.Error(Title, "System information refresh failed", failure);
            return;
        }

        var report = outcome.Value;
        _view.Bind(report);
        var status = report.Warnings.Count == 0
            ? $"Loaded {report.Items.Count:N0} system properties across {report.Categories.Count:N0} categories."
            : $"Collected {report.Items.Count:N0} properties with {report.Warnings.Count:N0} optional provider warning(s).";
        _view.SetBusy(false, status);
        _activity.Info(Title, "System information refreshed", status);
        foreach (var warning in report.Warnings)
            _activity.Warning(Title, "Optional system-information provider unavailable", warning);
    }

    public void Dispose()
    {
        Deactivate();
        _view.RefreshRequested -= View_RefreshRequested;
        _view.OpenMsInfoRequested -= View_OpenMsInfoRequested;
        _view.ReportCopied -= View_ReportCopied;
        _view.PropertyCopied -= View_PropertyCopied;
    }

    private async void View_RefreshRequested(object? sender, EventArgs e) => await RefreshAsync();

    private void View_OpenMsInfoRequested(object? sender, EventArgs e)
    {
        try
        {
            _shellLauncher.OpenSystemInformation();
            _activity.Info(Title, "Opened Windows System Information");
        }
        catch (Exception exception)
        {
            _activity.Error(Title, "Could not open Windows System Information", exception.Message);
        }
    }

    private void View_ReportCopied(object? sender, EventArgs e) =>
        _activity.Info(Title, "Copied visible system information", "The report may contain identifiers; the UI reminds the user to review it before sharing.");

    private void View_PropertyCopied(object? sender, EventArgs e) =>
        _activity.Info(Title, "Copied one system-information property", "The property may contain an identifier; the UI reminds the user to review it before sharing.");
}
