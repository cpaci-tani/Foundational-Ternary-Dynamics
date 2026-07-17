using System.Runtime.InteropServices;

namespace Sift.Services;

public readonly record struct SystemMemorySnapshot(double Percent, double UsedGb, double TotalGb);

public static class SystemMemoryReader
{
    public static SystemMemorySnapshot Read()
    {
        var status = new MemoryStatus { Length = (uint)Marshal.SizeOf<MemoryStatus>() };
        if (!GlobalMemoryStatusEx(ref status) || status.TotalPhysical == 0)
            return new SystemMemorySnapshot(0, 0, 0);

        var total = status.TotalPhysical / 1073741824d;
        var available = status.AvailablePhysical / 1073741824d;
        return new SystemMemorySnapshot(status.MemoryLoad, total - available, total);
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    private struct MemoryStatus
    {
        public uint Length;
        public uint MemoryLoad;
        public ulong TotalPhysical;
        public ulong AvailablePhysical;
        public ulong TotalPageFile;
        public ulong AvailablePageFile;
        public ulong TotalVirtual;
        public ulong AvailableVirtual;
        public ulong AvailableExtendedVirtual;
    }

    [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    private static extern bool GlobalMemoryStatusEx(ref MemoryStatus status);
}
