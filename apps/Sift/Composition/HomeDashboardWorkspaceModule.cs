using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Infrastructure.Settings;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure.Interop;
using Sift.WinUI.Views;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Composition;

public sealed class HomeDashboardWorkspaceModule : IWorkspaceModule, IChartSettingsAware
{
    private const string OperationKey = "workspace.home.dashboard";
    private const string ActionOperationKey = "workspace.home.dashboard.action";
    private readonly IDashboardProfileStore _profiles;
    private readonly IDashboardTelemetrySource _telemetry;
    private readonly IDashboardHistoryStore _history;
    private readonly IDashboardAlertEngine _alerts;
    private readonly AppSettings _settings;
    private readonly SettingsPersistenceCoordinator _settingsPersistence;
    private readonly OperationCoordinator _operations;
    private readonly ActivityHub _activity;
    private readonly IWorkspaceNavigator _navigator;
    private readonly HomeDashboardWorkspaceView _view;
    private readonly IDashboardActionRouter _actionRouter;
    private readonly DispatcherTimer _timer = new() { Interval = TimeSpan.FromSeconds(2) };
    private readonly DashboardSamplingCoordinator _cadence = new();
    private DashboardProfileDocument _document;
    private DashboardEditSession? _edit;
    private bool _active;
    private bool _sampling;
    private bool _sourceOwnsAlerts;
    private DateTimeOffset _lastCompactionUtc;

    public HomeDashboardWorkspaceModule(
        IDashboardProfileStore profiles,
        IDashboardTelemetrySource telemetry,
        IDashboardHistoryStore history,
        IDashboardAlertEngine alerts,
        AppSettings settings,
        SettingsPersistenceCoordinator settingsPersistence,
        OperationCoordinator operations,
        ActivityHub activity,
        IWorkspaceNavigator navigator,
        IClipboardService clipboard,
        IDashboardActionRouter actionRouter)
    {
        _profiles = profiles;
        _telemetry = telemetry;
        _history = history;
        _alerts = alerts;
        _settings = settings;
        _settingsPersistence = settingsPersistence;
        _operations = operations;
        _activity = activity;
        _navigator = navigator;
        _actionRouter = actionRouter;
        _document = _profiles.LoadOrCreate(settings.HomeWidgets);
        _view = new HomeDashboardWorkspaceView(clipboard);
        _view.SetProfiles(_document);
        BindActive();
        ApplyChartSettings();
        _timer.Tick += Timer_Tick;
        _view.RefreshRequested += View_RefreshRequested;
        _view.BeginCustomizeRequested += View_BeginCustomizeRequested;
        _view.SaveCustomizeRequested += View_SaveCustomizeRequested;
        _view.CancelCustomizeRequested += View_CancelCustomizeRequested;
        _view.UndoRequested += View_UndoRequested;
        _view.RedoRequested += View_RedoRequested;
        _view.TidyRequested += View_TidyRequested;
        _view.ProfileChanged += View_ProfileChanged;
        _view.AddWidgetRequested += View_AddWidgetRequested;
        _view.DuplicateWidgetRequested += View_DuplicateWidgetRequested;
        _view.MoveWidgetRequested += View_MoveWidgetRequested;
        _view.SizeWidgetRequested += View_SizeWidgetRequested;
        _view.HideWidgetRequested += View_HideWidgetRequested;
        _view.ConfigureWidgetRequested += View_ConfigureWidgetRequested;
        _view.WidgetActionRequested += View_WidgetActionRequested;
        _view.PlaceWidgetRequested += View_PlaceWidgetRequested;
        _view.ResetWidgetRequested += View_ResetWidgetRequested;
        _view.OpenWorkspaceRequested += View_OpenWorkspaceRequested;
        _view.DuplicateProfileRequested += View_DuplicateProfileRequested;
        _view.ResetProfileRequested += View_ResetProfileRequested;
        _view.DeleteProfileRequested += View_DeleteProfileRequested;
        _view.ImportProfileRequested += View_ImportProfileRequested;
        _view.ExportProfileRequested += View_ExportProfileRequested;
        _view.RenameProfileRequested += View_RenameProfileRequested;
        _view.MoveProfileRequested += View_MoveProfileRequested;
        _view.ProfileDensityRequested += View_ProfileDensityRequested;
    }

