using Sift.Models;
using Sift.Services;
using Sift.WinUI.Views;

namespace Sift.WinUI.Composition;

public sealed record DashboardActionExecutionResult(
    bool Succeeded,
    bool Cancelled,
    string Summary,
    IReadOnlyList<string> Log);

public interface IDashboardActionInteraction : IOptimizeMutationInteraction
{
    Task<IReadOnlyList<MaintenanceFinding>?> SelectMaintenanceAsync(
        IReadOnlyList<MaintenanceFinding> findings,
        CancellationToken cancellationToken);
    Task<bool> ConfirmMaintenanceAsync(
        IReadOnlyList<MaintenanceFinding> selection,
        CleanResult review,
        CancellationToken cancellationToken);
    Task<bool> ConfirmProcessAsync(ProcessSnapshot process, bool restart, CancellationToken cancellationToken);
    Task<bool> ConfirmServiceAsync(
        DashboardServiceSnapshot service,
        ServiceActionKind action,
        bool requiresElevation,
        CancellationToken cancellationToken);
    void ReportActionProgress(string text);
}

public interface IDashboardActionRouter
{
    Task<DashboardActionExecutionResult> ExecuteAsync(
        DashboardWidgetActionIntent intent,
        bool offerRestorePoint,
        IDashboardActionInteraction interaction,
        CancellationToken cancellationToken = default);
}

