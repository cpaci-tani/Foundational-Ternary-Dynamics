using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Services;
using Sift.WinUI.Views;
using Sift.WinUI.Infrastructure.Interop;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public sealed class RecoveryWorkspaceModule : IWorkspaceModule
{
    private const string RefreshOperation = "workspace.recovery.refresh";
    private const string RestoreOperation = "workspace.recovery.restore";
    private readonly IRecoveryManager _manager;
    private readonly ITweakExecutor _executor;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly IWindowsShellLauncher _shellLauncher;
    private readonly RecoveryWorkspaceView _view = new();

    public RecoveryWorkspaceModule(IRecoveryManager manager, ITweakExecutor executor,
        OperationCoordinator operations, ActivityHub activity, IWindowsShellLauncher shellLauncher)
    {
        _manager = manager;
        _executor = executor;
        _operations = operations;
        _activity = activity;
        _shellLauncher = shellLauncher;
        _view.RefreshRequested += View_RefreshRequested;
        _view.OpenFolderRequested += View_OpenFolderRequested;
        _view.RestoreRequested += View_RestoreRequested;
    }

    public string Key => "Recovery";
    public string Title => "Recovery";
    public Control View => _view;
    public Task ActivateAsync(CancellationToken cancellationToken = default) => RefreshAsync(cancellationToken);
    public void FocusPrimarySearch() => _view.FocusSearch();

    public async Task RefreshAsync(CancellationToken cancellationToken = default)
    {
        _view.SetBusy(true, "Loading backups…");
        var outcome = await _operations.RunLatestAsync(RefreshOperation, Title, "backup inventory",
            token => Task.Run(_manager.ListBackups, token), cancellationToken);
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Recovery inventory failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }
        _view.Bind(outcome.Value);
        _view.SetBusy(false, $"{outcome.Value.Count:N0} backup(s) available");
    }

    public void Deactivate()
    {
        _operations.Cancel(RefreshOperation);
    }

    public void Dispose()
    {
        Deactivate();
        _view.RefreshRequested -= View_RefreshRequested;
        _view.OpenFolderRequested -= View_OpenFolderRequested;
        _view.RestoreRequested -= View_RestoreRequested;
    }

    private async void View_RefreshRequested(object? sender, EventArgs e) => await RefreshAsync();

    private void View_OpenFolderRequested(object? sender, EventArgs e)
    {
        Directory.CreateDirectory(_executor.BackupDirectory);
        _shellLauncher.OpenFolder(_executor.BackupDirectory);
        _activity.Info(Title, "Opened Sift backup folder", _executor.BackupDirectory);
    }

    private async void View_RestoreRequested(object? sender, EventArgs e)
    {
        var selected = _view.SelectedBackup;
        if (selected is null) return;
        _view.SetBusy(true, $"Checking {selected.FileName}…");
        var preflight = await _operations.RunLatestAsync(RestoreOperation, Title, "backup check",
            token => Task.Run(() => _manager.InspectExact(selected.Path), token));
        if (preflight.Cancelled) return;
        if (!preflight.Succeeded || preflight.Value is null || !preflight.Value.CanRestore)
        {
            _view.SetBusy(false, $"Restore unavailable: {preflight.Value?.Detail ?? preflight.Error?.Message ?? "unknown error"}");
            return;
        }
        _view.SetBusy(false, $"{preflight.Value.PendingCount:N0} entr{(preflight.Value.PendingCount == 1 ? "y" : "ies")} ready to restore");
        if (!await _view.ConfirmRestoreAsync(preflight.Value))
        {
            _activity.Info(Title, "Backup restore cancelled", preflight.Value.FileName);
            return;
        }

        _view.SetBusy(true, $"Restoring {preflight.Value.FileName}…");
        var result = await _operations.RunCommittedAsync(RestoreOperation, Title, "confirmed backup restore",
            token => _manager.RestoreAsync(preflight.Value.Path, token));
        if (result.Cancelled) return;
        if (!result.Succeeded || result.Value is null)
        {
            _view.SetBusy(false, $"Restore failed: {result.Error?.Message ?? "unknown error"}");
            return;
        }
        if (result.Value.Cancelled)
        {
            _activity.Info(Title, "Backup restore cancelled", preflight.Value.FileName);
            foreach (var line in result.Value.Log)
                _activity.Publish(ActivityEvent.Create(Title, line, ActivitySeverity.Trace));
            _view.SetBusy(false, result.Value.Message);
            return;
        }
        var severity = result.Value.Succeeded ? ActivitySeverity.Info : ActivitySeverity.Warning;
        _activity.Publish(ActivityEvent.Create(Title, result.Value.Message, severity,
            $"Restored {result.Value.Restored}; skipped {result.Value.Skipped}; failed {result.Value.Failed}",
            preflight.Value.Path, persist: true));
        foreach (var line in result.Value.Log)
            _activity.Publish(ActivityEvent.Create(Title, line, ActivitySeverity.Trace));
        await RefreshAsync();
        _view.SetBusy(false, result.Value.Message);
    }
}
