using System.Globalization;
using System.Text.RegularExpressions;
using Microsoft.UI.Xaml.Media;
using Windows.Foundation;

namespace Sift.WinUI.Infrastructure.Icons;

/// <summary>Builds WinUI path geometry from Sift SVG-style figure strings.</summary>
public static partial class SiftPathGeometryFactory
{
    public static Geometry? Parse(string? figures)
    {
        if (string.IsNullOrWhiteSpace(figures)) return null;

        var tokens = Tokenize(figures);
        var geometry = new PathGeometry();
        PathFigure? figure = null;
        var current = new Point(0, 0);
        var start = current;
        var index = 0;

        while (index < tokens.Count)
        {
            var token = tokens[index++];
            if (!IsCommand(token)) continue;

            var command = token[0];
            var relative = char.IsLower(command);
            switch (char.ToUpperInvariant(command))
            {
                case 'M':
                    while (index + 1 < tokens.Count && !IsCommand(tokens[index]))
                    {
                        var point = ReadPoint(tokens, ref index, current, relative);
                        if (figure is not null && (figure.Segments.Count > 0 || geometry.Figures.Count > 0))
                        {
                            geometry.Figures.Add(figure);
                            figure = null;
                        }

                        current = point;
                        start = current;
                        figure ??= new PathFigure { StartPoint = current, IsClosed = false };
                        figure.StartPoint = current;
                    }
                    break;

                case 'L':
                    figure = EnsureFigure(figure, geometry, ref current, ref start);
                    while (index + 1 < tokens.Count && !IsCommand(tokens[index]))
                    {
                        current = ReadPoint(tokens, ref index, current, relative);
                        figure.Segments.Add(new LineSegment { Point = current });
                    }
                    break;

                case 'H':
                    figure = EnsureFigure(figure, geometry, ref current, ref start);
                    while (index < tokens.Count && !IsCommand(tokens[index]))
                    {
                        var x = ParseDouble(tokens[index++]);
                        if (relative) x += current.X;
                        current = new Point(x, current.Y);
                        figure.Segments.Add(new LineSegment { Point = current });
                    }
                    break;

                case 'V':
                    figure = EnsureFigure(figure, geometry, ref current, ref start);
                    while (index < tokens.Count && !IsCommand(tokens[index]))
                    {
                        var y = ParseDouble(tokens[index++]);
                        if (relative) y += current.Y;
                        current = new Point(current.X, y);
                        figure.Segments.Add(new LineSegment { Point = current });
                    }
                    break;

                case 'C':
                    figure = EnsureFigure(figure, geometry, ref current, ref start);
                    while (index + 5 < tokens.Count && !IsCommand(tokens[index]))
                    {
                        var control1 = ReadPoint(tokens, ref index, current, relative);
                        var control2 = ReadPoint(tokens, ref index, current, relative);
                        var end = ReadPoint(tokens, ref index, current, relative);
                        figure.Segments.Add(new BezierSegment
                        {
                            Point1 = control1,
                            Point2 = control2,
                            Point3 = end
                        });
                        current = end;
                    }
                    break;

                case 'A':
                    figure = EnsureFigure(figure, geometry, ref current, ref start);
                    while (index + 6 < tokens.Count && !IsCommand(tokens[index]))
                    {
                        var radiusX = ParseDouble(tokens[index++]);
                        var radiusY = ParseDouble(tokens[index++]);
                        var rotation = ParseDouble(tokens[index++]);
                        var isLargeArc = ParseDouble(tokens[index++]) != 0;
                        var isClockwise = ParseDouble(tokens[index++]) != 0;
                        var end = ReadPoint(tokens, ref index, current, relative);
                        figure.Segments.Add(new ArcSegment
                        {
                            Point = end,
                            Size = new Size(radiusX, radiusY),
                            RotationAngle = rotation,
                            IsLargeArc = isLargeArc,
                            SweepDirection = isClockwise ? SweepDirection.Clockwise : SweepDirection.Counterclockwise
                        });
                        current = end;
                    }
                    break;

                case 'Z':
                    if (figure is not null)
                    {
                        figure.IsClosed = true;
                        current = start;
                    }
                    break;
            }
        }

        if (figure is not null && (figure.Segments.Count > 0 || geometry.Figures.Count == 0))
            geometry.Figures.Add(figure);

        return geometry.Figures.Count > 0 ? geometry : null;
    }

    private static PathFigure EnsureFigure(
        PathFigure? figure,
        PathGeometry geometry,
        ref Point current,
        ref Point start)
    {
        if (figure is not null) return figure;

        figure = new PathFigure { StartPoint = current, IsClosed = false };
        start = current;
        return figure;
    }

    private static Point ReadPoint(IReadOnlyList<string> tokens, ref int index, Point current, bool relative)
    {
        var x = ParseDouble(tokens[index++]);
        var y = ParseDouble(tokens[index++]);
        if (relative)
        {
            x += current.X;
            y += current.Y;
        }

        return new Point(x, y);
    }

    private static double ParseDouble(string token) =>
        double.Parse(token, CultureInfo.InvariantCulture);

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
