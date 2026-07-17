using Sift.Models;
using Sift.Services;
using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;

namespace Sift.UnitTests;

public sealed class OptimizeMutationWorkflowTests
{
    [Fact]
    public async Task First_confirmation_decline_prevents_restore_and_mutation()
    {
        var restore = new RecordingRestoreService();
        var workflow = new OptimizeMutationWorkflow(new FakeExecutor(), restore);
        var interaction = new RecordingInteraction(confirmFirst: false);
        var phases = new RecordingPhases();

        var result = await workflow.RunAsync(Selection(), true, interaction, phases,
            TestContext.Current.CancellationToken);

        Assert.False(result.Succeeded);
        Assert.True(result.Cancelled);
        Assert.False(result.MutationStarted);
        Assert.Equal(0, restore.ExecuteCount);
        Assert.Equal(0, phases.MachineCount + phases.LocalCount);
    }

    [Fact]
    public async Task Restore_failure_second_confirmation_close_prevents_mutation()
    {
        var restore = new RecordingRestoreService(failRestore: true);
        var workflow = new OptimizeMutationWorkflow(new FakeExecutor(), restore);
        var interaction = new RecordingInteraction(confirmFirst: true, confirmSecond: false);
        var phases = new RecordingPhases();

        var result = await workflow.RunAsync(Selection(), true, interaction, phases,
            TestContext.Current.CancellationToken);

        Assert.False(result.Succeeded);
        Assert.True(result.Cancelled);
        Assert.False(result.MutationStarted);
        Assert.Equal(1, restore.ExecuteCount);
        Assert.Equal(0, phases.MachineCount + phases.LocalCount);
        Assert.Equal("Continue without restore point?", interaction.SecondTitle);
    }

    [Fact]
    public async Task Machine_phase_rejection_prevents_local_phase()
    {
        var workflow = new OptimizeMutationWorkflow(new FakeExecutor(), new RecordingRestoreService());
        var interaction = new RecordingInteraction(confirmFirst: true);
        var phases = new RecordingPhases(machineSuccess: false);

        var result = await workflow.RunAsync(Selection(), true, interaction, phases,
            TestContext.Current.CancellationToken);

        Assert.False(result.Succeeded);
        Assert.True(result.MutationStarted);
        Assert.Equal(1, phases.MachineCount);
        Assert.Equal(0, phases.LocalCount);
    }

    [Fact]
    public async Task Successful_flow_runs_machine_then_local_phase()
    {
        var workflow = new OptimizeMutationWorkflow(new FakeExecutor(), new RecordingRestoreService());
        var interaction = new RecordingInteraction(confirmFirst: true);
        var phases = new RecordingPhases();

        var result = await workflow.RunAsync(Selection(), true, interaction, phases,
            TestContext.Current.CancellationToken);

        Assert.True(result.Succeeded);
        Assert.True(result.MutationStarted);
        Assert.Equal(1, phases.MachineCount);
        Assert.Equal(1, phases.LocalCount);
    }

    [Fact]
    public async Task Production_phases_execute_machine_and_local_once_under_outer_coordinator()
    {
        var selection = MixedSelection();
        var executor = new FakeExecutor();
        var broker = new RecordingElevationBroker();
        var workflow = new OptimizeMutationWorkflow(executor, new RecordingRestoreService());
        var phases = new OptimizeMutationPhases(executor, broker, selection, isElevated: () => false);
        using var coordinator = new OperationCoordinator(new ActivityHub());

        var outcome = await coordinator.RunLatestAsync("workspace.optimize.mutate", "Optimize", "test apply",
            token => workflow.RunAsync(selection, false, new RecordingInteraction(confirmFirst: true), phases, token),
            TestContext.Current.CancellationToken);

        Assert.True(outcome.Succeeded);
        Assert.NotNull(outcome.Value);
        Assert.True(outcome.Value.Succeeded);
        Assert.Single(broker.Batches);
        Assert.Single(broker.Batches[0]);
        Assert.True(ElevatedOperationPolicy.IsElevatedOptimizeTweak(broker.Batches[0][0]));
        Assert.Single(executor.MutationBatches);
        Assert.Single(executor.MutationBatches[0]);
        Assert.False(ElevatedOperationPolicy.IsElevatedOptimizeTweak(executor.MutationBatches[0][0]));
        Assert.False(broker.ObservedCancelledToken);
        Assert.False(executor.ObservedCancelledToken);
        Assert.Equal(2, phases.CombinedLog.Count);
        Assert.Single(phases.CombinedLog, line => line.StartsWith("MACHINE", StringComparison.Ordinal));
        Assert.Single(phases.CombinedLog, line => line.StartsWith("LOCAL", StringComparison.Ordinal));
    }

