using System.Runtime.InteropServices;

namespace Sift.MonitorHost;

internal sealed class TrayIcon : IDisposable
{
    private const int WmApp = 0x8000;
    private const int WmTray = WmApp + 1;
    private const int WmClose = 0x0010;
    private const int WmDestroy = 0x0002;
    private const int WmLButtonDblClk = 0x0203;
    private const int WmRButtonUp = 0x0205;
    private const uint NimAdd = 0;
    private const uint NimDelete = 2;
    private const uint NifMessage = 1;
    private const uint NifIcon = 2;
    private const uint NifTip = 4;
    private const uint MfString = 0;
    private const uint TpmReturnCmd = 0x0100;
    private readonly Action _open;
    private readonly Action _settings;
    private readonly Action _pauseHour;
    private readonly Action _pause;
    private readonly Action _resume;
    private readonly Action _exit;
    private readonly WndProc _wndProc;
    private readonly string _className = "SiftMonitorTray" + Guid.NewGuid().ToString("N");
    private IntPtr _window;
    private bool _disposed;

    public TrayIcon(Action open, Action settings, Action pauseHour, Action pause, Action resume, Action exit)
    {
        _open = open;
        _settings = settings;
        _pauseHour = pauseHour;
        _pause = pause;
        _resume = resume;
        _exit = exit;
        _wndProc = WindowProc;
        var windowClass = new WndClass { lpfnWndProc = _wndProc, lpszClassName = _className };
        RegisterClass(ref windowClass);
        _window = CreateWindowEx(0, _className, "Sift Monitor", 0, 0, 0, 0, 0,
            IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero);
        var iconPath = Path.Combine(AppContext.BaseDirectory, "Sift.ico");
        var icon = File.Exists(iconPath) ? LoadImage(IntPtr.Zero, iconPath, 1, 0, 0, 0x0010) : IntPtr.Zero;
        var data = Data(icon);
        Shell_NotifyIcon(NimAdd, ref data);
    }

    public void RunMessageLoop(CancellationToken token)
    {
        using var registration = token.Register(() => PostMessage(_window, WmClose, IntPtr.Zero, IntPtr.Zero));
        while (GetMessage(out var message, IntPtr.Zero, 0, 0) > 0)
        {
            TranslateMessage(ref message);
            DispatchMessage(ref message);
        }
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        if (_window != IntPtr.Zero)
        {
            var data = Data(IntPtr.Zero);
            Shell_NotifyIcon(NimDelete, ref data);
            DestroyWindow(_window);
            _window = IntPtr.Zero;
        }
        UnregisterClass(_className, IntPtr.Zero);
    }

