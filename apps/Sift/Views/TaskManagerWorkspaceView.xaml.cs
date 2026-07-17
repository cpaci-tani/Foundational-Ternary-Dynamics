using System.Collections.ObjectModel;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure.Dialogs;
using Sift.WinUI.Models;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;

namespace Sift.WinUI.Views;

public sealed partial class TaskManagerWorkspaceView : UserControl
{
    private readonly ObservableCollection<TaskProcessRow> _processes = [];
    private readonly ObservableCollection<ServiceInfo> _services = [];
    private readonly ObservableCollection<ScheduledTaskInfo> _tasks = [];
    private TaskManagerInventory? _inventory;
    private bool _busy;
    private bool _canEndProcess;
    private bool _canRestartProcess;
    private bool _canStartService;
    private bool _canRestartService;
    private bool _canEnableTask;
    private bool _canDisableTask;

    public TaskManagerWorkspaceView()
    {
        InitializeComponent();
        ProcessTable.ItemsSource = _processes;
        ServiceTable.ItemsSource = _services;
        TaskTable.ItemsSource = _tasks;
    }

    public event EventHandler? RefreshRequested;
    public event EventHandler? SelectionChanged;
    public event EventHandler? EndProcessRequested;
    public event EventHandler? RestartProcessRequested;
    public event EventHandler? StartServiceRequested;
    public event EventHandler? RestartServiceRequested;
    public event EventHandler? EnableTaskRequested;
    public event EventHandler? DisableTaskRequested;
    public TaskProcessRow? SelectedProcess => ProcessTable.SelectedItem as TaskProcessRow;
    public ServiceInfo? SelectedService => ServiceTable.SelectedItem as ServiceInfo;
    public ScheduledTaskInfo? SelectedTask => TaskTable.SelectedItem as ScheduledTaskInfo;

    public void Bind(TaskManagerInventory inventory)
    {
        _inventory = inventory;
        CpuText.Text = $"{inventory.System.CpuPercent:0}%";
        MemoryText.Text = $"{inventory.System.MemoryPercent:0}%";
        ServiceText.Text = $"{inventory.Services.Count(x => x.Status == "Running"):N0} / {inventory.Services.Count:N0}";
        TaskText.Text = inventory.Tasks.Count.ToString("N0");
        ApplyFilter();
        UpdateSelectionDisplay();
    }

    public void SetBusy(bool busy, string status)
    {
        _busy = busy;
        BusyRing.IsActive = busy;
        RefreshButton.IsEnabled = !busy;
        StatusText.Text = status;
        UpdateActionButtons();
    }

    public void SetStatus(string status) => StatusText.Text = status;

    public void SetProcessActionAvailability(bool canEnd, string endReason, bool canRestart, string restartReason)
    {
        _canEndProcess = canEnd;
        _canRestartProcess = canRestart;
        ToolTipService.SetToolTip(EndProcessButton, endReason);
        ToolTipService.SetToolTip(RestartProcessButton, restartReason);
        UpdateActionButtons();
    }

    public void SetServiceActionAvailability(bool canStart, string startReason, bool canRestart, string restartReason)
    {
        _canStartService = canStart;
        _canRestartService = canRestart;
        ToolTipService.SetToolTip(StartServiceButton, startReason);
        ToolTipService.SetToolTip(RestartServiceButton, restartReason);
        UpdateActionButtons();
    }

    public void SetTaskActionAvailability(bool canEnable, string enableReason, bool canDisable, string disableReason)
    {
        _canEnableTask = canEnable;
        _canDisableTask = canDisable;
        ToolTipService.SetToolTip(EnableTaskButton, enableReason);
        ToolTipService.SetToolTip(DisableTaskButton, disableReason);
        UpdateActionButtons();
    }

