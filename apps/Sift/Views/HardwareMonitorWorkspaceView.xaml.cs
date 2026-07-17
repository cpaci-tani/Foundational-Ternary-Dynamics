using System.Collections.ObjectModel;
using System.ComponentModel;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Controls;
using Sift.Infrastructure.Icons;
using Sift.WinUI.Infrastructure.Interop;

namespace Sift.WinUI.Views;

public sealed partial class HardwareMonitorWorkspaceView : UserControl
{
    private readonly ObservableCollection<HardwareDeviceGroup> _groups = [];
    private readonly List<HardwareDeviceGroup> _allGroups = [];
    private readonly Dictionary<string, HardwareDeviceGroup> _groupIndex = new(StringComparer.Ordinal);
    private readonly Dictionary<string, bool> _expansion = new(StringComparer.Ordinal);
    private readonly IClipboardService _clipboard;
    private IDockSession? _dock;
    private SensorHistoryStore? _history;
    private HardwareSensorItem? _selectedSensor;
    private bool _bindingFilters;
    private HardwareMonitorSnapshot? _latestSnapshot;

    public HardwareMonitorWorkspaceView(IClipboardService clipboard)
    {
        _clipboard = clipboard ?? throw new ArgumentNullException(nameof(clipboard));
        InitializeComponent();
        DeviceGroups.ItemsSource = _groups;
        SensorTypeBox.Items.Add("All sensor types");
        SensorTypeBox.SelectedIndex = 0;
    }

    public event EventHandler? RefreshRequested;
    public event EventHandler? PauseRequested;
    public event EventHandler<HardwareSensorItem>? GraphRequested;

    public DockHostControl GraphDockHost => GraphDock;

    public void AttachGraphs(IDockSession session) => _dock = session;

    public void AttachHistory(SensorHistoryStore history) => _history = history;

    public void Bind(HardwareMonitorSnapshot snapshot)
    {
        _latestSnapshot = snapshot;
        var topologyChanged = SynchronizeGroups(snapshot);
        if (topologyChanged)
        {
            UpdateSensorTypes();
            ApplyFilter();
        }
        UpdateSummary(snapshot);
        SetText(ProviderText, string.Join("  ·  ", snapshot.Providers.Select(provider =>
            $"{provider.Name}: {(provider.Available ? provider.Detail : $"unavailable — {provider.Detail}")} ({provider.Duration.TotalMilliseconds:0} ms)")) +
            (snapshot.IsElevated ? "  ·  elevated sensor access" : "  ·  standard-user access; some board sensors may be unavailable"));

        if (_selectedSensor is not null)
        {
            _selectedSensor = _allGroups.SelectMany(group => group.AllSensors)
                .FirstOrDefault(sensor => sensor.Id == _selectedSensor.Id);
            if (_selectedSensor is not null) UpdateSelectionReadout(_selectedSensor);
            else
            {
                SetText(ChartTitle, "Sensor graphs");
                SetText(ChartSubtitle, "Pin sensors with Graph. Arrange, tidy, or pop out a floating dock board.");
            }
        }

        RefreshGraphValues();
        RefreshSparklines();
    }

    public void RefreshGraphValues()
    {
        if (_latestSnapshot is null) return;
        var labels = _latestSnapshot.Devices
            .SelectMany(device => device.Sensors)
            .ToDictionary(sensor => sensor.Id, sensor => sensor.ValueLabel, StringComparer.OrdinalIgnoreCase);
        GraphDock.ApplyData(labels);
    }

    public void SetBusy(bool busy, string status)
    {
        BusyRing.IsActive = busy;
        RefreshButton.IsEnabled = !busy;
        SetSampleStatus(status);
    }

    public void SetSampleStatus(string status) => SetText(StatusText, status);

    public void SetPaused(bool paused)
    {
        PauseButton.Label = paused ? "Resume" : "Pause";
        PauseButton.Icon = paused ? SiftIconKind.Play : SiftIconKind.Pause;
        SetSampleStatus(paused ? "Sensor sampling paused · histories retained" : "Live hardware sampling active");
    }

