using System.Collections.ObjectModel;
using Sift.Models;
using Sift.WinUI.Infrastructure.Dialogs;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Views;

public sealed partial class RecoveryWorkspaceView : UserControl
{
    private readonly ObservableCollection<RecoveryBackupInfo> _visible = [];
    private IReadOnlyList<RecoveryBackupInfo> _all = [];
    private bool _binding;

    public RecoveryWorkspaceView()
    {
        InitializeComponent();
        BackupsTable.ItemsSource = _visible;
        StatusFilter.SelectedIndex = 0;
    }

    public event EventHandler? RefreshRequested;
    public event EventHandler? OpenFolderRequested;
    public event EventHandler? RestoreRequested;
    public RecoveryBackupInfo? SelectedBackup => BackupsTable.SelectedItem as RecoveryBackupInfo;

    public void Bind(IReadOnlyList<RecoveryBackupInfo> backups)
    {
        _binding = true;
        _all = backups;
        BackupCountText.Text = backups.Count.ToString("N0");
        ReadyCountText.Text = backups.Count(item => item.CanRestore).ToString("N0");
        ProtectedCountText.Text = backups.Count(item => item.RequiresElevation && item.PendingCount > 0).ToString("N0");
        PendingCountText.Text = backups.Sum(item => item.PendingCount).ToString("N0");
        _binding = false;
        ApplyFilter();
    }

    public void SetBusy(bool busy, string status)
    {
        BusyRing.IsActive = busy;
        RefreshButton.IsEnabled = !busy;
        BackupsTable.IsEnabled = !busy;
        StatusText.Text = status;
        if (busy)
        {
            EmptyState.Visibility = _all.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
            StateProgressRing.IsActive = _all.Count == 0;
            StateTitleText.Text = "Loading recovery backups";
            StateDetailText.Text = "Reading Sift backup files.";
        }
        else
        {
            StateProgressRing.IsActive = false;
            UpdateSelection();
        }
    }

    public void FocusSearch() => SearchBox.Focus(FocusState.Programmatic);

    public async Task<bool> ConfirmRestoreAsync(RecoveryBackupInfo backup)
    {
        var panel = new StackPanel { Spacing = 10, MaxWidth = 620 };
        panel.Children.Add(new TextBlock
        {
            Text = $"{backup.PendingCount:N0} pending entr{(backup.PendingCount == 1 ? "y" : "ies")} will be restored. Scope: {backup.ScopeDisplay}." +
                (backup.RequiresElevation
                    ? Environment.NewLine + Environment.NewLine +
                      "Windows will ask for administrator permission before this action starts."
                    : string.Empty),
            TextWrapping = TextWrapping.Wrap
        });
        var details = new TextBlock
        {
            Text = $"Source: {backup.Source}{Environment.NewLine}Created: {backup.CreatedDisplay}{Environment.NewLine}Machine: {backup.MachineName}{Environment.NewLine}File: {backup.FileName}{Environment.NewLine}{backup.Detail}",
            MinHeight = 112,
            TextWrapping = TextWrapping.Wrap,
            FontFamily = new Microsoft.UI.Xaml.Media.FontFamily("Consolas"),
            FontSize = 12,
            LineHeight = 19,
            Foreground = (Microsoft.UI.Xaml.Media.Brush)Application.Current.Resources["SiftMutedBrush"]
        };
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(details, "Selected recovery backup details");
        panel.Children.Add(new Border
        {
            Background = (Microsoft.UI.Xaml.Media.Brush)Application.Current.Resources["SiftPanelBrush"],
            BorderBrush = (Microsoft.UI.Xaml.Media.Brush)Application.Current.Resources["SiftLineBrush"],
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(7),
            Padding = new Thickness(12, 10, 12, 10),
            Child = details
        });
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = $"Restore {backup.FileName}?",
            Content = panel,
            PrimaryButtonText = "Restore backup",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    private void ApplyFilter()
    {
        if (_binding) return;
        var query = SearchBox.Text.Trim();
        var status = (StatusFilter.SelectedItem as ComboBoxItem)?.Content?.ToString();
        var selectedPath = SelectedBackup?.Path;
        var filtered = _all.Where(item =>
            (string.IsNullOrWhiteSpace(query) || string.Join(' ', item.CreatedDisplay, item.Source, item.Status,
                item.ScopeDisplay, item.MachineName, item.FileName).Contains(query, StringComparison.OrdinalIgnoreCase)) &&
            (status is null or "All backups" || item.Status.Equals(status, StringComparison.OrdinalIgnoreCase))).ToList();
        _visible.Clear();
        foreach (var item in filtered) _visible.Add(item);
        VisibleCountText.Text = $"{filtered.Count:N0} shown";
        EmptyState.Visibility = filtered.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
        StateProgressRing.IsActive = false;
        StateTitleText.Text = _all.Count == 0 ? "No Sift backups yet" : "No backups match this filter";
        StateDetailText.Text = _all.Count == 0
            ? "Backups are created before supported mutations."
            : "Try a broader search or status filter.";
        if (!string.IsNullOrWhiteSpace(selectedPath))
            BackupsTable.SelectedItem = filtered.FirstOrDefault(item => item.Path.Equals(selectedPath,
                StringComparison.OrdinalIgnoreCase));
        UpdateSelection();
    }

    private void UpdateSelection()
    {
        var backup = SelectedBackup;
        if (backup is null)
        {
            SelectedTitleText.Text = "Select a backup to see its contents.";
            SelectedDetailText.Text = "Nothing changes until you select Restore backup and confirm.";
            SelectedPathText.Text = string.Empty;
            RestoreButton.IsEnabled = false;
            SelectionNoteText.Text = "Select a ready backup";
            return;
        }
        SelectedTitleText.Text = $"{backup.Source} · {backup.CreatedDisplay} · {backup.Status}";
        SelectedDetailText.Text = backup.Detail;
        SelectedPathText.Text = backup.Path;
        RestoreButton.IsEnabled = backup.CanRestore && !BusyRing.IsActive;
        SelectionNoteText.Text = backup.CanRestore
            ? $"{backup.ScopeDisplay} · {backup.FileName}"
            : "Restore unavailable · inspect the reason";
    }

    private void RefreshButton_Click(object sender, RoutedEventArgs e) => RefreshRequested?.Invoke(this, EventArgs.Empty);
    private void OpenFolderButton_Click(object sender, RoutedEventArgs e) => OpenFolderRequested?.Invoke(this, EventArgs.Empty);
    private void RestoreButton_Click(object sender, RoutedEventArgs e) => RestoreRequested?.Invoke(this, EventArgs.Empty);
    private void SearchBox_TextChanged(object sender, TextChangedEventArgs e) => ApplyFilter();
    private void StatusFilter_SelectionChanged(object sender, SelectionChangedEventArgs e) => ApplyFilter();
    private void BackupsTable_SelectionChanged(object sender, SelectionChangedEventArgs e) => UpdateSelection();
}
