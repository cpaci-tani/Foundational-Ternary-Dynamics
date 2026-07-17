using Sift.Services;

namespace Sift.UnitTests;

public sealed class ChartRefreshIntervalPolicyTests
{
    [Theory]
    [InlineData(null, "2 seconds")]
    [InlineData("", "2 seconds")]
    [InlineData("1 second", "1 second")]
    [InlineData("5 SECONDS", "5 seconds")]
    [InlineData("nope", "2 seconds")]
    public void Normalize_maps_known_labels(string? value, string expected) =>
        Assert.Equal(expected, ChartRefreshIntervalPolicy.Normalize(value));

    [Fact]
    public void Resolve_returns_expected_spans()
    {
        Assert.Equal(TimeSpan.FromSeconds(1), ChartRefreshIntervalPolicy.Resolve("1 second"));
        Assert.Equal(TimeSpan.FromSeconds(5), ChartRefreshIntervalPolicy.Resolve("5 seconds"));
        Assert.Equal(TimeSpan.FromSeconds(2), ChartRefreshIntervalPolicy.Resolve("unknown"));
    }
}