    public void FocusSearch() => SearchBox.Focus(FocusState.Programmatic);

    private void RefreshSparklines()
    {
        if (_history is null) return;
        foreach (var sensor in _allGroups.SelectMany(group => group.AllSensors))
            sensor.UpdateSparkValues(_history.GetValues(sensor.Id));
    }

    private bool SynchronizeGroups(HardwareMonitorSnapshot snapshot)
    {
        var changed = false;
        var next = new List<HardwareDeviceGroup>(snapshot.Devices.Count);
        foreach (var device in snapshot.Devices)
        {
            if (!_groupIndex.TryGetValue(device.Id, out var group) || !group.Matches(device))
            {
                var expanded = !_expansion.TryGetValue(device.Id, out var saved) || saved;
                group = new HardwareDeviceGroup(device, expanded);
                _groupIndex[device.Id] = group;
                changed = true;
            }
            else if (group.Update(device)) changed = true;
            _expansion[device.Id] = group.IsExpanded;
            next.Add(group);
        }
        foreach (var stale in _groupIndex.Keys.Except(next.Select(group => group.Id), StringComparer.Ordinal).ToList())
        {
            _groupIndex.Remove(stale);
            changed = true;
        }
        _allGroups.Clear();
        _allGroups.AddRange(next);
        return changed;
    }

    private void UpdateSensorTypes()
    {
        _bindingFilters = true;
        var selected = SensorTypeBox.SelectedItem as string;
        SensorTypeBox.Items.Clear();
        SensorTypeBox.Items.Add("All sensor types");
        foreach (var type in _allGroups.SelectMany(group => group.AllSensors).Select(sensor => sensor.Type)
                     .Distinct(StringComparer.OrdinalIgnoreCase).OrderBy(type => type))
            SensorTypeBox.Items.Add(type);
        SensorTypeBox.SelectedItem = selected is not null && SensorTypeBox.Items.Contains(selected)
            ? selected
            : "All sensor types";
        _bindingFilters = false;
    }

    private void ApplyFilter()
    {
        var query = SearchBox.Text?.Trim() ?? string.Empty;
        var type = SensorTypeBox.SelectedItem as string;
        var filtered = _allGroups
            .Select(group => group.Filter(query, type))
            .Where(group => group is not null)
            .Cast<HardwareDeviceGroup>()
            .ToList();
        SyncObservable(_groups, filtered, group => group.Id);
        EmptyState.Visibility = _groups.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
        RefreshSparklines();
    }

    private void UpdateSummary(HardwareMonitorSnapshot snapshot)
    {
        var all = snapshot.Devices.SelectMany(device => device.Sensors.Select(sensor => (device, sensor))).ToList();
        SetMetric(all.Where(item => item.sensor.Type == "Temperature").OrderByDescending(item => item.sensor.Value).FirstOrDefault(), TemperatureText, TemperatureDetail);
        SetMetric(all.Where(item => item.sensor.Type == "Power").OrderByDescending(item => item.sensor.Value).FirstOrDefault(), PowerText, PowerDetail);
        SetMetric(all.Where(item => item.sensor.Type == "Fan").OrderByDescending(item => item.sensor.Value).FirstOrDefault(), FanText, FanDetail);
        SetText(SensorCountText, $"{snapshot.SensorCount:N0} sensors");
        SetText(DeviceCountText, $"{snapshot.Devices.Count:N0} devices · {snapshot.Providers.Count(provider => provider.Available):N0}/{snapshot.Providers.Count:N0} providers available");
    }

    private static void SetMetric((HardwareDeviceSnapshot device, HardwareSensorReading sensor) metric, TextBlock value, TextBlock detail)
    {
        if (metric.sensor is null)
        {
            SetText(value, "—");
            SetText(detail, "Not reported");
            return;
        }
        SetText(value, metric.sensor.ValueLabel);
        var range = metric.sensor.RangeLabel;
        SetText(detail, string.IsNullOrWhiteSpace(range)
            ? $"{metric.device.Name} · {metric.sensor.Name}"
            : $"{metric.device.Name} · {metric.sensor.Name} · {range}");
    }

