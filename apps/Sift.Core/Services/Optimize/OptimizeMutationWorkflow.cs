using Sift.Models;

namespace Sift.Services;

public sealed record OptimizeMutationReview(
    ApplyResult TweakPreflight,
    SystemRestorePointPreflight? RestorePointPreflight,
    IReadOnlyList<string> AdministratorActions);

public sealed record OptimizeMutationWorkflowResult(
    bool Succeeded,
    bool Cancelled,
    bool MutationStarted,
    string Summary);

public enum OptimizeMutationPhaseStatus
{
    Succeeded,
    Cancelled,
    Failed
}

public enum OptimizeMutationPhaseProgress
{
    RequestingAdministratorPermission,
    ApplyingCurrentUserChanges
}

public sealed record OptimizeMutationPhaseResult(
    OptimizeMutationPhaseStatus Status,
    bool MutationStarted,
    string Summary,
    IReadOnlyList<string> Log);

public interface IOptimizeMutationInteraction
{
    Task<bool> ConfirmReviewedBatchAsync(
        OptimizeMutationReview review,
        CancellationToken cancellationToken);
    Task<bool> ConfirmContinueWithoutRestorePointAsync(
        SystemRestorePointResult failure,
        CancellationToken cancellationToken);
}

public interface IOptimizeMutationPhases
{
    Task<OptimizeMutationPhaseResult> ExecuteMachinePhaseAsync(CancellationToken cancellationToken);
    Task<OptimizeMutationPhaseResult> ExecuteLocalPhaseAsync(CancellationToken cancellationToken);
}

public interface IOptimizeMutationWorkflow
{
    Task<OptimizeMutationWorkflowResult> RunAsync(
        IReadOnlyList<Tweak> selection,
        bool offerSystemRestorePoint,
        IOptimizeMutationInteraction interaction,
        IOptimizeMutationPhases phases,
        CancellationToken cancellationToken = default);
}

public sealed class OptimizeMutationWorkflow(
    ITweakExecutor executor,
    ISystemRestorePointService restorePoints,
    Func<bool>? isElevated = null) : IOptimizeMutationWorkflow
{
    private readonly Func<bool> _isElevated = isElevated ?? ElevationHelper.IsElevated;

    public async Task<OptimizeMutationWorkflowResult> RunAsync(
        IReadOnlyList<Tweak> selection,
        bool offerSystemRestorePoint,
        IOptimizeMutationInteraction interaction,
        IOptimizeMutationPhases phases,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var preflight = await executor.ApplyAsync(selection, dryRun: true, cancellationToken);
        if (preflight.Previewed != selection.Count || preflight.Failed > 0)
        {
            return new OptimizeMutationWorkflowResult(false, false, false,
                $"Could not check all selected changes: checked {preflight.Previewed:N0} of {selection.Count:N0}; {preflight.Failed:N0} failed.");
        }

        SystemRestorePointPreflight? restorePreflight = null;
        if (restorePoints.IsEligible(offerSystemRestorePoint, selection))
            restorePreflight = restorePoints.Preflight(offerSystemRestorePoint, selection);

        var administratorActions = new List<string>();
        if (!_isElevated())
        {
            if (restorePreflight is not null)
                administratorActions.Add("create a Windows restore point");
            if (selection.Any(ElevatedOperationPolicy.IsElevatedOptimizeTweak))
                administratorActions.Add("apply machine-wide changes");
        }

        if (!await interaction.ConfirmReviewedBatchAsync(
                new OptimizeMutationReview(preflight, restorePreflight, administratorActions), cancellationToken))
        {
            if (restorePreflight is not null) restorePoints.Revoke(restorePreflight.TicketId);
            return new OptimizeMutationWorkflowResult(false, true, false,
                "Optimize changes cancelled; nothing was changed.");
        }

        if (restorePreflight is not null)
        {
            var restore = await restorePoints.ExecuteAsync(restorePreflight.TicketId, cancellationToken);
            if (!restore.Succeeded)
            {
                if (!await interaction.ConfirmContinueWithoutRestorePointAsync(restore, cancellationToken))
                {
                    return new OptimizeMutationWorkflowResult(false, true, false,
                        "Optimize changes cancelled; nothing was changed.");
                }
            }
        }

        var machine = await phases.ExecuteMachinePhaseAsync(cancellationToken);
        if (machine.Status == OptimizeMutationPhaseStatus.Cancelled)
            return new OptimizeMutationWorkflowResult(false, true, machine.MutationStarted, machine.Summary);
        if (machine.Status == OptimizeMutationPhaseStatus.Failed)
            return new OptimizeMutationWorkflowResult(false, false, machine.MutationStarted, machine.Summary);

        var local = await phases.ExecuteLocalPhaseAsync(cancellationToken);
        if (local.Status == OptimizeMutationPhaseStatus.Cancelled)
            return new OptimizeMutationWorkflowResult(false, true,
                machine.MutationStarted || local.MutationStarted, local.Summary);
        if (local.Status == OptimizeMutationPhaseStatus.Failed)
            return new OptimizeMutationWorkflowResult(false, false,
                machine.MutationStarted || local.MutationStarted, local.Summary);

        return new OptimizeMutationWorkflowResult(true, false,
            machine.MutationStarted || local.MutationStarted, "Optimize changes completed.");
    }
}

