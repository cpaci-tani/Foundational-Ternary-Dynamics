namespace Sift.WinUI.Composition;

public interface IWorkspaceRegistry : IDisposable
{
    IReadOnlyCollection<string> Keys { get; }
    event EventHandler? ShellSettingsChanged;
    bool Contains(string key);
    bool TryGet(string key, out IWorkspaceModule module);
}

public sealed class WorkspaceRegistry : IWorkspaceRegistry
{
    private readonly Dictionary<string, IWorkspaceModule> _modules;
    private readonly List<IWorkspaceModule> _ownedModules = [];
    private readonly IShellSettingsChangeSource? _settings;
    private bool _disposed;

    public WorkspaceRegistry(IEnumerable<IWorkspaceModule> modules)
    {
        ArgumentNullException.ThrowIfNull(modules);
        _modules = new Dictionary<string, IWorkspaceModule>(StringComparer.OrdinalIgnoreCase);

        try
        {
            foreach (var module in modules)
            {
                ArgumentNullException.ThrowIfNull(module);
                _ownedModules.Add(module);
                ArgumentException.ThrowIfNullOrWhiteSpace(module.Key);
                if (!_modules.TryAdd(module.Key, module))
                    throw new InvalidOperationException($"Duplicate workspace key '{module.Key}'.");
            }
            _settings = _modules.Values.OfType<IShellSettingsChangeSource>().SingleOrDefault();
            if (_settings is not null) _settings.ShellSettingsChanged += Settings_ShellSettingsChanged;
        }
        catch
        {
            DisposeModules();
            throw;
        }
    }

    public IReadOnlyCollection<string> Keys => _modules.Keys;
    public event EventHandler? ShellSettingsChanged;

    public bool Contains(string key) => !_disposed && _modules.ContainsKey(key);

    public bool TryGet(string key, out IWorkspaceModule module)
    {
        if (!_disposed && _modules.TryGetValue(key, out var found))
        {
            module = found;
            return true;
        }

        module = null!;
        return false;
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        if (_settings is not null) _settings.ShellSettingsChanged -= Settings_ShellSettingsChanged;
        DisposeModules();
    }

    private void Settings_ShellSettingsChanged(object? sender, EventArgs e)
    {
        foreach (var module in _modules.Values.OfType<IChartSettingsAware>())
        {
            try { module.ApplyChartSettings(); }
            catch (Exception exception) { System.Diagnostics.Debug.WriteLine(exception); }
        }

        ShellSettingsChanged?.Invoke(this, e);
    }

    private void DisposeModules()
    {
        foreach (var module in _ownedModules.Distinct<IWorkspaceModule>(ReferenceEqualityComparer.Instance))
        {
            try { module.Dispose(); }
            catch (Exception exception) { System.Diagnostics.Debug.WriteLine(exception); }
        }
        _ownedModules.Clear();
        _modules.Clear();
    }
}
