using System.Collections.Specialized;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Shapes;
using Windows.Foundation;
using Windows.UI;

namespace Sift.WinUI.Controls;

/// <summary>Lightweight path sparkline for dense sensor grids (not LiveCharts).</summary>
public sealed class MiniSparkline : UserControl
{
    public static readonly DependencyProperty ValuesProperty = DependencyProperty.Register(
        nameof(Values), typeof(object), typeof(MiniSparkline),
        new PropertyMetadata(null, OnValuesChanged));

    public static readonly DependencyProperty StrokeProperty = DependencyProperty.Register(
        nameof(Stroke), typeof(Brush), typeof(MiniSparkline),
        new PropertyMetadata(null, OnAppearanceChanged));

    public static readonly DependencyProperty FillProperty = DependencyProperty.Register(
        nameof(Fill), typeof(Brush), typeof(MiniSparkline),
        new PropertyMetadata(null, OnAppearanceChanged));

    private readonly Microsoft.UI.Xaml.Shapes.Path _fillPath = new() { Stretch = Stretch.None };
    private readonly Microsoft.UI.Xaml.Shapes.Path _strokePath = new() { Stretch = Stretch.None, StrokeThickness = 1.25 };
    private readonly Grid _root = new();
    private INotifyCollectionChanged? _listening;

    public MiniSparkline()
    {
        IsHitTestVisible = false;
        MinHeight = 16;
        Height = 18;
        _root.Children.Add(_fillPath);
        _root.Children.Add(_strokePath);
        Content = _root;
        SizeChanged += (_, _) => Redraw();
        ApplyDefaultBrushes();
    }

    public object? Values
    {
        get => GetValue(ValuesProperty);
        set => SetValue(ValuesProperty, value);
    }

    public Brush? Stroke
    {
        get => (Brush?)GetValue(StrokeProperty);
        set => SetValue(StrokeProperty, value);
    }

    public Brush? Fill
    {
        get => (Brush?)GetValue(FillProperty);
        set => SetValue(FillProperty, value);
    }

    private static void OnValuesChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var spark = (MiniSparkline)d;
        spark.DetachCollection();
        if (e.NewValue is INotifyCollectionChanged notify)
        {
            spark._listening = notify;
            notify.CollectionChanged += spark.Values_CollectionChanged;
        }
        spark.Redraw();
    }

    private void Values_CollectionChanged(object? sender, NotifyCollectionChangedEventArgs e) => Redraw();

    private void DetachCollection()
    {
        if (_listening is null) return;
        _listening.CollectionChanged -= Values_CollectionChanged;
        _listening = null;
    }

    private static void OnAppearanceChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) =>
        ((MiniSparkline)d).ApplyBrushes();

    private void ApplyDefaultBrushes()
    {
        if (Stroke is null && Application.Current.Resources.TryGetValue("SiftAccentBrush", out var accent) &&
            accent is Brush accentBrush)
            Stroke = accentBrush;
        if (Fill is null && Application.Current.Resources.TryGetValue("SiftAccentDarkBrush", out var dark) &&
            dark is SolidColorBrush solid)
        {
            Fill = new SolidColorBrush(Color.FromArgb(0x55, solid.Color.R, solid.Color.G, solid.Color.B));
        }
        ApplyBrushes();
    }

    private void ApplyBrushes()
    {
        _strokePath.Stroke = Stroke;
        _fillPath.Fill = Fill;
    }

    private void Redraw()
    {
        var samples = ReadValues();
        var width = Math.Max(1, ActualWidth);
        var height = Math.Max(1, ActualHeight);
        if (samples.Count < 2 || width < 2 || height < 2)
        {
            _strokePath.Data = null;
            _fillPath.Data = null;
            return;
        }

        var min = samples.Min();
        var max = samples.Max();
        var span = Math.Max(0.0001, max - min);
        var step = width / (samples.Count - 1);
        var geometry = new PathGeometry();
        var figure = new PathFigure { IsClosed = false, IsFilled = false };
        for (var index = 0; index < samples.Count; index++)
        {
            var x = index * step;
            var y = height - ((samples[index] - min) / span) * (height - 2) - 1;
            var point = new Point(x, y);
            if (index == 0) figure.StartPoint = point;
            else figure.Segments.Add(new LineSegment { Point = point });
        }
        geometry.Figures.Add(figure);
        _strokePath.Data = geometry;

        var fillGeometry = new PathGeometry();
        var fillFigure = new PathFigure
        {
            StartPoint = new Point(0, height),
            IsClosed = true,
            IsFilled = true
        };
        fillFigure.Segments.Add(new LineSegment { Point = figure.StartPoint });
        foreach (var segment in figure.Segments.OfType<LineSegment>())
            fillFigure.Segments.Add(new LineSegment { Point = segment.Point });
        fillFigure.Segments.Add(new LineSegment { Point = new Point(width, height) });
        fillGeometry.Figures.Add(fillFigure);
        _fillPath.Data = fillGeometry;
    }

    private List<double> ReadValues()
    {
        return Values switch
        {
            IReadOnlyList<double> list => list.Where(double.IsFinite).ToList(),
            IEnumerable<double> enumerable => enumerable.Where(double.IsFinite).ToList(),
            _ => []
        };
    }
}
