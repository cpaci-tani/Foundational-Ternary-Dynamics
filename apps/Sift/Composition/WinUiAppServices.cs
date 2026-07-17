using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Logging;
using Sift.Infrastructure.Operations;
using Sift.Infrastructure.Settings;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure.Interop;
using Sift.WinUI.Infrastructure.Monitoring;

namespace Sift.WinUI.Composition;

public sealed class WinUiAppServices : IDisposable
{
    private bool _disposed;

    public required AppSettings Settings { get; init; }
    public required ITweakExecutor Tweaks { get; init; }
    public required IProcessSampler Processes { get; init; }
    public required IHardwareMonitorService HardwareMonitor { get; init; }
    public required IDockSession HardwareDock { get; init; }
    public required SensorHistoryStore SensorHistory { get; init; }
    public required IMaintenanceScanner MaintenanceScanner { get; init; }
    public required IMaintenanceCleaner MaintenanceCleaner { get; init; }
    public required IStorageScanner StorageScanner { get; init; }
    public required IStorageSelectionDeletionManager StorageDeletion { get; init; }
    public required IServiceInventory Services { get; init; }
    public required IScheduledTaskInventory Tasks { get; init; }
    public required IStartupInventory Startup { get; init; }
    public required IInstalledAppInventory InstalledApps { get; init; }
    public required IInstalledAppManager InstalledAppManager { get; init; }
    public required IInstalledAppTrustInspector InstalledAppTrust { get; init; }
    public required IAppLeftoverManager AppLeftovers { get; init; }
    public required ISystemInformationService SystemInformation { get; init; }
    public required IGuardedSystemActions GuardedActions { get; init; }
    public required IElevationBroker Elevation { get; init; }
    public required IScheduledTaskActionWorkflow ScheduledTaskWorkflow { get; init; }
    public required IOptimizeMutationWorkflow OptimizeWorkflow { get; init; }
    public required IHistoryService History { get; init; }
    public required IHealthWorkspaceOrchestrator HealthOrchestrator { get; init; }
    public required IRecoveryManager Recovery { get; init; }
    public required ActivityHub Activity { get; init; }
    public required ISiftLog Log { get; init; }
    public required OperationCoordinator Operations { get; init; }
    public required SettingsPersistenceCoordinator SettingsPersistence { get; init; }
    public required IScriptCommandService Scripts { get; init; }
    public required IScriptStudioService ScriptStudio { get; init; }
    public required IDashboardProfileStore DashboardProfiles { get; init; }
    public required IDashboardTelemetrySource DashboardTelemetry { get; init; }
    public required IDashboardHistoryStore DashboardHistory { get; init; }
    public required IDashboardAlertEngine DashboardAlerts { get; init; }
    public required IDashboardMonitorController DashboardMonitor { get; init; }
    public required IClipboardService Clipboard { get; init; }
    public required IWindowsShellLauncher ShellLauncher { get; init; }

    public WinUiShellServices Shell => new(Settings, Activity, SettingsPersistence, Clipboard);

    public WorkspaceInfrastructure Infrastructure =>
        new(Settings, Activity, Operations, SettingsPersistence);

    public DashboardServices Dashboard =>
        new(DashboardProfiles, DashboardTelemetry, DashboardHistory, DashboardAlerts, DashboardMonitor);

    public OptimizeServices Optimize =>
        new(Tweaks, Elevation, OptimizeWorkflow, Recovery, History);

    public InventoryServices Inventory =>
        new(Processes, Services, Tasks, Startup, InstalledApps, SystemInformation, HardwareMonitor);

    public MutationServices Mutations =>
        new(GuardedActions, ScheduledTaskWorkflow, MaintenanceScanner, MaintenanceCleaner,
            StorageScanner, StorageDeletion, InstalledAppManager, InstalledAppTrust, AppLeftovers);

    public ScriptServices Scripting => new(Scripts, ScriptStudio);

    public HardwareGraphServices HardwareGraphs => new(HardwareDock, SensorHistory);

    public HealthServices Health => new(HealthOrchestrator);

    public DesktopInteropServices Desktop => new(Clipboard, ShellLauncher);

