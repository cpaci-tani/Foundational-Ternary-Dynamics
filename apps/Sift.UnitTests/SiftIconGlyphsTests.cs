using Sift.Infrastructure.Icons;

namespace Sift.UnitTests;

public sealed class SiftIconGlyphsTests
{
    [Theory]
    [InlineData(SiftIconKind.Refresh)]
    [InlineData(SiftIconKind.NavHome)]
    [InlineData(SiftIconKind.Play)]
    [InlineData(SiftIconKind.Close)]
    public void GetPathData_returns_geometry_for_known_icons(SiftIconKind kind)
    {
        var path = SiftIconGlyphs.GetPathData(kind);
        Assert.False(string.IsNullOrWhiteSpace(path));
    }

    [Fact]
    public void All_navigation_icons_have_paths()
    {
        var nav = new[]
        {
            SiftIconKind.NavHome, SiftIconKind.NavOptimize, SiftIconKind.NavTaskManager,
            SiftIconKind.NavPerformance, SiftIconKind.NavHardware, SiftIconKind.NavStartup,
            SiftIconKind.NavMaintenance, SiftIconKind.NavScripts, SiftIconKind.NavHealth,
            SiftIconKind.NavRecovery, SiftIconKind.NavStorage, SiftIconKind.NavApps,
            SiftIconKind.NavSystemInfo, SiftIconKind.NavSettings
        };
        foreach (var kind in nav)
            Assert.False(string.IsNullOrWhiteSpace(SiftIconGlyphs.GetPathData(kind)), kind.ToString());
    }
}
