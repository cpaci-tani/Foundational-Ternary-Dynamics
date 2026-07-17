namespace Sift.Models;

public sealed record StorageSelectionDeletePreflight(
    bool CanDelete,
    string Summary,
    string Detail,
    string TicketId,
    string TargetPath,
    bool IsDirectory,
    long SizeBytes,
    int FileCount,
    int DirectoryCount,
    DateTime ExpiresUtc)
{
    public string SizeDisplay => StorageRow.FormatSize(SizeBytes);
    public string TypeDisplay => IsDirectory ? "Folder" : "File";
}

public sealed record StorageSelectionDeleteResult(
    bool Succeeded,
    string Summary,
    string TargetPath,
    IReadOnlyList<string> Log);
