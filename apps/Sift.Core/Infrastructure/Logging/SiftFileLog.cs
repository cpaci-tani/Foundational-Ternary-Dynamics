using Serilog;
using Serilog.Events;
using Sift.Services;

namespace Sift.Infrastructure.Logging;

/// <summary>
/// Serilog file sink under <c>%LOCALAPPDATA%\Sift\logs</c>. Local only; never phones home.
/// </summary>
public sealed class SiftFileLog : ISiftLog
{
    private readonly ILogger _logger;
    private bool _disposed;

    public SiftFileLog(string? directory = null, bool verbose = false)
    {
        var root = directory ?? Path.Combine(ProductPaths.DataRoot, "logs");
        Directory.CreateDirectory(root);
        var level = verbose || IsVerboseEnvironment() ? LogEventLevel.Debug : LogEventLevel.Information;
        _logger = new LoggerConfiguration()
            .MinimumLevel.Is(level)
            .WriteTo.File(
                Path.Combine(root, "sift-.log"),
                rollingInterval: RollingInterval.Day,
                retainedFileCountLimit: 14,
                shared: true,
                outputTemplate: "{Timestamp:yyyy-MM-dd HH:mm:ss.fff zzz} [{Level:u3}] {SourceContext}: {Message:lj}{NewLine}{Exception}")
            .CreateLogger();
    }

    public void Debug(string source, string message) =>
        _logger.ForContext("SourceContext", source).Debug("{Message}", message);

    public void Information(string source, string message) =>
        _logger.ForContext("SourceContext", source).Information("{Message}", message);

    public void Warning(string source, string message, string? detail = null) =>
        _logger.ForContext("SourceContext", source).Warning(
            string.IsNullOrWhiteSpace(detail) ? "{Message}" : "{Message} · {Detail}",
            message, detail);

    public void Error(string source, string message, Exception? exception = null)
    {
        var log = _logger.ForContext("SourceContext", source);
        if (exception is null) log.Error("{Message}", message);
        else log.Error(exception, "{Message}", message);
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        if (_logger is IDisposable disposable) disposable.Dispose();
    }

    private static bool IsVerboseEnvironment() =>
        string.Equals(Environment.GetEnvironmentVariable("SIFT_LOG_VERBOSE"), "1", StringComparison.Ordinal) ||
        string.Equals(Environment.GetEnvironmentVariable("SIFT_LOG_VERBOSE"), "true", StringComparison.OrdinalIgnoreCase);
}
