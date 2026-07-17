using Sift.Models;

namespace Sift.Services;

public interface IScheduledTaskController
{
    ScheduledTaskIdentity? Inspect(ScheduledTaskId id);
    ScheduledTaskActionResult SetEnabled(
        ScheduledTaskId id,
        ScheduledTaskChange change,
        bool expectedEnabled,
        string expectedDefinitionHash);
}

public interface IScheduledTaskActionService
{
    ScheduledTaskActionPreflight Preflight(ScheduledTaskId id, ScheduledTaskChange change);
    void Revoke(Guid ticketId);
    Task<ScheduledTaskActionResult> ExecuteAsync(Guid ticketId, CancellationToken cancellationToken = default);
}

public interface IScheduledTaskConfirmation
{
    Task<bool> ConfirmAsync(
        ScheduledTaskActionPreflight preflight,
        CancellationToken cancellationToken);
}

public interface IScheduledTaskActionWorkflow
{
    Task<ScheduledTaskActionResult> RunAsync(
        ScheduledTaskId id,
        ScheduledTaskChange change,
        IScheduledTaskConfirmation confirmation,
        CancellationToken cancellationToken = default);
}

public sealed class ScheduledTaskActionWorkflow(IScheduledTaskActionService actions) : IScheduledTaskActionWorkflow
{
    public async Task<ScheduledTaskActionResult> RunAsync(
        ScheduledTaskId id,
        ScheduledTaskChange change,
        IScheduledTaskConfirmation confirmation,
        CancellationToken cancellationToken = default)
    {
        var preflight = actions.Preflight(id, change);
        if (!await confirmation.ConfirmAsync(preflight, cancellationToken))
        {
            actions.Revoke(preflight.TicketId);
            return new ScheduledTaskActionResult(false, true,
                "Scheduled-task action cancelled; nothing was changed.", []);
        }

        return await actions.ExecuteAsync(preflight.TicketId, cancellationToken);
    }
}

public sealed class ScheduledTaskActionService : IScheduledTaskActionService
{
    private readonly IScheduledTaskController _controller;
    private readonly IElevationBroker _elevation;
    private readonly Func<bool> _isElevated;
    private readonly TimeProvider _time;
    private readonly TimeSpan _ticketLifetime;
    private readonly Dictionary<Guid, Ticket> _tickets = new();
    private readonly object _sync = new();

    public ScheduledTaskActionService(
        IScheduledTaskController controller,
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

    public ScheduledTaskActionPreflight Preflight(ScheduledTaskId id, ScheduledTaskChange change)
    {
        var identity = _controller.Inspect(id) ??
            throw new InvalidOperationException("The scheduled task is no longer present or readable.");
        var desiredEnabled = change == ScheduledTaskChange.Enable;
        if (identity.Enabled == desiredEnabled)
            throw new InvalidOperationException(
                $"The scheduled task is already {(desiredEnabled ? "enabled" : "disabled")}.");
        if (!IsValidHash(identity.DefinitionHash))
            throw new InvalidOperationException("The scheduled-task definition hash is unavailable.");

        var ticketId = Guid.NewGuid();
        var expiresUtc = _time.GetUtcNow().UtcDateTime.Add(_ticketLifetime);
        var evidence =
            $"Task: {identity.DisplayName}\nCurrent state: {identity.State}\nRequested change: {change}\n" +
            $"Expected enabled after change: {desiredEnabled}\nExpires UTC: {expiresUtc:O}\n" +
            "No change was made during preflight.";
        var preflight = new ScheduledTaskActionPreflight(
            ticketId, id, identity.DisplayName, change, identity.Enabled, identity.State,
            identity.DefinitionHash, expiresUtc, evidence);
        lock (_sync) _tickets[ticketId] = new Ticket(preflight, false);
        return preflight;
    }

    public void Revoke(Guid ticketId)
    {
        lock (_sync) _tickets.Remove(ticketId);
    }

    public async Task<ScheduledTaskActionResult> ExecuteAsync(Guid ticketId,
        CancellationToken cancellationToken = default)
    {
        Ticket ticket;
        lock (_sync)
        {
            if (!_tickets.TryGetValue(ticketId, out ticket!))
                return Failed("The scheduled-task authorization expired or was already used.");
            if (ticket.Consumed)
                return Failed("The scheduled-task authorization expired or was already used.");
            ticket.Consumed = true;
        }

        if (_time.GetUtcNow().UtcDateTime > ticket.Preflight.ExpiresUtc)
        {
            Revoke(ticketId);
            return Failed("The scheduled-task authorization expired or was already used.");
        }

        var preflight = ticket.Preflight;
        var current = _controller.Inspect(preflight.Id);
        if (current is null ||
            current.Enabled != preflight.ExpectedEnabled ||
            !string.Equals(current.State, preflight.ExpectedState, StringComparison.Ordinal) ||
            !string.Equals(current.DefinitionHash, preflight.ExpectedDefinitionHash, StringComparison.Ordinal))
        {
            Revoke(ticketId);
            return Failed("The scheduled task changed after confirmation; nothing was changed.");
        }

        ScheduledTaskActionResult result;
        if (_isElevated())
        {
            result = _controller.SetEnabled(preflight.Id, preflight.Change, preflight.ExpectedEnabled,
                preflight.ExpectedDefinitionHash);
        }
        else
        {
            var response = await _elevation.ChangeScheduledTaskAsync(
                preflight.Id, preflight.Change, preflight.ExpectedEnabled, preflight.ExpectedDefinitionHash,
                cancellationToken);
            result = MapScheduledTaskResponse(response);
        }

        Revoke(ticketId);
        return result;
    }

    internal static ScheduledTaskActionResult MapScheduledTaskResponse(ElevatedOperationResponse response)
    {
        if (response.Cancelled)
            return new ScheduledTaskActionResult(false, true, response.Message, response.Log);
        if (response.Succeeded && response.Applied == 1 && response.Failed == 0)
            return new ScheduledTaskActionResult(true, false, response.Message, response.Log);
        return new ScheduledTaskActionResult(false, false, response.Message, response.Log);
    }

    private static bool IsValidHash(string hash) =>
        hash.Length == 64 && hash.All(character => character is >= '0' and <= '9' or >= 'A' and <= 'F');

    private static ScheduledTaskActionResult Failed(string summary) =>
        new(false, false, summary, [summary]);

    private sealed class Ticket(ScheduledTaskActionPreflight preflight, bool consumed)
    {
        public ScheduledTaskActionPreflight Preflight { get; } = preflight;
        public bool Consumed { get; set; } = consumed;
    }
}
