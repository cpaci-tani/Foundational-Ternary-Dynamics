using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Sift.Models;

public sealed class AppLeftoverCandidate : INotifyPropertyChanged
{
    private bool _isSelected;
    public required string AppIdentity { get; init; }
    public required string AppDisplayName { get; init; }
    public required string Path { get; init; }
    public required string Scope { get; init; }
    public required string Evidence { get; init; }
    public long SizeBytes { get; init; }
    public long FileCount { get; init; }
    public bool CanDelete { get; init; }
    public string BlockReason { get; init; } = string.Empty;

    public bool IsSelected
    {
        get => _isSelected;
        set
        {
            if (!CanDelete || _isSelected == value) return;
            _isSelected = value;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsSelected)));
        }
    }

    public string SizeDisplay => SizeBytes switch
    {
        < 1024 => $"{SizeBytes} B",
        < 1_048_576 => $"{SizeBytes / 1024d:0.0} KB",
        < 1_073_741_824 => $"{SizeBytes / 1_048_576d:0.0} MB",
        _ => $"{SizeBytes / 1_073_741_824d:0.00} GB"
    };

    public string FileDisplay => $"{FileCount:N0} file{(FileCount == 1 ? string.Empty : "s")}";
    public string PolicyDisplay => CanDelete ? "Recycle Bin eligible" : BlockReason;
    public event PropertyChangedEventHandler? PropertyChanged;
}

public sealed record AppLeftoverScanResult(
    bool Blocked,
    string Message,
    IReadOnlyList<AppLeftoverCandidate> Candidates);

public sealed record AppLeftoverDeleteResult(
    bool Preview,
    int Previewed,
    int Deleted,
    int Skipped,
    int Failed,
    IReadOnlyList<string> Log)
{
    public string Summary => Preview
        ? $"Preflight reviewed {Previewed:N0} leftover folder(s); no files were changed."
        : $"Moved {Deleted:N0} folder(s) to the Recycle Bin; skipped {Skipped:N0}; failed {Failed:N0}.";
}
