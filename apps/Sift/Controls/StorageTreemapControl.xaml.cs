using Sift.Models;
using Microsoft.UI;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Automation;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;

namespace Sift.WinUI.Controls;

public sealed class StorageNodeInvokedEventArgs(int nodeIndex) : EventArgs
{
    public int NodeIndex { get; } = nodeIndex;
}

public sealed partial class StorageTreemapControl : UserControl
{
    private const int MaximumVisuals = 1_200;
    private const int MaximumChildrenPerLevel = 96;
    private const int MaximumDepth = 5;
    private StorageTree? _tree;
    private int _rootIndex = -1;
    private int _visualCount;

    public StorageTreemapControl() => InitializeComponent();

    public event EventHandler<StorageNodeInvokedEventArgs>? NodeInvoked;
    public event EventHandler? RenderCompleted;
    public int RenderedNodeCount => _visualCount;
    public bool IsTruncated { get; private set; }

    public void Bind(StorageTree tree, int rootIndex)
    {
        _tree = tree;
        _rootIndex = rootIndex;
        Render();
    }

    public void Clear(string title = "Scan a folder or drive", string? detail = null)
    {
        _tree = null;
        _rootIndex = -1;
        Surface.Children.Clear();
        EmptyTitle.Text = title;
        EmptyDetail.Text = detail ?? "The disk map appears here after an explicit local scan.";
        EmptyState.Visibility = Visibility.Visible;
    }

    private void Render()
    {
        Surface.Children.Clear();
        _visualCount = 0;
        IsTruncated = false;
        if (_tree is null || _rootIndex < 0 || Surface.ActualWidth < 20 || Surface.ActualHeight < 20)
        {
            EmptyState.Visibility = Visibility.Visible;
            return;
        }

        var root = _tree.Nodes[_rootIndex];
        if (root.Children.Count == 0 || root.Size <= 0)
        {
            EmptyTitle.Text = "Nothing to map";
            EmptyDetail.Text = "The selected location contains no readable, non-link files.";
            EmptyState.Visibility = Visibility.Visible;
            return;
        }

        EmptyState.Visibility = Visibility.Collapsed;
        RenderChildren(root, new TreemapBounds(4, 4, Surface.ActualWidth - 8, Surface.ActualHeight - 8), 0);
        RenderCompleted?.Invoke(this, EventArgs.Empty);
    }

    private void RenderChildren(StorageNode parent, TreemapBounds bounds, int depth)
    {
        if (_tree is null || _visualCount >= MaximumVisuals || bounds.Width < 3 || bounds.Height < 3) return;
        var children = parent.Children
            .Select(index => _tree.Nodes[index])
            .Where(node => node.Size > 0)
            .OrderByDescending(node => node.Size)
            .ToList();
        if (children.Count > MaximumChildrenPerLevel)
        {
            children = children.Take(MaximumChildrenPerLevel).ToList();
            IsTruncated = true;
        }

        var layouts = SquarifiedTreemap.Layout(
            children.Select(node => new TreemapWeightedItem(node.Index, node.Size)), bounds);
        foreach (var layout in layouts)
        {
            if (_visualCount >= MaximumVisuals)
            {
                IsTruncated = true;
                break;
            }
            var node = _tree.Nodes[layout.Id];
            var tile = Inset(layout.Bounds, 1);
            if (tile.Width < 1 || tile.Height < 1) continue;
            AddTile(node, tile);
            _visualCount++;

            if (node.IsDirectory && !node.IsReparsePoint && node.Children.Count > 0 && depth < MaximumDepth &&
                tile.Width >= 72 && tile.Height >= 54 && _visualCount < MaximumVisuals)
            {
                var childBounds = new TreemapBounds(tile.X + 3, tile.Y + 21,
                    Math.Max(0, tile.Width - 6), Math.Max(0, tile.Height - 24));
                RenderChildren(node, childBounds, depth + 1);
            }
        }
    }

