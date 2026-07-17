using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Infrastructure.Settings;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure.Windowing;
using Sift.WinUI.Views;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public sealed class StorageWorkspaceModule : IWorkspaceModule
{
    private const string OperationKey = "workspace.storage.scan";
    private const string DeleteOperationKey = "workspace.storage.delete";
    private readonly IStorageScanner _scanner;
    private readonly IStorageSelectionDeletionManager _deletion;
    private readonly IFolderPickerService _folderPicker;
    private readonly OperationCoordinator _operations;
    private readonly SettingsPersistenceCoordinator _persistence;
    private readonly ActivityHub _activity;
    private readonly AppSettings _settings;
    private readonly StorageWorkspaceView _view = new();
    private StorageTree? _tree;
    private bool _scanning;
    private bool _deleting;

    public StorageWorkspaceModule(IStorageScanner scanner, IStorageSelectionDeletionManager deletion,
        IFolderPickerService folderPicker, OperationCoordinator operations,
        SettingsPersistenceCoordinator persistence, ActivityHub activity, AppSettings settings)
    {
        _scanner = scanner;
        _deletion = deletion;
        _folderPicker = folderPicker;
        _operations = operations;
        _persistence = persistence;
        _activity = activity;
        _settings = settings;
        _view.ConfigureRoots(StorageScanner.ListCandidateRoots(), settings.StorageRoots.FirstOrDefault());
        _view.ScanRequested += View_ScanRequested;
        _view.CancelRequested += View_CancelRequested;
        _view.BrowseRequested += View_BrowseRequested;
        _view.DeleteRequested += View_DeleteRequested;
    }

    public string Key => "Storage";
    public string Title => "Storage map";
    public Control View => _view;

    public Task ActivateAsync(CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (_tree is null) _view.SetIdle("Choose a drive or paste a folder path, then start an explicit scan.");
        else _view.Bind(_tree);
        return Task.CompletedTask;
    }

    public Task RefreshAsync(CancellationToken cancellationToken = default) => ScanAsync(cancellationToken);
    public void FocusPrimarySearch() => _view.FocusRoot();

    public void Deactivate()
    {
        _operations.Cancel(OperationKey);
        _operations.Cancel(DeleteOperationKey);
        _scanning = false;
        _deleting = false;
        _view.SetBusy(false, "Storage operation cancelled when the workspace was closed.");
    }

    public void Dispose()
    {
        Deactivate();
        _view.ScanRequested -= View_ScanRequested;
        _view.CancelRequested -= View_CancelRequested;
        _view.BrowseRequested -= View_BrowseRequested;
        _view.DeleteRequested -= View_DeleteRequested;
    }

    private async Task ScanAsync(CancellationToken cancellationToken = default)
    {
        var root = _view.SelectedRoot;
        if (string.IsNullOrWhiteSpace(root) || (!Directory.Exists(root) && !StorageScanner.IsDriveRoot(root)))
        {
            _view.SetBusy(false, "Choose an existing local folder or drive root.");
            return;
        }

        _scanning = true;
        _view.SetBusy(true, $"Starting scan of {root}…");
        var progress = new Progress<StorageScanProgress>(_view.ReportProgress);
        var outcome = await _operations.RunLatestAsync(OperationKey, Key, "storage scan",
            token => _scanner.ScanAsync([root], progress, token), cancellationToken);
        _scanning = false;
        if (outcome.Cancelled)
        {
            _view.SetBusy(false, "Storage scan cancelled; no partial map was retained.");
            return;
        }
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Storage scan failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }

        _tree = outcome.Value;
        _view.Bind(_tree);
        _view.SetBusy(false, $"Mapped {StorageRow.FormatSize(_tree.TotalSize)} across {_tree.TotalFiles:N0} files in {outcome.Elapsed.TotalSeconds:0.0} s.");
        _settings.StorageRoots = [root];
        _settings.LastStorageScanUtc = DateTime.UtcNow.ToString("O");
        _persistence.Schedule(_settings);
        _activity.Info(Key, "Storage map ready",
            $"{root} · {StorageRow.FormatSize(_tree.TotalSize)} · {_tree.TotalFiles:N0} files");
    }

    private async void View_ScanRequested(object? sender, EventArgs e) => await ScanAsync();

    private async void View_BrowseRequested(object? sender, EventArgs e)
    {
        try
        {
            var path = await _folderPicker.PickFolderAsync();
            if (!string.IsNullOrWhiteSpace(path)) _view.SetSelectedRoot(path);
        }
        catch (Exception exception)
        {
            _view.SetBusy(false, $"Folder picker failed: {exception.Message}");
        }
    }

    private async void View_DeleteRequested(object? sender, EventArgs e)
    {
        if (_tree is null || _view.SelectedNodeIndex < 0) return;
        _deleting = true;
        _view.SetBusy(true, "Completely inventorying the exact selected path…");
        var preflightOutcome = await _operations.RunLatestAsync(DeleteOperationKey, Key,
            "storage deletion check",
            token => _deletion.PreflightAsync(_tree, _view.SelectedNodeIndex, token));
        _deleting = false;
        if (preflightOutcome.Cancelled) return;
        if (!preflightOutcome.Succeeded || preflightOutcome.Value is null)
        {
            _view.SetBusy(false, $"Could not check the selected item: {preflightOutcome.Error?.Message ?? "unknown error"}");
            return;
        }
        var preflight = preflightOutcome.Value;
        if (!preflight.CanDelete)
        {
            _view.SetBusy(false, $"Cannot move the selected item: {preflight.Detail}");
            _activity.Warning(Key, "Storage deletion unavailable", preflight.Detail);
            return;
        }
        _view.SetBusy(false, preflight.Summary);
        if (!await _view.ConfirmDeleteAsync(preflight))
        {
            _deletion.Revoke(preflight.TicketId);
            _activity.Info(Key, "Storage deletion cancelled", preflight.TargetPath);
            _view.SetBusy(false, "Deletion cancelled; nothing was moved.");
            return;
        }

        _deleting = true;
        _view.SetBusy(true, "Checking the selected item and moving it to the Recycle Bin…");
        var deleteOutcome = await _operations.RunCommittedAsync(DeleteOperationKey, Key,
            "confirmed storage deletion", token => _deletion.ExecuteAsync(preflight.TicketId, token));
        _deleting = false;
        if (deleteOutcome.Cancelled) return;
        if (!deleteOutcome.Succeeded || deleteOutcome.Value is null)
        {
            _view.SetBusy(false, $"Deletion failed: {deleteOutcome.Error?.Message ?? "unknown error"}");
            return;
        }
        var result = deleteOutcome.Value;
        foreach (var line in result.Log)
            _activity.Publish(ActivityEvent.Create(Key, line, result.Succeeded ? ActivitySeverity.Info : ActivitySeverity.Warning));
        if (!result.Succeeded)
        {
            _activity.Warning(Key, "Storage deletion stopped", result.Summary, persist: true);
            _view.SetBusy(false, result.Summary);
            return;
        }
        _activity.Info(Key, result.Summary, result.TargetPath, persist: true);
        await ScanAsync();
    }

    private void View_CancelRequested(object? sender, EventArgs e)
    {
        if (_deleting) _operations.Cancel(DeleteOperationKey);
        if (_scanning) _operations.Cancel(OperationKey);
        _activity.Warning(Key, "User cancelled storage operation");
    }
}
