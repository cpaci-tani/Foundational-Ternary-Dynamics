namespace Sift.WinUI.Infrastructure.Interop;

public enum WindowsSettingsPage
{
    StartupApps,
    InstalledApps
}

public interface IWindowsShellLauncher
{
    void OpenSettings(WindowsSettingsPage page);
    void OpenSystemInformation();
    void OpenFolder(string path);
}
