namespace Sift.Infrastructure.Logging;

/// <summary>
/// Local diagnostic log. No network sinks. Complements <c>ActivityHub</c> (customer-visible activity)
/// with structured file output under the per-user data root.
/// </summary>
public interface ISiftLog : IDisposable
{
    void Debug(string source, string message);
    void Information(string source, string message);
    void Warning(string source, string message, string? detail = null);
    void Error(string source, string message, Exception? exception = null);
}
