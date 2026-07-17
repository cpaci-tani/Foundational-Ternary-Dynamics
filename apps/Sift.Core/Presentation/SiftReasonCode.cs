namespace Sift.Presentation;

/// <summary>
/// Stable machine identifiers for customer-facing policy outcomes.
/// Core keeps English fallbacks; WinUI may map codes to .resw resources.
/// </summary>
public enum SiftReasonCode
{
    Unspecified = 0,
    TargetUnavailable,
    ProcessProtected,
    ProcessElevatedRestartDisabled,
    ProcessSessionMismatch,
    ProcessInstanceChanged,
    ProcessPathUnreadable,
    ServiceStateMismatch,
    ServiceActionStateMismatch,
    ServiceNotManageable,
    ScriptCatalogMismatch,
    ScriptForbiddenToken,
    ScriptHostMissing,
    ScriptElevationRequired,
    ElevationRecipeIdentityInvalid,
    ElevationRecipeNotAdministrator,
    ElevationRecipeHashMismatch,
    ElevationRequestShapeInvalid
}

public static class ReasonMessages
{
    public static string Format(SiftReasonCode code, params object?[] args) => code switch
    {
        SiftReasonCode.TargetUnavailable => "target unavailable",
        SiftReasonCode.ProcessProtected => "protected process",
        SiftReasonCode.ProcessElevatedRestartDisabled =>
            "process restart is disabled while Sift is elevated to prevent an elevated child process",
        SiftReasonCode.ProcessSessionMismatch =>
            "process actions are limited to the current interactive session",
        SiftReasonCode.ProcessInstanceChanged =>
            "The process instance changed after selection; action cancelled.",
        SiftReasonCode.ProcessPathUnreadable => "executable path is unavailable",
        SiftReasonCode.ServiceStateMismatch =>
            args.Length > 0
                ? $"service state is {args[0]}"
                : "the service state changed after review",
        SiftReasonCode.ServiceActionStateMismatch =>
            "the requested action does not match the confirmed service state",
        SiftReasonCode.ServiceNotManageable =>
            args.Length > 0 ? Convert.ToString(args[0]) ?? "this service cannot be managed"
                : "this service cannot be managed",
        SiftReasonCode.ScriptCatalogMismatch =>
            "The recipe and all security metadata must exactly match the bundled catalog.",
        SiftReasonCode.ScriptForbiddenToken =>
            args.Length > 0
                ? $"The command contains blocked token '{args[0]}'."
                : "The command contains a blocked token.",
        SiftReasonCode.ScriptHostMissing =>
            args.Length >= 2
                ? $"The trusted {args[0]} host is not installed: {args[1]}"
                : "The trusted script host is not installed.",
        SiftReasonCode.ScriptElevationRequired =>
            "Administrator recipes require Windows administrator permission.",
        SiftReasonCode.ElevationRecipeIdentityInvalid =>
            "The catalog recipe identity is incomplete or invalid.",
        SiftReasonCode.ElevationRecipeNotAdministrator =>
            "Only administrator catalog recipes can cross the elevation boundary.",
        SiftReasonCode.ElevationRecipeHashMismatch =>
            "The catalog recipe no longer matches the reviewed identity.",
        SiftReasonCode.ElevationRequestShapeInvalid =>
            "The catalog recipe request is incomplete or contains unrelated fields.",
        _ => args.Length > 0 ? Convert.ToString(args[0]) ?? "Action unavailable." : "Action unavailable."
    };
}