    private IntPtr WindowProc(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam)
    {
        if (message == WmTray)
        {
            var mouseMessage = unchecked((int)lParam.ToInt64());
            if (mouseMessage == WmLButtonDblClk) _open();
            else if (mouseMessage == WmRButtonUp) ShowMenu();
            return IntPtr.Zero;
        }
        if (message == WmClose)
        {
            _exit();
            DestroyWindow(hwnd);
            return IntPtr.Zero;
        }
        if (message == WmDestroy)
        {
            PostQuitMessage(0);
            return IntPtr.Zero;
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    private void ShowMenu()
    {
        var menu = CreatePopupMenu();
        try
        {
            AppendMenu(menu, MfString, 1, "Open Sift");
            AppendMenu(menu, MfString, 2, "Pause for one hour");
            AppendMenu(menu, MfString, 3, "Pause until resumed");
            AppendMenu(menu, MfString, 4, "Resume");
            AppendMenu(menu, MfString, 5, "Settings");
            AppendMenu(menu, MfString, 6, "Exit for this session");
            GetCursorPos(out var point);
            SetForegroundWindow(_window);
            var command = TrackPopupMenu(menu, TpmReturnCmd, point.X, point.Y, 0, _window, IntPtr.Zero);
            switch (command)
            {
                case 1: _open(); break;
                case 2: _pauseHour(); break;
                case 3: _pause(); break;
                case 4: _resume(); break;
                case 5: _settings(); break;
                case 6: _exit(); PostMessage(_window, WmClose, IntPtr.Zero, IntPtr.Zero); break;
            }
        }
        finally { DestroyMenu(menu); }
    }

    private NotifyIconData Data(IntPtr icon) => new()
    {
        cbSize = Marshal.SizeOf<NotifyIconData>(),
        hWnd = _window,
        uID = 1,
        uFlags = NifMessage | NifIcon | NifTip,
        uCallbackMessage = WmTray,
        hIcon = icon,
        szTip = "Sift background monitor"
    };

    private delegate IntPtr WndProc(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct WndClass
    {
        public uint style;
        public WndProc lpfnWndProc;
        public int cbClsExtra;
        public int cbWndExtra;
        public IntPtr hInstance;
        public IntPtr hIcon;
        public IntPtr hCursor;
        public IntPtr hbrBackground;
        public string? lpszMenuName;
        public string lpszClassName;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct NotifyIconData
    {
        public int cbSize;
        public IntPtr hWnd;
        public uint uID;
        public uint uFlags;
        public uint uCallbackMessage;
        public IntPtr hIcon;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)] public string szTip;
        public uint dwState;
        public uint dwStateMask;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)] public string szInfo;
        public uint uVersion;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)] public string szInfoTitle;
        public uint dwInfoFlags;
        public Guid guidItem;
        public IntPtr hBalloonIcon;
    }

    [StructLayout(LayoutKind.Sequential)] private struct Point { public int X; public int Y; }
    [StructLayout(LayoutKind.Sequential)] private struct Message { public IntPtr hwnd; public uint message; public IntPtr wParam; public IntPtr lParam; public uint time; public Point point; }

    [DllImport("user32.dll", CharSet = CharSet.Unicode)] private static extern ushort RegisterClass(ref WndClass value);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] private static extern bool UnregisterClass(string name, IntPtr instance);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] private static extern IntPtr CreateWindowEx(uint exStyle, string className, string windowName, uint style, int x, int y, int width, int height, IntPtr parent, IntPtr menu, IntPtr instance, IntPtr parameter);
    [DllImport("user32.dll")] private static extern bool DestroyWindow(IntPtr window);
    [DllImport("user32.dll")] private static extern IntPtr DefWindowProc(IntPtr window, uint message, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")] private static extern int GetMessage(out Message message, IntPtr window, uint minimum, uint maximum);
    [DllImport("user32.dll")] private static extern bool TranslateMessage(ref Message message);
    [DllImport("user32.dll")] private static extern IntPtr DispatchMessage(ref Message message);
    [DllImport("user32.dll")] private static extern void PostQuitMessage(int code);
    [DllImport("user32.dll")] private static extern bool PostMessage(IntPtr window, uint message, IntPtr wParam, IntPtr lParam);
    [DllImport("shell32.dll", CharSet = CharSet.Unicode)] private static extern bool Shell_NotifyIcon(uint message, ref NotifyIconData data);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] private static extern IntPtr LoadImage(IntPtr instance, string name, uint type, int width, int height, uint load);
    [DllImport("user32.dll")] private static extern IntPtr CreatePopupMenu();
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] private static extern bool AppendMenu(IntPtr menu, uint flags, uint id, string text);
    [DllImport("user32.dll")] private static extern uint TrackPopupMenu(IntPtr menu, uint flags, int x, int y, int reserved, IntPtr window, IntPtr rectangle);
    [DllImport("user32.dll")] private static extern bool DestroyMenu(IntPtr menu);
    [DllImport("user32.dll")] private static extern bool GetCursorPos(out Point point);
    [DllImport("user32.dll")] private static extern bool SetForegroundWindow(IntPtr window);
}
