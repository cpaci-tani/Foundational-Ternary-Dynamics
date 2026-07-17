using System.IO;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Media.Imaging;
using Windows.Storage.Streams;

namespace Sift.WinUI.Infrastructure.Converters;

/// <summary>
/// Converts a PNG byte buffer (an application icon thumbnail extracted in Core) into a
/// <see cref="BitmapImage"/> for display in a table cell. Returns null when there is no icon so the
/// cell renders empty. Runs on the UI thread and never throws; decoding is capped to a small size.
/// </summary>
public sealed class PngToImageSourceConverter : IValueConverter
{
    public object? Convert(object value, Type targetType, object parameter, string language)
    {
        if (value is not byte[] png || png.Length == 0) return null;
        try
        {
            var stream = new InMemoryRandomAccessStream();
            var output = stream.AsStreamForWrite();
            output.Write(png, 0, png.Length);
            output.Flush();
            stream.Seek(0);
            var image = new BitmapImage { DecodePixelWidth = 48, DecodePixelHeight = 48 };
            image.SetSource(stream);
            return image;
        }
        catch
        {
            return null;
        }
    }

    public object ConvertBack(object value, Type targetType, object parameter, string language) =>
        throw new NotSupportedException();
}
