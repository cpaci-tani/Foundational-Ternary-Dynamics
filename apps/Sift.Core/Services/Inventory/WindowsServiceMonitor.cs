using System.ServiceProcess;
using Microsoft.Win32;

namespace Sift.Services;

public sealed record ServiceInfo(
    string Name,
    string DisplayName,
    string Status,
    string StartType,
    bool IsProtected,
    bool CanManage,
    string GroupKey,
    byte[]? IconPng = null);

public static class WindowsServiceMonitor
{
    private static readonly HashSet<string> Protected = new(StringComparer.OrdinalIgnoreCase)
    {
        "WinDefend", "WdNisSvc", "Sense", "WdFilter", "WdBoot", "WdNisDrv",
        "wuauserv", "UsoSvc", "WaaSMedicSvc", "DoSvc",
        "mpssvc", "BFE", "MpsSvc",
        "RpcSs", "RpcEptMapper", "DcomLaunch", "LSM", "SamSs",
        "EventLog", "PlugPlay", "Schedule", "CryptSvc", "Winmgmt",
        "Power", "ProfSvc", "UserManager", "BrokerInfrastructure",
        "BDESVC", "wscsvc", "SecurityHealthService", "InstallService", "AppXSvc", "ClipSVC",
        "LicenseManager", "VSS", "swprv", "SDRSVC", "wbengine", "TrustedInstaller", "bits"
    };

    public static bool IsProtectedName(string name) => Protected.Contains(name);

    public static bool CanManageName(string name, out string reason)
    {
        if (IsProtectedName(name))
        {
            reason = "protected Windows service";
            return false;
        }

        try
        {
            using var key = Registry.LocalMachine.OpenSubKey($@"SYSTEM\CurrentControlSet\Services\{name}");
            var raw = key?.GetValue("ImagePath", null, RegistryValueOptions.DoNotExpandEnvironmentNames)?.ToString();
            if (string.IsNullOrWhiteSpace(raw))
            {
                reason = "service executable is unavailable";
                return false;
            }
            var expanded = Environment.ExpandEnvironmentVariables(raw).Trim();
            var executable = ExtractExecutablePath(expanded);
            var full = Path.GetFullPath(executable);
            var windows = Path.GetFullPath(Environment.GetFolderPath(Environment.SpecialFolder.Windows)).TrimEnd('\\') + "\\";
            if (full.StartsWith(windows, StringComparison.OrdinalIgnoreCase))
            {
                reason = "Windows/system services are read-only";
                return false;
            }
            if (!File.Exists(full))
            {
                reason = "service executable is unavailable";
                return false;
            }
            reason = "third-party executable outside Windows";
            return true;
        }
        catch (Exception exception)
        {
            reason = exception.Message;
            return false;
        }
    }

    private static string ExtractExecutablePath(string command) => AppIconExtractor.ExtractLeadingPath(command);

    private static byte[]? TryReadIcon(string name)
    {
        try
        {
            using var key = Registry.LocalMachine.OpenSubKey($@"SYSTEM\CurrentControlSet\Services\{name}");
            var raw = key?.GetValue("ImagePath")?.ToString();
            return AppIconExtractor.TryExtractPngFromCommandLine(raw);
        }
        catch
        {
            return null;
        }
    }

    public static IReadOnlyList<ServiceInfo> Enumerate()
    {
        var list = new List<ServiceInfo>();
        foreach (var sc in ServiceController.GetServices())
        {
            using (sc)
            {
                try
                {
                    var manageable = CanManageName(sc.ServiceName, out _);
                    list.Add(Describe(sc, manageable));
                }
                catch { /* access denied on some services */ }
            }
        }
        return list.OrderBy(x => x.IsProtected).ThenBy(x => x.DisplayName, StringComparer.OrdinalIgnoreCase).ToList();
    }

    public static ServiceInfo? FindExact(string name)
    {
        if (string.IsNullOrWhiteSpace(name) || name.Length > 256) return null;
        try
        {
            using var service = new ServiceController(name);
            var status = service.Status; // Forces an exact SCM lookup instead of trusting the constructor input.
            if (!service.ServiceName.Equals(name, StringComparison.OrdinalIgnoreCase)) return null;
            var manageable = CanManageName(service.ServiceName, out _);
            return Describe(service, manageable, status);
        }
        catch { return null; }
    }

    private static ServiceInfo Describe(ServiceController service, bool manageable,
        ServiceControllerStatus? knownStatus = null)
    {
        var protectedService = !manageable;
        var status = (knownStatus ?? service.Status).ToString();
        var manageGroup = protectedService ? "Windows/system (read-only)" : "Third-party (manageable)";
        return new ServiceInfo(
            service.ServiceName,
            string.IsNullOrWhiteSpace(service.DisplayName) ? service.ServiceName : service.DisplayName,
            status,
            service.StartType.ToString(),
            protectedService,
            manageable,
            $"{manageGroup} · {status}",
            TryReadIcon(service.ServiceName));
    }

    public static string Act(string name, ServiceActionKind action, ServiceObservedState expectedState)
    {
        if (!CanManageName(name, out var reason)) return $"SKIPPED  {name} ({reason})";
        try
        {
            using var sc = new ServiceController(name);
            var currentState = sc.Status.ToString();
            if (!currentState.Equals(expectedState.ToString(), StringComparison.OrdinalIgnoreCase))
                return $"SKIPPED  {name} (state changed from {expectedState} to {currentState})";
            switch (action)
            {
                case ServiceActionKind.Start:
                    if (expectedState != ServiceObservedState.Stopped)
                        return $"SKIPPED  {name} (Start requires a confirmed Stopped state)";
                    sc.Start();
                    sc.WaitForStatus(ServiceControllerStatus.Running, TimeSpan.FromSeconds(20));
                    return $"STARTED  {name}";
                case ServiceActionKind.Restart:
                    if (expectedState != ServiceObservedState.Running)
                        return $"SKIPPED  {name} (Restart requires a confirmed Running state)";
                    sc.Stop();
                    sc.WaitForStatus(ServiceControllerStatus.Stopped, TimeSpan.FromSeconds(20));
                    sc.Start();
                    sc.WaitForStatus(ServiceControllerStatus.Running, TimeSpan.FromSeconds(20));
                    return $"RESTARTED {name}";
                default:
                    return $"FAILED   {name} (unknown action)";
            }
        }
        catch (Exception ex)
        {
            return $"FAILED   {name}: {ex.Message}";
        }
    }
}
