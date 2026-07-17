using Sift.Services;

namespace Sift.UnitTests;

public sealed class ChartSmoothingPolicyTests
{
    [Fact]
    public void Options_are_the_canonical_ordered_set()
    {
        Assert.Equal(new[] { "None", "Light", "Medium", "High" }, ChartSmoothingPolicy.Options);
    }

    [Theory]
    [InlineData("None", 0.0)]
    [InlineData("Light", 0.35)]
    [InlineData("Medium", 0.6)]
    [InlineData("High", 0.85)]
    public void ResolveSmoothness_maps_each_canonical_option(string value, double expected)
    {
        Assert.Equal(expected, ChartSmoothingPolicy.ResolveSmoothness(value), 3);
    }

    [Fact]
    public void ResolveSmoothness_is_case_insensitive_and_trims_whitespace()
    {
        Assert.Equal(0.6, ChartSmoothingPolicy.ResolveSmoothness("  medium  "), 3);
    }

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("   ")]
    [InlineData("bogus")]
    public void ResolveSmoothness_falls_back_to_light_default(string? value)
    {
        Assert.Equal(0.35, ChartSmoothingPolicy.ResolveSmoothness(value), 3);
    }

    [Fact]
    public void Normalize_returns_canonical_casing_or_default()
    {
        Assert.Equal("High", ChartSmoothingPolicy.Normalize("high"));
        Assert.Equal("None", ChartSmoothingPolicy.Normalize("NONE"));
        Assert.Equal("Light", ChartSmoothingPolicy.Normalize("bogus"));
        Assert.Equal("Light", ChartSmoothingPolicy.Normalize(null));
    }
}
