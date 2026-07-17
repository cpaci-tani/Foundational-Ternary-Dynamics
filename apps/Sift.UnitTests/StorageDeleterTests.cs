using Sift.Services;

namespace Sift.UnitTests;

public sealed class StorageDeleterTests
{
    [Fact]
    public void Preflight_allows_a_normal_file_on_a_fixed_volume()
    {
        var path = Path.Combine(Path.GetTempPath(), "sift-storage-deleter-" + Guid.NewGuid().ToString("N") + ".tmp");
        File.WriteAllText(path, "content");
        try
        {
            // The temp directory is on a fixed volume, so the Recycle-Bin availability guard must not
            // block it; a dry run reports it as recyclable rather than skipped.
            var result = new StorageDeleter().MoveToRecycleBin([path], dryRun: true);

            Assert.Equal(1, result.Deleted);
            Assert.Equal(0, result.Skipped);
            Assert.DoesNotContain(result.Log, line => line.Contains("cannot recycle", StringComparison.OrdinalIgnoreCase));
        }
        finally
        {
            if (File.Exists(path)) File.Delete(path);
        }
    }
}
