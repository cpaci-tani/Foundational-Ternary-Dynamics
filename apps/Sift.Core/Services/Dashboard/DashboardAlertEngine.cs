using Sift.Models;

namespace Sift.Services;

public interface IDashboardAlertEngine
{
    IReadOnlyList<DashboardAlert> Alerts { get; }
    Task InitializeAsync(CancellationToken cancellationToken = default);
    Task<IReadOnlyList<DashboardAlert>> EvaluateAsync(
        DashboardSnapshotDelta snapshot,
        DashboardPreferences preferences,
        CancellationToken cancellationToken = default);
    Task SynchronizeAsync(IReadOnlyList<DashboardAlert> alerts, CancellationToken cancellationToken = default);
    Task ResetAsync(CancellationToken cancellationToken = default);
    Task AcknowledgeAsync(string alertId, DateTimeOffset nowUtc, CancellationToken cancellationToken = default);
    Task SnoozeAsync(string alertId, DateTimeOffset untilUtc, CancellationToken cancellationToken = default);
}

public sealed class DashboardAlertEngine(IDashboardHistoryStore store) : IDashboardAlertEngine
{
    private sealed class RuleState
    {
        public DateTimeOffset? ConditionSinceUtc { get; set; }
        public DateTimeOffset? LastRaisedUtc { get; set; }
    }

    private readonly Dictionary<string, RuleState> _states = new(StringComparer.OrdinalIgnoreCase);
    private readonly List<DashboardAlert> _alerts = [];
    private readonly SemaphoreSlim _operations = new(1, 1);
    private readonly object _stateGate = new();
    private bool _initialized;

    public IReadOnlyList<DashboardAlert> Alerts
    {
        get { lock (_stateGate) return _alerts.ToList(); }
    }

    public async Task InitializeAsync(CancellationToken cancellationToken = default)
    {
        await _operations.WaitAsync(cancellationToken);
        try { await EnsureInitializedAsync(cancellationToken); }
        finally { _operations.Release(); }
    }

    public async Task<IReadOnlyList<DashboardAlert>> EvaluateAsync(
        DashboardSnapshotDelta snapshot,
        DashboardPreferences preferences,
        CancellationToken cancellationToken = default)
    {
        await _operations.WaitAsync(cancellationToken);
        try
        {
            await EnsureInitializedAsync(cancellationToken);
            var changed = new List<DashboardAlert>();
            foreach (var rule in preferences.AlertRules.Where(rule => rule.Enabled))
            {
                cancellationToken.ThrowIfCancellationRequested();
                if (!snapshot.HasChangedMetric(rule.MetricKey) ||
                    !snapshot.Metrics.TryGetValue(rule.MetricKey, out var metric)) continue;
                if (!_states.TryGetValue(rule.Id, out var state)) _states[rule.Id] = state = new RuleState();
                DashboardAlert? active;
                lock (_stateGate)
                {
                    active = _alerts.FirstOrDefault(alert =>
                        alert.RuleId.Equals(rule.Id, StringComparison.OrdinalIgnoreCase) && alert.ClearedUtc is null);
                }
                var triggered = rule.TriggerWhenBelow ? metric.Value < rule.Threshold : metric.Value > rule.Threshold;
                var cleared = rule.TriggerWhenBelow
                    ? metric.Value >= rule.Threshold + Math.Abs(rule.Hysteresis)
                    : metric.Value <= rule.Threshold - Math.Abs(rule.Hysteresis);

                if (active is not null)
                {
                    if (!cleared) continue;
                    var resolved = active with
                    {
                        Detail = $"{metric.Value:0.##} {metric.Unit} · cleared",
                        ClearedUtc = snapshot.TimestampUtc
                    };
                    Replace(active, resolved);
                    await store.UpsertAlertAsync(resolved, cancellationToken);
                    changed.Add(resolved);
                    state.ConditionSinceUtc = null;
                    continue;
                }

                if (!triggered)
                {
                    state.ConditionSinceUtc = null;
                    continue;
                }
                state.ConditionSinceUtc ??= snapshot.TimestampUtc;
                if (snapshot.TimestampUtc - state.ConditionSinceUtc < rule.RequiredDuration) continue;
                if (state.LastRaisedUtc is { } last && snapshot.TimestampUtc - last < rule.Cooldown) continue;

                var alert = new DashboardAlert(
                    $"{rule.Id}.{snapshot.TimestampUtc.ToUnixTimeSeconds()}",
                    rule.Id,
                    rule.MetricKey,
                    rule.Title,
                    $"{metric.Value:0.##} {metric.Unit} · threshold {rule.Threshold:0.##}",
                    rule.Severity,
                    snapshot.TimestampUtc,
                    null,
                    null,
                    null);
                lock (_stateGate)
                {
                    _alerts.Insert(0, alert);
                    if (_alerts.Count > 500) _alerts.RemoveRange(500, _alerts.Count - 500);
                }
                state.LastRaisedUtc = snapshot.TimestampUtc;
                await store.UpsertAlertAsync(alert, cancellationToken);
                changed.Add(alert);
            }
            return changed;
        }
        finally { _operations.Release(); }
    }

