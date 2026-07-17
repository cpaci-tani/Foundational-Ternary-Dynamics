namespace Sift.Models;

public sealed class ApplyResult
{
    public string BackupPath { get; init; } = "";
    public List<string> Log { get; init; } = [];
    public int Succeeded { get; init; }
    public int Failed { get; init; }
    public int Previewed { get; init; }
    public bool IsPartialFailure => Failed > 0 && Succeeded > 0;
    public bool HasFailures => Failed > 0;
}

public sealed class RestoreResult
{
    public string BackupPath { get; init; } = "";
    public List<string> Log { get; init; } = [];
    public int Restored { get; init; }
    public int Skipped { get; init; }
    public int Failed { get; init; }
}

public sealed class BackupInfo
{
    public required string Path { get; init; }
    public required DateTime CreatedUtc { get; init; }
    public int EntryCount { get; init; }
    public int SuccessCount { get; init; }
    public string DisplayName => $"{CreatedUtc.ToLocalTime():yyyy-MM-dd HH:mm:ss} · {SuccessCount}/{EntryCount} applied";
}
