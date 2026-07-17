using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Infrastructure.Settings;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Views;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public sealed class PerformanceWorkspaceModule : IWorkspaceModule, IChartSettingsAware
{
    private readonly IProcessSampler _sampler;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly AppSettings _settings;
    private readonly SettingsPersistenceCoordinator _settingsPersistence;
    private readonly PerformanceWorkspaceView _view = new();
    private readonly DispatcherTimer _timer = new() { Interval = TimeSpan.FromSeconds(2) };
    private PdhSystemSampler? _pdh;
    private bool _active;
    private bool _paused;
    private bool _sampling;

    public PerformanceWorkspaceModule(
        IProcessSampler sampler,
        OperationCoordinator operations,
        ActivityHub activity,
        AppSettings settings,
        SettingsPersistenceCoordinator settingsPersistence)
    {
        _sampler = sampler;
        _operations = operations;
        _activity = activity;
        _settings = settings;
        _settingsPersistence = settingsPersistence;
        _timer.Tick += Timer_Tick;
        _view.RefreshRequested += View_RefreshRequested;
        _view.PauseRequested += View_PauseRequested;
        _view.ChartOptionsChanged += View_ChartOptionsChanged;
        ApplyChartSettings();
    }

    public string Key => "Performance";
    public string Title => "Performance";
    public Control View => _view;

    public void ApplyChartSettings()
    {
        ApplyTimerInterval();
        _view.ApplyChartOptions(_settings);
    }

    public async Task ActivateAsync(CancellationToken cancellationToken = default)
    {
        _active = true;
        _pdh?.Dispose();
        _pdh = new PdhSystemSampler();
        _ = _pdh.TryOpen();
        ApplyChartSettings();
        await SampleAsync(showBusy: true, cancellationToken);
        if (_active && !_paused) _timer.Start();
    }

    public void Deactivate()
    {
        _active = false;
        _timer.Stop();
        _operations.Cancel("workspace.performance");
        _pdh?.Dispose();
        _pdh = null;
    }

    public Task RefreshAsync(CancellationToken cancellationToken = default) =>
        SampleAsync(showBusy: true, cancellationToken);

    private async Task SampleAsync(bool showBusy, CancellationToken cancellationToken = default)
    {
        if (_sampling) return;
        _sampling = true;
        try
        {
            if (showBusy) _view.SetBusy(true, "Sampling system activity…");
            var pdh = _pdh;
            var outcome = await _operations.RunLatestAsync(
                "workspace.performance",
                Key,
                "performance sample",
                token => Task.Run(() =>
                {
                    var snapshot = _sampler.Sample(token);
                    var counters = pdh?.Sample();
                    return counters is null ? snapshot : snapshot with { Counters = counters };
                }, token),
                cancellationToken);

            if (outcome.Cancelled || !_active)
            {
                if (showBusy && _active) _view.SetBusy(false, "Performance sample cancelled.");
                return;
            }
            if (!outcome.Succeeded || outcome.Value is null)
            {
                _view.SetBusy(false, $"Sample failed: {outcome.Error?.Message ?? "unknown error"}");
                return;
            }

            ApplyTimerInterval();
            _view.Bind(outcome.Value);
            var pdhNote = outcome.Value.Counters is null ? " · process CPU" : " · PDH CPU/disk";
            _view.SetBusy(false, $"Live sample · updated {DateTime.Now:T} · {outcome.Elapsed.TotalMilliseconds:0} ms{pdhNote}");
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
        _view.ChartOptionsChanged -= View_ChartOptionsChanged;
    }

    private async void Timer_Tick(object? sender, object e)
    {
        if (_active && !_paused && !_sampling) await SampleAsync(showBusy: false);
    }

    private async void View_RefreshRequested(object? sender, EventArgs e) => await RefreshAsync();

    private void View_PauseRequested(object? sender, EventArgs e)
    {
        _paused = !_paused;
        if (_paused) _timer.Stop(); else if (_active) _timer.Start();
        _view.SetPaused(_paused);
        _activity.Info(Key, _paused ? "Paused performance sampling" : "Resumed performance sampling");
    }

    private void View_ChartOptionsChanged(object? sender, EventArgs e)
    {
        _settings.PerformanceShowCpuSeries = _view.ShowCpuSeries;
        _settings.PerformanceShowMemorySeries = _view.ShowMemorySeries;
        _settings.PerformanceShowLegend = _view.ShowLegend;
        _settingsPersistence.Schedule(_settings);
    }

    private void ApplyTimerInterval()
    {
        var configured = ChartRefreshIntervalPolicy.Resolve(_settings.RefreshInterval);
        var batterySaver = BatteryReportReader.Read().BatterySaver;
        _timer.Interval = batterySaver
            ? TimeSpan.FromSeconds(Math.Max(configured.TotalSeconds, 10))
            : configured;
    }
}