/// <summary>
/// Routes only cataloged dashboard actions into the same production workflows used by the full
/// workspaces. It never accepts a command line, executable, path, URI, or elevation request.
/// </summary>
public sealed class DashboardActionRouter(
    ITweakExecutor tweaks,
    IElevationBroker elevation,
    IOptimizeMutationWorkflow optimize,
    IMaintenanceScanner maintenanceScanner,
    IMaintenanceCleaner maintenanceCleaner,
    IProcessSampler processes,
    IGuardedSystemActions guardedActions,
    IDashboardAlertEngine alerts) : IDashboardActionRouter
{
    public async Task<DashboardActionExecutionResult> ExecuteAsync(
        DashboardWidgetActionIntent intent,
        bool offerRestorePoint,
        IDashboardActionInteraction interaction,
        CancellationToken cancellationToken = default) => intent.Action switch
    {
        DashboardActionKind.OptimizePreset => await OptimizeAsync(offerRestorePoint, interaction, cancellationToken),
        DashboardActionKind.MaintenanceCleanup => await MaintenanceAsync(interaction, cancellationToken),
        DashboardActionKind.EndProcess => await ProcessAsync(intent.Process, restart: false, interaction, cancellationToken),
        DashboardActionKind.RestartProcess => await ProcessAsync(intent.Process, restart: true, interaction, cancellationToken),
        DashboardActionKind.StartService => await ServiceAsync(intent.Service, ServiceActionKind.Start, interaction, cancellationToken),
        DashboardActionKind.RestartService => await ServiceAsync(intent.Service, ServiceActionKind.Restart, interaction, cancellationToken),
        DashboardActionKind.AcknowledgeAlert => await AcknowledgeAsync(intent.AlertId, cancellationToken),
        DashboardActionKind.SnoozeAlert => await SnoozeAsync(intent.AlertId, cancellationToken),
        _ => new(false, false, "This dashboard action is not supported.", [])
    };

    private async Task<DashboardActionExecutionResult> OptimizeAsync(
        bool offerRestorePoint,
        IDashboardActionInteraction interaction,
        CancellationToken token)
    {
        var selection = TweakCatalog.Create().Where(tweak => tweak.Recommended).ToList();
        var phases = new OptimizeMutationPhases(tweaks, elevation, selection,
            progress: (phase, count) => interaction.ReportActionProgress(phase switch
            {
                OptimizeMutationPhaseProgress.RequestingAdministratorPermission =>
                    $"Waiting for Windows administrator permission for {count:N0} change(s)…",
                _ => $"Applying {count:N0} current-user change(s)…"
            }));
        var result = await optimize.RunAsync(selection, offerRestorePoint, interaction, phases, token);
        return new(result.Succeeded, result.Cancelled, result.Summary, phases.CombinedLog);
    }

    private async Task<DashboardActionExecutionResult> MaintenanceAsync(
        IDashboardActionInteraction interaction,
        CancellationToken token)
    {
        interaction.ReportActionProgress("Scanning supported maintenance locations…");
        var findings = await Task.Run(() => maintenanceScanner.Scan(), token);
        var selection = await interaction.SelectMaintenanceAsync(findings, token);
        if (selection is null || selection.Count == 0)
            return new(false, true, "Maintenance cleanup cancelled; nothing was changed.", []);

        interaction.ReportActionProgress("Checking the selected contents…");
        var review = await maintenanceCleaner.ReviewAsync(selection, token);
        var checkedSelection = review.Result;
        if (!review.CanExecute || checkedSelection.Previewed != selection.Count ||
            checkedSelection.Skipped > 0 || checkedSelection.Failed > 0)
        {
            if (!string.IsNullOrWhiteSpace(review.TicketId)) maintenanceCleaner.Discard(review.TicketId);
            return new(false, false,
                $"Could not check all selected items: checked {checkedSelection.Previewed:N0} of {selection.Count:N0}; " +
                $"skipped {checkedSelection.Skipped:N0}; failed {checkedSelection.Failed:N0}.", checkedSelection.Log);
        }
        if (!await interaction.ConfirmMaintenanceAsync(selection, checkedSelection, token))
        {
            maintenanceCleaner.Discard(review.TicketId!);
            return new(false, true, "Maintenance cleanup cancelled; nothing was changed.", checkedSelection.Log);
        }

        interaction.ReportActionProgress("Revalidating the reviewed contents…");
        var result = await maintenanceCleaner.ExecuteAsync(review.TicketId!, token);
        var succeeded = result.Status == MaintenanceCleanupStatus.Completed && result.Failed == 0;
        return new(succeeded, false, result.Summary, result.Log);
    }

    private async Task<DashboardActionExecutionResult> ProcessAsync(
        ProcessSnapshot? selected,
        bool restart,
        IDashboardActionInteraction interaction,
        CancellationToken token)
    {
        if (selected is null) return new(false, false, "Select one process first.", []);
        var current = await Task.Run(() => processes.Sample(token).Processes.FirstOrDefault(process =>
            process.Id == selected.Id && process.SessionId == selected.SessionId &&
            process.StartTimeUtcTicks == selected.StartTimeUtcTicks), token);
        if (current is null)
            return new(false, false, "The selected process ended or changed before review.", []);
        var target = new ProcessActionTarget(current.Id, current.Name, current.ExecutablePath,
            current.SessionId, current.StartTimeUtcTicks);
        var plan = restart ? guardedActions.PlanProcessRestart([target]) : guardedActions.PlanProcessEnd([target]);
        if (plan.Allowed.Count != 1)
            return new(false, false, plan.Blocked.FirstOrDefault().Reason ?? "The process action is unavailable.", []);
        if (!await interaction.ConfirmProcessAsync(selected, restart, token))
            return new(false, true, "Process action cancelled; nothing was changed.", []);
        var result = restart
            ? await guardedActions.RestartProcessesAsync([target], token)
            : await guardedActions.EndProcessesAsync([target], token);
        return Guarded(result, restart ? "Restarted process." : "Ended process.");
    }

    private async Task<DashboardActionExecutionResult> ServiceAsync(
        DashboardServiceSnapshot? service,
        ServiceActionKind action,
        IDashboardActionInteraction interaction,
        CancellationToken token)
    {
        if (service is null) return new(false, false, "Select one service first.", []);
        if (!Enum.TryParse<ServiceObservedState>(service.Status, true, out var expected))
            return new(false, false, $"Service action is unavailable while its state is {service.Status}.", []);
        var target = new ServiceActionTarget(service.Name, service.DisplayName, expected);
        var plan = guardedActions.PlanServiceAction([target], action);
        if (plan.Allowed.Count != 1)
            return new(false, false, plan.Blocked.FirstOrDefault().Reason ?? "The service action is unavailable.", []);
        var needsElevation = !ElevationHelper.IsElevated();
        if (!await interaction.ConfirmServiceAsync(service, action, needsElevation, token))
            return new(false, true, "Service action cancelled; nothing was changed.", []);
        if (needsElevation)
        {
            var elevated = await elevation.ManageServiceAsync(target, action, token);
            return new(elevated.Succeeded, elevated.Cancelled, elevated.Message, elevated.Log);
        }
        return Guarded(await Task.Run(() => guardedActions.ActOnServices([target], action, token), token),
            $"{action} completed for {service.DisplayName}.");
    }

    private async Task<DashboardActionExecutionResult> AcknowledgeAsync(string? alertId, CancellationToken token)
    {
        if (string.IsNullOrWhiteSpace(alertId)) return new(false, false, "Select one alert first.", []);
        await alerts.AcknowledgeAsync(alertId, DateTimeOffset.UtcNow, token);
        return new(true, false, "Alert acknowledged.", []);
    }

    private async Task<DashboardActionExecutionResult> SnoozeAsync(string? alertId, CancellationToken token)
    {
        if (string.IsNullOrWhiteSpace(alertId)) return new(false, false, "Select one alert first.", []);
        await alerts.SnoozeAsync(alertId, DateTimeOffset.UtcNow.AddHours(1), token);
        return new(true, false, "Alert snoozed for one hour.", []);
    }

    private static DashboardActionExecutionResult Guarded(GuardedActionResult result, string success) =>
        new(result.Succeeded == 1 && result.Failed == 0 && result.Skipped == 0, false,
            result.Succeeded == 1 && result.Failed == 0 && result.Skipped == 0
                ? success
                : $"Action completed with {result.Succeeded:N0} succeeded, {result.Skipped:N0} blocked/skipped, and {result.Failed:N0} failed.",
            result.Log);
}
