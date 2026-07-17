using Sift.Infrastructure.Activity;

namespace Sift.Infrastructure.Operations;

public sealed record OperationOutcome<T>(
    bool Succeeded,
    bool Cancelled,
    T? Value,
    Exception? Error,
    TimeSpan Elapsed)
{
    public static OperationOutcome<T> Success(T value, TimeSpan elapsed) => new(true, false, value, null, elapsed);
    public static OperationOutcome<T> Cancel(TimeSpan elapsed) => new(false, true, default, null, elapsed);
    public static OperationOutcome<T> Failure(Exception error, TimeSpan elapsed) => new(false, false, default, error, elapsed);
}

public sealed class OperationCoordinator(ActivityHub activity) : IDisposable
{
    private readonly object _gate = new();
    private readonly Dictionary<string, CancellationTokenSource> _active = new(StringComparer.OrdinalIgnoreCase);
    private readonly HashSet<string> _committed = new(StringComparer.OrdinalIgnoreCase);
    private bool _disposed;

    public async Task<OperationOutcome<T>> RunLatestAsync<T>(
        string key,
        string category,
        string description,
        Func<CancellationToken, Task<T>> operation,
        CancellationToken cancellationToken = default)
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        CancellationTokenSource linked;
        lock (_gate)
        {
            if (_committed.Contains(key))
                return OperationOutcome<T>.Failure(
                    new InvalidOperationException($"A committed operation is already running for '{key}'."),
                    TimeSpan.Zero);
            if (_active.Remove(key, out var previous))
            {
                previous.Cancel();
            }
            linked = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
            _active[key] = linked;
        }

        var started = DateTime.UtcNow;
        var operationId = Guid.NewGuid().ToString("N")[..8];
        activity.Info(category, $"Started {description}", operationId: operationId);
        try
        {
            var value = await operation(linked.Token).ConfigureAwait(false);
            linked.Token.ThrowIfCancellationRequested();
            var elapsed = DateTime.UtcNow - started;
            activity.Info(category, $"Completed {description} in {Format(elapsed)}", operationId: operationId);
            return OperationOutcome<T>.Success(value, elapsed);
        }
        catch (OperationCanceledException) when (linked.IsCancellationRequested)
        {
            var elapsed = DateTime.UtcNow - started;
            activity.Warning(category, $"Cancelled {description} after {Format(elapsed)}", operationId: operationId);
            return OperationOutcome<T>.Cancel(elapsed);
        }
        catch (Exception ex)
        {
            var elapsed = DateTime.UtcNow - started;
            activity.Error(category, $"Failed {description}: {ex.Message}", operationId: operationId);
            return OperationOutcome<T>.Failure(ex, elapsed);
        }
        finally
        {
            lock (_gate)
            {
                if (_active.TryGetValue(key, out var current) && ReferenceEquals(current, linked))
                    _active.Remove(key);
            }
            linked.Dispose();
        }
    }

    /// <summary>
    /// Runs a confirmed mutation exactly once for the supplied key. Unlike latest-wins reads, a
    /// committed mutation is never cancelled by navigation, a duplicate click, or coordinator
    /// disposal. The caller token is honored only before the commit begins.
    /// </summary>
    public async Task<OperationOutcome<T>> RunCommittedAsync<T>(
        string key,
        string category,
        string description,
        Func<CancellationToken, Task<T>> operation,
        CancellationToken cancellationToken = default)
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
        ArgumentException.ThrowIfNullOrWhiteSpace(key);
        ArgumentNullException.ThrowIfNull(operation);
        if (cancellationToken.IsCancellationRequested) return OperationOutcome<T>.Cancel(TimeSpan.Zero);

        lock (_gate)
        {
            if (_committed.Contains(key) || _active.ContainsKey(key))
                return OperationOutcome<T>.Failure(
                    new InvalidOperationException($"An operation is already running for '{key}'."),
                    TimeSpan.Zero);
            _committed.Add(key);
        }

        var started = DateTime.UtcNow;
        var operationId = Guid.NewGuid().ToString("N")[..8];
        activity.Info(category, $"Started committed {description}", persist: true, operationId: operationId);
        try
        {
            var value = await operation(CancellationToken.None).ConfigureAwait(false);
            var elapsed = DateTime.UtcNow - started;
            activity.Info(category, $"Completed committed {description} in {Format(elapsed)}",
                persist: true, operationId: operationId);
            return OperationOutcome<T>.Success(value, elapsed);
        }
        catch (Exception exception)
        {
            var elapsed = DateTime.UtcNow - started;
            activity.Error(category, $"Committed {description} failed: {exception.Message}",
                persist: true, operationId: operationId);
            return OperationOutcome<T>.Failure(exception, elapsed);
        }
        finally
        {
            lock (_gate) _committed.Remove(key);
        }
    }

    public void Cancel(string key)
    {
        lock (_gate)
        {
            if (_committed.Contains(key)) return;
            if (_active.TryGetValue(key, out var current)) current.Cancel();
        }
    }

    public void Dispose()
    {
        lock (_gate)
        {
            if (_disposed) return;
            _disposed = true;
            foreach (var operation in _active.Values) operation.Cancel();
            _active.Clear();
        }
    }

    private static string Format(TimeSpan value) => value.TotalSeconds < 1 ? $"{value.TotalMilliseconds:0} ms" : $"{value.TotalSeconds:0.0} s";
}
