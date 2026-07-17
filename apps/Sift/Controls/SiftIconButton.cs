using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using Sift.Infrastructure.Icons;

namespace Sift.WinUI.Controls;

/// <summary>Button with a Sift SVG icon and optional text label.</summary>
public sealed class SiftIconButton : Button
{
    public static readonly DependencyProperty IconProperty = DependencyProperty.Register(
        nameof(Icon), typeof(SiftIconKind), typeof(SiftIconButton),
        new PropertyMetadata(SiftIconKind.None, OnContentPropertyChanged));

    public static readonly DependencyProperty LabelProperty = DependencyProperty.Register(
        nameof(Label), typeof(string), typeof(SiftIconButton),
        new PropertyMetadata(string.Empty, OnContentPropertyChanged));

    public static readonly DependencyProperty IconSizeProperty = DependencyProperty.Register(
        nameof(IconSize), typeof(double), typeof(SiftIconButton),
        new PropertyMetadata(16d, OnContentPropertyChanged));

    private readonly StackPanel _content = new() { Orientation = Orientation.Horizontal, Spacing = 8 };
    private readonly SiftIcon _icon = new();
    private readonly TextBlock _label = new() { VerticalAlignment = VerticalAlignment.Center };

    public SiftIconButton()
    {
        _content.Children.Add(_icon);
        _content.Children.Add(_label);
        Content = _content;
        RegisterPropertyChangedCallback(ForegroundProperty, OnVisualPropertyChanged);
        RegisterPropertyChangedCallback(FontWeightProperty, OnVisualPropertyChanged);
        RebuildContent();
    }

    public SiftIconKind Icon
    {
        get => (SiftIconKind)GetValue(IconProperty);
        set => SetValue(IconProperty, value);
    }

    public string Label
    {
        get => (string)GetValue(LabelProperty);
        set => SetValue(LabelProperty, value);
    }

    public double IconSize
    {
        get => (double)GetValue(IconSizeProperty);
        set => SetValue(IconSizeProperty, value);
    }

    private static void OnContentPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) =>
        ((SiftIconButton)d).RebuildContent();

    private static void OnVisualPropertyChanged(DependencyObject d, DependencyProperty dp) =>
        ((SiftIconButton)d).RebuildContent();

    private void RebuildContent()
    {
        _icon.Kind = Icon;
        _icon.Size = IconSize;
        _icon.Foreground = Foreground;
        _label.Text = Label;
        _label.Foreground = Foreground;
        _label.FontWeight = FontWeight;
        _label.Visibility = string.IsNullOrWhiteSpace(Label) ? Visibility.Collapsed : Visibility.Visible;
        _content.HorizontalAlignment = HorizontalAlignment.Center;
    }
}
