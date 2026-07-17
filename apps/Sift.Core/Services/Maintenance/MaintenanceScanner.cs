using System.IO;
using Sift.Models;
using Microsoft.Win32;

namespace Sift.Services;

public sealed class MaintenanceScanner : IMaintenanceScanner
{
    private readonly Func<bool> _isElevated;

    public MaintenanceScanner(Func<bool>? isElevated = null) =>
        _isElevated = isElevated ?? ElevationHelper.IsElevated;

    public bool DeliveryOptimizationSkippedForElevation { get; private set; }
    public bool PrefetchSkippedForElevation { get; private set; }

    public IReadOnlyList<MaintenanceFinding> Scan(IProgress<string>? progress = null)
    {
        DeliveryOptimizationSkippedForElevation = false;
        PrefetchSkippedForElevation = false;
        var findings = new List<MaintenanceFinding>();

        progress?.Report("Scanning temp folders…");
        findings.AddRange(ScanTempFolder("User temp", Environment.GetEnvironmentVariable("TEMP") ?? Path.GetTempPath(), "maintenance.temp.user"));
        findings.AddRange(ScanTempFolder("LocalAppData temp", Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Temp"), "maintenance.temp.local"));

        progress?.Report("Scanning Recycle Bin…");
        findings.Add(ScanRecycleBin());

        progress?.Report("Scanning thumbnail cache…");
        findings.AddRange(ScanThumbnailCache());

        progress?.Report("Scanning WER queue and crash dumps…");
        findings.AddRange(ScanWerQueue());
        findings.AddRange(ScanCrashDumps());

        var elevated = _isElevated();
        progress?.Report("Scanning orphan uninstall entries…");
        findings.AddRange(ScanOrphanUninstallEntries(includeMachineWide: elevated));

        if (elevated)
        {
            progress?.Report("Scanning Delivery Optimization cache…");
            findings.AddRange(ScanDeliveryOptimizationCache());
            progress?.Report("Scanning Prefetch…");
            findings.AddRange(ScanPrefetch());
        }
        else
        {
            DeliveryOptimizationSkippedForElevation = true;
            PrefetchSkippedForElevation = true;
        }

        return findings
            .Where(f => f.SizeBytes > 0 || f.Category is MaintenanceCategory.RecycleBin or MaintenanceCategory.OrphanUninstall)
            .GroupBy(FindingIdentity, StringComparer.OrdinalIgnoreCase)
            .Select(group => group.First())
            .OrderByDescending(f => f.SizeBytes)
            .ToList();
    }

    private static string FindingIdentity(MaintenanceFinding finding)
    {
        if (!string.IsNullOrWhiteSpace(finding.RegistrySubKey))
            return $"registry:{finding.RegistryHive}:{finding.RegistrySubKey}";
        return $"path:{finding.Path.Trim().TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar)}";
    }

    private static IEnumerable<MaintenanceFinding> ScanTempFolder(string title, string path, string id)
    {
        if (!Directory.Exists(path)) yield break;
        var (bytes, files, capped) = MeasureDirectory(path, maxDepth: 3, maxFiles: 8000);
        if (bytes <= 0) yield break;
        yield return new MaintenanceFinding
        {
            Id = id,
            Category = MaintenanceCategory.TempFiles,
            Title = title,
            Path = path,
            Detail = $"{files:N0} items{(capped ? " (size ≥ estimate; scan capped)" : "")}.",
            SizeBytes = bytes,
            SizeCapped = capped,
            CanClean = true,
            Confidence = MaintenanceConfidence.High
        };
    }

    private static MaintenanceFinding ScanRecycleBin()
    {
        long size = 0;
        var capped = false;
        try
        {
            foreach (var drive in DriveInfo.GetDrives().Where(d => d.IsReady && d.DriveType == DriveType.Fixed))
            {
                var recycle = Path.Combine(drive.Name, "$Recycle.Bin");
                if (!Directory.Exists(recycle)) continue;
                var measure = MeasureDirectory(recycle, maxDepth: 3, maxFiles: 4000);
                size += measure.bytes;
                capped |= measure.capped;
            }
        }
        catch { /* access denied on some drives */ }

        return new MaintenanceFinding
        {
            Id = "maintenance.recycle",
            Category = MaintenanceCategory.RecycleBin,
            Title = "Recycle Bin",
            Path = "Recycle Bin",
            Detail = size > 0 ? "Permanently deletes items already in the Recycle Bin." : "Recycle Bin appears empty.",
            SizeBytes = size,
            SizeCapped = capped,
            CanClean = size > 0,
            Confidence = MaintenanceConfidence.High
        };
    }

    private static IEnumerable<MaintenanceFinding> ScanThumbnailCache()
    {
        var dir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Microsoft", "Windows", "Explorer");
        if (!Directory.Exists(dir)) yield break;
        long bytes = 0;
        var count = 0;
        foreach (var file in Directory.EnumerateFiles(dir, "thumbcache_*.db"))
        {
            try { bytes += new FileInfo(file).Length; count++; } catch { /* skip */ }
        }
        if (bytes <= 0) yield break;
        yield return new MaintenanceFinding
        {
            Id = "maintenance.thumbnails",
            Category = MaintenanceCategory.ThumbnailCache,
            Title = "Explorer thumbnail cache",
            Path = Path.Combine(dir, "thumbcache_*.db"),
            Detail = $"{count} thumbcache_*.db file(s). Windows rebuilds thumbnails as needed.",
            SizeBytes = bytes,
            CanClean = true,
            Confidence = MaintenanceConfidence.High
        };
    }

    private static IEnumerable<MaintenanceFinding> ScanWerQueue()
    {
        var path = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Microsoft", "Windows", "WER", "ReportQueue");
        if (!Directory.Exists(path)) yield break;
        var (bytes, files, capped) = MeasureDirectory(path, maxDepth: 4, maxFiles: 3000);
        if (bytes <= 0) yield break;
        yield return new MaintenanceFinding
        {
            Id = "maintenance.wer-queue",
            Category = MaintenanceCategory.WerQueue,
            Title = "Windows Error Reporting queue",
            Path = path,
            Detail = $"{files:N0} queued report item(s){(capped ? " (≥ estimate)" : "")}.",
            SizeBytes = bytes,
            SizeCapped = capped,
            CanClean = true,
            Confidence = MaintenanceConfidence.High
        };
    }

    private static IEnumerable<MaintenanceFinding> ScanCrashDumps()
    {
        var path = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "CrashDumps");
        if (!Directory.Exists(path)) yield break;
        long bytes = 0;
        var count = 0;
        foreach (var file in Directory.EnumerateFiles(path, "*.dmp"))
        {
            try { bytes += new FileInfo(file).Length; count++; } catch { /* skip */ }
        }
        if (bytes <= 0) yield break;
        yield return new MaintenanceFinding
        {
            Id = "maintenance.crash-dumps",
            Category = MaintenanceCategory.CrashDumps,
            Title = "User crash dumps",
            Path = Path.Combine(path, "*.dmp"),
            Detail = $"{count} .dmp file(s) in the user CrashDumps folder.",
            SizeBytes = bytes,
            CanClean = true,
            Confidence = MaintenanceConfidence.High
        };
    }

    private static IEnumerable<MaintenanceFinding> ScanDeliveryOptimizationCache()
    {
        var path = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "ServiceProfiles", "NetworkService", "AppData", "Local", "Microsoft", "Windows", "DeliveryOptimization", "Cache");
        if (!Directory.Exists(path)) yield break;
        var (bytes, files, capped) = MeasureDirectory(path, maxDepth: 3, maxFiles: 4000);
        if (bytes <= 0) yield break;
        yield return new MaintenanceFinding
        {
            Id = "maintenance.delivery-cache",
            Category = MaintenanceCategory.UpdateCache,
            Title = "Delivery Optimization cache",
            Path = path,
            Detail = $"{files:N0} cached peer-update files{(capped ? " (≥ estimate)" : "")}.",
            SizeBytes = bytes,
            SizeCapped = capped,
            CanClean = true,
            RequiresElevation = true,
            Confidence = MaintenanceConfidence.High
        };
    }

