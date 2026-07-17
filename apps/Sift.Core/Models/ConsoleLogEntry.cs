namespace Sift.Models;

public sealed class ConsoleLogEntry
{
    public DateTime CreatedLocal { get; init; } = DateTime.Now;
    public string Level { get; init; } = "INFO";
    public string Category { get; init; } = "APP";
    public string Message { get; init; } = "";

    public string TimeLabel => CreatedLocal.ToString("HH:mm:ss.fff");
    public string Header => $"{TimeLabel}  {Level,-5}  {Category.ToUpperInvariant()}";
    public string CopyText => $"{Header}  {Message}";
}
