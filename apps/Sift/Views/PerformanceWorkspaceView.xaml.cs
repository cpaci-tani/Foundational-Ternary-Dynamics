using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Sift.Models;
using Sift.Presentation;
using Sift.Services;
using Sift.WinUI.Infrastructure;
using Sift.Infrastructure.Icons;
using LiveChartsCore;
using LiveChartsCore.SkiaSharpView;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Views;

public sealed partial class PerformanceWorkspaceView : UserControl
{
    private readonly ObservableCollection<double> _cpuHistory = [];
    private readonly ObservableCollection<double> _memoryHistory = [];
    private readonly ObservableCollection<PerformanceProcessRow> _topCpu = [];
    private readonly ObservableCollection<PerformanceProcessRow> _topMemory = [];
    private LineSeries<double>? _cpuSeries;
    private LineSeries<double>? _memorySeries;
    private double _lineSmoothness = ChartSmoothingPolicy.ResolveSmoothness(ChartSmoothingPolicy.Default);
    private bool _showCpu = true;
    private bool _showMemory = true;
    private bool _showLegend = true;
    private bool _bindingToggles;

    public PerformanceWorkspaceView()
    {
        InitializeComponent();
        TopCpuList.ItemsSource = _topCpu;
        TopMemoryList.ItemsSource = _topMemory;
        _bindingToggles = true;
        ShowCpuCheck.IsChecked = true;
        ShowMemoryCheck.IsChecked = true;
        ShowLegendCheck.IsChecked = true;
        _bindingToggles = false;
        ConfigureChart();
    }

    public event EventHandler? RefreshRequested;
    public event EventHandler? PauseRequested;
    public event EventHandler? ChartOptionsChanged;
    public int HistoryLimit { get; set; } = 120;

    public bool ShowCpuSeries => ShowCpuCheck.IsChecked == true;
    public bool ShowMemorySeries => ShowMemoryCheck.IsChecked == true;
    public bool ShowLegend => ShowLegendCheck.IsChecked == true;

    public void ApplyChartOptions(AppSettings settings)
    {
        HistoryLimit = Math.Clamp(settings.ChartHistory, 30, 600);
        SetSmoothing(settings.ChartSmoothing);
        _bindingToggles = true;
        ShowCpuCheck.IsChecked = settings.PerformanceShowCpuSeries;
        ShowMemoryCheck.IsChecked = settings.PerformanceShowMemorySeries;
        ShowLegendCheck.IsChecked = settings.PerformanceShowLegend;
        _bindingToggles = false;
        _showCpu = settings.PerformanceShowCpuSeries;
        _showMemory = settings.PerformanceShowMemorySeries;
        _showLegend = settings.PerformanceShowLegend;
        if (!_showCpu && !_showMemory)
        {
            _showCpu = true;
            ShowCpuCheck.IsChecked = true;
        }
        ApplySeriesVisibility();
        UpdateSubtitle();
    }

    public void SetSmoothing(string? smoothing)
    {
        _lineSmoothness = ChartSmoothingPolicy.ResolveSmoothness(smoothing);
        if (_cpuSeries is not null) _cpuSeries.LineSmoothness = _lineSmoothness;
        if (_memorySeries is not null) _memorySeries.LineSmoothness = _lineSmoothness;
    }