public sealed class OptimizeMutationPhases : IOptimizeMutationPhases
{
    private readonly ITweakExecutor _executor;
    private readonly IElevationBroker _elevation;
    private readonly IReadOnlyList<Tweak> _machine;
    private readonly IReadOnlyList<Tweak> _local;
    private readonly Action<OptimizeMutationPhaseProgress, int>? _progress;
    private readonly List<string> _combinedLog = [];

    public OptimizeMutationPhases(
        ITweakExecutor executor,
        IElevationBroker elevation,
        IReadOnlyList<Tweak> selection,
        Func<bool>? isElevated = null,
        Action<OptimizeMutationPhaseProgress, int>? progress = null)
    {
        _executor = executor;
        _elevation = elevation;
        _progress = progress;
        var machine = !(isElevated ?? ElevationHelper.IsElevated)()
            ? selection.Where(ElevatedOperationPolicy.IsElevatedOptimizeTweak).ToList()
            : [];
        _machine = machine;
        _local = machine.Count == 0 ? selection.ToList() : selection.Except(machine).ToList();
    }

    public IReadOnlyList<string> CombinedLog => _combinedLog;

    public async Task<OptimizeMutationPhaseResult> ExecuteMachinePhaseAsync(CancellationToken cancellationToken)
    {
        if (_machine.Count == 0)
            return Success(false, "No administrator changes selected.", []);

        cancellationToken.ThrowIfCancellationRequested();
        _progress?.Invoke(OptimizeMutationPhaseProgress.RequestingAdministratorPermission, _machine.Count);
        var response = await _elevation.ApplyMachineTweaksAsync(_machine, cancellationToken);
        _combinedLog.AddRange(response.Log);

        if (response.Cancelled)
            return new OptimizeMutationPhaseResult(OptimizeMutationPhaseStatus.Cancelled,
                response.Applied > 0, response.Message, response.Log);
        if (!response.Succeeded || response.Failed > 0)
            return new OptimizeMutationPhaseResult(OptimizeMutationPhaseStatus.Failed,
                response.Applied > 0, response.Message, response.Log);
        return Success(response.Applied > 0, response.Message, response.Log);
    }

    public async Task<OptimizeMutationPhaseResult> ExecuteLocalPhaseAsync(CancellationToken cancellationToken)
    {
        if (_local.Count == 0)
            return Success(false, "No current-user changes selected.", []);

        cancellationToken.ThrowIfCancellationRequested();
        _progress?.Invoke(OptimizeMutationPhaseProgress.ApplyingCurrentUserChanges, _local.Count);
        var result = await _executor.ApplyAsync(_local, dryRun: false, cancellationToken);
        _combinedLog.AddRange(result.Log);

        var started = result.Succeeded > 0 || result.Failed > 0;
        if (result.Failed > 0)
            return new OptimizeMutationPhaseResult(OptimizeMutationPhaseStatus.Failed, started,
                $"Applied {result.Succeeded:N0} current-user change(s); {result.Failed:N0} failed.", result.Log);
        return Success(started, $"Applied {result.Succeeded:N0} current-user change(s).", result.Log);
    }

    private static OptimizeMutationPhaseResult Success(bool mutationStarted, string summary,
        IReadOnlyList<string> log) =>
        new(OptimizeMutationPhaseStatus.Succeeded, mutationStarted, summary, log);
}