    private void SelectSensor(HardwareSensorItem sensor)
    {
        _selectedSensor = sensor;
        UpdateSelectionReadout(sensor);
    }

    private void UpdateSelectionReadout(HardwareSensorItem sensor)
    {
        SetText(ChartTitle, sensor.Name);
        SetText(ChartSubtitle, $"{sensor.Type} · {sensor.ValueLabel} · use Graph to pin a live tile (max {DockLayoutDocument.DefaultMaximumTiles}).");
    }

    private void CoreTile_Tapped(object sender, TappedRoutedEventArgs e)
    {
        if (IsInteractiveChild(e.OriginalSource as DependencyObject)) return;
        if (sender is FrameworkElement { DataContext: HardwareSensorItem sensor })
            SelectSensor(sensor);
    }

    private void MetricCard_Tapped(object sender, TappedRoutedEventArgs e)
    {
        if (IsInteractiveChild(e.OriginalSource as DependencyObject)) return;
        if (sender is FrameworkElement { DataContext: HardwareSensorItem sensor })
            SelectSensor(sensor);
    }

    private void GraphButton_Click(object sender, RoutedEventArgs e)
    {
        if (!TryResolveSensor(sender, out var sensor)) return;
        SelectSensor(sensor);
        GraphRequested?.Invoke(this, sensor);
    }

    private static bool TryResolveSensor(object sender, out HardwareSensorItem sensor)
    {
        sensor = null!;
        if (sender is not FrameworkElement element) return false;
        if (element.DataContext is HardwareSensorItem fromContext)
        {
            sensor = fromContext;
            return true;
        }
        if (element.Tag is HardwareSensorItem fromTag)
        {
            sensor = fromTag;
            return true;
        }
        return false;
    }

    private static bool IsInteractiveChild(DependencyObject? source)
    {
        while (source is not null)
        {
            if (source is Button) return true;
            source = VisualTreeHelper.GetParent(source);
        }
        return false;
    }

    private void Filter_Changed(object sender, object e)
    {
        if (!_bindingFilters) ApplyFilter();
    }

    private void ExpandAllButton_Click(object sender, RoutedEventArgs e)
    {
        foreach (var group in _groups) group.IsExpanded = true;
    }

    private void CollapseAllButton_Click(object sender, RoutedEventArgs e)
    {
        foreach (var group in _groups) group.IsExpanded = false;
    }

    private void RefreshButton_Click(object sender, RoutedEventArgs e) => RefreshRequested?.Invoke(this, EventArgs.Empty);
    private void PauseButton_Click(object sender, RoutedEventArgs e) => PauseRequested?.Invoke(this, EventArgs.Empty);

    private void CopyReadingButton_Click(object sender, RoutedEventArgs e)
    {
        if (_selectedSensor is null)
        {
            SetSampleStatus("Select a sensor card first.");
            return;
        }
        _clipboard.CopyText($"{_selectedSensor.Name}\t{_selectedSensor.ValueLabel}\t{_selectedSensor.Type}");
        SetSampleStatus($"Copied {_selectedSensor.Name} · {_selectedSensor.ValueLabel}");
    }

    private void ClearHistoryButton_Click(object sender, RoutedEventArgs e)
    {
        if (_history is null || _selectedSensor is null)
        {
            SetSampleStatus("Select a sensor card first.");
            return;
        }
        _history.Clear(_selectedSensor.Id);
        _selectedSensor.UpdateSparkValues([]);
        SetSampleStatus($"Cleared history for {_selectedSensor.Name}");
        RefreshGraphValues();
    }

