using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Sift.Models;
using Sift.Services;

namespace Sift.WinUI.Controls;

public sealed partial class GraphBoardView : UserControl, IDockBoardView
{
    private readonly Dictionary<string, SensorGraphTileHost> _tiles = new(StringComparer.OrdinalIgnoreCase);
    private IDockSession? _session;
    private DockBoardNode? _board;
    private IReadOnlyDictionary<string, string>? _valueLabels;
    private SensorHistoryStore? _history;

    public GraphBoardView()
    {
        InitializeComponent();
    }

    public string BoardId => _board?.Id ?? string.Empty;

    public void Bind(IDockSession session, DockBoardNode board, SensorHistoryStore history)
    {
        _session = session;
        _board = DockLayoutEngine.FindBoard(session.Layout, board.Id) ?? board;
        _history = history;
        TileGrid.Columns = Math.Clamp(_board.Columns, 1, 6);
        Rebuild();
    }

    public void ApplyHistories(IReadOnlyDictionary<string, string>? valueLabels = null)
    {
        if (_history is null) return;
        _valueLabels = valueLabels;
        foreach (var host in _tiles.Values)
        {
            var label = valueLabels?.GetValueOrDefault(host.SensorId);
            host.ApplyHistory(_history.GetValues(host.SensorId), label);
        }
    }

    public void ApplyChartPreferences(HardwareChartPreferences preferences)
    {
        foreach (var host in _tiles.Values)
            host.ApplyChartPreferences(preferences);
    }

    private void Rebuild()
    {
        if (_session is null || _board is null || _history is null) return;
        var visible = _board.Tiles.Select(tile => tile.InstanceId).ToHashSet(StringComparer.OrdinalIgnoreCase);
        foreach (var stale in _tiles.Keys.Where(id => !visible.Contains(id)).ToList())
        {
            TileGrid.Children.Remove(_tiles[stale]);
            _tiles.Remove(stale);
        }

        foreach (var tile in _board.Tiles.Where(tile =>
                     tile.ContentType.Equals(DockContentTypes.HardwareSensor, StringComparison.OrdinalIgnoreCase)))
        {
            if (!_tiles.TryGetValue(tile.InstanceId, out var host))
            {
                host = new SensorGraphTileHost();
                host.MoveRequested += Host_MoveRequested;
                host.ResizeRequested += Host_ResizeRequested;
                host.CloseRequested += Host_CloseRequested;
                host.SnapPreviewChanged += Host_SnapPreviewChanged;
                _tiles[tile.InstanceId] = host;
                TileGrid.Children.Add(host);
            }
            host.Configure(_board.Id, tile);
            DashboardGridPanel.SetRow(host, tile.Row);
            DashboardGridPanel.SetColumn(host, tile.Column);
            DashboardGridPanel.SetRowSpan(host, tile.RowSpan);
            DashboardGridPanel.SetColumnSpan(host, tile.ColumnSpan);
            var label = _valueLabels?.GetValueOrDefault(tile.ContentKey);
            host.ApplyHistory(_history.GetValues(tile.ContentKey), label);
        }

        EmptyState.Visibility = _board.Tiles.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
    }

    private void Host_MoveRequested(object? sender, SensorGraphTileMoveRequest request)
    {
        if (_session is null || _board is null) return;
        var boardId = _board.Id;
        var tile = _board.Tiles.Single(value => value.InstanceId == request.InstanceId);
        var instanceId = request.InstanceId;
        var row = request.Row;
        var column = request.Column;
        var rowSpan = tile.RowSpan;
        var columnSpan = tile.ColumnSpan;
        DispatcherQueue.TryEnqueue(() => _session.PlaceTile(boardId, instanceId, row, column, rowSpan, columnSpan));
    }

    private void Host_ResizeRequested(object? sender, SensorGraphTileResizeRequest request)
    {
        if (_session is null || _board is null) return;
        var boardId = _board.Id;
        var tile = _board.Tiles.Single(value => value.InstanceId == request.InstanceId);
        var instanceId = request.InstanceId;
        var row = tile.Row;
        var column = tile.Column;
        var rowSpan = request.RowSpan;
        var columnSpan = request.ColumnSpan;
        DispatcherQueue.TryEnqueue(() => _session.PlaceTile(boardId, instanceId, row, column, rowSpan, columnSpan));
    }

    private void Host_CloseRequested(object? sender, SensorGraphTileCloseRequest request)
    {
        if (_session is null) return;
        var instanceId = request.InstanceId;
        // Defer so the tile host is not torn down during its close-button click handler.
        DispatcherQueue.TryEnqueue(() => _session.RemoveTile(instanceId));
    }

    private void Host_SnapPreviewChanged(object? sender, DashboardWidgetSnapPreview preview)
    {
        if (!preview.Active)
        {
            SnapGhost.Visibility = Visibility.Collapsed;
            return;
        }
        var columns = Math.Clamp(TileGrid.Columns, 1, 6);
        var cellWidth = DashboardGridMath.CellWidth(TileGrid.ActualWidth, columns, TileGrid.Spacing);
        var columnSpan = Math.Clamp(preview.ColumnSpan, 1, columns);
        var column = Math.Clamp(preview.Column, 0, columns - columnSpan);
        var rowSpan = Math.Clamp(preview.RowSpan, 1, 12);
        SnapGhost.Width = cellWidth * columnSpan + TileGrid.Spacing * (columnSpan - 1);
        SnapGhost.Height = TileGrid.RowHeight * rowSpan + TileGrid.Spacing * (rowSpan - 1);
        SnapGhost.Margin = new Thickness(
            column * (cellWidth + TileGrid.Spacing),
            preview.Row * (TileGrid.RowHeight + TileGrid.Spacing),
            0, 0);
        SnapGhost.Visibility = Visibility.Visible;
    }
}
