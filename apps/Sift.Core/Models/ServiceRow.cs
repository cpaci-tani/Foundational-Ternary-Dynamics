namespace Sift.Models;

public sealed class ServiceRow
{
    public required string Name { get; init; }
    public required string DisplayName { get; init; }
    public required string Status { get; init; }
    public required string StartType { get; init; }
    public required bool IsProtected { get; init; }
    public required bool CanManage { get; init; }
    public required string GroupKey { get; init; }
    public byte[]? IconPng { get; init; }
    public string ManageLabel => CanManage ? "Yes" : "Protected";
}

public sealed class TaskRow
{
    public required string TaskName { get; init; }
    public required string TaskPath { get; init; }
    public required string State { get; init; }
    public required string Author { get; init; }
    public required bool IsAllowlisted { get; init; }
    public required string GroupKey { get; init; }
    public byte[]? IconPng { get; init; }
    public string FullPath => TaskPath.TrimEnd('\\') + "\\" + TaskName;
    public string AllowLabel => IsAllowlisted ? "Supported" : "View only";
}