    private static IEnumerable<MaintenanceFinding> ScanPrefetch()
    {
        var path = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "Prefetch");
        if (!Directory.Exists(path)) yield break;
        long bytes = 0;
        var count = 0;
        try
        {
            foreach (var file in Directory.EnumerateFiles(path, "*.pf"))
            {
                try { bytes += new FileInfo(file).Length; count++; } catch { /* skip */ }
            }
        }
        catch { yield break; }
        if (bytes <= 0) yield break;
        yield return new MaintenanceFinding
        {
            Id = "maintenance.prefetch",
            Category = MaintenanceCategory.Prefetch,
            Title = "Prefetch files",
            Path = Path.Combine(path, "*.pf"),
            Detail = $"{count} .pf file(s). Cleaning may briefly slow app launches; Advanced confirm required.",
            SizeBytes = bytes,
            CanClean = true,
            RequiresElevation = true,
            RequiresAdvancedConfirm = true,
            Confidence = MaintenanceConfidence.Medium
        };
    }

    public static IEnumerable<MaintenanceFinding> ScanOrphanUninstallEntries(bool includeMachineWide)
    {
        foreach (var orphan in EnumerateOrphanUninstalls(includeMachineWide))
        {
            yield return new MaintenanceFinding
            {
                Id = $"orphan-uninstall.{SanitizeId(orphan.DisplayName)}.{SanitizeId(orphan.SubKeyName)}",
                Category = MaintenanceCategory.OrphanUninstall,
                Title = $"Orphan uninstall: {orphan.DisplayName}",
                Path = $"{orphan.Hive}\\{orphan.SubKey}",
                Detail = orphan.Evidence,
                SizeBytes = 0,
                CanClean = true,
                RequiresElevation = orphan.Hive == "HKLM",
                RequiresAdvancedConfirm = true,
                Confidence = MaintenanceConfidence.Medium,
                RegistryHive = orphan.Hive,
                RegistrySubKey = orphan.SubKey,
                RegistryValues = orphan.Values
            };
        }
    }

    public static IReadOnlyList<OrphanUninstall> EnumerateOrphanUninstalls(bool includeMachineWide)
    {
        var results = new List<OrphanUninstall>();
        CollectOrphans(Registry.CurrentUser, "HKCU", @"Software\Microsoft\Windows\CurrentVersion\Uninstall", results);
        if (includeMachineWide)
        {
            CollectOrphans(Registry.LocalMachine, "HKLM", @"Software\Microsoft\Windows\CurrentVersion\Uninstall", results);
            CollectOrphans(Registry.LocalMachine, "HKLM", @"Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall", results);
        }
        return results;
    }

    private static void CollectOrphans(RegistryKey root, string hiveName, string path, List<OrphanUninstall> results)
    {
        try
        {
            using var key = root.OpenSubKey(path);
            if (key is null) return;
            foreach (var sub in key.GetSubKeyNames())
            {
                try
                {
                    using var app = key.OpenSubKey(sub);
                    if (app is null) continue;
                    var displayName = app.GetValue("DisplayName")?.ToString()?.Trim();
                    if (string.IsNullOrWhiteSpace(displayName)) continue;
                    var publisher = app.GetValue("Publisher")?.ToString()?.Trim() ?? "";
                    var registryValues = new InstalledAppRegistryValues(
                        displayName,
                        publisher,
                        app.GetValue("DisplayVersion")?.ToString()?.Trim() ?? string.Empty,
                        app.GetValue("InstallLocation")?.ToString()?.Trim() ?? string.Empty,
                        app.GetValue("InstallDate")?.ToString()?.Trim() ?? string.Empty,
                        0,
                        app.GetValue("UninstallString")?.ToString()?.Trim() ?? string.Empty,
                        ReadFlag(app, "WindowsInstaller"),
                        ReadFlag(app, "SystemComponent"),
                        app.GetValue("ReleaseType")?.ToString()?.Trim() ?? string.Empty,
                        app.GetValue("ParentKeyName")?.ToString()?.Trim() ?? string.Empty);
                    if (!InstalledAppPolicy.IsConservativeOrphan(registryValues, out var evidence)) continue;

                    var values = new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase);
                    foreach (var name in app.GetValueNames())
                        values[name] = app.GetValue(name)?.ToString();

                    results.Add(new OrphanUninstall(displayName, publisher, hiveName, Path.Combine(path, sub), sub, values, evidence));
                }
                catch { /* skip */ }
            }
        }
        catch { /* access denied */ }
    }

    private static bool ReadFlag(RegistryKey key, string name) => key.GetValue(name) switch
    {
        int value => value == 1,
        long value => value == 1,
        string value => value == "1",
        _ => false
    };

    private static (long bytes, int files, bool capped) MeasureDirectory(string path, int maxDepth, int maxFiles)
    {
        long bytes = 0;
        var files = 0;
        var capped = false;
        try
        {
            foreach (var file in EnumerateFilesLimited(path, maxDepth, maxFiles, out capped))
            {
                try { bytes += new FileInfo(file).Length; files++; }
                catch { /* skip locked files */ }
            }
        }
        catch { /* skip inaccessible trees */ }
        return (bytes, files, capped);
    }

    private static IEnumerable<string> EnumerateFilesLimited(string root, int maxDepth, int maxFiles, out bool capped)
    {
        capped = false;
        var list = new List<string>();
        var queue = new Queue<(string path, int depth)>();
        queue.Enqueue((root, 0));
        while (queue.Count > 0 && list.Count < maxFiles)
        {
            var (current, depth) = queue.Dequeue();
            try
            {
                foreach (var file in Directory.EnumerateFiles(current))
                {
                    list.Add(file);
                    if (list.Count >= maxFiles) { capped = true; return list; }
                }
            }
            catch { continue; }
            if (depth >= maxDepth) continue;
            try
            {
                foreach (var dir in Directory.EnumerateDirectories(current))
                    queue.Enqueue((dir, depth + 1));
            }
            catch { /* skip */ }
        }
        if (queue.Count > 0) capped = true;
        return list;
    }

    private static string SanitizeId(string value)
    {
        var chars = value.Where(char.IsLetterOrDigit).Take(24).ToArray();
        return chars.Length == 0 ? "folder" : new string(chars).ToLowerInvariant();
    }

}

public sealed record OrphanUninstall(
    string DisplayName,
    string Publisher,
    string Hive,
    string SubKey,
    string SubKeyName,
    Dictionary<string, string?> Values,
    string Evidence);
