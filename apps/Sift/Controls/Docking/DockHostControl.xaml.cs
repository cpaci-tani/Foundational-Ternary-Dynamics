using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using Sift.Models;
using Sift.Services;

namespace Sift.WinUI.Controls;

public sealed partial class DockHostControl : UserControl
{
    private readonly Dictionary<string, FrameworkElement> _boards = new(StringComparer.OrdinalIgnoreCase);
    private IDockSession? _session;
    private IDockBoardPresenter? _presenter;
    private DockNode? _root;
    private bool _embedded = true;
    private string? _floatingSiteId;
    private object? _boardData;
    private string _caption = "Dock";
    private bool _rebuilding;

    public event EventHandler<string>? PopOutRequested;
    public event EventHandler? LayoutMutated;

    public DockHostControl()
    {
        InitializeComponent();
    }

    public void BindEmbedded(IDockSession session, IDockBoardPresenter presenter, string caption = "Dock")
    {
        _session = session;
        _presenter = presenter;
        _embedded = true;
        _floatingSiteId = null;
        _caption = caption;
        _root = session.Layout.EmbeddedRoot;
        PopOutButton.Visibility = Visibility.Visible;
        CaptionText.Text = caption;
        Rebuild();
    }

    public void BindFloating(IDockSession session, IDockBoardPresenter presenter, FloatingDockSite site, string caption = "Floating dock")
    {
        _session = session;
        _presenter = presenter;
        _embedded = false;
        _floatingSiteId = site.Id;
        _caption = caption;
        _root = site.Root;
        PopOutButton.Visibility = Visibility.Collapsed;
        CaptionText.Text = caption;
        Rebuild();
    }

    public void ShowDropOverlay(bool visible) =>
        DropOverlay.Visibility = visible ? Visibility.Visible : Visibility.Collapsed;

    public DockDropZone ResolveDropZone(Windows.Foundation.Point positionRelativeToOverlay)
    {
        var width = Math.Max(1, DropOverlay.ActualWidth);
        var height = Math.Max(1, DropOverlay.ActualHeight);
        var x = positionRelativeToOverlay.X / width;
        var y = positionRelativeToOverlay.Y / height;
        if (x < 0.25) return DockDropZone.Left;
        if (x > 0.75) return DockDropZone.Right;
        if (y < 0.25) return DockDropZone.Top;
        if (y > 0.75) return DockDropZone.Bottom;
        return DockDropZone.Tab;
    }

    public void ApplyData(object? data)
    {
        _boardData = data;
        if (_presenter is null) return;
        foreach (var board in _boards.Values) _presenter.ApplyData(board, data);
        UpdateCount();
    }

    public void ForEachBoard(Action<FrameworkElement> action)
    {
        ArgumentNullException.ThrowIfNull(action);
        foreach (var board in _boards.Values) action(board);
    }

    public void RefreshFromSession()
    {
        if (_session is null) return;
        UpdateRootFromSession();
        if (!TryRefreshBoardsInPlace())
            Rebuild();
        ApplyData(_boardData);
    }

    private void UpdateRootFromSession()
    {
        if (_session is null) return;
        if (_embedded) _root = _session.Layout.EmbeddedRoot;
        else if (_floatingSiteId is not null)
        {
            var site = _session.Layout.FloatingSites.FirstOrDefault(value =>
                value.Id.Equals(_floatingSiteId, StringComparison.OrdinalIgnoreCase));
            if (site is null) return;
            _root = site.Root;
        }
    }

    /// <summary>
    /// Tile add/remove/move only mutates board contents. Rebind cached leaf views instead of rebuilding tabs/splits.
    /// </summary>
    private bool TryRefreshBoardsInPlace()
    {
        if (_session is null || _root is null || _presenter is null || RootPresenter.Content is null)
            return false;

        var boardsInRoot = DockLayoutEngine.EnumerateBoards(_root).ToList();
        if (boardsInRoot.Count == 0 || boardsInRoot.Any(board => !_boards.ContainsKey(board.Id)))
            return false;

        foreach (var board in boardsInRoot)
            _presenter.BindBoard(_boards[board.Id], _session, board);

        CaptionText.Text = _caption;
        UpdateCount();
        return true;
    }

    private void Rebuild()
    {
        if (_rebuilding) return;
        _rebuilding = true;
        try
        {
            foreach (var board in _boards.Values)
                DetachFromParent(board);

            RootPresenter.Content = null;
            _boards.Clear();

            if (_session is null || _root is null || _presenter is null)
                return;

            RootPresenter.Content = BuildNode(_root);
            CaptionText.Text = _caption;
            UpdateCount();
        }
        finally
        {
            _rebuilding = false;
        }
    }

