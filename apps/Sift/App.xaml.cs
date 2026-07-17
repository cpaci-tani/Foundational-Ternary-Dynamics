using Sift.WinUI.Composition;
using Sift.Services;
using Microsoft.UI.Xaml;

namespace Sift.WinUI;

public partial class App : Application
{
    private static readonly string StartupLogPath = Path.Combine(ProductPaths.DataRoot, "winui-startup.log");
    private Window? _window;
    private WinUiAppServices? _services;

    public App()
    {
        InitializeComponent();
        UnhandledException += (_, args) =>
        {
            System.Diagnostics.Debug.WriteLine(args.Exception);
            _services?.Activity.Error("App", "Unhandled WinUI exception", args.Exception.ToString(), persist: true);
            _services?.Log.Error("App", "Unhandled WinUI exception", args.Exception);
            WriteStartupFailure("Unhandled WinUI exception", args.Exception);
            // Fail closed for public builds: do not swallow faults. Evidence is already on disk.
        };
    }

    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        try
        {
            _services = WinUiAppServices.CreateDefault();
            var requestedWorkspace = Environment.GetCommandLineArgs()
                .FirstOrDefault(argument => argument.StartsWith("--open=", StringComparison.OrdinalIgnoreCase))?
                .Split('=', 2)[1];
            if (requestedWorkspace is "Home" or "Settings") _services.Settings.LastWorkspace = requestedWorkspace;
            if (_services.Settings.Dashboard.MonitorWhenClosed)
            {
                _ = _services.DashboardMonitor.EnsureRunningAsync().ContinueWith(task =>
                {
                    if (task.Exception is not null)
                        _services?.Activity.Warning("Monitor", "Background monitor could not be started", task.Exception.GetBaseException().Message);
                }, TaskScheduler.Default);
            }
            _window = new MainWindow(_services.Shell, new WorkspaceRegistryFactory(_services));
            _window.Closed += MainWindow_Closed;
            _window.Activate();
        }
        catch (Exception exception)
        {
            _services?.Activity.Error("App", "Main window construction failed", exception.ToString(), persist: true);
            _services?.Log.Error("App", "Main window construction failed", exception);
            _services?.Dispose();
            _services = null;
            WriteStartupFailure("Main window construction failed", exception);
            throw;
        }
    }

    private void MainWindow_Closed(object sender, WindowEventArgs args)
    {
        if (_window is not null) _window.Closed -= MainWindow_Closed;
        _window = null;
        _services?.Dispose();
        _services = null;
    }

    private static void WriteStartupFailure(string context, Exception exception)
    {
        try
        {
            Directory.CreateDirectory(Path.GetDirectoryName(StartupLogPath)!);
            File.AppendAllText(
                StartupLogPath,
                $"{DateTimeOffset.Now:O} {context}{Environment.NewLine}{exception}{Environment.NewLine}{Environment.NewLine}");
        }
        catch
        {
            // Startup diagnostics must never mask the original WinUI failure.
        }
    }
}
