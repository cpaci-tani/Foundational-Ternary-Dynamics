namespace Sift.Services;

/// <summary>
/// Canonical refresh-interval labels shared by Performance, Hardware, and Settings UI.
/// </summary>
public static class ChartRefreshIntervalPolicy
{
    public const string Default = "2 seconds";

    public static IReadOnlyList<string> Options { get; } =
        ["1 second", "2 seconds", "3 seconds", "5 seconds"];

    public static string Normalize(string? value)
    {
        if (!string.IsNullOrWhiteSpace(value))
        {
            var trimmed = value.Trim();
            foreach (var option in Options)
                if (string.Equals(option, trimmed, StringComparison.OrdinalIgnoreCase))
                    return option;
        }

        return Default;
    }

    public static TimeSpan Resolve(string? value) => Normalize(value) switch
    {
        "1 second" => TimeSpan.FromSeconds(1),
        "3 seconds" => TimeSpan.FromSeconds(3),
        "5 seconds" => TimeSpan.FromSeconds(5),
        _ => TimeSpan.FromSeconds(2)
    };

    public static int IndexOf(string? value)
    {
        var normalized = Normalize(value);
        for (var index = 0; index < Options.Count; index++)
            if (Options[index] == normalized) return index;
        return 1;
    }
}
