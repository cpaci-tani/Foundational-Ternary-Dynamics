using System.Collections.ObjectModel;
using System.Numerics;
using LiveChartsCore;
using LiveChartsCore.SkiaSharpView;
using Microsoft.UI.Input;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure;

namespace Sift.WinUI.Controls;

public sealed record SensorGraphTileMoveRequest(string InstanceId, int Row, int Column);
public sealed record SensorGraphTileResizeRequest(string InstanceId, int RowSpan, int ColumnSpan);
public sealed record SensorGraphTileCloseRequest(string InstanceId);

public sealed partial class SensorGraphTileHost : UserControl
{
    private readonly ObservableCollection<double> _values = [];
    private readonly LineSeries<double> _series;
    private uint? _pointerId;
    private UIElement? _captureSource;
    private Windows.Foundation.Point _start;
    private bool _resizing;
    private Vector3 _translation;
    private int? _snapRow;
    private int? _snapColumn;
    private int? _snapRowSpan;
    private int? _snapColumnSpan;
    private string _sensorType = string.Empty;

    public SensorGraphTileHost()
    {
        InitializeComponent();
        _series = new LineSeries<double>
        {
            Values = _values,
            GeometrySize = 0,
            LineSmoothness = ChartSmoothingPolicy.ResolveSmoothness(ChartSmoothingPolicy.Default),
            Stroke = ChartTheme.Stroke(ChartTheme.Clay, 1.75f),
            Fill = ChartTheme.Fill(ChartTheme.Clay)
        };
        Chart.Series = new ISeries[] { _series };
        Chart.AnimationsSpeed = TimeSpan.Zero;
        ChartTheme.ApplyChrome(Chart, showLegend: false, showAxes: false);
    }

    public string InstanceId { get; private set; } = string.Empty;
    public string SensorId { get; private set; } = string.Empty;
    public string BoardId { get; private set; } = string.Empty;
    public DockTile? Tile { get; private set; }

    public event EventHandler<SensorGraphTileMoveRequest>? MoveRequested;
    public event EventHandler<SensorGraphTileResizeRequest>? ResizeRequested;
    public event EventHandler<SensorGraphTileCloseRequest>? CloseRequested;
    public event EventHandler<DashboardWidgetSnapPreview>? SnapPreviewChanged;

    public void Configure(string boardId, DockTile tile)
    {
        BoardId = boardId;
        Tile = tile;
        InstanceId = tile.InstanceId;
        SensorId = tile.ContentKey;
        TitleText.Text = tile.Title;
        _sensorType = tile.Metadata.GetValueOrDefault("sensorType") ?? string.Empty;
        _series.Name = tile.Title;
        ApplySeriesColor();
    }

    public void ApplyChartPreferences(HardwareChartPreferences preferences)
    {
        _series.LineSmoothness = ChartSmoothingPolicy.ResolveSmoothness(preferences.ChartSmoothing);
        ApplySeriesColor();
        ChartTheme.ApplyChrome(Chart, preferences.ShowLegend, preferences.ShowAxes);
    }

    public void ApplyHistory(IReadOnlyList<double> values, string? valueLabel = null)
    {
        SyncCollection(_values, values);
        if (!string.IsNullOrWhiteSpace(valueLabel)) ValueText.Text = valueLabel;
    }

    private void ApplySeriesColor()
    {
        var color = ChartTheme.ForSensorType(_sensorType);
        _series.Stroke = ChartTheme.Stroke(color, 1.75f);
        _series.Fill = ChartTheme.Fill(color);
    }

    private static void SyncCollection(ObservableCollection<double> target, IReadOnlyList<double> values)
    {
        var shared = Math.Min(target.Count, values.Count);
        for (var index = 0; index < shared; index++)
            if (Math.Abs(target[index] - values[index]) > 0.000001) target[index] = values[index];
        while (target.Count > values.Count) target.RemoveAt(target.Count - 1);
        for (var index = target.Count; index < values.Count; index++) target.Add(values[index]);
    }

    private void CloseButton_Click(object sender, RoutedEventArgs e) =>
        CloseRequested?.Invoke(this, new SensorGraphTileCloseRequest(InstanceId));

    private void DragHandle_PointerPressed(object sender, PointerRoutedEventArgs e) => BeginPointer(DragHandle, e, false);
    private void ResizeHandle_PointerPressed(object sender, PointerRoutedEventArgs e) => BeginPointer(ResizeHandle, e, true);
    private void DragHandle_PointerMoved(object sender, PointerRoutedEventArgs e) => MovePointer(e);
    private void ResizeHandle_PointerMoved(object sender, PointerRoutedEventArgs e) => MovePointer(e);
    private void DragHandle_PointerReleased(object sender, PointerRoutedEventArgs e) => EndPointer(e, false);
    private void ResizeHandle_PointerReleased(object sender, PointerRoutedEventArgs e) => EndPointer(e, false);
    private void DragHandle_PointerCanceled(object sender, PointerRoutedEventArgs e) => EndPointer(e, true);
    private void ResizeHandle_PointerCanceled(object sender, PointerRoutedEventArgs e) => EndPointer(e, true);
    private void InteractionHandle_PointerCaptureLost(object sender, PointerRoutedEventArgs e)
    {
        if (_pointerId == e.Pointer.PointerId) ResetPointerState();
    }

