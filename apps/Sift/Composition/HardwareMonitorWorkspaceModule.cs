using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Controls;
using Sift.WinUI.Infrastructure.Interop;
using Sift.WinUI.Views;

namespace Sift.WinUI.Composition;

public sealed class HardwareMonitorWorkspaceModule : IWorkspaceModule, IChartSettingsAware
{
    private readonly IHardwareMonitorService _monitor;
    private readonly IDockSession _dock;
    private readonly SensorHistoryStore _history;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly AppSettings _settings;
    private readonly HardwareMonitorWorkspaceView _view;
    private readonly DockWindowManager _windows;
    private readonly DispatcherTimer _timer = new() { Interval = TimeSpan.FromSeconds(2) };
    private bool _active;
    private bool _paused;
    private bool _sampling;

    public HardwareMonitorWorkspaceModule(
        IHardwareMonitorService monitor,
        IDockSession hardwareDock,
        SensorHistoryStore history,
        OperationCoordinator operations,
        ActivityHub activity,
        IClipboardService clipboard,
        AppSettings settings,
        DispatcherQueue dispatcher)
    {
        _monitor = monitor;
        _dock = hardwareDock;
        _history = history;
        _operations = operations;
        _activity = activity;
        _settings = settings;
        _view = new HardwareMonitorWorkspaceView(clipboard);
        var presenter = new SensorGraphBoardPresenter(history);
        _windows = new DockWindowManager(
            hardwareDock, presenter, dispatcher,
            embeddedCaption: "Sensor graphs",
            floatingCaption: "Floating sensor graphs");
        _view.AttachGraphs(hardwareDock);
        _view.AttachHistory(history);
        _windows.AttachEmbedded(_view.GraphDockHost);
        _timer.Tick += Timer_Tick;
        _view.RefreshRequested += View_RefreshRequested;
        _view.PauseRequested += View_PauseRequested;
        _view.GraphRequested += View_GraphRequested;
        ApplyChartSettings();
    }

    public string Key => "HardwareMonitor";
    public string Title => "Hardware monitor";
    public Control View => _view;

    public void ApplyChartSettings()
    {
        var charts = _settings.HardwareCharts;
        _history.SetCapacity(Math.Clamp(charts.HistorySamples, 30, 600));
        ApplyTimerInterval();
        ApplyBoardChartPreferences();
    }

    public async Task ActivateAsync(CancellationToken cancellationToken = default)
    {
        _active = true;
        _dock.Retain();
        ApplyChartSettings();
        await RefreshAsync(cancellationToken);
        UpdateTimer();
    }

    public void Deactivate()
    {
        _active = false;
        _dock.Release();
        UpdateTimer();
        if (!_dock.IsRetained)
            _operations.Cancel("workspace.hardware-monitor");
        _dock.Persist();
    }

    public async Task RefreshAsync(CancellationToken cancellationToken = default)
        => await SampleAsync(showBusy: true, cancellationToken);

    private async Task SampleAsync(bool showBusy, CancellationToken cancellationToken = default)
    {
        if (_sampling) return;
        _sampling = true;
        try
        {
            if (showBusy) _view.SetBusy(true, "Reading local hardware sensors…");
            var outcome = await _operations.RunLatestAsync(
                "workspace.hardware-monitor", Key, "hardware sensor sample",
                token => Task.Run(() => _monitor.Sample(token), token), cancellationToken);
            if (outcome.Cancelled || (!_active && !_dock.IsRetained))
            {
                if (showBusy && _active) _view.SetBusy(false, "Sensor sample cancelled.");
                return;
            }
            if (!outcome.Succeeded || outcome.Value is null)
            {
                var status = $"Sensor sample failed: {outcome.Error?.Message ?? "unknown error"}";
                if (showBusy) _view.SetBusy(false, status); else _view.SetSampleStatus(status);
                return;
            }

            ApplyTimerInterval();
            _history.AppendSnapshot(outcome.Value.Devices
                .SelectMany(device => device.Sensors)
                .Select(sensor => (sensor.Id, sensor.Value)));
            var labels = outcome.Value.Devices
                .SelectMany(device => device.Sensors)
                .ToDictionary(sensor => sensor.Id, sensor => sensor.ValueLabel, StringComparer.OrdinalIgnoreCase);
            if (_active) _view.Bind(outcome.Value);
            _windows.ApplyData(labels);
            ApplyBoardChartPreferences();

            var completed = $"Live sensors · updated {DateTime.Now:T} · {outcome.Elapsed.TotalMilliseconds:0} ms · {_history.Capacity} samples";
            if (showBusy) _view.SetBusy(false, completed); else if (_active) _view.SetSampleStatus(completed);
        }
        finally
        {
            _sampling = false;
        }
    }

