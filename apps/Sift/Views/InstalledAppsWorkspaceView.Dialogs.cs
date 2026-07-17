using Microsoft.UI.Xaml.Controls;
using Sift.Models;
using Sift.WinUI.Infrastructure.Dialogs;

namespace Sift.WinUI.Views;

public sealed partial class InstalledAppsWorkspaceView
{
    public async Task<bool> ConfirmUninstallAsync(InstalledApp app, string review)
    {
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = $"Open the uninstaller for {app.DisplayName}?",
            Content = $"{review}\n\nThis opens the app's registered uninstaller. The app or Windows controls the remaining steps. Sift cannot undo changes made by the uninstaller.",
            PrimaryButtonText = "Open uninstaller",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    public async Task<bool> ConfirmRegistrationCleanupAsync(InstalledApp app, string review)
    {
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = $"Remove the leftover registration for {app.DisplayName}?",
            Content = $"{review}\n\n{app.OrphanEvidence}\n\nThis removes only the uninstall registration. App files remain, and Sift creates a backup first.",
            PrimaryButtonText = "Remove registration",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    public async Task<bool> ConfirmFileLeftoverDeletionAsync(InstalledApp app,
        IReadOnlyList<AppLeftoverCandidate> selection, AppLeftoverDeleteResult review)
    {
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = $"Move {selection.Count:N0} leftover folder(s) for {app.DisplayName} to the Recycle Bin?",
            Content = $"{review.Summary}\n\nSelected AppData may include settings, caches, saved sessions, or user-created content ({FormatBytes(selection.Sum(item => item.SizeBytes))}). Another app may share a selected folder.",
            PrimaryButtonText = "Move to Recycle Bin",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }
}
