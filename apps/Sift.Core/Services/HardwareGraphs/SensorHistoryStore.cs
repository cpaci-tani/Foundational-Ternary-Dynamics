namespace Sift.Services;

public sealed class SensorHistoryStore
{
    public const int DefaultCapacity = 180;
    public const int MissingRetentionSamples = 30;

    private readonly Dictionary<string, RingBuffer> _buffers = new(StringComparer.Ordinal);
    private long _generation;

    public int Capacity { get; private set; }

    public SensorHistoryStore(int capacity = DefaultCapacity)
    {
        if (capacity < 8) throw new ArgumentOutOfRangeException(nameof(capacity));
        Capacity = capacity;
    }

    /// <summary>Rebuilds ring buffers to a new capacity, retaining the most recent samples.</summary>
    public void SetCapacity(int capacity)
    {
        capacity = Math.Clamp(capacity, 8, 600);
        if (capacity == Capacity) return;
        Capacity = capacity;
        foreach (var key in _buffers.Keys.ToList())
        {
            var prior = _buffers[key].ToArray();
            var buffer = new RingBuffer(capacity);
            foreach (var value in prior.TakeLast(capacity))
                buffer.Append(value, _generation);
            _buffers[key] = buffer;
        }
    }

    public long Generation => _generation;

    public void AppendSnapshot(IEnumerable<(string SensorId, double Value)> samples)
    {
        ArgumentNullException.ThrowIfNull(samples);
        var generation = ++_generation;
        foreach (var (sensorId, value) in samples)
        {
            if (string.IsNullOrWhiteSpace(sensorId)) continue;
            if (!_buffers.TryGetValue(sensorId, out var buffer))
                _buffers[sensorId] = buffer = new RingBuffer(Capacity);
            buffer.Append(value, generation);
        }
        PurgeStale(generation);
    }

    public void Append(string sensorId, double value)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(sensorId);
        if (!_buffers.TryGetValue(sensorId, out var buffer))
            _buffers[sensorId] = buffer = new RingBuffer(Capacity);
        buffer.Append(value, _generation);
    }

    public IReadOnlyList<double> GetValues(string sensorId)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(sensorId);
        return _buffers.TryGetValue(sensorId, out var buffer)
            ? buffer.ToArray()
            : Array.Empty<double>();
    }

    public void Clear(string sensorId)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(sensorId);
        if (_buffers.TryGetValue(sensorId, out var buffer)) buffer.Clear();
    }

    public void ClearAll() => _buffers.Clear();

    private void PurgeStale(long generation)
    {
        foreach (var sensorId in _buffers
                     .Where(pair => generation - pair.Value.LastSeenGeneration >= MissingRetentionSamples)
                     .Select(pair => pair.Key)
                     .ToList())
            _buffers.Remove(sensorId);
    }

    private sealed class RingBuffer(int capacity)
    {
        private readonly double[] _values = new double[capacity];
        private int _start;
        private int _count;

        public long LastSeenGeneration { get; private set; }

        public void Append(double value, long generation)
        {
            LastSeenGeneration = generation;
            if (_count < _values.Length)
            {
                _values[(_start + _count) % _values.Length] = value;
                _count++;
                return;
            }
            _values[_start] = value;
            _start = (_start + 1) % _values.Length;
        }

        public double[] ToArray()
        {
            var values = new double[_count];
            for (var index = 0; index < _count; index++)
                values[index] = _values[(_start + index) % _values.Length];
            return values;
        }

        public void Clear()
        {
            _start = 0;
            _count = 0;
        }
    }
}
