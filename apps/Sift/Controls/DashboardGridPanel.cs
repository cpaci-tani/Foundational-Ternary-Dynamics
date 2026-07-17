using Sift.Services;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media.Animation;
using Windows.Foundation;

namespace Sift.WinUI.Controls;

public sealed class DashboardGridPanel : Panel
{
    public static readonly DependencyProperty ColumnsProperty = DependencyProperty.Register(
        nameof(Columns), typeof(int), typeof(DashboardGridPanel),
        new PropertyMetadata(6, LayoutPropertyChanged));

    public static readonly DependencyProperty RowHeightProperty = DependencyProperty.Register(
        nameof(RowHeight), typeof(double), typeof(DashboardGridPanel),
        new PropertyMetadata(96d, LayoutPropertyChanged));

    public static readonly DependencyProperty SpacingProperty = DependencyProperty.Register(
        nameof(Spacing), typeof(double), typeof(DashboardGridPanel),
        new PropertyMetadata(12d, LayoutPropertyChanged));

    public static readonly DependencyProperty RowProperty = DependencyProperty.RegisterAttached(
        "Row", typeof(int), typeof(DashboardGridPanel), new PropertyMetadata(0, ChildLayoutPropertyChanged));
    public static readonly DependencyProperty ColumnProperty = DependencyProperty.RegisterAttached(
        "Column", typeof(int), typeof(DashboardGridPanel), new PropertyMetadata(0, ChildLayoutPropertyChanged));
    public static readonly DependencyProperty RowSpanProperty = DependencyProperty.RegisterAttached(
        "RowSpan", typeof(int), typeof(DashboardGridPanel), new PropertyMetadata(1, ChildLayoutPropertyChanged));
    public static readonly DependencyProperty ColumnSpanProperty = DependencyProperty.RegisterAttached(
        "ColumnSpan", typeof(int), typeof(DashboardGridPanel), new PropertyMetadata(1, ChildLayoutPropertyChanged));

    public DashboardGridPanel()
    {
        ChildrenTransitions = new TransitionCollection { new RepositionThemeTransition() };
    }

    public int Columns
    {
        get => (int)GetValue(ColumnsProperty);
        set => SetValue(ColumnsProperty, value);
    }

    public double RowHeight
    {
        get => (double)GetValue(RowHeightProperty);
        set => SetValue(RowHeightProperty, value);
    }

    public double Spacing
    {
        get => (double)GetValue(SpacingProperty);
        set => SetValue(SpacingProperty, value);
    }

    public static int GetRow(DependencyObject value) => (int)value.GetValue(RowProperty);
    public static void SetRow(DependencyObject value, int row) => value.SetValue(RowProperty, row);
    public static int GetColumn(DependencyObject value) => (int)value.GetValue(ColumnProperty);
    public static void SetColumn(DependencyObject value, int column) => value.SetValue(ColumnProperty, column);
    public static int GetRowSpan(DependencyObject value) => (int)value.GetValue(RowSpanProperty);
    public static void SetRowSpan(DependencyObject value, int span) => value.SetValue(RowSpanProperty, span);
    public static int GetColumnSpan(DependencyObject value) => (int)value.GetValue(ColumnSpanProperty);
    public static void SetColumnSpan(DependencyObject value, int span) => value.SetValue(ColumnSpanProperty, span);

    public (int Row, int Column) CellFromOffset(
        double x,
        double y,
        double availableWidth,
        int? currentRow = null,
        int? currentColumn = null,
        double hysteresis = DashboardGridMath.DefaultHysteresis) =>
        DashboardGridMath.CellFromOffset(
            x, y, availableWidth, Columns, RowHeight, Spacing, currentRow, currentColumn, hysteresis);

    public (int Rows, int Columns) SpanFromSize(
        double width,
        double height,
        double availableWidth,
        int? currentRows = null,
        int? currentColumns = null,
        double hysteresis = DashboardGridMath.DefaultHysteresis) =>
        DashboardGridMath.SpanFromSize(
            width, height, availableWidth, Columns, RowHeight, Spacing, currentRows, currentColumns, hysteresis);

    protected override Size MeasureOverride(Size availableSize)
    {
        var width = double.IsFinite(availableSize.Width) ? Math.Max(0, availableSize.Width) : 0;
        var columns = Math.Clamp(Columns, 1, 6);
        var cellWidth = Math.Max(1, (width - Spacing * (columns - 1)) / columns);
        var maximumRow = 0;
        foreach (var child in Children)
        {
            var rowSpan = Math.Clamp(GetRowSpan(child), 1, 12);
            var columnSpan = Math.Clamp(GetColumnSpan(child), 1, columns);
            child.Measure(new Size(
                cellWidth * columnSpan + Spacing * (columnSpan - 1),
                RowHeight * rowSpan + Spacing * (rowSpan - 1)));
            maximumRow = Math.Max(maximumRow, Math.Max(0, GetRow(child)) + rowSpan);
        }
        var height = maximumRow == 0 ? 0 : maximumRow * RowHeight + (maximumRow - 1) * Spacing;
        return new Size(width, height);
    }

    protected override Size ArrangeOverride(Size finalSize)
    {
        var columns = Math.Clamp(Columns, 1, 6);
        var cellWidth = Math.Max(1, (finalSize.Width - Spacing * (columns - 1)) / columns);
        foreach (var child in Children)
        {
            var row = Math.Max(0, GetRow(child));
            var columnSpan = Math.Clamp(GetColumnSpan(child), 1, columns);
            var column = Math.Clamp(GetColumn(child), 0, columns - columnSpan);
            var rowSpan = Math.Clamp(GetRowSpan(child), 1, 12);
            child.Arrange(new Rect(
                column * (cellWidth + Spacing),
                row * (RowHeight + Spacing),
                cellWidth * columnSpan + Spacing * (columnSpan - 1),
                RowHeight * rowSpan + Spacing * (rowSpan - 1)));
        }
        return finalSize;
    }

    private static void LayoutPropertyChanged(DependencyObject sender, DependencyPropertyChangedEventArgs args)
    {
        if (sender is DashboardGridPanel panel) panel.InvalidateMeasure();
    }

    private static void ChildLayoutPropertyChanged(DependencyObject sender, DependencyPropertyChangedEventArgs args)
    {
        if (sender is FrameworkElement element && element.Parent is DashboardGridPanel panel) panel.InvalidateMeasure();
    }
}
