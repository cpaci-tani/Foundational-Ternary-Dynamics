using Sift.Models;
using System.Diagnostics;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class GuardedSystemActionsTests
{
    private readonly GuardedSystemActions _actions = new();

    [Fact]
    public void ProcessPlan_BlocksSiftCriticalAndMissingPathTargets()
    {
        var end = _actions.PlanProcessEnd([
            new ProcessActionTarget(Environment.ProcessId, "Sift"),
            new ProcessActionTarget(4, "System"),
            new ProcessActionTarget(999999, "example", Environment.ProcessPath, -1)
        ]);
        var restart = _actions.PlanProcessRestart([new ProcessActionTarget(999999, "example", "Unavailable")]);

        Assert.Equal(3, end.Blocked.Count);
        Assert.Empty(end.Allowed);
        Assert.Empty(restart.Allowed);
        Assert.Single(restart.Blocked);
    }

    [Fact]
    public void ProcessPlan_AllowsOnlyMatchingCurrentSessionProcessInstances()
    {
        var fixture = StartFixtureProcess();
        try
        {
            var target = Target(fixture.Process, fixture.ExecutablePath);
            var allowed = _actions.PlanProcessEnd([
                target
            ]);
            var crossSession = _actions.PlanProcessEnd([
                target with { SessionId = target.SessionId + 1 }
            ]);

            Assert.Single(allowed.Allowed);
            Assert.Empty(allowed.Blocked);
            Assert.Empty(crossSession.Allowed);
            Assert.Contains("current interactive session", Assert.Single(crossSession.Blocked).Reason,
                StringComparison.OrdinalIgnoreCase);
        }
        finally { DisposeFixture(fixture); }
    }

    [Fact]
    public void ProcessPlan_RejectsAReusedPidWhenStartIdentityChanged()
    {
        var fixture = StartFixtureProcess();
        try
        {
            var stale = Target(fixture.Process, fixture.ExecutablePath) with
            {
                StartTimeUtcTicks = fixture.Process.StartTime.ToUniversalTime().Ticks + 1
            };

            var plan = _actions.PlanProcessEnd([stale]);

            Assert.Empty(plan.Allowed);
            Assert.Contains("process instance changed", Assert.Single(plan.Blocked).Reason,
                StringComparison.OrdinalIgnoreCase);
            Assert.False(fixture.Process.HasExited);
        }
        finally { DisposeFixture(fixture); }
    }

    [Fact]
    public async Task ProcessCancellation_LeavesConfirmedProcessRunning()
    {
        var fixture = StartFixtureProcess();
        try
        {
            using var cancellation = new CancellationTokenSource();
            cancellation.Cancel();
            var target = Target(fixture.Process, fixture.ExecutablePath);

            await Assert.ThrowsAsync<OperationCanceledException>(() =>
                _actions.EndProcessesAsync([target], cancellation.Token));

            Assert.False(fixture.Process.HasExited);
        }
        finally { DisposeFixture(fixture); }
    }

    [Fact]
    public async Task Elevated_process_restart_is_blocked_without_ending_or_relaunching_the_target()
    {
        var fixture = StartFixtureProcess();
        try
        {
            var actions = new GuardedSystemActions(
                new RecordingServiceRuntime(Service("Running")),
                isElevated: () => true);
            var target = Target(fixture.Process, fixture.ExecutablePath);

            Assert.Single(actions.PlanProcessEnd([target]).Allowed);
            var plan = actions.PlanProcessRestart([target]);
            var blocked = Assert.Single(plan.Blocked);
            Assert.Contains("disabled while Sift is elevated", blocked.Reason, StringComparison.OrdinalIgnoreCase);

            var result = await actions.RestartProcessesAsync([target], TestContext.Current.CancellationToken);

            Assert.Equal(0, result.Succeeded);
            Assert.Equal(1, result.Skipped);
            Assert.Equal(0, result.Failed);
            Assert.False(fixture.Process.HasExited);
        }
        finally { DisposeFixture(fixture); }
    }

    [Fact]
    public void ServiceActions_RecheckProtectedNamesBelowUiLayer()
    {
        var result = _actions.ActOnServices([
                new ServiceActionTarget("WinDefend", "Microsoft Defender", ServiceObservedState.Running)
            ], ServiceActionKind.Restart, TestContext.Current.CancellationToken);

        Assert.Equal(0, result.Succeeded);
        Assert.Equal(1, result.Skipped);
        Assert.Contains(result.Log, line => line.StartsWith("BLOCKED"));
    }

    [Fact]
    public void ServicePlan_ExposesPolicyWithoutMutation()
    {
        var service = _actions.PlanServiceAction([
            new ServiceActionTarget("EventLog", "Windows Event Log", ServiceObservedState.Running)
        ], ServiceActionKind.Restart);

        Assert.Empty(service.Allowed);
        Assert.Single(service.Blocked);
    }

    [Fact]
    public void ServiceAction_RejectsStateDriftAfterConfirmation()
    {
        var runtime = new RecordingServiceRuntime(Service("Running"));
        var actions = new GuardedSystemActions(runtime);
        var target = new ServiceActionTarget("AcmeService", "Acme Service", ServiceObservedState.Running);
        Assert.Single(actions.PlanServiceAction([target], ServiceActionKind.Restart).Allowed);
        runtime.Current = Service("Stopped");

        var result = actions.ActOnServices([target], ServiceActionKind.Restart,
            TestContext.Current.CancellationToken);

        Assert.Equal(0, result.Succeeded);
        Assert.Equal(1, result.Skipped);
        Assert.Equal(0, runtime.ActionCalls);
        Assert.Contains(result.Log, line => line.Contains("state changed", StringComparison.OrdinalIgnoreCase));
    }

    [Fact]
    public void ServiceAction_RejectsActionThatDoesNotMatchConfirmedState()
    {
        var runtime = new RecordingServiceRuntime(Service("Stopped"));
        var actions = new GuardedSystemActions(runtime);
        var target = new ServiceActionTarget("AcmeService", "Acme Service", ServiceObservedState.Stopped);

        var plan = actions.PlanServiceAction([target], ServiceActionKind.Restart);

        Assert.Empty(plan.Allowed);
        Assert.Contains("does not match", Assert.Single(plan.Blocked).Reason, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void ServiceCancellation_StopsBeforeRevalidationOrMutation()
    {
        var runtime = new RecordingServiceRuntime(Service("Running"));
        var actions = new GuardedSystemActions(runtime);
        using var cancellation = new CancellationTokenSource();
        cancellation.Cancel();

        Assert.Throws<OperationCanceledException>(() => actions.ActOnServices([
            new ServiceActionTarget("AcmeService", "Acme Service", ServiceObservedState.Running)
        ], ServiceActionKind.Restart, cancellation.Token));
        Assert.Equal(0, runtime.FindCalls);
        Assert.Equal(0, runtime.ActionCalls);
    }

    [Theory]
    [InlineData(@"\AdobeMalware", "Updater")]
    [InlineData(@"\GoogleEvil", "Updater")]
    [InlineData(@"\Microsoft\Office", "Office Automatic Updates Evil")]
    public void TaskCatalog_RequiresExactOfficeIdentities(string path, string name)
    {
        Assert.False(ScheduledTaskIdentityCatalog.TryResolve(path, name, out _));
    }

    [Fact]
    public void TaskCatalog_AllowsOnlyTheTwoOfficeUpdaterIdentities()
    {
        Assert.True(ScheduledTaskIdentityCatalog.TryResolve(@"\Microsoft\Office", "Office Automatic Updates", out var automatic));
        Assert.True(ScheduledTaskIdentityCatalog.TryResolve(@"\Microsoft\Office", "Office Feature Updates", out var feature));
        Assert.Equal(ScheduledTaskId.OfficeAutomaticUpdates, automatic.Id);
        Assert.Equal(ScheduledTaskId.OfficeFeatureUpdates, feature.Id);
    }

    [Fact]
    public void ServicePolicy_DefaultsUnknownAndWindowsServicesToReadOnly()
    {
        Assert.False(WindowsServiceMonitor.CanManageName("EventLog", out _));
        Assert.False(WindowsServiceMonitor.CanManageName("SiftDefinitelyMissingService", out _));
    }

    private static (Process Process, string ExecutablePath, string Directory) StartFixtureProcess()
    {
        var source = Environment.GetEnvironmentVariable("ComSpec")
            ?? throw new InvalidOperationException("ComSpec is unavailable.");
        var directory = Path.Combine(Path.GetTempPath(), "sift-process-fixture-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(directory);
        var executable = Path.Combine(directory, "sift-action-fixture.exe");
        File.Copy(source, executable);
        var process = Process.Start(new ProcessStartInfo(executable, "/d /c ping 127.0.0.1 -n 30 > nul")
        {
            UseShellExecute = false,
            CreateNoWindow = true,
            WorkingDirectory = directory
        }) ?? throw new InvalidOperationException("Could not start the process fixture.");
        process.Refresh();
        return (process, executable, directory);
    }

    private static ProcessActionTarget Target(Process process, string executablePath) => new(
        process.Id,
        process.ProcessName,
        executablePath,
        process.SessionId,
        process.StartTime.ToUniversalTime().Ticks);

    private static void DisposeFixture((Process Process, string ExecutablePath, string Directory) fixture)
    {
        using (fixture.Process)
        {
            if (!fixture.Process.HasExited)
            {
                fixture.Process.Kill(entireProcessTree: true);
                fixture.Process.WaitForExit();
            }
        }
        Directory.Delete(fixture.Directory, recursive: true);
    }

    private static ServiceInfo Service(string status) => new(
        "AcmeService", "Acme Service", status, "Manual", false, true, "Third-party");

    private sealed class RecordingServiceRuntime(ServiceInfo current) : IServiceActionRuntime
    {
        public ServiceInfo Current { get; set; } = current;
        public int FindCalls { get; private set; }
        public int ActionCalls { get; private set; }

        public ServiceInfo? FindExact(string name)
        {
            FindCalls++;
            return Current.Name.Equals(name, StringComparison.OrdinalIgnoreCase) ? Current : null;
        }

        public bool CanManageName(string name, out string reason)
        {
            reason = "test service";
            return true;
        }

        public string Act(string name, ServiceActionKind action, ServiceObservedState expectedState)
        {
            ActionCalls++;
            return action == ServiceActionKind.Start ? $"STARTED  {name}" : $"RESTARTED {name}";
        }
    }
}
