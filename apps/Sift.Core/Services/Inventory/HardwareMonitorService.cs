using System.Diagnostics;
using LibreHardwareMonitor.Hardware;
using Sift.Models;

namespace Sift.Services;

public sealed class HardwareMonitorService : IHardwareMonitorService
{
    private readonly IReadOnlyList<IHardwareSensorProvider> _providers;

    public HardwareMonitorService(IEnumerable<IHardwareSensorProvider>? providers = null) =>
        _providers = providers?.ToList() ?? [new LibreHardwareSensorProvider()];

    public HardwareMonitorSnapshot Sample(CancellationToken cancellationToken = default)
    {
        var devices = new List<HardwareDeviceSnapshot>();
        var statuses = new List<HardwareProviderStatus>();
        foreach (var provider in _providers)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var timer = Stopwatch.StartNew();
            try
            {
                var sampled = provider.Sample(cancellationToken)
                    .Select(DeduplicateSensors)
                    .ToList();
                devices.AddRange(sampled);
                statuses.Add(new HardwareProviderStatus(provider.Id, provider.Name, true,
                    $"{sampled.Count:N0} devices · {sampled.Sum(device => device.Sensors.Count):N0} sensors", timer.Elapsed));
            }
            catch (Exception exception) when (exception is not OperationCanceledException)
            {
                statuses.Add(new HardwareProviderStatus(provider.Id, provider.Name, false, exception.Message, timer.Elapsed));
            }
        }

        return new HardwareMonitorSnapshot(DateTimeOffset.Now,
            devices.OrderBy(device => DeviceOrder(device.Type)).ThenBy(device => device.Name).ToList(),
            statuses, ElevationHelper.IsElevated());
    }

    private static HardwareDeviceSnapshot DeduplicateSensors(HardwareDeviceSnapshot device)
    {
        var sensors = device.Sensors
            .GroupBy(sensor => sensor.Id, StringComparer.Ordinal)
            .Select(group => group.First())
            .ToList();
        return sensors.Count == device.Sensors.Count
            ? device
            : device with { Sensors = sensors };
    }

    public void Dispose()
    {
        foreach (var provider in _providers)
        {
            try { provider.Dispose(); }
            catch { /* Provider cleanup must not prevent the remaining providers from closing. */ }
        }
    }

    private static int DeviceOrder(string type) => type switch
    {
        "Cpu" => 0,
        "GpuNvidia" or "GpuAmd" or "GpuIntel" => 1,
        "Memory" => 2,
        "Motherboard" or "SuperIO" => 3,
        "Storage" => 4,
        "Network" => 5,
        "Battery" => 6,
        _ => 10
    };
}

public sealed class LibreHardwareSensorProvider : IHardwareSensorProvider
{
    private readonly object _gate = new();
    private readonly Func<bool> _isElevated;
    private Computer? _computer;
    private bool _disposed;

    public LibreHardwareSensorProvider(Func<bool>? isElevated = null) =>
        _isElevated = isElevated ?? ElevationHelper.IsElevated;

    public string Id => "libre-hardware-monitor";
    public string Name => "LibreHardwareMonitor 0.9.6";

    public IReadOnlyList<HardwareDeviceSnapshot> Sample(CancellationToken cancellationToken = default)
    {
        if (_isElevated())
            throw new InvalidOperationException(
                "Hardware sensors are unavailable in administrator sessions until driver access is explicitly enabled.");
        lock (_gate)
        {
            ObjectDisposedException.ThrowIf(_disposed, this);
            var computer = EnsureOpen();
            var devices = new List<HardwareDeviceSnapshot>();
            foreach (var hardware in computer.Hardware)
                Collect(hardware, devices, cancellationToken);
            return devices;
        }
    }

    public void Dispose()
    {
        lock (_gate)
        {
            if (_disposed) return;
            _disposed = true;
            _computer?.Close();
            _computer = null;
        }
    }

    private Computer EnsureOpen()
    {
        if (_computer is not null) return _computer;
        var computer = CreateComputer();
        computer.Open();
        _computer = computer;
        return computer;
    }

    private static Computer CreateComputer() => new()
        {
            IsBatteryEnabled = true,
            IsControllerEnabled = true,
            IsCpuEnabled = true,
            IsGpuEnabled = true,
            IsMemoryEnabled = true,
            IsMotherboardEnabled = true,
            IsNetworkEnabled = true,
            IsPsuEnabled = true,
            IsStorageEnabled = true
        };

    private static void Collect(IHardware hardware, ICollection<HardwareDeviceSnapshot> destination, CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        hardware.Update();
        // LibreHardwareMonitor can emit duplicate Identifier values for some GPU load sensors.
        // Keep the first finite reading per id so UI dictionaries and elevation-safe sampling never throw.
        var sensors = hardware.Sensors
            .Where(sensor => sensor.Value.HasValue && float.IsFinite(sensor.Value.Value))
            .Select(ToReading)
            .GroupBy(sensor => sensor.Id, StringComparer.Ordinal)
            .Select(group => group.First())
            .OrderBy(sensor => SensorOrder(sensor.Type))
            .ThenBy(sensor => sensor.Name)
            .ToList();
        if (sensors.Count > 0)
            destination.Add(new HardwareDeviceSnapshot(hardware.Identifier.ToString(), hardware.Name,
                hardware.HardwareType.ToString(), sensors));
        foreach (var child in hardware.SubHardware) Collect(child, destination, cancellationToken);
    }

    private static HardwareSensorReading ToReading(ISensor sensor)
    {
        var type = sensor.SensorType.ToString();
        return new HardwareSensorReading(sensor.Identifier.ToString(), sensor.Name, type, sensor.Value!.Value,
            sensor.Min, sensor.Max, UnitFor(type));
    }

    private static string UnitFor(string type) => type switch
    {
        "Temperature" => "°C",
        "Load" or "Control" or "Level" => "%",
        "Clock" => "MHz",
        "Fan" => "RPM",
        "Voltage" => "V",
        "Current" => "A",
        "Power" => "W",
        "Data" => "GB",
        "SmallData" => "MB",
        "Throughput" => "MiB/s",
        "Frequency" => "Hz",
        "TimeSpan" => "s",
        _ => string.Empty
    };

    private static int SensorOrder(string type) => type switch
    {
        "Temperature" => 0,
        "Load" => 1,
        "Power" => 2,
        "Fan" => 3,
        "Clock" => 4,
        "Voltage" => 5,
        _ => 10
    };
}
