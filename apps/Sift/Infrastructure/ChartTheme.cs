using LiveChartsCore.Measure;
using LiveChartsCore.SkiaSharpView;
using LiveChartsCore.SkiaSharpView.Painting;
using LiveChartsCore.SkiaSharpView.WinUI;
using SkiaSharp;

namespace Sift.WinUI.Infrastructure;

/// <summary>Shared LiveCharts paints and accent/sensor palettes for Sift charts.</summary>
public static class ChartTheme
{
    public static readonly SKColor Clay = SKColor.Parse("#D89462");
    public static readonly SKColor Sage = SKColor.Parse("#A9BB98");
    public static readonly SKColor Neutral = SKColor.Parse("#BDB2A7");
    public static readonly SKColor Cool = SKColor.Parse("#8FA8A0");
    public static readonly SKColor Label = SKColor.Parse("#BDB2A7");
    public static readonly SKColor Separator = SKColor.Parse("#413930");

    public static SKColor ForAccent(string? accent) => accent?.Trim() switch
    {
        "Sage" => Sage,
        "Neutral" => Neutral,
        _ => Clay
    };

    public static SKColor ForSensorType(string? type) => type?.Trim() switch
    {
        "Temperature" => Clay,
        "Power" => Sage,
        "Fan" => Cool,
        "Load" or "Usage" or "Level" => Neutral,
        _ => Clay
    };

    public static SolidColorPaint Stroke(SKColor color, float thickness = 2f) =>
        new(color) { StrokeThickness = thickness };

    public static SolidColorPaint Fill(SKColor color, byte alpha = 0x22) =>
        new(color.WithAlpha(alpha));

    public static SolidColorPaint LabelPaint() => new(Label);

    public static SolidColorPaint SeparatorPaint() =>
        new(Separator) { StrokeThickness = 1 };

    public static void ApplyChrome(
        CartesianChart chart,
        bool showLegend,
        bool showAxes,
        double? yMin = 0,
        double? yMax = null,
        Func<double, string>? yLabeler = null)
    {
        var labelPaint = LabelPaint();
        var separatorPaint = SeparatorPaint();
        chart.LegendTextPaint = labelPaint;
        chart.LegendPosition = showLegend ? LegendPosition.Bottom : LegendPosition.Hidden;
        chart.XAxes =
        [
            new Axis
            {
                IsVisible = showAxes,
                LabelsPaint = labelPaint,
                SeparatorsPaint = separatorPaint,
                TextSize = 10
            }
        ];
        chart.YAxes =
        [
            new Axis
            {
                IsVisible = showAxes,
                MinLimit = yMin,
                MaxLimit = yMax,
                LabelsPaint = labelPaint,
                SeparatorsPaint = separatorPaint,
                TextSize = 10,
                Labeler = yLabeler ?? (value => $"{value:0.##}")
            }
        ];
    }
}
