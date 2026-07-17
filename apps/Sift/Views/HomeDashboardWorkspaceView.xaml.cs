using Sift.Models;
using Sift.Services;
using Sift.WinUI.Controls;
using Sift.WinUI.Infrastructure.Interop;
using Sift.WinUI.Infrastructure.Dialogs;
using Sift.WinUI.Composition;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Automation;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Views;

public sealed partial class HomeDashboardWorkspaceView : UserControl, IDashboardActionInteraction
{
    private readonly IClipboardService _clipboard;
    private readonly IReadOnlyDictionary<string, DashboardWidgetDefinition> _definitions = DashboardWidgetCatalog.ById();
    private readonly Dictionary<string, DashboardWidgetHost> _hosts = new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<string, DashboardWidgetContent> _contents = new(StringComparer.OrdinalIgnoreCase);
    private DashboardProfile? _profile;
    private DashboardSnapshotDelta? _snapshot;
    private IReadOnlyList<DashboardAlert> _alerts = [];
    private DashboardPreferences _chartDefaults = new();
    private DashboardBreakpoint _breakpoint = DashboardBreakpoint.Wide;
    private bool _customizing;
    private bool _bindingProfiles;
    private bool _bindingBreakpoint;

    public HomeDashboardWorkspaceView(IClipboardService clipboard)
    {
        _clipboard = clipboard;
        InitializeComponent();
        BreakpointBox.ItemsSource = Enum.GetValues<DashboardBreakpoint>();
        BreakpointBox.SelectedItem = DashboardBreakpoint.Wide;
    }

    public void ApplyChartDefaults(DashboardPreferences preferences)
    {
        _chartDefaults = preferences;
        foreach (var content in _contents.Values)
            content.ApplyChartDefaults(preferences);
    }

    public event EventHandler? RefreshRequested;
    public event EventHandler? BeginCustomizeRequested;
    public event EventHandler? SaveCustomizeRequested;
    public event EventHandler? CancelCustomizeRequested;
    public event EventHandler? UndoRequested;
    public event EventHandler? RedoRequested;
    public event EventHandler<DashboardBreakpoint>? TidyRequested;
    public event EventHandler<string>? ProfileChanged;
    public event EventHandler<string>? AddWidgetRequested;
    public event EventHandler<string>? DuplicateWidgetRequested;
    public event EventHandler<DashboardWidgetMoveIntent>? MoveWidgetRequested;
    public event EventHandler<DashboardWidgetSizeIntent>? SizeWidgetRequested;
    public event EventHandler<DashboardWidgetVisibilityIntent>? HideWidgetRequested;
    public event EventHandler<DashboardWidgetConfigurationIntent>? ConfigureWidgetRequested;
    public event EventHandler<DashboardWidgetActionIntent>? WidgetActionRequested;
    public event EventHandler<DashboardWidgetPlacementIntent>? PlaceWidgetRequested;
    public event EventHandler<string>? ResetWidgetRequested;
    public event EventHandler<string>? OpenWorkspaceRequested;
    public event EventHandler? DuplicateProfileRequested;
    public event EventHandler? ResetProfileRequested;
    public event EventHandler? DeleteProfileRequested;
    public event EventHandler<string>? ImportProfileRequested;
    public event EventHandler? ExportProfileRequested;
    public event EventHandler<string>? RenameProfileRequested;
    public event EventHandler<int>? MoveProfileRequested;
    public event EventHandler<DashboardDensity>? ProfileDensityRequested;

    public DashboardBreakpoint ActiveBreakpoint => _breakpoint;

    public void SetProfiles(DashboardProfileDocument document)
    {
        _bindingProfiles = true;
        ProfileBox.ItemsSource = document.Profiles.Select(profile => new ProfileChoice(profile.Id, profile.Name)).ToList();
        ProfileBox.DisplayMemberPath = nameof(ProfileChoice.Name);
        ProfileBox.SelectedItem = ProfileBox.Items.Cast<ProfileChoice>().FirstOrDefault(choice =>
            choice.Id.Equals(document.ActiveProfileId, StringComparison.OrdinalIgnoreCase));
        _bindingProfiles = false;
    }

