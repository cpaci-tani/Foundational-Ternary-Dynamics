using System.IO.Pipes;
using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class DashboardMonitorProtocolTests
{
    [Fact]
    public async Task Protocol_round_trips_typed_snapshot_without_action_payloads()
    {
        await using var stream = new MemoryStream();
        var request = new DashboardMonitorEnvelope(
            DashboardMonitorProtocol.CurrentVersion, "1.0.0", "snapshot", DashboardSampleKind.Fast);
        await DashboardMonitorProtocol.WriteAsync(stream, request, TestContext.Current.CancellationToken);
        stream.Position = 0;
        var result = await DashboardMonitorProtocol.ReadAsync(stream, TestContext.Current.CancellationToken);

        Assert.Equal("snapshot", result.Command);
        Assert.Equal(DashboardSampleKind.Fast, result.SampleKind);
        Assert.DoesNotContain("elevat", result.Command, StringComparison.OrdinalIgnoreCase);
        Assert.DoesNotContain("execute", result.Command, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public async Task Protocol_rejects_oversized_frame_before_allocation()
    {
        await using var stream = new MemoryStream();
        var header = BitConverter.GetBytes(DashboardMonitorProtocol.MaximumPayloadBytes + 1);
        await stream.WriteAsync(header, TestContext.Current.CancellationToken);
        stream.Position = 0;
        await Assert.ThrowsAsync<InvalidDataException>(() =>
            DashboardMonitorProtocol.ReadAsync(stream, TestContext.Current.CancellationToken));
    }

    [Fact]
    public async Task Monitor_wrapper_marks_duplicate_generation_unchanged()
    {
        var pipeName = $"Sift.Test.{Guid.NewGuid():N}";
        var now = DateTimeOffset.UtcNow;
        var snapshot = Snapshot(now, generation: 7);
        var server = ServeAsync(pipeName, "1.0.0", snapshot, connections: 2);
        var source = new DashboardMonitorTelemetrySource(
            new FixtureTelemetrySource(now), () => true, "1.0.0", pipeName, () => now);

        var first = await source.SampleAsync(DashboardSampleKind.Fast, TestContext.Current.CancellationToken);
        var second = await source.SampleAsync(DashboardSampleKind.Fast, TestContext.Current.CancellationToken);
        await server;

        Assert.Equal(DashboardTelemetryOrigin.MonitorHost, first.Origin);
        Assert.True(first.IsFresh);
        Assert.True(first.SourceOwnsHistory);
        Assert.True(first.SourceOwnsAlerts);
        Assert.False(second.IsFresh);
    }

    [Fact]
    public async Task Monitor_wrapper_rejects_stale_host_sample_and_assigns_fallback_ownership()
    {
        var pipeName = $"Sift.Test.{Guid.NewGuid():N}";
        var now = DateTimeOffset.UtcNow;
        var server = ServeAsync(pipeName, "1.0.0", Snapshot(now.AddMinutes(-1), generation: 3), connections: 1);
        var fallback = new FixtureTelemetrySource(now);
        var source = new DashboardMonitorTelemetrySource(fallback, () => true, "1.0.0", pipeName, () => now);

        var result = await source.SampleAsync(DashboardSampleKind.Fast, TestContext.Current.CancellationToken);
        await server;

        Assert.Equal(DashboardTelemetryOrigin.InProcess, result.Origin);
        Assert.False(result.SourceHealthy);
        Assert.False(result.SourceOwnsHistory);
        Assert.False(result.SourceOwnsAlerts);
        Assert.Contains("stale", result.Status, StringComparison.OrdinalIgnoreCase);
        Assert.Equal(1, fallback.Calls);
    }

    [Fact]
    public async Task Monitor_wrapper_accepts_lower_generation_after_host_restart()
    {
        var pipeName = $"Sift.Test.{Guid.NewGuid():N}";
        var now = DateTimeOffset.UtcNow;
        var source = new DashboardMonitorTelemetrySource(
            new FixtureTelemetrySource(now), () => true, "1.0.0", pipeName, () => now);

        var firstServer = ServeAsync(pipeName, "1.0.0", Snapshot(now.AddSeconds(-2), generation: 50),
            connections: 1, hostStartedUtc: now.AddHours(-1));
        var first = await source.SampleAsync(DashboardSampleKind.Fast, TestContext.Current.CancellationToken);
        await firstServer;

        var restartedServer = ServeAsync(pipeName, "1.0.0", Snapshot(now, generation: 1),
            connections: 1, hostStartedUtc: now.AddSeconds(-5));
        var restarted = await source.SampleAsync(DashboardSampleKind.Fast, TestContext.Current.CancellationToken);
        await restartedServer;

        Assert.True(first.IsFresh);
        Assert.True(restarted.IsFresh);
        Assert.Equal(1, restarted.Snapshot.Generation);
    }

    [Fact]
    public void Snapshot_exposes_only_explicitly_changed_metrics()
    {
        var now = DateTimeOffset.UtcNow;
        var metrics = new Dictionary<string, DashboardMetricSample>(StringComparer.OrdinalIgnoreCase)
        {
            ["cpu.percent"] = new("cpu.percent", 10, "%", now),
            ["storage.lowest_free_percent"] = new("storage.lowest_free_percent", 50, "%", now.AddSeconds(-30))
        };
        var snapshot = new DashboardSnapshotDelta(1, now, metrics, [], [],
            ChangedMetricKeys: ["cpu.percent"]);

        Assert.Equal("cpu.percent", Assert.Single(snapshot.GetChangedMetrics()).Key);
        Assert.False(snapshot.HasChangedMetric("storage.lowest_free_percent"));
    }

    private static async Task ServeAsync(
        string pipeName,
        string version,
        DashboardSnapshotDelta snapshot,
        int connections,
        DateTimeOffset? hostStartedUtc = null)
    {
        for (var index = 0; index < connections; index++)
        {
            await using var pipe = new NamedPipeServerStream(pipeName, PipeDirection.InOut, 1,
                PipeTransmissionMode.Byte, PipeOptions.Asynchronous | PipeOptions.CurrentUserOnly);
            await pipe.WaitForConnectionAsync(TestContext.Current.CancellationToken);
            var request = await DashboardMonitorProtocol.ReadAsync(pipe, TestContext.Current.CancellationToken);
            await DashboardMonitorProtocol.WriteAsync(pipe,
                new DashboardMonitorEnvelope(DashboardMonitorProtocol.CurrentVersion, version, "snapshot",
                    request.SampleKind, snapshot, Alerts: [],
                    HostStartedUtc: hostStartedUtc ?? snapshot.TimestampUtc.AddHours(-1),
                    LastSampleUtc: snapshot.TimestampUtc,
                    Capabilities: [DashboardMonitorProtocol.TelemetryOwnershipCapability]),
                TestContext.Current.CancellationToken);
        }
    }

    private static DashboardSnapshotDelta Snapshot(DateTimeOffset timestamp, long generation) => new(
        generation,
        timestamp,
        new Dictionary<string, DashboardMetricSample>(StringComparer.OrdinalIgnoreCase)
        {
            ["cpu.percent"] = new("cpu.percent", 20, "%", timestamp)
        },
        [],
        [],
        ChangedMetricKeys: ["cpu.percent"]);

    private sealed class FixtureTelemetrySource(DateTimeOffset timestamp) : IDashboardTelemetrySource
    {
        public int Calls { get; private set; }

        public Task<DashboardTelemetryRead> SampleAsync(
            DashboardSampleKind kind,
            CancellationToken cancellationToken = default)
        {
            Calls++;
            var snapshot = Snapshot(timestamp, Calls);
            return Task.FromResult(new DashboardTelemetryRead(snapshot, DashboardTelemetryOrigin.InProcess,
                true, true, false, false, "fixture"));
        }

        public void Dispose() { }
    }
}
