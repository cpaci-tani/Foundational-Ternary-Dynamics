using System.Diagnostics;
using System.Diagnostics.Eventing.Reader;
using System.IO;
using Sift.Models;
using Microsoft.Win32;

namespace Sift.Services;

public static class HealthScanner
{
    public static IReadOnlyList<HealthCheckRow> Scan()
    {
        var rows = new List<HealthCheckRow>();
        rows.AddRange(ScanDiskSpace());
        rows.AddRange(ScanPendingReboot());
        rows.Add(ScanMemory());
        rows.AddRange(ScanStoppedAutoServices());
        rows.Add(ScanWindowsUpdate());
        rows.Add(ScanRecentErrors());
        rows.AddRange(ScanDiskHealth());
        return rows;
    }

    private static IEnumerable<HealthCheckRow> ScanDiskSpace()
    {
        foreach (var drive in DriveInfo.GetDrives().Where(d => d.DriveType == DriveType.Fixed))
        {
            if (!drive.IsReady)
            {
                yield return new HealthCheckRow
                {
                    Id = $"disk-{drive.Name}",
                    Title = $"Drive {drive.Name.TrimEnd('\\')} not ready",
                    Status = HealthStatus.Warning,
                    Detail = "Windows could not read free space for this volume.",
                    Recommendation = "Open Disk Management or Storage settings to inspect the drive.",
                    ActionKind = HealthActionKind.OpenStorageSettings
                };
                continue;
            }

            var freeGb = drive.AvailableFreeSpace / 1073741824.0;
            var totalGb = drive.TotalSize / 1073741824.0;
            var pctFree = totalGb > 0 ? freeGb / totalGb * 100 : 0;
            var status = pctFree < 5 || freeGb < 5
                ? HealthStatus.Critical
                : pctFree < 10 || freeGb < 20
                    ? HealthStatus.Warning
                    : HealthStatus.Ok;
            yield return new HealthCheckRow
            {
                Id = $"disk-{drive.Name}",
                Title = $"Drive {drive.Name.TrimEnd('\\')} free space",
                Status = status,
                Detail = $"{freeGb:0.0} GB free of {totalGb:0.0} GB ({pctFree:0}% free).",
                Recommendation = status == HealthStatus.Ok
                    ? "No action needed. Use Storage to find large folders if you want to reclaim space."
                    : "Low free space can cause updates and apps to fail. Review large folders in Storage or Maintenance before deleting anything.",
                ActionKind = status == HealthStatus.Ok ? HealthActionKind.None : HealthActionKind.NavigateStorage
            };
        }
    }

