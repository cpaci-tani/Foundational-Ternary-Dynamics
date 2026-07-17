using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Views;

public sealed partial class ScriptCenterWorkspaceView
{
    private void ApplySubtitle()
    {
        var standardCount = _all.Count(x => !x.RequiresAdministrator);
        var administratorCount = _all.Count - standardCount;
        SubtitleText.Text = ReferenceEquals(WorkspaceTabs.SelectedItem, StudioTab)
            ? "Analyze PowerShell, Python, Bash, CMD, JavaScript, and TypeScript without running the script"
            : $"{standardCount:N0} standard and {administratorCount:N0} administrator commands";
    }

    private async void WorkspaceTabs_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        var studioSelected = ReferenceEquals(WorkspaceTabs.SelectedItem, StudioTab);
        LibraryActions.Visibility = studioSelected ? Visibility.Collapsed : Visibility.Visible;
        ApplySubtitle();
        if (studioSelected)
        {
            StudioWebView.Visibility = Visibility.Visible;
            await Task.Yield();
            await InitializeStudioAsync();
            ResumeStudio();
            return;
        }

        // Suspend (do not destroy) the WebView when leaving the Studio tab. TrySuspendAsync releases
        // the browser process's working set while keeping the control reusable, and requires the
        // control to be hidden first. Close()/DisposeStudio is terminal — a closed WebView2 cannot be
        // re-initialized, so calling it here bricked the editor on the next visit; it now belongs only
        // to module disposal.
        StudioWebView.Visibility = Visibility.Collapsed;
        if (_studioInitialized) await SuspendStudioAsync();
    }

    private void RootGrid_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        var tabsHeight = Math.Max(430, e.NewSize.Height - HeaderGrid.ActualHeight - 14);
        WorkspaceTabs.Height = tabsHeight;
        var contentHeight = Math.Max(380, tabsHeight - 50);
        LibraryContentGrid.Height = contentHeight;
        StudioContentGrid.Height = contentHeight;

        var stackLibrary = e.NewSize.Width < 1_040;
        LibraryListColumn.Width = stackLibrary ? new GridLength(1, GridUnitType.Star) : new GridLength(1.05, GridUnitType.Star);
        LibraryTerminalColumn.Width = stackLibrary ? new GridLength(0) : new GridLength(1.35, GridUnitType.Star);
        LibraryTopRow.Height = stackLibrary ? new GridLength(1.1, GridUnitType.Star) : new GridLength(1, GridUnitType.Star);
        LibraryBottomRow.Height = stackLibrary ? new GridLength(0.9, GridUnitType.Star) : new GridLength(0);
        Grid.SetColumn(LibraryTerminalPanel, stackLibrary ? 0 : 1);
        Grid.SetRow(LibraryTerminalPanel, stackLibrary ? 1 : 0);

        var stackFilters = e.NewSize.Width < 650;
        LibraryFilterSecondaryRow.Height = stackFilters ? GridLength.Auto : new GridLength(0);
        Grid.SetColumnSpan(CategoryBox, stackFilters ? 3 : 1);
        Grid.SetRow(ExpandAllButton, stackFilters ? 1 : 0);
        Grid.SetRow(CollapseAllButton, stackFilters ? 1 : 0);
    }
}