    public string Key => "Home";
    public string Title => "Home";
    public Control View => _view;

    public void ApplyChartSettings() => _view.ApplyChartDefaults(_settings.Dashboard);

    public async Task ActivateAsync(CancellationToken cancellationToken = default)
    {
        _active = true;
        ApplyChartSettings();
        await SampleAsync(DashboardSampleKind.Fast, showBusy: true, cancellationToken);
        await LoadHistoryAsync(cancellationToken);
        _cadence.Reset(DateTimeOffset.UtcNow);
        if (_active) _timer.Start();
    }

    public async Task RefreshAsync(CancellationToken cancellationToken = default) =>
        await SampleAsync(DashboardSampleKind.Slow, showBusy: true, cancellationToken);

    public void Deactivate()
    {
        _active = false;
        _timer.Stop();
        _operations.Cancel(OperationKey);
        _operations.Cancel(ActionOperationKey);
    }

    public void FocusPrimarySearch() => _view.FocusPrimary();

    public void Dispose()
    {
        Deactivate();
        _timer.Tick -= Timer_Tick;
        _view.RefreshRequested -= View_RefreshRequested;
        _view.BeginCustomizeRequested -= View_BeginCustomizeRequested;
        _view.SaveCustomizeRequested -= View_SaveCustomizeRequested;
        _view.CancelCustomizeRequested -= View_CancelCustomizeRequested;
        _view.UndoRequested -= View_UndoRequested;
        _view.RedoRequested -= View_RedoRequested;
        _view.TidyRequested -= View_TidyRequested;
        _view.ProfileChanged -= View_ProfileChanged;
        _view.AddWidgetRequested -= View_AddWidgetRequested;
        _view.DuplicateWidgetRequested -= View_DuplicateWidgetRequested;
        _view.MoveWidgetRequested -= View_MoveWidgetRequested;
        _view.SizeWidgetRequested -= View_SizeWidgetRequested;
        _view.HideWidgetRequested -= View_HideWidgetRequested;
        _view.ConfigureWidgetRequested -= View_ConfigureWidgetRequested;
        _view.WidgetActionRequested -= View_WidgetActionRequested;
        _view.PlaceWidgetRequested -= View_PlaceWidgetRequested;
        _view.ResetWidgetRequested -= View_ResetWidgetRequested;
        _view.OpenWorkspaceRequested -= View_OpenWorkspaceRequested;
        _view.DuplicateProfileRequested -= View_DuplicateProfileRequested;
        _view.ResetProfileRequested -= View_ResetProfileRequested;
        _view.DeleteProfileRequested -= View_DeleteProfileRequested;
        _view.ImportProfileRequested -= View_ImportProfileRequested;
        _view.ExportProfileRequested -= View_ExportProfileRequested;
        _view.RenameProfileRequested -= View_RenameProfileRequested;
        _view.MoveProfileRequested -= View_MoveProfileRequested;
        _view.ProfileDensityRequested -= View_ProfileDensityRequested;
    }

