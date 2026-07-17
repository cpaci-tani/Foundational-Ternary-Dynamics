using System.Diagnostics;
using System.IO;
using Sift.Presentation;

namespace Sift.Services;

public enum ServiceActionKind
{
    Start,
    Restart
}

public enum ServiceObservedState
{
    Stopped,
    Running
}

public sealed record ProcessActionTarget(
    int Id,
    string Name,
    string? ExecutablePath = null,
    int SessionId = -1,
    long StartTimeUtcTicks = 0);

public sealed record ServiceActionTarget(
    string Name,
    string DisplayName,
    ServiceObservedState ExpectedState);

public sealed record GuardedActionPlan<T>(IReadOnlyList<T> Allowed, IReadOnlyList<(T Target, string Reason)> Blocked);

public sealed record GuardedActionResult(
    int Succeeded,
    int Skipped,
    int Failed,
    bool NeedsElevation,
    IReadOnlyList<string> Log);

public interface IGuardedSystemActions
{
    GuardedActionPlan<ProcessActionTarget> PlanProcessEnd(IEnumerable<ProcessActionTarget> targets);
    GuardedActionPlan<ProcessActionTarget> PlanProcessRestart(IEnumerable<ProcessActionTarget> targets);
    GuardedActionPlan<ServiceActionTarget> PlanServiceAction(
        IEnumerable<ServiceActionTarget> targets,
        ServiceActionKind action);
    Task<GuardedActionResult> EndProcessesAsync(
        IEnumerable<ProcessActionTarget> targets,
        CancellationToken cancellationToken = default);
    Task<GuardedActionResult> RestartProcessesAsync(
        IEnumerable<ProcessActionTarget> targets,
        CancellationToken cancellationToken = default);
    GuardedActionResult ActOnServices(
        IEnumerable<ServiceActionTarget> targets,
        ServiceActionKind action,
        CancellationToken cancellationToken = default);
}

public sealed class GuardedSystemActions : IGuardedSystemActions
{
    private readonly IServiceActionRuntime _services;
    private readonly Func<bool> _isElevated;

    private static readonly HashSet<string> ProtectedProcesses = new(StringComparer.OrdinalIgnoreCase)
    {
        "System", "Registry", "Idle", "smss", "csrss", "wininit", "winlogon", "services", "lsass",
        "fontdrvhost", "svchost", "dwm", "sihost", "taskhostw", "explorer"
    };

    public GuardedSystemActions() : this(new WindowsServiceActionRuntime(), ElevationHelper.IsElevated) { }

    internal GuardedSystemActions(IServiceActionRuntime services) : this(services, ElevationHelper.IsElevated) { }

    internal GuardedSystemActions(IServiceActionRuntime services, Func<bool> isElevated)
    {
        _services = services;
        _isElevated = isElevated;
    }

    public GuardedActionPlan<ProcessActionTarget> PlanProcessEnd(IEnumerable<ProcessActionTarget> targets) =>
        PlanProcesses(targets);

    public GuardedActionPlan<ProcessActionTarget> PlanProcessRestart(IEnumerable<ProcessActionTarget> targets)
    {
        var candidates = targets.ToList();
        if (!_isElevated()) return PlanProcesses(candidates);
        return new GuardedActionPlan<ProcessActionTarget>([], candidates.Select(target =>
            (target, ReasonMessages.Format(SiftReasonCode.ProcessElevatedRestartDisabled))).ToList());
    }

    public GuardedActionPlan<ServiceActionTarget> PlanServiceAction(
        IEnumerable<ServiceActionTarget> targets,
        ServiceActionKind action)
    {
        var allowed = new List<ServiceActionTarget>();
        var blocked = new List<(ServiceActionTarget, string)>();
        foreach (var target in targets.DistinctBy(target => target.Name, StringComparer.OrdinalIgnoreCase))
        {
            if (!IsExpectedStateForAction(action, target.ExpectedState))
            {
                blocked.Add((target, ReasonMessages.Format(SiftReasonCode.ServiceActionStateMismatch)));
                continue;
            }
            var current = _services.FindExact(target.Name);
            if (current is null)
            {
                blocked.Add((target, "the exact service is no longer present or readable"));
                continue;
            }
            if (!current.CanManage)
            {
                _services.CanManageName(current.Name, out var reason);
                blocked.Add((target, reason));
            }
            else if (current.StartType.Equals("Disabled", StringComparison.OrdinalIgnoreCase))
                blocked.Add((target, "the service start type is Disabled"));
            else if (!Enum.TryParse<ServiceObservedState>(current.Status, ignoreCase: true, out var currentState) ||
                     currentState != target.ExpectedState)
                blocked.Add((target, $"the service state changed from {target.ExpectedState} to {current.Status}"));
            else allowed.Add(new ServiceActionTarget(current.Name, current.DisplayName, target.ExpectedState));
        }
        return new GuardedActionPlan<ServiceActionTarget>(allowed, blocked);
    }

