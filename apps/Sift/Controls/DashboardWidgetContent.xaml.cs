using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using LiveChartsCore;
using LiveChartsCore.SkiaSharpView;
using Sift.Models;
using Sift.Presentation;
using Sift.Services;
using Sift.WinUI.Infrastructure;
using Sift.Infrastructure.Icons;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Controls;

public sealed partial class DashboardWidgetContent : UserControl
{
    private const int HistoryLimit = 900;
    private readonly ObservableCollection<double> _history = [];
    private readonly ObservableCollection<DashboardListItem> _items = [];
    private readonly LineSeries<double> _series;
    private DashboardWidgetDefinition? _definition;
    private DashboardActionKind? _action;
    private DashboardActionKind? _secondaryAction;
    private bool _showBody = true;
    private bool _showActions = true;
    private string? _metricOverride;
    private string? _volumeOverride;
    private string _accent = "Clay";
    private int _listCount = 6;
    private string? _filter;
    private string _sort = "Automatic";
    private TimeSpan _historyRange = TimeSpan.FromMinutes(30);
    private bool _useStoredHistory;
    private bool _chartShowLegend = true;
    private bool _chartShowAxes = true;
    private string _chartSmoothing = ChartSmoothingPolicy.Default;
    private DashboardDensityTokens _tokens = DashboardDensityTokens.For(DashboardDensity.Default);
    private bool _placementAllowsChrome;

    public DashboardWidgetContent()
    {
        InitializeComponent();
        ItemsList.ItemsSource = _items;
        _series = new LineSeries<double>
        {
            Values = _history,
            GeometrySize = 0,
            LineSmoothness = ChartSmoothingPolicy.ResolveSmoothness(ChartSmoothingPolicy.Default),
            Stroke = ChartTheme.Stroke(ChartTheme.Clay, 1.75f),
            Fill = ChartTheme.Fill(ChartTheme.Clay)
        };
        MetricChart.Series = new ISeries[] { _series };
        MetricChart.AnimationsSpeed = TimeSpan.Zero;
        ChartTheme.ApplyChrome(MetricChart, showLegend: false, showAxes: false, yMin: 0);
    }

    public event EventHandler<DashboardWidgetActionRequest>? ActionRequested;

    public void Configure(DashboardWidgetDefinition definition)
    {
        _definition = definition;
        var metric = MetricKey(definition.Id) is not null;
        MetricChart.Visibility = metric ? Visibility.Visible : Visibility.Collapsed;
        ItemsList.Visibility = IsList(definition.Id) ? Visibility.Visible : Visibility.Collapsed;
        ConfigureAction(definition.Id);
    }

    public void ApplyChartDefaults(DashboardPreferences preferences)
    {
        _chartShowLegend = preferences.ChartShowLegend;
        _chartShowAxes = preferences.ChartShowAxes;
        _chartSmoothing = ChartSmoothingPolicy.Normalize(preferences.ChartSmoothing);
        ApplySeriesStyle();
        ApplyChartChrome();
    }

    public void ConfigurePresentation(
        DashboardPlacement placement,
        DashboardWidgetInstance instance,
        DashboardDensity density = DashboardDensity.Default)
    {
        ApplyDensity(density);
        // Body (chart/list) for any span larger than 1×1 so 2×1 and 1×2 stay useful.
        _showBody = placement.RowSpan >= 2 || placement.ColumnSpan >= 2;
        _placementAllowsChrome = placement.RowSpan >= 2 || placement.ColumnSpan >= 3;
        _showActions = !instance.Settings.TryGetValue("showActions", out var raw) ||
                       !bool.TryParse(raw, out var enabled) || enabled;
        _accent = string.IsNullOrWhiteSpace(instance.Accent) ? "Clay" : instance.Accent!;
        _metricOverride = _definition?.Id switch
        {
            "metric.chart" when instance.Settings.TryGetValue("metric", out var metric) => metric,
            "sensor.chart" when instance.Settings.TryGetValue("sensor", out var sensor) => sensor,
            _ => null
        };
        _volumeOverride = instance.Settings.TryGetValue("volume", out var volume) && !string.IsNullOrWhiteSpace(volume)
            ? volume.Trim().TrimEnd('\\', ':')
            : null;
        _listCount = instance.Settings.TryGetValue("count", out var count) && int.TryParse(count, out var parsed)
            ? Math.Clamp(parsed, 1, 20) : 6;
        _filter = instance.Settings.GetValueOrDefault("filter");
        _sort = instance.Settings.GetValueOrDefault("sort", "Automatic");
        _historyRange = ParseRange(instance.Settings.GetValueOrDefault("timeRange", "30 minutes"));
        _useStoredHistory = _historyRange > TimeSpan.FromMinutes(30);
        DetailText.Visibility = placement.RowSpan > 1 || placement.ColumnSpan >= 2
            ? Visibility.Visible : Visibility.Collapsed;
        MetricChart.Visibility = _showBody && MetricKey(_definition?.Id ?? string.Empty) is not null
            ? Visibility.Visible : Visibility.Collapsed;
        ItemsList.Visibility = _showBody && IsList(_definition?.Id ?? string.Empty) && _items.Count > 0
            ? Visibility.Visible : Visibility.Collapsed;
        ActionButton.Visibility = _showActions && placement.RowSpan >= 2 && _action is not null
            ? Visibility.Visible : Visibility.Collapsed;
        SecondaryActionButton.Visibility = _showActions && placement.RowSpan >= 2 && _secondaryAction is not null
            ? Visibility.Visible : Visibility.Collapsed;
        ApplySeriesStyle();
        ApplyChartChrome();
    }

