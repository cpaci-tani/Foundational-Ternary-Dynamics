using Sift.Models;

namespace Sift.Services;

public interface IHardwareSensorProvider : IDisposable
{
    string Id { get; }
    string Name { get; }
    IReadOnlyList<HardwareDeviceSnapshot> Sample(CancellationToken cancellationToken = default);
}

public interface IHardwareMonitorService : IDisposable
{
    HardwareMonitorSnapshot Sample(CancellationToken cancellationToken = default);
}