    public void BindProfile(DashboardProfile profile, bool customizing, bool canUndo, bool canRedo)
    {
        _profile = profile;
        _customizing = customizing;
        if (!customizing) SnapGhost.Visibility = Visibility.Collapsed;
        CustomizeBar.Visibility = customizing ? Visibility.Visible : Visibility.Collapsed;
        CustomizeButton.Visibility = customizing ? Visibility.Collapsed : Visibility.Visible;
        AddWidgetNormalButton.Visibility = customizing ? Visibility.Collapsed : Visibility.Visible;
        ProfileBox.IsEnabled = !customizing;
        ProfileMenuButton.IsEnabled = !customizing;
        UndoButton.IsEnabled = canUndo;
        RedoButton.IsEnabled = canRedo;
        UpdateBreakpoint();
        ReconcileWidgets();
    }

    public void ApplySnapshot(DashboardSnapshotDelta snapshot, IReadOnlyList<DashboardAlert> alerts)
    {
        _snapshot = snapshot;
        _alerts = alerts;
        foreach (var content in _contents.Values) content.ApplySnapshot(snapshot, alerts);
        SetText(StatusText, $"Updated {snapshot.TimestampUtc.ToLocalTime():T}" +
            (snapshot.Warnings.Count == 0 ? string.Empty : $" · {snapshot.Warnings.Count} unavailable source(s)"));
    }

    public void SetBusy(bool busy, string status)
    {
        BusyRing.IsActive = busy;
        RefreshButton.IsEnabled = !busy;
        SetText(StatusText, status);
    }

    public void SetMonitorStatus(string status) => SetText(MonitorText, status);

    public IReadOnlyList<DashboardHistoryRequest> HistoryRequests() => _contents
        .Where(pair => pair.Value.HistoryMetricKey is not null && pair.Value.HistoryRange > TimeSpan.FromMinutes(30))
        .Select(pair => new DashboardHistoryRequest(pair.Key, pair.Value.HistoryMetricKey!, pair.Value.HistoryRange))
        .ToList();

    public void ApplyHistory(string instanceId, IReadOnlyList<DashboardHistoryPoint> points)
    {
        if (_contents.TryGetValue(instanceId, out var content)) content.ApplyHistory(points);
    }

    public void FocusPrimary() => ProfileBox.Focus(FocusState.Programmatic);

    public void CopyExport(string json)
    {
        _clipboard.CopyText(json);
        SetText(StatusText, "Copied dashboard profile JSON.");
    }

    public void ReportActionProgress(string text) => SetBusy(true, text);

