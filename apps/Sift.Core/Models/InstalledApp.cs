namespace Sift.Models;

public sealed record InstalledAppRegistryLocation(
    string Hive,
    string View,
    string SubKeyName)
{
    public string Identity => $"{Hive}|{View}|{SubKeyName}";
}

public sealed record InstalledApp(
    InstalledAppRegistryLocation RegistryLocation,
    string DisplayName,
    string Publisher,
    string DisplayVersion,
    string InstallLocation,
    string InstallDate,
    long EstimatedSizeBytes,
    string UninstallString,
    string Source,
    bool CanUninstall,
    string PolicyReason)
{
    public bool IsOrphanedRegistration { get; init; }
    public bool CanCleanRegistration { get; init; }
    public string OrphanEvidence { get; init; } = string.Empty;

    /// <summary>PNG thumbnail extracted from the registered <c>DisplayIcon</c>, or null when unavailable.</summary>
    public byte[]? IconPng { get; init; }
    public string SizeDisplay => EstimatedSizeBytes > 0 ? FormatBytes(EstimatedSizeBytes) : "—";
    public string InstallDateDisplay => string.IsNullOrWhiteSpace(InstallDate) ? "—" : InstallDate;
    public string PublisherDisplay => string.IsNullOrWhiteSpace(Publisher) ? "Unknown publisher" : Publisher;
    public string VersionDisplay => string.IsNullOrWhiteSpace(DisplayVersion) ? "—" : DisplayVersion;
    public string UninstallabilityDisplay => CanUninstall ? "Yes" : "No";
    public string PolicyDisplay => IsOrphanedRegistration
        ? CanCleanRegistration ? "Leftover registration" : "Leftover · admin required"
        : CanUninstall ? "Eligible" : "Windows Settings only";

    private static string FormatBytes(long bytes)
    {
        string[] units = ["B", "KB", "MB", "GB", "TB"];
        var value = (double)bytes;
        var unit = 0;
        while (value >= 1024 && unit < units.Length - 1)
        {
            value /= 1024;
            unit++;
        }

        return $"{value:0.##} {units[unit]}";
    }
}

public sealed record InstalledAppActionResult(
    bool Previewed,
    bool Executed,
    bool Blocked,
    string Message)
{
    public string? ContinuationToken { get; init; }
    public string? UninstallSessionId { get; init; }
    public int? ProcessId { get; init; }
}

public sealed record InstalledAppUninstallCompletion(
    bool Completed,
    bool Blocked,
    string Message)
{
    public string? ContinuationToken { get; init; }
}
