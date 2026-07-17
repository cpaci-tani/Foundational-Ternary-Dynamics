using Sift.WinUI.Infrastructure.Windowing;

namespace Sift.WinUI.Composition;

public interface IWorkspaceRegistryFactory
{
    IWorkspaceRegistry Create(IWorkspaceNavigator navigator, Func<nint> windowHandle);
}

public sealed class WorkspaceRegistryFactory(WinUiAppServices services) : IWorkspaceRegistryFactory
{
    public IWorkspaceRegistry Create(IWorkspaceNavigator navigator, Func<nint> windowHandle)
    {
        ArgumentNullException.ThrowIfNull(navigator);
        ArgumentNullException.ThrowIfNull(windowHandle);

        var infra = services.Infrastructure;
        var dashboard = services.Dashboard;
        var optimize = services.Optimize;
        var inventory = services.Inventory;
        var mutations = services.Mutations;
        var scripting = services.Scripting;
        var hardwareGraphs = services.HardwareGraphs;
        var health = services.Health;
        var desktop = services.Desktop;

        var modules = new List<IWorkspaceModule>();
        try
        {
            modules.Add(new HomeDashboardWorkspaceModule(
                dashboard.Profiles, dashboard.Telemetry, dashboard.History,
                dashboard.Alerts, infra.Settings, infra.SettingsPersistence,
                infra.Operations, infra.Activity, navigator, desktop.Clipboard,
                new DashboardActionRouter(
                    optimize.Tweaks, optimize.Elevation, optimize.OptimizeWorkflow,
                    mutations.MaintenanceScanner, mutations.MaintenanceCleaner,
                    inventory.Processes, mutations.GuardedActions, dashboard.Alerts)));
            modules.Add(new OptimizeWorkspaceModule(
                optimize.Tweaks, optimize.Elevation, optimize.OptimizeWorkflow, infra.Settings,
                infra.Operations, infra.Activity, navigator));
            modules.Add(new TaskManagerWorkspaceModule(
                inventory.Processes, inventory.Services, inventory.Tasks, mutations.GuardedActions,
                mutations.ScheduledTaskWorkflow, optimize.Elevation, infra.Operations, infra.Activity));
            modules.Add(new PerformanceWorkspaceModule(
                inventory.Processes, infra.Operations, infra.Activity, infra.Settings,
                infra.SettingsPersistence));
            modules.Add(new HardwareMonitorWorkspaceModule(
                inventory.HardwareMonitor, hardwareGraphs.HardwareDock, hardwareGraphs.SensorHistory,
                infra.Operations, infra.Activity, desktop.Clipboard, infra.Settings,
                Microsoft.UI.Dispatching.DispatcherQueue.GetForCurrentThread()));
            modules.Add(new StartupWorkspaceModule(
                inventory.Startup, infra.Operations, infra.Activity, desktop.ShellLauncher));
            modules.Add(new MaintenanceWorkspaceModule(
                mutations.MaintenanceScanner, mutations.MaintenanceCleaner, infra.Operations,
                infra.Activity, infra.Settings, infra.SettingsPersistence));
            modules.Add(new ScriptCenterWorkspaceModule(
                scripting.Scripts, scripting.ScriptStudio, optimize.Elevation, infra.Activity, infra.Operations,
                desktop.Clipboard, desktop.ShellLauncher));
            modules.Add(new HealthWorkspaceModule(
                health.HealthOrchestrator, infra.Operations, infra.Activity));
            modules.Add(new RecoveryWorkspaceModule(
                optimize.Recovery, optimize.Tweaks, infra.Operations, infra.Activity, desktop.ShellLauncher));
            modules.Add(new StorageWorkspaceModule(
                mutations.StorageScanner, mutations.StorageDeletion, new FolderPickerService(windowHandle),
                infra.Operations, infra.SettingsPersistence, infra.Activity, infra.Settings));
            modules.Add(new InstalledAppsWorkspaceModule(
                inventory.InstalledApps, mutations.InstalledAppManager, mutations.AppLeftovers,
                mutations.InstalledAppTrust, infra.Operations, infra.Activity, desktop.ShellLauncher));
            modules.Add(new SystemInformationWorkspaceModule(
                inventory.SystemInformation, infra.Operations, infra.Activity,
                desktop.Clipboard, desktop.ShellLauncher));
            modules.Add(new SettingsWorkspaceModule(
                infra.Settings, infra.SettingsPersistence, infra.Activity,
                dashboard.Monitor, dashboard.History));
        }
        catch
        {
            foreach (var module in modules)
            {
                try { module.Dispose(); }
                catch (Exception exception) { System.Diagnostics.Debug.WriteLine(exception); }
            }
            throw;
        }

        return new WorkspaceRegistry(modules);
    }
}
