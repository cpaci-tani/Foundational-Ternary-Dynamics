using System.Collections.ObjectModel;
using Sift.Models;
using Sift.WinUI.Infrastructure.Dialogs;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Views;

public sealed partial class MaintenanceWorkspaceView : UserControl
{
    private readonly ObservableCollection<MaintenanceFinding> _visible = [];
    private IReadOnlyList<MaintenanceFinding> _all = [];
    private bool _binding;

    public MaintenanceWorkspaceView()
    {
        InitializeComponent();
        FindingList.ItemsSource = _visible;
    }

    public event EventHandler? ScanRequested;
    public event EventHandler? CleanRequested;
    public IReadOnlyList<MaintenanceFinding> Selected => _all.Where(x => x.IsSelected && x.CanClean).ToList();

    public void Bind(IReadOnlyList<MaintenanceFinding> findings)
    {
        _binding = true;
        _all = findings;
        CategoryBox.Items.Clear();
        CategoryBox.Items.Add("All categories");
        foreach (var category in findings.Select(x => x.CategoryLabel).Distinct().OrderBy(x => x)) CategoryBox.Items.Add(category);
        CategoryBox.SelectedIndex = 0;
        foreach (var finding in findings) finding.PropertyChanged += (_, _) => UpdateSelection();
        _binding = false;
        ApplyFilter();
    }

    public void SetBusy(bool busy, string status)
    {
        BusyRing.IsActive = busy;
        ScanButton.IsEnabled = !busy;
        FindingList.IsEnabled = !busy;
        StatusText.Text = status;
        if (busy && _all.Count == 0) ShowState("Scanning maintenance locations", "Checking temporary files, caches, and app registrations.", true);
        if (!busy) UpdateSelection();
    }

    public void FocusSearch() => SearchBox.Focus(FocusState.Programmatic);

    public async Task<bool> ConfirmCleanAsync(IReadOnlyList<MaintenanceFinding> selection, CleanResult review)
    {
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Clean selected items?",
            Content = BuildReviewContent(selection, review),
            PrimaryButtonText = "Clean",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    private static StackPanel BuildReviewContent(IReadOnlyList<MaintenanceFinding> selection, CleanResult review)
    {
        var panel = new StackPanel { Spacing = 10, MaxWidth = 620 };
        var registrationNote = selection.Any(item => item.Category == MaintenanceCategory.OrphanUninstall)
            ? " Removed app registrations are backed up by Sift."
            : string.Empty;
        panel.Children.Add(new TextBlock
        {
            Text = $"Clean {review.Previewed:N0} selected item(s) ({FormatBytes(selection.Sum(x => x.SizeBytes))})? " +
                   $"This permanently deletes their current contents. Files in use may be skipped.{registrationNote}",
            TextWrapping = TextWrapping.Wrap
        });
        var details = new TextBox
        {
            Text = string.Join(Environment.NewLine, review.Log),
            IsReadOnly = true,
            AcceptsReturn = true,
            MinHeight = 120,
            MaxHeight = 210,
            TextWrapping = TextWrapping.Wrap,
            FontFamily = new Microsoft.UI.Xaml.Media.FontFamily("Consolas")
        };
        ScrollViewer.SetVerticalScrollBarVisibility(details, ScrollBarVisibility.Auto);
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(details, "Maintenance cleanup details");
        panel.Children.Add(details);
        return panel;
    }

    private void ApplyFilter()
    {
        if (_binding) return;
        var query = SearchBox.Text.Trim();
        var category = CategoryBox.SelectedItem?.ToString();
        var rows = _all.Where(x =>
            (string.IsNullOrWhiteSpace(query) || x.Title.Contains(query, StringComparison.OrdinalIgnoreCase) ||
             x.Path.Contains(query, StringComparison.OrdinalIgnoreCase) || x.Detail.Contains(query, StringComparison.OrdinalIgnoreCase)) &&
            (string.IsNullOrWhiteSpace(category) || category == "All categories" || x.CategoryLabel == category)).ToList();
        _visible.Clear();
        foreach (var row in rows) _visible.Add(row);
        if (rows.Count == 0) ShowState(_all.Count == 0 ? "No maintenance items" : "No matching items",
            _all.Count == 0 ? "Known locations are currently clear or unavailable." : "Try another category or a broader search.", false);
        else EmptyState.Visibility = Visibility.Collapsed;
        UpdateSelection();
    }

    private void Select(Func<MaintenanceFinding, bool> predicate)
    {
        foreach (var finding in _all) finding.IsSelected = predicate(finding);
        UpdateSelection();
    }

    private void UpdateSelection()
    {
        if (SizeText is null) return;
        var selected = Selected;
        SizeText.Text = $"{selected.Count:N0} selected · {FormatBytes(selected.Sum(x => x.SizeBytes))}";
        CleanButton.Label = "Clean selected";
        CleanButton.IsEnabled = selected.Count > 0 && !BusyRing.IsActive;
    }

    private void ShowState(string title, string detail, bool progress)
    {
        StateTitle.Text = title;
        StateDetail.Text = detail;
        StateRing.IsActive = progress;
        StateRing.Visibility = progress ? Visibility.Visible : Visibility.Collapsed;
        EmptyState.Visibility = Visibility.Visible;
    }

    private static string FormatBytes(long bytes) => Sift.Presentation.SiftDisplay.Bytes(bytes);

    private void SearchBox_TextChanged(object sender, TextChangedEventArgs e) => ApplyFilter();
    private void CategoryBox_SelectionChanged(object sender, SelectionChangedEventArgs e) => ApplyFilter();
    private void SelectRecommendedButton_Click(object sender, RoutedEventArgs e) =>
        Select(x => x.CanClean && !x.RequiresAdvancedConfirm && x.Confidence == MaintenanceConfidence.High);
    private void ClearButton_Click(object sender, RoutedEventArgs e) => Select(_ => false);
    private void ScanButton_Click(object sender, RoutedEventArgs e) => ScanRequested?.Invoke(this, EventArgs.Empty);
    private void CleanButton_Click(object sender, RoutedEventArgs e) => CleanRequested?.Invoke(this, EventArgs.Empty);
}
