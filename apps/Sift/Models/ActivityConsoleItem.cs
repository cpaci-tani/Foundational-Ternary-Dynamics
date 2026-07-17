using Sift.Infrastructure.Activity;

namespace Sift.WinUI.Models;

public sealed record ActivityConsoleItem(
    DateTime CreatedLocal,
    ActivitySeverity Severity,
    string Category,
    string Summary,
    string? Detail,
    string? OperationId)
{
    public string Header => $"{CreatedLocal:HH:mm:ss.fff}   {Severity.ToString().ToUpperInvariant(),-7}   {Category.ToUpperInvariant()}";
    public string Message => string.IsNullOrWhiteSpace(Detail) ? Summary : $"{Summary} · {Detail}";
    public string SearchText => $"{Severity} {Category} {Summary} {Detail} {OperationId}";

    public static ActivityConsoleItem From(ActivityEvent value) => new(
        value.CreatedUtc.ToLocalTime(), value.Severity, value.Category, value.Summary, value.Detail, value.OperationId);
}
