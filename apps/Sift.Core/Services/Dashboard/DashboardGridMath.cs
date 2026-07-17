namespace Sift.Services;

/// <summary>
/// Pure grid snap math for the Home dashboard. Hysteresis keeps the active cell until
/// the pointer crosses 0.5 ± hysteresis of a cell boundary, reducing boundary flicker.
/// </summary>
public static class DashboardGridMath
{
    public const double DefaultHysteresis = 0.35;

    public static double CellWidth(double availableWidth, int columns, double spacing)
    {
        columns = Math.Clamp(columns, 1, 6);
        return Math.Max(1, (availableWidth - spacing * (columns - 1)) / columns);
    }

    public static int SnapIndex(double fractional, int? current, double hysteresis = DefaultHysteresis)
    {
        var nearest = (int)Math.Round(fractional, MidpointRounding.ToEven);
        if (current is null) return nearest;

        if (nearest == current.Value) return current.Value;

        var boundary = nearest > current.Value
            ? current.Value + 0.5
            : current.Value - 0.5;
        var threshold = hysteresis;

        if (nearest > current.Value)
            return fractional >= boundary + threshold ? nearest : current.Value;

        return fractional <= boundary - threshold ? nearest : current.Value;
    }

    public static (int Row, int Column) CellFromOffset(
        double x,
        double y,
        double availableWidth,
        int columns,
        double rowHeight,
        double spacing,
        int? currentRow = null,
        int? currentColumn = null,
        double hysteresis = DefaultHysteresis)
    {
        columns = Math.Clamp(columns, 1, 6);
        var cellWidth = CellWidth(availableWidth, columns, spacing);
        var strideX = cellWidth + spacing;
        var strideY = rowHeight + spacing;
        var column = Math.Clamp(
            SnapIndex(x / strideX, currentColumn, hysteresis),
            0,
            columns - 1);
        var row = Math.Max(0, SnapIndex(y / strideY, currentRow, hysteresis));
        return (row, column);
    }

    public static (int Rows, int Columns) SpanFromSize(
        double width,
        double height,
        double availableWidth,
        int columns,
        double rowHeight,
        double spacing,
        int? currentRows = null,
        int? currentColumns = null,
        double hysteresis = DefaultHysteresis)
    {
        columns = Math.Clamp(columns, 1, 6);
        var cellWidth = CellWidth(availableWidth, columns, spacing);
        var strideX = cellWidth + spacing;
        var strideY = rowHeight + spacing;
        var columnSpan = Math.Clamp(
            SnapIndex((width + spacing) / strideX, currentColumns, hysteresis),
            1,
            columns);
        var rowSpan = Math.Clamp(
            SnapIndex((height + spacing) / strideY, currentRows, hysteresis),
            1,
            12);
        return (rowSpan, columnSpan);
    }
}
