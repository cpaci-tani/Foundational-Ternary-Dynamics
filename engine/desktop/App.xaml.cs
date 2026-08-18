using System.Windows;
using System.Windows.Threading;

namespace FtdDesktop;

public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);
        DispatcherUnhandledException += App_DispatcherUnhandledException;

        DesktopOptions? options = null;
        try
        {
            options = DesktopOptions.Parse(e.Args);
            DesktopPaths paths = DesktopPaths.Resolve(options.RepositoryRoot);
            var window = new MainWindow(options, paths);
            MainWindow = window;
            window.Show();
        }
        catch (Exception ex)
        {
            bool smokeTest = options?.SmokeTest == true ||
                e.Args.Any(arg => string.Equals(
                    arg,
                    "--smoke-test",
                    StringComparison.OrdinalIgnoreCase));
            if (!smokeTest)
            {
                MessageBox.Show(
                    ex.Message,
                    "FTD Desktop could not start",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
            }
            Environment.ExitCode = 1;
            Shutdown(1);
        }
    }

    private void App_DispatcherUnhandledException(
        object sender,
        DispatcherUnhandledExceptionEventArgs e)
    {
        if (MainWindow is MainWindow window)
        {
            window.ReportUnhandledException(e.Exception);
            // Keep the shell alive long enough to expose logs and offer a clean
            // engine restart. The dashboard and CUDA host are isolated
            // processes, so a WPF dispatcher failure need not destroy them.
            e.Handled = true;
        }
    }
}
