using System.IO;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Composition;
using Sift.Infrastructure.Icons;
using Sift.WinUI.Infrastructure.Icons;
using Sift.WinUI.Infrastructure.Windowing;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Windows.Graphics;
using Windows.System;

namespace Sift.WinUI;

public sealed partial class MainWindow : Window
{
    private readonly WinUiShellServices _shell;
    private readonly IWorkspaceRegistry _workspaces;
    private readonly AppSettings _settings;
    private readonly WorkspaceNavigator _navigator = new();
    private bool _windowConfigured;
    private bool _navigationInitialized;
    private bool _suppressNavigation;
    private bool _consoleVisible;
    private bool _narrowLayout;
    private bool _forceConsoleOnNarrow;
    private int _navigationGeneration;
    private string _currentWorkspace = "Home";
    private WindowMinimumSize? _minimumSize;

    public MainWindow(WinUiShellServices shell, IWorkspaceRegistryFactory workspaceFactory)
    {
        _shell = shell;
        _settings = _shell.Settings;
        _consoleVisible = _settings.ConsoleVisible;
        InitializeComponent();
        Title = "Sift";
        ExtendsContentIntoTitleBar = true;
        SetTitleBar(TitleBarDragRegion);
        _workspaces = workspaceFactory.Create(
            _navigator, () => WinRT.Interop.WindowNative.GetWindowHandle(this));
        try
        {
            ValidateNavigationRegistry();
        }
        catch
        {
            _workspaces.Dispose();
            throw;
        }
        ConsolePanel.Connect(_shell.Activity, _shell.Clipboard);
        ConsolePanel.HideRequested += ConsolePanel_HideRequested;
        _navigator.NavigationRequested += Navigator_NavigationRequested;
        Activated += MainWindow_Activated;
        Closed += MainWindow_Closed;
        _workspaces.ShellSettingsChanged += Settings_ShellSettingsChanged;
        ApplyUiScale();
        ApplyConsoleVisibility();
        ElevationText.Text = ElevationHelper.IsElevated() ? "Administrator" : "Standard user";
        var version = typeof(MainWindow).Assembly.GetName().Version?.ToString(3) ?? "unknown";
        Publish("App", $"Sift WinUI 3 v{version} started as {(ElevationHelper.IsElevated() ? "administrator" : "standard user")}");
    }

    private void ValidateNavigationRegistry()
    {
        var routeKeys = ShellNavigation.MenuItems.OfType<NavigationViewItem>()
            .Select(item => item.Tag?.ToString())
            .Where(key => !string.IsNullOrWhiteSpace(key))
            .Cast<string>()
            .Append("Settings")
            .ToHashSet(StringComparer.OrdinalIgnoreCase);
        var routesWithoutModules = routeKeys.Where(key => !_workspaces.Contains(key)).ToList();
        var modulesWithoutRoutes = _workspaces.Keys.Where(key => !routeKeys.Contains(key)).ToList();
        if (routesWithoutModules.Count == 0 && modulesWithoutRoutes.Count == 0) return;

        throw new InvalidOperationException(
            $"Workspace registration does not match shell routes. " +
            $"Missing modules: {string.Join(", ", routesWithoutModules)}. " +
            $"Missing routes: {string.Join(", ", modulesWithoutRoutes)}.");
    }

