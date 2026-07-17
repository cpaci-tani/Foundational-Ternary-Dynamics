using Sift.Models;

namespace Sift.Services;

public static class DashboardPackingEngine
{
    public static DashboardBreakpointLayout Place(
        DashboardBreakpointLayout source,
        string instanceId,
        int row,
        int column,
        int rowSpan,
        int columnSpan)
    {
        ArgumentNullException.ThrowIfNull(source);
        ArgumentException.ThrowIfNullOrWhiteSpace(instanceId);
        if (source.Columns is < 1 or > 6) throw new ArgumentOutOfRangeException(nameof(source.Columns));

        var placements = source.Placements.Select(placement => placement.Copy()).ToList();
        var target = placements.SingleOrDefault(placement =>
            placement.InstanceId.Equals(instanceId, StringComparison.OrdinalIgnoreCase))
            ?? throw new KeyNotFoundException($"Dashboard widget instance '{instanceId}' is not in this layout.");

        target.RowSpan = Math.Clamp(rowSpan, 1, 12);
        target.ColumnSpan = Math.Clamp(columnSpan, 1, source.Columns);
        target.Row = Math.Max(0, row);
        target.Column = Math.Clamp(column, 0, source.Columns - target.ColumnSpan);

        var resolved = new List<DashboardPlacement> { target };
        foreach (var placement in placements
                     .Where(candidate => !ReferenceEquals(candidate, target))
                     .OrderBy(candidate => candidate.Row)
                     .ThenBy(candidate => candidate.Column)
                     .ThenBy(candidate => candidate.InstanceId, StringComparer.OrdinalIgnoreCase))
        {
            Normalize(placement, source.Columns);
            if (placement.Visible)
            {
                while (resolved.Any(other => other.Visible && Overlaps(placement, other))) placement.Row++;
            }
            resolved.Add(placement);
        }

        return new DashboardBreakpointLayout
        {
            Breakpoint = source.Breakpoint,
            Columns = source.Columns,
            Placements = placements
        };
    }

    public static DashboardBreakpointLayout Tidy(DashboardBreakpointLayout source)
    {
        ArgumentNullException.ThrowIfNull(source);
        var result = new DashboardBreakpointLayout { Breakpoint = source.Breakpoint, Columns = source.Columns };
        var occupied = new List<DashboardPlacement>();
        foreach (var placement in source.Placements.Select(value => value.Copy())
                     .OrderBy(value => value.Row)
                     .ThenBy(value => value.Column)
                     .ThenBy(value => value.InstanceId, StringComparer.OrdinalIgnoreCase))
        {
            Normalize(placement, source.Columns);
            if (!placement.Visible)
            {
                result.Placements.Add(placement);
                continue;
            }

            var placed = false;
            for (var row = 0; !placed; row++)
            {
                for (var column = 0; column <= source.Columns - placement.ColumnSpan; column++)
                {
                    placement.Row = row;
                    placement.Column = column;
                    if (occupied.Any(other => Overlaps(placement, other))) continue;
                    occupied.Add(placement);
                    placed = true;
                    break;
                }
            }
            result.Placements.Add(placement);
        }
        return result;
    }

    public static DashboardBreakpointLayout Reflow(
        DashboardBreakpointLayout source,
        DashboardBreakpoint breakpoint,
        int columns)
    {
        if (columns is < 1 or > 6) throw new ArgumentOutOfRangeException(nameof(columns));
        var next = new DashboardBreakpointLayout
        {
            Breakpoint = breakpoint,
            Columns = columns,
            Placements = source.Placements.Select(placement => placement.Copy()).ToList()
        };
        foreach (var placement in next.Placements) Normalize(placement, columns);
        return Tidy(next);
    }

    public static IReadOnlyList<string> Validate(
        DashboardBreakpointLayout layout,
        IReadOnlyDictionary<string, DashboardWidgetInstance> instances,
        IReadOnlyDictionary<string, DashboardWidgetDefinition> definitions)
    {
        var errors = new List<string>();
        if (layout.Columns is < 1 or > 6) errors.Add($"{layout.Breakpoint} has an invalid column count.");
        var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (var placement in layout.Placements)
        {
            if (!seen.Add(placement.InstanceId)) errors.Add($"Duplicate placement: {placement.InstanceId}.");
            if (!instances.TryGetValue(placement.InstanceId, out var instance))
            {
                errors.Add($"Unknown widget instance: {placement.InstanceId}.");
                continue;
            }
            if (!definitions.TryGetValue(instance.DefinitionId, out var definition))
            {
                errors.Add($"Unknown widget definition: {instance.DefinitionId}.");
                continue;
            }
            if (placement.Row < 0 || placement.Column < 0 || placement.ColumnSpan < 1 || placement.RowSpan < 1 ||
                placement.Column + placement.ColumnSpan > layout.Columns)
                errors.Add($"Placement is outside {layout.Breakpoint}: {placement.InstanceId}.");
            if (placement.ColumnSpan < Math.Min(definition.MinColumnSpan, layout.Columns) ||
                placement.ColumnSpan > Math.Min(definition.MaxColumnSpan, layout.Columns) ||
                placement.RowSpan < definition.MinRowSpan || placement.RowSpan > definition.MaxRowSpan)
                errors.Add($"Placement size is unsupported: {placement.InstanceId}.");
        }
        var visible = layout.Placements.Where(placement => placement.Visible).ToList();
        for (var i = 0; i < visible.Count; i++)
            for (var j = i + 1; j < visible.Count; j++)
                if (Overlaps(visible[i], visible[j])) errors.Add($"Overlapping widgets: {visible[i].InstanceId}, {visible[j].InstanceId}.");
        return errors;
    }

    public static bool Overlaps(DashboardPlacement left, DashboardPlacement right) =>
        left.Column < right.Column + right.ColumnSpan &&
        left.Column + left.ColumnSpan > right.Column &&
        left.Row < right.Row + right.RowSpan &&
        left.Row + left.RowSpan > right.Row;

    private static void Normalize(DashboardPlacement placement, int columns)
    {
        placement.Row = Math.Max(0, placement.Row);
        placement.RowSpan = Math.Clamp(placement.RowSpan, 1, 12);
        placement.ColumnSpan = Math.Clamp(placement.ColumnSpan, 1, columns);
        placement.Column = Math.Clamp(placement.Column, 0, columns - placement.ColumnSpan);
    }
}
