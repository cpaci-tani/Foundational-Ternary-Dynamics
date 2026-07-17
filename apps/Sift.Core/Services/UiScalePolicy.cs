namespace Sift.Services;

/// <summary>
/// Presentation-neutral policy for the persisted <c>UiScale</c> preference.
/// Factors are applied by the WinUI shell as a uniform content scale.
/// </summary>
public static class UiScalePolicy
{
    public const string Default = "Default";

    /// <summary>Canonical, ordered UI size options presented in Settings.</summary>
    public static IReadOnlyList<string> Options { get; } = ["Compact", "Default", "Large"];

    /// <summary>
    /// Resolves a UI size label to a uniform scale factor.
    /// Unknown, blank, or null values fall back to <see cref="Default"/>.
    /// </summary>
    public static double ResolveFactor(string? value) => Normalize(value) switch
    {
        "Compact" => 0.9,
        "Large" => 1.12,
        _ => 1.0,
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
