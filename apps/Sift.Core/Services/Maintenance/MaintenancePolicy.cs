using Sift.Models;

namespace Sift.Services;

internal static class MaintenancePolicy
{
    private static readonly string LocalAppData = Full(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));
    public static bool IsAllowed(MaintenanceFinding item, out string reason)
    {
        reason = string.Empty;
        try
        {
            var allowed = item.Category switch
            {
                MaintenanceCategory.RecycleBin => item.Id == "maintenance.recycle" && item.Path == "Recycle Bin",
                MaintenanceCategory.TempFiles => IsExact(item.Path, Environment.GetEnvironmentVariable("TEMP") ?? Path.GetTempPath()) ||
                                                 IsExact(item.Path, Path.Combine(LocalAppData, "Temp")),
                MaintenanceCategory.ThumbnailCache => IsExact(item.Path, Path.Combine(LocalAppData, "Microsoft", "Windows", "Explorer", "thumbcache_*.db")),
                MaintenanceCategory.WerQueue => IsExact(item.Path, Path.Combine(LocalAppData, "Microsoft", "Windows", "WER", "ReportQueue")),
                MaintenanceCategory.CrashDumps => IsExact(item.Path, Path.Combine(LocalAppData, "CrashDumps", "*.dmp")),
                MaintenanceCategory.UpdateCache => IsExact(item.Path, Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows),
                    "ServiceProfiles", "NetworkService", "AppData", "Local", "Microsoft", "Windows", "DeliveryOptimization", "Cache")),
                MaintenanceCategory.Prefetch => IsExact(item.Path, Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "Prefetch", "*.pf")),
                // File leftovers require the Installed Apps workflow's exact app identity,
                // same-session authorization, explicit selection, and Recycle Bin execution.
                // Legacy MaintenanceFinding objects intentionally cannot authorize them.
                MaintenanceCategory.AppLeftover => false,
                MaintenanceCategory.OrphanUninstall => IsAllowedUninstallKey(item),
                _ => false
            };
            if (!allowed) reason = "this item is not supported by Maintenance";
            return allowed;
        }
        catch (Exception exception)
        {
            reason = exception.Message;
            return false;
        }
    }

    public static bool IsAllowedUninstallKey(string? hive, string? subKey)
    {
        if (hive is not ("HKCU" or "HKLM") || string.IsNullOrWhiteSpace(subKey)) return false;
        var normalized = subKey.Replace('/', '\\').Trim('\\');
        var roots = hive == "HKCU"
            ? new[] { @"Software\Microsoft\Windows\CurrentVersion\Uninstall" }
            : new[] { @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall", @"SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall" };
        return roots.Any(root => normalized.StartsWith(root + "\\", StringComparison.OrdinalIgnoreCase));
    }

    private static bool IsAllowedUninstallKey(MaintenanceFinding item) =>
        IsAllowedUninstallKey(item.RegistryHive, item.RegistrySubKey) &&
        item.Path.Equals($"{item.RegistryHive}\\{item.RegistrySubKey}", StringComparison.OrdinalIgnoreCase);

    private static bool IsExact(string left, string right) => Full(left).Equals(Full(right), StringComparison.OrdinalIgnoreCase);
    private static string Full(string path) => Path.GetFullPath(path).TrimEnd('\\');
}
