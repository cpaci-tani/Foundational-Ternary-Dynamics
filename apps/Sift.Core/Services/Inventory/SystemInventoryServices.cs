using Sift.Models;

namespace Sift.Services;

public interface IHealthInventory
{
    IReadOnlyList<HealthCheckRow> Scan();
}

public interface IServiceInventory
{
    IReadOnlyList<ServiceInfo> Enumerate();
}

public interface IScheduledTaskInventory
{
    IReadOnlyList<ScheduledTaskInfo> Enumerate();
}

public interface IStartupInventory
{
    IReadOnlyList<StartupEnumerator.StartupEntry> Enumerate();
}

public sealed class HealthInventory : IHealthInventory
{
    public IReadOnlyList<HealthCheckRow> Scan() => HealthScanner.Scan();
}

public sealed class ServiceInventory : IServiceInventory
{
    public IReadOnlyList<ServiceInfo> Enumerate() => WindowsServiceMonitor.Enumerate();
}

public sealed class ScheduledTaskInventory : IScheduledTaskInventory
{
    public IReadOnlyList<ScheduledTaskInfo> Enumerate() => ScheduledTaskMonitor.Enumerate();
}

public sealed class StartupInventory : IStartupInventory
{
    public IReadOnlyList<StartupEnumerator.StartupEntry> Enumerate() => StartupEnumerator.Enumerate();
}
