using System.Numerics;
using Sift.Models;
using Sift.Services;
using Microsoft.UI.Input;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Automation;
using Microsoft.UI.Xaml.Automation.Peers;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;

namespace Sift.WinUI.Controls;

public sealed record DashboardWidgetMoveRequest(string InstanceId, int Row, int Column);
public sealed record DashboardWidgetResizeRequest(string InstanceId, int RowSpan, int ColumnSpan);
public sealed record DashboardWidgetHideRequest(string InstanceId, bool AllBreakpoints);
public sealed record DashboardWidgetPlacementRequest(
    string InstanceId, int Row, int Column, int RowSpan, int ColumnSpan);
public sealed record DashboardWidgetSnapPreview(
    string InstanceId, int Row, int Column, int RowSpan, int ColumnSpan, bool Active);

public sealed partial class DashboardWidgetHost : UserControl
{
    private uint? _pointerId;
    private Windows.Foundation.Point _start;
    private bool _resizing;
    private Vector3 _translation;
    private DashboardPlacement? _keyboardPreview;
    private int? _snapRow;
    private int? _snapColumn;
    private int? _snapRowSpan;
    private int? _snapColumnSpan;

    public DashboardWidgetHost()
    {
        InitializeComponent();
    }

    public string InstanceId { get; private set; } = string.Empty;
    public DashboardWidgetDefinition? Definition { get; private set; }
    public DashboardPlacement? Placement { get; private set; }
    public int LayoutColumns { get; private set; } = 6;

    public event EventHandler<DashboardWidgetMoveRequest>? MoveRequested;
    public event EventHandler<DashboardWidgetResizeRequest>? ResizeRequested;
    public event EventHandler<DashboardWidgetHideRequest>? HideRequested;
    public event EventHandler<string>? DuplicateRequested;
    public event EventHandler<string>? ConfigureRequested;
    public event EventHandler<string>? OpenRequested;
    public event EventHandler<string>? ResetRequested;
    public event EventHandler<DashboardWidgetPlacementRequest>? KeyboardPlacementRequested;
    public event EventHandler<DashboardWidgetSnapPreview>? SnapPreviewChanged;

    public void Configure(
        DashboardWidgetInstance instance,
        DashboardWidgetDefinition definition,
        DashboardPlacement placement,
        int layoutColumns,
        bool customizing,
        DashboardDensity density)
    {
        InstanceId = instance.InstanceId;
        Definition = definition;
        Placement = placement;
        LayoutColumns = layoutColumns;
        var tokens = DashboardDensityTokens.For(density);
        CardBorder.Padding = new Thickness(tokens.HostPadding);
        HostGrid.RowSpacing = tokens.HostRowSpacing;
        HeaderGrid.ColumnSpacing = tokens.HostRowSpacing;
        TitleText.FontSize = tokens.TitleFontSize;
        SizeText.FontSize = tokens.MetaFontSize;
        DragHandle.Width = density == DashboardDensity.Compact ? 26 : 30;
        DragHandle.Height = DragHandle.Width;
        MenuButton.Width = density == DashboardDensity.Compact ? 30 : 34;
        MenuButton.Height = density == DashboardDensity.Compact ? 26 : 30;
        SetText(TitleText, string.IsNullOrWhiteSpace(instance.TitleOverride) ? definition.Title : instance.TitleOverride);
        SetText(SizeText, $"{placement.ColumnSpan} column{(placement.ColumnSpan == 1 ? "" : "s")} × {placement.RowSpan} row{(placement.RowSpan == 1 ? "" : "s")}");
        DragHandle.Visibility = customizing ? Visibility.Visible : Visibility.Collapsed;
        ResizeHandle.Visibility = customizing ? Visibility.Visible : Visibility.Collapsed;
        SizeText.Visibility = customizing ? Visibility.Visible : Visibility.Collapsed;
        CardBorder.BorderThickness = customizing ? new Thickness(2) : new Thickness(1);
        AutomationProperties.SetName(this, $"{TitleText.Text} dashboard widget, {SizeText.Text}");
    }

    public void SetContent(UIElement content)
    {
        if (!ReferenceEquals(ContentHost.Content, content)) ContentHost.Content = content;
    }

