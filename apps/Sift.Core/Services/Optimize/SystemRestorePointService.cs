using System.Management;
using System.Security.Cryptography;
using System.Text;
using Sift.Models;
using Microsoft.Win32;

namespace Sift.Services;

public enum SystemProtectionState
{
    Available,
    Disabled,
    Unavailable,
    Unknown
}

public sealed record SystemRestorePointResult(
    bool Succeeded,
    bool Cancelled,
    string Message,
    IReadOnlyList<string> Log);

public sealed record SystemRestorePointInspection(
    string MachineIdentityHash,
    bool SystemRestoreAvailable,
    SystemProtectionState ProtectionState,
    string Evidence);

public sealed record SystemRestorePointPreflight(
    Guid TicketId,
    DateTime ExpiresUtc,
    string ExpectedMachineIdentityHash,
    bool ExpectedSystemRestoreAvailable,
    SystemProtectionState ExpectedProtectionState,
    string Evidence);

public interface ISystemRestorePointController
{
    SystemRestorePointInspection Inspect();
    SystemRestorePointResult Create(SystemRestorePointInspection expected);
}

public interface ISystemRestorePointService
{
    bool IsEligible(bool offerEnabled, IReadOnlyList<Tweak> confirmedSelection);
    SystemRestorePointPreflight Preflight(bool offerEnabled, IReadOnlyList<Tweak> confirmedSelection);
    void Revoke(Guid ticketId);
    Task<SystemRestorePointResult> ExecuteAsync(Guid ticketId, CancellationToken cancellationToken = default);
}

public sealed class SystemRestorePointController : ISystemRestorePointController
{
    private const string Description = "Sift before Optimize changes";

    public SystemRestorePointInspection Inspect()
    {
        var hash = ComputeMachineHash();
        var (available, state, evidence) = ReadProtectionState();
        return new SystemRestorePointInspection(hash, available, state, evidence);
    }

    public SystemRestorePointResult Create(SystemRestorePointInspection expected)
    {
        var current = Inspect();
        if (!Matches(current, expected))
            return Failed("The System Restore environment changed after confirmation; no restore point was created.");

        if (!ElevationHelper.IsElevated())
            return Failed("Administrator rights are required to create a System Restore point.");

        try
        {
            var scope = new ManagementScope(@"\\.\root\default");
            scope.Connect();
            using var restoreClass = new ManagementClass(scope, new ManagementPath("SystemRestore"), null);
            var inParams = restoreClass.GetMethodParameters("CreateRestorePoint");
            inParams["Description"] = Description;
            inParams["RestorePointType"] = 0;
            inParams["EventType"] = 100;
            var result = restoreClass.InvokeMethod("CreateRestorePoint", inParams, null);
            var code = Convert.ToInt32(result?["ReturnValue"] ?? -1);
            if (code == 0)
            {
                const string message =
                    "Sift requested a best-effort System Restore point. Windows may decline creation when System Protection is unavailable, disabled, rate-limited, or blocked by policy.";
                return new SystemRestorePointResult(true, false, message, [message]);
            }

            var failure =
                $"Sift requested a best-effort System Restore point, but Windows returned code {code}. Restore may be disabled on this PC.";
            return new SystemRestorePointResult(false, false, failure, [failure]);
        }
        catch (Exception exception)
        {
            var failure = $"Could not create a System Restore point: {exception.Message}";
            return new SystemRestorePointResult(false, false, failure, [failure]);
        }
    }

    private static bool Matches(SystemRestorePointInspection current, SystemRestorePointInspection expected) =>
        string.Equals(current.MachineIdentityHash, expected.MachineIdentityHash, StringComparison.Ordinal) &&
        current.SystemRestoreAvailable == expected.SystemRestoreAvailable &&
        current.ProtectionState == expected.ProtectionState;

    private static string ComputeMachineHash()
    {
        var material = $"{Environment.MachineName}|{Environment.OSVersion.Version}";
        return Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(material)));
    }

    private static (bool available, SystemProtectionState state, string evidence) ReadProtectionState()
    {
        try
        {
            using var key = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\Microsoft\Windows NT\CurrentVersion\SystemRestore");
            if (key is null)
                return (false, SystemProtectionState.Unavailable, "System Restore registry information is unavailable.");
            var disabled = key.GetValue("DisableSR") as int? ?? 0;
            if (disabled != 0)
                return (false, SystemProtectionState.Disabled,
                    "System Restore appears disabled by policy or configuration on this PC.");
            return (true, SystemProtectionState.Available,
                "System Restore appears available for a best-effort point. This is not a guarantee of rollback.");
        }
        catch (Exception exception)
        {
            return (false, SystemProtectionState.Unknown,
                $"Could not read System Restore availability: {exception.Message}");
        }
    }

    private static SystemRestorePointResult Failed(string message) =>
        new(false, false, message, [message]);
}

public sealed class SystemRestorePointService : ISystemRestorePointService
{
    private readonly ISystemRestorePointController _controller;
    private readonly IElevationBroker _elevation;
    private readonly Func<bool> _isElevated;
    private readonly TimeProvider _time;
    private readonly TimeSpan _ticketLifetime;
    private readonly Dictionary<Guid, Ticket> _tickets = new();
    private readonly object _sync = new();