    private void RootGrid_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        var stacked = e.NewSize.Width < 1100;
        InventoryColumn.Width = stacked ? new GridLength(1, GridUnitType.Star) : new GridLength(1.3, GridUnitType.Star);
        ChartColumn.Width = stacked ? new GridLength(0) : new GridLength(1, GridUnitType.Star);
        MonitorTopRow.Height = stacked ? new GridLength(1.1, GridUnitType.Star) : new GridLength(1, GridUnitType.Star);
        MonitorBottomRow.Height = stacked ? new GridLength(0.9, GridUnitType.Star) : new GridLength(0);
        if (stacked)
        {
            Grid.SetColumn(ChartPanel, 0);
            Grid.SetRow(ChartPanel, 1);
        }
        else
        {
            Grid.SetColumn(ChartPanel, 1);
            Grid.SetRow(ChartPanel, 0);
        }
        ChartPanel.Visibility = Visibility.Visible;
    }

    private static void SetText(TextBlock target, string value)
    {
        if (!string.Equals(target.Text, value, StringComparison.Ordinal)) target.Text = value;
    }

    private static void SyncObservable<T>(ObservableCollection<T> target, IReadOnlyList<T> desired, Func<T, string> keySelector)
    {
        while (target.Count > desired.Count) target.RemoveAt(target.Count - 1);
        for (var index = 0; index < desired.Count; index++)
        {
            var desiredItem = desired[index];
            var desiredKey = keySelector(desiredItem);
            if (index < target.Count && string.Equals(keySelector(target[index]), desiredKey, StringComparison.Ordinal))
                continue;

            var existingIndex = -1;
            for (var candidate = index; candidate < target.Count; candidate++)
            {
                if (!string.Equals(keySelector(target[candidate]), desiredKey, StringComparison.Ordinal)) continue;
                existingIndex = candidate;
                break;
            }

            if (existingIndex >= 0) target.Move(existingIndex, index);
            else target.Insert(index, desiredItem);
        }
    }

    public sealed class HardwareDeviceGroup : INotifyPropertyChanged
    {
        private bool _isExpanded;
        private List<HardwareSensorItem> _allSensors = [];
        private Dictionary<string, HardwareSensorItem> _sensorIndex = new(StringComparer.Ordinal);
        private int _visibleCount;

        public HardwareDeviceGroup(HardwareDeviceSnapshot device, bool isExpanded)
        {
            Id = device.Id;
            Name = device.Name;
            Type = device.Type;
            _isExpanded = isExpanded;
            ReplaceSensors(device.Sensors);
            RebuildSections(_allSensors);
        }

        public event PropertyChangedEventHandler? PropertyChanged;
        public string Id { get; }
        public string Name { get; private set; }
        public string Type { get; private set; }
        public string CountLabel => $"{_visibleCount} sensors";
        public string AutomationName => $"{Name} hardware device";
        public string SensorListAutomationName => $"{Name} sensors";
        public bool IsExpanded
        {
            get => _isExpanded;
            set
            {
                if (_isExpanded == value) return;
                _isExpanded = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsExpanded)));
            }
        }
        public ObservableCollection<HardwareSensorSection> Sections { get; } = [];
        public IReadOnlyList<HardwareSensorItem> AllSensors => _allSensors;

        public bool Matches(HardwareDeviceSnapshot device) =>
            Id == device.Id && Name == device.Name && Type == device.Type;

        public bool Update(HardwareDeviceSnapshot device)
        {
            var changed = false;
            if (!string.Equals(Name, device.Name, StringComparison.Ordinal))
            {
                Name = device.Name;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Name)));
                changed = true;
            }
            if (!string.Equals(Type, device.Type, StringComparison.Ordinal))
            {
                Type = device.Type;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Type)));
                changed = true;
            }
            if (ReplaceSensors(device.Sensors)) changed = true;
            return changed;
        }

        public HardwareDeviceGroup? Filter(string query, string? type)
        {
            var sensors = _allSensors.Where(sensor =>
                (string.IsNullOrWhiteSpace(type) || type == "All sensor types" ||
                 sensor.Type.Equals(type, StringComparison.OrdinalIgnoreCase)) &&
                (string.IsNullOrWhiteSpace(query) ||
                 Name.Contains(query, StringComparison.CurrentCultureIgnoreCase) ||
                 sensor.Name.Contains(query, StringComparison.CurrentCultureIgnoreCase) ||
                 sensor.Type.Contains(query, StringComparison.CurrentCultureIgnoreCase))).ToList();
            if (sensors.Count == 0) return null;
            RebuildSections(sensors);
            return this;
        }

        private bool ReplaceSensors(IReadOnlyList<HardwareSensorReading> readings)
        {
            var changed = readings.Count != _allSensors.Count;
            var next = new List<HardwareSensorItem>(readings.Count);
            var index = new Dictionary<string, HardwareSensorItem>(StringComparer.Ordinal);
            foreach (var reading in readings)
            {
                if (_sensorIndex.TryGetValue(reading.Id, out var existing) && existing.Matches(reading))
                {
                    existing.Update(reading);
                    next.Add(existing);
                }
                else
                {
                    next.Add(new HardwareSensorItem(reading));
                    changed = true;
                }
                index[reading.Id] = next[^1];
            }
            if (!changed && next.Select(sensor => sensor.Id).SequenceEqual(_allSensors.Select(sensor => sensor.Id)))
            {
                foreach (var sensor in next) sensor.Update(readings.First(reading => reading.Id == sensor.Id));
                RebuildSections(next);
                return false;
            }
            _allSensors = next;
            _sensorIndex = index;
            RebuildSections(next);
            return true;
        }

        private void RebuildSections(IReadOnlyList<HardwareSensorItem> sensors)
        {
            _visibleCount = sensors.Count;
            var built = sensors
                .GroupBy(sensor => sensor.Type, StringComparer.OrdinalIgnoreCase)
                .OrderBy(group => HardwareSensorLayout.TypeOrder(group.Key))
                .ThenBy(group => group.Key, StringComparer.OrdinalIgnoreCase)
                .Select(group =>
                {
                    var ordered = group
                        .OrderBy(sensor => HardwareSensorLayout.CoreSortKey(sensor.Name))
                        .ThenBy(sensor => sensor.Name, StringComparer.CurrentCultureIgnoreCase)
                        .ToList();
                    var cores = ordered.Where(sensor => HardwareSensorLayout.IsPerCoreSensor(sensor.Type, sensor.Name)).ToList();
                    var metrics = ordered.Where(sensor => !HardwareSensorLayout.IsPerCoreSensor(sensor.Type, sensor.Name)).ToList();
                    return (Key: group.Key, Cores: cores, Metrics: metrics);
                })
                .Where(section => section.Cores.Count > 0 || section.Metrics.Count > 0)
                .ToList();

            while (Sections.Count > built.Count) Sections.RemoveAt(Sections.Count - 1);
            for (var index = 0; index < built.Count; index++)
            {
                var desired = built[index];
                if (index < Sections.Count &&
                    Sections[index].Key.Equals(desired.Key, StringComparison.OrdinalIgnoreCase))
                {
                    Sections[index].Sync(desired.Cores, desired.Metrics);
                    continue;
                }

                var existingIndex = -1;
                for (var candidate = index; candidate < Sections.Count; candidate++)
                {
                    if (!Sections[candidate].Key.Equals(desired.Key, StringComparison.OrdinalIgnoreCase)) continue;
                    existingIndex = candidate;
                    break;
                }

                if (existingIndex >= 0)
                {
                    Sections.Move(existingIndex, index);
                    Sections[index].Sync(desired.Cores, desired.Metrics);
                }
                else
                {
                    Sections.Insert(index, new HardwareSensorSection(desired.Key, desired.Cores, desired.Metrics));
                }
            }

            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(CountLabel)));
        }
    }

    public sealed class HardwareSensorSection : INotifyPropertyChanged
    {
        private string _subtitle = string.Empty;
        private Visibility _hasCoreSensors = Visibility.Collapsed;
        private Visibility _hasMetricSensors = Visibility.Collapsed;

        public HardwareSensorSection(string type, IReadOnlyList<HardwareSensorItem> cores, IReadOnlyList<HardwareSensorItem> metrics)
        {
            Key = type;
            Title = type.ToUpperInvariant();
            CoreSensors = [];
            MetricSensors = [];
            CoreGridAutomationName = $"{type} core grid";
            MetricGridAutomationName = $"{type} sensor grid";
            Sync(cores, metrics);
        }

        public event PropertyChangedEventHandler? PropertyChanged;
        public string Key { get; }
        public string Title { get; }
        public string Subtitle
        {
            get => _subtitle;
            private set
            {
                if (_subtitle == value) return;
                _subtitle = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Subtitle)));
            }
        }
        public ObservableCollection<HardwareSensorItem> CoreSensors { get; }
        public ObservableCollection<HardwareSensorItem> MetricSensors { get; }
        public Visibility HasCoreSensors
        {
            get => _hasCoreSensors;
            private set
            {
                if (_hasCoreSensors == value) return;
                _hasCoreSensors = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(HasCoreSensors)));
            }
        }
        public Visibility HasMetricSensors
        {
            get => _hasMetricSensors;
            private set
            {
                if (_hasMetricSensors == value) return;
                _hasMetricSensors = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(HasMetricSensors)));
            }
        }
        public string CoreGridAutomationName { get; }
        public string MetricGridAutomationName { get; }

        public void Sync(IReadOnlyList<HardwareSensorItem> cores, IReadOnlyList<HardwareSensorItem> metrics)
        {
            SyncObservable(CoreSensors, cores, sensor => sensor.Id);
            SyncObservable(MetricSensors, metrics, sensor => sensor.Id);
            var parts = new List<string>();
            if (cores.Count > 0) parts.Add($"{cores.Count} cores");
            if (metrics.Count > 0) parts.Add($"{metrics.Count} readings");
            Subtitle = string.Join(" · ", parts);
            HasCoreSensors = cores.Count > 0 ? Visibility.Visible : Visibility.Collapsed;
            HasMetricSensors = metrics.Count > 0 ? Visibility.Visible : Visibility.Collapsed;
        }
    }

    public sealed class HardwareSensorItem : INotifyPropertyChanged
    {
        private HardwareSensorReading _reading;
        private readonly ObservableCollection<double> _sparkValues = [];

        public HardwareSensorItem(HardwareSensorReading reading) => _reading = reading;

        public event PropertyChangedEventHandler? PropertyChanged;
        public string Id => _reading.Id;
        public string Name => _reading.Name;
        public string Type => _reading.Type;
        public string Unit => _reading.Unit;
        public double Value => _reading.Value;
        public string ValueLabel => _reading.ValueLabel;
        public string MinimumLabel => _reading.MinimumLabel;
        public string MaximumLabel => _reading.MaximumLabel;
        public string CoreLabel => HardwareSensorLayout.CoreShortLabel(Name);
        public string GraphAutomationName => $"Add graph for {Name}";
        public string RangeSummary => $"min {MinimumLabel}  ·  max {MaximumLabel}";
        public ObservableCollection<double> SparkValues => _sparkValues;

        public bool Matches(HardwareSensorReading reading) =>
            Id == reading.Id && Name == reading.Name && Type == reading.Type && Unit == reading.Unit;

        public void Update(HardwareSensorReading reading)
        {
            var valueChanged = ValueLabel != reading.ValueLabel;
            var minimumChanged = MinimumLabel != reading.MinimumLabel;
            var maximumChanged = MaximumLabel != reading.MaximumLabel;
            _reading = reading;
            if (valueChanged)
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Value)));
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(ValueLabel)));
            }
            if (minimumChanged || maximumChanged)
            {
                if (minimumChanged) PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MinimumLabel)));
                if (maximumChanged) PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(MaximumLabel)));
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RangeSummary)));
            }
        }

        public void UpdateSparkValues(IReadOnlyList<double> values)
        {
            var shared = Math.Min(_sparkValues.Count, values.Count);
            for (var index = 0; index < shared; index++)
                if (Math.Abs(_sparkValues[index] - values[index]) > 0.000001) _sparkValues[index] = values[index];
            while (_sparkValues.Count > values.Count) _sparkValues.RemoveAt(_sparkValues.Count - 1);
            for (var index = _sparkValues.Count; index < values.Count; index++) _sparkValues.Add(values[index]);
        }
    }
}