    private void ApplyDensity(DashboardDensity density)
    {
        _tokens = DashboardDensityTokens.For(density);
        RootGrid.RowSpacing = _tokens.ContentRowSpacing;
        ActionRow.Spacing = _tokens.ContentRowSpacing;
        ValueText.FontSize = _tokens.MetricFontSize;
        DetailText.FontSize = _tokens.MetaFontSize;
        EmptyText.FontSize = _tokens.MetaFontSize;
        ActionButton.MinHeight = _tokens.ActionMinHeight;
        SecondaryActionButton.MinHeight = _tokens.ActionMinHeight;
        ActionButton.Padding = new Thickness(_tokens.ActionMinHeight >= 36 ? 12 : 10, 4, _tokens.ActionMinHeight >= 36 ? 12 : 10, 4);
        SecondaryActionButton.Padding = ActionButton.Padding;
        MetricChart.DrawMargin = new LiveChartsCore.Measure.Margin(0f, 2f, 0f, (float)_tokens.ChartBottomMargin);
        ItemsList.ItemContainerStyle = BuildListItemStyle(_tokens.ListRowHeight);
    }

    private void ApplySeriesStyle()
    {
        var color = ChartTheme.ForAccent(_accent);
        _series.Name = _definition?.Title ?? "Metric";
        _series.Stroke = ChartTheme.Stroke(color, (float)_tokens.ChartStrokeThickness);
        _series.Fill = ChartTheme.Fill(color);
        _series.LineSmoothness = ChartSmoothingPolicy.ResolveSmoothness(_chartSmoothing);
    }

    private void ApplyChartChrome()
    {
        var showLegend = _chartShowLegend && _placementAllowsChrome && _showBody;
        var showAxes = _chartShowAxes && _placementAllowsChrome && _showBody;
        ChartTheme.ApplyChrome(MetricChart, showLegend, showAxes, yMin: 0);
    }

    private static Style BuildListItemStyle(double minHeight)
    {
        var style = new Style(typeof(ListViewItem));
        style.Setters.Add(new Setter(FrameworkElement.MinHeightProperty, minHeight));
        style.Setters.Add(new Setter(Control.PaddingProperty, new Thickness(0, 1, 0, 1)));
        style.Setters.Add(new Setter(Control.HorizontalContentAlignmentProperty, HorizontalAlignment.Stretch));
        return style;
    }

