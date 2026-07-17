using System.Collections.ObjectModel;
using Sift.Models;
using Sift.WinUI.Controls;
using Sift.WinUI.Infrastructure.Dialogs;
using Sift.WinUI.Models;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;

namespace Sift.WinUI.Views;

public sealed partial class StorageWorkspaceView : UserControl
{
    private readonly ObservableCollection<StorageMapRow> _largest = [];
    private readonly ObservableCollection<StorageLegendRow> _legend = [];
    private StorageTree? _tree;
    private int _currentIndex = -1;
    private int _selectedIndex = -1;
    private bool _busy;

    public StorageWorkspaceView()
    {
        InitializeComponent();
        LargestList.ItemsSource = _largest;
        LegendList.ItemsSource = _legend;
        Treemap.NodeInvoked += Treemap_NodeInvoked;
        Treemap.RenderCompleted += Treemap_RenderCompleted;
    }

    public event EventHandler? ScanRequested;
    public event EventHandler? CancelRequested;
    public event EventHandler? BrowseRequested;
    public event EventHandler? DeleteRequested;
    public string SelectedRoot => (RootBox.Text ?? RootBox.SelectedItem?.ToString() ?? "").Trim();
    public int SelectedNodeIndex => _selectedIndex;

    public void ConfigureRoots(IReadOnlyList<string> roots, string? selected)
    {
        RootBox.ItemsSource = roots;
        RootBox.Text = !string.IsNullOrWhiteSpace(selected) ? selected : roots.FirstOrDefault() ?? "";
        SetIdle("Choose a drive or paste a folder path, then start an explicit scan.");
    }

    public void SetSelectedRoot(string path)
    {
        RootBox.Text = path;
        StatusText.Text = "Folder selected. Press Scan storage to build a new map.";
    }

    public void Bind(StorageTree tree)
    {
        _tree = tree;
        _selectedIndex = -1;
        SizeText.Text = StorageRow.FormatSize(tree.TotalSize);
        FilesText.Text = tree.TotalFiles.ToString("N0");
        NodesText.Text = tree.Nodes.Count.ToString("N0");
        BuildLegend(tree);
        if (tree.RootIndices.Count == 0)
        {
            _currentIndex = -1;
            Treemap.Clear("Nothing readable was found", "Check the path and your access permissions.");
            UpdateDeleteButton();
            return;
        }
        ShowNode(tree.RootIndices[0]);
    }

    public void SetBusy(bool busy, string status)
    {
        _busy = busy;
        BusyRing.IsActive = busy;
        ScanButton.IsEnabled = !busy;
        BrowseButton.IsEnabled = !busy;
        RootBox.IsEnabled = !busy;
        CancelButton.Visibility = busy ? Visibility.Visible : Visibility.Collapsed;
        StatusText.Text = status;
        UpdateDeleteButton();
    }

    public void SetIdle(string status)
    {
        SetBusy(false, status);
        if (_tree is null) Treemap.Clear();
    }

    public void ReportProgress(StorageScanProgress progress) => StatusText.Text = progress.Message;
    public void FocusRoot() => RootBox.Focus(FocusState.Programmatic);