    private async Task SampleAsync(
        DashboardSampleKind kind,
        bool showBusy,
        CancellationToken cancellationToken = default)
    {
        if (_sampling) return;
        _sampling = true;
        try
        {
            if (showBusy) _view.SetBusy(true, "Refreshing dashboard…");
            var outcome = await _operations.RunLatestAsync(
                OperationKey, Key, $"dashboard {kind.ToString().ToLowerInvariant()} sample",
                token => _telemetry.SampleAsync(kind, token), cancellationToken);
            if (outcome.Cancelled || !_active) return;
            if (!outcome.Succeeded || outcome.Value is null)
            {
                _view.SetBusy(false, $"Dashboard refresh failed: {outcome.Error?.Message ?? "unknown error"}");
                return;
            }

            var read = outcome.Value;
            var snapshot = read.Snapshot;
            _sourceOwnsAlerts = read.SourceOwnsAlerts;
            if (read.SourceOwnsAlerts && read.Alerts is not null)
                await _alerts.SynchronizeAsync(read.Alerts, cancellationToken);
            else if (read.IsFresh)
                await _alerts.EvaluateAsync(snapshot, _settings.Dashboard, cancellationToken);

            if (read.IsFresh && !read.SourceOwnsHistory)
                await _history.AppendAsync(snapshot.GetChangedMetrics(), cancellationToken);
            if (!read.SourceOwnsHistory && DateTimeOffset.UtcNow - _lastCompactionUtc > TimeSpan.FromHours(12))
            {
                await _history.CompactAsync(DateTimeOffset.UtcNow,
                    Math.Clamp(_settings.Dashboard.HistoryRetentionDays, 7, 365), cancellationToken);
                _lastCompactionUtc = DateTimeOffset.UtcNow;
            }
            if (read.IsFresh) _view.ApplySnapshot(snapshot, _alerts.Alerts);
            _timer.Interval = DashboardSamplingCoordinator.Delay(
                snapshot.Metrics.TryGetValue("power.battery_saver", out var saver) && saver.Value > 0);
            if (read.IsFresh && kind != DashboardSampleKind.Fast) await LoadHistoryAsync(cancellationToken);
            _view.SetMonitorStatus(read.Status);
            if (showBusy) _view.SetBusy(false, read.IsFresh
                ? $"Updated {snapshot.TimestampUtc.ToLocalTime():T}"
                : $"No new monitor sample · last update {snapshot.TimestampUtc.ToLocalTime():T}");
        }
        finally { _sampling = false; }
    }

    private async Task LoadHistoryAsync(CancellationToken cancellationToken)
    {
        var now = DateTimeOffset.UtcNow;
        foreach (var request in _view.HistoryRequests())
        {
            cancellationToken.ThrowIfCancellationRequested();
            var points = await _history.QueryAsync(request.MetricKey, now - request.Range, now, cancellationToken);
            _view.ApplyHistory(request.InstanceId, points);
        }
    }

    private DashboardProfile Active() => _document.Profiles.Single(profile =>
        profile.Id.Equals(_document.ActiveProfileId, StringComparison.OrdinalIgnoreCase));

    private DashboardProfile Displayed() => _edit?.WorkingProfile ?? Active();

    private void BindActive()
    {
        _view.SetProfiles(_document);
        _view.BindProfile(Displayed(), _edit is not null, _edit?.CanUndo == true, _edit?.CanRedo == true);
    }

    private void Mutate(Action<DashboardProfile> mutation)
    {
        _edit ??= new DashboardEditSession(Active());
        _edit.Apply(mutation);
        BindActive();
    }

    // A void async event handler that throws unwinds to App.UnhandledException, which fails closed
    // and terminates the process. These handlers reach SQLite and the monitor IPC channel, so a
    // transient DB lock, disk-full, or dead monitor host must be contained here rather than crash a
    // passive background timer. Report through the activity feed and status bar instead.
    private async Task GuardedAsync(string context, Func<Task> work)
    {
        try
        {
            await work();
        }
        catch (Exception exception)
        {
            _activity.Warning(Key, $"{context} did not complete", exception.Message);
            try { _view.SetBusy(false, $"{context} did not complete: {exception.Message}"); }
            catch (Exception statusException) { System.Diagnostics.Debug.WriteLine(statusException); }
        }
    }

    private async void Timer_Tick(object? sender, object e)
    {
        if (!_active || _sampling) return;
        var kind = _cadence.Next(DateTimeOffset.UtcNow);
        await GuardedAsync("Dashboard sample", () => SampleAsync(kind, showBusy: false));
    }

    private async void View_RefreshRequested(object? sender, EventArgs e) =>
        await GuardedAsync("Dashboard refresh", () => RefreshAsync());

    private void View_BeginCustomizeRequested(object? sender, EventArgs e)
    {
        _edit ??= new DashboardEditSession(Active());
        BindActive();
    }

    private void View_SaveCustomizeRequested(object? sender, EventArgs e)
    {
        if (_edit is null) return;
        var index = _document.Profiles.FindIndex(profile => profile.Id.Equals(_document.ActiveProfileId, StringComparison.OrdinalIgnoreCase));
        _document.Profiles[index] = _edit.Commit();
        _edit = null;
        _profiles.Save(_document);
        BindActive();
        _activity.Info(Key, "Saved dashboard layout");
    }

