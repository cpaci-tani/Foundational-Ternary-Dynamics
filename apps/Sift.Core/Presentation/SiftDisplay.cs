using Humanizer;
using Humanizer.Bytes;

namespace Sift.Presentation;

/// <summary>
/// Presentation-neutral formatting helpers for inventory and dashboard copy.
/// Uses Humanizer for byte sizes and relative times; keeps Sift's compact numeric style.
/// Units are always explicit in rate and memory helpers.
/// </summary>
public static class SiftDisplay
{
    public static string Bytes(long bytes)
    {
        if (bytes < 0) return "—";
        if (bytes == 0) return "0 B";
        return ByteSize.FromBytes(bytes).Humanize("#.##");
    }

    public static string BytesOrDash(long bytes) => bytes <= 0 ? "—" : Bytes(bytes);

    public static string RelativeDaysAgo(double ageDays)
    {
        if (ageDays <= 0) return string.Empty;
        var wholeDays = (int)Math.Round(ageDays);
        if (wholeDays <= 0) wholeDays = 1;
        return DateTimeOffset.UtcNow.AddDays(-wholeDays).Humanize();
    }

    public static string LatestBackupAge(double ageDays) =>
        ageDays > 0 ? $"Latest backup {RelativeDaysAgo(ageDays)}" : "No recent backup details";

    /// <summary>Percent with one decimal and a trailing % sign.</summary>
    public static string Percent(double value) => $"{value:0.0}%";

    /// <summary>CPU utilization with unit word.</summary>
    public static string CpuPercent(double value) => $"{value:0.0}% CPU";

    /// <summary>Memory utilization with unit word.</summary>
    public static string MemoryPercentUsed(double value) => $"{value:0.0}% used";

    /// <summary>Free-space percentage (storage widgets).</summary>
    public static string FreePercent(double value) => $"{value:0.0}% free";

    /// <summary>Battery charge with unit word.</summary>
    public static string BatteryCharge(double value) => $"{value:0.0}% charge";

    /// <summary>Physical memory used/total in GB.</summary>
    public static string PhysicalMemoryGb(double usedGb, double totalGb) =>
        $"{usedGb:0.0} GB of {totalGb:0.0} GB physical";

    /// <summary>Working-set memory in mebibytes (process counters use ÷1,048,576).</summary>
    public static string WorkingSetMiB(double mebibytes) => $"{mebibytes:0} MiB working set";

    /// <summary>Compact working-set label for dense lists.</summary>
    public static string WorkingSetMiBShort(double mebibytes) => $"{mebibytes:0} MiB";

    /// <summary>Network rate in megabits per second.</summary>
    public static string MegabitsPerSec(double mbps) => $"{mbps:0.00} Mbps";

    /// <summary>Disk / process throughput in mebibytes per second.</summary>
    public static string MebibytesPerSec(double mibPerSec) => $"{mibPerSec:0.0} MiB/s";

    public static string DiskReadWriteMiB(double readMib, double writeMib) =>
        $"Read {readMib:0.0} MiB/s · Write {writeMib:0.0} MiB/s";

    public static string NetworkDownUpMbps(double downloadMbps, double uploadMbps) =>
        $"Download {downloadMbps:0.00} Mbps · upload {uploadMbps:0.00} Mbps";

    public static string TemperatureCelsius(double celsius) => $"{celsius:0.0} °C";

    public static string HottestTemperature(double celsius) => $"{celsius:0.0} °C hottest";

    public static string Uptime(double hours) =>
        hours >= 24 ? $"{hours / 24:0.0} days uptime" : $"{hours:0.0} hours uptime";

    public static string HistoryWindow(TimeSpan range) =>
        range.TotalDays >= 1 ? $"{range.TotalDays:0} day history" : $"{range.TotalMinutes:0} min history";

    public static string CountNoun(double count, string singular, string plural) =>
        Math.Abs(count - 1) < 0.0001 ? $"1 {singular}" : $"{count:0} {plural}";

    public static string DaysAgoOrNever(double ageDays, string neverLabel = "No scan yet")
    {
        if (ageDays <= 0) return neverLabel;
        var relative = RelativeDaysAgo(ageDays);
        return string.IsNullOrWhiteSpace(relative) ? neverLabel : $"Last scan {relative}";
    }

    /// <summary>Formats a dashboard metric sample with an explicit unit and optional role word.</summary>
    public static string MetricPrimary(string metricKey, double value, string unit) => metricKey switch
    {
        "cpu.percent" or "cpu.pdh_percent" => CpuPercent(value),
        "memory.percent" => MemoryPercentUsed(value),
        "network.download_mbps" => $"↓ {MegabitsPerSec(value)}",
        "network.upload_mbps" => $"↑ {MegabitsPerSec(value)}",
        "storage.lowest_free_percent" => FreePercent(value),
        var key when key.StartsWith("storage.", StringComparison.OrdinalIgnoreCase) &&
                     key.EndsWith(".free_percent", StringComparison.OrdinalIgnoreCase) => FreePercent(value),
        "battery.charge_percent" => BatteryCharge(value),
        "hardware.hottest_c" => HottestTemperature(value),
        "system.uptime_hours" => Uptime(value),
        _ => unit switch
        {
            "%" => Percent(value),
            "GB" => $"{value:0.0} GB",
            "Mb/s" or "Mbps" => MegabitsPerSec(value),
            "MB/s" or "MiB/s" => MebibytesPerSec(value),
            "°C" => TemperatureCelsius(value),
            "h" => Uptime(value),
            "min" => $"{value:0} min",
            "mW" => $"{value:0} mW",
            "mWh" => $"{value:0} mWh",
            "count" => $"{value:0}",
            _ => $"{value:0.##}{(string.IsNullOrWhiteSpace(unit) ? "" : $" {unit}")}"
        }
    };

    public static string MetricTitle(string metricKey) => metricKey switch
    {
        "cpu.percent" or "cpu.pdh_percent" => "CPU utilization",
        "memory.percent" => "Memory used",
        "network.download_mbps" => "Network download",
        "network.upload_mbps" => "Network upload",
        "storage.lowest_free_percent" => "Lowest free space",
        "battery.charge_percent" => "Battery charge",
        "hardware.hottest_c" => "Hottest temperature",
        "system.uptime_hours" => "System uptime",
        _ => metricKey
    };
}
