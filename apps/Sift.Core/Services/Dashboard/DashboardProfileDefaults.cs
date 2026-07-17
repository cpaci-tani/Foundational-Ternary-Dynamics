using Sift.Models;

namespace Sift.Services;

public static class DashboardProfileDefaults
{
    public static DashboardProfileDocument Create(IReadOnlyDictionary<string, bool>? legacyVisibility = null)
    {
        var document = new DashboardProfileDocument
        {
            Profiles =
            [
                CreateProfile("overview", "Overview", DashboardDensity.Default,
                [
                    Spec("systemInfo", 0, 0, 6, 1),
                    Spec("cpu", 1, 0, 2, 2), Spec("memory", 1, 2, 2, 2), Spec("storage", 1, 4, 2, 2),
                    Spec("thermals", 3, 0, 3, 2), Spec("alerts", 3, 3, 3, 2),
                    Spec("topCpu", 5, 0, 3, 3), Spec("health", 5, 3, 3, 2),
                    Spec("activity", 8, 0, 6, 2)
                ], legacyVisibility),
                CreateProfile("troubleshooting", "Troubleshooting", DashboardDensity.Compact,
                [
                    Spec("alerts", 0, 0, 6, 1),
                    Spec("cpu", 1, 0, 2, 2), Spec("memory", 1, 2, 2, 2), Spec("network", 1, 4, 2, 2),
                    Spec("thermals", 3, 0, 3, 2), Spec("storage", 3, 3, 3, 2),
                    Spec("topCpu", 5, 0, 3, 3), Spec("topMem", 5, 3, 3, 3),
                    Spec("services", 8, 0, 3, 3), Spec("activity", 8, 3, 3, 3),
                    Spec("recovery", 11, 0, 3, 2), Spec("systemInfo", 11, 3, 3, 2)
                ]),
                CreateProfile("gaming", "Gaming", DashboardDensity.Compact,
                [
                    Spec("cpu", 0, 0, 2, 2), Spec("memory", 0, 2, 2, 2), Spec("network", 0, 4, 2, 2),
                    Spec("thermals", 2, 0, 3, 2), Spec("sensor.chart", 2, 3, 3, 2, "gpu"),
                    Spec("topCpu", 4, 0, 3, 3), Spec("topIo", 4, 3, 3, 3),
                    Spec("alerts", 7, 0, 3, 2), Spec("quickLinks", 7, 3, 3, 2)
                ]),
                CreateProfile("minimal", "Minimal", DashboardDensity.Comfortable,
                [
                    Spec("systemInfo", 0, 0, 6, 1),
                    Spec("cpu", 1, 0, 2, 2), Spec("memory", 1, 2, 2, 2), Spec("storage", 1, 4, 2, 2),
                    Spec("alerts", 3, 0, 6, 1)
                ])
            ]
        };
        return document;
    }

    private static DashboardProfile CreateProfile(
        string id,
        string name,
        DashboardDensity density,
        IReadOnlyList<PlacementSpec> specs,
        IReadOnlyDictionary<string, bool>? legacyVisibility = null)
    {
        var definitions = DashboardWidgetCatalog.ById();
        var widgets = new List<DashboardWidgetInstance>();
        var placements = new List<DashboardPlacement>();
        foreach (var spec in specs)
        {
            var definition = definitions[spec.DefinitionId];
            var instanceId = $"{id}.{spec.DefinitionId}{(string.IsNullOrWhiteSpace(spec.Suffix) ? string.Empty : "." + spec.Suffix)}";
            widgets.Add(new DashboardWidgetInstance
            {
                InstanceId = instanceId,
                DefinitionId = definition.Id,
                Settings = string.IsNullOrWhiteSpace(spec.Suffix)
                    ? new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
                    : new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase) { ["preset"] = spec.Suffix }
            });
            placements.Add(new DashboardPlacement
            {
                InstanceId = instanceId,
                Row = spec.Row,
                Column = spec.Column,
                RowSpan = spec.RowSpan,
                ColumnSpan = spec.ColumnSpan,
                Visible = LegacyVisible(definition.Id, legacyVisibility)
            });
        }

        var wide = new DashboardBreakpointLayout
        {
            Breakpoint = DashboardBreakpoint.Wide,
            Columns = 6,
            Placements = placements
        };
        return new DashboardProfile
        {
            Id = id,
            Name = name,
            IsBuiltIn = true,
            Density = density,
            Widgets = widgets,
            Layouts =
            [
                wide,
                DashboardPackingEngine.Reflow(wide, DashboardBreakpoint.Medium, 4),
                DashboardPackingEngine.Reflow(wide, DashboardBreakpoint.Compact, 2)
            ]
        };
    }

    private static bool LegacyVisible(string definitionId, IReadOnlyDictionary<string, bool>? legacy)
    {
        if (legacy is null) return true;
        var key = definitionId switch
        {
            "storage" => legacy.ContainsKey("storage") ? "storage" : "disk",
            "topCpu" => "topCpu",
            "topMem" => "topMem",
            "services" => "services",
            "startup" => "startup",
            "maintenance" => "maintenance",
            "optimize" => "optimize",
            "activity" => "activity",
            _ => definitionId
        };
        return !legacy.TryGetValue(key, out var visible) || visible;
    }

    private static PlacementSpec Spec(
        string definitionId,
        int row,
        int column,
        int columnSpan,
        int rowSpan,
        string? suffix = null) => new(definitionId, row, column, columnSpan, rowSpan, suffix);

    private sealed record PlacementSpec(
        string DefinitionId,
        int Row,
        int Column,
        int ColumnSpan,
        int RowSpan,
        string? Suffix);
}