    private void View_CancelCustomizeRequested(object? sender, EventArgs e)
    {
        _edit = null;
        BindActive();
    }

    private void View_UndoRequested(object? sender, EventArgs e)
    {
        if (_edit?.Undo() == true) BindActive();
    }

    private void View_RedoRequested(object? sender, EventArgs e)
    {
        if (_edit?.Redo() == true) BindActive();
    }

    private void View_TidyRequested(object? sender, DashboardBreakpoint breakpoint) => Mutate(profile =>
    {
        var index = profile.Layouts.FindIndex(layout => layout.Breakpoint == breakpoint);
        profile.Layouts[index] = DashboardPackingEngine.Tidy(profile.Layouts[index]);
    });

    private void View_ProfileChanged(object? sender, string profileId)
    {
        if (!_document.Profiles.Any(profile => profile.Id.Equals(profileId, StringComparison.OrdinalIgnoreCase))) return;
        _edit = null;
        _document.ActiveProfileId = profileId;
        _profiles.Save(_document);
        BindActive();
    }

    private void View_AddWidgetRequested(object? sender, string definitionId) => Mutate(profile =>
    {
        var definition = DashboardWidgetCatalog.ById()[definitionId];
        var existing = profile.Widgets.FirstOrDefault(widget =>
            widget.DefinitionId.Equals(definitionId, StringComparison.OrdinalIgnoreCase));
        if (existing is not null && !definition.AllowMultiple)
        {
            foreach (var layout in profile.Layouts)
                layout.Placements.Single(placement => placement.InstanceId == existing.InstanceId).Visible = true;
            return;
        }
        var instanceId = $"{profile.Id}.{definition.Id}.{Guid.NewGuid():N}";
        profile.Widgets.Add(new DashboardWidgetInstance { InstanceId = instanceId, DefinitionId = definition.Id });
        foreach (var layout in profile.Layouts)
        {
            var columnSpan = Math.Clamp(definition.DefaultColumnSpan, 1, layout.Columns);
            var row = layout.Placements.Where(placement => placement.Visible)
                .Select(placement => placement.Row + placement.RowSpan).DefaultIfEmpty(0).Max();
            layout.Placements.Add(new DashboardPlacement
            {
                InstanceId = instanceId,
                Row = row,
                Column = 0,
                ColumnSpan = columnSpan,
                RowSpan = definition.DefaultRowSpan
            });
        }
    });

    private void View_DuplicateWidgetRequested(object? sender, string instanceId) => Mutate(profile =>
    {
        var source = profile.Widgets.Single(widget => widget.InstanceId == instanceId);
        var definition = DashboardWidgetCatalog.ById()[source.DefinitionId];
        if (!definition.AllowMultiple) return;
        var copyId = $"{profile.Id}.{definition.Id}.{Guid.NewGuid():N}";
        profile.Widgets.Add(new DashboardWidgetInstance
        {
            InstanceId = copyId,
            DefinitionId = source.DefinitionId,
            TitleOverride = source.TitleOverride,
            Accent = source.Accent,
            Settings = new Dictionary<string, string>(source.Settings, StringComparer.OrdinalIgnoreCase)
        });
        foreach (var layout in profile.Layouts)
        {
            var placement = layout.Placements.Single(value => value.InstanceId == instanceId);
            layout.Placements.Add(new DashboardPlacement
            {
                InstanceId = copyId,
                Row = placement.Row + placement.RowSpan,
                Column = placement.Column,
                RowSpan = placement.RowSpan,
                ColumnSpan = placement.ColumnSpan,
                Visible = placement.Visible
            });
            var placed = DashboardPackingEngine.Place(layout, copyId,
                placement.Row + placement.RowSpan, placement.Column, placement.RowSpan, placement.ColumnSpan);
            layout.Placements = placed.Placements;
        }
    });