    public void ApplySnapshot(DashboardSnapshotDelta snapshot, IReadOnlyList<DashboardAlert> alerts)
    {
        if (_definition is null) return;
        var id = _definition.Id;
        if (MetricKey(id) is { } key)
        {
            if (!snapshot.Metrics.TryGetValue(key, out var metric))
            {
                SetText(ValueText, "—");
                SetText(DetailText, "Not reported by this PC.");
                return;
            }
            SetText(ValueText, SiftDisplay.MetricPrimary(metric.Key, metric.Value, metric.Unit));
            SetText(DetailText, Detail(id, snapshot, metric));
            if (!_useStoredHistory && snapshot.HasChangedMetric(key))
            {
                _history.Add(metric.Value);
                while (_history.Count > HistoryLimit) _history.RemoveAt(0);
            }
            return;
        }

        switch (id)
        {
            case "topCpu":
                ApplyProcesses(
                    "CPU",
                    snapshot.TopProcesses.OrderByDescending(process => process.CpuPercent),
                    process => $"{process.Name}  ·  PID {process.Id}  ·  {SiftDisplay.CpuPercent(process.CpuPercent)}  ·  {SiftDisplay.WorkingSetMiBShort(process.MemoryMb)}");
                break;
            case "topMem":
                ApplyProcesses(
                    "memory",
                    snapshot.TopProcesses.OrderByDescending(process => process.MemoryMb),
                    process => $"{process.Name}  ·  PID {process.Id}  ·  {SiftDisplay.WorkingSetMiB(process.MemoryMb)}  ·  {SiftDisplay.CpuPercent(process.CpuPercent)}");
                break;
            case "topIo":
                ApplyProcesses(
                    "disk I/O",
                    snapshot.TopProcesses.OrderByDescending(process => process.ReadRateMb + process.WriteRateMb),
                    process => $"{process.Name}  ·  PID {process.Id}  ·  {SiftDisplay.DiskReadWriteMiB(process.ReadRateMb, process.WriteRateMb)}");
                break;
            case "services":
                ApplyCount(snapshot, "services.running", "services.total", "running", "services total");
                var services = (snapshot.Services ?? []).Where(service => service.CanManage && Matches(service.DisplayName));
                if (_sort == "Name") services = services.OrderBy(service => service.DisplayName, StringComparer.CurrentCultureIgnoreCase);
                SetItems(services.Take(_listCount)
                    .Select(service => DashboardListItem.ForService(service)));
                break;
            case "startup":
                var enabled = Value(snapshot, "startup.enabled");
                var startupTotal = Value(snapshot, "startup.total");
                var disabled = Math.Max(0, startupTotal - enabled);
                SetText(ValueText, $"{enabled:0} enabled");
                SetText(DetailText, $"{startupTotal:0} total · {disabled:0} disabled · open Startup for details");
                break;
            case "health":
                var warning = Value(snapshot, "health.warnings");
                var critical = Value(snapshot, "health.critical");
                var failed = Value(snapshot, "health.failed");
                SetText(ValueText, critical > 0 && warning > 0
                    ? $"{critical:0} critical · {warning:0} warnings"
                    : critical > 0 ? $"{critical:0} critical"
                    : warning > 0 ? $"{warning:0} warnings"
                    : "Healthy");
                SetText(DetailText, failed > 0
                    ? $"{failed:0} failed checks · open Health for recommendations"
                    : "Latest local health checks · no failures");
                break;
            case "alerts":
                var active = alerts.Where(alert => alert.ClearedUtc is null).Take(6).ToList();
                SetText(ValueText, active.Count == 0 ? "All clear" : SiftDisplay.CountNoun(active.Count, "active alert", "active alerts"));
                SetText(DetailText, active.Count == 0
                    ? "No active lifecycle alerts."
                    : string.Join(" · ", active.Take(2).Select(alert => alert.Title)));
                SetItems(active.Select(DashboardListItem.ForAlert));
                break;
            case "installedApps":
                var uninstallable = Value(snapshot, "apps.uninstallable");
                var appsTotal = Value(snapshot, "apps.total");
                var leftovers = Value(snapshot, "apps.leftovers");
                SetText(ValueText, $"{uninstallable:0} uninstallable");
                SetText(DetailText, leftovers > 0
                    ? $"{leftovers:0} leftover registrations · {appsTotal:0} registered"
                    : $"{appsTotal:0} registered apps · no leftover registrations");
                break;
            case "recovery":
                SetText(ValueText, SiftDisplay.CountNoun(Value(snapshot, "recovery.backups"), "backup", "backups"));
                var age = Value(snapshot, "recovery.latest_age_days");
                SetText(DetailText, $"{SiftDisplay.LatestBackupAge(age)} · open Recovery to restore");
                break;
            case "maintenance":
                var scanAge = Value(snapshot, "maintenance.latest_age_days");
                SetText(ValueText, SiftDisplay.DaysAgoOrNever(scanAge, "Ready to scan"));
                SetText(DetailText, "Temporary files, caches, and unused registrations.");
                break;
            case "optimize":
                SetText(ValueText, "Balanced");
                SetText(DetailText, "Reversible preset · Advanced stays opt-in.");
                break;
            case "systemInfo":
                SetText(ValueText, Environment.MachineName);
                SetText(DetailText,
                    $"{Environment.OSVersion.VersionString} · {Environment.ProcessorCount} logical processors · {Environment.Is64BitOperatingSystem switch { true => "64-bit", _ => "32-bit" }}");
                break;
            default:
                SetText(ValueText, "Ready");
                SetText(DetailText, _definition.Description);
                break;
        }
    }