    private static void DetachFromParent(FrameworkElement element)
    {
        DependencyObject? node = element;
        while (node is not null)
        {
            var parent = VisualTreeHelper.GetParent(node);
            switch (parent)
            {
                case Panel panel when node is UIElement ui && panel.Children.Contains(ui):
                    panel.Children.Remove(ui);
                    return;
                case ContentControl content when ReferenceEquals(content.Content, node) || ReferenceEquals(content.Content, element):
                    content.Content = null;
                    return;
                case Border border when ReferenceEquals(border.Child, node) || ReferenceEquals(border.Child, element):
                    border.Child = null;
                    return;
            }

            node = parent;
        }
    }

    private FrameworkElement BuildNode(DockNode node) => node switch
    {
        DockBoardNode board => BuildBoard(board),
        DockTabGroupNode tabs => BuildTabs(tabs),
        DockSplitNode split => BuildSplit(split),
        _ => new TextBlock { Text = "Unsupported dock node", Style = (Style)Application.Current.Resources["TypeMetaStyle"] }
    };

    private FrameworkElement BuildBoard(DockBoardNode board)
    {
        if (!_boards.TryGetValue(board.Id, out var view))
        {
            view = _presenter!.CreateBoard(_session!, board);
            _boards[board.Id] = view;
        }
        DetachFromParent(view);
        _presenter!.BindBoard(view, _session!, board);
        return view;
    }

    private FrameworkElement BuildTabs(DockTabGroupNode tabs)
    {
        var pivot = new Pivot();
        for (var index = 0; index < tabs.Tabs.Count; index++)
        {
            var child = tabs.Tabs[index];
            var header = child switch
            {
                DockBoardNode board => board.Title,
                DockSplitNode => "Split",
                DockTabGroupNode => "Tabs",
                _ => "Panel"
            };
            var content = BuildNode(child);
            DetachFromParent(content);
            pivot.Items.Add(new PivotItem
            {
                Header = header,
                Content = content
            });
        }
        if (tabs.Tabs.Count > 0)
            pivot.SelectedIndex = Math.Clamp(tabs.ActiveIndex, 0, tabs.Tabs.Count - 1);
        pivot.SelectionChanged += (_, _) =>
        {
            if (_session is null || pivot.SelectedIndex < 0) return;
            tabs.ActiveIndex = pivot.SelectedIndex;
            if (tabs.Tabs[pivot.SelectedIndex] is DockBoardNode board)
                _session.SetActiveBoard(board.Id);
        };
        return pivot;
    }

    private FrameworkElement BuildSplit(DockSplitNode split)
    {
        var grid = new Grid { ColumnSpacing = 8, RowSpacing = 8 };
        if (split.Orientation == DockSplitOrientation.Horizontal)
        {
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(split.Ratio, GridUnitType.Star) });
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1 - split.Ratio, GridUnitType.Star) });
            var first = BuildNode(split.First);
            var second = BuildNode(split.Second);
            DetachFromParent(first);
            DetachFromParent(second);
            Grid.SetColumn(second, 1);
            grid.Children.Add(first);
            grid.Children.Add(second);
        }
        else
        {
            grid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(split.Ratio, GridUnitType.Star) });
            grid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1 - split.Ratio, GridUnitType.Star) });
            var first = BuildNode(split.First);
            var second = BuildNode(split.Second);
            DetachFromParent(first);
            DetachFromParent(second);
            Grid.SetRow(second, 1);
            grid.Children.Add(first);
            grid.Children.Add(second);
        }
        return grid;
    }

    private void UpdateCount()
    {
        if (_session is null) { CountText.Text = string.Empty; return; }
        var count = DockLayoutEngine.CountTiles(_session.Layout);
        CountText.Text = $"{count}/{_session.Layout.MaximumTiles}";
    }

    private void TidyButton_Click(object sender, RoutedEventArgs e)
    {
        if (_session is null) return;
        var board = DockLayoutEngine.ResolveActiveBoard(_session.Layout);
        _session.TidyBoard(board.Id);
        RefreshFromSession();
        LayoutMutated?.Invoke(this, EventArgs.Empty);
    }

    private void PopOutButton_Click(object sender, RoutedEventArgs e)
    {
        if (_session is null || !_embedded) return;
        var board = DockLayoutEngine.ResolveActiveBoard(_session.Layout);
        PopOutRequested?.Invoke(this, board.Id);
    }
}
