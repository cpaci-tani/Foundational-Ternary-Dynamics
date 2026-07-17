using Microsoft.Data.Sqlite;
using Sift.Models;

namespace Sift.Services;

public interface IDashboardHistoryStore : IDisposable
{
    string DatabasePath { get; }
    Task AppendAsync(IEnumerable<DashboardMetricSample> samples, CancellationToken cancellationToken = default);
    Task<IReadOnlyList<DashboardHistoryPoint>> QueryAsync(
        string metricKey, DateTimeOffset fromUtc, DateTimeOffset toUtc, CancellationToken cancellationToken = default);
    Task CompactAsync(DateTimeOffset nowUtc, int retentionDays = 90, CancellationToken cancellationToken = default);
    Task<IReadOnlyList<DashboardAlert>> LoadAlertsAsync(CancellationToken cancellationToken = default);
    Task UpsertAlertAsync(DashboardAlert alert, CancellationToken cancellationToken = default);
    Task ClearAsync(CancellationToken cancellationToken = default);
}

public sealed class DashboardHistoryStore : IDashboardHistoryStore
{
    private const int CurrentSchemaVersion = 1;
    private const int MaximumStoredAlerts = 500;
    private readonly SemaphoreSlim _gate = new(1, 1);
    private readonly object _pendingGate = new();
    private readonly Dictionary<(string Key, long Bucket), PendingAggregate> _pending = new();
    private bool _initialized;
    private bool _disposed;

    public DashboardHistoryStore(string? directory = null)
    {
        var root = directory ?? ProductPaths.DataRoot;
        Directory.CreateDirectory(root);
        DatabasePath = Path.Combine(root, "dashboard.db");
    }

    public string DatabasePath { get; }

    public async Task AppendAsync(IEnumerable<DashboardMetricSample> samples, CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(samples);
        var accepted = samples.Where(sample => DashboardMetricPolicy.IsPersistable(sample.Key) && double.IsFinite(sample.Value)).ToList();
        if (accepted.Count == 0) return;
        List<PendingAggregate> ready;
        lock (_pendingGate)
        {
            foreach (var sample in accepted)
            {
                var bucket = sample.TimestampUtc.ToUnixTimeSeconds() / 60 * 60;
                var id = (sample.Key, bucket);
                if (!_pending.TryGetValue(id, out var aggregate))
                    _pending[id] = aggregate = new PendingAggregate(sample.Key, bucket, sample.Value, sample.Value, 0, 0);
                aggregate.Minimum = Math.Min(aggregate.Minimum, sample.Value);
                aggregate.Maximum = Math.Max(aggregate.Maximum, sample.Value);
                aggregate.Total += sample.Value;
                aggregate.Count++;
            }
            var newestBucket = accepted.Max(sample => sample.TimestampUtc.ToUnixTimeSeconds() / 60 * 60);
            ready = _pending.Where(pair => pair.Key.Bucket < newestBucket).Select(pair => pair.Value).ToList();
            foreach (var aggregate in ready) _pending.Remove((aggregate.Key, aggregate.Bucket));
        }
        if (ready.Count > 0)
        {
            try { await WriteAggregatesAsync(ready, cancellationToken); }
            catch
            {
                Requeue(ready);
                throw;
            }
        }
    }

    public Task<IReadOnlyList<DashboardHistoryPoint>> QueryAsync(
        string metricKey,
        DateTimeOffset fromUtc,
        DateTimeOffset toUtc,
        CancellationToken cancellationToken = default)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(metricKey);
        if (!DashboardMetricPolicy.IsPersistable(metricKey)) throw new ArgumentException("Metric history is not supported.", nameof(metricKey));
        return QueryFlushedAsync();

