using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public interface IWorkspaceModule : IDisposable
{
    string Key { get; }
    string Title { get; }
    Control View { get; }
    Task ActivateAsync(CancellationToken cancellationToken = default);
    Task RefreshAsync(CancellationToken cancellationToken = default);
    void Deactivate();
    void FocusPrimarySearch();
}

public interface IShellSettingsChangeSource
{
    event EventHandler? ShellSettingsChanged;
}

/// <summary>Workspaces that re-apply chart preferences when Settings save.</summary>
public interface IChartSettingsAware
{
    void ApplyChartSettings();
}