    public void Bind(SystemSnapshot snapshot)
    {
        var cpu = snapshot.Counters?.CpuPercent ?? snapshot.CpuPercent;
        Append(_cpuHistory, cpu);
        Append(_memoryHistory, snapshot.MemoryPercent);
        SetText(CpuText, SiftDisplay.CpuPercent(cpu));
        SetText(CpuDetail, snapshot.Counters is null
            ? $"{snapshot.Processes.Count:N0} processes · process-sampled CPU"
            : $"{snapshot.Processes.Count:N0} processes · PDH total CPU");
        SetText(MemoryText, SiftDisplay.MemoryPercentUsed(snapshot.MemoryPercent));
        SetText(MemoryDetail, SiftDisplay.PhysicalMemoryGb(snapshot.UsedMemoryGb, snapshot.TotalMemoryGb));
        SetText(ProcessText, SiftDisplay.CountNoun(snapshot.Processes.Count, "process", "processes"));
        SetText(ProcessDetail, "Sampled this session · top consumers below");
        if (snapshot.Counters is { } counters)
        {
            SetText(DiskText, SiftDisplay.DiskReadWriteMiB(counters.DiskReadMbPerSec, counters.DiskWriteMbPerSec));
            SetText(DiskDetail, "PDH physical disk throughput (MiB/s)");
        }
        else
        {
            var processIo = snapshot.Processes.Sum(x => x.ReadRateMb + x.WriteRateMb);
            SetText(DiskText, SiftDisplay.MebibytesPerSec(processIo));
            SetText(DiskDetail, "Sum of process I/O rates (MiB/s)");
        }

        Reconcile(_topCpu, snapshot.Processes.OrderByDescending(x => x.CpuPercent).Take(8),
            x => $"PID {x.Id} · {SiftDisplay.WorkingSetMiBShort(x.MemoryMb)}", x => SiftDisplay.CpuPercent(x.CpuPercent));
        Reconcile(_topMemory, snapshot.Processes.OrderByDescending(x => x.MemoryMb).Take(8),
            x => $"PID {x.Id} · {SiftDisplay.CpuPercent(x.CpuPercent)}", x => SiftDisplay.WorkingSetMiBShort(x.MemoryMb));
        UpdateSubtitle();
    }

    public void SetBusy(bool busy, string status)
    {
        BusyRing.IsActive = busy;
        RefreshButton.IsEnabled = !busy;
        StatusText.Text = status;
    }

    public void SetPaused(bool paused)
    {
        PauseButton.Label = paused ? "Resume" : "Pause";
        PauseButton.Icon = paused ? SiftIconKind.Play : SiftIconKind.Pause;
        StatusText.Text = paused ? "Sampling paused · existing history retained" : "Live sampling active";
    }

    public void FocusSearch() => RefreshButton.Focus(FocusState.Programmatic);

    private void ConfigureChart()
    {
        _cpuSeries = new LineSeries<double>
        {
            Name = "CPU",
            Values = _cpuHistory,
            Stroke = ChartTheme.Stroke(ChartTheme.Clay, 2.5f),
            Fill = ChartTheme.Fill(ChartTheme.Clay, 0x18),
            GeometrySize = 0,
            LineSmoothness = _lineSmoothness
        };
        _memorySeries = new LineSeries<double>
        {
            Name = "Memory",
            Values = _memoryHistory,
            Stroke = ChartTheme.Stroke(ChartTheme.Sage, 2.5f),
            Fill = ChartTheme.Fill(ChartTheme.Sage, 0x18),
            GeometrySize = 0,
            LineSmoothness = _lineSmoothness
        };
        HistoryChart.Series = [_cpuSeries, _memorySeries];
        HistoryChart.AnimationsSpeed = TimeSpan.Zero;
        ChartTheme.ApplyChrome(HistoryChart, showLegend: true, showAxes: true, yMin: 0, yMax: 100,
            yLabeler: value => $"{value:0}%");
    }

    private void ApplySeriesVisibility()
    {
        if (_cpuSeries is null || _memorySeries is null) return;
        _cpuSeries.IsVisible = _showCpu;
        _memorySeries.IsVisible = _showMemory;
        ChartTheme.ApplyChrome(HistoryChart, _showLegend, showAxes: true, yMin: 0, yMax: 100,
            yLabeler: value => $"{value:0}%");
    }

    private void UpdateSubtitle()
    {
        var series = (_showCpu, _showMemory) switch
        {
            (true, true) => "CPU and memory percentage",
            (true, false) => "CPU percentage",
            (false, true) => "Memory percentage",
            _ => "No series selected"
        };
        SetText(HistorySubtitle, $"{series} · last {HistoryLimit} readings");
    }

