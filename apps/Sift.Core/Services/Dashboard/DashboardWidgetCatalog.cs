using Sift.Models;

namespace Sift.Services;

public static class DashboardWidgetCatalog
{
    private static readonly IReadOnlyList<DashboardActionKind> Navigate = [DashboardActionKind.Navigate, DashboardActionKind.Refresh];

    public static IReadOnlyList<DashboardWidgetDefinition> Create() =>
    [
        Metric("cpu", "CPU", "Current processor use and history.", "Performance"),
        Metric("memory", "Memory", "Physical memory use and history.", "Performance"),
        Metric("network", "Network", "Aggregate upload and download throughput.", "Performance"),
        Metric("storage", "Storage capacity", "Free space and capacity trends by volume.", "Storage"),
        Metric("uptime", "Uptime", "Boot time, uptime, and Windows version.", "SystemInfo", allowMultiple: false, minColumns: 1),
        Metric("battery", "Battery", "Charge, power state, and available health details.", "SystemInfo", allowMultiple: false),
        Widget("thermals", "Thermals", "CPU and GPU temperature, fan, and power summary.", DashboardWidgetCategory.Hardware,
            false, 2, 6, 2, 6, 3, 2, "HardwareMonitor", [DashboardActionKind.Navigate, DashboardActionKind.Refresh, DashboardActionKind.Pause],
            ["hardware.hottest_c"], DashboardCadence.Fast, ["timeRange", "showActions"]),
        Widget("sensor.chart", "Sensor chart", "History for a selected hardware sensor.", DashboardWidgetCategory.Hardware,
            true, 2, 6, 2, 8, 3, 2, "HardwareMonitor", Navigate,
            ["sensor.*"], DashboardCadence.Fast, ["sensor", "timeRange", "showActions", "preset"]),
        List("topCpu", "Top CPU processes", "Processes using the most CPU.", [DashboardActionKind.Navigate, DashboardActionKind.Refresh, DashboardActionKind.EndProcess, DashboardActionKind.RestartProcess]),
        List("topMem", "Top memory processes", "Processes using the most memory.", [DashboardActionKind.Navigate, DashboardActionKind.Refresh, DashboardActionKind.EndProcess, DashboardActionKind.RestartProcess]),
        List("topIo", "Top I/O processes", "Processes with the highest current I/O.", [DashboardActionKind.Navigate, DashboardActionKind.Refresh, DashboardActionKind.EndProcess, DashboardActionKind.RestartProcess]),
        Widget("services", "Services", "Service state summary and supported actions.", DashboardWidgetCategory.System,
            false, 2, 6, 2, 8, 3, 3, "TaskManager", [DashboardActionKind.Navigate, DashboardActionKind.Refresh, DashboardActionKind.StartService, DashboardActionKind.RestartService],
            ["services.total", "services.running"], DashboardCadence.Medium, ["sort", "count", "filter", "showActions"]),
        Widget("startup", "Startup", "Enabled and disabled startup entries.", DashboardWidgetCategory.System,
            false, 2, 6, 1, 5, 2, 2, "Startup", [DashboardActionKind.Navigate, DashboardActionKind.Refresh, DashboardActionKind.OpenWindowsSettings],
            ["startup.total", "startup.enabled"], DashboardCadence.Medium, ["showActions"]),
        Widget("health", "Health", "Current health checks and warnings.", DashboardWidgetCategory.Health,
            false, 2, 6, 2, 6, 3, 2, "Health", Navigate,
            ["health.warnings", "health.critical", "health.failed"], DashboardCadence.Slow, ["showActions"]),
        Widget("alerts", "Alerts", "Active lifecycle alerts and acknowledgement controls.", DashboardWidgetCategory.Health,
            false, 2, 6, 1, 8, 3, 2, "Home", [DashboardActionKind.AcknowledgeAlert, DashboardActionKind.SnoozeAlert],
            [], DashboardCadence.Fast, ["count", "filter", "showActions"]),
        Widget("maintenance", "Maintenance", "Scan and clean selected maintenance findings.", DashboardWidgetCategory.Maintenance,
            false, 2, 6, 2, 8, 3, 3, "Maintenance", [DashboardActionKind.Navigate, DashboardActionKind.Refresh, DashboardActionKind.MaintenanceCleanup],
            ["maintenance.latest_age_days"], DashboardCadence.Explicit, ["showActions"]),
        Widget("optimize", "Optimize", "Review and apply a selected Sift preset.", DashboardWidgetCategory.Maintenance,
            false, 2, 6, 2, 6, 3, 2, "Optimize", [DashboardActionKind.Navigate, DashboardActionKind.Refresh, DashboardActionKind.OptimizePreset],
            [], DashboardCadence.Explicit, ["preset", "showActions"]),
        Widget("recovery", "Recovery", "Recent backups and recovery status.", DashboardWidgetCategory.Maintenance,
            false, 2, 6, 1, 6, 3, 2, "Recovery", Navigate,
            ["recovery.backups", "recovery.latest_age_days"], DashboardCadence.Slow, ["showActions"]),
        Widget("installedApps", "Installed apps", "Desktop app inventory and cleanup status.", DashboardWidgetCategory.System,
            false, 2, 6, 1, 6, 3, 2, "Apps", Navigate,
            ["apps.total", "apps.uninstallable", "apps.leftovers"], DashboardCadence.Slow, ["showActions"]),
        Widget("activity", "Recent activity", "Recent Sift operations and results.", DashboardWidgetCategory.Activity,
            false, 2, 6, 2, 8, 6, 2, "Health", Navigate),
        Widget("timeline", "Lifecycle timeline", "Health, maintenance, optimization, and recovery history.", DashboardWidgetCategory.Activity,
            false, 3, 6, 2, 10, 6, 2, "Health", Navigate),
        Widget("systemInfo", "System information", "Core device and Windows identity details.", DashboardWidgetCategory.System,
            false, 2, 6, 1, 6, 3, 2, "SystemInfo", Navigate),
        Widget("quickLinks", "Quick links", "Configurable links to Sift workspaces.", DashboardWidgetCategory.Shortcuts,
            true, 1, 6, 1, 6, 2, 2, "Home", [DashboardActionKind.Navigate]),
        Widget("metric.chart", "Metric history", "Configurable live and historical metric chart.", DashboardWidgetCategory.Performance,
            true, 2, 6, 2, 8, 3, 2, "Performance", Navigate,
            ["cataloged metric"], DashboardCadence.Fast, ["metric", "timeRange", "showActions"])
    ];