    public async Task<bool> ConfirmReviewedBatchAsync(
        OptimizeMutationReview review,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var details = new TextBox
        {
            Text = string.Join(Environment.NewLine, review.TweakPreflight.Log),
            IsReadOnly = true,
            AcceptsReturn = true,
            MinHeight = 130,
            MaxHeight = 240,
            TextWrapping = TextWrapping.Wrap,
            FontFamily = new Microsoft.UI.Xaml.Media.FontFamily("Consolas")
        };
        var panel = new StackPanel { Spacing = 10, MaxWidth = 640 };
        panel.Children.Add(new TextBlock
        {
            Text = $"Balanced contains {review.TweakPreflight.Previewed:N0} reviewed change(s). Close affected apps first; some changes may require sign-out.",
            TextWrapping = TextWrapping.Wrap
        });
        if (review.AdministratorActions.Count > 0)
            panel.Children.Add(new TextBlock
            {
                Text = "Windows will request administrator permission to " + string.Join("; then ", review.AdministratorActions) + ".",
                TextWrapping = TextWrapping.Wrap,
                Foreground = (Microsoft.UI.Xaml.Media.Brush)Application.Current.Resources["SiftWarningBrush"]
            });
        panel.Children.Add(details);
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Apply the Balanced optimization preset?",
            Content = panel,
            PrimaryButtonText = "Apply",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    public async Task<bool> ConfirmContinueWithoutRestorePointAsync(
        SystemRestorePointResult failure,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Continue without a restore point?",
            Content = new TextBlock { Text = failure.Message, TextWrapping = TextWrapping.Wrap, MaxWidth = 620 },
            PrimaryButtonText = "Continue",
            CloseButtonText = "Cancel all changes",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    public async Task<IReadOnlyList<MaintenanceFinding>?> SelectMaintenanceAsync(
        IReadOnlyList<MaintenanceFinding> findings,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var panel = new StackPanel { Spacing = 8, MaxWidth = 680 };
        var checks = new List<(CheckBox Check, MaintenanceFinding Finding)>();
        foreach (var finding in findings.Where(finding => finding.CanClean))
        {
            var check = new CheckBox
            {
                Content = $"{finding.Title} · {finding.SizeLabel}\n{finding.Detail}",
                IsChecked = finding.Confidence == MaintenanceConfidence.High && !finding.RequiresAdvancedConfirm,
                IsThreeState = false
            };
            AutomationProperties.SetName(check, $"Select {finding.Title}, {finding.SizeLabel}");
            panel.Children.Add(check);
            checks.Add((check, finding));
        }
        if (checks.Count == 0) return [];
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Select maintenance findings",
            Content = new ScrollViewer { Content = panel, MaxHeight = 520, VerticalScrollBarVisibility = ScrollBarVisibility.Auto },
            PrimaryButtonText = "Review selected",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary
            ? checks.Where(item => item.Check.IsChecked == true).Select(item => item.Finding).ToList()
            : null;
    }

    public async Task<bool> ConfirmMaintenanceAsync(
        IReadOnlyList<MaintenanceFinding> selection,
        CleanResult review,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var advanced = selection.Count(finding => finding.RequiresAdvancedConfirm);
        var text = $"Rechecked {review.Previewed:N0} selected item(s). " +
                   $"The reviewed contents will be checked once more immediately before cleanup." +
                   (advanced > 0 ? $"\n\n{advanced:N0} selection(s) have an advanced-risk confirmation." : string.Empty);
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Clean the reviewed selection?",
            Content = new TextBlock { Text = text, TextWrapping = TextWrapping.Wrap, MaxWidth = 620 },
            PrimaryButtonText = "Clean",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    public async Task<bool> ConfirmProcessAsync(
        ProcessSnapshot process,
        bool restart,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var verb = restart ? "Restart" : "End";
        var detail = restart
            ? "Sift will end the exact reviewed process instance and start its executable again. Arguments and unsaved state are not restored."
            : "Ending a process can discard unsaved work. Sift will revalidate the exact process instance before acting.";
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = $"{verb} {process.Name} ({process.Id})?",
            Content = new TextBlock { Text = detail, TextWrapping = TextWrapping.Wrap, MaxWidth = 620 },
            PrimaryButtonText = verb,
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    public async Task<bool> ConfirmServiceAsync(
        DashboardServiceSnapshot service,
        ServiceActionKind action,
        bool requiresElevation,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = $"{action} {service.DisplayName}?",
            Content = new TextBlock
            {
                Text = $"Current reviewed state: {service.Status}. Sift will revalidate the exact service before acting." +
                       (requiresElevation ? " Windows will request administrator permission." : string.Empty),
                TextWrapping = TextWrapping.Wrap,
                MaxWidth = 620
            },
            PrimaryButtonText = action.ToString(),
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    private void ReconcileWidgets()
    {
        if (_profile is null) return;
        var layout = DisplayLayout();
        DashboardGrid.Columns = layout.Columns;
        var visibleIds = layout.Placements.Where(placement => placement.Visible)
            .Select(placement => placement.InstanceId).ToHashSet(StringComparer.OrdinalIgnoreCase);

        foreach (var stale in _hosts.Keys.Where(id => !_profile.Widgets.Any(widget =>
                     widget.InstanceId.Equals(id, StringComparison.OrdinalIgnoreCase))).ToList())
        {
            DashboardGrid.Children.Remove(_hosts[stale]);
            _hosts.Remove(stale);
            _contents.Remove(stale);
        }

        foreach (var instance in _profile.Widgets)
        {
            if (!_definitions.TryGetValue(instance.DefinitionId, out var definition)) continue;
            var placement = layout.Placements.Single(value => value.InstanceId.Equals(instance.InstanceId, StringComparison.OrdinalIgnoreCase));
            if (!_hosts.TryGetValue(instance.InstanceId, out var host))
            {
                host = new DashboardWidgetHost();
                var content = new DashboardWidgetContent();
                content.Configure(definition);
                content.ApplyChartDefaults(_chartDefaults);
                content.ActionRequested += (_, request) => WidgetActionRequested?.Invoke(this,
                    new DashboardWidgetActionIntent(instance.InstanceId, request.Action,
                        request.Process, request.Service, request.AlertId));
                host.SetContent(content);
                host.MoveRequested += Host_MoveRequested;
                host.ResizeRequested += Host_ResizeRequested;
                host.SnapPreviewChanged += Host_SnapPreviewChanged;
                host.HideRequested += (_, request) => HideWidgetRequested?.Invoke(this,
                    new DashboardWidgetVisibilityIntent(request.InstanceId, _breakpoint, request.AllBreakpoints));
                host.DuplicateRequested += (_, id) => DuplicateWidgetRequested?.Invoke(this, id);
                host.ConfigureRequested += async (_, id) => await ConfigureWidgetAsync(id);
                host.OpenRequested += (_, id) => OpenWorkspaceRequested?.Invoke(this, id);
                host.ResetRequested += (_, id) => ResetWidgetRequested?.Invoke(this, id);
                host.KeyboardPlacementRequested += (_, request) => PlaceWidgetRequested?.Invoke(this,
                    new DashboardWidgetPlacementIntent(request.InstanceId, _breakpoint, request.Row,
                        request.Column, request.RowSpan, request.ColumnSpan));
                _hosts[instance.InstanceId] = host;
                _contents[instance.InstanceId] = content;
                DashboardGrid.Children.Add(host);
            }
            host.Visibility = visibleIds.Contains(instance.InstanceId) ? Visibility.Visible : Visibility.Collapsed;
            host.Configure(instance, definition, placement, layout.Columns, _customizing, _profile.Density);
            _contents[instance.InstanceId].ConfigurePresentation(placement, instance, _profile.Density);
            DashboardGridPanel.SetRow(host, placement.Row);
            DashboardGridPanel.SetColumn(host, placement.Column);
            DashboardGridPanel.SetRowSpan(host, placement.RowSpan);
            DashboardGridPanel.SetColumnSpan(host, placement.ColumnSpan);
            if (_snapshot is not null) _contents[instance.InstanceId].ApplySnapshot(_snapshot, _alerts);
        }

        EmptyState.Visibility = visibleIds.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
        SetText(LayoutSummaryText, $"{_breakpoint} · {layout.Columns} columns · {visibleIds.Count} widgets");
    }

    private DashboardBreakpointLayout Layout() => _profile!.Layouts.Single(layout => layout.Breakpoint == _breakpoint);

    private DashboardBreakpointLayout DisplayLayout()
    {
        var layout = Layout();
        return !_customizing && _breakpoint == DashboardBreakpoint.Compact && ActualWidth < 480
            ? DashboardPackingEngine.Reflow(layout, DashboardBreakpoint.Compact, 1)
            : layout;
    }

    private void UpdateBreakpoint()
    {
        var next = _customizing && BreakpointBox.SelectedItem is DashboardBreakpoint preview
            ? preview
            : ActualWidth switch
            {
                >= 1180 => DashboardBreakpoint.Wide,
                >= 760 => DashboardBreakpoint.Medium,
                _ => DashboardBreakpoint.Compact
            };
        if (_breakpoint == next) return;
        _breakpoint = next;
        _bindingBreakpoint = true;
        BreakpointBox.SelectedItem = next;
        _bindingBreakpoint = false;
    }

    private void Host_MoveRequested(object? sender, DashboardWidgetMoveRequest request)
    {
        if (!_customizing || _profile is null) return;
        MoveWidgetRequested?.Invoke(this,
            new DashboardWidgetMoveIntent(request.InstanceId, _breakpoint, request.Row, request.Column));
    }

    private void Host_ResizeRequested(object? sender, DashboardWidgetResizeRequest request)
    {
        if (!_customizing) BeginCustomizeRequested?.Invoke(this, EventArgs.Empty);
        SizeWidgetRequested?.Invoke(this, new DashboardWidgetSizeIntent(request.InstanceId, _breakpoint, request.RowSpan, request.ColumnSpan));
    }

    private void Host_SnapPreviewChanged(object? sender, DashboardWidgetSnapPreview preview)
    {
        if (!preview.Active)
        {
            SnapGhost.Visibility = Visibility.Collapsed;
            return;
        }

        var columns = Math.Clamp(DashboardGrid.Columns, 1, 6);
        var cellWidth = DashboardGridMath.CellWidth(DashboardGrid.ActualWidth, columns, DashboardGrid.Spacing);
        var columnSpan = Math.Clamp(preview.ColumnSpan, 1, columns);
        var column = Math.Clamp(preview.Column, 0, columns - columnSpan);
        var rowSpan = Math.Clamp(preview.RowSpan, 1, 12);
        SnapGhost.Width = cellWidth * columnSpan + DashboardGrid.Spacing * (columnSpan - 1);
        SnapGhost.Height = DashboardGrid.RowHeight * rowSpan + DashboardGrid.Spacing * (rowSpan - 1);
        SnapGhost.Margin = new Thickness(
            column * (cellWidth + DashboardGrid.Spacing),
            preview.Row * (DashboardGrid.RowHeight + DashboardGrid.Spacing),
            0,
            0);
        SnapGhost.Visibility = Visibility.Visible;
    }

    private async Task ConfigureWidgetAsync(string instanceId)
    {
        if (_profile is null) return;
        var instance = _profile.Widgets.Single(widget => widget.InstanceId == instanceId);
        var definition = _definitions[instance.DefinitionId];
        var title = new TextBox { Header = "Title", Text = instance.TitleOverride ?? string.Empty, PlaceholderText = "Use the default title" };
        var accent = new ComboBox { Header = "Accent", ItemsSource = new[] { "Clay", "Sage", "Neutral" }, SelectedItem = instance.Accent ?? "Clay" };
        var range = new ComboBox { Header = "History range", ItemsSource = new[] { "30 minutes", "24 hours", "7 days", "30 days", "90 days" }, SelectedItem = instance.Settings.GetValueOrDefault("timeRange", "30 minutes") };
        var metric = new TextBox { Header = "Metric key", Text = instance.Settings.GetValueOrDefault("metric", string.Empty), PlaceholderText = "For configurable metric charts" };
        var sensor = new TextBox { Header = "Sensor key", Text = instance.Settings.GetValueOrDefault("sensor", string.Empty), PlaceholderText = "For configurable sensor charts" };
        var volume = new TextBox { Header = "Volume", Text = instance.Settings.GetValueOrDefault("volume", string.Empty), PlaceholderText = "Example: C:" };
        var sort = new ComboBox { Header = "List sort", ItemsSource = new[] { "Automatic", "Highest first", "Lowest first", "Name" }, SelectedItem = instance.Settings.GetValueOrDefault("sort", "Automatic") };
        var count = new NumberBox { Header = "List item count", Minimum = 1, Maximum = 20, Value = int.TryParse(instance.Settings.GetValueOrDefault("count"), out var configuredCount) ? configuredCount : 6, SpinButtonPlacementMode = NumberBoxSpinButtonPlacementMode.Compact };
        var filter = new TextBox { Header = "List filter", Text = instance.Settings.GetValueOrDefault("filter", string.Empty), PlaceholderText = "Optional name filter" };
        var showActions = new ToggleSwitch { Header = "Show supported action buttons", IsOn = !instance.Settings.TryGetValue("showActions", out var actionSetting) || !bool.TryParse(actionSetting, out var actionEnabled) || actionEnabled };
        range.Visibility = Supports("timeRange") ? Visibility.Visible : Visibility.Collapsed;
        metric.Visibility = Supports("metric") ? Visibility.Visible : Visibility.Collapsed;
        sensor.Visibility = Supports("sensor") ? Visibility.Visible : Visibility.Collapsed;
        volume.Visibility = Supports("volume") ? Visibility.Visible : Visibility.Collapsed;
        sort.Visibility = Supports("sort") ? Visibility.Visible : Visibility.Collapsed;
        count.Visibility = Supports("count") ? Visibility.Visible : Visibility.Collapsed;
        filter.Visibility = Supports("filter") ? Visibility.Visible : Visibility.Collapsed;
        showActions.Visibility = Supports("showActions") ? Visibility.Visible : Visibility.Collapsed;
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Configure widget",
            Content = new ScrollViewer
            {
                MaxHeight = 560,
                Content = new StackPanel { Spacing = 12, Children = { title, accent, range, metric, sensor, volume, sort, count, filter, showActions } }
            },
            PrimaryButtonText = "Save",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Primary
        };
        if (await dialog.ShowAsync() == ContentDialogResult.Primary)
            ConfigureWidgetRequested?.Invoke(this, new DashboardWidgetConfigurationIntent(
                instanceId, string.IsNullOrWhiteSpace(title.Text) ? null : title.Text.Trim(),
                accent.SelectedItem?.ToString(), range.SelectedItem?.ToString() ?? "30 minutes",
                Empty(metric.Text), Empty(sensor.Text), Empty(volume.Text),
                sort.SelectedItem?.ToString() ?? "Automatic", (int)Math.Clamp(count.Value, 1, 20),
                Empty(filter.Text), showActions.IsOn));

        bool Supports(string key) => definition.SettingKeys.Contains(key, StringComparer.OrdinalIgnoreCase);
    }

    private async void AddWidgetButton_Click(object sender, RoutedEventArgs e)
    {
        if (_profile is null) return;
        var choices = _definitions.Values.OrderBy(value => value.Category).ThenBy(value => value.Title).ToList();
        var list = new ListView { ItemsSource = choices, DisplayMemberPath = nameof(DashboardWidgetDefinition.Title), SelectionMode = ListViewSelectionMode.Single, MinWidth = 420, MaxHeight = 500 };
        var dialog = new ContentDialog { XamlRoot = XamlRoot, Title = "Add widget", Content = list, PrimaryButtonText = "Add", CloseButtonText = "Cancel" };
        if (await dialog.ShowAsync() == ContentDialogResult.Primary && list.SelectedItem is DashboardWidgetDefinition definition)
            AddWidgetRequested?.Invoke(this, definition.Id);
    }

    private void ProfileMenuButton_Click(object sender, RoutedEventArgs e)
    {
        var menu = new MenuFlyout();
        Add(menu, "Rename profile", async () => await RenameProfileAsync());
        Add(menu, "Profile density", async () => await ChangeDensityAsync());
        Add(menu, "Duplicate profile", () => DuplicateProfileRequested?.Invoke(this, EventArgs.Empty));
        Add(menu, "Move profile left", () => MoveProfileRequested?.Invoke(this, -1));
        Add(menu, "Move profile right", () => MoveProfileRequested?.Invoke(this, 1));
        Add(menu, "Reset profile", () => ResetProfileRequested?.Invoke(this, EventArgs.Empty));
        Add(menu, "Delete profile", () => DeleteProfileRequested?.Invoke(this, EventArgs.Empty));
        menu.Items.Add(new MenuFlyoutSeparator());
        Add(menu, "Export profile", () => ExportProfileRequested?.Invoke(this, EventArgs.Empty));
        Add(menu, "Import profile", async () => await ImportProfileAsync());
        menu.ShowAt(ProfileMenuButton);
    }

    private async Task RenameProfileAsync()
    {
        if (_profile is null) return;
        var name = new TextBox { Text = _profile.Name, MaxLength = 80, MinWidth = 360, Header = "Profile name" };
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Rename dashboard profile",
            Content = name,
            PrimaryButtonText = "Rename",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Primary
        };
        if (await dialog.ShowAsync() == ContentDialogResult.Primary && !string.IsNullOrWhiteSpace(name.Text))
            RenameProfileRequested?.Invoke(this, name.Text.Trim());
    }

    private async Task ChangeDensityAsync()
    {
        if (_profile is null) return;
        var density = new ComboBox
        {
            Header = "Widget content density",
            ItemsSource = Enum.GetValues<DashboardDensity>(),
            SelectedItem = _profile.Density,
            MinWidth = 280
        };
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Dashboard profile density",
            Content = density,
            PrimaryButtonText = "Save",
            CloseButtonText = "Cancel"
        };
        if (await dialog.ShowAsync() == ContentDialogResult.Primary && density.SelectedItem is DashboardDensity selected)
            ProfileDensityRequested?.Invoke(this, selected);
    }

    private async Task ImportProfileAsync()
    {
        var text = new TextBox { AcceptsReturn = true, TextWrapping = TextWrapping.Wrap, MinWidth = 520, MinHeight = 260, PlaceholderText = "Paste exported dashboard profile JSON" };
        var dialog = new ContentDialog { XamlRoot = XamlRoot, Title = "Import dashboard profile", Content = text, PrimaryButtonText = "Import", CloseButtonText = "Cancel" };
        if (await dialog.ShowAsync() == ContentDialogResult.Primary) ImportProfileRequested?.Invoke(this, text.Text);
    }

    private static void Add(MenuFlyout menu, string text, Action action)
    {
        var item = new MenuFlyoutItem { Text = text };
        item.Click += (_, _) => action();
        menu.Items.Add(item);
    }

    private void RefreshButton_Click(object sender, RoutedEventArgs e) => RefreshRequested?.Invoke(this, EventArgs.Empty);
    private void CustomizeButton_Click(object sender, RoutedEventArgs e) => BeginCustomizeRequested?.Invoke(this, EventArgs.Empty);
    private void SaveButton_Click(object sender, RoutedEventArgs e) => SaveCustomizeRequested?.Invoke(this, EventArgs.Empty);
    private void CancelButton_Click(object sender, RoutedEventArgs e) => CancelCustomizeRequested?.Invoke(this, EventArgs.Empty);
    private void UndoButton_Click(object sender, RoutedEventArgs e) => UndoRequested?.Invoke(this, EventArgs.Empty);
    private void RedoButton_Click(object sender, RoutedEventArgs e) => RedoRequested?.Invoke(this, EventArgs.Empty);
    private void TidyButton_Click(object sender, RoutedEventArgs e) => TidyRequested?.Invoke(this, _breakpoint);

    private void ProfileBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (!_bindingProfiles && ProfileBox.SelectedItem is ProfileChoice choice) ProfileChanged?.Invoke(this, choice.Id);
    }

    private void BreakpointBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_bindingBreakpoint || !_customizing || BreakpointBox.SelectedItem is not DashboardBreakpoint selected) return;
        _breakpoint = selected;
        ReconcileWidgets();
    }

    private void Root_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        if (_customizing || _profile is null) return;
        var previous = _breakpoint;
        var wasSingleColumnCompact = previous == DashboardBreakpoint.Compact && e.PreviousSize.Width < 480;
        UpdateBreakpoint();
        var isSingleColumnCompact = _breakpoint == DashboardBreakpoint.Compact && e.NewSize.Width < 480;
        if (previous != _breakpoint || wasSingleColumnCompact != isSingleColumnCompact) ReconcileWidgets();
    }

    private static void SetText(TextBlock target, string value)
    {
        if (!string.Equals(target.Text, value, StringComparison.Ordinal)) target.Text = value;
    }

    private static string? Empty(string value) => string.IsNullOrWhiteSpace(value) ? null : value.Trim();

    private sealed record ProfileChoice(string Id, string Name);
}

public sealed record DashboardHistoryRequest(string InstanceId, string MetricKey, TimeSpan Range);