    private void View_MoveWidgetRequested(object? sender, DashboardWidgetMoveIntent intent) => Mutate(profile =>
    {
        var index = profile.Layouts.FindIndex(layout => layout.Breakpoint == intent.Breakpoint);
        var layout = profile.Layouts[index];
        var current = layout.Placements.Single(placement => placement.InstanceId == intent.InstanceId);
        profile.Layouts[index] = DashboardPackingEngine.Place(layout, intent.InstanceId,
            intent.Row, intent.Column, current.RowSpan, current.ColumnSpan);
    });

    private void View_SizeWidgetRequested(object? sender, DashboardWidgetSizeIntent intent) => Mutate(profile =>
    {
        var index = profile.Layouts.FindIndex(layout => layout.Breakpoint == intent.Breakpoint);
        var layout = profile.Layouts[index];
        var current = layout.Placements.Single(placement => placement.InstanceId == intent.InstanceId);
        var definition = DashboardWidgetCatalog.ById()[profile.Widgets.Single(widget => widget.InstanceId == intent.InstanceId).DefinitionId];
        var rowSpan = Math.Clamp(intent.RowSpan, definition.MinRowSpan, definition.MaxRowSpan);
        var columnSpan = Math.Clamp(intent.ColumnSpan, Math.Min(definition.MinColumnSpan, layout.Columns),
            Math.Min(definition.MaxColumnSpan, layout.Columns));
        profile.Layouts[index] = DashboardPackingEngine.Place(layout, intent.InstanceId,
            current.Row, current.Column, rowSpan, columnSpan);
    });

    private void View_PlaceWidgetRequested(object? sender, DashboardWidgetPlacementIntent intent) => Mutate(profile =>
    {
        var index = profile.Layouts.FindIndex(layout => layout.Breakpoint == intent.Breakpoint);
        var layout = profile.Layouts[index];
        var definition = DashboardWidgetCatalog.ById()[profile.Widgets.Single(widget => widget.InstanceId == intent.InstanceId).DefinitionId];
        var rowSpan = Math.Clamp(intent.RowSpan, definition.MinRowSpan, definition.MaxRowSpan);
        var columnSpan = Math.Clamp(intent.ColumnSpan, Math.Min(definition.MinColumnSpan, layout.Columns),
            Math.Min(definition.MaxColumnSpan, layout.Columns));
        profile.Layouts[index] = DashboardPackingEngine.Place(layout, intent.InstanceId,
            intent.Row, intent.Column, rowSpan, columnSpan);
    });

    private void View_ResetWidgetRequested(object? sender, string instanceId) => Mutate(profile =>
    {
        var widget = profile.Widgets.Single(value => value.InstanceId == instanceId);
        var definition = DashboardWidgetCatalog.ById()[widget.DefinitionId];
        widget.TitleOverride = null;
        widget.Accent = null;
        widget.Settings.Clear();
        foreach (var layout in profile.Layouts)
        {
            var placement = layout.Placements.Single(value => value.InstanceId == instanceId);
            var reset = DashboardPackingEngine.Place(layout, instanceId, placement.Row, placement.Column,
                definition.DefaultRowSpan, Math.Min(definition.DefaultColumnSpan, layout.Columns));
            layout.Placements = reset.Placements;
        }
    });

    private void View_HideWidgetRequested(object? sender, DashboardWidgetVisibilityIntent intent) => Mutate(profile =>
    {
        foreach (var layout in profile.Layouts.Where(layout => intent.AllBreakpoints || layout.Breakpoint == intent.Breakpoint))
            layout.Placements.Single(placement => placement.InstanceId == intent.InstanceId).Visible = false;
    });

    private void View_ConfigureWidgetRequested(object? sender, DashboardWidgetConfigurationIntent intent) => Mutate(profile =>
    {
        var widget = profile.Widgets.Single(value => value.InstanceId == intent.InstanceId);
        var definition = DashboardWidgetCatalog.ById()[widget.DefinitionId];
        widget.TitleOverride = intent.Title;
        widget.Accent = intent.Accent;
        SetAllowed(widget.Settings, definition, "timeRange", intent.TimeRange);
        SetAllowed(widget.Settings, definition, "metric", intent.Metric);
        SetAllowed(widget.Settings, definition, "sensor", intent.Sensor);
        SetAllowed(widget.Settings, definition, "volume", intent.Volume);
        SetAllowed(widget.Settings, definition, "sort", intent.Sort);
        SetAllowed(widget.Settings, definition, "count", intent.Count.ToString(System.Globalization.CultureInfo.InvariantCulture));
        SetAllowed(widget.Settings, definition, "filter", intent.Filter);
        SetAllowed(widget.Settings, definition, "showActions", intent.ShowActions.ToString());
    });