    private void MainWindow_Activated(object sender, WindowActivatedEventArgs args)
    {
        if (_windowConfigured) return;
        _windowConfigured = true;
        var hwnd = WinRT.Interop.WindowNative.GetWindowHandle(this);
        var id = Microsoft.UI.Win32Interop.GetWindowIdFromWindow(hwnd);
        var appWindow = AppWindow.GetFromWindowId(id);
        _minimumSize = WindowMinimumSize.Attach(hwnd, 1100, 720);
        appWindow.Resize(new SizeInt32(1500, 920));
        appWindow.SetIcon(Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "Sift.ico")));
        if (AppWindowTitleBar.IsCustomizationSupported())
        {
            appWindow.TitleBar.ButtonBackgroundColor = Windows.UI.Color.FromArgb(0, 0, 0, 0);
            appWindow.TitleBar.ButtonInactiveBackgroundColor = Windows.UI.Color.FromArgb(0, 0, 0, 0);
            appWindow.TitleBar.ButtonForegroundColor = Windows.UI.Color.FromArgb(255, 242, 238, 232);
        }
    }

    private async void RootGrid_Loaded(object sender, RoutedEventArgs e)
    {
        SiftNavIconFactory.ApplyNavigationIcons(ShellNavigation);
        if (_navigationInitialized) return;
        _navigationInitialized = true;
        var initialKey = !string.IsNullOrWhiteSpace(_settings.LastWorkspace) && _workspaces.Contains(_settings.LastWorkspace)
            ? _settings.LastWorkspace
            : "Home";
        if (ShellNavigation.SelectedItem is null)
        {
            _suppressNavigation = true;
            ShellNavigation.SelectedItem = initialKey.Equals("Settings", StringComparison.OrdinalIgnoreCase)
                ? ShellNavigation.SettingsItem
                : ShellNavigation.MenuItems.OfType<NavigationViewItem>()
                    .FirstOrDefault(item => string.Equals(item.Tag?.ToString(), initialKey, StringComparison.OrdinalIgnoreCase));
            _suppressNavigation = false;
        }
        await OpenWorkspaceAsync(initialKey);
    }

    private async void ShellNavigation_SelectionChanged(NavigationView sender, NavigationViewSelectionChangedEventArgs args)
    {
        if (_suppressNavigation) return;
        var key = args.IsSettingsSelected ? "Settings" : args.SelectedItemContainer?.Tag as string;
        if (key is null) return;
        await OpenWorkspaceAsync(key);
    }

    private async Task OpenWorkspaceAsync(string key)
    {
        if (!_workspaces.TryGet(key, out var module)) return;
        // Selection changes arrive on the UI thread but each await yields to the message loop, so a
        // second navigation can interleave while ActivateAsync is still awaiting. Stamp a generation
        // so a superseded activation abandons itself instead of leaving two modules sampling at once.
        var generation = ++_navigationGeneration;
        if (_workspaces.TryGet(_currentWorkspace, out var previous) && !ReferenceEquals(previous, module)) previous.Deactivate();
        _currentWorkspace = key;
        _settings.LastWorkspace = key;
        _shell.SettingsPersistence.Schedule(_settings);
        WorkspaceHost.Content = module.View;
        Publish("Navigation", $"Opening {module.Title} workspace");
        await module.ActivateAsync();
        if (generation != _navigationGeneration &&
            !string.Equals(_currentWorkspace, key, StringComparison.OrdinalIgnoreCase))
            module.Deactivate();
    }

    private void Navigator_NavigationRequested(string workspaceKey)
    {
        var destination = workspaceKey.Equals("Settings", StringComparison.OrdinalIgnoreCase)
            ? ShellNavigation.SettingsItem
            : ShellNavigation.MenuItems.OfType<NavigationViewItem>()
                .FirstOrDefault(item => string.Equals(item.Tag?.ToString(), workspaceKey,
                    StringComparison.OrdinalIgnoreCase));
        if (destination is not null) ShellNavigation.SelectedItem = destination;
    }

    private void WorkspaceAccelerator_Invoked(KeyboardAccelerator sender, KeyboardAcceleratorInvokedEventArgs args)
    {
        var index = sender.Key switch
        {
            VirtualKey.Number1 => 0,
            VirtualKey.Number2 => 1,
            VirtualKey.Number3 => 2,
            VirtualKey.Number4 => 3,
            VirtualKey.Number5 => 4,
            VirtualKey.Number6 => 5,
            VirtualKey.Number7 => 6,
            VirtualKey.Number8 => 7,
            VirtualKey.Number9 => 8,
            VirtualKey.Number0 => ShellNavigation.MenuItems.Count,
            _ => -1
        };
        if (index < 0 || index > ShellNavigation.MenuItems.Count) return;
        ShellNavigation.SelectedItem = index == ShellNavigation.MenuItems.Count
            ? ShellNavigation.SettingsItem
            : ShellNavigation.MenuItems[index];
        args.Handled = true;
    }

    private void SearchAccelerator_Invoked(KeyboardAccelerator sender, KeyboardAcceleratorInvokedEventArgs args)
    {
        if (!_workspaces.TryGet(_currentWorkspace, out var module)) return;
        module.FocusPrimarySearch();
        args.Handled = true;
    }

    private void SystemInformationAccelerator_Invoked(KeyboardAccelerator sender, KeyboardAcceleratorInvokedEventArgs args)
    {
        ShellNavigation.SelectedItem = ShellNavigation.MenuItems.OfType<NavigationViewItem>()
            .FirstOrDefault(item => string.Equals(item.Tag?.ToString(), "SystemInfo", StringComparison.OrdinalIgnoreCase));
        args.Handled = true;
    }

    private void Publish(string category, string summary) => _shell.Activity.Info(category, summary);

    private void ConsoleToggleButton_Click(object sender, RoutedEventArgs e)
    {
        var currentlyShown = _consoleVisible && (!_narrowLayout || _forceConsoleOnNarrow);
        _consoleVisible = !currentlyShown;
        _forceConsoleOnNarrow = _narrowLayout && _consoleVisible;
        _settings.ConsoleVisible = _consoleVisible;
        _shell.SettingsPersistence.Schedule(_settings);
        ApplyConsoleVisibility();
    }

    private void ConsolePanel_HideRequested(object? sender, EventArgs e)
    {
        _consoleVisible = false;
        _forceConsoleOnNarrow = false;
        _settings.ConsoleVisible = false;
        _shell.SettingsPersistence.Schedule(_settings);
        ApplyConsoleVisibility();
    }

    private void RootGrid_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        ApplyUiScale();
        ApplyConsoleVisibility();
    }

    private void ApplyConsoleVisibility()
    {
        var show = _consoleVisible && (!_narrowLayout || _forceConsoleOnNarrow);
        var width = Math.Clamp(_settings.ConsoleWidth, 300, 520);
        ConsoleColumn.Width = new GridLength(show && !_narrowLayout ? width : 0);
        Grid.SetColumn(ConsolePanel, _narrowLayout ? 0 : 1);
        Grid.SetColumnSpan(ConsolePanel, _narrowLayout ? 2 : 1);
        ConsolePanel.HorizontalAlignment = _narrowLayout ? HorizontalAlignment.Right : HorizontalAlignment.Stretch;
        var factor = UiScalePolicy.ResolveFactor(_settings.UiScale);
        var logicalWidth = factor <= 0 ? RootGrid.ActualWidth : RootGrid.ActualWidth / factor;
        ConsolePanel.Width = _narrowLayout ? Math.Min(width, Math.Max(300, logicalWidth - 56)) : double.NaN;
        ConsolePanel.Visibility = show ? Visibility.Visible : Visibility.Collapsed;
        ConsoleToggleButton.Label = show ? "Hide activity" : "Show activity";
        ConsoleToggleButton.Icon = show ? SiftIconKind.Hide : SiftIconKind.Show;
    }

    private void Settings_ShellSettingsChanged(object? sender, EventArgs e)
    {
        _consoleVisible = _settings.ConsoleVisible;
        ApplyUiScale();
        _forceConsoleOnNarrow = _narrowLayout && _consoleVisible;
        ApplyConsoleVisibility();
    }

    private void ApplyUiScale()
    {
        if (ShellContentGrid is null) return;
        var factor = UiScalePolicy.ResolveFactor(_settings.UiScale);
        ShellContentGrid.RenderTransformOrigin = new Windows.Foundation.Point(0, 0);
        var isDefault = Math.Abs(factor - 1.0) < 0.001;
        ShellContentGrid.RenderTransform = isDefault
            ? null
            : new ScaleTransform { ScaleX = factor, ScaleY = factor };

        if (isDefault || RootGrid.ActualWidth <= 0 || RootGrid.RowDefinitions[1].ActualHeight <= 0)
        {
            ShellContentGrid.Width = double.NaN;
            ShellContentGrid.Height = double.NaN;
            ShellContentGrid.HorizontalAlignment = HorizontalAlignment.Stretch;
            ShellContentGrid.VerticalAlignment = VerticalAlignment.Stretch;
        }
        else
        {
            // A render transform alone does not participate in measure or scrolling. Giving the
            // shell the inverse logical size lets it measure responsively and makes the transformed
            // result exactly fill the available content row without clipping or unused edges.
            ShellContentGrid.Width = RootGrid.ActualWidth / factor;
            ShellContentGrid.Height = RootGrid.RowDefinitions[1].ActualHeight / factor;
            ShellContentGrid.HorizontalAlignment = HorizontalAlignment.Left;
            ShellContentGrid.VerticalAlignment = VerticalAlignment.Top;
        }

        var logicalWidth = RootGrid.ActualWidth <= 0 ? 0 : RootGrid.ActualWidth / factor;
        _narrowLayout = logicalWidth > 0 && logicalWidth < 1180;
        if (!_narrowLayout) _forceConsoleOnNarrow = false;
        WorkspaceHost.Margin = _narrowLayout
            ? new Thickness(18, 20, 18, 20)
            : new Thickness(34, 26, 34, 26);
    }

    private void MainWindow_Closed(object sender, WindowEventArgs args)
    {
        Activated -= MainWindow_Activated;
        Closed -= MainWindow_Closed;
        ConsolePanel.HideRequested -= ConsolePanel_HideRequested;
        _navigator.NavigationRequested -= Navigator_NavigationRequested;
        _workspaces.ShellSettingsChanged -= Settings_ShellSettingsChanged;
        _minimumSize?.Dispose();
        _minimumSize = null;
        ConsolePanel.Dispose();
        _workspaces.Dispose();
    }
}
