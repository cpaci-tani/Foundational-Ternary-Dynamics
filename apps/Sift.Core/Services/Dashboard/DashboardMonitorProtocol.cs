using System.Buffers.Binary;
using System.IO.Pipes;
using System.Security.Cryptography;
using System.Security.Principal;
using System.Text.Json;
using Sift.Models;

namespace Sift.Services;

public sealed record DashboardMonitorEnvelope(
    int ProtocolVersion,
    string AppVersion,
    string Command,
    DashboardSampleKind? SampleKind = null,
    DashboardSnapshotDelta? Snapshot = null,
    string? Message = null,
    bool Paused = false,
    IReadOnlyList<DashboardAlert>? Alerts = null,
    string? AlertId = null,
    DateTimeOffset? AlertTimestampUtc = null,
    DateTimeOffset? HostStartedUtc = null,
    DateTimeOffset? LastSampleUtc = null,
    IReadOnlyList<string>? Capabilities = null);

public static class DashboardMonitorProtocol
{
    public const int CurrentVersion = 1;
    public const int MaximumPayloadBytes = 2 * 1024 * 1024;
    public const string TelemetryOwnershipCapability = "telemetry-ownership-v2";
    private static readonly JsonSerializerOptions JsonOptions = new() { PropertyNameCaseInsensitive = false };

    public static string PipeName
    {
        get
        {
            using var identity = WindowsIdentity.GetCurrent();
            var value = identity.User?.Value ?? Environment.UserName;
            var hash = Convert.ToHexString(SHA256.HashData(System.Text.Encoding.UTF8.GetBytes(value)))[..16];
            return $"Sift.Monitor.{hash}";
        }
    }

    public static async Task WriteAsync(Stream stream, DashboardMonitorEnvelope envelope, CancellationToken token)
    {
        var payload = JsonSerializer.SerializeToUtf8Bytes(envelope, JsonOptions);
        if (payload.Length > MaximumPayloadBytes) throw new InvalidDataException("Dashboard monitor payload exceeds 2 MB.");
        var header = new byte[4];
        BinaryPrimitives.WriteInt32LittleEndian(header, payload.Length);
        await stream.WriteAsync(header, token);
        await stream.WriteAsync(payload, token);
        await stream.FlushAsync(token);
    }

    public static async Task<DashboardMonitorEnvelope> ReadAsync(Stream stream, CancellationToken token)
    {
        var header = new byte[4];
        await ReadExactlyAsync(stream, header, token);
        var length = BinaryPrimitives.ReadInt32LittleEndian(header);
        if (length is <= 0 or > MaximumPayloadBytes) throw new InvalidDataException("Dashboard monitor payload length is invalid.");
        var payload = new byte[length];
        await ReadExactlyAsync(stream, payload, token);
        var envelope = JsonSerializer.Deserialize<DashboardMonitorEnvelope>(payload, JsonOptions)
            ?? throw new InvalidDataException("Dashboard monitor payload is empty.");
        if (envelope.ProtocolVersion != CurrentVersion) throw new InvalidDataException("Dashboard monitor protocol version mismatch.");
        return envelope;
    }

    private static async Task ReadExactlyAsync(Stream stream, Memory<byte> buffer, CancellationToken token)
    {
        var offset = 0;
        while (offset < buffer.Length)
        {
            var read = await stream.ReadAsync(buffer[offset..], token);
            if (read == 0) throw new EndOfStreamException("Dashboard monitor disconnected.");
            offset += read;
        }
    }
}

public interface IDashboardMonitorAlertClient
{
    Task<bool> AcknowledgeAlertAsync(string alertId, DateTimeOffset acknowledgedUtc,
        CancellationToken cancellationToken = default);
    Task<bool> SnoozeAlertAsync(string alertId, DateTimeOffset snoozedUntilUtc,
        CancellationToken cancellationToken = default);
}

public sealed class DashboardMonitorTelemetrySource : IDashboardTelemetrySource, IDashboardMonitorAlertClient, IDisposable
{
    public static readonly TimeSpan MaximumHostSampleAge = TimeSpan.FromSeconds(15);
    private readonly IDashboardTelemetrySource _fallback;
    private readonly Func<bool> _monitorEnabled;
    private readonly string _appVersion;
    private readonly string _pipeName;
    private readonly Func<DateTimeOffset> _utcNow;
    private readonly object _generationGate = new();
    private long _lastHostGeneration;
    private DateTimeOffset _lastHostTimestampUtc;
    private DateTimeOffset? _lastHostStartedUtc;
    private bool _disposed;

    public DashboardMonitorTelemetrySource(
        IDashboardTelemetrySource fallback,
        Func<bool> monitorEnabled,
        string appVersion,
        string? pipeName = null,
        Func<DateTimeOffset>? utcNow = null)
    {
        _fallback = fallback;
        _monitorEnabled = monitorEnabled;
        _appVersion = appVersion;
        _pipeName = pipeName ?? DashboardMonitorProtocol.PipeName;
        _utcNow = utcNow ?? (() => DateTimeOffset.UtcNow);
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        if (_fallback is IDisposable disposable) disposable.Dispose();
    }

