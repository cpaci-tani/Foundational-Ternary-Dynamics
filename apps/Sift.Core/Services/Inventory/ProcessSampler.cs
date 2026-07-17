using System.Diagnostics;
using Sift.Models;
using Windows.Win32;
using Windows.Win32.Foundation;
using Windows.Win32.System.Threading;

namespace Sift.Services;

public sealed class ProcessSampler : IProcessSampler
{
    private readonly SemaphoreSlim _sampleGate = new(1, 1);
    private readonly Dictionary<int, TimeSpan> _previousCpu = [];
    private readonly Dictionary<int, (ulong Read, ulong Write)> _previousIo = [];
    private long _previousTimestamp;
    private bool _warmedUp;

    public SystemSnapshot Sample(CancellationToken cancellationToken = default)
    {
        _sampleGate.Wait(cancellationToken);
        try
        {
            return SampleCore(cancellationToken);
        }
        finally
        {
            _sampleGate.Release();
        }
    }

    private SystemSnapshot SampleCore(CancellationToken cancellationToken)
    {
        if (!_warmedUp)
        {
            _warmedUp = true;
            CaptureBaselines(cancellationToken);
            if (cancellationToken.WaitHandle.WaitOne(80)) cancellationToken.ThrowIfCancellationRequested();
        }

        var now = Stopwatch.GetTimestamp();
        var elapsed = _previousTimestamp == 0 ? 0 : (now - _previousTimestamp) / (double)Stopwatch.Frequency;
        _previousTimestamp = now;
        var nextCpu = new Dictionary<int, TimeSpan>();
        var nextIo = new Dictionary<int, (ulong Read, ulong Write)>();
        var rows = new List<ProcessSnapshot>();
        var totalCpu = 0d;

        foreach (var process in Process.GetProcesses())
        {
            cancellationToken.ThrowIfCancellationRequested();
            using (process)
            {
                try
                {
                    var cpuTime = process.TotalProcessorTime;
                    nextCpu[process.Id] = cpuTime;
                    var cpu = elapsed > 0 && _previousCpu.TryGetValue(process.Id, out var prior)
                        ? Math.Clamp((cpuTime - prior).TotalSeconds / elapsed / Environment.ProcessorCount * 100d, 0, 100)
                        : 0;
                    totalCpu += cpu;
                    var io = ReadIo(process);
                    nextIo[process.Id] = io;
                    var read = 0d;
                    var write = 0d;
                    if (elapsed > 0 && _previousIo.TryGetValue(process.Id, out var previous))
                    {
                        read = io.Read >= previous.Read ? (io.Read - previous.Read) / elapsed / 1048576d : 0;
                        write = io.Write >= previous.Write ? (io.Write - previous.Write) / elapsed / 1048576d : 0;
                    }

                    var executable = Safe(() => process.MainModule?.FileName ?? "Unavailable", "Unavailable");
                    rows.Add(new ProcessSnapshot(
                        process.Id,
                        process.ProcessName,
                        cpu,
                        process.WorkingSet64 / 1048576d,
                        Safe(() => process.PrivateMemorySize64 / 1048576d, 0),
                        read,
                        write,
                        Safe(() => process.Threads.Count, 0),
                        Safe(() => process.HandleCount, 0),
                        Safe(() => process.Responding ? "Running" : "Not responding", "Running"),
                        Safe(() => process.PriorityClass.ToString(), "Protected"),
                        Safe(() => process.MainWindowTitle, ""),
                        Safe(() => Math.Max(0, (DateTime.Now - process.StartTime).TotalSeconds), 0),
                        Safe(() => process.SessionId, -1),
                        Safe(() => process.StartTime.ToUniversalTime().Ticks, 0L),
                        ReadArchitecture(process),
                        executable,
                        IconFor(executable)));
                }
                catch { }
            }
        }

        _previousCpu.Clear();
        foreach (var pair in nextCpu) _previousCpu[pair.Key] = pair.Value;
        _previousIo.Clear();
        foreach (var pair in nextIo) _previousIo[pair.Key] = pair.Value;
        var memory = SystemMemoryReader.Read();
        return new SystemSnapshot(
            rows.OrderByDescending(x => x.CpuPercent).ThenByDescending(x => x.MemoryMb).ToList(),
            Math.Clamp(totalCpu, 0, 100), memory.Percent, memory.UsedGb, memory.TotalGb);
    }

    private void CaptureBaselines(CancellationToken cancellationToken)
    {
        _previousTimestamp = Stopwatch.GetTimestamp();
        foreach (var process in Process.GetProcesses())
        {
            cancellationToken.ThrowIfCancellationRequested();
            using (process)
            {
                try
                {
                    _previousCpu[process.Id] = process.TotalProcessorTime;
                    _previousIo[process.Id] = ReadIo(process);
                }
                catch { }
            }
        }
    }

    private static byte[]? IconFor(string executable) =>
        string.IsNullOrWhiteSpace(executable) || executable.Equals("Unavailable", StringComparison.OrdinalIgnoreCase)
            ? null
            : AppIconExtractor.TryExtractPng(executable);

    private static T Safe<T>(Func<T> read, T fallback) { try { return read(); } catch { return fallback; } }

    private static unsafe (ulong Read, ulong Write) ReadIo(Process process)
    {
        IO_COUNTERS io;
        return PInvoke.GetProcessIoCounters((HANDLE)process.Handle, &io)
            ? (io.ReadTransferCount, io.WriteTransferCount)
            : (0, 0);
    }

    private static unsafe string ReadArchitecture(Process process)
    {
        try
        {
            if (!Environment.Is64BitOperatingSystem) return "x86";
            BOOL wow64;
            return PInvoke.IsWow64Process((HANDLE)process.Handle, &wow64) && wow64 ? "x86" : "x64";
        }
        catch { return "—"; }
    }
}
