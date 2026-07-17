using Sift.Models;

namespace Sift.Services;

/// <summary>
/// Single construction site for dashboard telemetry dependencies shared by the WinUI shell
/// and MonitorHost. Hardware monitor instances must not be shared across process lifetimes.
/// </summary>
/// <remarks>
/// Hardware inclusion policy (intentional):
/// - WinUI live dashboard uses <c>IncludeHardware = true</c> so Home widgets always attempt sensors.
/// - MonitorHost uses <c>BackgroundHardwareSensors</c> so background sampling stays opt-in.
/// </remarks>
public sealed record DashboardSlowSampleContext(
    ITweakExecutor Tweaks,
    IActivityStore ActivityStore,
    IHistoryService History,
    IInstalledAppInventory InstalledApps,
    IRecoveryManager Recovery,
    IHealthWorkspaceOrchestrator Health);

public sealed record DashboardTelemetryDependencies(
    IProcessSampler Processes,
    IHardwareMonitorService Hardware,
    IServiceInventory Services,
    IStartupInventory Startup,
    IInstalledAppInventory InstalledApps,
    IRecoveryManager Recovery,
    IHealthWorkspaceOrchestrator Health,
    Func<bool> IncludeHardware,
    Func<DateTimeOffset?> LastMaintenanceScanUtc);

public sealed record DashboardTelemetryHostOptions(
    Func<bool> MonitorEnabled,
    string AppVersion,
    string? PipeName = null,
    Func<DateTimeOffset>? UtcNow = null);

public static class DashboardRuntimeFactory
{
    public static DashboardSlowSampleContext CreateSlowSampleContext(
        IElevationBroker? elevation = null,
        ITweakExecutor? tweaks = null,
        IActivityStore? activityStore = null)
    {
        tweaks ??= new TweakExecutor();
        activityStore ??= new ActivityStore();
        elevation ??= new ElevationBroker();
        var history = new HistoryService(tweaks, activityStore);
        var installedApps = new InstalledAppInventory();
        var recovery = new RecoveryManager(tweaks, elevation);
        var health = new HealthWorkspaceOrchestrator(new HealthInventory(), history);
        return new DashboardSlowSampleContext(tweaks, activityStore, history, installedApps, recovery, health);
    }

    public static DashboardTelemetryDependencies CreateDependencies(
        IProcessSampler processes,
        IHardwareMonitorService hardware,
        IServiceInventory services,
        IStartupInventory startup,
        DashboardSlowSampleContext slow,
        Func<bool> includeHardware,
        Func<DateTimeOffset?> lastMaintenanceScanUtc) =>
        new(processes, hardware, services, startup, slow.InstalledApps, slow.Recovery, slow.Health,
            includeHardware, lastMaintenanceScanUtc);

    public static DashboardTelemetryService CreateInProcess(DashboardTelemetryDependencies deps) =>
        new(deps.Processes, deps.Hardware, deps.Services, deps.Startup, deps.InstalledApps,
            deps.Recovery, deps.Health, deps.IncludeHardware, deps.LastMaintenanceScanUtc);

    public static DashboardMonitorTelemetrySource CreateMonitorClient(
        DashboardTelemetryDependencies deps,
        DashboardTelemetryHostOptions options) =>
        new(CreateInProcess(deps), options.MonitorEnabled, options.AppVersion, options.PipeName, options.UtcNow);

    public static IDashboardTelemetrySource CreateMonitorHostSampler(DashboardTelemetryDependencies deps) =>
        CreateInProcess(deps);

    public static DashboardSamplingCoordinator CreateDefaultCadence() => new();
}