    private void AddTile(StorageNode node, TreemapBounds bounds)
    {
        var color = node.IsDirectory
            ? ParseColor(StorageTree.ColorHexForExtension(node.Name), 95)
            : ParseColor(StorageTree.ColorHexForExtension(StorageTree.ExtensionOf(node.Name)), 225);
        var tileButton = new Button
        {
            Width = bounds.Width,
            Height = bounds.Height,
            Background = new SolidColorBrush(color),
            BorderBrush = node.IsDirectory
                ? (Brush)Application.Current.Resources["SiftLineStrongBrush"]
                : new SolidColorBrush(ParseColor("#151311", 170)),
            BorderThickness = new Thickness(node.IsDirectory ? 1 : 0.5),
            CornerRadius = new CornerRadius(node.IsDirectory ? 3 : 1),
            Padding = new Thickness(0),
            HorizontalContentAlignment = HorizontalAlignment.Stretch,
            VerticalContentAlignment = VerticalAlignment.Stretch,
            UseSystemFocusVisuals = true,
            Tag = node.Index
        };
        AutomationProperties.SetName(tileButton,
            $"{(node.IsDirectory ? "Folder" : "File")} {node.Name}, {StorageRow.FormatSize(node.Size)}");
        AutomationProperties.SetHelpText(tileButton, node.FullPath);
        tileButton.Click += Tile_Click;
        tileButton.PointerEntered += Tile_PointerEntered;
        tileButton.PointerExited += Tile_PointerExited;
        Canvas.SetLeft(tileButton, bounds.X);
        Canvas.SetTop(tileButton, bounds.Y);
        ToolTipService.SetToolTip(tileButton, CreateToolTip(node));

        if (bounds.Width >= 68 && bounds.Height >= (node.IsDirectory ? 22 : 34))
        {
            tileButton.Content = new TextBlock
            {
                Text = node.IsDirectory
                    ? node.Name
                    : $"{node.Name}{Environment.NewLine}{StorageRow.FormatSize(node.Size)}",
                Margin = new Thickness(6, 3, 5, 3),
                FontSize = node.IsDirectory ? 10 : 9,
                FontWeight = node.IsDirectory ? Microsoft.UI.Text.FontWeights.SemiBold : Microsoft.UI.Text.FontWeights.Normal,
                Foreground = (Brush)Application.Current.Resources["SiftTextBrush"],
                TextTrimming = TextTrimming.CharacterEllipsis,
                MaxLines = node.IsDirectory ? 1 : 2,
                IsHitTestVisible = false
            };
        }
        Surface.Children.Add(tileButton);
    }

    private void Tile_Click(object sender, RoutedEventArgs e)
    {
        if (sender is FrameworkElement { Tag: int index })
            NodeInvoked?.Invoke(this, new StorageNodeInvokedEventArgs(index));
    }

    private void Tile_PointerEntered(object sender, PointerRoutedEventArgs e)
    {
        if (sender is not Button button) return;
        button.BorderBrush = (Brush)Application.Current.Resources["SiftAccentBrush"];
        button.BorderThickness = new Thickness(2);
    }

    private void Tile_PointerExited(object sender, PointerRoutedEventArgs e)
    {
        if (sender is not Button { Tag: int index } button || _tree is null) return;
        var node = _tree.Nodes[index];
        button.BorderBrush = node.IsDirectory
            ? (Brush)Application.Current.Resources["SiftLineStrongBrush"]
            : new SolidColorBrush(ParseColor("#151311", 170));
        button.BorderThickness = new Thickness(node.IsDirectory ? 1 : 0.5);
    }

    private string CreateToolTip(StorageNode node)
    {
        var rootSize = _tree is null || _rootIndex < 0 ? 0 : _tree.Nodes[_rootIndex].Size;
        var percent = rootSize <= 0 ? 0 : node.Size * 100d / rootSize;
        var type = node.IsReparsePoint ? "Link (not traversed)" : node.IsDirectory ? "Folder" : StorageTree.ExtensionOf(node.Name);
        var modified = node.LastWriteUtc == default ? "Unknown" : node.LastWriteUtc.ToLocalTime().ToString("g");
        return $"{node.Name}{Environment.NewLine}" +
               $"{type} · {StorageRow.FormatSize(node.Size)} · {percent:0.0}% of this view{Environment.NewLine}" +
               $"{node.FileCount:N0} file(s) · modified {modified}{Environment.NewLine}" +
               node.FullPath;
    }

    private static TreemapBounds Inset(TreemapBounds value, double amount) => new(
        value.X + amount, value.Y + amount,
        Math.Max(0, value.Width - amount * 2), Math.Max(0, value.Height - amount * 2));

    private static Windows.UI.Color ParseColor(string hex, byte alpha)
    {
        var value = hex.TrimStart('#');
        return Windows.UI.Color.FromArgb(alpha,
            Convert.ToByte(value[..2], 16), Convert.ToByte(value.Substring(2, 2), 16), Convert.ToByte(value.Substring(4, 2), 16));
    }

    private void Surface_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        if (_tree is not null) DispatcherQueue.TryEnqueue(Render);
    }
}