    public SystemRestorePointService(
        ISystemRestorePointController controller,
        IElevationBroker elevation,
        Func<bool> isElevated,
        TimeProvider? time = null,
        TimeSpan? ticketLifetime = null)
    {
        _controller = controller;
        _elevation = elevation;
        _isElevated = isElevated;
        _time = time ?? TimeProvider.System;
        _ticketLifetime = ticketLifetime ?? TimeSpan.FromMinutes(5);
    }

    public bool IsEligible(bool offerEnabled, IReadOnlyList<Tweak> confirmedSelection) =>
        offerEnabled && confirmedSelection.Any(tweak =>
            (tweak.Kind == TweakKind.Registry &&
             tweak.Target.StartsWith("HKLM\\", StringComparison.OrdinalIgnoreCase)) ||
            tweak is { Kind: TweakKind.AppPackage, Risk: TweakRisk.Advanced });

    public SystemRestorePointPreflight Preflight(bool offerEnabled, IReadOnlyList<Tweak> confirmedSelection)
    {
        if (!IsEligible(offerEnabled, confirmedSelection))
            throw new InvalidOperationException("The selected Optimize batch is not eligible for a restore point.");

        var inspection = _controller.Inspect();
        if (!IsValidHash(inspection.MachineIdentityHash))
            throw new InvalidOperationException("The machine identity hash is unavailable.");

        var ticketId = Guid.NewGuid();
        var expiresUtc = _time.GetUtcNow().UtcDateTime.Add(_ticketLifetime);
        var evidence =
            "Sift requested a best-effort System Restore point before eligible Optimize changes. " +
            "Windows may decline creation when System Protection is unavailable, disabled, rate-limited, or blocked by policy. " +
            "This is not a guarantee of rollback or recovery.\n" +
            $"Protection state: {inspection.ProtectionState}\n" +
            $"{inspection.Evidence}\n" +
            $"Fixed description: {SystemRestorePointControllerDescription}\n" +
            $"Expires UTC: {expiresUtc:O}\n" +
            "No restore point was created during preflight.";
        var preflight = new SystemRestorePointPreflight(
            ticketId, expiresUtc, inspection.MachineIdentityHash, inspection.SystemRestoreAvailable,
            inspection.ProtectionState, evidence);
        lock (_sync) _tickets[ticketId] = new Ticket(preflight, inspection, false);
        return preflight;
    }

    private const string SystemRestorePointControllerDescription = "Sift before Optimize changes";

    public void Revoke(Guid ticketId)
    {
        lock (_sync) _tickets.Remove(ticketId);
    }

    public async Task<SystemRestorePointResult> ExecuteAsync(Guid ticketId,
        CancellationToken cancellationToken = default)
    {
        Ticket ticket;
        lock (_sync)
        {
            if (!_tickets.TryGetValue(ticketId, out ticket!))
                return Failed("The restore-point authorization expired or was already used.");
            if (ticket.Consumed)
                return Failed("The restore-point authorization expired or was already used.");
            ticket.Consumed = true;
        }

        if (_time.GetUtcNow().UtcDateTime > ticket.Preflight.ExpiresUtc)
        {
            Revoke(ticketId);
            return Failed("The restore-point authorization expired or was already used.");
        }

        var current = _controller.Inspect();
        if (!string.Equals(current.MachineIdentityHash, ticket.Preflight.ExpectedMachineIdentityHash,
                StringComparison.Ordinal) ||
            current.SystemRestoreAvailable != ticket.Preflight.ExpectedSystemRestoreAvailable ||
            current.ProtectionState != ticket.Preflight.ExpectedProtectionState)
        {
            Revoke(ticketId);
            return Failed("The System Restore environment changed after confirmation; no restore point was created.");
        }

        SystemRestorePointResult result;
        if (_isElevated())
            result = _controller.Create(current);
        else
        {
            var response = await _elevation.CreateSystemRestorePointAsync(cancellationToken);
            result = MapElevatedResponse(response);
        }

        Revoke(ticketId);
        return result;
    }

    internal static SystemRestorePointResult MapElevatedResponse(ElevatedOperationResponse response)
    {
        if (response.Cancelled)
            return new SystemRestorePointResult(false, true, response.Message, response.Log);
        if (response.Succeeded && response.Applied == 1 && response.Failed == 0)
            return new SystemRestorePointResult(true, false, response.Message, response.Log);
        return new SystemRestorePointResult(false, false, response.Message, response.Log);
    }

    private static bool IsValidHash(string hash) =>
        hash.Length == 64 && hash.All(character => character is >= '0' and <= '9' or >= 'A' and <= 'F');

    private static SystemRestorePointResult Failed(string message) =>
        new(false, false, message, [message]);

    private sealed class Ticket(
        SystemRestorePointPreflight preflight,
        SystemRestorePointInspection inspection,
        bool consumed)
    {
        public SystemRestorePointPreflight Preflight { get; } = preflight;
        public SystemRestorePointInspection Inspection { get; } = inspection;
        public bool Consumed { get; set; } = consumed;
    }
}