    public void FocusPrimarySearch() => _view.FocusSearch();

    public void Dispose()
    {
        Deactivate();
        _timer.Tick -= Timer_Tick;
        _view.RefreshRequested -= View_RefreshRequested;
        _view.PauseRequested -= View_PauseRequested;
        _view.GraphRequested -= View_GraphRequested;
        _windows.Dispose();
        _dock.Persist();
    }

    private async void Timer_Tick(object? sender, object e)
    {
        if (!_paused && !_sampling && (_active || _dock.IsRetained))
            await SampleAsync(showBusy: false);
    }

    private void UpdateTimer()
    {
        if (!_paused && (_active || _dock.IsRetained)) _timer.Start();
        else _timer.Stop();
    }

    private void ApplyTimerInterval()
    {
        var configured = ChartRefreshIntervalPolicy.Resolve(_settings.HardwareCharts.RefreshInterval);
        var batterySaver = BatteryReportReader.Read().BatterySaver;
        _timer.Interval = batterySaver
            ? TimeSpan.FromSeconds(Math.Max(configured.TotalSeconds, 10))
            : configured;
    }

    private void ApplyBoardChartPreferences()
    {
        var charts = _settings.HardwareCharts;
        _windows.ForEachHost(host => host.ForEachBoard(view =>
        {
            if (view is GraphBoardView board)
                board.ApplyChartPreferences(charts);
        }));
    }

    private async void View_RefreshRequested(object? sender, EventArgs e) => await RefreshAsync();

    private void View_PauseRequested(object? sender, EventArgs e)
    {
        _paused = !_paused;
        UpdateTimer();
        _view.SetPaused(_paused);
        _activity.Info(Key, _paused ? "Paused hardware sensors" : "Resumed hardware sensors");
    }

    private void View_GraphRequested(object? sender, HardwareMonitorWorkspaceView.HardwareSensorItem sensor)
    {
        var sensorId = sensor.Id;
        var sensorName = sensor.Name;
        var sensorType = sensor.Type;
        var sensorUnit = sensor.Unit;
        _view.DispatcherQueue.TryEnqueue(() => PinSensorGraph(sensorId, sensorName, sensorType, sensorUnit));
    }

    private void PinSensorGraph(string sensorId, string sensorName, string sensorType, string sensorUnit)
    {
        var metadata = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["unit"] = sensorUnit,
            ["sensorType"] = sensorType
        };
        if (!_dock.TryAddTile(DockContentTypes.HardwareSensor, sensorId, sensorName, metadata, out var error))
        {
            _view.SetSampleStatus(error ?? "Could not add sensor graph.");
            return;
        }
        // TryAddTile raises LayoutChanged, which refreshes the embedded dock host once.
        ApplyBoardChartPreferences();
        _view.RefreshGraphValues();
        _view.SetSampleStatus($"Pinned graph for {sensorName} · {DockLayoutEngine.CountTiles(_dock.Layout)}/{_dock.Layout.MaximumTiles}.");
        _activity.Info(Key, $"Pinned sensor graph · {sensorName}");
    }
}