    private void Append(ObservableCollection<double> values, double value)
    {
        values.Add(Math.Clamp(value, 0, 100));
        while (values.Count > HistoryLimit) values.RemoveAt(0);
    }

    private static void Reconcile(
        ObservableCollection<PerformanceProcessRow> destination,
        IEnumerable<ProcessSnapshot> processes,
        Func<ProcessSnapshot, string> context,
        Func<ProcessSnapshot, string> value)
    {
        var desired = processes.ToList();
        var desiredKeys = desired.Select(ProcessKey).ToHashSet(StringComparer.Ordinal);
        for (var index = destination.Count - 1; index >= 0; index--)
        {
            if (!desiredKeys.Contains(destination[index].Key)) destination.RemoveAt(index);
        }

        for (var index = 0; index < desired.Count; index++)
        {
            var process = desired[index];
            var key = ProcessKey(process);
            var existingIndex = IndexOf(destination, key);
            if (existingIndex < 0)
            {
                destination.Insert(index, new PerformanceProcessRow(
                    key, process.Name, context(process), value(process), process.IconPng));
                continue;
            }

            var existing = destination[existingIndex];
            existing.Update(process.Name, context(process), value(process), process.IconPng);
            if (existingIndex != index) destination.Move(existingIndex, index);
        }
    }

    private static int IndexOf(IReadOnlyList<PerformanceProcessRow> rows, string key)
    {
        for (var index = 0; index < rows.Count; index++)
            if (rows[index].Key.Equals(key, StringComparison.Ordinal)) return index;
        return -1;
    }

    private static string ProcessKey(ProcessSnapshot process) => $"{process.Id}:{process.StartTimeUtcTicks}";

    private static void SetText(TextBlock target, string value)
    {
        if (!string.Equals(target.Text, value, StringComparison.Ordinal)) target.Text = value;
    }

    private void RefreshButton_Click(object sender, RoutedEventArgs e) => RefreshRequested?.Invoke(this, EventArgs.Empty);
    private void PauseButton_Click(object sender, RoutedEventArgs e) => PauseRequested?.Invoke(this, EventArgs.Empty);

    private void ChartToggle_Changed(object sender, RoutedEventArgs e)
    {
        if (_bindingToggles) return;
        _showCpu = ShowCpuCheck.IsChecked == true;
        _showMemory = ShowMemoryCheck.IsChecked == true;
        _showLegend = ShowLegendCheck.IsChecked == true;
        if (!_showCpu && !_showMemory)
        {
            _bindingToggles = true;
            ShowCpuCheck.IsChecked = true;
            _bindingToggles = false;
            _showCpu = true;
        }
        ApplySeriesVisibility();
        UpdateSubtitle();
        ChartOptionsChanged?.Invoke(this, EventArgs.Empty);
    }

    public sealed class PerformanceProcessRow : INotifyPropertyChanged
    {
        private string _name;
        private string _context;
        private string _value;
        private byte[]? _iconPng;

        public PerformanceProcessRow(string key, string name, string context, string value, byte[]? iconPng)
        {
            Key = key;
            _name = name;
            _context = context;
            _value = value;
            _iconPng = iconPng;
        }

        public string Key { get; }
        public string Name { get => _name; private set => Set(ref _name, value); }
        public string Context { get => _context; private set => Set(ref _context, value); }
        public string Value { get => _value; private set => Set(ref _value, value); }
        public byte[]? IconPng { get => _iconPng; private set => Set(ref _iconPng, value); }
        public event PropertyChangedEventHandler? PropertyChanged;

        public void Update(string name, string context, string value, byte[]? iconPng)
        {
            Name = name;
            Context = context;
            Value = value;
            if (!SameBytes(_iconPng, iconPng)) IconPng = iconPng;
        }

        private static bool SameBytes(byte[]? left, byte[]? right) =>
            ReferenceEquals(left, right) || left is not null && right is not null && left.AsSpan().SequenceEqual(right);

        private void Set<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return;
            field = value;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
