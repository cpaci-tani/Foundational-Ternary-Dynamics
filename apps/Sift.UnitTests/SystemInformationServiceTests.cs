using Sift.Services;

namespace Sift.UnitTests;

public sealed class SystemInformationServiceTests
{
    [Fact]
    public void Collect_builds_typed_multi_category_report_and_keeps_partial_results()
    {
        var source = new FixtureSource();
        var report = new SystemInformationService(source).Collect(cancellationToken: TestContext.Current.CancellationToken);

        Assert.Equal(Environment.MachineName, report.DeviceName);
        Assert.Equal("Acme Workstation", report.DeviceModel);
        Assert.Contains("Windows 11 Pro", report.WindowsVersion);
        Assert.Equal("Acme 16-Core CPU", report.Processor);
        Assert.Equal("32.00 GB", report.Memory);
        Assert.Contains(report.Items, item => item.Category == "Processor" && item.Property == "Physical cores" && item.Value == "16");
        Assert.Contains(report.Items, item => item.Category == "Memory" && item.Property == "Type" && item.Value == "DDR5");
        Assert.Contains(report.Items, item => item.Category == "Storage" && item.Property == "Free percentage" && item.Value == "50.0%");
        Assert.Contains(report.Items, item => item.Category == "Network" && item.Property == "IP addresses" && item.Value.Contains("192.0.2.10"));
        Assert.DoesNotContain(report.Items, item => string.IsNullOrWhiteSpace(item.Value));
        Assert.Contains(report.Warnings, warning => warning.Contains("baseboard", StringComparison.OrdinalIgnoreCase));
    }

    [Fact]
    public void Collect_honors_cancellation_before_provider_queries()
    {
        using var source = new CancellationTokenSource();
        source.Cancel();
        var data = new FixtureSource();

        Assert.Throws<OperationCanceledException>(() =>
            new SystemInformationService(data).Collect(cancellationToken: source.Token));
    }

    private sealed class FixtureSource : ISystemInformationDataSource
    {
        public IReadOnlyList<IReadOnlyDictionary<string, object?>> Query(string namespacePath, string wql, CancellationToken cancellationToken)
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (wql.Contains("Win32_BaseBoard", StringComparison.Ordinal)) throw new InvalidOperationException("fixture provider offline");
            if (wql.Contains("Win32_OperatingSystem", StringComparison.Ordinal)) return [Row(
                ("Caption", "Windows 11 Pro"), ("Version", "10.0.26100"), ("BuildNumber", "26100"),
                ("OSArchitecture", "64-bit"), ("TotalVisibleMemorySize", 33_554_432UL), ("FreePhysicalMemory", 8_388_608UL))];
            if (wql.Contains("Win32_ComputerSystem", StringComparison.Ordinal)) return [Row(
                ("Manufacturer", "Acme"), ("Model", "Workstation"), ("TotalPhysicalMemory", 34_359_738_368UL),
                ("NumberOfProcessors", 1U), ("NumberOfLogicalProcessors", 32U), ("PartOfDomain", false), ("Workgroup", "WORKGROUP"))];
            if (wql.Contains("Win32_Processor", StringComparison.Ordinal)) return [Row(
                ("Name", "Acme 16-Core CPU"), ("NumberOfCores", 16U), ("NumberOfLogicalProcessors", 32U),
                ("MaxClockSpeed", 5700U), ("L3CacheSize", 131_072U), ("VirtualizationFirmwareEnabled", true))];
            if (wql.Contains("Win32_PhysicalMemory", StringComparison.Ordinal)) return [Row(
                ("DeviceLocator", "DIMM A1"), ("Capacity", 34_359_738_368UL), ("ConfiguredClockSpeed", 6000U),
                ("SMBIOSMemoryType", 34U), ("FormFactor", 8U))];
            if (wql.Contains("Win32_LogicalDisk", StringComparison.Ordinal)) return [Row(
                ("DeviceID", "C:"), ("FileSystem", "NTFS"), ("Size", 1_000_000UL), ("FreeSpace", 500_000UL))];
            if (wql.Contains("Win32_NetworkAdapterConfiguration", StringComparison.Ordinal)) return [Row(
                ("Description", "Ethernet"), ("IPAddress", new[] { "192.0.2.10", "2001:db8::10" }),
                ("DefaultIPGateway", new[] { "192.0.2.1" }), ("DHCPEnabled", true))];
            return [];
        }

        private static IReadOnlyDictionary<string, object?> Row(params (string Key, object? Value)[] values) =>
            values.ToDictionary(value => value.Key, value => value.Value, StringComparer.OrdinalIgnoreCase);
    }
}
