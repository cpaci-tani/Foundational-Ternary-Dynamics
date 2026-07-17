using Sift.Models;
using Sift.Services;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Sift.WinUI.Controls;
using Sift.Infrastructure.Icons;

namespace Sift.WinUI.Views;

public sealed record SettingChangedEventArgs(string Name);
public sealed record MonitorCommandEventArgs(string Command);

public sealed partial class SettingsWorkspaceView : UserControl
{
    private AppSettings? _settings;
    private bool _binding;

    public SettingsWorkspaceView() => InitializeComponent();

    public event EventHandler<SettingChangedEventArgs>? SettingChanged;
    public event EventHandler<MonitorCommandEventArgs>? MonitorCommandRequested;
    public event EventHandler? ClearDashboardHistoryRequested;

    public void Bind(AppSettings settings)
    {
        _binding = true;
        _settings = settings;
        ConsoleVisibleToggle.IsOn = settings.ConsoleVisible;
        ConsoleWidthSlider.Value = Math.Clamp(settings.ConsoleWidth, 300, 520);
        RefreshIntervalBox.SelectedIndex = ChartRefreshIntervalPolicy.IndexOf(settings.RefreshInterval);
        HistoryNumberBox.Value = Math.Clamp(settings.ChartHistory, 30, 600);
        SmoothingBox.SelectedIndex = IndexOfSmoothing(settings.ChartSmoothing);
        PerformanceLegendToggle.IsOn = settings.PerformanceShowLegend;
        PerformanceCpuToggle.IsOn = settings.PerformanceShowCpuSeries;
        PerformanceMemoryToggle.IsOn = settings.PerformanceShowMemorySeries;
        HomeChartLegendToggle.IsOn = settings.Dashboard.ChartShowLegend;
        HomeChartAxesToggle.IsOn = settings.Dashboard.ChartShowAxes;
        HomeSmoothingBox.SelectedIndex = IndexOfSmoothing(settings.Dashboard.ChartSmoothing);
        HardwareRefreshBox.SelectedIndex = ChartRefreshIntervalPolicy.IndexOf(settings.HardwareCharts.RefreshInterval);
        HardwareHistoryBox.Value = Math.Clamp(settings.HardwareCharts.HistorySamples, 30, 600);
        HardwareSmoothingBox.SelectedIndex = IndexOfSmoothing(settings.HardwareCharts.ChartSmoothing);
        HardwareLegendToggle.IsOn = settings.HardwareCharts.ShowLegend;
        HardwareAxesToggle.IsOn = settings.HardwareCharts.ShowAxes;
        UiScaleBox.SelectedIndex = IndexOfUiScale(settings.UiScale);
        RestorePointToggle.IsOn = settings.OfferSystemRestorePoint;
        MonitorWhenClosedToggle.IsOn = settings.Dashboard.MonitorWhenClosed;
        BackgroundHardwareToggle.IsOn = settings.Dashboard.BackgroundHardwareSensors;
        DashboardNotificationsToggle.IsOn = settings.Dashboard.NotificationsEnabled;
        DashboardRetentionBox.Value = Math.Clamp(settings.Dashboard.HistoryRetentionDays, 7, 90);
        QuietStartPicker.Time = settings.Dashboard.QuietHoursStart.ToTimeSpan();
        QuietEndPicker.Time = settings.Dashboard.QuietHoursEnd.ToTimeSpan();
        AlertRuleCountText.Text = $"{settings.Dashboard.AlertRules.Count(rule => rule.Enabled):N0} enabled · {settings.Dashboard.AlertRules.Count:N0} total";
        UpdateWidthLabel();
        StatusText.Text = "Preferences are stored locally and saved automatically.";
        _binding = false;
    }

    public void FocusPrimaryControl() => ConsoleVisibleToggle.Focus(FocusState.Programmatic);

    public void SetMonitorState(string text) => DashboardMonitorStatusText.Text = text;

    private void Changed(string name)
    {
        if (_binding || _settings is null) return;
        StatusText.Text = $"Saved {name} · {DateTime.Now:T}";
        SettingChanged?.Invoke(this, new SettingChangedEventArgs(name));
    }