    private void MenuButton_Click(object sender, RoutedEventArgs e)
    {
        if (Definition is null || Placement is null) return;
        var menu = new MenuFlyout();
        var width = new MenuFlyoutSubItem { Text = "Columns" };
        for (var value = 1; value <= LayoutColumns; value++)
        {
            var span = value;
            var item = new ToggleMenuFlyoutItem
            {
                Text = value.ToString(),
                IsChecked = Placement.ColumnSpan == value,
                IsEnabled = value >= Math.Min(Definition.MinColumnSpan, LayoutColumns) &&
                            value <= Math.Min(Definition.MaxColumnSpan, LayoutColumns)
            };
            item.Click += (_, _) => ResizeRequested?.Invoke(this,
                new DashboardWidgetResizeRequest(InstanceId, Placement.RowSpan, span));
            width.Items.Add(item);
        }
        menu.Items.Add(width);

        var height = new MenuFlyoutSubItem { Text = "Rows" };
        for (var value = Definition.MinRowSpan; value <= Definition.MaxRowSpan; value++)
        {
            var span = value;
            var item = new ToggleMenuFlyoutItem { Text = value.ToString(), IsChecked = Placement.RowSpan == value };
            item.Click += (_, _) => ResizeRequested?.Invoke(this,
                new DashboardWidgetResizeRequest(InstanceId, span, Placement.ColumnSpan));
            height.Items.Add(item);
        }
        menu.Items.Add(height);
        menu.Items.Add(new MenuFlyoutSeparator());
        Add(menu, "Configure", () => ConfigureRequested?.Invoke(this, InstanceId));
        if (Definition.AllowMultiple) Add(menu, "Duplicate", () => DuplicateRequested?.Invoke(this, InstanceId));
        Add(menu, "Hide in this layout", () => HideRequested?.Invoke(this, new DashboardWidgetHideRequest(InstanceId, false)));
        Add(menu, "Hide in all layouts", () => HideRequested?.Invoke(this, new DashboardWidgetHideRequest(InstanceId, true)));
        Add(menu, "Reset widget", () => ResetRequested?.Invoke(this, InstanceId));
        menu.Items.Add(new MenuFlyoutSeparator());
        Add(menu, $"Open {Definition.DestinationWorkspace}", () => OpenRequested?.Invoke(this, InstanceId));
        menu.ShowAt(MenuButton);
    }

    private static void Add(MenuFlyout menu, string text, Action action)
    {
        var item = new MenuFlyoutItem { Text = text };
        item.Click += (_, _) => action();
        menu.Items.Add(item);
    }

    private void DragHandle_KeyDown(object sender, KeyRoutedEventArgs e)
    {
        if (Placement is null || Definition is null) return;
        if (e.Key == Windows.System.VirtualKey.Space && _keyboardPreview is null)
        {
            _keyboardPreview = Placement.Copy();
            RaiseSnapPreview(_keyboardPreview.Row, _keyboardPreview.Column, _keyboardPreview.RowSpan, _keyboardPreview.ColumnSpan, true);
            Announce($"Picked up {TitleText.Text}. Row {_keyboardPreview.Row + 1}, column {_keyboardPreview.Column + 1}, {Size(_keyboardPreview)}. Use arrows to move, Shift plus arrows to resize, Enter to drop, or Escape to cancel.");
            e.Handled = true;
            return;
        }
        if (_keyboardPreview is null) return;
        if (e.Key == Windows.System.VirtualKey.Escape)
        {
            CancelKeyboardEdit("Move cancelled.");
            e.Handled = true;
            return;
        }
        if (e.Key == Windows.System.VirtualKey.Enter)
        {
            var placement = _keyboardPreview;
            _keyboardPreview = null;
            Translation = Vector3.Zero;
            Opacity = 1;
            ClearSnapPreview();
            KeyboardPlacementRequested?.Invoke(this, new DashboardWidgetPlacementRequest(
                InstanceId, placement.Row, placement.Column, placement.RowSpan, placement.ColumnSpan));
            Announce($"Dropped {TitleText.Text}. Row {placement.Row + 1}, column {placement.Column + 1}, {Size(placement)}.");
            e.Handled = true;
            return;
        }

        var horizontal = e.Key is Windows.System.VirtualKey.Left or Windows.System.VirtualKey.Right;
        var vertical = e.Key is Windows.System.VirtualKey.Up or Windows.System.VirtualKey.Down;
        if (!horizontal && !vertical) return;
        var direction = e.Key is Windows.System.VirtualKey.Left or Windows.System.VirtualKey.Up ? -1 : 1;
        var shift = Microsoft.UI.Input.InputKeyboardSource.GetKeyStateForCurrentThread(Windows.System.VirtualKey.Shift)
            .HasFlag(Windows.UI.Core.CoreVirtualKeyStates.Down);
        if (shift)
        {
            if (horizontal)
                _keyboardPreview.ColumnSpan = Math.Clamp(_keyboardPreview.ColumnSpan + direction,
                    Math.Min(Definition.MinColumnSpan, LayoutColumns), Math.Min(Definition.MaxColumnSpan, LayoutColumns));
            else
                _keyboardPreview.RowSpan = Math.Clamp(_keyboardPreview.RowSpan + direction,
                    Definition.MinRowSpan, Definition.MaxRowSpan);
            _keyboardPreview.Column = Math.Min(_keyboardPreview.Column, LayoutColumns - _keyboardPreview.ColumnSpan);
        }
        else
        {
            if (horizontal)
                _keyboardPreview.Column = Math.Clamp(_keyboardPreview.Column + direction, 0,
                    LayoutColumns - _keyboardPreview.ColumnSpan);
            else _keyboardPreview.Row = Math.Max(0, _keyboardPreview.Row + direction);
        }
        UpdateKeyboardTranslation();
        RaiseSnapPreview(_keyboardPreview.Row, _keyboardPreview.Column, _keyboardPreview.RowSpan, _keyboardPreview.ColumnSpan, true);
        Announce($"Row {_keyboardPreview.Row + 1}, column {_keyboardPreview.Column + 1}, {Size(_keyboardPreview)}.");
        e.Handled = true;
    }