    private void ApplyProcesses(string sortLabel, IEnumerable<ProcessSnapshot> processes, Func<ProcessSnapshot, string> format)
    {
        var filtered = processes.Where(process => Matches(process.Name));
        if (_sort == "Name") filtered = filtered.OrderBy(process => process.Name, StringComparer.CurrentCultureIgnoreCase);
        else if (_sort == "Lowest first") filtered = filtered.Reverse();
        var rows = filtered.Take(_listCount).Select(process => DashboardListItem.ForProcess(process, format(process))).ToList();
        SetText(ValueText, rows.Count == 0 ? "No processes" : $"Top {rows.Count} by {sortLabel}");
        SetText(DetailText, rows.Count == 0
            ? "No matching processes in this session."
            : "This session · select a row to end or restart");
        SetItems(rows);
    }

    private void ApplyCount(
        DashboardSnapshotDelta snapshot,
        string valueKey,
        string totalKey,
        string valueLabel,
        string totalLabel = "total")
    {
        var value = Value(snapshot, valueKey);
        var total = Value(snapshot, totalKey);
        SetText(ValueText, $"{value:0} {valueLabel}");
        SetText(DetailText, $"{total:0} {totalLabel}");
    }

    private void SetItems(IEnumerable<DashboardListItem> values)
    {
        var next = values.ToList();
        for (var index = 0; index < next.Count; index++)
        {
            var incoming = next[index];
            var existingIndex = IndexOf(incoming.Key);
            if (existingIndex < 0) _items.Insert(index, incoming);
            else
            {
                var existing = _items[existingIndex];
                existing.UpdateFrom(incoming);
                if (existingIndex != index) _items.Move(existingIndex, index);
            }
        }
        while (_items.Count > next.Count) _items.RemoveAt(_items.Count - 1);
        ItemsList.Visibility = _showBody && next.Count > 0 ? Visibility.Visible : Visibility.Collapsed;
        EmptyText.Visibility = _showBody && next.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
    }

    private int IndexOf(string key)
    {
        for (var index = 0; index < _items.Count; index++)
            if (_items[index].Key.Equals(key, StringComparison.OrdinalIgnoreCase)) return index;
        return -1;
    }

    private bool Matches(string value) => string.IsNullOrWhiteSpace(_filter) ||
        value.Contains(_filter, StringComparison.CurrentCultureIgnoreCase);

    public string? HistoryMetricKey => _definition is null ? null : MetricKey(_definition.Id);
    public TimeSpan HistoryRange => _historyRange;

    public void ApplyHistory(IReadOnlyList<DashboardHistoryPoint> points)
    {
        if (!_useStoredHistory) return;
        var values = points.Select(point => point.Average).TakeLast(500).ToList();
        var shared = Math.Min(_history.Count, values.Count);
        for (var index = 0; index < shared; index++)
            if (Math.Abs(_history[index] - values[index]) > 0.000001) _history[index] = values[index];
        while (_history.Count > values.Count) _history.RemoveAt(_history.Count - 1);
        for (var index = _history.Count; index < values.Count; index++) _history.Add(values[index]);
    }

    private static TimeSpan ParseRange(string value) => value switch
    {
        "24 hours" => TimeSpan.FromHours(24),
        "7 days" => TimeSpan.FromDays(7),
        "30 days" => TimeSpan.FromDays(30),
        "90 days" => TimeSpan.FromDays(90),
        _ => TimeSpan.FromMinutes(30)
    };

    private void ConfigureAction(string id)
    {
        var selection = id switch
        {
            "optimize" => ((DashboardActionKind?)DashboardActionKind.OptimizePreset, "Review Balanced", (DashboardActionKind?)null, string.Empty),
            "maintenance" => ((DashboardActionKind?)DashboardActionKind.MaintenanceCleanup, "Scan and review", (DashboardActionKind?)null, string.Empty),
            "topCpu" or "topMem" or "topIo" => ((DashboardActionKind?)DashboardActionKind.EndProcess, "End selected", (DashboardActionKind?)DashboardActionKind.RestartProcess, "Restart selected"),
            "services" => ((DashboardActionKind?)DashboardActionKind.StartService, "Start selected", (DashboardActionKind?)DashboardActionKind.RestartService, "Restart selected"),
            "alerts" => ((DashboardActionKind?)DashboardActionKind.AcknowledgeAlert, "Acknowledge", (DashboardActionKind?)DashboardActionKind.SnoozeAlert, "Snooze"),
            _ => ((DashboardActionKind?)null, string.Empty, (DashboardActionKind?)null, string.Empty)
        };
        _action = selection.Item1;
        SetActionButton(ActionButton, selection.Item2);
        ActionButton.Visibility = _action is null ? Visibility.Collapsed : Visibility.Visible;
        _secondaryAction = selection.Item3;
        SetActionButton(SecondaryActionButton, selection.Item4);
        SecondaryActionButton.Visibility = _secondaryAction is null ? Visibility.Collapsed : Visibility.Visible;
        UpdateActionAvailability();
    }

