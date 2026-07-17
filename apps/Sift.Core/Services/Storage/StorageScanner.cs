using System.Collections.Concurrent;
using System.IO;
using System.Runtime.InteropServices;
using Sift.Models;
using Microsoft.Win32.SafeHandles;

namespace Sift.Services;

public sealed class StorageScanner : IStorageScanner
{
    private const int FindFirstExLargeFetch = 2;
    private const int FileAttributeDirectory = 0x10;
    private const int FileAttributeReparsePoint = 0x400;

    public static IReadOnlyList<string> DefaultFixedDriveRoots()
    {
        return DriveInfo.GetDrives()
            .Where(d => d.DriveType == DriveType.Fixed && d.IsReady)
            .Select(d => d.RootDirectory.FullName.TrimEnd('\\') + "\\")
            .OrderBy(x => x, StringComparer.OrdinalIgnoreCase)
            .ToList();
    }

    public static IReadOnlyList<string> ListCandidateRoots()
    {
        var list = new List<string>();
        foreach (var d in DriveInfo.GetDrives().Where(x => x.IsReady))
        {
            var root = d.RootDirectory.FullName.TrimEnd('\\') + "\\";
            var label = d.DriveType switch
            {
                DriveType.Fixed => "Fixed",
                DriveType.Removable => "Removable",
                DriveType.Network => "Network",
                DriveType.CDRom => "Optical",
                _ => d.DriveType.ToString()
            };
            list.Add(root);
            _ = label;
        }
        return list;
    }

    public Task<StorageTree> ScanAsync(IReadOnlyList<string> roots, IProgress<StorageScanProgress>? progress, CancellationToken ct) =>
        Task.Run(() => Scan(roots, progress, ct), ct);

    public StorageTree Scan(IReadOnlyList<string> roots, IProgress<StorageScanProgress>? progress, CancellationToken ct)
    {
        var tree = new StorageTree();
        var counters = new long[2]; // 0=files, 1=bytes
        var gate = new object();
        var lastReport = 0L;
        void Report(string path)
        {
            var now = Environment.TickCount64;
            if (now - lastReport < 200 && counters[0] > 0) return;
            lastReport = now;
            progress?.Report(new StorageScanProgress
            {
                CurrentPath = path,
                FilesSeen = Volatile.Read(ref counters[0]),
                BytesSeen = Volatile.Read(ref counters[1])
            });
        }

        foreach (var root in roots.Distinct(StringComparer.OrdinalIgnoreCase))
        {
            ct.ThrowIfCancellationRequested();
            if (!Directory.Exists(root) && !IsDriveRoot(root)) continue;

            if (ElevationHelper.IsElevated() && TryScanVolumeViaUsn(root, tree, counters, progress, ct))
                continue;

            var rootNode = tree.Add(
                IsDriveRoot(root) ? root.TrimEnd('\\') : Path.GetFileName(root.TrimEnd('\\')),
                NormalizeDisplayPath(root),
                -1,
                isDirectory: true,
                isReparse: false,
                ownSize: 0,
                lastWriteUtc: Directory.GetLastWriteTimeUtc(root));

            WalkParallel(tree, rootNode.Index, NormalizeLongPath(root), gate, counters, Report, ct);
        }

        tree.Rollup();
        progress?.Report(new StorageScanProgress
        {
            CurrentPath = "Done",
            FilesSeen = counters[0],
            BytesSeen = tree.TotalSize
        });
        return tree;
    }

    private void WalkParallel(
        StorageTree tree,
        int rootIndex,
        string longRoot,
        object gate,
        long[] counters,
        Action<string> report,
        CancellationToken ct)
    {
        var queue = new ConcurrentQueue<(int parentIndex, string longPath)>();
        queue.Enqueue((rootIndex, longRoot));
        var pending = 1;
        var workers = Math.Clamp(Environment.ProcessorCount, 2, 16);

        void Worker()
        {
            while (true)
            {
                while (queue.TryDequeue(out var item))
                {
                    ct.ThrowIfCancellationRequested();
                    try
                    {
                        EnumerateDirectory(tree, item.parentIndex, item.longPath, queue, ref pending, gate, counters, report, ct);
                    }
                    finally
                    {
                        Interlocked.Decrement(ref pending);
                    }
                }

                if (Volatile.Read(ref pending) <= 0) return;
                Thread.SpinWait(40);
                if (Volatile.Read(ref pending) <= 0 && queue.IsEmpty) return;
            }
        }

        Parallel.For(0, workers, new ParallelOptions { CancellationToken = ct }, _ => Worker());
    }

