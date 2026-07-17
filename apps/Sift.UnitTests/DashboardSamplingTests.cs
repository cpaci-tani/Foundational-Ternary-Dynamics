using Sift.Services;

namespace Sift.UnitTests;

public sealed class DashboardSamplingTests
{
    [Fact]
    public void Coordinator_uses_wall_clock_cadences_and_forces_slow_after_resume()
    {
        var coordinator = new DashboardSamplingCoordinator();
        var start = DateTimeOffset.UtcNow;

        Assert.Equal(DashboardSampleKind.Fast, coordinator.Next(start));
        Assert.Equal(DashboardSampleKind.Fast, coordinator.Next(start.AddSeconds(2)));
        Assert.Equal(DashboardSampleKind.Medium, coordinator.Next(start.AddSeconds(30)));
        Assert.Equal(DashboardSampleKind.Slow, coordinator.Next(start.AddMinutes(5)));
        Assert.Equal(DashboardSampleKind.Slow, coordinator.Next(start.AddMinutes(6)));
    }

    [Fact]
    public void Coordinator_slows_fast_samples_on_battery_saver()
    {
        Assert.Equal(TimeSpan.FromSeconds(2), DashboardSamplingCoordinator.Delay(false));
        Assert.Equal(TimeSpan.FromSeconds(10), DashboardSamplingCoordinator.Delay(true));
    }
}
