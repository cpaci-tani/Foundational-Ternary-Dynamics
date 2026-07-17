namespace Sift.Infrastructure.Activity;

public enum ActivitySeverity
{
    Trace,
    Info,
    Warning,
    Error
}

public sealed record ActivityEvent(
    DateTime CreatedUtc,
    ActivitySeverity Severity,
    string Category,
    string Summary,
    string? Detail = null,
    string? RelatedPath = null,
    bool Persist = false,
    string? OperationId = null)
{
    public static ActivityEvent Create(
        string category,
        string summary,
        ActivitySeverity severity = ActivitySeverity.Info,
        string? detail = null,
        string? relatedPath = null,
        bool persist = false,
        string? operationId = null) =>
        new(DateTime.UtcNow, severity, category, summary, detail, relatedPath, persist, operationId);
}