    private static void SetActionButton(SiftIconButton button, string label)
    {
        button.Label = label;
        button.Icon = label switch
        {
            "Review Balanced" => SiftIconKind.Customize,
            "Scan and review" => SiftIconKind.Scan,
            "End selected" => SiftIconKind.EndTask,
            "Restart selected" => SiftIconKind.Restart,
            "Start selected" => SiftIconKind.Start,
            "Acknowledge" => SiftIconKind.Done,
            "Snooze" => SiftIconKind.Pause,
            _ => SiftIconKind.None
        };
    }

    private void ActionButton_Click(object sender, RoutedEventArgs e)
    {
        if (_action is { } action) RaiseAction(action);
    }

    private void SecondaryActionButton_Click(object sender, RoutedEventArgs e)
    {
        if (_secondaryAction is { } action) RaiseAction(action);
    }

    private void RaiseAction(DashboardActionKind action)
    {
        var selected = ItemsList.SelectedItem as DashboardListItem;
        ActionRequested?.Invoke(this, new DashboardWidgetActionRequest(
            action, selected?.Process, selected?.Service, selected?.AlertId));
    }

    private void ItemsList_SelectionChanged(object sender, SelectionChangedEventArgs e) => UpdateActionAvailability();

    private void UpdateActionAvailability()
    {
        var needsSelection = _definition?.Id is "topCpu" or "topMem" or "topIo" or "services" or "alerts";
        var enabled = !needsSelection || ItemsList.SelectedItem is not null;
        ActionButton.IsEnabled = enabled;
        SecondaryActionButton.IsEnabled = enabled;
    }

    private static bool IsList(string id) => id is "topCpu" or "topMem" or "topIo" or "services" or "alerts" or "activity" or "timeline";

    private string? MetricKey(string id) => id switch
    {
        "cpu" => "cpu.percent",
        "memory" => "memory.percent",
        "network" => "network.download_mbps",
        "storage" when !string.IsNullOrWhiteSpace(_volumeOverride) =>
            $"storage.{NormalizeVolume(_volumeOverride!)}.free_percent",
        "storage" => "storage.lowest_free_percent",
        "uptime" => "system.uptime_hours",
        "battery" => "battery.charge_percent",
        "thermals" => "hardware.hottest_c",
        "sensor.chart" => string.IsNullOrWhiteSpace(_metricOverride) ? "hardware.hottest_c" : _metricOverride,
        "metric.chart" => string.IsNullOrWhiteSpace(_metricOverride) ? "cpu.percent" : _metricOverride,
        _ => null
    };

    private static string NormalizeVolume(string volume) =>
        new string(volume.Where(ch => char.IsLetterOrDigit(ch)).ToArray()).ToUpperInvariant();

