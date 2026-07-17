using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using Sift.Infrastructure.Icons;
using Sift.WinUI.Infrastructure.Icons;

namespace Sift.WinUI.Controls;

public sealed partial class SiftIcon : UserControl
{
    public static readonly DependencyProperty KindProperty = DependencyProperty.Register(
        nameof(Kind), typeof(SiftIconKind), typeof(SiftIcon),
        new PropertyMetadata(SiftIconKind.None, OnVisualPropertyChanged));

    public static readonly DependencyProperty SizeProperty = DependencyProperty.Register(
        nameof(Size), typeof(double), typeof(SiftIcon),
        new PropertyMetadata(16d, OnVisualPropertyChanged));

    public static readonly DependencyProperty IconBrushProperty = DependencyProperty.Register(
        nameof(IconBrush), typeof(Brush), typeof(SiftIcon),
        new PropertyMetadata(null, OnVisualPropertyChanged));

    public SiftIcon()
    {
        InitializeComponent();
        IsHitTestVisible = false;
        Loaded += (_, _) => ApplyGlyph();
    }

    public SiftIconKind Kind
    {
        get => (SiftIconKind)GetValue(KindProperty);
        set => SetValue(KindProperty, value);
    }

    public double Size
    {
        get => (double)GetValue(SizeProperty);
        set => SetValue(SizeProperty, value);
    }

    public Brush? IconBrush
    {
        get => (Brush?)GetValue(IconBrushProperty);
        set => SetValue(IconBrushProperty, value);
    }

    private static void OnVisualPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) =>
        ((SiftIcon)d).ScheduleApplyGlyph();

    private void ScheduleApplyGlyph()
    {
        if (IsLoaded)
            ApplyGlyph();
    }

    private void ApplyGlyph()
    {
        IconViewbox.Width = Size;
        IconViewbox.Height = Size;
        var brush = IconBrush ?? (Foreground as Brush) ?? ResolveThemeBrush("SiftTextBrush");
        var glyph = SiftIconGlyphs.Get(Kind);

        if (glyph.UseFill && !string.IsNullOrWhiteSpace(glyph.FillPath))
        {
            FillPath.Data = ParseGeometry(glyph.FillPath);
            FillPath.Fill = brush;
            FillPath.Visibility = Visibility.Visible;
            StrokePath.Visibility = Visibility.Collapsed;
            return;
        }

        FillPath.Visibility = Visibility.Collapsed;
        StrokePath.Visibility = string.IsNullOrWhiteSpace(glyph.StrokePath) ? Visibility.Collapsed : Visibility.Visible;
        StrokePath.Data = ParseGeometry(glyph.StrokePath);
        StrokePath.Stroke = brush;
        StrokePath.StrokeThickness = glyph.StrokeThickness;
    }

    private static Geometry? ParseGeometry(string? path) =>
        SiftPathGeometryFactory.Parse(path);

    private static Brush ResolveThemeBrush(string key) =>
        Application.Current.Resources.TryGetValue(key, out var resource) && resource is Brush brush
            ? brush
            : new SolidColorBrush(Microsoft.UI.Colors.White);
}