    private void UpdateKeyboardTranslation()
    {
        if (_keyboardPreview is null || Placement is null || Parent is not DashboardGridPanel grid) return;
        var cellWidth = DashboardGridMath.CellWidth(grid.ActualWidth, LayoutColumns, grid.Spacing);
        Translation = new Vector3(
            (float)((_keyboardPreview.Column - Placement.Column) * (cellWidth + grid.Spacing)),
            (float)((_keyboardPreview.Row - Placement.Row) * (grid.RowHeight + grid.Spacing)), 12);
        Opacity = 0.88;
    }

    private void CancelKeyboardEdit(string announcement)
    {
        _keyboardPreview = null;
        Translation = Vector3.Zero;
        Opacity = 1;
        ClearSnapPreview();
        Announce(announcement);
    }

    private void Announce(string value)
    {
        SizeText.Text = value;
        AutomationProperties.SetLiveSetting(SizeText, AutomationLiveSetting.Assertive);
        FrameworkElementAutomationPeer.FromElement(SizeText)?.RaiseAutomationEvent(AutomationEvents.LiveRegionChanged);
    }

    private static string Size(DashboardPlacement placement) =>
        $"{placement.ColumnSpan} column{(placement.ColumnSpan == 1 ? "" : "s")} by {placement.RowSpan} row{(placement.RowSpan == 1 ? "" : "s")}";

    private void DragHandle_PointerPressed(object sender, PointerRoutedEventArgs e) => BeginPointer(DragHandle, e, resizing: false);
    private void ResizeHandle_PointerPressed(object sender, PointerRoutedEventArgs e) => BeginPointer(ResizeHandle, e, resizing: true);
    private void DragHandle_PointerMoved(object sender, PointerRoutedEventArgs e) => MovePointer(e);
    private void ResizeHandle_PointerMoved(object sender, PointerRoutedEventArgs e) => MovePointer(e);
    private void DragHandle_PointerReleased(object sender, PointerRoutedEventArgs e) => EndPointer(e, cancelled: false);
    private void ResizeHandle_PointerReleased(object sender, PointerRoutedEventArgs e) => EndPointer(e, cancelled: false);
    private void DragHandle_PointerCanceled(object sender, PointerRoutedEventArgs e) => EndPointer(e, cancelled: true);
    private void ResizeHandle_PointerCanceled(object sender, PointerRoutedEventArgs e) => EndPointer(e, cancelled: true);
    private void InteractionHandle_PointerCaptureLost(object sender, PointerRoutedEventArgs e)
    {
        if (_pointerId == e.Pointer.PointerId) ResetPointerState();
    }

    private void BeginPointer(UIElement source, PointerRoutedEventArgs e, bool resizing)
    {
        _pointerId = e.Pointer.PointerId;
        _start = e.GetCurrentPoint(Parent as UIElement ?? this).Position;
        _resizing = resizing;
        if (Placement is not null)
        {
            _snapRow = Placement.Row;
            _snapColumn = Placement.Column;
            _snapRowSpan = Placement.RowSpan;
            _snapColumnSpan = Placement.ColumnSpan;
            RaiseSnapPreview(Placement.Row, Placement.Column, Placement.RowSpan, Placement.ColumnSpan, true);
        }
        source.CapturePointer(e.Pointer);
        ProtectedCursor = InputSystemCursor.Create(InputSystemCursorShape.SizeAll);
        e.Handled = true;
    }

