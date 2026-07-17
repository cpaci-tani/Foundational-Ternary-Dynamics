using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Infrastructure.Settings;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure.Interop;
using Sift.WinUI.Infrastructure.Monitoring;

namespace Sift.WinUI.Composition;

public sealed record DashboardServices(
    IDashboardProfileStore Profiles,
    IDashboardTelemetrySource Telemetry,
    IDashboardHistoryStore History,
    IDashboardAlertEngine Alerts,
    IDashboardMonitorController Monitor);

public sealed record OptimizeServices(
    ITweakExecutor Tweaks,
    IElevationBroker Elevation,
    IOptimizeMutationWorkflow OptimizeWorkflow,
    IRecoveryManager Recovery,
    IHistoryService History);

public sealed record InventoryServices(
    IProcessSampler Processes,
    IServiceInventory Services,
    IScheduledTaskInventory Tasks,
    IStartupInventory Startup,
    IInstalledAppInventory InstalledApps,
    ISystemInformationService SystemInformation,
    IHardwareMonitorService HardwareMonitor);

public sealed record MutationServices(
    IGuardedSystemActions GuardedActions,
    IScheduledTaskActionWorkflow ScheduledTaskWorkflow,
    IMaintenanceScanner MaintenanceScanner,
    IMaintenanceCleaner MaintenanceCleaner,
    IStorageScanner StorageScanner,
    IStorageSelectionDeletionManager StorageDeletion,
    IInstalledAppManager InstalledAppManager,
    IInstalledAppTrustInspector InstalledAppTrust,
    IAppLeftoverManager AppLeftovers);

public sealed record HardwareGraphServices(
    IDockSession HardwareDock,
    SensorHistoryStore SensorHistory);

public sealed record HealthServices(
    IHealthWorkspaceOrchestrator HealthOrchestrator);

public sealed record ScriptServices(
    IScriptCommandService Scripts,
    IScriptStudioService ScriptStudio);

public sealed record DesktopInteropServices(
    IClipboardService Clipboard,
    IWindowsShellLauncher ShellLauncher);

public sealed record WorkspaceInfrastructure(
    AppSettings Settings,
    ActivityHub Activity,
    OperationCoordinator Operations,
    SettingsPersistenceCoordinator SettingsPersistence);
