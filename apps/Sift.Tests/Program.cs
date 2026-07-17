using System.Text.Json;
using Sift.Models;
using Sift.Services;

namespace Sift.Tests;

internal static class Program
{
    private static async Task Main()
    {
        var catalog = TweakCatalog.Create();
        Check(catalog.Count >= 20, "catalog contains the expected baseline");
        Check(catalog.Select(x => x.Id).Distinct().Count() == catalog.Count, "tweak ids are unique");
        Check(catalog.Where(x => x.Minimal).All(x => x.Recommended), "minimal is a subset of balanced");
        Check(catalog.Where(x => x.Recommended).All(x => x.Risk != TweakRisk.Advanced && x.Reversible), "balanced excludes advanced and irreversible actions");
        Check(catalog.Where(x => x.Kind == TweakKind.Registry).All(x => x.DesiredValue is int && x.ValueName is not null), "registry actions are explicit DWORD writes");
        Check(catalog.Where(x => x.Kind == TweakKind.AppPackage).All(x => x.Risk == TweakRisk.Advanced && !x.Reversible), "package removal is advanced and irreversible");
        Check(catalog.All(x => !x.Target.Contains("Windows Defender", StringComparison.OrdinalIgnoreCase)), "catalog does not alter Defender");

        var previewSelection = catalog.Where(x => x.Minimal).ToList();
        var preview = await new TweakExecutor().ApplyAsync(previewSelection, dryRun: true);
        Check(preview.Log.Count == previewSelection.Count && preview.Log.All(x => x.StartsWith("PREFLIGHT")), "Optimize preflight emits audit rows without mutation");
        Check(preview.Previewed == previewSelection.Count && preview.Succeeded == 0, "preflight result counters are honest");

        var processSample = new ProcessSampler().Sample();
        Check(processSample.Processes.Any(x => x.Id == Environment.ProcessId), "process sampler includes the validation process");
        Check(processSample.TotalMemoryGb > 0 && processSample.MemoryPercent is >= 0 and <= 100, "memory telemetry is valid");
        Check(processSample.Processes.All(x => x.MemoryMb >= 0 && x.PrivateMemoryMb >= 0 && x.ReadRateMb >= 0 && x.WriteRateMb >= 0), "process telemetry is non-negative");

        using (var pdh = new PdhSystemSampler())
        {
            Check(pdh.TryOpen(), "PDH system sampler opens");
            _ = pdh.Sample();
            Thread.Sleep(120);
            var counters = pdh.Sample();
            if (counters is not null)
            {
                Check(counters.CpuPercent is >= 0 and <= 100, "PDH CPU percent is in range");
                Check(counters.DiskReadMbPerSec >= 0 && counters.DiskWriteMbPerSec >= 0, "PDH disk rates are non-negative");
            }
        }
        Check(BatteryReportReader.Read() is { } battery && (!battery.Present || battery.ChargePercent is null or (>= 0 and <= 100)),
            "battery report reader is safe");

        var tempRoot = Path.Combine(Path.GetTempPath(), "SiftTests-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempRoot);
        try
        {
            ValidateSettings(tempRoot);
            ValidateCsv(tempRoot, processSample);
            await ValidateMaintenance(tempRoot);
            ValidateStorage(tempRoot);
            ValidateActivity(tempRoot);
            ValidateLegacySettingsMigration(tempRoot);
        }
        finally
        {
            try { Directory.Delete(tempRoot, recursive: true); } catch { }
        }

        ValidateWindowsInventories();
        await ValidateInstalledApps();
        ValidateHealth();
        ValidateSystemInformation();
        ValidateFeatureAuditContract();
        ValidateCanonicalWinUiSource();

        Console.WriteLine($"Sift validation passed: {catalog.Count} tweaks, {previewSelection.Count} preflight-reviewed, {processSample.Processes.Count} processes sampled, WinUI-only source verified.");
    }

    private static void ValidateSettings(string root)
    {
        var store = new SettingsStore(root);
        var settings = new AppSettings
        {
            RefreshInterval = "5 seconds",
            LastWorkspace = "Performance",
            ConsoleVisible = false,
            ConsoleWidth = 420,
            ChartHistory = 120
        };
        settings.ChartHistory = 120;
        settings.OfferSystemRestorePoint = false;
        settings.ChartSmoothing = "High";
        settings.UiScale = "Large";
        store.Save(settings);
        var loaded = store.Load();
        Check(loaded.RefreshInterval == "5 seconds" && loaded.LastWorkspace == "Performance", "settings round-trip");
        Check(!loaded.ConsoleVisible && loaded.ConsoleWidth == 420 && !loaded.OfferSystemRestorePoint,
            "shell and restore-point settings persist");
        Check(loaded.ChartSmoothing == "High", "chart smoothing preference persists");
        Check(loaded.UiScale == "Large", "UI size preference persists");
    }

    private static void ValidateCsv(string root, SystemSnapshot sample)
    {
        var path = Path.Combine(root, "processes.csv");
        var rows = sample.Processes.Take(3).Select(value =>
        {
            var row = new ProcessRow { Id = value.Id, Name = value.Name };
            row.Update(value);
            return row;
        }).ToList();
        ProcessCsvExporter.Export(path, rows);
        Check(File.Exists(path) && File.ReadAllText(path).Contains("Name,PID"), "CSV exporter writes a headered file");
    }

    private static async Task ValidateMaintenance(string root)
    {
        var findings = new MaintenanceScanner().Scan();
        Check(findings.All(x => !string.IsNullOrWhiteSpace(x.Title) && x.SizeBytes >= 0), "maintenance findings are well formed");
        Check(findings.Select(x => string.IsNullOrWhiteSpace(x.RegistrySubKey)
                ? $"path:{x.Path.TrimEnd('\\', '/')}"
                : $"registry:{x.RegistryHive}:{x.RegistrySubKey}")
            .Distinct(StringComparer.OrdinalIgnoreCase).Count() == findings.Count,
            "maintenance findings do not duplicate canonical targets");
        var missing = new MaintenanceFinding
        {
            Id = "test.missing",
            Category = MaintenanceCategory.TempFiles,
            Title = "Missing fixture",
            Path = Path.Combine(root, "missing"),
            Detail = "preview fixture",
            SizeBytes = 1,
            CanClean = true
        };
        var blocked = (await new MaintenanceCleaner().ReviewAsync([missing])).Result;
        Check(blocked.Skipped == 1 && blocked.Log.All(x => x.StartsWith("BLOCKED")), "maintenance rejects caller-constructed paths");
        var recycle = new MaintenanceFinding
        {
            Id = "maintenance.recycle",
            Category = MaintenanceCategory.RecycleBin,
            Title = "Recycle Bin fixture",
            Path = "Recycle Bin",
            Detail = "preview fixture",
            SizeBytes = 1,
            CanClean = true
        };
        var preview = (await new MaintenanceCleaner().ReviewAsync([recycle])).Result;
        Check(preview.Previewed == 1 && preview.Log.All(x => x.StartsWith("REVIEWED")), "maintenance review never mutates");
    }

    private static void ValidateStorage(string root)
    {
        var fixture = Path.Combine(root, "storage");
        Directory.CreateDirectory(Path.Combine(fixture, "sub"));
        File.WriteAllBytes(Path.Combine(fixture, "a.bin"), new byte[1000]);
        File.WriteAllBytes(Path.Combine(fixture, "sub", "b.txt"), new byte[500]);
        var tree = new StorageScanner().Scan([fixture], progress: null, CancellationToken.None);
        Check(tree.RootIndices.Count == 1 && tree.TotalSize == 1500 && tree.TotalFiles == 2, "storage scanner totals fixture content");
        var target = Path.Combine(fixture, "a.bin");
        var preview = new StorageDeleter().MoveToRecycleBin([target], dryRun: true);
        Check(preview.Log.All(x => x.StartsWith("PREFLIGHT")) && File.Exists(target), "storage preflight leaves files intact");
        Check(new StorageDeleter().IsProtected(Environment.GetFolderPath(Environment.SpecialFolder.Windows), out _), "Windows folder is protected");
    }

    private static void ValidateActivity(string root)
    {
        var store = new ActivityStore(Path.Combine(root, "activity"));
        store.Append("Test", "Fixture", "local detail");
        var entry = store.Load().Single();
        Check(entry.Category == "Test" && entry.Summary == "Fixture", "activity store round-trip");
    }

    private static void ValidateWindowsInventories()
    {
        var startup = StartupEnumerator.Enumerate();
        Check(startup.All(x => !string.IsNullOrWhiteSpace(x.Status)), "startup entries include status");
        var services = WindowsServiceMonitor.Enumerate();
        Check(services.Count > 0, "service inventory returns rows");
        Check(WindowsServiceMonitor.IsProtectedName("WinDefend") && WindowsServiceMonitor.IsProtectedName("wuauserv"), "Defender and Update services are protected");
        Check(WindowsServiceMonitor.Act("WinDefend", ServiceActionKind.Start, ServiceObservedState.Stopped).StartsWith("SKIPPED"),
            "protected service action is blocked");
        Check(!ScheduledTaskMonitor.IsAllowlisted(@"\Microsoft\Windows\WindowsUpdate", "Scheduled Start"),
            "Windows Update task is not allowlisted");
        Check(!ScheduledTaskIdentityCatalog.TryResolve(@"\Microsoft\Windows\WindowsUpdate", "Scheduled Start", out _),
            "Windows Update task identity is not resolvable");
    }

    private static async Task ValidateInstalledApps()
    {
        var inventory = new InstalledAppInventory().Enumerate();
        Check(inventory.All(app => !string.IsNullOrWhiteSpace(app.DisplayName)), "installed-app inventory excludes unnamed registry entries");
        Check(inventory.Select(app => app.RegistryLocation.Identity).Distinct(StringComparer.OrdinalIgnoreCase).Count() == inventory.Count,
            "installed-app inventory exposes unique registered identities");
        Check(inventory.Where(app => app.IsOrphanedRegistration).All(app =>
                !string.IsNullOrWhiteSpace(app.OrphanEvidence) && !app.Publisher.Contains("Microsoft", StringComparison.OrdinalIgnoreCase)),
            "leftover registrations require explicit two-signal evidence and exclude Microsoft-owned entries");

        Check(InstalledAppPolicy.TryParseUninstallCommand("MsiExec.exe /I{01234567-89AB-CDEF-0123-456789ABCDEF}", out _, out _),
            "plain interactive MSI uninstall commands are supported");
        Check(!InstalledAppPolicy.TryParseUninstallCommand("powershell.exe -Command Remove-AppxPackage anything", out _, out _),
            "script-host uninstall commands are blocked");
        Check(!InstalledAppPolicy.TryParseUninstallCommand("MsiExec.exe /X{01234567-89AB-CDEF-0123-456789ABCDEF} /qn", out _, out _),
            "silent MSI commands are blocked");

        var values = new InstalledAppRegistryValues("Example Driver", "Example", "1", string.Empty, string.Empty, 0,
            "MsiExec.exe /I{01234567-89AB-CDEF-0123-456789ABCDEF}", false, false, string.Empty, string.Empty);
        Check(!InstalledAppPolicy.Evaluate(values).Allowed, "driver and system-component names are conservatively protected");

        var fixture = new InstalledApp(
            new InstalledAppRegistryLocation("HKCU", "64-bit", @"Software\Microsoft\Windows\CurrentVersion\Uninstall\fixture"),
            "Sift test fixture", "Test", "1.0", string.Empty, string.Empty, 0,
            "MsiExec.exe /I{01234567-89AB-CDEF-0123-456789ABCDEF}", "Current user", true,
            "The registered interactive uninstaller can be opened after confirmation.");
        var manager = new InstalledAppManager(new FixtureInstalledAppInventory(fixture));
        var preview = await manager.UninstallAsync(fixture, preview: true);
        Check(preview.Previewed && !preview.Executed && !preview.Blocked, "installed-app preflight revalidates without launching anything");
        var stale = fixture with { UninstallString = "MsiExec.exe /I{11111111-1111-1111-1111-111111111111}" };
        var blocked = await manager.UninstallAsync(stale, preview: true);
        Check(blocked.Blocked && !blocked.Executed, "stale or forged installed-app commands are rejected");
    }

    private static void ValidateHealth()
    {
        var checks = HealthScanner.Scan();
        Check(checks.Count >= 5 && checks.Any(x => x.Id == "memory"), "health scanner returns baseline checks");
        Check(checks.All(x => !string.IsNullOrWhiteSpace(x.Recommendation)), "health checks include recommendations");
    }

    private static void ValidateSystemInformation()
    {
        var report = new SystemInformationService().Collect();
        Check(report.Items.Count >= 25, "system information returns a detailed local inventory");
        Check(report.Categories.Contains("Windows") && report.Categories.Contains("Processor") &&
              report.Categories.Contains("Memory") && report.Categories.Contains("Storage") &&
              report.Categories.Contains("Network"), "system information covers core hardware and Windows categories");
        Check(report.Items.All(item => !string.IsNullOrWhiteSpace(item.Property) && !string.IsNullOrWhiteSpace(item.Value)),
            "system information properties are complete and displayable");
    }

    private static void ValidateFeatureAuditContract()
    {
        var root = FindRepositoryRoot();
        var app = Path.Combine(root, "apps", "Sift");
        var auditPath = Path.Combine(app, "docs", "audits", "sift-feature-audit.json");
        var validatorPath = Path.Combine(app, "scripts", "validate-feature-audit.ps1");
        Check(File.Exists(auditPath), "canonical Sift feature audit exists");
        Check(File.Exists(validatorPath), "feature-audit validator exists");
        if (!File.Exists(auditPath)) return;

        using var document = JsonDocument.Parse(File.ReadAllText(auditPath));
        var rootElement = document.RootElement;
        Check(rootElement.GetProperty("schemaVersion").GetInt32() == 1, "feature audit uses schema version 1");
        var entries = rootElement.GetProperty("entries").EnumerateArray().ToArray();
        var ids = entries.Select(entry => entry.GetProperty("id").GetString()).ToArray();
        Check(ids.All(id => !string.IsNullOrWhiteSpace(id)) &&
              ids.Distinct(StringComparer.OrdinalIgnoreCase).Count() == ids.Length,
            "feature audit IDs are present and unique");

        foreach (var route in new[]
        {
            "Home", "Optimize", "TaskManager", "Performance", "HardwareMonitor", "Startup", "Maintenance",
            "Health", "Recovery", "Storage", "Apps", "SystemInfo", "Settings"
        })
            Check(entries.Any(entry =>
                    entry.GetProperty("kind").GetString() == "route" &&
                    string.Equals(entry.GetProperty("route").GetString(), route, StringComparison.Ordinal)),
                $"feature audit covers route {route}");

        var statuses = new HashSet<string>(StringComparer.Ordinal)
        {
            "wired", "intentionally-internal", "future", "disconnected",
            "blocked-external", "obsolete"
        };
        Check(entries.All(entry => statuses.Contains(entry.GetProperty("status").GetString() ?? "")),
            "feature audit statuses use the declared vocabulary");
        Check(entries.All(entry =>
                entry.GetProperty("automatedEvidence").ValueKind == JsonValueKind.Array &&
                entry.GetProperty("visualStates").ValueKind == JsonValueKind.Array),
            "feature audit records automated and visual evidence arrays");
    }

    private static void ValidateCanonicalWinUiSource()
    {
        var root = FindRepositoryRoot();
        var app = Path.Combine(root, "apps", "Sift");
        var project = File.ReadAllText(Path.Combine(app, "Sift.csproj"));
        Check(project.Contains("<UseWinUI>true</UseWinUI>") && !project.Contains("UseWPF"), "canonical project is WinUI-only");
        var appManifest = File.ReadAllText(Path.Combine(app, "app.manifest"));
        var elevationHost = Path.Combine(root, "apps", "Sift.ElevationHost");
        var elevationManifest = File.ReadAllText(Path.Combine(elevationHost, "app.manifest"));
        Check(appManifest.Contains("level=\"asInvoker\"") && elevationManifest.Contains("level=\"requireAdministrator\"") &&
              File.Exists(Path.Combine(elevationHost, "Program.cs")) && project.Contains("Sift.ElevationHost.csproj"),
            "the main UI remains standard-user while a separate one-shot helper owns allowlisted elevation");
        var packageTemplate = File.ReadAllText(Path.Combine(app, "Packaging", "AppxManifest.xml.template"));
        var packageScript = File.ReadAllText(Path.Combine(app, "build-msix.ps1"));
        var cleanAccountScript = File.ReadAllText(Path.Combine(app, "scripts", "validate-clean-account.ps1"));
        Check(packageTemplate.Contains("Windows.FullTrustApplication") && packageTemplate.Contains("runFullTrust") &&
              packageScript.Contains("CertificateThumbprint") && packageScript.Contains("signtool") &&
              packageScript.Contains("verify /pa /all") && cleanAccountScript.Contains("Add-AppxPackage") &&
              cleanAccountScript.Contains("ElevatedOperationKind.ValidateElevation") && cleanAccountScript.Contains("Operation = 2") &&
              cleanAccountScript.Contains("Remove-AppxPackage"),
            "MSIX packaging requires explicit trusted signing and verifies the app, helper, and package signatures");
        Check(project.Contains("LiveChartsCore.SkiaSharpView.WinUI"), "canonical graph framework is referenced");
        var shell = File.ReadAllText(Path.Combine(app, "MainWindow.xaml"));
        foreach (var tag in new[] { "Home", "Optimize", "TaskManager", "Performance", "HardwareMonitor", "Startup", "Maintenance", "Health", "Recovery", "Storage", "Apps", "SystemInfo" })
            Check(shell.Contains($"Tag=\"{tag}\""), $"shell contains {tag} route");
        Check(shell.Contains("IsSettingsVisible=\"True\""), "shell exposes the native Settings route");
        Check(shell.Contains("HorizontalContentAlignment=\"Stretch\"") && shell.Contains("VerticalContentAlignment=\"Stretch\""),
            "shell stretches workspace content in both dimensions");
        var performance = File.ReadAllText(Path.Combine(app, "Views", "PerformanceWorkspaceView.xaml"));
        Check(performance.Contains("CartesianChart"), "Performance uses the native chart framework");
        var hardwareMonitor = File.ReadAllText(Path.Combine(app, "Views", "HardwareMonitorWorkspaceView.xaml"));
        var hardwareService = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Inventory", "HardwareMonitorService.cs"));
        Check(hardwareMonitor.Contains("CartesianChart") && hardwareMonitor.Contains("HardwareDeviceTemplate") &&
              hardwareService.Contains("IHardwareSensorProvider") && hardwareService.Contains("LibreHardwareSensorProvider"),
            "Hardware Monitor uses provider-isolated sensors and the canonical chart framework");
        var homeDashboard = File.ReadAllText(Path.Combine(app, "Views", "HomeDashboardWorkspaceView.xaml.cs"));
        var dashboardHost = File.ReadAllText(Path.Combine(app, "Controls", "DashboardWidgetHost.xaml.cs"));
        var dashboardContent = File.ReadAllText(Path.Combine(app, "Controls", "DashboardWidgetContent.xaml.cs"));
        var dashboardPanel = File.ReadAllText(Path.Combine(app, "Controls", "DashboardGridPanel.cs"));
        var monitorProtocol = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Dashboard", "DashboardMonitorProtocol.cs"));
        var monitorProgram = File.ReadAllText(Path.Combine(root, "apps", "Sift.MonitorHost", "Program.cs"));
        Check(homeDashboard.Contains("foreach (var content in _contents.Values) content.ApplySnapshot") &&
              homeDashboard.Contains("if (!_hosts.TryGetValue") &&
              dashboardHost.Contains("if (!ReferenceEquals(ContentHost.Content, content))") &&
              dashboardContent.Contains("existing.UpdateFrom(incoming)") &&
              dashboardContent.Contains("MetricChart.AnimationsSpeed = TimeSpan.Zero") &&
              dashboardContent.Contains("snapshot.HasChangedMetric(key)"),
            "dashboard telemetry updates existing widget, list-row, and chart objects without rebuilding controls or animating samples");
        Check(dashboardPanel.Contains("class DashboardGridPanel") && dashboardPanel.Contains("RepositionThemeTransition") &&
              homeDashboard.Contains("DashboardBreakpoint.Compact") && homeDashboard.Contains("ActualWidth < 480") &&
              homeDashboard.Contains("wasSingleColumnCompact != isSingleColumnCompact") &&
              dashboardHost.Contains("InteractionHandle_PointerCaptureLost") &&
              dashboardHost.Contains("ResetPointerState()"),
            "Home uses the custom span-aware panel and derives its one-column accessibility layout");
        Check(monitorProtocol.Contains("PipeOptions.CurrentUserOnly") &&
              !monitorProtocol.Contains("ElevatedOperationKind") && !monitorProtocol.Contains("ProcessStartInfo") &&
              monitorProgram.Contains("PipeOptions.CurrentUserOnly") && monitorProgram.Contains("\"unsupported\""),
            "monitor IPC is current-user typed telemetry/status only and has no elevation or executable channel");
        Check(File.Exists(Path.Combine(app, "Views", "OptimizeWorkspaceView.xaml")) &&
              File.Exists(Path.Combine(app, "Views", "MaintenanceWorkspaceView.xaml")) &&
              File.Exists(Path.Combine(app, "Views", "HealthWorkspaceView.xaml")),
            "mutation and health workspaces use dedicated views");
        var taskManager = File.ReadAllText(Path.Combine(app, "Views", "TaskManagerWorkspaceView.xaml"));
        var taskManagerModule = File.ReadAllText(Path.Combine(app, "Composition", "TaskManagerWorkspaceModule.cs"));
        var guardedActions = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Guards", "GuardedSystemActions.cs"));
        var elevationBroker = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Elevation", "ElevationBroker.cs"));
        var elevationProgram = File.ReadAllText(Path.Combine(root, "apps", "Sift.ElevationHost", "Program.cs"));
        var scriptModule = File.ReadAllText(Path.Combine(app, "Composition", "ScriptCenterWorkspaceModule.cs"));
        var scriptView = File.ReadAllText(Path.Combine(app, "Views", "ScriptCenterWorkspaceView.xaml.cs"));
        var scriptLibraryView = File.ReadAllText(Path.Combine(app, "Views", "ScriptCenterWorkspaceView.Library.cs"));
        var scriptTerminalView = File.ReadAllText(Path.Combine(app, "Views", "ScriptCenterWorkspaceView.Terminal.cs"));
        var scriptStudioView = File.ReadAllText(Path.Combine(app, "Views", "ScriptCenterWorkspaceView.Studio.cs"));
        var scriptBridgeView = File.ReadAllText(Path.Combine(app, "Views", "ScriptCenterWorkspaceView.Bridge.cs"));
        var scriptLayoutView = File.ReadAllText(Path.Combine(app, "Views", "ScriptCenterWorkspaceView.Layout.cs"));
        var scriptViewXaml = File.ReadAllText(Path.Combine(app, "Views", "ScriptCenterWorkspaceView.xaml"));
        var scriptAccessPolicy = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Scripts", "ScriptRecipeAccessPolicy.cs"));
        Check(scriptModule.Contains("ScriptRecipeAccessPolicy.RequiresConfirmation(recipe)") &&
              scriptModule.Contains("ConfirmStateChangingAsync") &&
              scriptModule.Contains("RunCatalogRecipeAsync") &&
              scriptAccessPolicy.Contains("recipe.Risk != ScriptRisk.ReadOnly") &&
              scriptView.Contains("Read-only recipes run without confirmation") &&
              elevationBroker.Contains("RecipeId") && elevationBroker.Contains("ExpectedRecipeHash") &&
              elevationBroker.Contains("RunCatalogRecipe") &&
              elevationProgram.Contains("RunCatalogRecipe") &&
              !elevationBroker.Contains("string? Command") && !elevationProgram.Contains("request.Command"),
            "Script Studio runs read-only catalog recipes directly, confirms state changes, and elevates administrator recipes by typed catalog identity only");
        Check(scriptView.Contains("IClipboardService clipboard") &&
              scriptLibraryView.Contains("ApplyFilter") && scriptLibraryView.Contains("QuickRunRecipeButton_Click") &&
              scriptTerminalView.Contains("MaximumTerminalLines") && scriptTerminalView.Contains("AppendBatch") &&
              scriptBridgeView.Contains("StudioWebView_WebMessageReceived") &&
              scriptBridgeView.Contains("MaximumBridgeTextCharacters") &&
              scriptBridgeView.Contains("persistAfterExit: false") &&
              scriptStudioView.Contains("core.WebMessageReceived -= StudioWebView_WebMessageReceived") &&
              scriptStudioView.Contains("StudioWebView.Close()") &&
              scriptLayoutView.Contains("RootGrid_SizeChanged") &&
              !scriptView.Contains("StudioWebView_WebMessageReceived") && !scriptStudioView.Contains("JsonDocument.Parse") &&
              scriptViewXaml.Contains("AutomationProperties.Name=\"{Binding CopyAutomationName}\"") &&
              scriptViewXaml.Contains("AutomationProperties.Name=\"Inserted command preview\"") &&
              scriptViewXaml.Contains("AutomationProperties.Name=\"Monaco script editor and xterm analysis terminal\""),
            "Script Studio separates library, terminal, WebView bridge, and layout ownership while preserving its automation surface");
        Check(taskManager.Contains("End selected task") && taskManager.Contains("Restart selected process") &&
              taskManager.Contains("Start selected service") && taskManager.Contains("Restart selected service") &&
              taskManager.Contains("Collapse or expand process inventory") &&
              taskManager.Contains("Collapse or expand service inventory") &&
              taskManager.Contains("Collapse or expand scheduled task inventory"),
            "Task Manager exposes selectable collapsible inventories and guarded process/service action bars");
        var processSampler = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Inventory", "ProcessSampler.cs"));
        var serviceMonitor = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Inventory", "WindowsServiceMonitor.cs"));
        var taskMonitor = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Inventory", "ScheduledTaskMonitor.cs"));
        var startupIcons = File.ReadAllText(Path.Combine(app, "Views", "StartupWorkspaceView.xaml"));
        var performanceIcons = File.ReadAllText(Path.Combine(app, "Views", "PerformanceWorkspaceView.xaml"));
        var performanceCode = File.ReadAllText(Path.Combine(app, "Views", "PerformanceWorkspaceView.xaml.cs"));
        var startupEnumerator = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Inventory", "SystemHelpers.cs"));
        Check(taskManager.Contains("AppIconConverter") &&
              (taskManager.Split("{Binding IconPng").Length - 1) >= 3 &&
              processSampler.Contains("IconFor(executable)") &&
              serviceMonitor.Contains("TryExtractPngFromCommandLine") &&
              taskMonitor.Contains("TryExtractPngFromCommandLine") &&
              startupIcons.Contains("{Binding IconPng") &&
              startupEnumerator.Contains("TryExtractPngFromCommandLine") &&
              performanceIcons.Contains("TopCpuList") &&
              (performanceIcons.Split("{Binding IconPng").Length - 1) >= 2 &&
              (performanceCode.Contains("x.IconPng") || performanceCode.Contains("process.IconPng")),
            "every application inventory (Task Manager, Startup, Performance top lists) shows icons from exact executable paths when available");
        Check(taskManagerModule.Contains("PlanProcessEnd") && taskManagerModule.Contains("ConfirmProcessActionAsync") &&
              taskManagerModule.Contains("PlanServiceAction") && taskManagerModule.Contains("ConfirmServiceActionAsync") &&
              taskManagerModule.Contains("ManageServiceAsync") && taskManagerModule.Contains("StartTimeUtcTicks") &&
              guardedActions.Contains("ValidateLiveTarget") && guardedActions.Contains("process.StartTime.ToUniversalTime().Ticks") &&
              guardedActions.Contains("Windows and Sift executables are protected") &&
              elevationBroker.Contains("TryResolveServiceAction") && elevationBroker.Contains("ExpectedServiceState") &&
              elevationProgram.Contains("ExpectedServiceState") && elevationProgram.Contains("ManageService(request)"),
            "Task Manager binds process start identity and expected service state through confirmation and execution");
        var console = File.ReadAllText(Path.Combine(app, "Controls", "ActivityConsolePanel.xaml"));
        Check(console.Contains("SeverityFilter") && console.Contains("FilterBox") && console.Contains("Copy visible"),
            "activity console exposes structured filtering and copy controls");
        var storage = File.ReadAllText(Path.Combine(app, "Views", "StorageWorkspaceView.xaml"));
        var storageModule = File.ReadAllText(Path.Combine(app, "Composition", "StorageWorkspaceModule.cs"));
        var storageDeletion = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Storage", "StorageSelectionDeletionManager.cs"));
        Check(storage.Contains("StorageTreemapControl") && storage.Contains("Scan storage") && storage.Contains("FILE TYPES") &&
              storage.Contains("Choose storage scan folder") && storage.Contains("Move selected storage item to Recycle Bin") &&
              storage.Contains("Move to Recycle Bin") && !storage.Contains("Content=\"Permanent"),
            "Storage exposes the accessible native treemap, native folder picker, explicit scan, extension legend, and Recycle Bin-only review");
        Check(storageModule.Contains("IStorageSelectionDeletionManager") && storageModule.Contains("PreflightAsync") &&
              storageModule.Contains("ExecuteAsync") && storageDeletion.Contains("Inventory") &&
              storageDeletion.Contains("Revoke") && storageDeletion.Contains("MoveToRecycleBin"),
            "Storage deletion uses a revocable one-use ticket and complete before-and-after inventory before Recycle Bin mutation");
        Check(File.Exists(Path.Combine(app, "Infrastructure", "Windowing", "WindowMinimumSize.cs")),
            "native minimum-window-size policy is present");
        var installedApps = File.ReadAllText(Path.Combine(app, "Views", "InstalledAppsWorkspaceView.xaml"));
        var installedAppsCode = File.ReadAllText(Path.Combine(app, "Views", "InstalledAppsWorkspaceView.xaml.cs"));
        var installedAppsModule = File.ReadAllText(Path.Combine(app, "Composition", "InstalledAppsWorkspaceModule.cs"));
        var installedAppsInventory = File.ReadAllText(Path.Combine(app, "Composition", "InstalledAppsInventoryController.cs"));
        var installedAppsUninstall = File.ReadAllText(Path.Combine(app, "Composition", "InstalledAppUninstallController.cs"));
        var installedAppsLeftovers = File.ReadAllText(Path.Combine(app, "Composition", "InstalledAppLeftoverController.cs"));
        Check(installedApps.Contains("Installed apps") && installedApps.Contains("Header=\"Uninstallable\"") &&
              installedApps.Contains("Leftover registrations") && installedApps.Contains("Scan file leftovers") &&
              installedApps.Contains("Recycle Bin") && installedApps.Contains("Open Windows Installed Apps") &&
              !installedApps.Contains("PreviewToggle") && !installedApps.Contains("FileLeftoverPreviewToggle"),
            "Installed Apps exposes explicit uninstallability, confirmed uninstall/cleanup, exact file-leftover review, and Windows Settings handoff without preview toggles");
        Check(installedAppsCode.Contains("Check uninstall status") &&
              installedAppsUninstall.Contains("WaitForUninstallCompletionAsync") &&
              installedAppsUninstall.Contains("CheckUninstallCompletionAsync") &&
              installedAppsModule.Contains("InstalledAppsInventoryController") &&
              installedAppsModule.Contains("InstalledAppUninstallController") &&
              installedAppsModule.Contains("InstalledAppLeftoverController") &&
              installedAppsModule.Contains("_leftovers.Dispose()") &&
              installedAppsModule.Contains("_uninstall.Dispose()") &&
              installedAppsModule.Contains("_inventory.Dispose()") &&
              installedAppsModule.Contains("_view.ReleaseSubscriptions()") &&
              installedAppsLeftovers.Contains("ContinuationFor(app)"),
            "Installed Apps tracks process completion, supports delegated-uninstaller rechecks, and gates leftover cleanup on verified removal");
        var trustInspector = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Apps", "InstalledAppTrustInspector.cs"));
        Check(installedApps.Contains("UNINSTALLER TRUST") &&
              installedApps.Contains("Selected app uninstaller trust summary") &&
              installedAppsInventory.Contains("IInstalledAppTrustInspector") &&
              trustInspector.Contains("WinVerifyTrust") &&
              trustInspector.Contains("CryptCATAdminEnumCatalogFromHash") &&
              trustInspector.Contains("WtdCacheOnlyUrlRetrieval"),
            "Installed Apps inspects selected uninstallers with local-only embedded and Windows catalog Authenticode verification");
        var recoveryTable = File.ReadAllText(Path.Combine(app, "Views", "RecoveryWorkspaceView.xaml"));
        var systemInfoTable = File.ReadAllText(Path.Combine(app, "Views", "SystemInformationWorkspaceView.xaml"));
        var startupTable = File.ReadAllText(Path.Combine(app, "Views", "StartupWorkspaceView.xaml"));
        static int RowSelections(string xaml) => xaml.Split("SelectionUnit=\"Row\"").Length - 1;
        static int TableViews(string xaml) => xaml.Split("<tv:TableView ").Length - 1;
        Check(RowSelections(taskManager) == TableViews(taskManager) && RowSelections(recoveryTable) == TableViews(recoveryTable) &&
              RowSelections(systemInfoTable) == TableViews(systemInfoTable) && RowSelections(startupTable) == TableViews(startupTable) &&
              RowSelections(installedApps) == TableViews(installedApps),
            "every dense inventory table selects whole rows instead of individual cells");
        var appIconExtractor = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Apps", "AppIconExtractor.cs"));
        var installedAppManager = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Apps", "InstalledAppManager.cs"));
        Check(installedApps.Contains("TableViewTemplateColumn") &&
              installedApps.Contains("Converter={StaticResource AppIconConverter}") &&
              installedApps.Contains("{Binding IconPng") &&
              File.Exists(Path.Combine(app, "Infrastructure", "Converters", "PngToImageSourceConverter.cs")) &&
              appIconExtractor.Contains("TryExtractPng") && appIconExtractor.Contains("ExtractIconEx") &&
              installedAppManager.Contains("IconPng = AppIconExtractor.TryExtractPng") &&
              installedAppsCode.Contains("OrderByDescending(app => app.CanUninstall)"),
            "Installed Apps shows registered DisplayIcon thumbnails and defaults to uninstallable-first ordering");
        var optimize = File.ReadAllText(Path.Combine(app, "Views", "OptimizeWorkspaceView.xaml"));
        var optimizeCode = File.ReadAllText(Path.Combine(app, "Views", "OptimizeWorkspaceView.xaml.cs"));
        var optimizeModule = File.ReadAllText(Path.Combine(app, "Composition", "OptimizeWorkspaceModule.cs"));
        var optimizeWorkflow = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Optimize", "OptimizeMutationWorkflow.cs"));
        var maintenance = File.ReadAllText(Path.Combine(app, "Views", "MaintenanceWorkspaceView.xaml"));
        var maintenanceModule = File.ReadAllText(Path.Combine(app, "Composition", "MaintenanceWorkspaceModule.cs"));
        var health = File.ReadAllText(Path.Combine(app, "Views", "HealthWorkspaceView.xaml"));
        var healthCode = File.ReadAllText(Path.Combine(app, "Views", "HealthWorkspaceView.xaml.cs"));
        var healthModule = File.ReadAllText(Path.Combine(app, "Composition", "HealthWorkspaceModule.cs"));
        var services = File.ReadAllText(Path.Combine(app, "Composition", "WinUiAppServices.cs"));
        var mainWindow = File.ReadAllText(Path.Combine(app, "MainWindow.xaml.cs"));
        var appCode = File.ReadAllText(Path.Combine(app, "App.xaml.cs"));
        var workspaceRegistry = File.ReadAllText(Path.Combine(app, "Composition", "WorkspaceRegistry.cs"));
        var workspaceFactory = File.ReadAllText(Path.Combine(app, "Composition", "WorkspaceRegistryFactory.cs"));
        var settings = File.ReadAllText(Path.Combine(app, "Views", "SettingsWorkspaceView.xaml"));
        Check(optimize.Contains("Apply selected") && maintenance.Contains("Clean selected") &&
              optimizeCode.Contains("ConfirmMutationAsync") &&
              maintenanceModule.Contains("ConfirmCleanAsync") &&
              !optimize.Contains("PreviewToggle") && !maintenance.Contains("PreviewToggle") &&
              !settings.Contains("PreviewToggle"),
            "mutation workspaces check selections before confirmation without preview-mode controls");
        var tweakCatalog = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Optimize", "TweakCatalog.cs"));
        var appXaml = File.ReadAllText(Path.Combine(app, "App.xaml"));
        Check(appXaml.Contains("TypeWorkspaceTitleStyle") && appXaml.Contains("TypeBodyStyle") &&
              appXaml.Contains("TypeMetricValueStyle") && appXaml.Contains("TypePanelTitleStyle") &&
              optimize.Contains("CategoryHost") && optimize.Contains("RiskBox") &&
              optimizeCode.Contains("BuildCategoryColumn") && optimizeCode.Contains("BuildPickerRow") &&
              optimizeCode.Contains("BuildDetailedTooltip") && optimizeCode.Contains("RelayoutCategoryColumns") &&
              !optimizeCode.Contains("FontIcon") &&
              optimizeWorkflow.Contains("IsElevatedOptimizeTweak") &&
              tweakCatalog.Contains("repair.dism-component-cleanup") &&
              tweakCatalog.Contains("repair.sfc-scan") &&
              tweakCatalog.Contains("ElevatedCommandIds") &&
              tweakCatalog.Contains("apps.weather"),
            "Optimize uses flat categorized columns with larger body type, detailed tooltips, formal type scale, and allowlisted Tron-inspired Advanced repair/Appx actions");
        Check(optimizeModule.Contains("IOptimizeMutationWorkflow") && optimizeModule.Contains("OfferSystemRestorePoint") &&
              health.Contains("Refresh health") && health.Contains("Show health checks") &&
              health.Contains("Show activity and recovery history") && health.Contains("Filter health checks") &&
              health.Contains("Filter activity and recovery history") && health.Contains("Loading health") &&
              healthCode.Contains("No checks returned") && healthCode.Contains("No matching checks") &&
              healthCode.Contains("No activity or recovery history yet") && healthCode.Contains("No matching history") &&
              (health.Contains("Partial history") || healthCode.Contains("Partial history")) &&
              healthModule.Contains("Could not load checks") && healthModule.Contains("Could not load history") &&
              healthModule.Contains("IHealthWorkspaceOrchestrator") &&
              !mainWindow.Contains("AddWorkspace(\"Health\"") && workspaceFactory.Contains("HealthWorkspaceModule") &&
              maintenanceModule.Contains("SaveNow(_settings)") &&
              services.Contains("OptimizeWorkflow") &&
              services.Contains("History") && services.Contains("HealthOrchestrator") &&
              !services.Contains("public required IStorageDeleter StorageDeleter"),
            "disconnected capabilities are composed through shared services and dedicated workspaces");
        Check(mainWindow.Contains("IWorkspaceRegistry") && mainWindow.Contains("workspaceFactory.Create") &&
              mainWindow.Contains("ValidateNavigationRegistry") && mainWindow.Contains("Workspace registration does not match shell routes") &&
              !mainWindow.Contains("new OptimizeWorkspaceModule") && !mainWindow.Contains("new RecoveryWorkspaceModule") &&
              workspaceRegistry.Contains("Duplicate workspace key") && workspaceRegistry.Contains("module.Dispose()") &&
              workspaceFactory.Contains("IWorkspaceRegistryFactory") && workspaceFactory.Contains("navigator") &&
              workspaceFactory.Contains("new OptimizeWorkspaceModule") && workspaceFactory.Contains("new RecoveryWorkspaceModule") &&
              appCode.Contains("new WorkspaceRegistryFactory(_services)") && appCode.Contains("MainWindow_Closed") &&
              !mainWindow.Contains("_services.Dispose()") &&
              !services.Contains("public required IHealthInventory Health") &&
              !services.Contains("public required IScheduledTaskActionService TaskActions") &&
              !services.Contains("public required ISystemRestorePointService RestorePoints") &&
              !services.Contains("public required ISettingsStore SettingsStore"),
            "typed workspace composition keeps module construction out of the shell and gives App explicit service ownership");
        var clipboardContract = File.ReadAllText(Path.Combine(app, "Infrastructure", "Interop", "IClipboardService.cs"));
        var clipboardAdapter = File.ReadAllText(Path.Combine(app, "Infrastructure", "Interop", "WinUiClipboardService.cs"));
        var shellContract = File.ReadAllText(Path.Combine(app, "Infrastructure", "Interop", "IWindowsShellLauncher.cs"));
        var shellAdapter = File.ReadAllText(Path.Combine(app, "Infrastructure", "Interop", "WindowsShellLauncher.cs"));
        var directPlatformCalls = Directory.EnumerateFiles(Path.Combine(app, "Views"), "*.cs", SearchOption.AllDirectories)
            .Concat(Directory.EnumerateFiles(Path.Combine(app, "Composition"), "*.cs", SearchOption.AllDirectories))
            .Concat(Directory.EnumerateFiles(Path.Combine(app, "Controls"), "*.cs", SearchOption.AllDirectories))
            .Select(File.ReadAllText)
            .ToArray();
        Check(clipboardContract.Contains("CopyText(string text") &&
              clipboardAdapter.Contains("DataPackageOperation.Copy") && clipboardAdapter.Contains("Clipboard.SetContent") &&
              shellContract.Contains("OpenSettings(WindowsSettingsPage page)") &&
              shellContract.Contains("OpenSystemInformation()") && shellContract.Contains("OpenFolder(string path)") &&
              shellAdapter.Contains("ArgumentList.Add(folder)") && shellAdapter.Contains("ms-settings:startupapps") &&
              shellAdapter.Contains("ms-settings:appsfeatures") &&
              services.Contains("Clipboard = new WinUiClipboardService()") &&
              services.Contains("ShellLauncher = new WindowsShellLauncher()") &&
              workspaceFactory.Contains("desktop.Clipboard") && workspaceFactory.Contains("desktop.ShellLauncher") &&
              directPlatformCalls.All(source => !source.Contains("Clipboard.SetContent", StringComparison.Ordinal) &&
                                                !source.Contains("Process.Start", StringComparison.Ordinal)),
            "clipboard and Windows shell calls are owned by narrow injectable WinUI adapters");
        var performanceModule = File.ReadAllText(Path.Combine(app, "Composition", "PerformanceWorkspaceModule.cs"));
        var hardwareMonitorModule = File.ReadAllText(Path.Combine(app, "Composition", "HardwareMonitorWorkspaceModule.cs"));
        var hardwareMonitorView = File.ReadAllText(Path.Combine(app, "Views", "HardwareMonitorWorkspaceView.xaml.cs"));
        var performanceView = File.ReadAllText(Path.Combine(app, "Views", "PerformanceWorkspaceView.xaml.cs"));
        var smoothingPolicy = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Inventory", "ChartSmoothingPolicy.cs"));
        var uiScalePolicy = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "UiScalePolicy.cs"));
        var mainWindowCode = File.ReadAllText(Path.Combine(app, "MainWindow.xaml.cs"));
        Check(settings.Contains("SmoothingBox") && settings.Contains("Line smoothing") &&
              performanceModule.Contains("SetSmoothing(_settings.ChartSmoothing)") &&
              performanceView.Contains("ChartSmoothingPolicy.ResolveSmoothness") &&
              smoothingPolicy.Contains("ResolveSmoothness"),
            "chart smoothing preference is applied to the production performance charts");
        Check(settings.Contains("UiScaleBox") && settings.Contains("UI size") &&
              uiScalePolicy.Contains("ResolveFactor") && mainWindowCode.Contains("ApplyUiScale") &&
              mainWindow.Contains("ShellContentGrid") &&
              mainWindowCode.Contains("ShellContentGrid.Width = RootGrid.ActualWidth / factor") &&
              mainWindowCode.Contains("ShellContentGrid.Height = RootGrid.RowDefinitions[1].ActualHeight / factor"),
            "UI size preference measures the shell at an inverse logical size before scaling it to the available layout");
        Check(performanceModule.Contains("if (_sampling) return") &&
              performanceModule.Contains("finally") && performanceModule.Contains("_sampling = false") &&
              performanceModule.IndexOf("await SampleAsync(showBusy: true", StringComparison.Ordinal) <
              performanceModule.IndexOf("_timer.Start()", StringComparison.Ordinal) &&
              hardwareMonitorModule.Contains("if (_sampling) return") && hardwareMonitorModule.Contains("finally") &&
              hardwareMonitorModule.Contains("_sampling = false") &&
              hardwareMonitorModule.IndexOf("await RefreshAsync(cancellationToken)", StringComparison.Ordinal) <
              hardwareMonitorModule.IndexOf("_timer.Start()", StringComparison.Ordinal),
            "periodic workspace sampling is serialized and starts only after the initial sample");
        Check(hardwareMonitorView.Contains("DeviceGroups.ItemsSource = _groups") &&
              hardwareMonitorView.Contains("ReconcileCollection(_groups, visibleGroups") &&
              hardwareMonitorView.Contains("sensor.Update(reading)") &&
              hardwareMonitorView.Contains("PropertyChangedEventArgs(nameof(ValueLabel))") &&
              hardwareMonitorView.Contains("SensorChart.AnimationsSpeed = TimeSpan.Zero") &&
              hardwareMonitorView.Contains("Dictionary<string, SensorHistory>") &&
              hardwareMonitorView.Contains("ObservableCollection<double> _selectedChartHistory") &&
              hardwareMonitorView.Contains("PurgeStaleHistories") &&
              !hardwareMonitorView.Contains("_groups.Clear()"),
            "hardware monitor preserves panel and row identity while notifying changed readings without chart animation");
        Check(performanceView.Contains("HistoryChart.AnimationsSpeed = TimeSpan.Zero") &&
              performanceView.Contains("existing.Update(process.Name") &&
              performanceView.Contains("destination.Move(existingIndex, index)") &&
              !performanceView.Contains("destination.Clear()"),
            "performance charts are animation-free and top-process rows reconcile in place");
        foreach (var modulePath in Directory.EnumerateFiles(Path.Combine(app, "Composition"), "*WorkspaceModule.cs"))
        {
            var moduleSource = File.ReadAllText(modulePath);
            foreach (var subscription in moduleSource.Split('\n')
                         .Select(line => line.Trim())
                         .Where(line => line.Contains(" += ", StringComparison.Ordinal)))
            {
                var detachment = subscription.Replace(" += ", " -= ", StringComparison.Ordinal);
                Check(moduleSource.Contains(detachment, StringComparison.Ordinal),
                    $"{Path.GetFileName(modulePath)} detaches {subscription.Split(" += ")[0]}");
            }
        }
        Check(Directory.EnumerateFiles(Path.Combine(app, "Views"), "*.xaml")
                .All(path => !File.ReadAllText(path).Contains("PreviewToggle", StringComparison.OrdinalIgnoreCase)),
            "no native workspace contains a preview toggle");
        var recovery = File.ReadAllText(Path.Combine(app, "Views", "RecoveryWorkspaceView.xaml"));
        var recoveryModule = File.ReadAllText(Path.Combine(app, "Composition", "RecoveryWorkspaceModule.cs"));
        var recoveryManager = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Recovery", "RecoveryManager.cs"));
        var workspaceNavigator = File.ReadAllText(Path.Combine(app, "Composition", "WorkspaceNavigator.cs"));
        var tweakExecutorInterface = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Optimize", "ITweakExecutor.cs"));
        var tweakExecutor = File.ReadAllText(Path.Combine(root, "apps", "Sift.Core", "Services", "Optimize", "TweakExecutor.cs"));
        Check(recovery.Contains("Recovery") && recovery.Contains("Restore backup") &&
              recovery.Contains("Filter recovery backups") && !recovery.Contains("PreviewToggle") &&
              recoveryModule.Contains("ConfirmRestoreAsync") && recoveryModule.Contains("result.Value.Cancelled") &&
              recoveryManager.Contains("InspectExact") &&
              recoveryManager.Contains("RestoreMachineBackupAsync"),
            "Recovery exposes exact backup inventory, automatic preflight confirmation, and scoped protected restore");
        Check(optimize.Contains("Open Recovery backups") &&
              optimizeModule.Contains("_navigator.NavigateTo(\"Recovery\")") &&
              workspaceNavigator.Contains("NavigationRequested") &&
              !optimize.Contains("Restore latest") && !optimizeModule.Contains("ConfirmRestoreAsync") &&
              !optimizeModule.Contains("RestoreLatestAsync") && !tweakExecutorInterface.Contains("RestoreLatestAsync") &&
              !tweakExecutor.Contains("RestoreLatestAsync"),
            "Optimize carries navigation intent only and Recovery remains the sole restore owner");
        var systemInformation = File.ReadAllText(Path.Combine(app, "Views", "SystemInformationWorkspaceView.xaml"));
        var systemInformationCode = File.ReadAllText(Path.Combine(app, "Views", "SystemInformationWorkspaceView.xaml.cs"));
        Check(systemInformation.Contains("System information") && systemInformation.Contains("Open msinfo32") &&
              systemInformation.Contains("Copy report") && systemInformation.Contains("Review identifiers before sharing") &&
              systemInformation.Contains("System information properties") &&
              systemInformation.Contains("ABOUT THIS PC") && systemInformation.Contains("SectionsHost") &&
              !systemInformation.Contains("tv:TableView") && !systemInformation.Contains("<Expander") &&
              systemInformationCode.Contains("RebuildSections") && !systemInformationCode.Contains("new Expander"),
            "System Information uses a scrollable About-style pane with always-visible category sections");
        Check(!Directory.Exists(Path.Combine(root, "apps", "Sift.WinUI")), "staging WinUI project is removed");
        Check(!File.Exists(Path.Combine(app, "AssemblyInfo.cs")) && !Directory.Exists(Path.Combine(app, "Dialogs")), "legacy WPF source is absent");
        Check(!File.Exists(Path.Combine(root, "apps", "Sift.Core", "Services", "TelemetryHub.cs")),
            "obsolete telemetry hub is removed");
    }

    private static void ValidateLegacySettingsMigration(string root)
    {
        var legacy = """
        {
          "OptimizeCategory": "Privacy",
          "OptimizeRiskFilter": "All risks",
          "ChartFps": 60,
          "VisibleColumns": { "Path": true },
          "CpuFilterIndex": 2,
          "PendingOptimizeSelectionIds": ["privacy.activity"],
          "UnknownFutureProperty": { "nested": true },
          "RefreshInterval": "3 seconds",
          "OfferSystemRestorePoint": true
        }
        """;
        var path = Path.Combine(root, "settings.json");
        File.WriteAllText(path, legacy);
        var store = new SettingsStore(root);
        var loaded = store.Load();
        Check(loaded.RefreshInterval == "3 seconds" && loaded.OfferSystemRestorePoint,
            "legacy settings deserialize without breaking retained fields");
    }

    private static string FindRepositoryRoot()
    {
        for (var directory = new DirectoryInfo(Directory.GetCurrentDirectory()); directory is not null; directory = directory.Parent)
            if (File.Exists(Path.Combine(directory.FullName, "apps", "Sift", "Sift.csproj"))) return directory.FullName;
        throw new InvalidOperationException("Could not locate the repository root.");
    }

    private static void Check(bool condition, string message)
    {
        if (!condition) throw new InvalidOperationException($"Validation failed: {message}");
    }

    private sealed class FixtureInstalledAppInventory(InstalledApp fixture) : IInstalledAppInventory
    {
        public IReadOnlyList<InstalledApp> Enumerate(CancellationToken cancellationToken = default) => [fixture];
        public InstalledApp? FindExact(InstalledAppRegistryLocation location) =>
            string.Equals(location.Identity, fixture.RegistryLocation.Identity, StringComparison.OrdinalIgnoreCase) ? fixture : null;
    }
}
