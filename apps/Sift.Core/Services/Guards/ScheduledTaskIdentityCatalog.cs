using Sift.Models;

namespace Sift.Services;

public sealed record ScheduledTaskDefinition(
    ScheduledTaskId Id,
    string TaskPath,
    string TaskName,
    string DisplayName);

public static class ScheduledTaskIdentityCatalog
{
    private static readonly ScheduledTaskDefinition[] Definitions =
    [
        new(ScheduledTaskId.OfficeAutomaticUpdates, @"\Microsoft\Office", "Office Automatic Updates",
            "Office Automatic Updates"),
        new(ScheduledTaskId.OfficeFeatureUpdates, @"\Microsoft\Office", "Office Feature Updates",
            "Office Feature Updates")
    ];

    public static ScheduledTaskDefinition Resolve(ScheduledTaskId id) =>
        Definitions.First(definition => definition.Id == id);

    public static bool TryResolve(string taskPath, string taskName, out ScheduledTaskDefinition definition)
    {
        definition = default!;
        if (ContainsBlockedIdentity(taskPath, taskName)) return false;

        var normalizedPath = NormalizePath(taskPath);
        var normalizedName = taskName.Trim();
        foreach (var candidate in Definitions)
        {
            if (string.Equals(NormalizedPath(candidate.TaskPath), normalizedPath, StringComparison.OrdinalIgnoreCase) &&
                string.Equals(candidate.TaskName, normalizedName, StringComparison.OrdinalIgnoreCase))
            {
                definition = candidate;
                return true;
            }
        }

        return false;
    }

    public static bool IsResolvableIdentity(string taskPath, string taskName) =>
        TryResolve(taskPath, taskName, out _);

    private static bool ContainsBlockedIdentity(string taskPath, string taskName)
    {
        var full = (NormalizePath(taskPath).TrimEnd('\\') + "\\" + taskName.Trim()).Replace("\\\\", "\\",
            StringComparison.Ordinal);
        if (full.Contains("Windows Defender", StringComparison.OrdinalIgnoreCase)) return true;
        if (full.Contains("WindowsUpdate", StringComparison.OrdinalIgnoreCase)) return true;
        if (full.StartsWith(@"\Microsoft\Windows\", StringComparison.OrdinalIgnoreCase)) return true;
        return false;
    }

    private static string NormalizePath(string taskPath)
    {
        var normalized = "\\" + taskPath.Trim().Trim('\\');
        return normalized == "\\" ? "\\" : normalized;
    }

    private static string NormalizedPath(string taskPath) => NormalizePath(taskPath);
}
