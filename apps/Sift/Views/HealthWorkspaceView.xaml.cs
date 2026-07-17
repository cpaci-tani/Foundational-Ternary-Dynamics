using System.Collections.ObjectModel;
using Sift.Models;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Views;

public sealed partial class HealthWorkspaceView : UserControl
{
    private readonly ObservableCollection<HealthCheckRow> _checks = [];
    private readonly ObservableCollection<HistoryRow> _history = [];
    private IReadOnlyList<HealthCheckRow> _allChecks = [];
    private IReadOnlyList<HistoryRow> _allHistory = [];
    private IReadOnlyList<string> _historyWarnings = [];
    private bool _loading = true;
    private string? _checksError;
    private string? _historyError;

    public HealthWorkspaceView()
    {
        InitializeComponent();
        ChecksList.ItemsSource = _checks;
        HistoryList.ItemsSource = _history;
        SetLoading();
    }

    public event EventHandler? RefreshRequested;

    public void SetLoading()
    {
        _loading = true;
        _checksError = null;
        _historyError = null;
        BusyRing.IsActive = true;
        RefreshButton.IsEnabled = false;
        ChecksList.IsEnabled = false;
        HistoryList.IsEnabled = false;
        PartialHistoryPanel.Visibility = Visibility.Collapsed;
        ShowChecksState("Loading health", "Loading checks and history.", active: true);
        ShowHistoryState("Loading health", "Loading checks and history.", active: true);
        StatusText.Text = "Loading health";
    }

    public void BindChecks(IReadOnlyList<HealthCheckRow> checks)
    {
        _loading = false;
        _allChecks = checks;
        _checksError = null;
        ApplyChecksFilter();
        BusyRing.IsActive = false;
        RefreshButton.IsEnabled = true;
        ChecksList.IsEnabled = true;
        HistoryList.IsEnabled = true;
    }

    public void BindHistory(HistorySnapshot history)
    {
        _loading = false;
        _allHistory = history.Rows;
        _historyWarnings = history.Warnings;
        _historyError = null;
        PartialHistoryPanel.Visibility = history.IsPartial ? Visibility.Visible : Visibility.Collapsed;
        PartialHistoryText.Text = history.IsPartial
            ? $"Partial history · {string.Join(" ", history.Warnings)}"
            : "Partial history";
        ApplyHistoryFilter();
        BusyRing.IsActive = false;
        RefreshButton.IsEnabled = true;
        ChecksList.IsEnabled = true;
        HistoryList.IsEnabled = true;
    }

    public void SetChecksError(string message)
    {
        _checksError = message;
        _loading = false;
        ShowChecksState(message, "Health checks could not be loaded from the local scanner.", active: false);
    }

    public void SetHistoryError(string message)
    {
        _historyError = message;
        _loading = false;
        ShowHistoryState(message, "Activity and recovery history could not be loaded.", active: false);
    }

    public void SetStatus(string message) => StatusText.Text = message;

    public void FocusSearch()
    {
        if (HealthTabs.SelectedIndex == 1)
            HistoryFilterBox.Focus(FocusState.Programmatic);
        else
            ChecksFilterBox.Focus(FocusState.Programmatic);
    }

    private void ApplyChecksFilter()
    {
        if (_loading) return;
        var query = ChecksFilterBox.Text.Trim();
        var rows = _allChecks.Where(check =>
            string.IsNullOrWhiteSpace(query) ||
            string.Join(' ', check.Title, check.Detail, check.Recommendation, check.StatusLabel)
                .Contains(query, StringComparison.OrdinalIgnoreCase)).ToList();
        _checks.Clear();
        foreach (var row in rows) _checks.Add(row);
        ChecksCountText.Text = $"{rows.Count:N0} shown";
        if (_checksError is not null)
        {
            ShowChecksState(_checksError, "Health checks could not be loaded from the local scanner.", active: false);
            return;
        }
        if (_allChecks.Count == 0)
            ShowChecksState("No checks returned", "The local scanner returned no health checks.", active: false);
        else if (rows.Count == 0)
            ShowChecksState("No matching checks", "Try a broader filter.", active: false);
        else
            ChecksEmptyState.Visibility = Visibility.Collapsed;
    }

    private void ApplyHistoryFilter()
    {
        if (_loading) return;
        var query = HistoryFilterBox.Text.Trim();
        var rows = _allHistory.Where(row =>
            string.IsNullOrWhiteSpace(query) ||
            string.Join(' ', row.DisplayTime, row.Category, row.Title, row.Detail)
                .Contains(query, StringComparison.OrdinalIgnoreCase)).ToList();
        _history.Clear();
        foreach (var row in rows) _history.Add(row);
        HistoryCountText.Text = $"{rows.Count:N0} shown";
        if (_historyError is not null)
        {
            ShowHistoryState(_historyError, "Activity and recovery history could not be loaded.", active: false);
            return;
        }
        if (_allHistory.Count == 0)
            ShowHistoryState("No activity or recovery history yet", "Persisted activity and backup history will appear here.", active: false);
        else if (rows.Count == 0)
            ShowHistoryState("No matching history", "Try a broader filter.", active: false);
        else
            HistoryEmptyState.Visibility = Visibility.Collapsed;
    }

    private void ShowChecksState(string title, string detail, bool active)
    {
        ChecksEmptyState.Visibility = Visibility.Visible;
        ChecksStateRing.IsActive = active;
        ChecksStateTitle.Text = title;
        ChecksStateDetail.Text = detail;
    }

    private void ShowHistoryState(string title, string detail, bool active)
    {
        HistoryEmptyState.Visibility = Visibility.Visible;
        HistoryStateRing.IsActive = active;
        HistoryStateTitle.Text = title;
        HistoryStateDetail.Text = detail;
    }

    private void RootGrid_SizeChanged(object sender, SizeChangedEventArgs e) =>
        VisualStateManager.GoToState(this, e.NewSize.Width < 980 ? "Narrow" : "Wide", true);

    private void RefreshButton_Click(object sender, RoutedEventArgs e) => RefreshRequested?.Invoke(this, EventArgs.Empty);
    private void ChecksFilterBox_TextChanged(object sender, TextChangedEventArgs e) => ApplyChecksFilter();
    private void HistoryFilterBox_TextChanged(object sender, TextChangedEventArgs e) => ApplyHistoryFilter();
    private void HealthTabs_SelectionChanged(object sender, SelectionChangedEventArgs e) { }
}
