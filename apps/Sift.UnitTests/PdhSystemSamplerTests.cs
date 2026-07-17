using Sift.Services;

namespace Sift.UnitTests;

public sealed class PdhSystemSamplerTests
{
    [Fact]
    public void Open_sample_dispose_is_safe_and_returns_finite_counters_when_available()
    {
        using var sampler = new PdhSystemSampler();
        Assert.True(sampler.TryOpen());
        Assert.True(sampler.IsOpen);

        // First post-open sample primes rate counters and may return null.
        _ = sampler.Sample();
        Thread.Sleep(120);
        var second = sampler.Sample();
        if (second is not null)
        {
            Assert.InRange(second.CpuPercent, 0, 100);
            Assert.True(second.DiskReadMbPerSec >= 0);
            Assert.True(second.DiskWriteMbPerSec >= 0);
        }

        sampler.Dispose();
        Assert.False(sampler.IsOpen);
        Assert.Throws<ObjectDisposedException>(() => sampler.TryOpen());
    }

    [Fact]
    public void Dispose_is_idempotent()
    {
        var sampler = new PdhSystemSampler();
        Assert.True(sampler.TryOpen());
        sampler.Dispose();
        sampler.Dispose();
    }
}