    public static IReadOnlyDictionary<string, DashboardWidgetDefinition> ById() =>
        Create().ToDictionary(definition => definition.Id, StringComparer.OrdinalIgnoreCase);

    private static DashboardWidgetDefinition Metric(
        string id,
        string title,
        string description,
        string destination,
        bool allowMultiple = false,
        int minColumns = 1) =>
        Widget(id, title, description, DashboardWidgetCategory.Performance, allowMultiple,
            minColumns, 6, 1, 6, 2, 2, destination, Navigate,
            [MetricKey(id)], id is "storage" or "battery" ? DashboardCadence.Medium : DashboardCadence.Fast,
            id == "storage" ? ["timeRange", "volume", "showActions"] : ["timeRange", "showActions"]);

    private static DashboardWidgetDefinition List(
        string id,
        string title,
        string description,
        IReadOnlyList<DashboardActionKind> actions) =>
        Widget(id, title, description, DashboardWidgetCategory.System, false,
            2, 6, 2, 8, 3, 3, "TaskManager", actions, [], DashboardCadence.Fast,
            ["sort", "count", "filter", "showActions"]);

    private static DashboardWidgetDefinition Widget(
        string id,
        string title,
        string description,
        DashboardWidgetCategory category,
        bool allowMultiple,
        int minColumns,
        int maxColumns,
        int minRows,
        int maxRows,
        int defaultColumns,
        int defaultRows,
        string destination,
        IReadOnlyList<DashboardActionKind> actions,
        IReadOnlyList<string>? metrics = null,
        DashboardCadence cadence = DashboardCadence.Explicit,
        IReadOnlyList<string>? settingKeys = null) =>
        new(id, title, description, category, allowMultiple, minColumns, maxColumns,
            minRows, maxRows, defaultColumns, defaultRows, destination, actions,
            metrics ?? [], cadence, settingKeys ?? ["showActions"]);

    private static string MetricKey(string id) => id switch
    {
        "cpu" => "cpu.percent",
        "memory" => "memory.percent",
        "network" => "network.download_mbps",
        "storage" => "storage.lowest_free_percent",
        "uptime" => "system.uptime_hours",
        "battery" => "battery.charge_percent",
        _ => id
    };
}