    public static WinUiAppServices CreateDefault()
    {
        var migration = ProductPaths.EnsureLegacyDataMigrated();
        var log = new SiftFileLog();
        var activityStore = new ActivityStore();
        var activity = new ActivityHub(new ActivityStoreSink(activityStore));
        if (migration.Changed)
        {
            activity.Info("App", "Migrated application data to Sift", migration.Detail, persist: true);
            log.Information("App", $"Migrated application data to Sift · {migration.Detail}");
        }
        else if (migration.Status is ProductDataMigrationStatus.BlockedReparsePoint or ProductDataMigrationStatus.Failed)
        {
            activity.Warning("App", "Legacy application data was not migrated", migration.Detail, persist: true);
            log.Warning("App", "Legacy application data was not migrated", migration.Detail);
        }
        log.Information("App", "WinUI services constructed");
        var settingsStore = new SettingsStore();
        var settings = settingsStore.Load();
        settings.Dashboard ??= new DashboardPreferences();
        settings.HardwareCharts ??= new HardwareChartPreferences();
        if (settings.Dashboard.AlertRules.Count == 0) settings.Dashboard.AlertRules = DashboardAlertDefaults.Create();
        var installedApps = new InstalledAppInventory();
        var storageDeleter = new StorageDeleter();
        var installedAppManager = new InstalledAppManager(installedApps, storageDeleter: storageDeleter);
        var tweaks = new TweakExecutor();
        var elevation = new ElevationBroker();
        var taskController = new ScheduledTaskController();
        var taskActions = new ScheduledTaskActionService(taskController, elevation, ElevationHelper.IsElevated);
        var restorePoints = new SystemRestorePointService(
            new SystemRestorePointController(), elevation, ElevationHelper.IsElevated);
        var history = new HistoryService(tweaks, activityStore);
        var processes = new ProcessSampler();
        var hardwareMonitor = new HardwareMonitorService();
        var serviceInventory = new ServiceInventory();
        var startupInventory = new StartupInventory();
        var recovery = new RecoveryManager(tweaks, elevation);
        var healthOrchestrator = new HealthWorkspaceOrchestrator(new HealthInventory(), history);
        var dashboardHistory = new DashboardHistoryStore();
        var dashboardAlerts = new DashboardAlertEngine(dashboardHistory);
        var appVersion = typeof(WinUiAppServices).Assembly.GetName().Version?.ToString(3) ?? "0.0.0";
        var slowSample = new DashboardSlowSampleContext(
            tweaks, activityStore, history, installedApps, recovery, healthOrchestrator);
        // Live Home dashboard always attempts hardware metrics; MonitorHost uses BackgroundHardwareSensors.
        var dashboardDeps = DashboardRuntimeFactory.CreateDependencies(
            processes, hardwareMonitor, serviceInventory, startupInventory, slowSample,
            includeHardware: () => true,
            lastMaintenanceScanUtc: () => DateTimeOffset.TryParse(settings.LastMaintenanceScanUtc, out var parsed)
                ? parsed
                : null);
        return new WinUiAppServices
        {
            Settings = settings,
            Tweaks = tweaks,
            Processes = processes,
            HardwareMonitor = hardwareMonitor,
            HardwareDock = new DockSession(new DockLayoutStore(
                DockShellIds.HardwareSensors, defaultBoardTitle: "Hardware graphs")),
            SensorHistory = new SensorHistoryStore(Math.Clamp(settings.HardwareCharts.HistorySamples, 30, 600)),
            MaintenanceScanner = new MaintenanceScanner(),
            MaintenanceCleaner = new MaintenanceCleaner(),
            StorageScanner = new StorageScanner(),
            StorageDeletion = new StorageSelectionDeletionManager(storageDeleter),
            Services = serviceInventory,
            Tasks = new ScheduledTaskInventory(),
            Startup = startupInventory,
            InstalledApps = installedApps,
            InstalledAppManager = installedAppManager,
            InstalledAppTrust = new InstalledAppTrustInspector(installedApps),
            AppLeftovers = installedAppManager,
            SystemInformation = new SystemInformationService(),
            GuardedActions = new GuardedSystemActions(),
            Elevation = elevation,
            ScheduledTaskWorkflow = new ScheduledTaskActionWorkflow(taskActions),
            OptimizeWorkflow = new OptimizeMutationWorkflow(tweaks, restorePoints),
            History = history,
            HealthOrchestrator = healthOrchestrator,
            Recovery = recovery,
            Activity = activity,
            Log = log,
            Operations = new OperationCoordinator(activity),
            SettingsPersistence = new SettingsPersistenceCoordinator(settingsStore),
            Scripts = new ScriptCommandService(),
            ScriptStudio = new ScriptStudioService(),
            DashboardProfiles = new DashboardProfileStore(),
            DashboardTelemetry = DashboardRuntimeFactory.CreateMonitorClient(
                dashboardDeps,
                new DashboardTelemetryHostOptions(
                    () => settings.Dashboard.MonitorWhenClosed,
                    appVersion)),
            DashboardHistory = dashboardHistory,
            DashboardAlerts = dashboardAlerts,
            DashboardMonitor = new DashboardMonitorController(appVersion),
            Clipboard = new WinUiClipboardService(),
            ShellLauncher = new WindowsShellLauncher()
        };
    }

    /// <summary>
    /// Owns process-lifetime disposable Core services constructed by <see cref="CreateDefault"/>.
    /// Workspace modules and the registry are disposed by <c>MainWindow</c> before this runs.
    /// Non-disposable services (scanners, brokers, inventories) hold no unmanaged lifetime and are omitted.
    /// </summary>
    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        foreach (var disposable in OwnedDisposables)
            DisposeOne(disposable);
    }

    /// <summary>
    /// Ordered disposal list for CreateDefault-owned <see cref="IDisposable"/> services.
    /// Keep in sync when adding a disposable to <see cref="CreateDefault"/>.
    /// </summary>
    internal IReadOnlyList<IDisposable> OwnedDisposables =>
    [
        Operations,
        HardwareMonitor,
        DashboardHistory,
        SettingsPersistence,
        Log,
        DashboardTelemetry
    ];

    private static void DisposeOne(IDisposable disposable)
    {
        try { disposable.Dispose(); }
        catch (Exception exception) { System.Diagnostics.Debug.WriteLine(exception); }
    }
}