    public Task<bool> ConfirmScheduledTaskActionAsync(ScheduledTaskActionPreflight preflight, bool requiresElevation)
    {
        var enable = preflight.Change == ScheduledTaskChange.Enable;
        return ConfirmActionAsync(
            enable ? "Enable this scheduled task?" : "Disable this scheduled task?",
            enable ? "Enable task" : "Disable task",
            "Leave task unchanged",
            $"Review the selected scheduled task. Its identity, state, and definition must still match when the action begins.{(requiresElevation ? " Windows will request administrator permission." : string.Empty)}{Environment.NewLine}{Environment.NewLine}{preflight.Evidence}",
            $"Display name: {preflight.DisplayName}{Environment.NewLine}Requested change: {preflight.Change}{Environment.NewLine}Current state: {preflight.ExpectedState}{Environment.NewLine}Target enabled: {!preflight.ExpectedEnabled}{Environment.NewLine}Expires UTC: {preflight.ExpiresUtc:O}",
            dangerous: !enable);
    }

    public Task<bool> ConfirmProcessActionAsync(TaskProcessRow process, bool restart)
    {
        var action = restart ? "Restart app" : "End task";
        var started = process.StartTimeUtcTicks > 0
            ? new DateTime(process.StartTimeUtcTicks, DateTimeKind.Utc).ToLocalTime().ToString("G")
            : "Unavailable";
        var warning = restart
            ? "Sift will end this process tree and reopen the same executable. Command-line arguments and unsaved state are not restored."
            : "Sift will end this process tree. Unsaved work can be lost, and Sift cannot restore it.";
        return ConfirmActionAsync(
            restart ? "Restart this app?" : "End this task?",
            action,
            "Keep process running",
            $"Review the selected process. Its PID, start time, session, name, and executable must still match when the action begins.{Environment.NewLine}{Environment.NewLine}{warning}",
            $"Name: {process.Name}{Environment.NewLine}PID: {process.Id}{Environment.NewLine}Started: {started}{Environment.NewLine}Session: {process.SessionId}{Environment.NewLine}Path: {process.ExecutablePath}",
            dangerous: !restart);
    }

    public Task<bool> ConfirmServiceActionAsync(ServiceInfo service, string action, bool requiresElevation) =>
        ConfirmActionAsync(
            $"{action} this service?",
            $"{action} service",
            "Leave service unchanged",
            $"Review the selected third-party service. Its registration, executable, start type, and {service.Status} state must still match when the action begins.{(requiresElevation ? " Windows will request administrator permission." : string.Empty)}",
            $"Display name: {service.DisplayName}{Environment.NewLine}Service: {service.Name}{Environment.NewLine}State: {service.Status}{Environment.NewLine}Start type: {service.StartType}",
            dangerous: action.Equals("Restart", StringComparison.Ordinal));

    public void FocusSearch() => SearchBox.Focus(FocusState.Programmatic);

    private void ApplyFilter()
    {
        if (_inventory is null) return;
        var query = SearchBox.Text.Trim();
        Replace(_processes, _inventory.System.Processes
            .Where(x => Matches(query, x.Name, x.Id.ToString(), x.Status, x.Architecture))
            .Select(TaskProcessRow.From));
        Replace(_services, _inventory.Services.Where(x => Matches(query, x.Name, x.DisplayName, x.Status, x.StartType, x.GroupKey)));
        Replace(_tasks, _inventory.Tasks.Where(x => Matches(query, x.TaskName, x.TaskPath, x.State, x.Author, x.GroupKey)));
        var count = InventoryTabs.SelectedIndex switch { 1 => _services.Count, 2 => _tasks.Count, _ => _processes.Count };
        StatusText.Text = $"{count:N0} visible in the active inventory · updated {DateTime.Now:T}";
        UpdateSelectionDisplay();
    }

