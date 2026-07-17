using Sift.Models;
using Sift.Services;

namespace Sift.WinUI.Models;

public sealed record TaskManagerInventory(
    SystemSnapshot System,
    IReadOnlyList<ServiceInfo> Services,
    IReadOnlyList<ScheduledTaskInfo> Tasks);

public sealed record TaskProcessRow(
    string Name,
    int Id,
    string CpuLabel,
    string MemoryLabel,
    int ThreadCount,
    string Architecture,
    string Status,
    string ExecutablePath,
    int SessionId,
    long StartTimeUtcTicks,
    byte[]? IconPng)
{
    public static TaskProcessRow From(ProcessSnapshot value) => new(
        value.Name, value.Id, $"{value.CpuPercent:0.0}%", $"{value.MemoryMb:0} MB",
        value.ThreadCount, value.Architecture, value.Status, value.ExecutablePath, value.SessionId,
        value.StartTimeUtcTicks,
        value.IconPng);
}
