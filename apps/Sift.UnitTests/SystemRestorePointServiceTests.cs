using System.Text.Json;
using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class SystemRestorePointServiceTests
{
    private const string Hash = "ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789";

    [Fact]
    public void Eligibility_requires_offer_flag_and_hklm_or_advanced_package()
    {
        var service = CreateService();
        var hklm = TweakCatalog.Create().First(tweak =>
            tweak.Kind == TweakKind.Registry && tweak.Target.StartsWith("HKLM\\", StringComparison.OrdinalIgnoreCase));
        var hkcu = TweakCatalog.Create().First(tweak =>
            tweak.Kind == TweakKind.Registry && tweak.Target.StartsWith("HKCU\\", StringComparison.OrdinalIgnoreCase));
        var advanced = TweakCatalog.Create().First(tweak => tweak is { Kind: TweakKind.AppPackage, Risk: TweakRisk.Advanced });

        Assert.True(service.IsEligible(true, [hklm]));
        Assert.True(service.IsEligible(true, [advanced]));
        Assert.False(service.IsEligible(false, [hklm]));
        Assert.False(service.IsEligible(true, [hkcu]));
    }

    [Fact]
    public void Preflight_is_non_mutating_and_records_expected_environment()
    {
        var controller = new RecordingRestoreController(Hash);
        var service = CreateService(controller);
        var selection = EligibleSelection();

        var preflight = service.Preflight(true, selection);

        Assert.Equal(1, controller.InspectCount);
        Assert.Equal(0, controller.CreateCount);
        Assert.Equal(Hash, preflight.ExpectedMachineIdentityHash);
        Assert.Contains("No restore point was created during preflight.", preflight.Evidence);
    }

    [Fact]
    public async Task Execute_rejects_changed_environment_without_mutation()
    {
        var controller = new RecordingRestoreController(Hash);
        var service = CreateService(controller);
        var preflight = service.Preflight(true, EligibleSelection());
        controller.ProtectionState = SystemProtectionState.Disabled;

        var result = await service.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);

        Assert.False(result.Succeeded);
        Assert.Equal(0, controller.CreateCount);
        Assert.Contains("changed after confirmation", result.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public async Task Ticket_is_single_use_and_expired_tickets_do_not_mutate()
    {
        var time = new FakeTimeProvider(DateTimeOffset.UtcNow);
        var controller = new RecordingRestoreController(Hash);
        var service = CreateService(controller, time: time);
        var preflight = service.Preflight(true, EligibleSelection());
        time.Advance(TimeSpan.FromMinutes(6));

        var expired = await service.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);
        Assert.False(expired.Succeeded);
        Assert.Equal(0, controller.CreateCount);

        service.Revoke(preflight.TicketId);
        var reused = await service.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);
        Assert.False(reused.Succeeded);
        Assert.Equal(0, controller.CreateCount);
    }

    [Fact]
    public async Task Standard_user_execution_uses_parameter_free_broker_path()
    {
        var broker = new RecordingElevationBroker();
        var service = CreateService(broker: broker, elevated: false);
        var preflight = service.Preflight(true, EligibleSelection());

        var result = await service.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);

        Assert.True(result.Succeeded);
        Assert.Equal(1, broker.RestorePointCalls);
        Assert.Equal(0, broker.RestorePointPayloadFields);
    }

    private static IReadOnlyList<Tweak> EligibleSelection() =>
        TweakCatalog.Create().Where(tweak =>
            tweak.Kind == TweakKind.Registry &&
            tweak.Target.StartsWith("HKLM\\", StringComparison.OrdinalIgnoreCase)).Take(1).ToList();

    private static SystemRestorePointService CreateService(
        RecordingRestoreController? controller = null,
        RecordingElevationBroker? broker = null,
        FakeTimeProvider? time = null,
        bool elevated = true) =>
        new(controller ?? new RecordingRestoreController(Hash), broker ?? new RecordingElevationBroker(),
            () => elevated, time);

    private sealed class RecordingRestoreController(string hash) : ISystemRestorePointController
    {
        public int InspectCount { get; private set; }
        public int CreateCount { get; private set; }
        public SystemProtectionState ProtectionState { get; set; } = SystemProtectionState.Available;

        public SystemRestorePointInspection Inspect()
        {
            InspectCount++;
            return new SystemRestorePointInspection(hash, true, ProtectionState, "fixture evidence");
        }

        public SystemRestorePointResult Create(SystemRestorePointInspection expected)
        {
            CreateCount++;
            return new SystemRestorePointResult(true, false, "restore point created", ["restore point created"]);
        }
    }

    private sealed class RecordingElevationBroker : IElevationBroker
    {
        public int RestorePointCalls { get; private set; }
        public int RestorePointPayloadFields { get; private set; }

        public Task<ElevatedOperationResponse> ApplyMachineTweaksAsync(IEnumerable<Tweak> selection,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> RestoreMachineBackupAsync(string backupPath,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> ManageServiceAsync(ServiceActionTarget target, ServiceActionKind action,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> ChangeScheduledTaskAsync(ScheduledTaskId id, ScheduledTaskChange change,
            bool expectedEnabled, string expectedDefinitionHash,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> CreateSystemRestorePointAsync(
            CancellationToken cancellationToken = default)
        {
            RestorePointCalls++;
            return Task.FromResult(new ElevatedOperationResponse("id", "nonce", true, false,
                "Sift requested a best-effort System Restore point.", 1, 0, []));
        }

        public Task<ElevatedOperationResponse> RunCatalogRecipeAsync(
            string recipeId, string expectedRecipeHash, CancellationToken cancellationToken = default) =>
            throw new NotSupportedException();
    }

    private sealed class FakeTimeProvider(DateTimeOffset start) : TimeProvider
    {
        private DateTimeOffset _now = start;
        public void Advance(TimeSpan span) => _now = _now.Add(span);
        public override DateTimeOffset GetUtcNow() => _now;
    }
}
