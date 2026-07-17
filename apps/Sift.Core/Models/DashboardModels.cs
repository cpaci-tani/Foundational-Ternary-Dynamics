namespace Sift.Models;

public enum DashboardBreakpoint
{
    Wide,
    Medium,
    Compact
}

public enum DashboardDensity
{
    Compact,
    Default,
    Comfortable
}

public enum DashboardWidgetCategory
{
    Performance,
    Hardware,
    System,
    Health,
    Maintenance,
    Activity,
    Shortcuts
}

public enum DashboardCadence
{
    Fast,
    Medium,
    Slow,
    Explicit
}

public enum DashboardActionKind
{
    Navigate,
    Refresh,
    Pause,
    OptimizePreset,
    MaintenanceCleanup,
    EndProcess,
    RestartProcess,
    StartService,
    RestartService,
    AcknowledgeAlert,
    SnoozeAlert,
    OpenWindowsSettings
}

public sealed record DashboardWidgetDefinition(
    string Id,
    string Title,
    string Description,
    DashboardWidgetCategory Category,
    bool AllowMultiple,
    int MinColumnSpan,
    int MaxColumnSpan,
    int MinRowSpan,
    int MaxRowSpan,
    int DefaultColumnSpan,
    int DefaultRowSpan,
    string DestinationWorkspace,
    IReadOnlyList<DashboardActionKind> Actions,
    IReadOnlyList<string> Metrics,
    DashboardCadence Cadence,
    IReadOnlyList<string> SettingKeys);

public sealed class DashboardWidgetInstance
{
    public string InstanceId { get; init; } = string.Empty;
    public string DefinitionId { get; init; } = string.Empty;
    public string? TitleOverride { get; set; }
    public string? Accent { get; set; }
    public Dictionary<string, string> Settings { get; set; } = new(StringComparer.OrdinalIgnoreCase);
}

public sealed class DashboardPlacement
{
    public string InstanceId { get; init; } = string.Empty;
    public int Row { get; set; }
    public int Column { get; set; }
    public int RowSpan { get; set; } = 1;
    public int ColumnSpan { get; set; } = 1;
    public bool Visible { get; set; } = true;

    public DashboardPlacement Copy() => new()
    {
        InstanceId = InstanceId,
        Row = Row,
        Column = Column,
        RowSpan = RowSpan,
        ColumnSpan = ColumnSpan,
        Visible = Visible
    };
}

public sealed class DashboardBreakpointLayout
{
    public DashboardBreakpoint Breakpoint { get; set; }
    public int Columns { get; set; }
    public List<DashboardPlacement> Placements { get; set; } = [];

    public DashboardBreakpointLayout Copy() => new()
    {
        Breakpoint = Breakpoint,
        Columns = Columns,
        Placements = Placements.Select(placement => placement.Copy()).ToList()
    };
}

public sealed class DashboardProfile
{
    public string Id { get; init; } = string.Empty;
    public string Name { get; set; } = string.Empty;
    public bool IsBuiltIn { get; init; }
    public DashboardDensity Density { get; set; } = DashboardDensity.Default;
    public List<DashboardWidgetInstance> Widgets { get; set; } = [];
    public List<DashboardBreakpointLayout> Layouts { get; set; } = [];
}

public sealed class DashboardProfileDocument
{
    public const int CurrentSchemaVersion = 1;
    public int SchemaVersion { get; set; } = CurrentSchemaVersion;
    public string ActiveProfileId { get; set; } = "overview";
    public List<DashboardProfile> Profiles { get; set; } = [];
}

public sealed class DashboardPreferences
{
    public bool MonitorWhenClosed { get; set; }
    public bool BackgroundHardwareSensors { get; set; }
    public bool NotificationsEnabled { get; set; }
    public int HistoryRetentionDays { get; set; } = 90;
    public TimeOnly QuietHoursStart { get; set; } = new(22, 0);
    public TimeOnly QuietHoursEnd { get; set; } = new(8, 0);
    public List<DashboardAlertRule> AlertRules { get; set; } = [];

    /// <summary>Default chart legend visibility for Home metric widgets.</summary>
    public bool ChartShowLegend { get; set; } = true;

    /// <summary>Default chart axis visibility for Home metric widgets large enough to show a body.</summary>
    public bool ChartShowAxes { get; set; } = true;

    /// <summary>Default line smoothing for Home metric widgets.</summary>
    public string ChartSmoothing { get; set; } = "Light";
}

/// <summary>Hardware workspace chart sampling and display preferences.</summary>
public sealed class HardwareChartPreferences
{
    public string RefreshInterval { get; set; } = "2 seconds";
    public int HistorySamples { get; set; } = 180;
    public string ChartSmoothing { get; set; } = "Light";
    public bool ShowLegend { get; set; }
    public bool ShowAxes { get; set; }
}

public sealed class DashboardAlertRule
{
    public string Id { get; init; } = string.Empty;
    public string MetricKey { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public double Threshold { get; set; }
    public bool TriggerWhenBelow { get; set; }
    public TimeSpan RequiredDuration { get; set; }
    public double Hysteresis { get; set; }
    public TimeSpan Cooldown { get; set; } = TimeSpan.FromMinutes(30);
    public string Severity { get; set; } = "Warning";
    public bool Enabled { get; set; } = true;
    public bool ToastEnabled { get; set; }
}

public sealed record DashboardMetricSample(
    string Key,
    double Value,
    string Unit,
    DateTimeOffset TimestampUtc,
    double? Minimum = null,
    double? Maximum = null);

public sealed record DashboardHistoryPoint(
    string MetricKey,
    DateTimeOffset BucketUtc,
    TimeSpan Resolution,
    double Minimum,
    double Maximum,
    double Average,
    long SampleCount);

public sealed record DashboardSnapshotDelta(
    long Generation,
    DateTimeOffset TimestampUtc,
    IReadOnlyDictionary<string, DashboardMetricSample> Metrics,
    IReadOnlyList<ProcessSnapshot> TopProcesses,
    IReadOnlyList<string> Warnings,
    IReadOnlyList<DashboardServiceSnapshot>? Services = null,
    IReadOnlyList<string>? ChangedMetricKeys = null)
{
    public bool HasChangedMetric(string metricKey) =>
        (ChangedMetricKeys ?? Metrics.Keys.ToList()).Contains(metricKey, StringComparer.OrdinalIgnoreCase);

    public IReadOnlyList<DashboardMetricSample> GetChangedMetrics()
    {
        var keys = ChangedMetricKeys ?? Metrics.Keys.ToList();
        return keys.Distinct(StringComparer.OrdinalIgnoreCase)
            .Where(Metrics.ContainsKey)
            .Select(key => Metrics[key])
            .ToList();
    }
}

public enum DashboardTelemetryOrigin
{
    InProcess,
    MonitorHost
}

public sealed record DashboardTelemetryRead(
    DashboardSnapshotDelta Snapshot,
    DashboardTelemetryOrigin Origin,
    bool IsFresh,
    bool SourceHealthy,
    bool SourceOwnsHistory,
    bool SourceOwnsAlerts,
    string Status,
    IReadOnlyList<DashboardAlert>? Alerts = null);

public sealed record DashboardServiceSnapshot(
    string Name,
    string DisplayName,
    string Status,
    bool IsProtected,
    bool CanManage);

public sealed record DashboardAlert(
    string Id,
    string RuleId,
    string MetricKey,
    string Title,
    string Detail,
    string Severity,
    DateTimeOffset RaisedUtc,
    DateTimeOffset? ClearedUtc,
    DateTimeOffset? AcknowledgedUtc,
    DateTimeOffset? SnoozedUntilUtc);
