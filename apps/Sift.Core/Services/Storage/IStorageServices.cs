using Sift.Models;

namespace Sift.Services;

public interface IStorageScanner
{
    Task<StorageTree> ScanAsync(IReadOnlyList<string> roots, IProgress<StorageScanProgress>? progress, CancellationToken ct);
}

public interface IStorageDeleter
{
    bool IsProtected(string path, out string reason);
    StorageDeleteResult MoveToRecycleBin(IEnumerable<string> paths, bool dryRun = false);
}
