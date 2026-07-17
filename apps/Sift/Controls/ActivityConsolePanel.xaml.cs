using System.Collections.ObjectModel;
using Sift.Infrastructure.Activity;
using Sift.WinUI.Models;
using Sift.WinUI.Infrastructure.Interop;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Controls;

public sealed partial class ActivityConsolePanel : UserControl, IDisposable
{
    private const int MaximumEntries = 1_000;
    private readonly List<ActivityConsoleItem> _allItems = [];
    private readonly ObservableCollection<ActivityConsoleItem> _visibleItems = [];
    private ActivityHub? _activity;
    private IClipboardService? _clipboard;

    public ActivityConsolePanel()
    {
        InitializeComponent();
        ConsoleList.ItemsSource = _visibleItems;
        ApplyFilter();
    }

    public event EventHandler? HideRequested;

    public void Connect(ActivityHub activity, IClipboardService clipboard)
    {
        ArgumentNullException.ThrowIfNull(activity);
        ArgumentNullException.ThrowIfNull(clipboard);
        if (ReferenceEquals(_activity, activity) && ReferenceEquals(_clipboard, clipboard)) return;
        Disconnect();
        _activity = activity;
        _clipboard = clipboard;
        _activity.Published += Activity_Published;
    }

    public void Dispose() => Disconnect();

    private void Disconnect()
    {
        if (_activity is not null) _activity.Published -= Activity_Published;
        _activity = null;
        _clipboard = null;
    }

    private void Activity_Published(object? sender, ActivityEvent activity)
    {
        DispatcherQueue.TryEnqueue(() =>
        {
            _allItems.Add(ActivityConsoleItem.From(activity));
            while (_allItems.Count > MaximumEntries) _allItems.RemoveAt(0);
            ApplyFilter(scrollToNewest: AutoScrollToggle.IsOn);
        });
    }

    private void ApplyFilter(bool scrollToNewest = false)
    {
        if (FilterBox is null || SeverityFilter is null || ConsoleList is null) return;
        var query = FilterBox.Text.Trim();
        var severity = (SeverityFilter.SelectedItem as ComboBoxItem)?.Content?.ToString();
        var matches = _allItems.Where(item =>
            (string.IsNullOrWhiteSpace(query) || item.SearchText.Contains(query, StringComparison.OrdinalIgnoreCase)) &&
            (severity is null || severity == "All levels" || item.Severity.ToString().Equals(severity, StringComparison.OrdinalIgnoreCase)))
            .ToList();

        _visibleItems.Clear();
        foreach (var item in matches) _visibleItems.Add(item);
        EmptyState.Visibility = matches.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
        ConsoleCountText.Text = $"{matches.Count:N0} shown · {_allItems.Count:N0} captured";
        if (scrollToNewest && _visibleItems.Count > 0) ConsoleList.ScrollIntoView(_visibleItems[^1]);
    }

    private void CopyButton_Click(object sender, RoutedEventArgs e)
    {
        if (_visibleItems.Count == 0) return;
        var text = string.Join(Environment.NewLine, _visibleItems.Select(item => $"{item.Header}  {item.Message}"));
        _clipboard?.CopyText(text, persistAfterExit: false);
    }

    private void ClearButton_Click(object sender, RoutedEventArgs e)
    {
        _allItems.Clear();
        ApplyFilter();
    }

    private void FilterBox_TextChanged(object sender, TextChangedEventArgs e) => ApplyFilter();
    private void SeverityFilter_SelectionChanged(object sender, SelectionChangedEventArgs e) => ApplyFilter();
    private void HideButton_Click(object sender, RoutedEventArgs e) => HideRequested?.Invoke(this, EventArgs.Empty);
}
