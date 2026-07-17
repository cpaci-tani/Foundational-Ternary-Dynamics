using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class ScheduledTaskActionServiceTests
{
    private const string Hash = "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789";

    [Fact]
    public void Preflight_is_non_mutating_and_records_expected_state_and_identity()
    {
        var controller = new RecordingScheduledTaskController
        {
            Identity = new ScheduledTaskIdentity(ScheduledTaskId.OfficeAutomaticUpdates, "Office Automatic Updates",
                false, "Disabled", Hash)
        };
        var service = CreateService(controller);

        var preflight = service.Preflight(ScheduledTaskId.OfficeAutomaticUpdates, ScheduledTaskChange.Enable);

        Assert.Equal(0, controller.SetCount);
        Assert.Equal(1, controller.InspectCount);
        Assert.Equal(ScheduledTaskId.OfficeAutomaticUpdates, preflight.Id);
        Assert.Equal(ScheduledTaskChange.Enable, preflight.Change);
        Assert.False(preflight.ExpectedEnabled);
        Assert.Equal(Hash, preflight.ExpectedDefinitionHash);
        Assert.Contains("No change was made during preflight.", preflight.Evidence);
    }

    [Fact]
    public async Task Execute_rejects_changed_expected_state_without_mutation()
    {
        var controller = new RecordingScheduledTaskController
        {
            Identity = new ScheduledTaskIdentity(ScheduledTaskId.OfficeFeatureUpdates, "Office Feature Updates", true,
                "Ready", Hash)
        };
        var service = CreateService(controller);
        var preflight = service.Preflight(ScheduledTaskId.OfficeFeatureUpdates, ScheduledTaskChange.Disable);
        controller.Identity = controller.Identity with { Enabled = false };

        var result = await service.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);

        Assert.False(result.Succeeded);
        Assert.Equal(0, controller.SetCount);
        Assert.Contains("changed after confirmation", result.Summary, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public async Task Execute_rejects_changed_definition_hash_without_mutation()
    {
        var controller = new RecordingScheduledTaskController
        {
            Identity = new ScheduledTaskIdentity(ScheduledTaskId.OfficeAutomaticUpdates, "Office Automatic Updates",
                false, "Disabled", Hash)
        };
        var service = CreateService(controller);
        var preflight = service.Preflight(ScheduledTaskId.OfficeAutomaticUpdates, ScheduledTaskChange.Enable);
        controller.Identity = controller.Identity with
        {
            DefinitionHash = "1111111111111111111111111111111111111111111111111111111111111111"
        };

        var result = await service.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);

        Assert.False(result.Succeeded);
        Assert.Equal(0, controller.SetCount);
    }

    [Fact]
    public async Task Ticket_is_single_use_and_expired_tickets_do_not_mutate()
    {
        var controller = new RecordingScheduledTaskController
        {
            Identity = new ScheduledTaskIdentity(ScheduledTaskId.OfficeAutomaticUpdates, "Office Automatic Updates",
                false, "Disabled", Hash)
        };
        var time = new FakeTimeProvider(DateTimeOffset.UtcNow);
        var service = new ScheduledTaskActionService(controller, new RecordingElevationBroker(), () => true, time,
            TimeSpan.FromMinutes(5));
        var preflight = service.Preflight(ScheduledTaskId.OfficeAutomaticUpdates, ScheduledTaskChange.Enable);
        await service.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);
        var reused = await service.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);

        Assert.False(reused.Succeeded);
        Assert.Equal(1, controller.SetCount);
    }

    [Fact]
    public void Policy_rejects_windows_update_defender_unknown_and_multiple_targets()
    {
        Assert.False(ScheduledTaskIdentityCatalog.TryResolve(@"\Microsoft\Windows\WindowsUpdate", "Scheduled Start",
            out _));
        Assert.False(ScheduledTaskIdentityCatalog.TryResolve(@"\Microsoft\Windows", "Defender Update", out _));
        Assert.False(ScheduledTaskIdentityCatalog.TryResolve(@"\Adobe", "Updater", out _));
        Assert.Throws<InvalidOperationException>(() =>
            new ScheduledTaskActionService(new RecordingScheduledTaskController(), new RecordingElevationBroker(),
                () => true).Preflight((ScheduledTaskId)99, ScheduledTaskChange.Enable));
    }

    [Fact]
    public async Task Standard_user_execution_sends_only_typed_identity_to_broker()
    {
        var controller = new RecordingScheduledTaskController
        {
            Identity = new ScheduledTaskIdentity(ScheduledTaskId.OfficeAutomaticUpdates, "Office Automatic Updates",
                false, "Disabled", Hash)
        };
        var broker = new RecordingElevationBroker();
        var service = CreateService(controller, broker, elevated: false);
        var preflight = service.Preflight(ScheduledTaskId.OfficeAutomaticUpdates, ScheduledTaskChange.Enable);

        await service.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);

        Assert.Equal(0, controller.SetCount);
        Assert.Equal(ScheduledTaskId.OfficeAutomaticUpdates, broker.LastTaskId);
        Assert.Equal(ScheduledTaskChange.Enable, broker.LastChange);
        Assert.False(broker.LastExpectedEnabled);
        Assert.Equal(Hash, broker.LastHash);
    }

    [Fact]
    public async Task Confirmation_decline_revokes_ticket_and_never_executes()
    {
        var controller = new RecordingScheduledTaskController
        {
            Identity = new ScheduledTaskIdentity(ScheduledTaskId.OfficeAutomaticUpdates, "Office Automatic Updates",
                false, "Disabled", Hash)
        };
        var workflow = new ScheduledTaskActionWorkflow(CreateService(controller));
        var result = await workflow.RunAsync(ScheduledTaskId.OfficeAutomaticUpdates, ScheduledTaskChange.Enable,
            new DecliningConfirmation(), TestContext.Current.CancellationToken);

        Assert.True(result.Cancelled);
        Assert.Equal(0, controller.SetCount);
    }

    [Fact]
    public async Task Confirmation_close_revokes_ticket_and_never_executes()
    {
        var controller = new RecordingScheduledTaskController
        {
            Identity = new ScheduledTaskIdentity(ScheduledTaskId.OfficeFeatureUpdates, "Office Feature Updates", true,
                "Ready", Hash)
        };
        var workflow = new ScheduledTaskActionWorkflow(CreateService(controller));
        var result = await workflow.RunAsync(ScheduledTaskId.OfficeFeatureUpdates, ScheduledTaskChange.Disable,
            new DecliningConfirmation(), TestContext.Current.CancellationToken);

        Assert.True(result.Cancelled);
        Assert.Equal(0, controller.SetCount);
    }

    private static ScheduledTaskActionService CreateService(
        RecordingScheduledTaskController controller,
        RecordingElevationBroker? broker = null,
        bool elevated = true) =>
        new(controller, broker ?? new RecordingElevationBroker(), () => elevated);

    private sealed class RecordingScheduledTaskController : IScheduledTaskController
    {
        public ScheduledTaskIdentity? Identity { get; set; }
        public int InspectCount { get; private set; }
        public int SetCount { get; private set; }

        public ScheduledTaskIdentity? Inspect(ScheduledTaskId id)
        {
            InspectCount++;
            return Identity?.Id == id ? Identity : null;
        }

        public ScheduledTaskActionResult SetEnabled(
            ScheduledTaskId id,
            ScheduledTaskChange change,
            bool expectedEnabled,
            string expectedDefinitionHash)
        {
            SetCount++;
            return new ScheduledTaskActionResult(true, false, "Changed.", ["Changed."]);
        }
    }

    private sealed class RecordingElevationBroker : IElevationBroker
    {
        public ScheduledTaskId? LastTaskId { get; private set; }
        public ScheduledTaskChange? LastChange { get; private set; }
        public bool? LastExpectedEnabled { get; private set; }
        public string? LastHash { get; private set; }

        public Task<ElevatedOperationResponse> ApplyMachineTweaksAsync(IEnumerable<Tweak> selection,
            CancellationToken cancellationToken = default) =>
            throw new NotSupportedException();

        public Task<ElevatedOperationResponse> RestoreMachineBackupAsync(string backupPath,
            CancellationToken cancellationToken = default) =>
            throw new NotSupportedException();

        public Task<ElevatedOperationResponse> ManageServiceAsync(ServiceActionTarget target, ServiceActionKind action,
            CancellationToken cancellationToken = default) =>
            throw new NotSupportedException();

        public Task<ElevatedOperationResponse> ChangeScheduledTaskAsync(
            ScheduledTaskId id,
            ScheduledTaskChange change,
            bool expectedEnabled,
            string expectedDefinitionHash,
            CancellationToken cancellationToken = default)
        {
            LastTaskId = id;
            LastChange = change;
            LastExpectedEnabled = expectedEnabled;
            LastHash = expectedDefinitionHash;
            return Task.FromResult(new ElevatedOperationResponse(Guid.NewGuid().ToString("N"),
                new string('A', 64), true, false, "Changed through elevation.", 1, 0, ["Changed through elevation."]));
        }

        public Task<ElevatedOperationResponse> CreateSystemRestorePointAsync(
            CancellationToken cancellationToken = default) =>
            throw new NotSupportedException();

        public Task<ElevatedOperationResponse> RunCatalogRecipeAsync(
            string recipeId, string expectedRecipeHash, CancellationToken cancellationToken = default) =>
            throw new NotSupportedException();
    }

    private sealed class DecliningConfirmation : IScheduledTaskConfirmation
    {
        public Task<bool> ConfirmAsync(ScheduledTaskActionPreflight preflight, CancellationToken cancellationToken) =>
            Task.FromResult(false);
    }

    private sealed class FakeTimeProvider(DateTimeOffset start) : TimeProvider
    {
        private DateTimeOffset _now = start;
        public override DateTimeOffset GetUtcNow() => _now;
        public void Advance(TimeSpan duration) => _now = _now.Add(duration);
    }
}
