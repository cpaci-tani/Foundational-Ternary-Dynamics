using Sift.Models;
using Sift.Services;
using System.Text.Json;

namespace Sift.Infrastructure.Settings;

public sealed class SettingsPersistenceCoordinator(ISettingsStore store, TimeSpan? delay = null) : IDisposable
{
    private static readonly JsonSerializerOptions SnapshotOptions = new();
    private readonly object _gate = new();
    private readonly TimeSpan _delay = delay ?? TimeSpan.FromMilliseconds(450);
    private Timer? _timer;
    private AppSettings? _pending;

    public void Schedule(AppSettings settings)
    {
        lock (_gate)
        {
            _pending = Snapshot(settings);
            _timer?.Dispose();
            _timer = new Timer(_ => Flush(), null, _delay, Timeout.InfiniteTimeSpan);
        }
    }

    public void SaveNow(AppSettings settings)
    {
        lock (_gate)
        {
            _timer?.Dispose();
            _timer = null;
            _pending = null;
            store.Save(settings);
        }
    }

    public void Flush()
    {
        lock (_gate)
        {
            if (_pending is null) return;
            var settings = _pending;
            _timer?.Dispose();
            _timer = null;
            try
            {
                store.Save(settings);
                _pending = null;
            }
            catch (Exception exception)
            {
                // This runs on a ThreadPool timer thread; an unhandled exception here would be
                // unobserved and terminate the process. Swallow it and keep _pending so the next
                // Schedule/SaveNow/Dispose retries the write. The interactive SaveNow path still
                // surfaces persistence failures normally.
                System.Diagnostics.Debug.WriteLine(exception);
            }
        }
    }

    public void Dispose()
    {
        Flush();
        lock (_gate) _timer?.Dispose();
    }

    private static AppSettings Snapshot(AppSettings settings) =>
        JsonSerializer.Deserialize<AppSettings>(JsonSerializer.Serialize(settings, SnapshotOptions), SnapshotOptions) ?? new AppSettings();
}