    private void ConsoleVisibleToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.ConsoleVisible = ConsoleVisibleToggle.IsOn;
        Changed("console visibility");
    }

    private void ConsoleWidthSlider_ValueChanged(object sender, Microsoft.UI.Xaml.Controls.Primitives.RangeBaseValueChangedEventArgs e)
    {
        UpdateWidthLabel();
        if (_settings is null) return;
        _settings.ConsoleWidth = Math.Round(ConsoleWidthSlider.Value / 20) * 20;
        Changed("console width");
    }

    private void RefreshIntervalBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_settings is null || RefreshIntervalBox.SelectedItem is not ComboBoxItem item) return;
        _settings.RefreshInterval = item.Content?.ToString() ?? "2 seconds";
        Changed("refresh interval");
    }

    private void HistoryNumberBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs args)
    {
        if (_settings is null || double.IsNaN(args.NewValue)) return;
        _settings.ChartHistory = (int)Math.Clamp(args.NewValue, 30, 600);
        Changed("chart history");
    }

    private void SmoothingBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_settings is null || SmoothingBox.SelectedItem is not ComboBoxItem item) return;
        _settings.ChartSmoothing = ChartSmoothingPolicy.Normalize(item.Content?.ToString());
        Changed("chart smoothing");
    }

    private void PerformanceLegendToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.PerformanceShowLegend = PerformanceLegendToggle.IsOn;
        Changed("performance legend");
    }

    private void PerformanceCpuToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.PerformanceShowCpuSeries = PerformanceCpuToggle.IsOn;
        if (!_settings.PerformanceShowCpuSeries && !_settings.PerformanceShowMemorySeries)
        {
            _settings.PerformanceShowMemorySeries = true;
            PerformanceMemoryToggle.IsOn = true;
        }
        Changed("performance CPU series");
    }

    private void PerformanceMemoryToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.PerformanceShowMemorySeries = PerformanceMemoryToggle.IsOn;
        if (!_settings.PerformanceShowCpuSeries && !_settings.PerformanceShowMemorySeries)
        {
            _settings.PerformanceShowCpuSeries = true;
            PerformanceCpuToggle.IsOn = true;
        }
        Changed("performance memory series");
    }

    private void HomeChartLegendToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.Dashboard.ChartShowLegend = HomeChartLegendToggle.IsOn;
        Changed("home chart legend");
    }

    private void HomeChartAxesToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.Dashboard.ChartShowAxes = HomeChartAxesToggle.IsOn;
        Changed("home chart axes");
    }

    private void HomeSmoothingBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_settings is null || HomeSmoothingBox.SelectedItem is not ComboBoxItem item) return;
        _settings.Dashboard.ChartSmoothing = ChartSmoothingPolicy.Normalize(item.Content?.ToString());
        Changed("home chart smoothing");
    }

    private void HardwareRefreshBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_settings is null || HardwareRefreshBox.SelectedItem is not ComboBoxItem item) return;
        _settings.HardwareCharts.RefreshInterval = ChartRefreshIntervalPolicy.Normalize(item.Content?.ToString());
        Changed("hardware refresh interval");
    }

    private void HardwareHistoryBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs args)
    {
        if (_settings is null || double.IsNaN(args.NewValue)) return;
        _settings.HardwareCharts.HistorySamples = (int)Math.Clamp(args.NewValue, 30, 600);
        Changed("hardware chart history");
    }

    private void HardwareSmoothingBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_settings is null || HardwareSmoothingBox.SelectedItem is not ComboBoxItem item) return;
        _settings.HardwareCharts.ChartSmoothing = ChartSmoothingPolicy.Normalize(item.Content?.ToString());
        Changed("hardware chart smoothing");
    }

    private void HardwareLegendToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.HardwareCharts.ShowLegend = HardwareLegendToggle.IsOn;
        Changed("hardware chart legend");
    }

    private void HardwareAxesToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.HardwareCharts.ShowAxes = HardwareAxesToggle.IsOn;
        Changed("hardware chart axes");
    }

    private void UiScaleBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_settings is null || UiScaleBox.SelectedItem is not ComboBoxItem item) return;
        _settings.UiScale = UiScalePolicy.Normalize(item.Content?.ToString());
        Changed("UI size");
    }

    private static int IndexOfSmoothing(string? value)
    {
        var normalized = ChartSmoothingPolicy.Normalize(value);
        for (var index = 0; index < ChartSmoothingPolicy.Options.Count; index++)
            if (ChartSmoothingPolicy.Options[index] == normalized) return index;
        return 0;
    }

    private static int IndexOfUiScale(string? value)
    {
        var normalized = UiScalePolicy.Normalize(value);
        for (var index = 0; index < UiScalePolicy.Options.Count; index++)
            if (UiScalePolicy.Options[index] == normalized) return index;
        return 1;
    }

    private void RestorePointToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.OfferSystemRestorePoint = RestorePointToggle.IsOn;
        Changed("restore-point offer");
    }

    private void MonitorWhenClosedToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.Dashboard.MonitorWhenClosed = MonitorWhenClosedToggle.IsOn;
        Changed("background monitoring");
    }

    private void BackgroundHardwareToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.Dashboard.BackgroundHardwareSensors = BackgroundHardwareToggle.IsOn;
        Changed("background hardware sensors");
    }

    private void DashboardNotificationsToggle_Toggled(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        _settings.Dashboard.NotificationsEnabled = DashboardNotificationsToggle.IsOn;
        Changed("dashboard notifications");
    }

    private void DashboardRetentionBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs args)
    {
        if (_settings is null || double.IsNaN(args.NewValue)) return;
        _settings.Dashboard.HistoryRetentionDays = (int)Math.Clamp(args.NewValue, 7, 90);
        Changed("dashboard retention");
    }

    private void PauseMonitorButton_Click(object sender, RoutedEventArgs e) =>
        MonitorCommandRequested?.Invoke(this, new MonitorCommandEventArgs("pause"));

    private void ResumeMonitorButton_Click(object sender, RoutedEventArgs e) =>
        MonitorCommandRequested?.Invoke(this, new MonitorCommandEventArgs("resume"));

    private void ClearDashboardHistoryButton_Click(object sender, RoutedEventArgs e) =>
        ClearDashboardHistoryRequested?.Invoke(this, EventArgs.Empty);

    private void QuietHours_TimeChanged(object sender, TimePickerValueChangedEventArgs args)
    {
        if (_binding || _settings is null) return;
        _settings.Dashboard.QuietHoursStart = TimeOnly.FromTimeSpan(QuietStartPicker.Time);
        _settings.Dashboard.QuietHoursEnd = TimeOnly.FromTimeSpan(QuietEndPicker.Time);
        Changed("dashboard quiet hours");
    }

    private async void ManageAlertRulesButton_Click(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        var host = new StackPanel { Spacing = 10, MaxWidth = 760 };
        var editors = new List<AlertRuleEditor>();

        void AddEditor(DashboardAlertRule rule)
        {
            var editor = new AlertRuleEditor(rule);
            editor.RemoveRequested += (_, _) =>
            {
                editors.Remove(editor);
                host.Children.Remove(editor.Root);
            };
            editors.Add(editor);
            host.Children.Add(editor.Root);
        }

        foreach (var rule in _settings.Dashboard.AlertRules) AddEditor(rule);
        var add = new SiftIconButton
        {
            Icon = SiftIconKind.Add,
            Label = "Add rule",
            Style = (Style)Application.Current.Resources["SecondaryButtonStyle"]
        };
        add.Click += (_, _) => AddEditor(new DashboardAlertRule
        {
            Id = $"custom.{Guid.NewGuid():N}",
            Title = "Custom lifecycle alert",
            MetricKey = "cpu.percent",
            Threshold = 90,
            RequiredDuration = TimeSpan.FromMinutes(5),
            Hysteresis = 5,
            Cooldown = TimeSpan.FromMinutes(30)
        });
        var content = new StackPanel { Spacing = 12 };
        content.Children.Add(new TextBlock
        {
            Text = "Rules use cataloged numeric metrics only. Duration, hysteresis, and cooldown prevent noisy alerts.",
            TextWrapping = TextWrapping.Wrap
        });
        content.Children.Add(new ScrollViewer { Content = host, MaxHeight = 570, VerticalScrollBarVisibility = ScrollBarVisibility.Auto });
        content.Children.Add(add);
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Lifecycle alert rules",
            Content = content,
            PrimaryButtonText = "Save rules",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Primary
        };
        if (await dialog.ShowAsync() != ContentDialogResult.Primary) return;
        var rules = editors.Select(editor => editor.Build()).Where(rule => rule is not null).Cast<DashboardAlertRule>().ToList();
        if (rules.Count == 0)
        {
            SetMonitorState("Keep at least one valid alert rule.");
            return;
        }
        _settings.Dashboard.AlertRules = rules;
        AlertRuleCountText.Text = $"{rules.Count(rule => rule.Enabled):N0} enabled · {rules.Count:N0} total";
        Changed("dashboard alert rules");
    }

    private void ResetButton_Click(object sender, RoutedEventArgs e)
    {
        if (_settings is null) return;
        var defaults = new AppSettings();
        _settings.ConsoleVisible = defaults.ConsoleVisible;
        _settings.ConsoleWidth = defaults.ConsoleWidth;
        _settings.RefreshInterval = defaults.RefreshInterval;
        _settings.ChartHistory = defaults.ChartHistory;
        _settings.ChartSmoothing = defaults.ChartSmoothing;
        _settings.PerformanceShowLegend = defaults.PerformanceShowLegend;
        _settings.PerformanceShowCpuSeries = defaults.PerformanceShowCpuSeries;
        _settings.PerformanceShowMemorySeries = defaults.PerformanceShowMemorySeries;
        _settings.HardwareCharts = defaults.HardwareCharts;
        _settings.UiScale = defaults.UiScale;
        _settings.OfferSystemRestorePoint = defaults.OfferSystemRestorePoint;
        _settings.Dashboard = defaults.Dashboard;
        _settings.Dashboard.AlertRules = DashboardAlertDefaults.Create();
        Bind(_settings);
        SettingChanged?.Invoke(this, new SettingChangedEventArgs("interface preferences"));
    }

    private void UpdateWidthLabel()
    {
        if (ConsoleWidthText is not null) ConsoleWidthText.Text = $"{ConsoleWidthSlider.Value:0} device-independent pixels";
    }

    private sealed class AlertRuleEditor
    {
        private static readonly string[] Metrics =
        [
            "cpu.percent", "memory.percent", "storage.lowest_free_percent", "storage.lowest_free_gb",
            "hardware.hottest_c", "battery.health_percent", "health.failed",
            "maintenance.latest_age_days", "recovery.latest_age_days"
        ];
        private readonly DashboardAlertRule _source;
        private readonly ToggleSwitch _enabled = new() { Header = "Enabled" };
        private readonly TextBox _title = new() { Header = "Title", MaxLength = 120 };
        private readonly ComboBox _metric = new() { Header = "Metric", ItemsSource = Metrics };
        private readonly NumberBox _threshold = new() { Header = "Threshold" };
        private readonly ToggleSwitch _below = new() { Header = "Trigger below threshold" };
        private readonly NumberBox _duration = new() { Header = "Duration (minutes)", Minimum = 0, Maximum = 10080 };
        private readonly NumberBox _hysteresis = new() { Header = "Hysteresis", Minimum = 0, Maximum = 100000 };
        private readonly NumberBox _cooldown = new() { Header = "Cooldown (minutes)", Minimum = 0, Maximum = 10080 };
        private readonly ComboBox _severity = new() { Header = "Severity", ItemsSource = new[] { "Info", "Warning", "Critical" } };
        private readonly ToggleSwitch _toast = new() { Header = "Allow Windows notification" };

        public AlertRuleEditor(DashboardAlertRule rule)
        {
            _source = rule;
            _enabled.IsOn = rule.Enabled;
            _title.Text = rule.Title;
            _metric.SelectedItem = Metrics.Contains(rule.MetricKey) ? rule.MetricKey : Metrics[0];
            _threshold.Value = rule.Threshold;
            _below.IsOn = rule.TriggerWhenBelow;
            _duration.Value = rule.RequiredDuration.TotalMinutes;
            _hysteresis.Value = rule.Hysteresis;
            _cooldown.Value = rule.Cooldown.TotalMinutes;
            _severity.SelectedItem = rule.Severity;
            _toast.IsOn = rule.ToastEnabled;
            var remove = new SiftIconButton
            {
                Icon = SiftIconKind.Remove,
                Label = "Remove",
                HorizontalAlignment = HorizontalAlignment.Right,
                Style = (Style)Application.Current.Resources["SiftDangerIconButtonStyle"]
            };
            remove.Click += (_, _) => RemoveRequested?.Invoke(this, EventArgs.Empty);
            var fields = new Grid { ColumnSpacing = 10, RowSpacing = 10 };
            fields.ColumnDefinitions.Add(new ColumnDefinition());
            fields.ColumnDefinitions.Add(new ColumnDefinition());
            var controls = new Control[] { _title, _metric, _threshold, _duration, _hysteresis, _cooldown, _severity, _below, _enabled, _toast };
            for (var row = 0; row < 5; row++) fields.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            for (var index = 0; index < controls.Length; index++)
            {
                Grid.SetRow(controls[index], index / 2);
                Grid.SetColumn(controls[index], index % 2);
                fields.Children.Add(controls[index]);
            }
            Root = new Border
            {
                BorderBrush = (Microsoft.UI.Xaml.Media.Brush)Application.Current.Resources["SiftLineBrush"],
                BorderThickness = new Thickness(1),
                CornerRadius = new CornerRadius(8),
                Padding = new Thickness(12),
                Child = new StackPanel { Spacing = 8, Children = { fields, remove } }
            };
        }

        public Border Root { get; }
        public event EventHandler? RemoveRequested;

        public DashboardAlertRule? Build()
        {
            if (string.IsNullOrWhiteSpace(_title.Text) || _metric.SelectedItem is not string metric ||
                double.IsNaN(_threshold.Value)) return null;
            return new DashboardAlertRule
            {
                Id = _source.Id,
                Title = _title.Text.Trim(),
                MetricKey = metric,
                Threshold = _threshold.Value,
                TriggerWhenBelow = _below.IsOn,
                RequiredDuration = TimeSpan.FromMinutes(Math.Max(0, _duration.Value)),
                Hysteresis = Math.Max(0, _hysteresis.Value),
                Cooldown = TimeSpan.FromMinutes(Math.Max(0, _cooldown.Value)),
                Severity = _severity.SelectedItem?.ToString() ?? "Warning",
                Enabled = _enabled.IsOn,
                ToastEnabled = _toast.IsOn
            };
        }
    }
}
