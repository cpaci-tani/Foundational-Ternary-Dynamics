namespace Sift.Models;

/// <summary>
/// Keeps the short-lived identity and authorization produced by one installed-app workflow.
/// A continuation is valid only for the exact registration identity and display name that
/// were reviewed when the uninstaller or registration cleanup started.
/// </summary>
public sealed class InstalledAppUninstallState
{
    public InstalledApp? Target { get; private set; }
    public string? SessionId { get; private set; }
    public string? ContinuationToken { get; private set; }
    public string Status { get; private set; } = string.Empty;

    public bool HasPendingSession => Target is not null &&
        !string.IsNullOrWhiteSpace(SessionId) && string.IsNullOrWhiteSpace(ContinuationToken);

    public bool CleanupAuthorized => Target is not null && !string.IsNullOrWhiteSpace(ContinuationToken);

    public void TrackUninstaller(InstalledApp target, string sessionId, string status)
    {
        ArgumentNullException.ThrowIfNull(target);
        ArgumentException.ThrowIfNullOrWhiteSpace(sessionId);
        Target = target;
        SessionId = sessionId;
        ContinuationToken = null;
        Status = status;
    }

    public void AuthorizeCleanup(InstalledApp target, string continuationToken, string status)
    {
        ArgumentNullException.ThrowIfNull(target);
        ArgumentException.ThrowIfNullOrWhiteSpace(continuationToken);
        Target = target;
        SessionId = null;
        ContinuationToken = continuationToken;
        Status = status;
    }

    public void UpdateStatus(string status) => Status = status;

    public bool Matches(InstalledApp? app) => app is not null && Target is not null &&
        string.Equals(Target.RegistryLocation.Identity, app.RegistryLocation.Identity,
            StringComparison.OrdinalIgnoreCase) &&
        string.Equals(Target.DisplayName, app.DisplayName, StringComparison.Ordinal);

    public string? ContinuationFor(InstalledApp app) => Matches(app) ? ContinuationToken : null;

    public void Clear()
    {
        Target = null;
        SessionId = null;
        ContinuationToken = null;
        Status = string.Empty;
    }
}
