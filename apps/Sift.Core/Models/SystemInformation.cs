namespace Sift.Models;

public sealed record SystemInfoItem(
    string Category,
    string Component,
    string Property,
    string Value,
    string Source)
{
    public string SearchText => $"{Category}\u001f{Component}\u001f{Property}\u001f{Value}\u001f{Source}";
}

public sealed record SystemInformationReport(
    string DeviceName,
    string DeviceModel,
    string WindowsVersion,
    string Processor,
    string Memory,
    string Architecture,
    DateTime GeneratedLocal,
    IReadOnlyList<SystemInfoItem> Items,
    IReadOnlyList<string> Warnings)
{
    public IReadOnlyList<string> Categories => Items
        .Select(item => item.Category)
        .Distinct(StringComparer.OrdinalIgnoreCase)
        .ToList();
}
