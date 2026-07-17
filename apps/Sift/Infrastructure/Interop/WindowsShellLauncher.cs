using System.Diagnostics;

namespace Sift.WinUI.Infrastructure.Interop;

public sealed class WindowsShellLauncher : IWindowsShellLauncher
{
    public void OpenSettings(WindowsSettingsPage page)
    {
        var uri = page switch
        {
            WindowsSettingsPage.StartupApps => "ms-settings:startupapps",
            WindowsSettingsPage.InstalledApps => "ms-settings:appsfeatures",
            _ => throw new ArgumentOutOfRangeException(nameof(page), page, null)
        };
        Process.Start(new ProcessStartInfo(uri) { UseShellExecute = true });
    }

    public void OpenSystemInformation()
    {
        var executable = Path.Combine(Environment.SystemDirectory, "msinfo32.exe");
        Process.Start(new ProcessStartInfo(executable) { UseShellExecute = true });
    }

    public void OpenFolder(string path)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(path);
        var folder = Path.GetFullPath(path);
        var explorer = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "explorer.exe");
        var start = new ProcessStartInfo(explorer) { UseShellExecute = false };
        start.ArgumentList.Add(folder);
        Process.Start(start);
    }
}