    public async Task<GuardedActionResult> EndProcessesAsync(
        IEnumerable<ProcessActionTarget> targets,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var plan = PlanProcessEnd(targets);
        var succeeded = 0;
        var failed = 0;
        var log = plan.Blocked.Select(x => $"BLOCKED  {x.Target.Name} ({x.Target.Id}) · {x.Reason}").ToList();
        foreach (var target in plan.Allowed)
        {
            try
            {
                cancellationToken.ThrowIfCancellationRequested();
                using var process = Process.GetProcessById(target.Id);
                ValidateLiveTarget(process, target);
                cancellationToken.ThrowIfCancellationRequested();
                process.Kill(entireProcessTree: true);
                await process.WaitForExitAsync(CancellationToken.None);
                succeeded++;
                log.Add($"ENDED    {target.Name} ({target.Id})");
            }
            catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
            {
                throw;
            }
            catch (Exception ex)
            {
                failed++;
                log.Add($"FAILED   {target.Name} ({target.Id}) · {ex.Message}");
            }
        }
        return new GuardedActionResult(succeeded, plan.Blocked.Count, failed, false, log);
    }

    public async Task<GuardedActionResult> RestartProcessesAsync(
        IEnumerable<ProcessActionTarget> targets,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var plan = PlanProcessRestart(targets);
        var succeeded = 0;
        var failed = 0;
        var log = plan.Blocked.Select(x => $"BLOCKED  {x.Target.Name} ({x.Target.Id}) · {x.Reason}").ToList();
        foreach (var target in plan.Allowed)
        {
            try
            {
                cancellationToken.ThrowIfCancellationRequested();
                using var process = Process.GetProcessById(target.Id);
                ValidateLiveTarget(process, target);
                cancellationToken.ThrowIfCancellationRequested();
                process.Kill(entireProcessTree: true);
                await process.WaitForExitAsync(CancellationToken.None);
                Process.Start(new ProcessStartInfo(target.ExecutablePath!) { UseShellExecute = true });
                succeeded++;
                log.Add($"RESTART  {target.Name} ({target.Id})");
            }
            catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
            {
                throw;
            }
            catch (Exception ex)
            {
                failed++;
                log.Add($"FAILED   {target.Name} ({target.Id}) · {ex.Message}");
            }
        }
        return new GuardedActionResult(succeeded, plan.Blocked.Count, failed, false, log);
    }

