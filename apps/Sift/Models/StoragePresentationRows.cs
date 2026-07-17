using Microsoft.UI.Xaml.Media;

namespace Sift.WinUI.Models;

public sealed record StorageMapRow(
    int NodeIndex,
    string Name,
    string Type,
    string Size,
    string Percent,
    string Path);

public sealed record StorageLegendRow(
    string Extension,
    string Size,
    Brush SwatchBrush)
{
    public string Tooltip => $"{Extension} files account for {Size} in this scan.";
}
