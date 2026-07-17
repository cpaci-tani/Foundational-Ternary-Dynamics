using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using Sift.Infrastructure.Icons;

namespace Sift.WinUI.Infrastructure.Icons;

/// <summary>Builds NavigationView PathIcon instances from Sift SVG-style figure strings.</summary>
public static class SiftNavIconFactory
{
    private static readonly IReadOnlyDictionary<string, SiftIconKind> NavigationMap =
        new Dictionary<string, SiftIconKind>(StringComparer.Ordinal)
        {
            ["Home"] = SiftIconKind.NavHome,
            ["Optimize"] = SiftIconKind.NavOptimize,
            ["TaskManager"] = SiftIconKind.NavTaskManager,
            ["Performance"] = SiftIconKind.NavPerformance,
            ["HardwareMonitor"] = SiftIconKind.NavHardware,
            ["Startup"] = SiftIconKind.NavStartup,
            ["Maintenance"] = SiftIconKind.NavMaintenance,
            ["Scripts"] = SiftIconKind.NavScripts,
            ["Health"] = SiftIconKind.NavHealth,
            ["Recovery"] = SiftIconKind.NavRecovery,
            ["Storage"] = SiftIconKind.NavStorage,
            ["Apps"] = SiftIconKind.NavApps,
            ["SystemInfo"] = SiftIconKind.NavSystemInfo
        };

    public static PathIcon Create(SiftIconKind kind)
    {
        var figures = SiftIconGlyphs.GetPathData(kind) ?? string.Empty;
        return new PathIcon
        {
            Data = ParseGeometry(figures),
            Foreground = ResolveBrush("SiftMutedBrush")
        };
    }

    public static void ApplyNavigationIcons(NavigationView navigation) => Apply(navigation, NavigationMap);

    public static void Apply(NavigationView navigation, IReadOnlyDictionary<string, SiftIconKind> map)
    {
        ArgumentNullException.ThrowIfNull(navigation);
        foreach (var item in navigation.MenuItems.OfType<NavigationViewItem>())
        {
            if (item.Tag is not string tag || !map.TryGetValue(tag, out var kind)) continue;
            item.Icon = Create(kind);
        }
    }

    private static Geometry? ParseGeometry(string figures) =>
        SiftPathGeometryFactory.Parse(figures);

    private static Brush ResolveBrush(string key) =>
        Application.Current.Resources.TryGetValue(key, out var resource) && resource is Brush brush
            ? brush
            : new SolidColorBrush(Microsoft.UI.Colors.White);
}
