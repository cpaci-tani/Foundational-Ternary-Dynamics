namespace Sift.Models;

public enum HealthStatus
{
    Ok,
    Info,
    Warning,
    Critical
}

public enum HealthActionKind
{
    None,
    OpenWindowsUpdate,
    OpenEventViewer,
    OpenStorageSettings,
    OpenStartupSettings,
    NavigateMaintenance,
    NavigateStorage,
    NavigateServices,
    NavigateProcesses,
    OpenBackupFolder
}

public sealed class HealthCheckRow
{
    public required string Id { get; init; }
    public required string Title { get; init; }
    public required HealthStatus Status { get; init; }
    public required string Detail { get; init; }
    public required string Recommendation { get; init; }
    public HealthActionKind ActionKind { get; init; } = HealthActionKind.None;
    public string ActionLabel => ActionKind switch
    {
        HealthActionKind.OpenWindowsUpdate => "Open Windows Update",
        HealthActionKind.OpenEventViewer => "Open Event Viewer",
        HealthActionKind.OpenStorageSettings => "Open Storage settings",
        HealthActionKind.OpenStartupSettings => "Open Startup settings",
        HealthActionKind.NavigateMaintenance => "Open Maintenance",
        HealthActionKind.NavigateStorage => "Open Storage",
        HealthActionKind.NavigateServices => "Open Task Manager → Services",
        HealthActionKind.NavigateProcesses => "Open Task Manager",
        HealthActionKind.OpenBackupFolder => "Open backup folder",
        _ => ""
    };
    public string StatusLabel => Status switch
    {
        HealthStatus.Ok => "OK",
        HealthStatus.Info => "Info",
        HealthStatus.Warning => "Warning",
        HealthStatus.Critical => "Critical",
        _ => Status.ToString()
    };
}

public sealed class ActivityEntry
{
    public DateTime CreatedUtc { get; init; } = DateTime.UtcNow;
    public required string Category { get; init; }
    public required string Summary { get; init; }
    public string? Detail { get; init; }
    public string? RelatedPath { get; init; }
}

public sealed class HistoryRow
{
    public required DateTime TimestampUtc { get; init; }
    public required string Category { get; init; }
    public required string Title { get; init; }
    public required string Detail { get; init; }
    public string? Path { get; init; }
    public bool CanRestoreOptimize { get; init; }
    public string DisplayTime => TimestampUtc.ToLocalTime().ToString("yyyy-MM-dd HH:mm:ss");
}

public sealed record HistorySnapshot(
    IReadOnlyList<HistoryRow> Rows,
    IReadOnlyList<string> Warnings)
{
    public bool IsPartial => Warnings.Count > 0;
}