    private void BeginPointer(UIElement source, PointerRoutedEventArgs e, bool resizing)
    {
        if (e.Pointer.PointerDeviceType == Microsoft.UI.Input.PointerDeviceType.Mouse &&
            !e.GetCurrentPoint(source).Properties.IsLeftButtonPressed)
            return;

        _pointerId = e.Pointer.PointerId;
        _start = e.GetCurrentPoint(Parent as UIElement ?? this).Position;
        _resizing = resizing;
        if (Tile is not null)
        {
            _snapRow = Tile.Row;
            _snapColumn = Tile.Column;
            _snapRowSpan = Tile.RowSpan;
            _snapColumnSpan = Tile.ColumnSpan;
            RaiseSnap(Tile.Row, Tile.Column, Tile.RowSpan, Tile.ColumnSpan, true);
        }
        source.CapturePointer(e.Pointer);
        _captureSource = source;
        ProtectedCursor = InputSystemCursor.Create(InputSystemCursorShape.SizeAll);
        e.Handled = true;
    }

    private void MovePointer(PointerRoutedEventArgs e)
    {
        if (_pointerId != e.Pointer.PointerId || Tile is null) return;
        var point = e.GetCurrentPoint(Parent as UIElement ?? this).Position;
        _translation = new Vector3((float)(point.X - _start.X), (float)(point.Y - _start.Y), 12);
        Translation = _translation;
        Opacity = 0.88;
        if (Parent is not DashboardGridPanel grid) { e.Handled = true; return; }

        if (_resizing)
        {
            var span = grid.SpanFromSize(
                Math.Max(1, ActualWidth + _translation.X),
                Math.Max(1, ActualHeight + _translation.Y),
                grid.ActualWidth, _snapRowSpan, _snapColumnSpan);
            _snapRowSpan = span.Rows;
            _snapColumnSpan = span.Columns;
            RaiseSnap(Tile.Row, Tile.Column, span.Rows, span.Columns, true);
        }
        else
        {
            var cellWidth = DashboardGridMath.CellWidth(grid.ActualWidth, grid.Columns, grid.Spacing);
            var originX = Tile.Column * (cellWidth + grid.Spacing) + _translation.X;
            var originY = Tile.Row * (grid.RowHeight + grid.Spacing) + _translation.Y;
            var cell = grid.CellFromOffset(originX, originY, grid.ActualWidth, _snapRow, _snapColumn);
            var column = Math.Clamp(cell.Column, 0, grid.Columns - Tile.ColumnSpan);
            _snapRow = cell.Row;
            _snapColumn = column;
            RaiseSnap(cell.Row, column, Tile.RowSpan, Tile.ColumnSpan, true);
        }
        e.Handled = true;
    }

    private void EndPointer(PointerRoutedEventArgs e, bool cancelled)
    {
        if (_pointerId != e.Pointer.PointerId) return;
        var resizing = _resizing;
        var row = _snapRow;
        var column = _snapColumn;
        var rowSpan = _snapRowSpan;
        var columnSpan = _snapColumnSpan;
        _pointerId = null;
        _captureSource?.ReleasePointerCapture(e.Pointer);
        _captureSource = null;
        ResetPointerState();
        if (!cancelled)
        {
            if (resizing && rowSpan is { } rows && columnSpan is { } cols)
                ResizeRequested?.Invoke(this, new SensorGraphTileResizeRequest(InstanceId, rows, cols));
            else if (!resizing && row is { } snapRow && column is { } snapColumn)
                MoveRequested?.Invoke(this, new SensorGraphTileMoveRequest(InstanceId, snapRow, snapColumn));
        }
        e.Handled = true;
    }

    private void ResetPointerState()
    {
        _pointerId = null;
        _captureSource = null;
        _translation = Vector3.Zero;
        Translation = Vector3.Zero;
        Opacity = 1;
        ProtectedCursor = null;
        _resizing = false;
        _snapRow = _snapColumn = _snapRowSpan = _snapColumnSpan = null;
        RaiseSnap(0, 0, 1, 1, false);
    }

    private void RaiseSnap(int row, int column, int rowSpan, int columnSpan, bool active) =>
        SnapPreviewChanged?.Invoke(this, new DashboardWidgetSnapPreview(InstanceId, row, column, rowSpan, columnSpan, active));
}
