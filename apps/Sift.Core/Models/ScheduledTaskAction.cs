namespace Sift.Models;

public enum ScheduledTaskId
{
    OfficeAutomaticUpdates,
    OfficeFeatureUpdates
}

public enum ScheduledTaskChange
{
    Enable,
    Disable
}

public sealed record ScheduledTaskIdentity(
    ScheduledTaskId Id,
    string DisplayName,
    bool Enabled,
    string State,
    string DefinitionHash);

public sealed record ScheduledTaskActionPreflight(
    Guid TicketId,
    ScheduledTaskId Id,
    string DisplayName,
    ScheduledTaskChange Change,
    bool ExpectedEnabled,
    string ExpectedState,
    string ExpectedDefinitionHash,
    DateTime ExpiresUtc,
    string Evidence);

public sealed record ScheduledTaskActionResult(
    bool Succeeded,
    bool Cancelled,
    string Summary,
    IReadOnlyList<string> Log);