    public async Task SynchronizeAsync(
        IReadOnlyList<DashboardAlert> alerts,
        CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(alerts);
        await _operations.WaitAsync(cancellationToken);
        try
        {
            lock (_stateGate)
            {
                _alerts.Clear();
                _alerts.AddRange(alerts.Take(500));
                _states.Clear();
                _initialized = true;
            }
        }
        finally { _operations.Release(); }
    }

    public async Task ResetAsync(CancellationToken cancellationToken = default)
    {
        await _operations.WaitAsync(cancellationToken);
        try
        {
            lock (_stateGate)
            {
                _alerts.Clear();
                _states.Clear();
                _initialized = true;
            }
        }
        finally { _operations.Release(); }
    }

    public async Task AcknowledgeAsync(
        string alertId,
        DateTimeOffset nowUtc,
        CancellationToken cancellationToken = default)
    {
        await _operations.WaitAsync(cancellationToken);
        try
        {
            await EnsureInitializedAsync(cancellationToken);
            var alert = Find(alertId);
            var updated = alert with { AcknowledgedUtc = nowUtc };
            Replace(alert, updated);
            await store.UpsertAlertAsync(updated, cancellationToken);
        }
        finally { _operations.Release(); }
    }

    public async Task SnoozeAsync(
        string alertId,
        DateTimeOffset untilUtc,
        CancellationToken cancellationToken = default)
    {
        await _operations.WaitAsync(cancellationToken);
        try
        {
            await EnsureInitializedAsync(cancellationToken);
            var alert = Find(alertId);
            if (untilUtc <= DateTimeOffset.UtcNow) throw new ArgumentOutOfRangeException(nameof(untilUtc));
            var updated = alert with { SnoozedUntilUtc = untilUtc };
            Replace(alert, updated);
            await store.UpsertAlertAsync(updated, cancellationToken);
        }
        finally { _operations.Release(); }
    }

    private async Task EnsureInitializedAsync(CancellationToken cancellationToken)
    {
        if (_initialized) return;
        var loaded = await store.LoadAlertsAsync(cancellationToken);
        lock (_stateGate)
        {
            if (_initialized) return;
            _alerts.Clear();
            _alerts.AddRange(loaded);
            _initialized = true;
        }
    }

    private DashboardAlert Find(string alertId)
    {
        lock (_stateGate)
            return _alerts.FirstOrDefault(alert => alert.Id.Equals(alertId, StringComparison.OrdinalIgnoreCase))
                   ?? throw new KeyNotFoundException($"Dashboard alert '{alertId}' was not found.");
    }

    private void Replace(DashboardAlert current, DashboardAlert replacement)
    {
        lock (_stateGate)
        {
            var index = _alerts.IndexOf(current);
            if (index >= 0) _alerts[index] = replacement;
        }
    }
}

public static class DashboardAlertDefaults
{
    public static List<DashboardAlertRule> Create() =>
    [
        Rule("memory.high", "memory.percent", "Memory use is high", 90, below: false,
            TimeSpan.FromMinutes(10), 5, "Warning"),
        Rule("storage.low", "storage.lowest_free_percent", "A drive is running low on space", 10, below: true,
            TimeSpan.Zero, 2, "Warning"),
        Rule("storage.low.gb", "storage.lowest_free_gb", "A drive has less than 10 GB free", 10, below: true,
            TimeSpan.Zero, 2, "Warning"),
        Rule("temperature.high", "hardware.hottest_c", "Hardware temperature is high", 90, below: false,
            TimeSpan.FromMinutes(2), 5, "Critical"),
        Rule("battery.health", "battery.health_percent", "Battery health is below 70%", 70, below: true,
            TimeSpan.Zero, 3, "Warning"),
        Rule("health.failed", "health.failed", "A health check needs attention", 0, below: false,
            TimeSpan.Zero, 0, "Critical"),
        Rule("maintenance.overdue", "maintenance.latest_age_days", "Maintenance scan is due", 30, below: false,
            TimeSpan.Zero, 1, "Info"),
        Rule("recovery.old", "recovery.latest_age_days", "Recovery backup is older than 30 days", 30, below: false,
            TimeSpan.Zero, 1, "Info")
    ];

    private static DashboardAlertRule Rule(
        string id,
        string metric,
        string title,
        double threshold,
        bool below,
        TimeSpan duration,
        double hysteresis,
        string severity) => new()
    {
        Id = id,
        MetricKey = metric,
        Title = title,
        Threshold = threshold,
        TriggerWhenBelow = below,
        RequiredDuration = duration,
        Hysteresis = hysteresis,
        Severity = severity,
        ToastEnabled = false
    };
}

public static class DashboardNotificationPolicy
{
    public static bool ShouldDeliver(
        DashboardPreferences preferences,
        DashboardAlertRule rule,
        TimeOnly localTime)
    {
        if (!preferences.NotificationsEnabled || !rule.ToastEnabled) return false;
        var start = preferences.QuietHoursStart;
        var end = preferences.QuietHoursEnd;
        var quiet = start <= end ? localTime >= start && localTime < end : localTime >= start || localTime < end;
        return !quiet;
    }
}