    private static void SetAllowed(
        IDictionary<string, string> settings,
        DashboardWidgetDefinition definition,
        string key,
        string? value)
    {
        if (!definition.SettingKeys.Contains(key, StringComparer.OrdinalIgnoreCase))
        {
            settings.Remove(key);
            return;
        }
        Set(settings, key, value);
    }

    private static void Set(IDictionary<string, string> settings, string key, string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) settings.Remove(key);
        else settings[key] = value;
    }

    private async void View_WidgetActionRequested(object? sender, DashboardWidgetActionIntent intent) =>
        await GuardedAsync($"Dashboard {intent.Action}", () => ExecuteWidgetActionAsync(sender, intent));

    private async Task ExecuteWidgetActionAsync(object? sender, DashboardWidgetActionIntent intent)
    {
        if (intent.Action == DashboardActionKind.Refresh)
        {
            await RefreshAsync();
            return;
        }
        if (intent.Action == DashboardActionKind.Navigate)
        {
            View_OpenWorkspaceRequested(sender, intent.InstanceId);
            return;
        }
        if (intent.Action == DashboardActionKind.OpenWindowsSettings)
        {
            View_OpenWorkspaceRequested(sender, intent.InstanceId);
            return;
        }
        _view.SetBusy(true, $"Reviewing {intent.Action.ToString().ToLowerInvariant()}…");
        var outcome = await _operations.RunCommittedAsync(ActionOperationKey, Key, $"dashboard {intent.Action}",
            token => _actionRouter.ExecuteAsync(intent, _settings.OfferSystemRestorePoint, _view, token));
        if (outcome.Cancelled)
        {
            _view.SetBusy(false, "Dashboard action cancelled.");
            return;
        }
        if (!outcome.Succeeded || outcome.Value is null)
        {
            _view.SetBusy(false, $"Dashboard action failed: {outcome.Error?.Message ?? "unknown error"}");
            return;
        }
        foreach (var line in outcome.Value.Log)
            _activity.Publish(ActivityEvent.Create(Key, line,
                outcome.Value.Succeeded ? ActivitySeverity.Info : ActivitySeverity.Warning));
        if (outcome.Value.Cancelled) _activity.Info(Key, "Dashboard action cancelled", outcome.Value.Summary);
        else if (outcome.Value.Succeeded) _activity.Info(Key, "Dashboard action completed", outcome.Value.Summary, persist: true);
        else _activity.Warning(Key, "Dashboard action incomplete", outcome.Value.Summary, persist: true);
        if (outcome.Value.Succeeded && _sourceOwnsAlerts &&
            _telemetry is IDashboardMonitorAlertClient monitorAlerts &&
            !string.IsNullOrWhiteSpace(intent.AlertId))
        {
            var synchronized = intent.Action switch
            {
                DashboardActionKind.AcknowledgeAlert => await monitorAlerts.AcknowledgeAlertAsync(
                    intent.AlertId, DateTimeOffset.UtcNow),
                DashboardActionKind.SnoozeAlert => await monitorAlerts.SnoozeAlertAsync(
                    intent.AlertId, DateTimeOffset.UtcNow.AddHours(1)),
                _ => true
            };
            if (!synchronized)
                _activity.Warning(Key, "Monitor alert state could not be synchronized",
                    "The local alert record was updated; the monitor will be refreshed on the next sample.");
        }
        if (intent.Action == DashboardActionKind.MaintenanceCleanup)
        {
            _settings.LastMaintenanceScanUtc = DateTime.UtcNow.ToString("O");
            _settingsPersistence.SaveNow(_settings);
        }
        _view.SetBusy(false, outcome.Value.Summary);
        if (outcome.Value.Succeeded) await SampleAsync(DashboardSampleKind.Slow, showBusy: false);
    }

    private void View_OpenWorkspaceRequested(object? sender, string instanceId)
    {
        var profile = Displayed();
        var instance = profile.Widgets.SingleOrDefault(widget => widget.InstanceId == instanceId);
        if (instance is null) return;
        var destination = DashboardWidgetCatalog.ById()[instance.DefinitionId].DestinationWorkspace;
        if (!destination.Equals("Home", StringComparison.OrdinalIgnoreCase)) _navigator.NavigateTo(destination);
    }

    private void View_DuplicateProfileRequested(object? sender, EventArgs e)
    {
        var source = Active();
        var id = Guid.NewGuid().ToString("N");
        var copy = DashboardProfileStore.CloneProfile(source, id, UniqueProfileName(source.Name), isBuiltIn: false);
        _document.Profiles.Add(copy);
        _document.ActiveProfileId = copy.Id;
        _profiles.Save(_document);
        BindActive();
    }

    private void View_ResetProfileRequested(object? sender, EventArgs e)
    {
        var current = Active();
        var defaults = DashboardProfileDefaults.Create();
        var template = defaults.Profiles.FirstOrDefault(profile => profile.Id.Equals(current.Id, StringComparison.OrdinalIgnoreCase))
            ?? defaults.Profiles[0];
        var reset = DashboardProfileStore.CloneProfile(template, current.Id, current.Name, current.IsBuiltIn);
        var index = _document.Profiles.IndexOf(current);
        _document.Profiles[index] = reset;
        _profiles.Save(_document);
        BindActive();
    }

    private void View_DeleteProfileRequested(object? sender, EventArgs e)
    {
        if (_document.Profiles.Count <= 1) return;
        if (Active().IsBuiltIn)
        {
            _view.SetBusy(false, "Built-in profiles stay available and can be reset; duplicate one to create a removable profile.");
            return;
        }
        _document.Profiles.Remove(Active());
        _document.ActiveProfileId = _document.Profiles[0].Id;
        _profiles.Save(_document);
        BindActive();
    }

    private void View_ImportProfileRequested(object? sender, string json)
    {
        try
        {
            var imported = _profiles.ImportProfile(_document, json);
            _document.ActiveProfileId = imported.Id;
            _profiles.Save(_document);
            BindActive();
        }
        catch (Exception exception) { _view.SetBusy(false, $"Could not import profile: {exception.Message}"); }
    }

    private void View_ExportProfileRequested(object? sender, EventArgs e) =>
        _view.CopyExport(_profiles.ExportProfile(Active()));

    private void View_RenameProfileRequested(object? sender, string name)
    {
        if (string.IsNullOrWhiteSpace(name) || name.Length > 80 || _document.Profiles.Any(profile =>
                !profile.Id.Equals(_document.ActiveProfileId, StringComparison.OrdinalIgnoreCase) &&
                profile.Name.Equals(name, StringComparison.OrdinalIgnoreCase)))
        {
            _view.SetBusy(false, "Profile names must be unique and no longer than 80 characters.");
            return;
        }
        Active().Name = name.Trim();
        _profiles.Save(_document);
        BindActive();
    }

    private void View_MoveProfileRequested(object? sender, int direction)
    {
        var index = _document.Profiles.FindIndex(profile => profile.Id.Equals(_document.ActiveProfileId, StringComparison.OrdinalIgnoreCase));
        var target = Math.Clamp(index + Math.Sign(direction), 0, _document.Profiles.Count - 1);
        if (target == index) return;
        var profile = _document.Profiles[index];
        _document.Profiles.RemoveAt(index);
        _document.Profiles.Insert(target, profile);
        _profiles.Save(_document);
        BindActive();
    }

    private void View_ProfileDensityRequested(object? sender, DashboardDensity density)
    {
        Active().Density = density;
        _profiles.Save(_document);
        BindActive();
    }

    private string UniqueProfileName(string baseName)
    {
        for (var suffix = 2; suffix < 100; suffix++)
        {
            var name = $"{baseName} ({suffix})";
            if (!_document.Profiles.Any(profile => profile.Name.Equals(name, StringComparison.OrdinalIgnoreCase))) return name;
        }
        return $"{baseName} copy";
    }
}