    [Fact]
    public async Task Production_phase_uac_cancellation_stops_before_local_mutation()
    {
        var selection = MixedSelection();
        var executor = new FakeExecutor();
        var broker = new RecordingElevationBroker(cancel: true);
        var workflow = new OptimizeMutationWorkflow(executor, new RecordingRestoreService());
        var phases = new OptimizeMutationPhases(executor, broker, selection, isElevated: () => false);
        using var coordinator = new OperationCoordinator(new ActivityHub());

        var outcome = await coordinator.RunLatestAsync("workspace.optimize.mutate", "Optimize", "test cancellation",
            token => workflow.RunAsync(selection, false, new RecordingInteraction(confirmFirst: true), phases, token),
            TestContext.Current.CancellationToken);

        Assert.True(outcome.Succeeded);
        Assert.NotNull(outcome.Value);
        Assert.True(outcome.Value.Cancelled);
        Assert.False(outcome.Value.MutationStarted);
        Assert.Single(broker.Batches);
        Assert.Empty(executor.MutationBatches);
    }

    [Fact]
    public async Task Review_lists_each_administrator_request_before_confirmation()
    {
        var interaction = new RecordingInteraction(confirmFirst: false);
        var workflow = new OptimizeMutationWorkflow(new FakeExecutor(), new RecordingRestoreService(),
            isElevated: () => false);

        var result = await workflow.RunAsync(MixedSelection(), true, interaction, new RecordingPhases(),
            TestContext.Current.CancellationToken);

        Assert.True(result.Cancelled);
        Assert.NotNull(interaction.Review);
        Assert.Equal(["create a Windows restore point", "apply machine-wide changes"],
            interaction.Review.AdministratorActions);
    }

    private static IReadOnlyList<Tweak> Selection() =>
        TweakCatalog.Create().Where(tweak => tweak.Minimal).Take(2).ToList();

    private static IReadOnlyList<Tweak> MixedSelection()
    {
        var catalog = TweakCatalog.Create();
        return
        [
            catalog.First(ElevatedOperationPolicy.IsElevatedOptimizeTweak),
            catalog.First(tweak => !ElevatedOperationPolicy.IsElevatedOptimizeTweak(tweak))
        ];
    }

    private sealed class FakeExecutor : ITweakExecutor
    {
        public List<IReadOnlyList<Tweak>> MutationBatches { get; } = [];
        public bool ObservedCancelledToken { get; private set; }
        public string BackupDirectory => Path.GetTempPath();
        public bool IsApplied(Tweak tweak) => false;
        public Task<ApplyResult> ApplyAsync(IEnumerable<Tweak> selection, bool dryRun, CancellationToken cancellationToken = default)
        {
            ObservedCancelledToken |= cancellationToken.IsCancellationRequested;
            var batch = selection.ToList();
            if (!dryRun) MutationBatches.Add(batch);
            return Task.FromResult(new ApplyResult
            {
                Previewed = dryRun ? batch.Count : 0,
                Succeeded = dryRun ? 0 : batch.Count,
                Failed = 0,
                Log = batch.Select(tweak => $"{(dryRun ? "PREFLIGHT" : "LOCAL")} {tweak.Id}").ToList()
            });
        }
        public Task<RestoreResult> RestoreFromAsync(string path, IReadOnlyDictionary<string, Tweak> catalog,
            RestoreScope scope = RestoreScope.All) => throw new NotSupportedException();
        public IReadOnlyList<BackupInfo> ListBackups() => [];
    }

