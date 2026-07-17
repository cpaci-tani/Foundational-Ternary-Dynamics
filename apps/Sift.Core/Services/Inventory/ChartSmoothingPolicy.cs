namespace Sift.Services;

/// <summary>
/// Presentation-neutral policy mapping the persisted <c>ChartSmoothing</c> preference to a
/// bounded line-curvature factor. Kept in Core so the mapping is deterministic and testable
/// without referencing any charting framework.
/// </summary>
public static class ChartSmoothingPolicy
{
    public const string Default = "Light";

    /// <summary>Canonical, ordered smoothing options presented in Settings.</summary>
    public static IReadOnlyList<string> Options { get; } = ["None", "Light", "Medium", "High"];

    /// <summary>
    /// Resolves a smoothing label to a line-curvature factor in the closed interval [0, 1].
    /// Unknown, blank, or null values fall back to the <see cref="Default"/> curvature.
    /// </summary>
    public static double ResolveSmoothness(string? value) => Normalize(value) switch
    {
        "None" => 0.0,
        "Medium" => 0.6,
        "High" => 0.85,
        _ => 0.35,
    };

    /// <summary>
    /// Returns the canonical option that matches <paramref name="value"/> case-insensitively,
    /// or <see cref="Default"/> when there is no match.
    /// </summary>
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
}
