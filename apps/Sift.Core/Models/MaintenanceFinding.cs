using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Sift.Models;

public enum MaintenanceCategory
{
    TempFiles,
    AppLeftover,
    RecycleBin,
    UpdateCache,
    ThumbnailCache,
    WerQueue,
    CrashDumps,
    Prefetch,
    OrphanUninstall,
    Other
}

public enum MaintenanceConfidence
{
    High,
    Medium
}

public sealed class MaintenanceFinding : INotifyPropertyChanged
{
    private bool _isSelected;
    public required string Id { get; init; }
    public required MaintenanceCategory Category { get; init; }
    public required string Title { get; init; }
    public required string Path { get; init; }
    public required string Detail { get; init; }
    public long SizeBytes { get; init; }
    public bool CanClean { get; init; } = true;
    public bool RequiresElevation { get; init; }
    public bool RequiresAdvancedConfirm { get; init; }
    public MaintenanceConfidence Confidence { get; init; } = MaintenanceConfidence.High;
    public bool SizeCapped { get; init; }
    public string? RegistryHive { get; init; }
    public string? RegistrySubKey { get; init; }
    public Dictionary<string, string?>? RegistryValues { get; init; }

    public bool IsSelected
    {
        get => _isSelected;
        set { if (CanClean) { _isSelected = value; Changed(); } }
    }

    public string CategoryLabel => Category switch
    {
        MaintenanceCategory.TempFiles => "Temp files",
        MaintenanceCategory.AppLeftover => "App leftover",
        MaintenanceCategory.RecycleBin => "Recycle Bin",
        MaintenanceCategory.UpdateCache => "Update cache",
        MaintenanceCategory.ThumbnailCache => "Thumbnails",
        MaintenanceCategory.WerQueue => "WER queue",
        MaintenanceCategory.CrashDumps => "Crash dumps",
        MaintenanceCategory.Prefetch => "Prefetch",
        MaintenanceCategory.OrphanUninstall => "Orphan uninstall",
        _ => "Other"
    };

    public string ConfidenceLabel => Confidence.ToString().ToUpperInvariant();

    public string SizeLabel
    {
        get
        {
            var prefix = SizeCapped ? "≥ " : "";
            return SizeBytes switch
            {
                < 1024 => $"{prefix}{SizeBytes} B",
                < 1048576 => $"{prefix}{SizeBytes / 1024.0:0.0} KB",
                < 1073741824 => $"{prefix}{SizeBytes / 1048576.0:0.0} MB",
                _ => $"{prefix}{SizeBytes / 1073741824.0:0.00} GB"
            };
        }
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Changed([CallerMemberName] string? name = null) => PropertyChanged?.Invoke(this, new(name));
}

public sealed class CleanResult
{
    public List<string> Log { get; init; } = [];
    public int Cleaned { get; init; }
    public int Skipped { get; init; }
    public int Failed { get; init; }
    public int Previewed { get; init; }
    public MaintenanceCleanupStatus Status { get; init; }
    public string Summary => Status switch
    {
        MaintenanceCleanupStatus.Reviewed => $"Checked {Previewed} item(s).",
        MaintenanceCleanupStatus.Invalidated => Log.FirstOrDefault() ?? "The reviewed selection changed.",
        _ => $"Cleaned {Cleaned}, skipped {Skipped}, failed {Failed}."
    };
}

public enum MaintenanceCleanupStatus
{
    Reviewed,
    Completed,
    Invalidated,
    Rejected
}

public sealed record MaintenanceCleanupReview(
    string? TicketId,
    CleanResult Result)
{
    public bool CanExecute =>
        !string.IsNullOrWhiteSpace(TicketId) &&
        Result.Status == MaintenanceCleanupStatus.Reviewed;
}