    public async Task<DashboardTelemetryRead> SampleAsync(
        DashboardSampleKind kind,
        CancellationToken cancellationToken = default)
    {
        if (!_monitorEnabled()) return await _fallback.SampleAsync(kind, cancellationToken);
        string fallbackReason = "The monitor host is unavailable.";
        try
        {
            using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
            timeout.CancelAfter(TimeSpan.FromMilliseconds(600));
            await using var pipe = new NamedPipeClientStream(".", _pipeName,
                PipeDirection.InOut, PipeOptions.Asynchronous | PipeOptions.CurrentUserOnly);
            await pipe.ConnectAsync(timeout.Token);
            await DashboardMonitorProtocol.WriteAsync(pipe,
                new DashboardMonitorEnvelope(DashboardMonitorProtocol.CurrentVersion, _appVersion, "snapshot", kind),
                timeout.Token);
            var response = await DashboardMonitorProtocol.ReadAsync(pipe, timeout.Token);
            if (response.Command == "snapshot" && response.Snapshot is not null &&
                response.AppVersion.Equals(_appVersion, StringComparison.OrdinalIgnoreCase) &&
                response.Capabilities?.Contains(DashboardMonitorProtocol.TelemetryOwnershipCapability,
                    StringComparer.Ordinal) == true &&
                response.Snapshot.ChangedMetricKeys is not null && response.Alerts is not null &&
                response.HostStartedUtc is { } hostStartedUtc)
            {
                var age = _utcNow() - response.Snapshot.TimestampUtc;
                if (age <= MaximumHostSampleAge || response.Paused)
                {
                    bool fresh;
                    lock (_generationGate)
                    {
                        if (_lastHostStartedUtc != hostStartedUtc)
                        {
                            _lastHostStartedUtc = hostStartedUtc;
                            _lastHostGeneration = 0;
                            _lastHostTimestampUtc = DateTimeOffset.MinValue;
                        }
                        fresh = response.Snapshot.Generation > _lastHostGeneration &&
                                response.Snapshot.TimestampUtc > _lastHostTimestampUtc;
                        if (fresh)
                        {
                            _lastHostGeneration = response.Snapshot.Generation;
                            _lastHostTimestampUtc = response.Snapshot.TimestampUtc;
                        }
                    }
                    return new DashboardTelemetryRead(
                        response.Snapshot,
                        DashboardTelemetryOrigin.MonitorHost,
                        fresh,
                        SourceHealthy: true,
                        SourceOwnsHistory: true,
                        SourceOwnsAlerts: true,
                        response.Paused ? "Monitor host is paused." : fresh ? "Monitor host supplied a fresh sample." : "Monitor host sample is unchanged.",
                        response.Alerts);
                }
                fallbackReason = $"The monitor host sample is stale ({Math.Max(0, age.TotalSeconds):0} seconds old).";
            }
            else if (response.Command == "version-mismatch")
                fallbackReason = response.Message ?? "The monitor host version does not match Sift.";
            else fallbackReason = response.Message ?? "The monitor host did not return a usable snapshot.";
        }
        catch (Exception exception) when (exception is IOException or TimeoutException or OperationCanceledException or InvalidDataException)
        {
            if (cancellationToken.IsCancellationRequested) throw new OperationCanceledException(cancellationToken);
            fallbackReason = exception is OperationCanceledException
                ? "The monitor host did not respond before the read deadline."
                : $"The monitor host is unavailable: {exception.Message}";
        }
        var local = await _fallback.SampleAsync(kind, cancellationToken);
        return local with
        {
            Origin = DashboardTelemetryOrigin.InProcess,
            IsFresh = true,
            SourceHealthy = false,
            SourceOwnsHistory = false,
            SourceOwnsAlerts = false,
            Status = fallbackReason + " Sift collected this sample in process.",
            Alerts = null
        };
    }

    public async Task<bool> AcknowledgeAlertAsync(
        string alertId,
        DateTimeOffset acknowledgedUtc,
        CancellationToken cancellationToken = default) =>
        await SendAlertCommandAsync("acknowledge-alert", alertId, acknowledgedUtc, cancellationToken);

    public async Task<bool> SnoozeAlertAsync(
        string alertId,
        DateTimeOffset snoozedUntilUtc,
        CancellationToken cancellationToken = default) =>
        await SendAlertCommandAsync("snooze-alert", alertId, snoozedUntilUtc, cancellationToken);

    private async Task<bool> SendAlertCommandAsync(
        string command,
        string alertId,
        DateTimeOffset timestampUtc,
        CancellationToken cancellationToken)
    {
        if (!_monitorEnabled() || string.IsNullOrWhiteSpace(alertId)) return false;
        var response = await SendCommandAsync(command, _appVersion, cancellationToken, alertId, timestampUtc, _pipeName);
        return response is { Command: "status" } &&
               response.AppVersion.Equals(_appVersion, StringComparison.OrdinalIgnoreCase) &&
               response.Message == "updated";
    }

    public static async Task<DashboardMonitorEnvelope?> SendCommandAsync(
        string command,
        string appVersion,
        CancellationToken cancellationToken = default,
        string? alertId = null,
        DateTimeOffset? alertTimestampUtc = null,
        string? pipeName = null)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(TimeSpan.FromSeconds(2));
        try
        {
            await using var pipe = new NamedPipeClientStream(".", pipeName ?? DashboardMonitorProtocol.PipeName,
                PipeDirection.InOut, PipeOptions.Asynchronous | PipeOptions.CurrentUserOnly);
            await pipe.ConnectAsync(timeout.Token);
            await DashboardMonitorProtocol.WriteAsync(pipe,
                new DashboardMonitorEnvelope(DashboardMonitorProtocol.CurrentVersion, appVersion, command,
                    AlertId: alertId, AlertTimestampUtc: alertTimestampUtc), timeout.Token);
            return await DashboardMonitorProtocol.ReadAsync(pipe, timeout.Token);
        }
        catch when (!cancellationToken.IsCancellationRequested) { return null; }
    }
}