    private async Task<bool> ConfirmActionAsync(string title, string primaryText, string closeText,
        string explanation, string details, bool dangerous)
    {
        var detailText = new TextBlock
        {
            Text = details,
            TextWrapping = TextWrapping.Wrap,
            FontFamily = new FontFamily("Consolas"),
            FontSize = 12,
            LineHeight = 19,
            Foreground = (Brush)Application.Current.Resources["SiftMutedBrush"]
        };
        var content = new StackPanel { Spacing = 10, MaxWidth = 650 };
        content.Children.Add(new TextBlock { Text = explanation, TextWrapping = TextWrapping.Wrap });
        content.Children.Add(new Border
        {
            Background = (Brush)Application.Current.Resources["SiftPanelBrush"],
            BorderBrush = (Brush)Application.Current.Resources["SiftLineBrush"],
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(7),
            Padding = new Thickness(12, 10, 12, 10),
            Child = detailText
        });
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = title,
            Content = content,
            PrimaryButtonText = primaryText,
            CloseButtonText = closeText
        };
        ConfirmationDialogStyle.Apply(dialog);
        if (dangerous) dialog.PrimaryButtonStyle = (Style)Application.Current.Resources["DangerButtonStyle"];
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    private void UpdateSelectionDisplay()
    {
        SelectedProcessText.Text = SelectedProcess is { } process
            ? $"{process.Name} · PID {process.Id} · {process.ExecutablePath}"
            : "Select one process to review available actions";
        SelectedServiceText.Text = SelectedService is { } service
            ? $"{service.DisplayName} · {service.Status} · {service.GroupKey}"
            : "Select one manageable third-party service";
        SelectedTaskText.Text = SelectedTask is { } task
            ? $"{task.TaskName} · {task.TaskPath} · {task.State} · {(task.ActionId.HasValue ? "Supported" : "View only")}"
            : "Select a scheduled task";
        UpdateActionButtons();
    }

    private void UpdateActionButtons()
    {
        EndProcessButton.IsEnabled = !_busy && SelectedProcess is not null && _canEndProcess;
        RestartProcessButton.IsEnabled = !_busy && SelectedProcess is not null && _canRestartProcess;
        StartServiceButton.IsEnabled = !_busy && SelectedService is not null && _canStartService;
        RestartServiceButton.IsEnabled = !_busy && SelectedService is not null && _canRestartService;
        EnableTaskButton.IsEnabled = !_busy && SelectedTask is not null && _canEnableTask;
        DisableTaskButton.IsEnabled = !_busy && SelectedTask is not null && _canDisableTask;
    }

    private static bool Matches(string query, params string?[] values) =>
        string.IsNullOrWhiteSpace(query) || values.Any(value => value?.Contains(query, StringComparison.OrdinalIgnoreCase) == true);

    private static void Replace<T>(ObservableCollection<T> destination, IEnumerable<T> source)
    {
        destination.Clear();
        foreach (var value in source) destination.Add(value);
    }

    private void SearchBox_TextChanged(object sender, TextChangedEventArgs e) => ApplyFilter();
    private void InventoryTabs_SelectionChanged(object sender, SelectionChangedEventArgs e) => ApplyFilter();
    private void InventorySelection_Changed(object sender, SelectionChangedEventArgs e)
    {
        UpdateSelectionDisplay();
        SelectionChanged?.Invoke(this, EventArgs.Empty);
    }
    private void RefreshButton_Click(object sender, RoutedEventArgs e) => RefreshRequested?.Invoke(this, EventArgs.Empty);
    private void EndProcessButton_Click(object sender, RoutedEventArgs e) => EndProcessRequested?.Invoke(this, EventArgs.Empty);
    private void RestartProcessButton_Click(object sender, RoutedEventArgs e) => RestartProcessRequested?.Invoke(this, EventArgs.Empty);
    private void StartServiceButton_Click(object sender, RoutedEventArgs e) => StartServiceRequested?.Invoke(this, EventArgs.Empty);
    private void RestartServiceButton_Click(object sender, RoutedEventArgs e) => RestartServiceRequested?.Invoke(this, EventArgs.Empty);
    private void EnableTaskButton_Click(object sender, RoutedEventArgs e) => EnableTaskRequested?.Invoke(this, EventArgs.Empty);
    private void DisableTaskButton_Click(object sender, RoutedEventArgs e) => DisableTaskRequested?.Invoke(this, EventArgs.Empty);
}