    private void MovePointer(PointerRoutedEventArgs e)
    {
        if (_pointerId != e.Pointer.PointerId) return;
        var point = e.GetCurrentPoint(Parent as UIElement ?? this).Position;
        var x = (float)(point.X - _start.X);
        var y = (float)(point.Y - _start.Y);
        _translation = new Vector3(x, y, 12);
        Translation = _translation;
        Opacity = 0.88;
        UpdatePointerSnapPreview();
        e.Handled = true;
    }

    private void UpdatePointerSnapPreview()
    {
        if (Placement is null || Parent is not DashboardGridPanel grid) return;
        if (_resizing)
        {
            var width = Math.Max(1, ActualWidth + _translation.X);
            var height = Math.Max(1, ActualHeight + _translation.Y);
            var span = grid.SpanFromSize(width, height, grid.ActualWidth, _snapRowSpan, _snapColumnSpan);
            _snapRowSpan = span.Rows;
            _snapColumnSpan = span.Columns;
            RaiseSnapPreview(Placement.Row, Placement.Column, span.Rows, span.Columns, true);
            SetText(SizeText, $"{span.Columns} column{(span.Columns == 1 ? "" : "s")} × {span.Rows} row{(span.Rows == 1 ? "" : "s")}");
            return;
        }

        var columns = Math.Clamp(LayoutColumns, 1, 6);
        var cellWidth = DashboardGridMath.CellWidth(grid.ActualWidth, columns, grid.Spacing);
        var originX = Placement.Column * (cellWidth + grid.Spacing) + _translation.X;
        var originY = Placement.Row * (grid.RowHeight + grid.Spacing) + _translation.Y;
        var cell = grid.CellFromOffset(originX, originY, grid.ActualWidth, _snapRow, _snapColumn);
        var column = Math.Clamp(cell.Column, 0, columns - Placement.ColumnSpan);
        _snapRow = cell.Row;
        _snapColumn = column;
        RaiseSnapPreview(cell.Row, column, Placement.RowSpan, Placement.ColumnSpan, true);
        SetText(SizeText, $"Row {cell.Row + 1}, column {column + 1}");
    }

    private void EndPointer(PointerRoutedEventArgs e, bool cancelled)
    {
        if (_pointerId != e.Pointer.PointerId) return;
        var resizing = _resizing;
        var snapRow = _snapRow;
        var snapColumn = _snapColumn;
        var snapRowSpan = _snapRowSpan;
        var snapColumnSpan = _snapColumnSpan;
        _pointerId = null;
        DragHandle.ReleasePointerCapture(e.Pointer);
        ResizeHandle.ReleasePointerCapture(e.Pointer);
        ResetPointerState();
        if (!cancelled && Placement is not null)
        {
            if (resizing && snapRowSpan is { } rows && snapColumnSpan is { } cols)
                ResizeRequested?.Invoke(this, new DashboardWidgetResizeRequest(InstanceId, rows, cols));
            else if (!resizing && snapRow is { } row && snapColumn is { } column)
                MoveRequested?.Invoke(this, new DashboardWidgetMoveRequest(InstanceId, row, column));
        }
        e.Handled = true;
    }

    private void ResetPointerState()
    {
        _pointerId = null;
        _translation = Vector3.Zero;
        Translation = Vector3.Zero;
        Opacity = 1;
        ProtectedCursor = null;
        _resizing = false;
        _snapRow = null;
        _snapColumn = null;
        _snapRowSpan = null;
        _snapColumnSpan = null;
        ClearSnapPreview();
    }

    private void RaiseSnapPreview(int row, int column, int rowSpan, int columnSpan, bool active) =>
        SnapPreviewChanged?.Invoke(this,
            new DashboardWidgetSnapPreview(InstanceId, row, column, rowSpan, columnSpan, active));

    private void ClearSnapPreview() =>
        SnapPreviewChanged?.Invoke(this, new DashboardWidgetSnapPreview(InstanceId, 0, 0, 1, 1, false));

    private static void SetText(TextBlock target, string value)
    {
        if (!string.Equals(target.Text, value, StringComparison.Ordinal)) target.Text = value;
    }
}
