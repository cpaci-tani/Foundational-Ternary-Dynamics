using System.Collections.ObjectModel;
using System.Text;
using Sift.Models;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using Sift.WinUI.Infrastructure.Interop;

namespace Sift.WinUI.Views;

public sealed partial class SystemInformationWorkspaceView : UserControl
{
    private readonly ObservableCollection<SystemInfoItem> _visible = [];
    private readonly List<ListView> _sectionLists = [];
    private SystemInformationReport? _report;
    private SystemInfoItem? _selected;
    private bool _binding;
    private bool _syncingSelection;
    private readonly IClipboardService _clipboard;

    public SystemInformationWorkspaceView(IClipboardService clipboard)
    {
        _clipboard = clipboard ?? throw new ArgumentNullException(nameof(clipboard));
        InitializeComponent();
        _binding = true;
        CategoryFilter.Items.Add("All categories");
        CategoryFilter.SelectedIndex = 0;
        _binding = false;
        ApplyFilter();
    }

    public event EventHandler? RefreshRequested;
    public event EventHandler? OpenMsInfoRequested;
    public event EventHandler? ReportCopied;
    public event EventHandler? PropertyCopied;

    public void Bind(SystemInformationReport report)
    {
        _report = report;
        DeviceNameText.Text = report.DeviceName;
        DeviceModelText.Text = report.DeviceModel;
        WindowsText.Text = report.WindowsVersion;
        ArchitectureText.Text = report.Architecture;
        ProcessorText.Text = report.Processor;
        MemoryText.Text = report.Memory;

        var selectedCategory = CategoryFilter.SelectedItem?.ToString() ?? "All categories";
        _binding = true;
        CategoryFilter.Items.Clear();
        CategoryFilter.Items.Add("All categories");
        foreach (var category in report.Categories) CategoryFilter.Items.Add(category);
        CategoryFilter.SelectedItem = CategoryFilter.Items.Cast<object>()
            .FirstOrDefault(item => string.Equals(item.ToString(), selectedCategory, StringComparison.OrdinalIgnoreCase))
            ?? CategoryFilter.Items[0];
        _binding = false;
        ApplyFilter();
        StatusText.Text = report.Warnings.Count == 0
            ? $"{report.Items.Count:N0} properties · {report.Categories.Count:N0} categories · updated {report.GeneratedLocal:T}"
            : $"{report.Items.Count:N0} properties · {report.Warnings.Count:N0} optional provider warning(s) · updated {report.GeneratedLocal:T}";
        ToolTipService.SetToolTip(StatusText, report.Warnings.Count == 0
            ? "All requested providers completed."
            : string.Join(Environment.NewLine, report.Warnings));
    }

    public void SetBusy(bool busy, string status)
    {
        BusyRing.IsActive = busy;
        RefreshButton.IsEnabled = !busy;
        CopyReportButton.IsEnabled = !busy && _visible.Count > 0;
        StatusText.Text = status;
        if (busy && _report is null) ShowState("Loading system information", status, true);
        else if (!busy && status.StartsWith("Refresh failed", StringComparison.OrdinalIgnoreCase))
            ShowState("Could not read system information", status, false);
    }

    public void SetProgress(string status)
    {
        if (!BusyRing.IsActive) return;
        StatusText.Text = status;
        if (_report is null) ShowState("Loading system information", status, true);
    }

    public void FocusSearch() => SearchBox.Focus(FocusState.Programmatic);

    private void ApplyFilter()
    {
        if (_binding || SectionsHost is null) return;
        var query = SearchBox.Text.Trim();
        var category = CategoryFilter.SelectedItem?.ToString() ?? "All categories";
        var rows = (_report?.Items ?? [])
            .Where(item => category == "All categories" || item.Category.Equals(category, StringComparison.OrdinalIgnoreCase))
            .Where(item => string.IsNullOrWhiteSpace(query) || item.SearchText.Contains(query, StringComparison.OrdinalIgnoreCase))
            .ToList();

        _visible.Clear();
        foreach (var row in rows) _visible.Add(row);
        CountText.Text = $"{rows.Count:N0} propert{(rows.Count == 1 ? "y" : "ies")}";
        CopyReportButton.IsEnabled = !BusyRing.IsActive && rows.Count > 0;
        RebuildSections(rows);

        if (rows.Count == 0)
        {
            var hasReport = _report is not null;
            ShowState(hasReport ? "No matching properties" : "Loading system information",
                hasReport ? "Try a broader search or choose another category." : "Windows providers will be read locally.",
                !hasReport && BusyRing.IsActive);
            PropertyScroller.Visibility = Visibility.Collapsed;
        }
        else
        {
            EmptyState.Visibility = Visibility.Collapsed;
            PropertyScroller.Visibility = Visibility.Visible;
        }

        if (_selected is not null && rows.All(item => !ReferenceEquals(item, _selected) &&
            !(item.Category == _selected.Category && item.Component == _selected.Component &&
              item.Property == _selected.Property && item.Value == _selected.Value)))
            _selected = null;
        UpdateSelection();
    }

