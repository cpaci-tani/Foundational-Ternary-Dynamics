using Sift.Services;

namespace Sift.UnitTests;

public sealed class DashboardRuntimeFactoryTests
{
    [Fact]
    public void CreateSlowSampleContext_wires_recovery_and_health_from_shared_history()
    {
        var slow = DashboardRuntimeFactory.CreateSlowSampleContext();
        Assert.NotNull(slow.Tweaks);
        Assert.NotNull(slow.ActivityStore);
        Assert.NotNull(slow.History);
        Assert.NotNull(slow.InstalledApps);
        Assert.NotNull(slow.Recovery);
        Assert.NotNull(slow.Health);
    }

    [Fact]
    public void CreateInProcess_and_monitor_client_share_dependency_shape()
    {
        var slow = DashboardRuntimeFactory.CreateSlowSampleContext();
        var hardware = new HardwareMonitorService();
        try
        {
            var deps = DashboardRuntimeFactory.CreateDependencies(
                new ProcessSampler(), hardware, new ServiceInventory(), new StartupInventory(), slow,
                includeHardware: () => false,
                lastMaintenanceScanUtc: () => null);
            var inProcess = DashboardRuntimeFactory.CreateInProcess(deps);
            var client = DashboardRuntimeFactory.CreateMonitorClient(
                deps, new DashboardTelemetryHostOptions(() => false, "0.0.0"));
            Assert.IsType<DashboardTelemetryService>(inProcess);
            Assert.IsType<DashboardMonitorTelemetrySource>(client);
            Assert.Same(inProcess.GetType(), DashboardRuntimeFactory.CreateMonitorHostSampler(deps).GetType());
        }
        finally
        {
            hardware.Dispose();
        }
    }

    [Fact]
    public void CreateDefaultCadence_returns_coordinator()
    {
        Assert.NotNull(DashboardRuntimeFactory.CreateDefaultCadence());
    }
}