    private void EnumerateDirectory(
        StorageTree tree,
        int parentIndex,
        string longPath,
        ConcurrentQueue<(int parentIndex, string longPath)> queue,
        ref int pending,
        object gate,
        long[] counters,
        Action<string> report,
        CancellationToken ct)
    {
        var search = longPath.TrimEnd('\\') + "\\*";
        var handle = FindFirstFileExW(search, FINDEX_INFO_LEVELS.FindExInfoBasic, out var data,
            FINDEX_SEARCH_OPS.FindExSearchNameMatch, IntPtr.Zero, FindFirstExLargeFetch);
        if (handle == new IntPtr(-1) || handle == IntPtr.Zero)
        {
            try
            {
                foreach (var entry in Directory.EnumerateFileSystemEntries(ToManagedPath(longPath)))
                {
                    ct.ThrowIfCancellationRequested();
                    AddEntryFromManaged(tree, parentIndex, entry, queue, ref pending, gate, counters, report);
                }
            }
            catch (UnauthorizedAccessException) { }
            catch (IOException) { }
            return;
        }

        try
        {
            do
            {
                ct.ThrowIfCancellationRequested();
                var name = data.cFileName;
                if (name is "." or "..") continue;
                var attrs = data.dwFileAttributes;
                var isDir = (attrs & FileAttributeDirectory) != 0;
                var isReparse = (attrs & FileAttributeReparsePoint) != 0;
                var childLong = longPath.TrimEnd('\\') + "\\" + name;
                var display = ToManagedPath(childLong);
                var size = isDir ? 0L : ((long)data.nFileSizeHigh << 32) | data.nFileSizeLow;
                var write = DateTime.FromFileTimeUtc(((long)data.ftLastWriteTime.dwHighDateTime << 32) | (uint)data.ftLastWriteTime.dwLowDateTime);

                int childIndex;
                lock (gate)
                {
                    var node = tree.Add(name, display, parentIndex, isDir, isReparse, size, write);
                    childIndex = node.Index;
                }

                if (!isDir)
                {
                    Interlocked.Increment(ref counters[0]);
                    Interlocked.Add(ref counters[1], size);
                    report(display);
                }
                else if (!isReparse)
                {
                    Interlocked.Increment(ref pending);
                    queue.Enqueue((childIndex, childLong));
                    report(display);
                }
            }
            while (FindNextFileW(handle, out data));
        }
        finally
        {
            FindClose(handle);
        }
    }

    private static void AddEntryFromManaged(
        StorageTree tree,
        int parentIndex,
        string entry,
        ConcurrentQueue<(int parentIndex, string longPath)> queue,
        ref int pending,
        object gate,
        long[] counters,
        Action<string> report)
    {
        try
        {
            var name = Path.GetFileName(entry);
            var attrs = File.GetAttributes(entry);
            var isDir = attrs.HasFlag(FileAttributes.Directory);
            var isReparse = attrs.HasFlag(FileAttributes.ReparsePoint);
            long size = 0;
            DateTime write;
            if (isDir)
            {
                write = Directory.GetLastWriteTimeUtc(entry);
            }
            else
            {
                var info = new FileInfo(entry);
                size = info.Length;
                write = info.LastWriteTimeUtc;
            }

            int childIndex;
            lock (gate)
            {
                var node = tree.Add(name, entry, parentIndex, isDir, isReparse, size, write);
                childIndex = node.Index;
            }

            if (!isDir)
            {
                Interlocked.Increment(ref counters[0]);
                Interlocked.Add(ref counters[1], size);
                report(entry);
            }
            else if (!isReparse)
            {
                Interlocked.Increment(ref pending);
                queue.Enqueue((childIndex, NormalizeLongPath(entry)));
                report(entry);
            }
        }
        catch { /* skip inaccessible */ }
    }

