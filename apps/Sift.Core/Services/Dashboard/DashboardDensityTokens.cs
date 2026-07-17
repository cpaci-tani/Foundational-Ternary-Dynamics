using Sift.Models;

namespace Sift.Services;

/// <summary>
/// Content-scale tokens for Home dashboard widgets. Density changes fonts, gaps, and
/// control sizes — not only host padding.
/// </summary>
public readonly record struct DashboardDensityTokens(
    double HostPadding,
    double HostRowSpacing,
    double ContentRowSpacing,
    double MetricFontSize,
    double TitleFontSize,
    double MetaFontSize,
    double ActionMinHeight,
    double ListRowHeight,
    double ChartBottomMargin,
    float ChartStrokeThickness)
{
    public static DashboardDensityTokens For(DashboardDensity density) => density switch
    {
        DashboardDensity.Compact => new(
            HostPadding: 8,
            HostRowSpacing: 4,
            ContentRowSpacing: 4,
            MetricFontSize: 16,
            TitleFontSize: 13,
            MetaFontSize: 10,
            ActionMinHeight: 28,
            ListRowHeight: 22,
            ChartBottomMargin: 4,
            ChartStrokeThickness: 1.5f),
        DashboardDensity.Comfortable => new(
            HostPadding: 16,
            HostRowSpacing: 8,
            ContentRowSpacing: 8,
            MetricFontSize: 22,
            TitleFontSize: 16,
            MetaFontSize: 11,
            ActionMinHeight: 36,
            ListRowHeight: 28,
            ChartBottomMargin: 12,
            ChartStrokeThickness: 2f),
        _ => new(
            HostPadding: 12,
            HostRowSpacing: 6,
            ContentRowSpacing: 6,
            MetricFontSize: 18,
            TitleFontSize: 14,
            MetaFontSize: 11,
            ActionMinHeight: 32,
            ListRowHeight: 26,
            ChartBottomMargin: 8,
            ChartStrokeThickness: 1.75f)
    };
}