    private sealed class RecordingElevationBroker(bool cancel = false) : IElevationBroker
    {
        public List<IReadOnlyList<Tweak>> Batches { get; } = [];
        public bool ObservedCancelledToken { get; private set; }

        public Task<ElevatedOperationResponse> ApplyMachineTweaksAsync(IEnumerable<Tweak> selection,
            CancellationToken cancellationToken = default)
        {
            ObservedCancelledToken |= cancellationToken.IsCancellationRequested;
            var batch = selection.ToList();
            Batches.Add(batch);
            return Task.FromResult(new ElevatedOperationResponse("request", "nonce", !cancel, cancel,
                cancel ? "Administrator permission was cancelled." : "Machine changes applied.",
                cancel ? 0 : batch.Count, 0,
                cancel ? [] : batch.Select(tweak => $"MACHINE {tweak.Id}").ToList()));
        }

        public Task<ElevatedOperationResponse> RestoreMachineBackupAsync(string backupPath,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> ManageServiceAsync(ServiceActionTarget target, ServiceActionKind action,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> ChangeScheduledTaskAsync(ScheduledTaskId id,
            ScheduledTaskChange change, bool expectedEnabled, string expectedDefinitionHash,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> CreateSystemRestorePointAsync(
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> RunCatalogRecipeAsync(
            string recipeId, string expectedRecipeHash, CancellationToken cancellationToken = default) =>
            throw new NotSupportedException();
    }

    private sealed class RecordingRestoreService(bool failRestore = false) : ISystemRestorePointService
    {
        public int ExecuteCount { get; private set; }

        public bool IsEligible(bool offerEnabled, IReadOnlyList<Tweak> confirmedSelection) => offerEnabled;

        public SystemRestorePointPreflight Preflight(bool offerEnabled, IReadOnlyList<Tweak> confirmedSelection) =>
            new(Guid.NewGuid(), DateTime.UtcNow.AddMinutes(5), "A".PadRight(64, 'A'), true,
                SystemProtectionState.Available, "fixture");

        public void Revoke(Guid ticketId) { }

        public Task<SystemRestorePointResult> ExecuteAsync(Guid ticketId, CancellationToken cancellationToken = default)
        {
            ExecuteCount++;
            return Task.FromResult(failRestore
                ? new SystemRestorePointResult(false, true, "restore cancelled", [])
                : new SystemRestorePointResult(true, false, "restore ok", []));
        }
    }

    private sealed class RecordingInteraction(bool confirmFirst, bool confirmSecond = true) : IOptimizeMutationInteraction
    {
        public string? SecondTitle { get; private set; }
        public OptimizeMutationReview? Review { get; private set; }

        public Task<bool> ConfirmReviewedBatchAsync(OptimizeMutationReview review, CancellationToken cancellationToken)
        {
            Review = review;
            return Task.FromResult(confirmFirst);
        }

        public Task<bool> ConfirmContinueWithoutRestorePointAsync(SystemRestorePointResult failure,
            CancellationToken cancellationToken)
        {
            SecondTitle = "Continue without restore point?";
            return Task.FromResult(confirmSecond);
        }
    }

    private sealed class RecordingPhases(bool machineSuccess = true, bool localSuccess = true) : IOptimizeMutationPhases
    {
        public int MachineCount { get; private set; }
        public int LocalCount { get; private set; }

        public Task<OptimizeMutationPhaseResult> ExecuteMachinePhaseAsync(CancellationToken cancellationToken)
        {
            MachineCount++;
            return Task.FromResult(machineSuccess
                ? new OptimizeMutationPhaseResult(OptimizeMutationPhaseStatus.Succeeded, true, "machine ok", [])
                : new OptimizeMutationPhaseResult(OptimizeMutationPhaseStatus.Failed, true, "machine failed", []));
        }

        public Task<OptimizeMutationPhaseResult> ExecuteLocalPhaseAsync(CancellationToken cancellationToken)
        {
            LocalCount++;
            return Task.FromResult(localSuccess
                ? new OptimizeMutationPhaseResult(OptimizeMutationPhaseStatus.Succeeded, true, "local ok", [])
                : new OptimizeMutationPhaseResult(OptimizeMutationPhaseStatus.Failed, true, "local failed", []));
        }
    }
}
