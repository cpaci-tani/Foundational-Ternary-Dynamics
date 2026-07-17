using System.Runtime.InteropServices;

namespace Sift.WinUI.Infrastructure.Windowing;

public sealed class WindowMinimumSize : IDisposable
{
    private const uint WmGetMinMaxInfo = 0x0024;
    private readonly IntPtr _window;
    private readonly nuint _subclassId;
    private readonly SubclassProcedure _procedure;
    private readonly int _minimumWidthDip;
    private readonly int _minimumHeightDip;
    private bool _disposed;

    private WindowMinimumSize(IntPtr window, int minimumWidthDip, int minimumHeightDip)
    {
        _window = window;
        _minimumWidthDip = minimumWidthDip;
        _minimumHeightDip = minimumHeightDip;
        _subclassId = 0x434C5257; // CLRW
        _procedure = WindowProcedure;
        if (!SetWindowSubclass(window, _procedure, _subclassId, 0))
            throw new InvalidOperationException("Could not install the Sift minimum-size window policy.");
    }

    public static WindowMinimumSize Attach(IntPtr window, int minimumWidthDip, int minimumHeightDip) =>
        new(window, minimumWidthDip, minimumHeightDip);

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        RemoveWindowSubclass(_window, _procedure, _subclassId);
    }

    private IntPtr WindowProcedure(IntPtr window, uint message, IntPtr wParam, IntPtr lParam, nuint id, nuint data)
    {
        if (message == WmGetMinMaxInfo)
        {
            var info = Marshal.PtrToStructure<MinMaxInfo>(lParam);
            var dpi = GetDpiForWindow(window);
            var scale = (dpi == 0 ? 96 : dpi) / 96d;
            info.MinimumTrackSize.X = Math.Max(info.MinimumTrackSize.X, (int)Math.Ceiling(_minimumWidthDip * scale));
            info.MinimumTrackSize.Y = Math.Max(info.MinimumTrackSize.Y, (int)Math.Ceiling(_minimumHeightDip * scale));
            Marshal.StructureToPtr(info, lParam, false);
        }
        return DefSubclassProc(window, message, wParam, lParam);
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct Point
    {
        public int X;
        public int Y;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct MinMaxInfo
    {
        public Point Reserved;
        public Point MaximumSize;
        public Point MaximumPosition;
        public Point MinimumTrackSize;
        public Point MaximumTrackSize;
    }

    private delegate IntPtr SubclassProcedure(IntPtr window, uint message, IntPtr wParam, IntPtr lParam, nuint id, nuint data);

    [DllImport("comctl32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool SetWindowSubclass(IntPtr window, SubclassProcedure procedure, nuint id, nuint data);

    [DllImport("comctl32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool RemoveWindowSubclass(IntPtr window, SubclassProcedure procedure, nuint id);

    [DllImport("comctl32.dll")]
    private static extern IntPtr DefSubclassProc(IntPtr window, uint message, IntPtr wParam, IntPtr lParam);

    [DllImport("user32.dll")]
    private static extern uint GetDpiForWindow(IntPtr window);
}
