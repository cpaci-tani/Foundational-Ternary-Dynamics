using Sift.Infrastructure.Icons;

namespace Sift.UnitTests;

public sealed class SiftPathMarkupTests
{
    [Fact]
    public void FromFigures_uses_commas_for_coordinate_pairs()
    {
        var markup = SiftPathMarkup.FromFigures("M4 11 L12 5 L20 11 V19 H15 V14 H9 V19 H4 Z");
        Assert.Contains("M4,11", markup);
        Assert.Contains("L12,5", markup);
        Assert.Contains("V19", markup);
    }

    [Fact]
    public void FromFigures_covers_all_navigation_icons()
    {
        foreach (var kind in new[]
        {
            SiftIconKind.NavHome, SiftIconKind.NavOptimize, SiftIconKind.NavTaskManager,
            SiftIconKind.NavPerformance, SiftIconKind.NavHardware, SiftIconKind.NavStartup,
            SiftIconKind.NavMaintenance, SiftIconKind.NavScripts, SiftIconKind.NavHealth,
            SiftIconKind.NavRecovery, SiftIconKind.NavStorage, SiftIconKind.NavApps,
            SiftIconKind.NavSystemInfo
        })
        {
            var markup = SiftPathMarkup.FromFigures(SiftIconGlyphs.GetPathData(kind)!);
            Assert.False(string.IsNullOrWhiteSpace(markup), kind.ToString());
        }
    }
}
