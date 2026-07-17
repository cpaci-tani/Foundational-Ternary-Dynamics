namespace Sift.Models;

public sealed class Backup
{
    public int SchemaVersion { get; init; } = 2;
    public string OperationId { get; init; } = Guid.NewGuid().ToString("N");
    public DateTime CreatedUtc { get; init; } = DateTime.UtcNow;
    public string MachineName { get; init; } = Environment.MachineName;
    public string WindowsVersion { get; init; } = Environment.OSVersion.VersionString;
    public List<BackupEntry> Entries { get; init; } = [];
}

public sealed class BackupEntry
{
    public required string TweakId { get; init; }
    public string State { get; set; } = BackupEntryStates.Prepared;
    public DateTime? AppliedUtc { get; set; }
    public DateTime? RestoredUtc { get; set; }
    public string? FailureDetail { get; set; }
    public bool KeyExisted { get; init; }
    public bool Existed { get; init; }
    public RegistryValueSnapshot? RegistryValue { get; init; }
    public RegistryKeySnapshot? RegistryTree { get; init; }
    public string? RegistryHive { get; init; }
    public string? RegistrySubKey { get; init; }

    // Legacy v1 fields remain readable so existing user backups are not orphaned.
    public string? Value { get; init; }
    public string? RegistryKind { get; init; }
    public bool AppliedSuccessfully { get; set; }
}

public static class BackupEntryStates
{
    public const string Prepared = "Prepared";
    public const string Applying = "Applying";
    public const string Applied = "Applied";
    public const string Restored = "Restored";
}

public sealed class RegistryValueSnapshot
{
    public required string Name { get; init; }
    public required string Kind { get; init; }
    public required string Encoding { get; init; }
    public required string Data { get; init; }
}

public sealed class RegistryKeySnapshot
{
    public List<RegistryValueSnapshot> Values { get; init; } = [];
    public Dictionary<string, RegistryKeySnapshot> SubKeys { get; init; } = new(StringComparer.OrdinalIgnoreCase);
}
