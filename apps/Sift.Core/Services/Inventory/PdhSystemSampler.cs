using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using Sift.Models;

namespace Sift.Services;

/// <summary>
/// System-level PDH counters (CPU + physical disk). Open while a workspace or host is sampling;
/// dispose to release the query handle. Never runs ETW and never phones home.
/// </summary>
[SupportedOSPlatform("windows10.0.17763.0")]
public interface IPdhSystemSampler : IDisposable
{
    bool IsOpen { get; }
    bool TryOpen();
    SystemCountersSnapshot? Sample();
}

/// <inheritdoc />
[SupportedOSPlatform("windows10.0.17763.0")]
public sealed class PdhSystemSampler : IPdhSystemSampler
{
    private const uint PdhFmtDouble = 0x00000200;
    private static readonly string[] CounterPaths =
    [
        @"\Processor(_Total)\% Processor Time",
        @"\PhysicalDisk(_Total)\Disk Read Bytes/sec",
        @"\PhysicalDisk(_Total)\Disk Write Bytes/sec"
    ];

    private readonly object _gate = new();
    private nint _query;
    private nint _cpu;
    private nint _diskRead;
    private nint _diskWrite;
    private bool _open;
    private bool _primed;
    private bool _disposed;

    public bool IsOpen
    {
        get { lock (_gate) return _open; }
    }

    public bool TryOpen()
    {
        lock (_gate)
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            return TryOpenUnlocked();
        }
    }

    public SystemCountersSnapshot? Sample()
    {
        lock (_gate)
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            if (!_open && !TryOpenUnlocked()) return null;
            if (PdhCollectQueryData(_query) != 0) return null;
            if (!_primed)
            {
                _primed = true;
                return null;
            }

            if (!TryRead(_cpu, out var cpu) ||
                !TryRead(_diskRead, out var readBytes) ||
                !TryRead(_diskWrite, out var writeBytes))
                return null;

            return new SystemCountersSnapshot(
                Math.Clamp(cpu, 0, 100),
                Math.Max(0, readBytes / 1_048_576d),
                Math.Max(0, writeBytes / 1_048_576d));
        }
    }

    public void Dispose()
    {
        lock (_gate)
        {
            if (_disposed) return;
            _disposed = true;
            CloseUnlocked();
        }
    }

    private bool TryOpenUnlocked()
    {
        if (_open) return true;
        if (PdhOpenQuery(null, 0, out _query) != 0) return false;
        if (PdhAddEnglishCounter(_query, CounterPaths[0], 0, out _cpu) != 0 ||
            PdhAddEnglishCounter(_query, CounterPaths[1], 0, out _diskRead) != 0 ||
            PdhAddEnglishCounter(_query, CounterPaths[2], 0, out _diskWrite) != 0)
        {
            CloseUnlocked();
            return false;
        }

        _ = PdhCollectQueryData(_query);
        _open = true;
        _primed = false;
        return true;
    }

    private void CloseUnlocked()
    {
        if (_query != 0)
        {
            _ = PdhCloseQuery(_query);
            _query = 0;
            _cpu = 0;
            _diskRead = 0;
            _diskWrite = 0;
        }
        _open = false;
        _primed = false;
    }

    private static bool TryRead(nint counter, out double value)
    {
        value = 0;
        if (PdhGetFormattedCounterValue(counter, PdhFmtDouble, out _, out var formatted) != 0)
            return false;
        if (formatted.CStatus != 0) return false;
        value = formatted.DoubleValue;
        return double.IsFinite(value);
    }

    [DllImport("pdh.dll", CharSet = CharSet.Unicode)]
    private static extern uint PdhOpenQuery(string? szDataSource, nuint dwUserData, out nint phQuery);

    [DllImport("pdh.dll", CharSet = CharSet.Unicode)]
    private static extern uint PdhAddEnglishCounter(nint hQuery, string szFullCounterPath, nuint dwUserData, out nint phCounter);

    [DllImport("pdh.dll")]
    private static extern uint PdhCollectQueryData(nint hQuery);

    [DllImport("pdh.dll")]
    private static extern uint PdhGetFormattedCounterValue(nint hCounter, uint dwFormat, out uint lpdwType, out PdhFmtCounterValue pValue);

    [DllImport("pdh.dll")]
    private static extern uint PdhCloseQuery(nint hQuery);

    [StructLayout(LayoutKind.Explicit)]
    private struct PdhFmtCounterValue
    {
        [FieldOffset(0)] public uint CStatus;
        [FieldOffset(8)] public double DoubleValue;
    }
}
