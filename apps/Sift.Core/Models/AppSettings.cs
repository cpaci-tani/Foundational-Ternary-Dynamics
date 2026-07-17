namespace Sift.Models;

public sealed class AppSettings
{
    public const int CurrentSchemaVersion = 1;
    public int SchemaVersion { get; set; } = CurrentSchemaVersion;
    public string RefreshInterval { get; set; } = "2 seconds";
    public int ChartHistory { get; set; } = 120;
    public string ChartSmoothing { get; set; } = "Light";
    public bool PerformanceShowLegend { get; set; } = true;
    public bool PerformanceShowCpuSeries { get; set; } = true;
    public bool PerformanceShowMemorySeries { get; set; } = true;
    /// <summary>Shell content scale: Compact, Default, or Large.</summary>
    public string UiScale { get; set; } = "Default";
    public Dictionary<string, bool> HomeWidgets { get; set; } = new(StringComparer.OrdinalIgnoreCase)
    {
        ["cpu"] = true,
        ["memory"] = true,
        ["disk"] = true,
        ["topCpu"] = true,
        ["topMem"] = true,
        ["services"] = true,
        ["startup"] = true,
        ["storage"] = true,
        ["maintenance"] = true,
        ["optimize"] = true,
        ["activity"] = true,
    };
    public DashboardPreferences Dashboard { get; set; } = new();
    public HardwareChartPreferences HardwareCharts { get; set; } = new();
    public bool OfferSystemRestorePoint { get; set; } = true;
    public string? LastWorkspace { get; set; } = "Home";
    public string? LastMaintenanceScanUtc { get; set; }
    public string? LastStorageScanUtc { get; set; }
    public List<string> StorageRoots { get; set; } = [];
    public bool ConsoleVisible { get; set; } = true;
    public double ConsoleWidth { get; set; } = 360;
}
