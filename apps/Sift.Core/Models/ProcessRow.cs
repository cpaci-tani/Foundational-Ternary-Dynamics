using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Sift.Models;

public sealed class ProcessRow : INotifyPropertyChanged
{
    private double _cpuPercent;
    private double _memoryMb;
    private double _privateMemoryMb;
    private double _readRateMb;
    private double _writeRateMb;
    private int _threadCount;
    private int _handleCount;
    private string _status = "Running";
    private string _priority = "—";
    private string _windowTitle = "";
    private string _architecture = "—";
    private string _executablePath = "Unavailable";
    private double _uptimeSeconds;
    private int _sessionId;
    private long _startTimeUtcTicks;
    private byte[]? _iconPng;

    public required int Id { get; init; }
    public required string Name { get; init; }
    public double CpuPercent { get => _cpuPercent; set => Set(ref _cpuPercent, value); }
    public double MemoryMb { get => _memoryMb; set => Set(ref _memoryMb, value); }
    public double PrivateMemoryMb { get => _privateMemoryMb; set => Set(ref _privateMemoryMb, value); }
    public double ReadRateMb { get => _readRateMb; set => Set(ref _readRateMb, value); }
    public double WriteRateMb { get => _writeRateMb; set => Set(ref _writeRateMb, value); }
    public int ThreadCount { get => _threadCount; set => Set(ref _threadCount, value); }
    public int HandleCount { get => _handleCount; set => Set(ref _handleCount, value); }
    public string Status { get => _status; set => Set(ref _status, value); }
    public string Priority { get => _priority; set => Set(ref _priority, value); }
    public string WindowTitle { get => _windowTitle; set => Set(ref _windowTitle, value); }
    public string Architecture { get => _architecture; set => Set(ref _architecture, value); }
    public string ExecutablePath { get => _executablePath; set => Set(ref _executablePath, value); }
    public double UptimeSeconds { get => _uptimeSeconds; set { if (Set(ref _uptimeSeconds, value)) Changed(nameof(UptimeLabel)); } }
    public int SessionId { get => _sessionId; set => Set(ref _sessionId, value); }
    public long StartTimeUtcTicks { get => _startTimeUtcTicks; set => Set(ref _startTimeUtcTicks, value); }
    public byte[]? IconPng { get => _iconPng; set => Set(ref _iconPng, value); }
    public string GroupKey => string.IsNullOrWhiteSpace(Name) ? "Other" : Name;
    public string DisplayName => string.IsNullOrWhiteSpace(WindowTitle) ? Name : WindowTitle;
    public string UptimeLabel => TimeSpan.FromSeconds(Math.Max(0, UptimeSeconds)) switch
    { var t when t.TotalDays >= 1 => $"{(int)t.TotalDays}d {t.Hours}h", var t when t.TotalHours >= 1 => $"{(int)t.TotalHours}h {t.Minutes}m", var t => $"{t.Minutes}m {t.Seconds}s" };

    public void Update(ProcessSnapshot value)
    {
        CpuPercent = value.CpuPercent;
        MemoryMb = value.MemoryMb;
        PrivateMemoryMb = value.PrivateMemoryMb;
        ReadRateMb = value.ReadRateMb;
        WriteRateMb = value.WriteRateMb;
        ThreadCount = value.ThreadCount;
        HandleCount = value.HandleCount;
        Status = value.Status;
        Priority = value.Priority;
        WindowTitle = value.WindowTitle;
        Architecture = value.Architecture;
        ExecutablePath = value.ExecutablePath;
        UptimeSeconds = value.UptimeSeconds;
        SessionId = value.SessionId;
        StartTimeUtcTicks = value.StartTimeUtcTicks;
        IconPng = value.IconPng;
        Changed(nameof(DisplayName));
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Changed([CallerMemberName] string? name = null) => PropertyChanged?.Invoke(this, new(name));
    private bool Set<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value)) return false;
        field = value; Changed(name); return true;
    }
}

public sealed record ProcessSnapshot(int Id, string Name, double CpuPercent, double MemoryMb, double PrivateMemoryMb,
    double ReadRateMb, double WriteRateMb, int ThreadCount, int HandleCount, string Status, string Priority,
    string WindowTitle, double UptimeSeconds, int SessionId, long StartTimeUtcTicks, string Architecture,
    string ExecutablePath, byte[]? IconPng);

/// <summary>System-level counters from PDH (optional enrichment on top of process sampling).</summary>
public sealed record SystemCountersSnapshot(double CpuPercent, double DiskReadMbPerSec, double DiskWriteMbPerSec);

public sealed record SystemSnapshot(
    IReadOnlyList<ProcessSnapshot> Processes,
    double CpuPercent,
    double MemoryPercent,
    double UsedMemoryGb,
    double TotalMemoryGb,
    SystemCountersSnapshot? Counters = null);