    private static IEnumerable<HealthCheckRow> ScanPendingReboot()
    {
        var reasons = new List<string>();
        if (RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)
                .OpenSubKey(@"SOFTWARE\Microsoft\Windows\CurrentVersion\WindowsUpdate\Auto Update")?.GetValue("RebootRequired") is 1)
            reasons.Add("Windows Update");
        if (RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)
                .OpenSubKey(@"SOFTWARE\Microsoft\Windows\CurrentVersion\Component Based Servicing")?.GetSubKeyNames()
                .Any(n => n.Equals("RebootPending", StringComparison.OrdinalIgnoreCase)) == true)
            reasons.Add("Component Based Servicing");
        try
        {
            using var session = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64)
                .OpenSubKey(@"SYSTEM\CurrentControlSet\Control\Session Manager");
            if (session?.GetValue("PendingFileRenameOperations") is string[] or byte[])
                reasons.Add("Pending file rename");
        }
        catch { /* ignore */ }

        if (reasons.Count == 0)
        {
            yield return new HealthCheckRow
            {
                Id = "reboot",
                Title = "Restart required",
                Status = HealthStatus.Ok,
                Detail = "No pending reboot flags were detected.",
                Recommendation = "Restart when Windows asks — not on a schedule from Sift."
            };
            yield break;
        }

        yield return new HealthCheckRow
        {
            Id = "reboot",
            Title = "Restart required",
            Status = HealthStatus.Warning,
            Detail = $"Pending restart signals: {string.Join(", ", reasons.Distinct())}.",
            Recommendation = "Save your work and restart when convenient. Sift will not force a reboot.",
            ActionKind = HealthActionKind.OpenWindowsUpdate
        };
    }

    private static HealthCheckRow ScanMemory()
    {
        try
        {
            var sample = SystemMemoryReader.Read();
            var status = sample.Percent >= 90 ? HealthStatus.Critical
                : sample.Percent >= 80 ? HealthStatus.Warning
                : HealthStatus.Ok;
            return new HealthCheckRow
            {
                Id = "memory",
                Title = "Physical memory in use",
                Status = status,
                Detail = $"{sample.Percent:0}% used ({sample.UsedGb:0.0} / {sample.TotalGb:0.0} GB).",
                Recommendation = status == HealthStatus.Ok
                    ? "Memory use looks normal for a running system."
                    : "High memory use is common with many apps open. Use Task Manager to see top consumers — Sift will not terminate processes automatically from here.",
                ActionKind = status == HealthStatus.Ok ? HealthActionKind.None : HealthActionKind.NavigateProcesses
            };
        }
        catch
        {
            return new HealthCheckRow
            {
                Id = "memory",
                Title = "Physical memory in use",
                Status = HealthStatus.Info,
                Detail = "Could not read memory counters.",
                Recommendation = "Open Task Manager for live memory use."
            };
        }
    }

    private static IEnumerable<HealthCheckRow> ScanStoppedAutoServices()
    {
        var stopped = WindowsServiceMonitor.Enumerate()
            .Where(s => s.CanManage &&
                        s.StartType.Contains("Automatic", StringComparison.OrdinalIgnoreCase) &&
                        s.Status.Equals("Stopped", StringComparison.OrdinalIgnoreCase))
            .Take(8)
            .ToList();

        if (stopped.Count == 0)
        {
            yield return new HealthCheckRow
            {
                Id = "services-auto",
                Title = "Automatic services (manageable)",
                Status = HealthStatus.Ok,
                Detail = "No stopped Automatic services outside the protected set.",
                Recommendation = "Protected services (Defender, Update, firewall, RPC) are intentionally excluded."
            };
            yield break;
        }

        var names = string.Join(", ", stopped.Select(s => s.DisplayName).Take(5));
        if (stopped.Count > 5) names += $" (+{stopped.Count - 5} more)";
        yield return new HealthCheckRow
        {
            Id = "services-auto",
            Title = "Stopped Automatic services (manageable)",
            Status = HealthStatus.Warning,
            Detail = names,
            Recommendation = "Some user/OEM services stop themselves when idle. Only start a service if you know what it does — Task Manager → Services shows the full list with Start/Stop controls and protected-service blocks.",
            ActionKind = HealthActionKind.NavigateServices
        };
    }

    private static HealthCheckRow ScanWindowsUpdate()
    {
        string serviceStatus;
        try
        {
            using var sc = new System.ServiceProcess.ServiceController("wuauserv");
            serviceStatus = sc.Status.ToString();
        }
        catch
        {
            serviceStatus = "Unknown";
        }

        var status = serviceStatus.Equals("Running", StringComparison.OrdinalIgnoreCase) ? HealthStatus.Ok : HealthStatus.Info;
        return new HealthCheckRow
        {
            Id = "wuauserv",
            Title = "Windows Update service (wuauserv)",
            Status = status,
            Detail = $"Service status: {serviceStatus}. Sift never disables Update.",
            Recommendation = "Open Windows Update settings to check for pending updates or pause policies set by your organization.",
            ActionKind = HealthActionKind.OpenWindowsUpdate
        };
    }

    private static HealthCheckRow ScanRecentErrors()
    {
        var events = new List<string>();
        try
        {
            var query = new EventLogQuery("System", PathType.LogName, "*") { ReverseDirection = true };
            using var reader = new EventLogReader(query);
            for (EventRecord? record = reader.ReadEvent(); record is not null && events.Count < 6; record = reader.ReadEvent())
            {
                if (record.TimeCreated is null || record.TimeCreated < DateTime.Now.AddDays(-1)) break;
                if (record.Level is not 1 and not 2) continue;
                events.Add($"{record.TimeCreated:HH:mm} {record.ProviderName}: {TryFormatEvent(record)}");
            }
        }
        catch
        {
            return new HealthCheckRow
            {
                Id = "events",
                Title = "Recent System log errors (24h)",
                Status = HealthStatus.Info,
                Detail = "Event log access was limited for this user session.",
                Recommendation = "Open Event Viewer as administrator for the full System log.",
                ActionKind = HealthActionKind.OpenEventViewer
            };
        }

        if (events.Count == 0)
        {
            return new HealthCheckRow
            {
                Id = "events",
                Title = "Recent System log errors (24h)",
                Status = HealthStatus.Ok,
                Detail = "No Critical/Error entries in the last 24 hours (System log).",
                Recommendation = "Informational events are normal. Sift does not clear or modify event logs."
            };
        }

        return new HealthCheckRow
        {
            Id = "events",
            Title = "Recent System log errors (24h)",
            Status = HealthStatus.Warning,
            Detail = string.Join(" · ", events.Take(3)),
            Recommendation = "These are read-only samples. Investigate in Event Viewer before changing drivers or services.",
            ActionKind = HealthActionKind.OpenEventViewer
        };
    }

    private static IEnumerable<HealthCheckRow> ScanDiskHealth()
    {
        var rows = new List<HealthCheckRow>();
        try
        {
            using var searcher = new System.Management.ManagementObjectSearcher("SELECT Model, Status FROM Win32_DiskDrive");
            var drives = searcher.Get().Cast<System.Management.ManagementObject>().ToList();
            if (drives.Count == 0)
            {
                rows.Add(new HealthCheckRow
                {
                    Id = "smart",
                    Title = "Disk health (WMI)",
                    Status = HealthStatus.Info,
                    Detail = "No physical disk status returned.",
                    Recommendation = "Use the drive manufacturer's tool or Windows Storage settings for SMART details."
                });
                return rows;
            }

            foreach (var drive in drives)
            {
                var model = drive["Model"]?.ToString()?.Trim() ?? "Disk";
                var wmiStatus = drive["Status"]?.ToString()?.Trim() ?? "Unknown";
                var ok = wmiStatus.Equals("OK", StringComparison.OrdinalIgnoreCase);
                rows.Add(new HealthCheckRow
                {
                    Id = $"smart-{model.GetHashCode()}",
                    Title = $"Disk health · {model}",
                    Status = ok ? HealthStatus.Ok : HealthStatus.Warning,
                    Detail = $"WMI Status: {wmiStatus}.",
                    Recommendation = ok
                        ? "WMI reports OK. This is not a substitute for manufacturer SMART tools."
                        : "Non-OK WMI status warrants checking Storage or your drive vendor utility. Sift will not run destructive disk repairs.",
                    ActionKind = HealthActionKind.OpenStorageSettings
                });
            }
        }
        catch
        {
            rows.Add(new HealthCheckRow
            {
                Id = "smart",
                Title = "Disk health (WMI)",
                Status = HealthStatus.Info,
                Detail = "WMI disk health query failed.",
                Recommendation = "Open Storage settings or your drive manufacturer's diagnostic tool."
            });
        }
        return rows;
    }

    private static string TryFormatEvent(EventRecord record)
    {
        try
        {
            return record.FormatDescription()?.Split('\n').FirstOrDefault()?.Trim() ?? record.Id.ToString();
        }
        catch
        {
            return $"Event {record.Id}";
        }
    }
}