    private void RebuildSections(IReadOnlyList<SystemInfoItem> rows)
    {
        SectionsHost.Children.Clear();
        _sectionLists.Clear();
        var groups = rows
            .GroupBy(item => item.Category, StringComparer.OrdinalIgnoreCase)
            .OrderBy(group => group.Key, StringComparer.CurrentCultureIgnoreCase)
            .ToList();

        var sectionIndex = 0;
        foreach (var group in groups)
        {
            var items = group
                .OrderBy(item => item.Component, StringComparer.CurrentCultureIgnoreCase)
                .ThenBy(item => item.Property, StringComparer.CurrentCultureIgnoreCase)
                .ToList();

            var list = new ListView
            {
                ItemsSource = items,
                ItemTemplate = (DataTemplate)Resources["PropertyRowTemplate"],
                ItemContainerStyle = (Style)Application.Current.Resources["InventoryListItemStyle"],
                SelectionMode = ListViewSelectionMode.Single,
                IsItemClickEnabled = true,
                Margin = new Thickness(0)
            };
            ScrollViewer.SetVerticalScrollBarVisibility(list, ScrollBarVisibility.Disabled);
            ScrollViewer.SetHorizontalScrollBarVisibility(list, ScrollBarVisibility.Disabled);
            list.SelectionChanged += SectionList_SelectionChanged;
            _sectionLists.Add(list);

            var headerGrid = new Grid { ColumnSpacing = 12 };
            headerGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
            headerGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
            var title = new TextBlock
            {
                Text = group.Key,
                Style = (Style)Application.Current.Resources["TypeSectionTitleStyle"]
            };
            var count = new TextBlock
            {
                Text = $"{items.Count:N0} propert{(items.Count == 1 ? "y" : "ies")}",
                Style = (Style)Application.Current.Resources["TypeMetaStyle"],
                VerticalAlignment = VerticalAlignment.Center
            };
            headerGrid.Children.Add(title);
            Grid.SetColumn(title, 0);
            headerGrid.Children.Add(count);
            Grid.SetColumn(count, 1);

            var header = new Border
            {
                Background = (Brush)Application.Current.Resources["SiftElevatedBrush"],
                BorderBrush = (Brush)Application.Current.Resources["SiftLineBrush"],
                BorderThickness = new Thickness(0, 0, 0, 1),
                Padding = new Thickness(16, 12, 16, 12),
                Margin = new Thickness(0, sectionIndex == 0 ? 4 : 14, 0, 0),
                Child = headerGrid
            };

            var body = new Border
            {
                Background = (Brush)Application.Current.Resources["SiftCardBrush"],
                BorderBrush = (Brush)Application.Current.Resources["SiftLineBrush"],
                BorderThickness = new Thickness(0, 0, 0, 1),
                Padding = new Thickness(0, 2, 0, 4),
                Child = list
            };

            var section = new StackPanel { Spacing = 0 };
            section.Children.Add(header);
            section.Children.Add(body);
            SectionsHost.Children.Add(section);
            sectionIndex++;
        }
    }

    private void SectionList_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_syncingSelection || sender is not ListView list) return;
        if (list.SelectedItem is not SystemInfoItem item)
        {
            if (_sectionLists.All(section => section.SelectedItem is null))
            {
                _selected = null;
                UpdateSelection();
            }
            return;
        }

        _selected = item;
        _syncingSelection = true;
        try
        {
            foreach (var other in _sectionLists)
            {
                if (!ReferenceEquals(other, list) && other.SelectedItem is not null)
                    other.SelectedItem = null;
            }
        }
        finally
        {
            _syncingSelection = false;
        }
        UpdateSelection();
    }

    private void UpdateSelection()
    {
        if (_selected is null)
        {
            SelectedPropertyText.Text = "Select a row to inspect its full value.";
            SelectedValueText.Text = "Values wrap in the list; this panel shows the complete line for copy.";
            CopySelectedButton.IsEnabled = false;
            return;
        }

        SelectedPropertyText.Text = $"{_selected.Category} · {_selected.Component} · {_selected.Property}";
        SelectedValueText.Text = $"{_selected.Value}    Source: {_selected.Source}";
        CopySelectedButton.IsEnabled = true;
    }

    private void CopyVisibleReport()
    {
        if (_report is null || _visible.Count == 0) return;
        var text = new StringBuilder()
            .AppendLine("Sift system information")
            .AppendLine($"Generated: {_report.GeneratedLocal:F}")
            .AppendLine($"Device: {_report.DeviceName} · {_report.DeviceModel}")
            .AppendLine($"Windows: {_report.WindowsVersion} · {_report.Architecture}")
            .AppendLine($"Processor: {_report.Processor}")
            .AppendLine($"Memory: {_report.Memory}")
            .AppendLine("Warning: Review serial numbers, device IDs, IP addresses, and MAC addresses before sharing.")
            .AppendLine();
        foreach (var group in _visible.GroupBy(item => item.Category))
        {
            text.AppendLine($"[{group.Key}]");
            foreach (var item in group) text.AppendLine($"{item.Component} · {item.Property}: {item.Value} ({item.Source})");
            text.AppendLine();
        }
        _clipboard.CopyText(text.ToString().TrimEnd());
        StatusText.Text = $"Copied {_visible.Count:N0} visible properties. Review identifiers before sharing.";
        ReportCopied?.Invoke(this, EventArgs.Empty);
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
    private void CategoryFilter_SelectionChanged(object sender, SelectionChangedEventArgs e) => ApplyFilter();
    private void RefreshButton_Click(object sender, RoutedEventArgs e) => RefreshRequested?.Invoke(this, EventArgs.Empty);
    private void OpenMsInfoButton_Click(object sender, RoutedEventArgs e) => OpenMsInfoRequested?.Invoke(this, EventArgs.Empty);
    private void CopyReportButton_Click(object sender, RoutedEventArgs e) => CopyVisibleReport();
    private void CopySelectedButton_Click(object sender, RoutedEventArgs e)
    {
        if (_selected is null) return;
        _clipboard.CopyText($"{_selected.Category} · {_selected.Component} · {_selected.Property}: {_selected.Value} ({_selected.Source})");
        StatusText.Text = "Copied the selected property. Review identifiers before sharing.";
        PropertyCopied?.Invoke(this, EventArgs.Empty);
    }
}
