using Sift.Models;

namespace Sift.Services;

public interface IDockSession
{
    string ShellId { get; }
    DockLayoutDocument Layout { get; }
    int RetainCount { get; }
    bool IsRetained { get; }
    event EventHandler? LayoutChanged;
    void Retain();
    void Release();
    bool TryAddTile(
        string contentType,
        string contentKey,
        string title,
        IReadOnlyDictionary<string, string>? metadata,
        out string? error);
    bool RemoveTile(string instanceId);
    void PlaceTile(string boardId, string instanceId, int row, int column, int rowSpan, int columnSpan);
    void TidyBoard(string boardId);
    FloatingDockSite PopOutBoard(string boardId);
    void RedockFloatingSite(string floatingSiteId, DockDropZone zone);
    void SetActiveBoard(string boardId);
    void UpdateFloatingBounds(string floatingSiteId, double x, double y, double width, double height);
    void Persist();
}

public sealed class DockSession : IDockSession
{
    private readonly IDockLayoutStore _store;
    private readonly object _gate = new();
    private int _retainCount;
    private DockLayoutDocument _layout;
    private DateTime _lastPersistUtc = DateTime.MinValue;

    public DockSession(IDockLayoutStore store)
    {
        _store = store ?? throw new ArgumentNullException(nameof(store));
        ShellId = store.ShellId;
        _layout = store.LoadOrCreate();
    }

    public string ShellId { get; }

    public DockLayoutDocument Layout
    {
        get { lock (_gate) return _layout; }
    }

    public int RetainCount
    {
        get { lock (_gate) return _retainCount; }
    }

    public bool IsRetained => RetainCount > 0;

    public event EventHandler? LayoutChanged;

    public void Retain()
    {
        lock (_gate) _retainCount++;
    }

    public void Release()
    {
        lock (_gate) _retainCount = Math.Max(0, _retainCount - 1);
    }

    public bool TryAddTile(
        string contentType,
        string contentKey,
        string title,
        IReadOnlyDictionary<string, string>? metadata,
        out string? error)
    {
        lock (_gate)
        {
            if (!DockLayoutEngine.TryAddTile(_layout, contentType, contentKey, title, metadata, out _, out error))
                return false;
            PersistUnlocked(force: true);
        }
        LayoutChanged?.Invoke(this, EventArgs.Empty);
        return true;
    }

    public bool RemoveTile(string instanceId)
    {
        bool removed;
        lock (_gate)
        {
            removed = DockLayoutEngine.RemoveTile(_layout, instanceId);
            if (removed) PersistUnlocked(force: true);
        }
        if (removed) LayoutChanged?.Invoke(this, EventArgs.Empty);
        return removed;
    }

    public void PlaceTile(string boardId, string instanceId, int row, int column, int rowSpan, int columnSpan)
    {
        lock (_gate)
        {
            DockLayoutEngine.PlaceTile(_layout, boardId, instanceId, row, column, rowSpan, columnSpan);
            PersistUnlocked(force: false);
        }
        LayoutChanged?.Invoke(this, EventArgs.Empty);
    }

    public void TidyBoard(string boardId)
    {
        lock (_gate)
        {
            DockLayoutEngine.TidyBoard(_layout, boardId);
            PersistUnlocked(force: true);
        }
        LayoutChanged?.Invoke(this, EventArgs.Empty);
    }

    public FloatingDockSite PopOutBoard(string boardId)
    {
        FloatingDockSite site;
        lock (_gate)
        {
            site = DockLayoutEngine.PopOutBoard(_layout, boardId);
            PersistUnlocked(force: true);
        }
        LayoutChanged?.Invoke(this, EventArgs.Empty);
        return site;
    }

    public void RedockFloatingSite(string floatingSiteId, DockDropZone zone)
    {
        lock (_gate)
        {
            DockLayoutEngine.RedockFloatingSite(_layout, floatingSiteId, zone);
            PersistUnlocked(force: true);
        }
        LayoutChanged?.Invoke(this, EventArgs.Empty);
    }

    public void SetActiveBoard(string boardId)
    {
        var changed = false;
        lock (_gate)
        {
            if (DockLayoutEngine.FindBoard(_layout, boardId) is null)
                throw new KeyNotFoundException($"Board '{boardId}' was not found.");
            if (string.Equals(_layout.ActiveBoardId, boardId, StringComparison.OrdinalIgnoreCase))
                return;
            _layout.ActiveBoardId = boardId;
            PersistUnlocked(force: false);
            changed = true;
        }
        if (changed) LayoutChanged?.Invoke(this, EventArgs.Empty);
    }

    public void UpdateFloatingBounds(string floatingSiteId, double x, double y, double width, double height)
    {
        lock (_gate)
        {
            var site = _layout.FloatingSites.SingleOrDefault(value =>
                value.Id.Equals(floatingSiteId, StringComparison.OrdinalIgnoreCase))
                ?? throw new KeyNotFoundException($"Floating site '{floatingSiteId}' was not found.");
            site.X = x;
            site.Y = y;
            site.Width = Math.Max(320, width);
            site.Height = Math.Max(240, height);
            PersistUnlocked(force: false);
        }
    }

    public void Persist()
    {
        lock (_gate) PersistUnlocked(force: true);
    }

    private void PersistUnlocked(bool force)
    {
        if (!force && DateTime.UtcNow - _lastPersistUtc < TimeSpan.FromMilliseconds(400))
            return;
        _store.Save(_layout);
        _lastPersistUtc = DateTime.UtcNow;
    }
}
