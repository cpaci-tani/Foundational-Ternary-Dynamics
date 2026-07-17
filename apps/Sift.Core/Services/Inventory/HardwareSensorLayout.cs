using System.Text.RegularExpressions;

namespace Sift.Services;

/// <summary>
/// Groups and classifies hardware sensors for inventory presentation
/// (type sections, per-core CPU grids).
/// </summary>
public static partial class HardwareSensorLayout
{
    private static readonly Regex PerCoreName = CoreNameRegex();

    public static bool IsPerCoreSensor(string type, string name)
    {
        if (string.IsNullOrWhiteSpace(type) || string.IsNullOrWhiteSpace(name)) return false;
        if (type is not ("Load" or "Clock")) return false;
        if (name.Contains("Average", StringComparison.OrdinalIgnoreCase)) return false;
        if (name.Contains("VID", StringComparison.OrdinalIgnoreCase)) return false;
        if (name.Contains("Total", StringComparison.OrdinalIgnoreCase)) return false;
        if (name.Contains("Max", StringComparison.OrdinalIgnoreCase) &&
            !PerCoreName.IsMatch(name)) return false;
        return PerCoreName.IsMatch(name);
    }

    public static string CoreShortLabel(string name)
    {
        var match = PerCoreName.Match(name);
        return match.Success ? $"#{match.Groups[1].Value}" : name;
    }

    public static int CoreSortKey(string name)
    {
        var match = PerCoreName.Match(name);
        return match.Success && int.TryParse(match.Groups[1].Value, out var index) ? index : int.MaxValue;
    }

    public static int TypeOrder(string type) => type switch
    {
        "Temperature" => 0,
        "Load" => 1,
        "Clock" => 2,
        "Power" => 3,
        "Fan" => 4,
        "Voltage" => 5,
        "Current" => 6,
        "Data" or "SmallData" => 7,
        "Throughput" => 8,
        _ => 20
    };

    [GeneratedRegex(@"^(?:CPU\s+)?Core\s*#\s*(\d+)", RegexOptions.IgnoreCase | RegexOptions.CultureInvariant)]
    private static partial Regex CoreNameRegex();
}