    public GuardedActionResult ActOnServices(
        IEnumerable<ServiceActionTarget> targets,
        ServiceActionKind action,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (action is not (ServiceActionKind.Start or ServiceActionKind.Restart))
            return new GuardedActionResult(0, 0, 1, false, [$"FAILED   Unsupported service action: {action}"]);
        var succeeded = 0;
        var blocked = 0;
        var failed = 0;
        var elevation = false;
        var log = new List<string>();
        foreach (var target in targets)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var plan = PlanServiceAction([target], action);
            if (plan.Allowed.Count == 0)
            {
                blocked++;
                log.Add($"BLOCKED  {target.DisplayName} · {plan.Blocked[0].Reason}");
                continue;
            }
            cancellationToken.ThrowIfCancellationRequested();
            var confirmedTarget = plan.Allowed[0];
            var line = _services.Act(confirmedTarget.Name, action, confirmedTarget.ExpectedState);
            log.Add(line);
            if (line.StartsWith(action.ToString(), StringComparison.OrdinalIgnoreCase)) succeeded++;
            else if (line.StartsWith("SKIPPED", StringComparison.OrdinalIgnoreCase)) blocked++;
            else failed++;
            if (line.Contains("access", StringComparison.OrdinalIgnoreCase) ||
                line.Contains("denied", StringComparison.OrdinalIgnoreCase) ||
                line.Contains("privilege", StringComparison.OrdinalIgnoreCase))
                elevation = true;
        }
        return new GuardedActionResult(succeeded, blocked, failed, elevation, log);
    }

    private static GuardedActionPlan<ProcessActionTarget> PlanProcesses(IEnumerable<ProcessActionTarget> targets)
    {
        var allowed = new List<ProcessActionTarget>();
        var blocked = new List<(ProcessActionTarget, string)>();
        foreach (var target in targets)
        {
            if (target.Id == Environment.ProcessId || target.Id <= 4 || ProtectedProcesses.Contains(target.Name))
            {
                blocked.Add((target, ReasonMessages.Format(SiftReasonCode.ProcessProtected)));
                continue;
            }
            using (var current = Process.GetCurrentProcess())
            {
                if (target.SessionId < 0 || target.SessionId != current.SessionId)
                {
                    blocked.Add((target, ReasonMessages.Format(SiftReasonCode.ProcessSessionMismatch)));
                    continue;
                }
            }
            if (string.IsNullOrWhiteSpace(target.ExecutablePath) ||
                target.ExecutablePath.Equals("Unavailable", StringComparison.OrdinalIgnoreCase) ||
                !File.Exists(target.ExecutablePath))
            {
                blocked.Add((target, ReasonMessages.Format(SiftReasonCode.ProcessPathUnreadable)));
                continue;
            }
            if (target.StartTimeUtcTicks <= 0)
            {
                blocked.Add((target, "process start identity is unavailable"));
                continue;
            }
            var full = Path.GetFullPath(target.ExecutablePath);
            var windows = Path.GetFullPath(Environment.GetFolderPath(Environment.SpecialFolder.Windows))
                .TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar;
            var sift = Path.GetFullPath(AppContext.BaseDirectory).TrimEnd(Path.DirectorySeparatorChar) +
                Path.DirectorySeparatorChar;
            if (full.StartsWith(windows, StringComparison.OrdinalIgnoreCase) ||
                full.StartsWith(sift, StringComparison.OrdinalIgnoreCase))
            {
                blocked.Add((target, "Windows and Sift executables are protected"));
                continue;
            }
            try
            {
                using var process = Process.GetProcessById(target.Id);
                ValidateLiveTarget(process, target);
            }
            catch (Exception exception)
            {
                blocked.Add((target, exception.Message));
                continue;
            }
            allowed.Add(target);
        }
        return new GuardedActionPlan<ProcessActionTarget>(allowed, blocked);
    }

    private static void ValidateLiveTarget(Process process, ProcessActionTarget target)
    {
        if (!process.ProcessName.Equals(target.Name, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("The process changed after selection; action cancelled.");
        if (process.SessionId != target.SessionId)
            throw new InvalidOperationException("The process session changed after selection; action cancelled.");
        if (process.StartTime.ToUniversalTime().Ticks != target.StartTimeUtcTicks)
            throw new InvalidOperationException("The process instance changed after selection; action cancelled.");
        string? livePath;
        try { livePath = process.MainModule?.FileName; }
        catch { livePath = null; }
        if (string.IsNullOrWhiteSpace(livePath) ||
            !Path.GetFullPath(livePath).Equals(Path.GetFullPath(target.ExecutablePath!), StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("The executable path changed after selection; action cancelled.");
    }

    private static bool IsExpectedStateForAction(ServiceActionKind action, ServiceObservedState expectedState) =>
        action switch
        {
            ServiceActionKind.Start => expectedState == ServiceObservedState.Stopped,
            ServiceActionKind.Restart => expectedState == ServiceObservedState.Running,
            _ => false
        };
}

internal interface IServiceActionRuntime
{
    ServiceInfo? FindExact(string name);
    bool CanManageName(string name, out string reason);
    string Act(string name, ServiceActionKind action, ServiceObservedState expectedState);
}

internal sealed class WindowsServiceActionRuntime : IServiceActionRuntime
{
    public ServiceInfo? FindExact(string name) => WindowsServiceMonitor.FindExact(name);
    public bool CanManageName(string name, out string reason) => WindowsServiceMonitor.CanManageName(name, out reason);
    public string Act(string name, ServiceActionKind action, ServiceObservedState expectedState) =>
        WindowsServiceMonitor.Act(name, action, expectedState);
}
