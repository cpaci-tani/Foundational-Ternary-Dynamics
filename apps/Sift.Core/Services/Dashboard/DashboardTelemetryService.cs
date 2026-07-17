using System.Diagnostics;
using System.Net.NetworkInformation;
using Sift.Models;

namespace Sift.Services;

public enum DashboardSampleKind
{
    Fast,
    Medium,
    Slow
}

public interface IDashboardTelemetrySource : IDisposable
{
    Task<DashboardTelemetryRead> SampleAsync(DashboardSampleKind kind, CancellationToken cancellationToken = default);
}

public sealed class DashboardTelemetryService(
    IProcessSampler processes,
    IHardwareMonitorService hardware,
    IServiceInventory services,
    IStartupInventory startup,
    IInstalledAppInventory installedApps,
    IRecoveryManager recovery,
    IHealthWorkspaceOrchestrator health,
    Func<bool>? includeHardware = null,
    Func<DateTimeOffset?>? lastMaintenanceScanUtc = null,
    IPdhSystemSampler? pdh = null) : IDashboardTelemetrySource, IDisposable
{
    private readonly NetworkRateSampler _network = new();
    private readonly IPdhSystemSampler _pdh = pdh ?? new PdhSystemSampler();
    private readonly bool _ownsPdh = pdh is null;
    private readonly object _cacheGate = new();
    private readonly Dictionary<string, DashboardMetricSample> _cache = new(StringComparer.OrdinalIgnoreCase);
    private IReadOnlyList<DashboardServiceSnapshot> _cachedServices = [];
    private long _generation;
    private bool _disposed;

    public Task<DashboardTelemetryRead> SampleAsync(
        DashboardSampleKind kind,
        CancellationToken cancellationToken = default) => kind == DashboardSampleKind.Slow
        ? SampleSlowAsync(cancellationToken)
        : Task.Run(() => SampleSynchronous(kind, cancellationToken), cancellationToken);

    private DashboardTelemetryRead SampleSynchronous(DashboardSampleKind kind, CancellationToken token)
    {
        token.ThrowIfCancellationRequested();
        var now = DateTimeOffset.UtcNow;
        Dictionary<string, DashboardMetricSample> metrics;
        IReadOnlyList<DashboardServiceSnapshot> serviceCache;
        lock (_cacheGate)
        {
            metrics = new Dictionary<string, DashboardMetricSample>(_cache, StringComparer.OrdinalIgnoreCase);
            serviceCache = _cachedServices;
        }
        var warnings = new List<string>();
        var serviceSnapshots = serviceCache.ToList();
        var sample = processes.Sample(token);
        Add(metrics, "cpu.percent", sample.CpuPercent, "%", now);
        Add(metrics, "memory.percent", sample.MemoryPercent, "%", now);
        Add(metrics, "memory.used_gb", sample.UsedMemoryGb, "GB", now);
        Add(metrics, "memory.total_gb", sample.TotalMemoryGb, "GB", now);
        Add(metrics, "system.uptime_hours", TimeSpan.FromMilliseconds(Environment.TickCount64).TotalHours, "h", now);

        TryCollect(warnings, "PDH", () =>
        {
            var counters = _pdh.Sample();
            if (counters is null) return;
            Add(metrics, "cpu.pdh_percent", counters.CpuPercent, "%", now);
            Add(metrics, "disk.read_mb_s", counters.DiskReadMbPerSec, "MiB/s", now);
            Add(metrics, "disk.write_mb_s", counters.DiskWriteMbPerSec, "MiB/s", now);
            // Prefer PDH CPU for the primary cpu.percent key when available (more accurate than process sum).
            Add(metrics, "cpu.percent", counters.CpuPercent, "%", now);
        });

        var network = _network.Sample(now);
        Add(metrics, "network.download_mbps", network.DownloadMbps, "Mbps", now);
        Add(metrics, "network.upload_mbps", network.UploadMbps, "Mbps", now);

        if ((includeHardware?.Invoke() ?? true)) AddHardware(metrics, now, warnings, token);

        if (kind == DashboardSampleKind.Medium || !metrics.ContainsKey("storage.volume_count"))
        {
            AddStorage(metrics, now, warnings);
            AddBattery(metrics, now);
        }

        if (kind == DashboardSampleKind.Medium)
        {
            TryCollect(warnings, "services", () =>
            {
                var rows = services.Enumerate();
                serviceSnapshots.Clear();
                serviceSnapshots.AddRange(rows.Take(40).Select(row => new DashboardServiceSnapshot(
                    row.Name, row.DisplayName, row.Status, row.IsProtected, row.CanManage)));
                Add(metrics, "services.total", rows.Count, "count", now);
                Add(metrics, "services.running", rows.Count(row => row.Status.Equals("Running", StringComparison.OrdinalIgnoreCase)), "count", now);
            });
            TryCollect(warnings, "startup", () =>
            {
                var rows = startup.Enumerate();
                Add(metrics, "startup.total", rows.Count, "count", now);
                Add(metrics, "startup.enabled", rows.Count(row => row.Status.Equals("Enabled", StringComparison.OrdinalIgnoreCase)), "count", now);
            });
        }

        UpdateCache(metrics, serviceSnapshots);

        var snapshot = new DashboardSnapshotDelta(
            Interlocked.Increment(ref _generation), now, metrics,
            sample.Processes.Take(12).Select(SanitizeProcess).ToList(), warnings, serviceSnapshots,
            metrics.Values.Where(metric => metric.TimestampUtc == now).Select(metric => metric.Key).ToList());
        return Local(snapshot);
    }

    private async Task<DashboardTelemetryRead> SampleSlowAsync(CancellationToken token)
    {
        var fastRead = await Task.Run(() => SampleSynchronous(DashboardSampleKind.Medium, token), token);
        var fast = fastRead.Snapshot;
        var metrics = new Dictionary<string, DashboardMetricSample>(fast.Metrics, StringComparer.OrdinalIgnoreCase);
        var warnings = fast.Warnings.ToList();
        var now = DateTimeOffset.UtcNow;
        await Task.Run(() =>
        {
            TryCollect(warnings, "installed apps", () =>
            {
                var apps = installedApps.Enumerate();
                Add(metrics, "apps.total", apps.Count, "count", now);
                Add(metrics, "apps.uninstallable", apps.Count(app => app.CanUninstall), "count", now);
                Add(metrics, "apps.leftovers", apps.Count(app => app.IsOrphanedRegistration), "count", now);
            });
            TryCollect(warnings, "recovery", () =>
            {
                var backups = recovery.ListBackups();
                Add(metrics, "recovery.backups", backups.Count, "count", now);
                var latest = backups.OrderByDescending(backup => backup.CreatedUtc).FirstOrDefault();
                if (latest is not null)
                    Add(metrics, "recovery.latest_age_days", Math.Max(0, (now.UtcDateTime - latest.CreatedUtc).TotalDays), "days", now);
                else Add(metrics, "recovery.latest_age_days", 999, "days", now);
            });
            var lastMaintenance = lastMaintenanceScanUtc?.Invoke();
            Add(metrics, "maintenance.latest_age_days",
                lastMaintenance is { } scan ? Math.Max(0, (now - scan).TotalDays) : 999, "days", now);
        }, token);

        try
        {
            var result = await health.RefreshAsync(token);
            if (!result.Stale)
            {
                Add(metrics, "health.warnings", result.Checks.Count(check => check.Status == HealthStatus.Warning), "count", now);
                Add(metrics, "health.critical", result.Checks.Count(check => check.Status == HealthStatus.Critical), "count", now);
                Add(metrics, "health.failed", result.Checks.Count(check => check.Status is HealthStatus.Warning or HealthStatus.Critical), "count", now);
                warnings.AddRange(result.Warnings);
            }
        }
        catch (Exception exception) when (exception is not OperationCanceledException)
        {
            warnings.Add($"Health summary is unavailable: {exception.Message}");
        }

        UpdateCache(metrics, fast.Services ?? []);
        var changed = (fast.ChangedMetricKeys ?? []).Concat(
                metrics.Values.Where(metric => metric.TimestampUtc == now).Select(metric => metric.Key))
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .ToList();
        return Local(new DashboardSnapshotDelta(
            Interlocked.Increment(ref _generation), now, metrics, fast.TopProcesses, warnings, fast.Services, changed));
    }

    private static DashboardTelemetryRead Local(DashboardSnapshotDelta snapshot) => new(
        snapshot,
        DashboardTelemetryOrigin.InProcess,
        IsFresh: true,
        SourceHealthy: true,
        SourceOwnsHistory: false,
        SourceOwnsAlerts: false,
        "Collected in process.");

    private void UpdateCache(
        IReadOnlyDictionary<string, DashboardMetricSample> metrics,
        IReadOnlyList<DashboardServiceSnapshot> services)
    {
        lock (_cacheGate)
        {
            foreach (var metric in metrics) _cache[metric.Key] = metric.Value;
            _cachedServices = services.ToList();
        }
    }

    private void AddHardware(
        IDictionary<string, DashboardMetricSample> metrics,
        DateTimeOffset now,
        ICollection<string> warnings,
        CancellationToken token)
    {
        try
        {
            var snapshot = hardware.Sample(token);
            var hottest = double.MinValue;
            foreach (var sensor in snapshot.Devices.SelectMany(device => device.Sensors))
            {
                if (!double.IsFinite(sensor.Value)) continue;
                Add(metrics, $"sensor.{NormalizeMetricSegment(sensor.Id)}", sensor.Value, sensor.Unit, now, sensor.Minimum, sensor.Maximum);
                if (sensor.Type.Equals("Temperature", StringComparison.OrdinalIgnoreCase)) hottest = Math.Max(hottest, sensor.Value);
            }
            if (hottest > double.MinValue) Add(metrics, "hardware.hottest_c", hottest, "°C", now);
            foreach (var provider in snapshot.Providers.Where(provider => !provider.Available))
                warnings.Add($"{provider.Name}: {provider.Detail}");
        }
        catch (Exception exception) when (exception is not OperationCanceledException)
        {
            warnings.Add($"Hardware sensors are unavailable: {exception.Message}");
        }
    }

    private static void AddStorage(
        IDictionary<string, DashboardMetricSample> metrics,
        DateTimeOffset now,
        ICollection<string> warnings)
    {
        try
        {
            var volumes = DriveInfo.GetDrives().Where(drive => drive.IsReady && drive.TotalSize > 0).ToList();
            var lowestPercent = 100d;
            var lowestGb = double.MaxValue;
            foreach (var drive in volumes)
            {
                var id = NormalizeMetricSegment(drive.Name.TrimEnd('\\'));
                var freeGb = drive.AvailableFreeSpace / 1073741824d;
                var freePercent = drive.AvailableFreeSpace * 100d / drive.TotalSize;
                Add(metrics, $"storage.{id}.free_gb", freeGb, "GB", now);
                Add(metrics, $"storage.{id}.free_percent", freePercent, "%", now);
                lowestPercent = Math.Min(lowestPercent, freePercent);
                lowestGb = Math.Min(lowestGb, freeGb);
            }
            Add(metrics, "storage.volume_count", volumes.Count, "count", now);
            if (volumes.Count > 0) Add(metrics, "storage.lowest_free_percent", lowestPercent, "%", now);
            if (volumes.Count > 0) Add(metrics, "storage.lowest_free_gb", lowestGb, "GB", now);
        }
        catch (Exception exception) { warnings.Add($"Storage capacity is unavailable: {exception.Message}"); }
    }

    private static void AddBattery(IDictionary<string, DashboardMetricSample> metrics, DateTimeOffset now)
    {
        var battery = BatteryReportReader.Read();
        if (!battery.Present) return;
        if (battery.ChargePercent is { } charge) Add(metrics, "battery.charge_percent", charge, "%", now);
        Add(metrics, "battery.on_ac", battery.OnAc ? 1 : 0, "boolean", now);
        Add(metrics, "power.battery_saver", battery.BatterySaver ? 1 : 0, "boolean", now);
        if (battery.RemainingMinutes is { } minutes) Add(metrics, "battery.remaining_minutes", minutes, "min", now);
        if (battery.HealthPercent is { } health) Add(metrics, "battery.health_percent", health, "%", now);
        if (battery.RemainingCapacityMwh is { } remaining) Add(metrics, "battery.remaining_mwh", remaining, "mWh", now);
        if (battery.FullChargeCapacityMwh is { } full) Add(metrics, "battery.full_charge_mwh", full, "mWh", now);
        if (battery.DesignCapacityMwh is { } design) Add(metrics, "battery.design_mwh", design, "mWh", now);
        if (battery.ChargeRateMw is { } rate) Add(metrics, "battery.charge_rate_mw", rate, "mW", now);
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        if (_ownsPdh) _pdh.Dispose();
    }

    private static void Add(
        IDictionary<string, DashboardMetricSample> metrics,
        string key,
        double value,
        string unit,
        DateTimeOffset timestamp,
        double? minimum = null,
        double? maximum = null)
    {
        if (!double.IsFinite(value)) return;
        metrics[key] = new DashboardMetricSample(key, value, unit, timestamp, minimum, maximum);
    }

    private static string NormalizeMetricSegment(string value) =>
        string.Concat(value.Select(character => char.IsLetterOrDigit(character) || character is '-' or '_' ? character : '_')).Trim('_');

    private static ProcessSnapshot SanitizeProcess(ProcessSnapshot process) => process with
    {
        WindowTitle = string.Empty,
        ExecutablePath = string.Empty,
        IconPng = null
    };

    private static void TryCollect(ICollection<string> warnings, string name, Action action)
    {
        try { action(); }
        catch (Exception exception) { warnings.Add($"{name} summary is unavailable: {exception.Message}"); }
    }

    private sealed class NetworkRateSampler
    {
        private readonly object _sync = new();
        private long _received;
        private long _sent;
        private long _timestamp;

        public (double DownloadMbps, double UploadMbps) Sample(DateTimeOffset now)
        {
            lock (_sync)
            {
                var received = 0L;
                var sent = 0L;
                foreach (var adapter in NetworkInterface.GetAllNetworkInterfaces().Where(adapter =>
                             adapter.OperationalStatus == OperationalStatus.Up &&
                             adapter.NetworkInterfaceType is not (NetworkInterfaceType.Loopback or NetworkInterfaceType.Tunnel)))
                {
                    try
                    {
                        var stats = adapter.GetIPv4Statistics();
                        received += stats.BytesReceived;
                        sent += stats.BytesSent;
                    }
                    catch { }
                }
                var timestamp = Stopwatch.GetTimestamp();
                var elapsed = _timestamp == 0 ? 0 : (timestamp - _timestamp) / (double)Stopwatch.Frequency;
                var download = elapsed > 0 && received >= _received ? (received - _received) * 8d / elapsed / 1_000_000d : 0;
                var upload = elapsed > 0 && sent >= _sent ? (sent - _sent) * 8d / elapsed / 1_000_000d : 0;
                _received = received;
                _sent = sent;
                _timestamp = timestamp;
                return (download, upload);
            }
        }
    }
}