    public async Task<bool> ConfirmDeleteAsync(StorageSelectionDeletePreflight preflight)
    {
        var details = new TextBlock
        {
            Text = $"Type: {preflight.TypeDisplay}{Environment.NewLine}Size: {preflight.SizeDisplay}{Environment.NewLine}Files: {preflight.FileCount:N0}{Environment.NewLine}Folders: {preflight.DirectoryCount:N0}{Environment.NewLine}Path: {preflight.TargetPath}{Environment.NewLine}{preflight.Detail}",
            TextWrapping = TextWrapping.Wrap,
            FontFamily = new FontFamily("Consolas"),
            FontSize = 12,
            LineHeight = 19,
            Foreground = (Brush)Application.Current.Resources["SiftMutedBrush"]
        };
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(details, "Selected storage item details");
        var border = new Border
        {
            Background = (Brush)Application.Current.Resources["SiftPanelBrush"],
            BorderBrush = (Brush)Application.Current.Resources["SiftLineBrush"],
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(7),
            Padding = new Thickness(12, 10, 12, 10),
            Child = details
        };
        var panel = new StackPanel { Spacing = 10, MaxWidth = 660 };
        panel.Children.Add(new TextBlock
        {
            Text = "The selected item will be checked again before it is moved. If its contents change, the operation will stop.",
            TextWrapping = TextWrapping.Wrap
        });
        panel.Children.Add(border);
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Move the selected item to the Recycle Bin?",
            Content = panel,
            PrimaryButtonText = "Move to Recycle Bin",
            CloseButtonText = "Cancel"
        };
        ConfirmationDialogStyle.Apply(dialog);
        dialog.PrimaryButtonStyle = (Style)Application.Current.Resources["DangerButtonStyle"];
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    private void ShowNode(int index)
    {
        if (_tree is null || index < 0 || index >= _tree.Nodes.Count) return;
        var node = _tree.Nodes[index];
        if (!node.IsDirectory) { ShowDetails(node); return; }
        _currentIndex = index;
        CurrentPathText.Text = node.FullPath;
        UpButton.IsEnabled = node.ParentIndex >= 0;
        Treemap.Bind(_tree, index);
        ShowDetails(node);
        _largest.Clear();
        foreach (var child in node.Children.Select(child => _tree.Nodes[child]).Where(child => child.Size > 0)
                     .OrderByDescending(child => child.Size).Take(15))
        {
            var percent = node.Size <= 0 ? 0 : child.Size * 100d / node.Size;
            _largest.Add(new StorageMapRow(child.Index, child.Name,
                child.IsReparsePoint ? "Link" : child.IsDirectory ? $"Folder · {child.FileCount:N0} files" : StorageTree.ExtensionOf(child.Name),
                StorageRow.FormatSize(child.Size), $"{percent:0.0}%", child.FullPath));
        }
    }

    private void ShowDetails(StorageNode node)
    {
        _selectedIndex = node.Index;
        SelectedNameText.Text = node.Name;
        SelectedSizeText.Text = $"{StorageRow.FormatSize(node.Size)} · {node.FileCount:N0} file(s)";
        SelectedPathText.Text = node.FullPath;
        UpdateDeleteButton();
    }

    private void UpdateDeleteButton()
    {
        var node = _tree is not null && _selectedIndex >= 0 && _selectedIndex < _tree.Nodes.Count
            ? _tree.Nodes[_selectedIndex]
            : null;
        DeleteButton.IsEnabled = !_busy && node is not null && node.ParentIndex >= 0 && !node.IsReparsePoint;
    }

    private void BuildLegend(StorageTree tree)
    {
        _legend.Clear();
        foreach (var item in tree.ExtensionBytes.OrderByDescending(x => x.Value).Take(14))
            _legend.Add(new StorageLegendRow(item.Key, StorageRow.FormatSize(item.Value),
                new SolidColorBrush(ParseColor(StorageTree.ColorHexForExtension(item.Key)))));
    }

    private void Treemap_NodeInvoked(object? sender, StorageNodeInvokedEventArgs e)
    {
        if (_tree is null) return;
        var node = _tree.Nodes[e.NodeIndex];
        if (node.IsDirectory && !node.IsReparsePoint) ShowNode(node.Index);
        else ShowDetails(node);
    }

    private void Treemap_RenderCompleted(object? sender, EventArgs e) =>
        MapMetaText.Text = Treemap.IsTruncated
            ? $"{Treemap.RenderedNodeCount:N0} largest tiles shown"
            : $"{Treemap.RenderedNodeCount:N0} tiles";

    private void LargestList_ItemClick(object sender, ItemClickEventArgs e)
    {
        if (e.ClickedItem is StorageMapRow row) ShowNode(row.NodeIndex);
    }

    private void UpButton_Click(object sender, RoutedEventArgs e)
    {
        if (_tree is null || _currentIndex < 0) return;
        var parent = _tree.Nodes[_currentIndex].ParentIndex;
        if (parent >= 0) ShowNode(parent);
    }

    private static Windows.UI.Color ParseColor(string hex)
    {
        var value = hex.TrimStart('#');
        return Windows.UI.Color.FromArgb(255,
            Convert.ToByte(value[..2], 16), Convert.ToByte(value.Substring(2, 2), 16), Convert.ToByte(value.Substring(4, 2), 16));
    }

    private void ScanButton_Click(object sender, RoutedEventArgs e) => ScanRequested?.Invoke(this, EventArgs.Empty);
    private void CancelButton_Click(object sender, RoutedEventArgs e) => CancelRequested?.Invoke(this, EventArgs.Empty);
    private void BrowseButton_Click(object sender, RoutedEventArgs e) => BrowseRequested?.Invoke(this, EventArgs.Empty);
    private void DeleteButton_Click(object sender, RoutedEventArgs e) => DeleteRequested?.Invoke(this, EventArgs.Empty);
}
