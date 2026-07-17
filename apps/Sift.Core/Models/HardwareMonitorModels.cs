namespace Sift.Models;

public sealed record HardwareSensorReading(
    string Id,
    string Name,
    string Type,
    double Value,
    double? Minimum,
    double? Maximum,
    string Unit)
{
    public string ValueLabel => Format(Value);
    public string MinimumLabel => Minimum is { } value ? Format(value) : "—";
    public string MaximumLabel => Maximum is { } value ? Format(value) : "—";

    public string RangeLabel => Minimum is { } min && Maximum is { } max
        ? $"min {Format(min)} · max {Format(max)}"
        : Minimum is { } onlyMin
            ? $"min {Format(onlyMin)}"
            : Maximum is { } onlyMax
                ? $"max {Format(onlyMax)}"
                : string.Empty;

    private string Format(double value) => Unit switch
    {
        "%" => $"{value:0.0}%",
        "°C" => $"{value:0.0} °C",
        "RPM" => $"{value:0} RPM",
        "MHz" => value >= 1_000 ? $"{value / 1_000:0.00} GHz" : $"{value:0} MHz",
        "Hz" => value >= 1_000_000_000 ? $"{value / 1_000_000_000:0.00} GHz"
            : value >= 1_000_000 ? $"{value / 1_000_000:0.00} MHz"
            : value >= 1_000 ? $"{value / 1_000:0.00} kHz"
            : $"{value:0} Hz",
        "GB" => $"{value:0.00} GB",
        "MB" => $"{value:0.0} MB",
        "MB/s" or "MiB/s" => $"{value:0.0} MiB/s",
        "V" => $"{value:0.000} V",
        "W" => $"{value:0.0} W",
        "A" => $"{value:0.00} A",
        "s" => value >= 60 ? $"{value / 60:0.0} min" : $"{value:0.##} s",
        _ => $"{value:0.##}{(Unit.Length == 0 ? "" : $" {Unit}")}"
    };
}

public sealed record HardwareDeviceSnapshot(
    string Id,
    string Name,
    string Type,
    IReadOnlyList<HardwareSensorReading> Sensors);

public sealed record HardwareProviderStatus(
    string Id,
    string Name,
    bool Available,
    string Detail,
    TimeSpan Duration);

public sealed record HardwareMonitorSnapshot(
    DateTimeOffset Timestamp,
    IReadOnlyList<HardwareDeviceSnapshot> Devices,
    IReadOnlyList<HardwareProviderStatus> Providers,
    bool IsElevated)
{
    public int SensorCount => Devices.Sum(device => device.Sensors.Count);
}