    private bool TryScanVolumeViaUsn(
        string root,
        StorageTree tree,
        long[] counters,
        IProgress<StorageScanProgress>? progress,
        CancellationToken ct)
    {
        if (!IsDriveRoot(root)) return false;
        try
        {
            var rootPath = root.TrimEnd('\\') + "\\";
            var drive = new DriveInfo(rootPath);
            if (!string.Equals(drive.DriveFormat, "NTFS", StringComparison.OrdinalIgnoreCase)) return false;

            var volumePath = @"\\.\" + root.TrimEnd('\\');
            using var handle = CreateFileW(volumePath, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete,
                IntPtr.Zero, FileMode.Open, 0, IntPtr.Zero);
            if (handle.IsInvalid) return false;

            var journal = new USN_JOURNAL_DATA_V0();
            var ok = DeviceIoControl(handle, FSCTL_QUERY_USN_JOURNAL, IntPtr.Zero, 0,
                ref journal, Marshal.SizeOf<USN_JOURNAL_DATA_V0>(), out _, IntPtr.Zero);
            if (!ok) return false;

            _ = tree;
            progress?.Report(new StorageScanProgress { CurrentPath = rootPath + " (NTFS)", FilesSeen = counters[0], BytesSeen = counters[1] });
            ct.ThrowIfCancellationRequested();
            return false;
        }
        catch
        {
            return false;
        }
    }

    public static string NormalizeLongPath(string path)
    {
        var full = Path.GetFullPath(path);
        if (full.StartsWith(@"\\?\", StringComparison.Ordinal)) return full.TrimEnd('\\');
        if (full.StartsWith(@"\\", StringComparison.Ordinal))
            return @"\\?\UNC\" + full.TrimStart('\\').TrimEnd('\\');
        return @"\\?\" + full.TrimEnd('\\');
    }

    public static string ToManagedPath(string longPath)
    {
        if (longPath.StartsWith(@"\\?\UNC\", StringComparison.OrdinalIgnoreCase))
            return @"\\" + longPath[8..];
        if (longPath.StartsWith(@"\\?\", StringComparison.OrdinalIgnoreCase))
            return longPath[4..];
        return longPath;
    }

    public static string NormalizeDisplayPath(string path)
    {
        var full = Path.GetFullPath(path);
        return IsDriveRoot(full) ? full.TrimEnd('\\') + "\\" : full.TrimEnd('\\');
    }

    public static bool IsDriveRoot(string path)
    {
        try
        {
            var full = Path.GetFullPath(path).TrimEnd('\\') + "\\";
            return full.Length == 3 && full[1] == ':';
        }
        catch { return false; }
    }

    private enum FINDEX_INFO_LEVELS { FindExInfoStandard = 0, FindExInfoBasic = 1 }
    private enum FINDEX_SEARCH_OPS { FindExSearchNameMatch = 0 }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct WIN32_FIND_DATA
    {
        public int dwFileAttributes;
        public FILETIME ftCreationTime;
        public FILETIME ftLastAccessTime;
        public FILETIME ftLastWriteTime;
        public uint nFileSizeHigh;
        public uint nFileSizeLow;
        public uint dwReserved0;
        public uint dwReserved1;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)] public string cFileName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 14)] public string cAlternateFileName;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct FILETIME
    {
        public uint dwLowDateTime;
        public uint dwHighDateTime;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct USN_JOURNAL_DATA_V0
    {
        public ulong UsnJournalID;
        public long FirstUsn;
        public long NextUsn;
        public long LowestValidUsn;
        public long MaxUsn;
        public ulong MaximumSize;
        public ulong AllocationDelta;
    }

    private const uint FSCTL_QUERY_USN_JOURNAL = 0x000900f4;

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern IntPtr FindFirstFileExW(string lpFileName, FINDEX_INFO_LEVELS fInfoLevelId,
        out WIN32_FIND_DATA lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, IntPtr lpSearchFilter, int dwAdditionalFlags);

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern bool FindNextFileW(IntPtr hFindFile, out WIN32_FIND_DATA lpFindFileData);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool FindClose(IntPtr hFindFile);

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern SafeFileHandle CreateFileW(string lpFileName, FileAccess dwDesiredAccess, FileShare dwShareMode,
        IntPtr lpSecurityAttributes, FileMode dwCreationDisposition, int dwFlagsAndAttributes, IntPtr hTemplateFile);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool DeviceIoControl(SafeFileHandle hDevice, uint dwIoControlCode, IntPtr lpInBuffer, int nInBufferSize,
        ref USN_JOURNAL_DATA_V0 lpOutBuffer, int nOutBufferSize, out int lpBytesReturned, IntPtr lpOverlapped);
}
