using Sift.Presentation;

namespace Sift.UnitTests;

public sealed class SiftDisplayTests
{
    [Fact]
    public void Bytes_formats_common_magnitudes()
    {
        Assert.Equal("0 B", SiftDisplay.Bytes(0));
        Assert.Contains("KB", SiftDisplay.Bytes(2048), StringComparison.OrdinalIgnoreCase);
        Assert.Contains("MB", SiftDisplay.Bytes(5 * 1024 * 1024), StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void BytesOrDash_hides_non_positive()
    {
        Assert.Equal("—", SiftDisplay.BytesOrDash(0));
        Assert.Equal("—", SiftDisplay.BytesOrDash(-1));
    }

    [Fact]
    public void LatestBackupAge_uses_relative_copy()
    {
        Assert.Equal("No recent backup details", SiftDisplay.LatestBackupAge(0));
        var text = SiftDisplay.LatestBackupAge(3);
        Assert.StartsWith("Latest backup ", text, StringComparison.Ordinal);
        Assert.Contains("ago", text, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void MetricPrimary_uses_explicit_units_and_roles()
    {
        Assert.Equal("12.3% CPU", SiftDisplay.MetricPrimary("cpu.percent", 12.3, "%"));
        Assert.Equal("48.0% used", SiftDisplay.MetricPrimary("memory.percent", 48, "%"));
        Assert.Equal("↓ 1.25 Mbps", SiftDisplay.MetricPrimary("network.download_mbps", 1.25, "Mbps"));
        Assert.Equal("12.0% free", SiftDisplay.MetricPrimary("storage.lowest_free_percent", 12, "%"));
        Assert.Equal("87.0% charge", SiftDisplay.MetricPrimary("battery.charge_percent", 87, "%"));
        Assert.Equal("71.0 °C hottest", SiftDisplay.MetricPrimary("hardware.hottest_c", 71, "°C"));
        Assert.Equal("2.5 days uptime", SiftDisplay.MetricPrimary("system.uptime_hours", 60, "h"));
    }

    [Fact]
    public void Rate_helpers_keep_Mbps_and_MiB_distinct()
    {
        Assert.Equal("1.25 Mbps", SiftDisplay.MegabitsPerSec(1.25));
        Assert.Equal("3.4 MiB/s", SiftDisplay.MebibytesPerSec(3.4));
        Assert.Equal("Read 1.2 MiB/s · Write 3.4 MiB/s", SiftDisplay.DiskReadWriteMiB(1.2, 3.4));
        Assert.Equal("Download 1.25 Mbps · upload 0.10 Mbps", SiftDisplay.NetworkDownUpMbps(1.25, 0.10));
    }

    [Fact]
    public void Memory_helpers_label_physical_and_working_set()
    {
        Assert.Equal("7.2 GB of 16.0 GB physical", SiftDisplay.PhysicalMemoryGb(7.2, 16));
        Assert.Equal("512 MiB working set", SiftDisplay.WorkingSetMiB(512));
    }
}