        async Task<IReadOnlyList<DashboardHistoryPoint>> QueryFlushedAsync()
        {
            await FlushPendingAsync(cancellationToken);
            return await WithConnectionAsync<IReadOnlyList<DashboardHistoryPoint>>(async connection =>
            {
                var rows = new List<DashboardHistoryPoint>();
                await using var command = connection.CreateCommand();
                command.CommandText = """
                SELECT metric_key, bucket_utc, resolution_seconds, minimum, maximum, total, sample_count
                FROM metric_buckets
                WHERE metric_key = $key AND bucket_utc BETWEEN $from AND $to
                ORDER BY bucket_utc, resolution_seconds;
                """;
                command.Parameters.AddWithValue("$key", metricKey);
                command.Parameters.AddWithValue("$from", fromUtc.ToUnixTimeSeconds());
                command.Parameters.AddWithValue("$to", toUtc.ToUnixTimeSeconds());
                await using var reader = await command.ExecuteReaderAsync(cancellationToken);
                while (await reader.ReadAsync(cancellationToken))
                {
                    var count = reader.GetInt64(6);
                    rows.Add(new DashboardHistoryPoint(
                        reader.GetString(0),
                        DateTimeOffset.FromUnixTimeSeconds(reader.GetInt64(1)),
                        TimeSpan.FromSeconds(reader.GetInt32(2)),
                        reader.GetDouble(3),
                        reader.GetDouble(4),
                        count == 0 ? 0 : reader.GetDouble(5) / count,
                        count));
                }
                return rows;
            }, cancellationToken);
        }
    }

    public Task CompactAsync(DateTimeOffset nowUtc, int retentionDays = 90, CancellationToken cancellationToken = default)
    {
        if (retentionDays is < 7 or > 365) throw new ArgumentOutOfRangeException(nameof(retentionDays));
        return WithConnectionAsync(async connection =>
        {
            var minuteCutoff = nowUtc.AddDays(-7).ToUnixTimeSeconds();
            var retentionCutoff = nowUtc.AddDays(-retentionDays).ToUnixTimeSeconds();
            await using var transaction = (SqliteTransaction)await connection.BeginTransactionAsync(cancellationToken);
            await using (var rollup = connection.CreateCommand())
            {
                rollup.Transaction = transaction;
                rollup.CommandText = """
                    INSERT INTO metric_buckets(metric_key, bucket_utc, resolution_seconds, minimum, maximum, total, sample_count)
                    SELECT metric_key, (bucket_utc / 900) * 900, 900, MIN(minimum), MAX(maximum), SUM(total), SUM(sample_count)
                    FROM metric_buckets
                    WHERE resolution_seconds = 60 AND bucket_utc < $minuteCutoff AND bucket_utc >= $retentionCutoff
                    GROUP BY metric_key, (bucket_utc / 900) * 900
                    ON CONFLICT(metric_key, bucket_utc, resolution_seconds) DO UPDATE SET
                      minimum = MIN(minimum, excluded.minimum),
                      maximum = MAX(maximum, excluded.maximum),
                      total = total + excluded.total,
                      sample_count = sample_count + excluded.sample_count;
                    """;
                rollup.Parameters.AddWithValue("$minuteCutoff", minuteCutoff);
                rollup.Parameters.AddWithValue("$retentionCutoff", retentionCutoff);
                await rollup.ExecuteNonQueryAsync(cancellationToken);
            }
            await using (var prune = connection.CreateCommand())
            {
                prune.Transaction = transaction;
                prune.CommandText = """
                    DELETE FROM metric_buckets
                    WHERE bucket_utc < $retentionCutoff OR (resolution_seconds = 60 AND bucket_utc < $minuteCutoff);
                    """;
                prune.Parameters.AddWithValue("$minuteCutoff", minuteCutoff);
                prune.Parameters.AddWithValue("$retentionCutoff", retentionCutoff);
                await prune.ExecuteNonQueryAsync(cancellationToken);
            }
            await transaction.CommitAsync(cancellationToken);
        }, cancellationToken);
    }

    public Task ClearAsync(CancellationToken cancellationToken = default)
    {
        lock (_pendingGate) _pending.Clear();
        return WithConnectionAsync(async connection =>
        {
            await using var command = connection.CreateCommand();
            command.CommandText = "DELETE FROM metric_buckets; DELETE FROM alerts;";
            await command.ExecuteNonQueryAsync(cancellationToken);
        }, cancellationToken);
    }

    public Task<IReadOnlyList<DashboardAlert>> LoadAlertsAsync(CancellationToken cancellationToken = default) =>
        WithConnectionAsync<IReadOnlyList<DashboardAlert>>(async connection =>
        {
            var alerts = new List<DashboardAlert>();
            await using var command = connection.CreateCommand();
            command.CommandText = """
                SELECT id, rule_id, metric_key, title, detail, severity, raised_utc,
                       cleared_utc, acknowledged_utc, snoozed_until_utc
                FROM alerts ORDER BY raised_utc DESC LIMIT 500;
                """;
            await using var reader = await command.ExecuteReaderAsync(cancellationToken);
            while (await reader.ReadAsync(cancellationToken))
            {
                alerts.Add(new DashboardAlert(
                    reader.GetString(0), reader.GetString(1), reader.GetString(2), reader.GetString(3),
                    reader.GetString(4), reader.GetString(5), DateTimeOffset.FromUnixTimeSeconds(reader.GetInt64(6)),
                    ReadTimestamp(reader, 7), ReadTimestamp(reader, 8), ReadTimestamp(reader, 9)));
            }
            return alerts;
        }, cancellationToken);

    public Task UpsertAlertAsync(DashboardAlert alert, CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(alert);
        return WithConnectionAsync(async connection =>
        {
            await using var command = connection.CreateCommand();
            command.CommandText = """
                INSERT INTO alerts(id, rule_id, metric_key, title, detail, severity, raised_utc,
                                   cleared_utc, acknowledged_utc, snoozed_until_utc)
                VALUES($id, $rule, $metric, $title, $detail, $severity, $raised, $cleared, $acknowledged, $snoozed)
                ON CONFLICT(id) DO UPDATE SET
                  detail = excluded.detail, severity = excluded.severity,
                  cleared_utc = excluded.cleared_utc,
                  acknowledged_utc = excluded.acknowledged_utc,
                  snoozed_until_utc = excluded.snoozed_until_utc;
                """;
            command.Parameters.AddWithValue("$id", alert.Id);
            command.Parameters.AddWithValue("$rule", alert.RuleId);
            command.Parameters.AddWithValue("$metric", alert.MetricKey);
            command.Parameters.AddWithValue("$title", alert.Title);
            command.Parameters.AddWithValue("$detail", alert.Detail);
            command.Parameters.AddWithValue("$severity", alert.Severity);
            command.Parameters.AddWithValue("$raised", alert.RaisedUtc.ToUnixTimeSeconds());
            command.Parameters.AddWithValue("$cleared", TimestampValue(alert.ClearedUtc));
            command.Parameters.AddWithValue("$acknowledged", TimestampValue(alert.AcknowledgedUtc));
            command.Parameters.AddWithValue("$snoozed", TimestampValue(alert.SnoozedUntilUtc));
            await command.ExecuteNonQueryAsync(cancellationToken);
            await using var prune = connection.CreateCommand();
            prune.CommandText = """
                DELETE FROM alerts
                WHERE id IN (
                    SELECT id FROM alerts ORDER BY raised_utc DESC, id DESC LIMIT -1 OFFSET $maximum);
                """;
            prune.Parameters.AddWithValue("$maximum", MaximumStoredAlerts);
            await prune.ExecuteNonQueryAsync(cancellationToken);
        }, cancellationToken);
    }

    public void Dispose()
    {
        if (_disposed) return;
        try { FlushPendingAsync(CancellationToken.None).GetAwaiter().GetResult(); }
        catch (Exception exception) { System.Diagnostics.Debug.WriteLine(exception); }
        // Drain any in-flight operation by taking the gate before disposing it, so a concurrent
        // writer cannot be handed an ObjectDisposedException from a half-disposed semaphore. Mark
        // disposed while holding the gate so new callers observe it and fail cleanly instead.
        var held = false;
        try { held = _gate.Wait(TimeSpan.FromSeconds(5)); }
        catch (Exception exception) { System.Diagnostics.Debug.WriteLine(exception); }
        _disposed = true;
        if (held)
        {
            try { _gate.Release(); }
            catch (Exception exception) { System.Diagnostics.Debug.WriteLine(exception); }
        }
        _gate.Dispose();
    }

    private Task FlushPendingAsync(CancellationToken token)
    {
        List<PendingAggregate> pending;
        lock (_pendingGate)
        {
            pending = _pending.Values.ToList();
            _pending.Clear();
        }
        return pending.Count == 0 ? Task.CompletedTask : FlushCapturedAsync(pending, token);

        async Task FlushCapturedAsync(IReadOnlyList<PendingAggregate> captured, CancellationToken cancellationToken)
        {
            try { await WriteAggregatesAsync(captured, cancellationToken); }
            catch
            {
                Requeue(captured);
                throw;
            }
        }
    }

    private Task WriteAggregatesAsync(IReadOnlyList<PendingAggregate> aggregates, CancellationToken token) =>
        WithConnectionAsync(async connection =>
        {
            await using var transaction = (SqliteTransaction)await connection.BeginTransactionAsync(token);
            foreach (var aggregate in aggregates)
            {
                await using var command = connection.CreateCommand();
                command.Transaction = transaction;
                command.CommandText = """
                    INSERT INTO metric_buckets(metric_key, bucket_utc, resolution_seconds, minimum, maximum, total, sample_count)
                    VALUES($key, $bucket, 60, $minimum, $maximum, $total, $count)
                    ON CONFLICT(metric_key, bucket_utc, resolution_seconds) DO UPDATE SET
                      minimum = MIN(minimum, excluded.minimum),
                      maximum = MAX(maximum, excluded.maximum),
                      total = total + excluded.total,
                      sample_count = sample_count + excluded.sample_count;
                    """;
                command.Parameters.AddWithValue("$key", aggregate.Key);
                command.Parameters.AddWithValue("$bucket", aggregate.Bucket);
                command.Parameters.AddWithValue("$minimum", aggregate.Minimum);
                command.Parameters.AddWithValue("$maximum", aggregate.Maximum);
                command.Parameters.AddWithValue("$total", aggregate.Total);
                command.Parameters.AddWithValue("$count", aggregate.Count);
                await command.ExecuteNonQueryAsync(token);
            }
            await transaction.CommitAsync(token);
        }, token);

    private sealed class PendingAggregate(
        string key, long bucket, double minimum, double maximum, double total, long count)
    {
        public string Key { get; } = key;
        public long Bucket { get; } = bucket;
        public double Minimum { get; set; } = minimum;
        public double Maximum { get; set; } = maximum;
        public double Total { get; set; } = total;
        public long Count { get; set; } = count;

        public void Merge(PendingAggregate other)
        {
            Minimum = Math.Min(Minimum, other.Minimum);
            Maximum = Math.Max(Maximum, other.Maximum);
            Total += other.Total;
            Count += other.Count;
        }
    }

    private void Requeue(IEnumerable<PendingAggregate> aggregates)
    {
        lock (_pendingGate)
        {
            foreach (var aggregate in aggregates)
            {
                var id = (aggregate.Key, aggregate.Bucket);
                if (_pending.TryGetValue(id, out var existing)) existing.Merge(aggregate);
                else _pending[id] = aggregate;
            }
        }
    }

    private async Task WithConnectionAsync(Func<SqliteConnection, Task> action, CancellationToken token)
    {
        await WithConnectionAsync<object?>(async connection => { await action(connection); return null; }, token);
    }

    private async Task<T> WithConnectionAsync<T>(Func<SqliteConnection, Task<T>> action, CancellationToken token)
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        try { await _gate.WaitAsync(token); }
        catch (ObjectDisposedException) { throw new ObjectDisposedException(nameof(DashboardHistoryStore)); }
        try
        {
            await EnsureInitializedAsync(token);
            await using var connection = OpenConnection();
            await connection.OpenAsync(token);
            await ConfigureConnectionAsync(connection, token);
            return await action(connection);
        }
        catch (SqliteException exception) when (exception.SqliteErrorCode is 11 or 26)
        {
            QuarantineDatabase();
            _initialized = false;
            lock (_pendingGate) _pending.Clear();
            try
            {
                await EnsureInitializedAsync(token);
                await using var recovered = OpenConnection();
                await recovered.OpenAsync(token);
                await ConfigureConnectionAsync(recovered, token);
                return await action(recovered);
            }
            catch (Exception recoveryException)
            {
                throw new InvalidDataException("Dashboard history was corrupt and could not be recreated.",
                    new AggregateException(exception, recoveryException));
            }
        }
        finally { _gate.Release(); }
    }

    private async Task EnsureInitializedAsync(CancellationToken token)
    {
        if (_initialized) return;
        await using var connection = OpenConnection();
        await connection.OpenAsync(token);
        await using (var configure = connection.CreateCommand())
        {
            configure.CommandText = """
            PRAGMA journal_mode=WAL;
            PRAGMA synchronous=NORMAL;
            PRAGMA busy_timeout=5000;
            """;
            await configure.ExecuteNonQueryAsync(token);
        }
        await using var versionCommand = connection.CreateCommand();
        versionCommand.CommandText = "PRAGMA user_version;";
        var version = Convert.ToInt32(await versionCommand.ExecuteScalarAsync(token),
            System.Globalization.CultureInfo.InvariantCulture);
        if (version > CurrentSchemaVersion)
            throw new InvalidDataException($"Dashboard history schema {version} is newer than this Sift release supports.");
        if (version < 0) throw new InvalidDataException("Dashboard history schema is invalid.");

        await using var command = connection.CreateCommand();
        command.CommandText = """
            CREATE TABLE IF NOT EXISTS metric_buckets(
              metric_key TEXT NOT NULL,
              bucket_utc INTEGER NOT NULL,
              resolution_seconds INTEGER NOT NULL,
              minimum REAL NOT NULL,
              maximum REAL NOT NULL,
              total REAL NOT NULL,
              sample_count INTEGER NOT NULL,
              PRIMARY KEY(metric_key, bucket_utc, resolution_seconds));
            CREATE TABLE IF NOT EXISTS alerts(
              id TEXT PRIMARY KEY,
              rule_id TEXT NOT NULL,
              metric_key TEXT NOT NULL,
              title TEXT NOT NULL,
              detail TEXT NOT NULL,
              severity TEXT NOT NULL,
              raised_utc INTEGER NOT NULL,
              cleared_utc INTEGER,
              acknowledged_utc INTEGER,
              snoozed_until_utc INTEGER);
            """ + (version == 0 ? $"{Environment.NewLine}PRAGMA user_version={CurrentSchemaVersion};" : string.Empty);
        await command.ExecuteNonQueryAsync(token);
        _initialized = true;
    }

    // busy_timeout is a per-connection setting; EnsureInitializedAsync configures only its own
    // throwaway connection, so every per-operation connection must set it too or a concurrent WAL
    // writer (e.g. MonitorHost) can surface SQLITE_BUSY before the intended wait elapses.
    private static async Task ConfigureConnectionAsync(SqliteConnection connection, CancellationToken token)
    {
        await using var pragma = connection.CreateCommand();
        pragma.CommandText = "PRAGMA busy_timeout=5000;";
        await pragma.ExecuteNonQueryAsync(token);
    }

    private SqliteConnection OpenConnection() => new(new SqliteConnectionStringBuilder
    {
        DataSource = DatabasePath,
        Mode = SqliteOpenMode.ReadWriteCreate,
        Cache = SqliteCacheMode.Shared,
        Pooling = false,
        DefaultTimeout = 5
    }.ToString());

    private static object TimestampValue(DateTimeOffset? value) =>
        value is { } timestamp ? timestamp.ToUnixTimeSeconds() : DBNull.Value;

    private static DateTimeOffset? ReadTimestamp(SqliteDataReader reader, int ordinal) =>
        reader.IsDBNull(ordinal) ? null : DateTimeOffset.FromUnixTimeSeconds(reader.GetInt64(ordinal));

    private void QuarantineDatabase()
    {
        try
        {
            foreach (var suffix in new[] { string.Empty, "-wal", "-shm" })
            {
                var path = DatabasePath + suffix;
                if (!File.Exists(path)) continue;
                File.Move(path, Path.Combine(Path.GetDirectoryName(DatabasePath)!,
                    $"dashboard.corrupt-{DateTime.UtcNow:yyyyMMddHHmmssfff}{suffix}.db"), overwrite: false);
            }
        }
        catch { }
    }
}

public static class DashboardMetricPolicy
{
    private static readonly string[] Forbidden = ["process", "command", "path", "file", "executable"];
    private static readonly string[] AllowedPrefixes =
    [
        "cpu.", "memory.", "network.", "storage.", "system.", "battery.", "hardware.",
        "sensor.", "services.", "startup.", "apps.", "recovery.", "health.", "maintenance.", "power."
    ];

    public static bool IsPersistable(string key) =>
        !string.IsNullOrWhiteSpace(key) && key.Length <= 240 &&
        AllowedPrefixes.Any(prefix => key.StartsWith(prefix, StringComparison.OrdinalIgnoreCase)) &&
        !Forbidden.Any(value => key.Contains(value, StringComparison.OrdinalIgnoreCase));
}
