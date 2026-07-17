using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Models;
using Sift.Presentation;
using Sift.Services;
using Sift.WinUI.Infrastructure.Localization;
using Sift.WinUI.Models;
using Sift.WinUI.Views;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public sealed class TaskManagerWorkspaceModule : IWorkspaceModule
{
    private readonly IProcessSampler _processes;
    private readonly IServiceInventory _services;
    private readonly IScheduledTaskInventory _tasks;
    private readonly IGuardedSystemActions _actions;
    private readonly IScheduledTaskActionWorkflow _scheduledTaskWorkflow;
    private readonly IElevationBroker _elevation;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly TaskManagerWorkspaceView _view = new();
    private const string RefreshOperation = "workspace.taskmanager.refresh";
    private const string ActionOperation = "workspace.taskmanager.action";

    public TaskManagerWorkspaceModule(IProcessSampler processes, IServiceInventory services,
        IScheduledTaskInventory tasks, IGuardedSystemActions actions, IScheduledTaskActionWorkflow scheduledTaskWorkflow,
        IElevationBroker elevation, OperationCoordinator operations, ActivityHub activity)
    {
        _processes = processes;
        _services = services;
        _tasks = tasks;
        _actions = actions;
        _scheduledTaskWorkflow = scheduledTaskWorkflow;
        _elevation = elevation;
        _operations = operations;
        _activity = activity;
        _view.RefreshRequested += View_RefreshRequested;
        _view.SelectionChanged += View_SelectionChanged;
        _view.EndProcessRequested += View_EndProcessRequested;
        _view.RestartProcessRequested += View_RestartProcessRequested;
        _view.StartServiceRequested += View_StartServiceRequested;
        _view.RestartServiceRequested += View_RestartServiceRequested;
        _view.EnableTaskRequested += View_EnableTaskRequested;
        _view.DisableTaskRequested += View_DisableTaskRequested;
    }

    public string Key => "TaskManager";
    public string Title => "Task Manager";
    public Control View => _view;
    public Task ActivateAsync(CancellationToken cancellationToken = default) => RefreshAsync(cancellationToken);
    public void FocusPrimarySearch() => _view.FocusSearch();

    public async Task RefreshAsync(CancellationToken cancellationToken = default)
    {
        _view.SetBusy(true, "Reading process, service, and task inventories…");
        var outcome = await _operations.RunLatestAsync(RefreshOperation, Title, "system inventory refresh",
            token => Task.Run(() =>
            {
                var processes = _processes.Sample(token);
                token.ThrowIfCancellationRequested();
                var services = _services.Enumerate();
                token.ThrowIfCancellationRequested();
                var tasks = _tasks.Enumerate();
                return new TaskManagerInventory(processes, services, tasks);
            }, token), cancellationToken);
        if (outcome.Cancelled) return;
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Refresh failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }
        _view.Bind(outcome.Value);
        _view.SetBusy(false, $"{outcome.Value.System.Processes.Count:N0} processes · {outcome.Value.Services.Count:N0} services · {outcome.Value.Tasks.Count:N0} tasks");
        UpdateActionAvailability();
    }

    public void Deactivate()
    {
        _operations.Cancel(RefreshOperation);
        _operations.Cancel(ActionOperation);
    }

    public void Dispose()
    {
        Deactivate();
        _view.RefreshRequested -= View_RefreshRequested;
        _view.SelectionChanged -= View_SelectionChanged;
        _view.EndProcessRequested -= View_EndProcessRequested;
        _view.RestartProcessRequested -= View_RestartProcessRequested;
        _view.StartServiceRequested -= View_StartServiceRequested;
        _view.RestartServiceRequested -= View_RestartServiceRequested;
        _view.EnableTaskRequested -= View_EnableTaskRequested;
        _view.DisableTaskRequested -= View_DisableTaskRequested;
    }

    private async void View_RefreshRequested(object? sender, EventArgs e) => await RefreshAsync();
    private void View_SelectionChanged(object? sender, EventArgs e) => UpdateActionAvailability();
    private async void View_EndProcessRequested(object? sender, EventArgs e) => await ActOnProcessAsync(restart: false);
    private async void View_RestartProcessRequested(object? sender, EventArgs e) => await ActOnProcessAsync(restart: true);
    private async void View_StartServiceRequested(object? sender, EventArgs e) =>
        await ActOnServiceAsync(ServiceActionKind.Start);
    private async void View_RestartServiceRequested(object? sender, EventArgs e) =>
        await ActOnServiceAsync(ServiceActionKind.Restart);
    private async void View_EnableTaskRequested(object? sender, EventArgs e) =>
        await ActOnScheduledTaskAsync(ScheduledTaskChange.Enable);
    private async void View_DisableTaskRequested(object? sender, EventArgs e) =>
        await ActOnScheduledTaskAsync(ScheduledTaskChange.Disable);

    private void UpdateActionAvailability()
    {
        if (_view.SelectedProcess is { } process)
        {
            var target = Target(process);
            var end = _actions.PlanProcessEnd([target]);
            var restart = _actions.PlanProcessRestart([target]);
            _view.SetProcessActionAvailability(end.Allowed.Count == 1,
                end.Allowed.Count == 1
                    ? "Review the selected process before ending it."
                    : $"End task unavailable: {FormatBlocked(end.Blocked.FirstOrDefault().Reason)}",
                restart.Allowed.Count == 1,
                restart.Allowed.Count == 1
                    ? "Review the selected process before restarting it. Arguments and unsaved state are not restored."
                    : $"Restart unavailable: {FormatBlocked(restart.Blocked.FirstOrDefault().Reason)}");
        }
        else _view.SetProcessActionAvailability(false, "Select one process first.", false, "Select one process first.");

        if (_view.SelectedService is { } service)
        {
            if (Enum.TryParse<ServiceObservedState>(service.Status, ignoreCase: true, out var expectedState))
            {
                var target = new ServiceActionTarget(service.Name, service.DisplayName, expectedState);
                var start = _actions.PlanServiceAction([target], ServiceActionKind.Start);
                var restart = _actions.PlanServiceAction([target], ServiceActionKind.Restart);
                _view.SetServiceActionAvailability(start.Allowed.Count == 1,
                    start.Allowed.Count == 1
                        ? "Review the stopped service before starting it."
                        : $"Start unavailable: {FormatBlocked(start.Blocked.FirstOrDefault().Reason)}",
                    restart.Allowed.Count == 1,
                    restart.Allowed.Count == 1
                        ? "Review the running service before restarting it."
                        : $"Restart unavailable: {FormatBlocked(restart.Blocked.FirstOrDefault().Reason)}");
            }
            else
            {
                _view.SetServiceActionAvailability(false,
                    $"Start unavailable while service state is {service.Status}.", false,
                    $"Restart unavailable while service state is {service.Status}.");
            }
        }
        else _view.SetServiceActionAvailability(false, "Select one service first.", false, "Select one service first.");

        if (_view.SelectedTask is { } task)
        {
            var supported = task.ActionId.HasValue;
            var disabled = task.State.Contains("Disabled", StringComparison.OrdinalIgnoreCase);
            var reason = supported
                ? "Review the selected scheduled task before changing it."
                : "This scheduled task is view-only.";
            _view.SetTaskActionAvailability(supported && disabled,
                supported && disabled ? reason : supported ? $"Enable unavailable while task state is {task.State}." : $"Enable unavailable: {reason}",
                supported && !disabled,
                supported && !disabled ? reason : supported ? $"Disable unavailable while task state is {task.State}." : $"Disable unavailable: {reason}");
        }
        else _view.SetTaskActionAvailability(false, "Select one scheduled task first.", false, "Select one scheduled task first.");
    }

    private async Task ActOnProcessAsync(bool restart)
    {
        var process = _view.SelectedProcess;
        if (process is null) return;
        var target = Target(process);
        var plan = restart ? _actions.PlanProcessRestart([target]) : _actions.PlanProcessEnd([target]);
        if (plan.Allowed.Count != 1)
        {
            var reason = FormatBlocked(plan.Blocked.FirstOrDefault().Reason);
            _view.SetStatus($"{(restart ? "Restart" : "End task")} blocked: {reason}.");
            _activity.Warning(Title, "Process action blocked", $"{process.Name} ({process.Id}) · {reason}");
            UpdateActionAvailability();
            return;
        }
        if (!await _view.ConfirmProcessActionAsync(process, restart))
        {
            _view.SetStatus("Process action cancelled; nothing was changed.");
            _activity.Info(Title, "Process action cancelled", $"{process.Name} ({process.Id})");
            return;
        }

        var action = restart ? "process restart" : "process end";
        _view.SetBusy(true, $"Checking the selected process and starting {action}…");
        var outcome = await _operations.RunCommittedAsync(ActionOperation, Title, action, async token =>
        {
            token.ThrowIfCancellationRequested();
            var result = restart
                ? await _actions.RestartProcessesAsync([target], token)
                : await _actions.EndProcessesAsync([target], token);
            return MutationResult.From(result, restart ? "Restarted" : "Ended", process.Name);
        });
        await CompleteMutationAsync(outcome, refreshOnSuccess: true);
    }

    private async Task ActOnServiceAsync(ServiceActionKind action)
    {
        var service = _view.SelectedService;
        if (service is null) return;
        if (!Enum.TryParse<ServiceObservedState>(service.Status, ignoreCase: true, out var expectedState))
        {
            _view.SetStatus($"{action} unavailable while service state is {service.Status}.");
            UpdateActionAvailability();
            return;
        }
        var target = new ServiceActionTarget(service.Name, service.DisplayName, expectedState);
        var plan = _actions.PlanServiceAction([target], action);
        if (plan.Allowed.Count != 1)
        {
            var reason = FormatBlocked(plan.Blocked.FirstOrDefault().Reason,
                ReasonMessages.Format(SiftReasonCode.ServiceStateMismatch, service.Status));
            _view.SetStatus($"{action} blocked: {reason}.");
            _activity.Warning(Title, "Service action blocked", $"{service.DisplayName} · {reason}");
            UpdateActionAvailability();
            return;
        }
        var needsElevation = !ElevationHelper.IsElevated();
        var actionLabel = action.ToString();
        if (!await _view.ConfirmServiceActionAsync(service, actionLabel, needsElevation))
        {
            _view.SetStatus("Service action cancelled; nothing was changed.");
            _activity.Info(Title, "Service action cancelled", service.DisplayName);
            return;
        }

        _view.SetBusy(true, $"Checking {service.DisplayName} and starting {actionLabel.ToLowerInvariant()}…");
        var outcome = await _operations.RunCommittedAsync(ActionOperation, Title, $"service {actionLabel.ToLowerInvariant()}",
            async token =>
            {
                token.ThrowIfCancellationRequested();
                if (!needsElevation)
                {
                    var result = await Task.Run(() => _actions.ActOnServices([target], action, token), token);
                    return MutationResult.From(result, $"{actionLabel} completed for", service.DisplayName);
                }
                var elevated = await _elevation.ManageServiceAsync(target, action, token);
                return new MutationResult(elevated.Succeeded, elevated.Cancelled, elevated.Message, elevated.Log);
            });
        await CompleteMutationAsync(outcome, refreshOnSuccess: true);
    }

    private async Task ActOnScheduledTaskAsync(ScheduledTaskChange change)
    {
        var task = _view.SelectedTask;
        if (task?.ActionId is not { } taskId)
        {
            _view.SetStatus("This scheduled task is view-only.");
            return;
        }

        _view.SetBusy(true, "Checking the selected scheduled task…");
        var needsElevation = !ElevationHelper.IsElevated();
        var outcome = await _operations.RunCommittedAsync(ActionOperation, Title, $"scheduled task {change.ToString().ToLowerInvariant()}",
            async token =>
            {
                token.ThrowIfCancellationRequested();
                try
                {
                    var confirmation = new ViewScheduledTaskConfirmation(_view, _activity, Title, needsElevation);
                    var result = await _scheduledTaskWorkflow.RunAsync(taskId, change, confirmation, token);
                    return new MutationResult(result.Succeeded, result.Cancelled, result.Summary, result.Log);
                }
                catch (InvalidOperationException exception)
                {
                    return new MutationResult(false, false, exception.Message, [exception.Message]);
                }
            });
        await CompleteMutationAsync(outcome, refreshOnSuccess: true);
    }

    private sealed class ViewScheduledTaskConfirmation(
        TaskManagerWorkspaceView view,
        ActivityHub activity,
        string title,
        bool requiresElevation) : IScheduledTaskConfirmation
    {
        public async Task<bool> ConfirmAsync(ScheduledTaskActionPreflight preflight, CancellationToken cancellationToken)
        {
            cancellationToken.ThrowIfCancellationRequested();
            activity.Publish(ActivityEvent.Create(title, preflight.Evidence, ActivitySeverity.Info));
            return await view.ConfirmScheduledTaskActionAsync(preflight, requiresElevation);
        }
    }

    private async Task CompleteMutationAsync(OperationOutcome<MutationResult> outcome, bool refreshOnSuccess)
    {
        if (outcome.Cancelled)
        {
            _view.SetBusy(false, "Action cancelled; no completion was reported.");
            return;
        }
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Action failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }
        var result = outcome.Value;
        foreach (var line in result.Log)
            _activity.Publish(ActivityEvent.Create(Title, line,
                result.Succeeded ? ActivitySeverity.Info : ActivitySeverity.Warning));
        if (result.Cancelled)
        {
            _view.SetBusy(false, result.Summary);
            _activity.Info(Title, "Administrator confirmation cancelled", result.Summary);
            return;
        }
        if (!result.Succeeded)
        {
            _view.SetBusy(false, result.Summary);
            _activity.Warning(Title, "Task Manager action failed", result.Summary, persist: true);
            UpdateActionAvailability();
            return;
        }
        _activity.Info(Title, "Task Manager action completed", result.Summary, persist: true);
        if (refreshOnSuccess) await RefreshAsync();
        else _view.SetBusy(false, result.Summary);
    }

    private static ProcessActionTarget Target(TaskProcessRow process) =>
        new(process.Id, process.Name, process.ExecutablePath, process.SessionId, process.StartTimeUtcTicks);

    private static string FormatBlocked(string? reason, string? fallback = null)
    {
        var text = string.IsNullOrWhiteSpace(reason) ? fallback : reason;
        var code = text switch
        {
            _ when text is not null &&
                   text.Equals(ReasonMessages.Format(SiftReasonCode.ProcessProtected), StringComparison.Ordinal) =>
                SiftReasonCode.ProcessProtected,
            _ when text is not null &&
                   text.Equals(ReasonMessages.Format(SiftReasonCode.ProcessElevatedRestartDisabled), StringComparison.Ordinal) =>
                SiftReasonCode.ProcessElevatedRestartDisabled,
            _ when text is not null &&
                   text.Equals(ReasonMessages.Format(SiftReasonCode.ProcessSessionMismatch), StringComparison.Ordinal) =>
                SiftReasonCode.ProcessSessionMismatch,
            _ when text is not null &&
                   text.Equals(ReasonMessages.Format(SiftReasonCode.ProcessPathUnreadable), StringComparison.Ordinal) =>
                SiftReasonCode.ProcessPathUnreadable,
            _ when text is not null &&
                   text.Equals(ReasonMessages.Format(SiftReasonCode.ServiceActionStateMismatch), StringComparison.Ordinal) =>
                SiftReasonCode.ServiceActionStateMismatch,
            _ => SiftReasonCode.Unspecified
        };
        return ReasonPresenter.PresentOrFallback(code, text ?? ReasonMessages.Format(SiftReasonCode.TargetUnavailable));
    }

    private sealed record MutationResult(bool Succeeded, bool Cancelled, string Summary, IReadOnlyList<string> Log)
    {
        public static MutationResult From(GuardedActionResult result, string verb, string target) =>
            new(result.Succeeded == 1 && result.Failed == 0 && result.Skipped == 0, false,
                result.Succeeded == 1
                    ? $"{verb} {target}."
                    : $"Action completed with {result.Succeeded:N0} succeeded, {result.Skipped:N0} blocked/skipped, and {result.Failed:N0} failed.",
                result.Log);
    }
}
