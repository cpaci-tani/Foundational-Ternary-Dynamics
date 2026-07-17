namespace Sift.WinUI.Composition;

public interface IWorkspaceNavigator
{
    event Action<string>? NavigationRequested;
    void NavigateTo(string workspaceKey);
}

public sealed class WorkspaceNavigator : IWorkspaceNavigator
{
    public event Action<string>? NavigationRequested;

    public void NavigateTo(string workspaceKey)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(workspaceKey);
        NavigationRequested?.Invoke(workspaceKey);
    }
}
