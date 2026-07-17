using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class HardwareMonitorServiceTests
{
    [Fact]
    public void Sample_AggregatesProvidersAndOrdersDevices()
    {
        using var service = new HardwareMonitorService([
            new StubProvider("extra", "Extra provider", [Device("/net/0", "Adapter", "Network", "Throughput", 12.5, "MB/s")]),
            new StubProvider("primary", "Primary provider", [Device("/cpu/0", "Processor", "Cpu", "Temperature", 51.25, "°C")])
        ]);

        var snapshot = service.Sample(TestContext.Current.CancellationToken);

        Assert.Equal(2, snapshot.SensorCount);
        Assert.Equal(["Processor", "Adapter"], snapshot.Devices.Select(device => device.Name));
        Assert.All(snapshot.Providers, status => Assert.True(status.Available));
    }

    [Fact]
    public void Sample_IsolatesUnavailableProvider()
    {
        using var service = new HardwareMonitorService([
            new ThrowingProvider(),
            new StubProvider("healthy", "Healthy provider", [Device("/gpu/0", "Graphics", "GpuNvidia", "Power", 80, "W")])
        ]);

        var snapshot = service.Sample(TestContext.Current.CancellationToken);

        Assert.Single(snapshot.Devices);
        Assert.False(snapshot.Providers.Single(status => status.Id == "failed").Available);
        Assert.True(snapshot.Providers.Single(status => status.Id == "healthy").Available);
    }

    [Fact]
    public void Dispose_ReleasesEveryProviderEvenWhenOneThrows()
    {
        var throwing = new ThrowingProvider(throwOnDispose: true);
        var healthy = new StubProvider("healthy", "Healthy provider", []);
        var service = new HardwareMonitorService([throwing, healthy]);

        service.Dispose();

        Assert.True(healthy.Disposed);
    }

    [Fact]
    public void PassiveLibreProvider_DoesNotOpenHardwareDriverInAdministratorSession()
    {
        using var provider = new LibreHardwareSensorProvider(() => true);
        using var service = new HardwareMonitorService([provider]);

        var snapshot = service.Sample(TestContext.Current.CancellationToken);

        Assert.Empty(snapshot.Devices);
        var status = Assert.Single(snapshot.Providers);
        Assert.False(status.Available);
        Assert.Contains("driver access is explicitly enabled", status.Detail, StringComparison.Ordinal);
    }

    [Fact]
    public void Sample_DeduplicatesSensorIdsWithinADevice()
    {
        using var service = new HardwareMonitorService([
            new StubProvider("gpu", "GPU",
            [
                new HardwareDeviceSnapshot("/gpu/0", "Graphics", "GpuNvidia",
                [
                    new HardwareSensorReading("/gpu-nvidia/0/load/3", "D3D 3D", "Load", 10, 10, 10, "%"),
                    new HardwareSensorReading("/gpu-nvidia/0/load/3", "D3D 3D duplicate", "Load", 20, 20, 20, "%"),
                    new HardwareSensorReading("/gpu-nvidia/0/load/4", "D3D Copy", "Load", 5, 5, 5, "%")
                ])
            ])
        ]);

        var snapshot = service.Sample(TestContext.Current.CancellationToken);
        var device = Assert.Single(snapshot.Devices);
        Assert.Equal(2, device.Sensors.Count);
        Assert.Equal(["/gpu-nvidia/0/load/3", "/gpu-nvidia/0/load/4"], device.Sensors.Select(s => s.Id));
        Assert.Equal(10, device.Sensors[0].Value);
    }

    private static HardwareDeviceSnapshot Device(
        string id, string name, string type, string sensorType, double value, string unit) =>
        new(id, name, type, [new HardwareSensorReading($"{id}/sensor", sensorType, sensorType, value, value, value, unit)]);

    private sealed class StubProvider(
        string id,
        string name,
        IReadOnlyList<HardwareDeviceSnapshot> devices) : IHardwareSensorProvider
    {
        public string Id => id;
        public string Name => name;
        public bool Disposed { get; private set; }
        public IReadOnlyList<HardwareDeviceSnapshot> Sample(CancellationToken cancellationToken = default) => devices;
        public void Dispose() => Disposed = true;
    }

    private sealed class ThrowingProvider(bool throwOnDispose = false) : IHardwareSensorProvider
    {
        public string Id => "failed";
        public string Name => "Failed provider";
        public IReadOnlyList<HardwareDeviceSnapshot> Sample(CancellationToken cancellationToken = default) =>
            throw new InvalidOperationException("Provider unavailable");
        public void Dispose()
        {
            if (throwOnDispose) throw new InvalidOperationException("Dispose failed");
        }
    }
}
