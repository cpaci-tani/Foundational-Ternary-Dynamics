using Sift.Services;

namespace Sift.UnitTests;

public sealed class UiScalePolicyTests
{
    [Fact]
    public void Options_AreOrderedCompactDefaultLarge() =>
        Assert.Equal(new[] { "Compact", "Default", "Large" }, UiScalePolicy.Options);

    [Theory]
    [InlineData("Compact", 0.9)]
    [InlineData("Default", 1.0)]
    [InlineData("Large", 1.12)]
    public void ResolveFactor_MapsCanonicalLabels(string value, double expected) =>
        Assert.Equal(expected, UiScalePolicy.ResolveFactor(value), 3);

    [Fact]
    public void ResolveFactor_TrimsAndIgnoresCase() =>
        Assert.Equal(1.12, UiScalePolicy.ResolveFactor("  LARGE  "), 3);

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("huge")]
    public void ResolveFactor_FallsBackToDefault(string? value) =>
        Assert.Equal(1.0, UiScalePolicy.ResolveFactor(value), 3);

    [Fact]
    public void Normalize_ReturnsCanonicalOrDefault()
    {
        Assert.Equal("Compact", UiScalePolicy.Normalize("compact"));
        Assert.Equal("Large", UiScalePolicy.Normalize("LARGE"));
        Assert.Equal("Default", UiScalePolicy.Normalize("bogus"));
        Assert.Equal("Default", UiScalePolicy.Normalize(null));
    }
}
