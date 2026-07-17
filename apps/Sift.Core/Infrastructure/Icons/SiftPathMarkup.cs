using System.Text;
using System.Text.RegularExpressions;

namespace Sift.Infrastructure.Icons;

/// <summary>Converts Sift SVG-style figure strings to WinUI path markup.</summary>
public static partial class SiftPathMarkup
{
    public static string FromFigures(string figures)
    {
        if (string.IsNullOrWhiteSpace(figures)) return string.Empty;
        var tokens = Tokenize(figures);
        var builder = new StringBuilder();
        var index = 0;
        while (index < tokens.Count)
        {
            var token = tokens[index++];
            if (!IsCommand(token)) continue;
            if (builder.Length > 0) builder.Append(' ');
            builder.Append(token);
            var command = char.ToUpperInvariant(token[0]);
            if (command == 'Z') continue;

            var numbers = new List<string>();
            while (index < tokens.Count && !IsCommand(tokens[index]))
                numbers.Add(tokens[index++]);

            AppendParameters(builder, command, numbers);
        }
        return builder.ToString();
    }

    private static void AppendParameters(StringBuilder builder, char command, IReadOnlyList<string> numbers)
    {
        var index = 0;
        var first = true;
        switch (command)
        {
            case 'H':
            case 'V':
                if (index < numbers.Count) builder.Append(numbers[index++]);
                break;
            case 'A':
                while (index + 6 < numbers.Count)
                {
                    if (!first) builder.Append(' ');
                    first = false;
                    builder
                        .Append(numbers[index]).Append(',').Append(numbers[index + 1]).Append(' ')
                        .Append(numbers[index + 2]).Append(' ')
                        .Append(numbers[index + 3]).Append(' ')
                        .Append(numbers[index + 4]).Append(' ')
                        .Append(numbers[index + 5]).Append(',').Append(numbers[index + 6]);
                    index += 7;
                }
                break;
            default:
                while (index + 1 < numbers.Count)
                {
                    if (!first) builder.Append(' ');
                    first = false;
                    builder.Append(numbers[index]).Append(',').Append(numbers[index + 1]);
                    index += 2;
                }
                if (index < numbers.Count)
                {
                    if (!first) builder.Append(' ');
                    builder.Append(numbers[index]);
                }
                break;
        }
    }

    private static bool IsCommand(string token) =>
        token.Length == 1 && char.IsLetter(token[0]);

    private static List<string> Tokenize(string figures)
    {
        var matches = FigureToken().Matches(figures);
        var tokens = new List<string>(matches.Count);
        foreach (Match match in matches) tokens.Add(match.Value);
        return tokens;
    }

    [GeneratedRegex(@"-?\d*\.?\d+|[A-Za-z]", RegexOptions.CultureInvariant)]
    private static partial Regex FigureToken();
}
