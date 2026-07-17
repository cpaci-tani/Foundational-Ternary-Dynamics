using System.Collections.ObjectModel;
using Sift.Services;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Views;

public sealed partial class StartupWorkspaceView : UserControl
{
    private readonly ObservableCollection<StartupEnumerator.StartupEntry> _visibleEntries = [];
    private IReadOnlyList<StartupEnumerator.StartupEntry> _allEntries = [];

    public StartupWorkspaceView()
    {
        InitializeComponent();
        StartupTable.ItemsSource = _visibleEntries;
        ApplyFilter();
    }

    public event EventHandler? RefreshRequested;
    public event EventHandler? OpenSettingsRequested;

    public void Bind(IReadOnlyList<StartupEnumerator.StartupEntry> entries, string status)
    {
        _allEntries = entries;
        StatusText.Text = status;
        ApplyFilter();
    }

    public void SetBusy(bool busy, string status)
    {
        BusyRing.IsActive = busy;
        RefreshButton.IsEnabled = !busy;
        StatusText.Text = status;
        if (busy && _allEntries.Count == 0) ShowState("Loading startup entries", "Reading Run keys and Startup folders…", true);
        else if (!busy && status.StartsWith("Refresh failed", StringComparison.OrdinalIgnoreCase))
            ShowState("Could not refresh", status, false);
    }

    public void FocusSearch() => SearchBox.Focus(FocusState.Programmatic);

    private void ApplyFilter()
    {
        var query = SearchBox.Text.Trim();
        var entries = string.IsNullOrWhiteSpace(query)
            ? _allEntries
            : _allEntries.Where(entry =>
                entry.Name.Contains(query, StringComparison.OrdinalIgnoreCase) ||
                entry.Command.Contains(query, StringComparison.OrdinalIgnoreCase) ||
                entry.Source.Contains(query, StringComparison.OrdinalIgnoreCase) ||
                entry.Status.Contains(query, StringComparison.OrdinalIgnoreCase)).ToList();

        _visibleEntries.Clear();
        foreach (var entry in entries) _visibleEntries.Add(entry);
        CountText.Text = $"{entries.Count:N0} entr{(entries.Count == 1 ? "y" : "ies")}";
        if (entries.Count == 0)
        {
            var filtered = !string.IsNullOrWhiteSpace(query);
            ShowState(filtered ? "No matching startup entries" : "No startup entries",
                filtered ? "Try a broader filter." : "No supported startup registrations were found.", false);
        }
        else EmptyState.Visibility = Visibility.Collapsed;
    }

    private void ShowState(string title, string detail, bool progress)
    {
        StateTitleText.Text = title;
        StateDetailText.Text = detail;
        StateProgressRing.IsActive = progress;
        StateProgressRing.Visibility = progress ? Visibility.Visible : Visibility.Collapsed;
        EmptyState.Visibility = Visibility.Visible;
    }

    private void SearchBox_TextChanged(object sender, TextChangedEventArgs e) => ApplyFilter();
    private void RefreshButton_Click(object sender, RoutedEventArgs e) => RefreshRequested?.Invoke(this, EventArgs.Empty);
    private void OpenSettingsButton_Click(object sender, RoutedEventArgs e) => OpenSettingsRequested?.Invoke(this, EventArgs.Empty);
}
