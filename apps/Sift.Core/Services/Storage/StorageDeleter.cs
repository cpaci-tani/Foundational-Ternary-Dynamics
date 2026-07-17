using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using Sift.Models;

namespace Sift.Services;

public sealed class StorageDeleteResult
{
    public List<string> Log { get; init; } = [];
    public int Deleted { get; init; }
    public int Skipped { get; init; }
    public int Failed { get; init; }
    public string Summary => $"Deleted {Deleted}, skipped {Skipped}, failed {Failed}.";
}

public sealed class StorageDeleter : IStorageDeleter
{
    private static readonly string SystemRoot = Path.GetFullPath(Environment.GetFolderPath(Environment.SpecialFolder.Windows)).TrimEnd('\\');
    private static readonly string SiftDir = Path.GetFullPath(AppContext.BaseDirectory).TrimEnd('\\');

    public bool IsProtected(string path, out string reason)
    {
        reason = "";
        try
        {
            var full = Path.GetFullPath(path).TrimEnd('\\');
            if (StorageScanner.IsDriveRoot(full + "\\") || StorageScanner.IsDriveRoot(full))
            {
                reason = "drive roots cannot be deleted";
                return true;
            }

            if (IsUnderOrEqual(full, SystemRoot))
            {
                reason = "Windows system paths are protected";
                return true;
            }

            if (IsUnderOrEqual(full, SiftDir) || string.Equals(full, Path.GetFullPath(Environment.ProcessPath ?? ""), StringComparison.OrdinalIgnoreCase))
            {
                reason = "Sift files are protected";
                return true;
            }

            var attrs = File.GetAttributes(full);
            if (attrs.HasFlag(FileAttributes.ReparsePoint))
            {
                reason = "reparse points / junctions are blocked";
                return true;
            }

            return false;
        }
        catch (Exception ex)
        {
            reason = ex.Message;
            return true;
        }
    }

    public StorageDeleteResult MoveToRecycleBin(IEnumerable<string> paths, bool dryRun)
    {
        var log = new List<string>();
        var deleted = 0;
        var skipped = 0;
        var failed = 0;

        foreach (var path in paths.Distinct(StringComparer.OrdinalIgnoreCase).OrderByDescending(x => x.Length))
        {
            if (IsProtected(path, out var reason))
            {
                skipped++;
                log.Add($"SKIPPED  {path} ({reason})");
                continue;
            }

            if (!CanRecycle(path, out var recycleReason))
            {
                skipped++;
                log.Add($"SKIPPED  {path} (cannot recycle: {recycleReason})");
                continue;
            }

            if (dryRun)
            {
                log.Add($"PREFLIGHT  {path} · Recycle Bin");
                deleted++;
                continue;
            }

            try
            {
                if (ShellDelete(path))
                {
                    deleted++;
                    log.Add($"DELETED  {path} · Recycle Bin");
                }
                else
                {
                    failed++;
                    log.Add($"FAILED   {path}");
                }
            }
            catch (Exception ex)
            {
                failed++;
                log.Add($"FAILED   {path}: {ex.Message}");
            }
        }

        return new StorageDeleteResult { Log = log, Deleted = deleted, Skipped = skipped, Failed = failed };
    }

    // SHFileOperation's FOF_ALLOWUNDO is best-effort: on a volume with no Recycle Bin (removable,
    // network, or a bin disabled/absent for the volume) it permanently deletes. Block the volumes we
    // can detect deterministically so we never silently and permanently delete while claiming a
    // Recycle-Bin move; the FOF_WANTNUKEWARNING flag catches the remaining cases (e.g. an item larger
    // than the bin, or a bin disabled by policy on a fixed volume) with an explicit user warning.
    private static bool CanRecycle(string fullPath, out string reason)
    {
        reason = "";
        try
        {
            var root = Path.GetPathRoot(Path.GetFullPath(fullPath));
            if (string.IsNullOrEmpty(root))
            {
                reason = "the target volume could not be determined";
                return false;
            }
            var drive = new DriveInfo(root);
            if (drive.DriveType != DriveType.Fixed)
            {
                reason = $"the Recycle Bin is not available on this {drive.DriveType.ToString().ToLowerInvariant()} volume";
                return false;
            }
            return true;
        }
        catch (Exception exception)
        {
            reason = exception.Message;
            return false;
        }
    }

    private static bool IsUnderOrEqual(string path, string root)
    {
        var p = path.TrimEnd('\\') + "\\";
        var r = root.TrimEnd('\\') + "\\";
        return p.StartsWith(r, StringComparison.OrdinalIgnoreCase) ||
               path.TrimEnd('\\').Equals(root.TrimEnd('\\'), StringComparison.OrdinalIgnoreCase);
    }

    private static bool ShellDelete(string path)
    {
        var full = Path.GetFullPath(path);
        // SHFileOperation requires double-null-terminated string.
        var from = full + "\0\0";
        var op = new SHFILEOPSTRUCT
        {
            wFunc = FO_DELETE,
            pFrom = from,
            // FOF_WANTNUKEWARNING partially overrides FOF_NOCONFIRMATION: a routine recycle proceeds
            // silently, but if the shell would permanently destroy the item instead of recycling it,
            // Windows warns first. If the user declines, fAnyOperationsAborted is set and we report a
            // failure rather than falsely claiming a Recycle-Bin move.
            fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO | FOF_WANTNUKEWARNING
        };
        var result = SHFileOperation(ref op);
        return result == 0 && !op.fAnyOperationsAborted;
    }

    private const int FO_DELETE = 0x0003;
    private const ushort FOF_ALLOWUNDO = 0x0040;
    private const ushort FOF_NOCONFIRMATION = 0x0010;
    private const ushort FOF_NOERRORUI = 0x0400;
    private const ushort FOF_SILENT = 0x0004;
    private const ushort FOF_WANTNUKEWARNING = 0x4000;

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct SHFILEOPSTRUCT
    {
        public IntPtr hwnd;
        public int wFunc;
        [MarshalAs(UnmanagedType.LPWStr)] public string pFrom;
        [MarshalAs(UnmanagedType.LPWStr)] public string? pTo;
        public ushort fFlags;
        [MarshalAs(UnmanagedType.Bool)] public bool fAnyOperationsAborted;
        public IntPtr hNameMappings;
        [MarshalAs(UnmanagedType.LPWStr)] public string? lpszProgressTitle;
    }

    [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
    private static extern int SHFileOperation(ref SHFILEOPSTRUCT fileOp);
}
