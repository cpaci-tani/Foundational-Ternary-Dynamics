using System.IO.Pipes;
using System.Reflection;
using Microsoft.Windows.AppNotifications;
using Microsoft.Windows.AppNotifications.Builder;
using Sift.Models;
using Sift.Services;

namespace Sift.MonitorHost;

internal static class Program
{
    [STAThread]
    private static int Main()
    {
        var version = Assembly.GetExecutingAssembly().GetName().Version?.ToString(3) ?? "0.0.0";
        using var mutex = new Mutex(initiallyOwned: true, $"Local\\Sift.MonitorHost.{DashboardMonitorProtocol.PipeName}", out var ownsMutex);
        if (!ownsMutex) return 0;

        using var runtime = new MonitorRuntime(version);
        using var tray = new TrayIcon(
            open: runtime.OpenSift,
            settings: runtime.OpenSettings,
            pauseHour: () => runtime.PauseUntil(DateTimeOffset.UtcNow.AddHours(1)),
            pause: runtime.Pause,
            resume: runtime.Resume,
            exit: runtime.Stop);
        runtime.Start();
        tray.RunMessageLoop(runtime.Token);
        runtime.Stop();
        return 0;
    }
}

internal sealed class MonitorRuntime : IDisposable
{
    private readonly string _version;
    private readonly CancellationTokenSource _shutdown = new();
    private readonly SettingsStore _settingsStore = new();
    private readonly DashboardHistoryStore _history = new();
    private readonly HardwareMonitorService _hardware = new();
    private readonly IDashboardTelemetrySource _telemetry;
    private readonly DashboardAlertEngine _alerts;
    private readonly DashboardNotificationSink _notifications;
    private readonly SemaphoreSlim _ownershipGate = new(1, 1);
    private DashboardSnapshotDelta? _latest;
    private AppSettings _settings;
    private long _pausedUntilUtcTicks;
    private long _lastSampleUtcTicks;
    private string? _lastSamplingError;
    private Task? _samplingTask;
    private Task? _serverTask;
    private readonly DashboardSamplingCoordinator _cadence = DashboardRuntimeFactory.CreateDefaultCadence();
    private DateTimeOffset _lastCompactionUtc;
    private readonly DateTimeOffset _startedUtc = DateTimeOffset.UtcNow;

    public MonitorRuntime(string version)
    {
        _version = version;
        _settings = LoadSettings();
        var slow = DashboardRuntimeFactory.CreateSlowSampleContext();
        var deps = DashboardRuntimeFactory.CreateDependencies(
            new ProcessSampler(), _hardware, new ServiceInventory(), new StartupInventory(), slow,
            includeHardware: () => Volatile.Read(ref _settings).Dashboard.BackgroundHardwareSensors,
            lastMaintenanceScanUtc: () => DateTimeOffset.TryParse(
                Volatile.Read(ref _settings).LastMaintenanceScanUtc, out var parsed)
                ? parsed
                : null);
        _telemetry = DashboardRuntimeFactory.CreateMonitorHostSampler(deps);
        _alerts = new DashboardAlertEngine(_history);
        _notifications = new DashboardNotificationSink(OpenSift);
    }

    public CancellationToken Token => _shutdown.Token;

    public void Start()
    {
        _samplingTask = Task.Run(() => SamplingLoopAsync(_shutdown.Token));
        _serverTask = Task.Run(() => ServerLoopAsync(_shutdown.Token));
    }

    public void Pause() => Interlocked.Exchange(ref _pausedUntilUtcTicks, long.MaxValue);
    public void PauseUntil(DateTimeOffset untilUtc) =>
        Interlocked.Exchange(ref _pausedUntilUtcTicks, untilUtc.UtcTicks);
    public void Resume() => Interlocked.Exchange(ref _pausedUntilUtcTicks, 0);
    public void Stop() => _shutdown.Cancel();

