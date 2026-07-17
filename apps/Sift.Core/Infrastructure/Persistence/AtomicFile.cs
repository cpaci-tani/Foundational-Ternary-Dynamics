using System.Text;
using System.IO;

namespace Sift.Infrastructure.Persistence;

public static class AtomicFile
{
    public static void WriteAllText(string path, string contents)
    {
        var directory = Path.GetDirectoryName(path) ?? throw new ArgumentException("A parent directory is required.", nameof(path));
        Directory.CreateDirectory(directory);
        var temporary = Path.Combine(directory, $".{Path.GetFileName(path)}.{Guid.NewGuid():N}.tmp");
        try
        {
            var bytes = new UTF8Encoding(encoderShouldEmitUTF8Identifier: false).GetBytes(contents);
            using (var stream = new FileStream(temporary, FileMode.CreateNew, FileAccess.Write, FileShare.None))
            {
                stream.Write(bytes, 0, bytes.Length);
                // Flush the temp file's data to disk before the rename. Without this, NTFS can commit
                // the rename metadata while the data blocks are still unwritten, so a crash or power
                // loss would leave the destination pointing at a zero-length/torn file.
                stream.Flush(flushToDisk: true);
            }
            File.Move(temporary, path, overwrite: true);
        }
        finally
        {
            try { if (File.Exists(temporary)) File.Delete(temporary); }
            catch { /* Best-effort cleanup; never hide the original persistence failure. */ }
        }
    }
}
