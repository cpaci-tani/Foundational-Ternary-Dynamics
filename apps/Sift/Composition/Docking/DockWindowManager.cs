using Microsoft.UI.Dispatching;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Controls;
using Sift.WinUI.Views;

namespace Sift.WinUI.Composition;

/// <summary>
/// Owns floating dock AppWindows and retains the dock session while any float is open.
/// </summary>
public sealed class DockWindowManager : IDisposable
{
    private readonly IDockSession _session;
    private readonly IDockBoardPresenter _presenter;
    private readonly DispatcherQueue _dispatcher;
    private readonly string _embeddedCaption;
    private readonly string _floatingCaption;
    private readonly Dictionary<string, DockBoardWindow> _windows = new(StringComparer.OrdinalIgnoreCase);
    private DockHostControl? _embeddedHost;
    private bool _disposed;
    private bool _syncing;
    private bool _refreshScheduled;
    private object? _boardData;

    public DockWindowManager(
        IDockSession session,
        IDockBoardPresenter presenter,
        DispatcherQueue dispatcher,
        string embeddedCaption = "Dock",
        string floatingCaption = "Floating dock")
    {
        _session = session;
        _presenter = presenter;
        _dispatcher = dispatcher;
        _embeddedCaption = embeddedCaption;
        _floatingCaption = floatingCaption;
        _session.LayoutChanged += Session_LayoutChanged;
    }

    public void AttachEmbedded(DockHostControl host)
    {
        _embeddedHost = host;
        host.PopOutRequested += Host_PopOutRequested;
        host.BindEmbedded(_session, _presenter, _embeddedCaption);
        SyncFloatingWindows();
    }

    public void ApplyData(object? data)
    {
        _boardData = data;
        _embeddedHost?.ApplyData(data);
        foreach (var window in _windows.Values) window.ApplyData(data);
    }

    public void ForEachHost(Action<DockHostControl> action)
    {
        ArgumentNullException.ThrowIfNull(action);
        if (_embeddedHost is not null) action(_embeddedHost);
        foreach (var window in _windows.Values) action(window.Host);
    }

    public void RefreshAll()
    {
        _embeddedHost?.RefreshFromSession();
        foreach (var window in _windows.Values) window.Refresh();
        SyncFloatingWindows();
    }

    public void PopOut(string boardId) => _session.PopOutBoard(boardId);

    private void Host_PopOutRequested(object? sender, string boardId) => PopOut(boardId);

    private void Session_LayoutChanged(object? sender, EventArgs e)
    {
        if (_syncing || _refreshScheduled) return;
        _refreshScheduled = true;
        _dispatcher.TryEnqueue(() =>
        {
            _refreshScheduled = false;
            if (_syncing) return;
            RefreshAll();
        });
    }

    private void SyncFloatingWindows()
    {
        if (_syncing) return;
        _syncing = true;
        try
        {
            var sites = _session.Layout.FloatingSites.ToDictionary(site => site.Id, StringComparer.OrdinalIgnoreCase);
            foreach (var stale in _windows.Keys.Where(id => !sites.ContainsKey(id)).ToList())
                CloseWindow(stale, redock: false, programmatic: true);

            foreach (var site in sites.Values)
            {
                if (_windows.ContainsKey(site.Id))
                {
                    _windows[site.Id].Refresh();
                    continue;
                }
                _session.Retain();
                EnsureWindow(site);
            }
        }
        finally { _syncing = false; }
    }

    private void EnsureWindow(FloatingDockSite site)
    {
        if (_windows.ContainsKey(site.Id)) return;
        var window = new DockBoardWindow(_session, _presenter, site, _floatingCaption);
        window.ClosedByUser += (_, _) => CloseWindow(site.Id, redock: true, programmatic: false);
        window.RedockRequested += (_, zone) =>
        {
            window.MarkProgrammaticClose();
            _windows.Remove(site.Id);
            window.Close();
            _session.Release();
            _session.RedockFloatingSite(site.Id, zone);
            _embeddedHost?.RefreshFromSession();
        };
        _windows[site.Id] = window;
        if (_boardData is not null) window.ApplyData(_boardData);
        window.Activate();
    }

    private void CloseWindow(string siteId, bool redock, bool programmatic)
    {
        if (!_windows.TryGetValue(siteId, out var window)) return;
        _windows.Remove(siteId);
        if (programmatic) window.MarkProgrammaticClose();
        try { window.Close(); } catch (Exception exception) { System.Diagnostics.Debug.WriteLine(exception); }
        _session.Release();
        if (redock && _session.Layout.FloatingSites.Any(site =>
                site.Id.Equals(siteId, StringComparison.OrdinalIgnoreCase)))
        {
            try { _session.RedockFloatingSite(siteId, DockDropZone.Tab); }
            catch (Exception exception) { System.Diagnostics.Debug.WriteLine(exception); }
        }
        _embeddedHost?.RefreshFromSession();
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        _session.LayoutChanged -= Session_LayoutChanged;
        foreach (var id in _windows.Keys.ToList())
            CloseWindow(id, redock: false, programmatic: true);
        if (_embeddedHost is not null)
            _embeddedHost.PopOutRequested -= Host_PopOutRequested;
    }
}