    public void OpenSift()
        => OpenSiftTo("Home");

    public void OpenSettings()
        => OpenSiftTo("Settings");

    private void OpenSiftTo(string workspace)
    {
        try
        {
            var root = Directory.GetParent(AppContext.BaseDirectory.TrimEnd(Path.DirectorySeparatorChar))?.FullName;
            var executable = root is null ? string.Empty : Path.Combine(root, "Sift.exe");
            if (!File.Exists(executable)) executable = Path.Combine(AppContext.BaseDirectory, "Sift.exe");
            if (File.Exists(executable) &&
                BinaryTrustPolicy.HaveSameTrustedSigner(Environment.ProcessPath, executable, out _))
                System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(executable, $"--open={workspace}") { UseShellExecute = true });
        }
        catch { }
    }

    public void Dispose()
    {
        Stop();
        var tasks = new[] { _samplingTask ?? Task.CompletedTask, _serverTask ?? Task.CompletedTask };
        var stopped = false;
        try
        {
            stopped = Task.WaitAll(tasks, TimeSpan.FromSeconds(10));
        }
        catch { stopped = tasks.All(task => task.IsCompleted); }
        _notifications.Dispose();
        if (stopped)
        {
            _telemetry.Dispose();
            _hardware.Dispose();
            _history.Dispose();
            _ownershipGate.Dispose();
            _shutdown.Dispose();
        }
    }

    private async Task SamplingLoopAsync(CancellationToken token)
    {
        while (!token.IsCancellationRequested)
        {
            try
            {
                ExpirePauseIfNeeded();
                if (!IsPaused)
                {
                    var kind = _cadence.Next(DateTimeOffset.UtcNow);
                    var read = await _telemetry.SampleAsync(kind, token);
                    var snapshot = read.Snapshot;
                    Volatile.Write(ref _latest, snapshot);
                    Interlocked.Exchange(ref _lastSampleUtcTicks, snapshot.TimestampUtc.UtcTicks);
                    await _ownershipGate.WaitAsync(token);
                    try
                    {
                        if (Volatile.Read(ref _lastSamplingError) is not null)
                            await _alerts.SynchronizeAsync(await _history.LoadAlertsAsync(token), token);
                        await _history.AppendAsync(snapshot.GetChangedMetrics(), token);
                        var settings = Volatile.Read(ref _settings);
                        var changed = await _alerts.EvaluateAsync(snapshot, settings.Dashboard, token);
                        foreach (var alert in changed.Where(alert => alert.ClearedUtc is null))
                        {
                            var rule = settings.Dashboard.AlertRules.FirstOrDefault(value => value.Id == alert.RuleId);
                            if (rule is not null && DashboardNotificationPolicy.ShouldDeliver(
                                    settings.Dashboard, rule, TimeOnly.FromDateTime(DateTime.Now)))
                                _notifications.Show(alert);
                        }
                        if (DateTimeOffset.UtcNow - _lastCompactionUtc > TimeSpan.FromHours(12))
                        {
                            Volatile.Write(ref _settings, LoadSettings());
                            settings = Volatile.Read(ref _settings);
                            await _history.CompactAsync(DateTimeOffset.UtcNow,
                                Math.Clamp(settings.Dashboard.HistoryRetentionDays, 7, 365), token);
                            _lastCompactionUtc = DateTimeOffset.UtcNow;
                        }
                        Volatile.Write(ref _lastSamplingError, null);
                    }
                    finally { _ownershipGate.Release(); }
                }
                var batterySaver = Volatile.Read(ref _latest)?.Metrics.TryGetValue("power.battery_saver", out var saver) == true && saver.Value > 0;
                await Task.Delay(DashboardSamplingCoordinator.Delay(batterySaver), token);
            }
            catch (OperationCanceledException) when (token.IsCancellationRequested) { break; }
            catch (Exception exception)
            {
                Volatile.Write(ref _lastSamplingError, exception.Message);
                await Task.Delay(TimeSpan.FromSeconds(5), token);
            }
        }
    }

    private async Task ServerLoopAsync(CancellationToken token)
    {
        while (!token.IsCancellationRequested)
        {
            try
            {
                await using var pipe = new NamedPipeServerStream(
                    DashboardMonitorProtocol.PipeName, PipeDirection.InOut, 1,
                    PipeTransmissionMode.Byte, PipeOptions.Asynchronous | PipeOptions.CurrentUserOnly);
                await pipe.WaitForConnectionAsync(token);
                using var requestDeadline = CancellationTokenSource.CreateLinkedTokenSource(token);
                requestDeadline.CancelAfter(TimeSpan.FromSeconds(2));
                var request = await DashboardMonitorProtocol.ReadAsync(pipe, requestDeadline.Token);
                var response = await HandleRequestAsync(request, requestDeadline.Token);
                await DashboardMonitorProtocol.WriteAsync(pipe, response, requestDeadline.Token);
            }
            catch (OperationCanceledException) when (token.IsCancellationRequested) { break; }
            catch { }
        }
    }

    private DashboardMonitorEnvelope PauseResponse()
    {
        Pause();
        return Status("paused");
    }

    private DashboardMonitorEnvelope ResumeResponse()
    {
        Resume();
        return Status("running");
    }

    private DashboardMonitorEnvelope ReloadResponse()
    {
        Volatile.Write(ref _settings, LoadSettings());
        return Status("reloaded");
    }

    private DashboardMonitorEnvelope ShutdownResponse()
    {
        _ = Task.Run(async () =>
        {
            await Task.Delay(150);
            Stop();
        });
        return Status("stopping");
    }

    private AppSettings LoadSettings()
    {
        var settings = _settingsStore.Load();
        settings.Dashboard ??= new DashboardPreferences();
        if (settings.Dashboard.AlertRules.Count == 0) settings.Dashboard.AlertRules = DashboardAlertDefaults.Create();
        return settings;
    }

    private bool IsPaused => Interlocked.Read(ref _pausedUntilUtcTicks) != 0;

    private void ExpirePauseIfNeeded()
    {
        var pausedUntilTicks = Interlocked.Read(ref _pausedUntilUtcTicks);
        if (pausedUntilTicks is 0 or long.MaxValue || pausedUntilTicks > DateTimeOffset.UtcNow.UtcTicks) return;
        Interlocked.CompareExchange(ref _pausedUntilUtcTicks, 0, pausedUntilTicks);
    }

    private async Task<DashboardMonitorEnvelope> HandleRequestAsync(
        DashboardMonitorEnvelope request,
        CancellationToken token)
    {
        if (!request.AppVersion.Equals(_version, StringComparison.OrdinalIgnoreCase))
            return new DashboardMonitorEnvelope(DashboardMonitorProtocol.CurrentVersion, _version,
                "version-mismatch", Message: "Sift and the monitor host must have the same version.",
                HostStartedUtc: _startedUtc, LastSampleUtc: LastSampleUtc(), Capabilities: Capabilities());

        return request.Command switch
        {
            "snapshot" => SnapshotResponse(request.SampleKind),
            "pause" => PauseResponse(),
            "resume" => ResumeResponse(),
            "reload" => ReloadResponse(),
            "shutdown" => ShutdownResponse(),
            "status" => Status(IsPaused ? "paused" : Volatile.Read(ref _lastSamplingError) is { } error
                ? $"degraded: {error}"
                : "running"),
            "acknowledge-alert" => await UpdateAlertAsync(request, snooze: false, token),
            "snooze-alert" => await UpdateAlertAsync(request, snooze: true, token),
            "clear-history" => await ClearHistoryAsync(token),
            _ => new DashboardMonitorEnvelope(DashboardMonitorProtocol.CurrentVersion, _version,
                "unsupported", Message: "This monitor command is not supported.",
                HostStartedUtc: _startedUtc, LastSampleUtc: LastSampleUtc(), Capabilities: Capabilities())
        };
    }

    private DashboardMonitorEnvelope SnapshotResponse(DashboardSampleKind? sampleKind)
    {
        var latest = Volatile.Read(ref _latest);
        return new DashboardMonitorEnvelope(DashboardMonitorProtocol.CurrentVersion, _version,
            "snapshot", sampleKind, latest,
            Message: Volatile.Read(ref _lastSamplingError),
            Paused: IsPaused,
            Alerts: _alerts.Alerts,
            HostStartedUtc: _startedUtc,
            LastSampleUtc: LastSampleUtc(),
            Capabilities: Capabilities());
    }

    private DashboardMonitorEnvelope Status(string message) =>
        new(DashboardMonitorProtocol.CurrentVersion, _version, "status", Message: message,
            Paused: IsPaused, HostStartedUtc: _startedUtc, LastSampleUtc: LastSampleUtc(),
            Capabilities: Capabilities());

    private static IReadOnlyList<string> Capabilities() =>
        [DashboardMonitorProtocol.TelemetryOwnershipCapability];

    private DateTimeOffset? LastSampleUtc()
    {
        var ticks = Interlocked.Read(ref _lastSampleUtcTicks);
        return ticks == 0 ? null : new DateTimeOffset(ticks, TimeSpan.Zero);
    }

    private async Task<DashboardMonitorEnvelope> UpdateAlertAsync(
        DashboardMonitorEnvelope request,
        bool snooze,
        CancellationToken token)
    {
        if (string.IsNullOrWhiteSpace(request.AlertId) || request.AlertTimestampUtc is not { } timestamp)
            return Status("invalid-alert-request");
        try
        {
            await _ownershipGate.WaitAsync(token);
            try
            {
                if (snooze) await _alerts.SnoozeAsync(request.AlertId, timestamp, token);
                else await _alerts.AcknowledgeAsync(request.AlertId, timestamp, token);
            }
            finally { _ownershipGate.Release(); }
            return Status("updated");
        }
        catch (Exception exception) when (exception is KeyNotFoundException or ArgumentOutOfRangeException)
        {
            return Status("alert-unavailable");
        }
    }

    private async Task<DashboardMonitorEnvelope> ClearHistoryAsync(CancellationToken token)
    {
        await _ownershipGate.WaitAsync(token);
        try
        {
            await _history.ClearAsync(token);
            await _alerts.ResetAsync(token);
        }
        finally { _ownershipGate.Release(); }
        return Status("cleared");
    }

}

internal sealed class DashboardNotificationSink : IDisposable
{
    private readonly AppNotificationManager _manager = AppNotificationManager.Default;
    private readonly Action _open;
    private bool _registered;

    public DashboardNotificationSink(Action open)
    {
        _open = open;
        try
        {
            _manager.NotificationInvoked += Manager_NotificationInvoked;
            _manager.Register();
            _registered = true;
        }
        catch { }
    }

    public void Show(DashboardAlert alert)
    {
        if (!_registered) return;
        try
        {
            var notification = new AppNotificationBuilder()
                .AddArgument("alert", alert.Id)
                .AddText(alert.Title)
                .AddText(alert.Detail)
                .BuildNotification();
            notification.Tag = alert.RuleId.Length <= 16 ? alert.RuleId : alert.RuleId[..16];
            notification.Group = "SiftDashboard";
            _manager.Show(notification);
        }
        catch { }
    }

    public void Dispose()
    {
        if (!_registered) return;
        try
        {
            _manager.NotificationInvoked -= Manager_NotificationInvoked;
            _manager.Unregister();
        }
        catch { }
        _registered = false;
    }

    private void Manager_NotificationInvoked(AppNotificationManager sender, AppNotificationActivatedEventArgs args) => _open();
}