    private string Detail(string id, DashboardSnapshotDelta snapshot, DashboardMetricSample metric) => id switch
    {
        "cpu" => $"{snapshot.TopProcesses.Count:N0} processes · {SiftDisplay.DiskReadWriteMiB(Value(snapshot, "disk.read_mb_s"), Value(snapshot, "disk.write_mb_s"))} · {_history.Count:N0} samples · {SiftDisplay.HistoryWindow(_historyRange)}",
        "memory" => $"{SiftDisplay.PhysicalMemoryGb(Value(snapshot, "memory.used_gb"), Value(snapshot, "memory.total_gb"))} · {SiftDisplay.HistoryWindow(_historyRange)}",
        "network" => $"{SiftDisplay.NetworkDownUpMbps(Value(snapshot, "network.download_mbps"), Value(snapshot, "network.upload_mbps"))} · {SiftDisplay.HistoryWindow(_historyRange)}",
        "storage" when !string.IsNullOrWhiteSpace(_volumeOverride) =>
            $"{_volumeOverride}: {Value(snapshot, $"storage.{NormalizeVolume(_volumeOverride)}.free_gb"):0.0} GB free ({SiftDisplay.FreePercent(metric.Value)}) · {SiftDisplay.HistoryWindow(_historyRange)}",
        "storage" => $"Lowest free: {Value(snapshot, "storage.lowest_free_gb"):0.0} GB free across {Value(snapshot, "storage.volume_count"):0} volumes · {SiftDisplay.HistoryWindow(_historyRange)}",
        "uptime" => $"Since last Windows boot · {Environment.ProcessorCount} logical processors",
        "battery" => BatteryDetail(snapshot),
        "thermals" or "sensor.chart" => metric.Minimum is { } min && metric.Maximum is { } max
            ? $"Hottest temperature · session range {SiftDisplay.TemperatureCelsius(min)}–{SiftDisplay.TemperatureCelsius(max)} · open Hardware for fans and power · {SiftDisplay.HistoryWindow(_historyRange)}"
            : $"Hottest available temperature · open Hardware for fans and power · {SiftDisplay.HistoryWindow(_historyRange)}",
        "metric.chart" => $"{SiftDisplay.MetricTitle(metric.Key)} ({metric.Unit}) · {SiftDisplay.HistoryWindow(_historyRange)}",
        _ => $"{SiftDisplay.MetricTitle(metric.Key)} · {metric.Unit} · {SiftDisplay.HistoryWindow(_historyRange)}"
    };

    private string BatteryDetail(DashboardSnapshotDelta snapshot)
    {
        var parts = new List<string>
        {
            Value(snapshot, "battery.on_ac") > 0 ? "AC power" : "On battery"
        };
        if (Value(snapshot, "battery.health_percent") > 0)
            parts.Add($"health {SiftDisplay.Percent(Value(snapshot, "battery.health_percent"))}");
        if (Value(snapshot, "battery.remaining_minutes") > 0)
            parts.Add($"{Value(snapshot, "battery.remaining_minutes"):0} min remaining");
        if (Value(snapshot, "battery.charge_rate_mw") != 0)
            parts.Add($"{Value(snapshot, "battery.charge_rate_mw"):0} mW");
        var remaining = Value(snapshot, "battery.remaining_mwh");
        var full = Value(snapshot, "battery.full_charge_mwh");
        var design = Value(snapshot, "battery.design_mwh");
        if (remaining > 0 && full > 0)
            parts.Add(design > 0
                ? $"{remaining:0} / {full:0} mWh (design {design:0} mWh)"
                : $"{remaining:0} / {full:0} mWh");
        if (Value(snapshot, "power.battery_saver") > 0)
            parts.Add("Battery Saver on");
        return string.Join(" · ", parts);
    }

    private static double Value(DashboardSnapshotDelta snapshot, string key) =>
        snapshot.Metrics.TryGetValue(key, out var metric) ? metric.Value : 0;

    private static void SetText(TextBlock target, string value)
    {
        if (!string.Equals(target.Text, value, StringComparison.Ordinal)) target.Text = value;
    }
}

public sealed record DashboardWidgetActionRequest(
    DashboardActionKind Action,
    ProcessSnapshot? Process,
    DashboardServiceSnapshot? Service,
    string? AlertId);

public sealed class DashboardListItem : INotifyPropertyChanged
{
    private string _display;

    private DashboardListItem(string key, string display)
    {
        Key = key;
        _display = display;
    }

    public string Key { get; }
    public string Display { get => _display; private set { if (_display == value) return; _display = value; Changed(); } }
    public ProcessSnapshot? Process { get; private set; }
    public DashboardServiceSnapshot? Service { get; private set; }
    public string? AlertId { get; private set; }
    public event PropertyChangedEventHandler? PropertyChanged;

    public static DashboardListItem ForProcess(ProcessSnapshot process, string display) =>
        new($"process:{process.Id}:{process.StartTimeUtcTicks}", display) { Process = process };
    public static DashboardListItem ForService(DashboardServiceSnapshot service) =>
        new($"service:{service.Name}", $"{service.DisplayName}  ·  {service.Status}") { Service = service };
    public static DashboardListItem ForAlert(DashboardAlert alert) =>
        new($"alert:{alert.Id}", $"{alert.Severity}  ·  {alert.Title}") { AlertId = alert.Id };

    public void UpdateFrom(DashboardListItem source)
    {
        Display = source.Display;
        Process = source.Process;
        Service = source.Service;
        AlertId = source.AlertId;
    }

    private void Changed([CallerMemberName] string? name = null) =>
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}
