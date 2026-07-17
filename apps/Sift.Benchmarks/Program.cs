using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Running;
using Sift.Services;

namespace Sift.Benchmarks;

internal static class Program
{
    private static void Main(string[] args) =>
        BenchmarkSwitcher.FromAssembly(typeof(Program).Assembly).Run(args);
}

/// <summary>
/// Overhead gate for Wave 2 telemetry. Ship PDH only when median ProcessSampler+PDH
/// stays within ~15% of ProcessSampler alone on an idle desktop (see ROADMAP / ARCHITECTURE).
/// </summary>
[MemoryDiagnoser]
public class SamplingBenchmarks
{
    private ProcessSampler _processes = null!;
    private PdhSystemSampler _pdh = null!;

    [GlobalSetup]
    public void Setup()
    {
        _processes = new ProcessSampler();
        _pdh = new PdhSystemSampler();
        _ = _pdh.TryOpen();
        _ = _processes.Sample();
        _ = _pdh.Sample();
        Thread.Sleep(100);
        _ = _pdh.Sample();
    }

    [GlobalCleanup]
    public void Cleanup() => _pdh.Dispose();

    [Benchmark(Baseline = true)]
    public void ProcessSampler_Only() => _ = _processes.Sample();

    [Benchmark]
    public void ProcessSampler_Plus_Pdh()
    {
        _ = _processes.Sample();
        _ = _pdh.Sample();
    }

    [Benchmark]
    public void Pdh_Only() => _ = _pdh.Sample();
}
