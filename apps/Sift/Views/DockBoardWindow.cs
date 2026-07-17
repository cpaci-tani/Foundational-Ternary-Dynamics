using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Controls;
using Sift.Infrastructure.Icons;
using WinRT.Interop;

namespace Sift.WinUI.Views;

public sealed class DockBoardWindow : Window
{
    private readonly IDockSession _session;
    private readonly string _siteId;
    private readonly DockHostControl _dockHost = new();
    private readonly ComboBox _redockZoneBox = new() { Width = 120 };
    private bool _closing;

    public DockBoardWindow(
        IDockSession session,
        IDockBoardPresenter presenter,
        FloatingDockSite site,
        string caption = "Floating dock")
    {
        _session = session;
        _siteId = site.Id;
        Title = $"Sift · {caption}";

        _redockZoneBox.ItemsSource = new[]
        {
            DockDropZone.Tab,
            DockDropZone.Left,
            DockDropZone.Right,
            DockDropZone.Top,
            DockDropZone.Bottom
        };
        _redockZoneBox.SelectedItem = DockDropZone.Tab;
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(_redockZoneBox, "Dock drop zone");

        var redockButton = new SiftIconButton
        {
            Icon = SiftIconKind.PopOut,
            Label = "Dock back",
            Style = (Style)Application.Current.Resources["SecondaryButtonStyle"]
        };
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(redockButton, "Dock floating panel back");
        redockButton.Click += (_, _) =>
        {
            var zone = _redockZoneBox.SelectedItem is DockDropZone selected ? selected : DockDropZone.Tab;
            RedockRequested?.Invoke(this, zone);
        };

        var header = new Grid { ColumnSpacing = 8, Margin = new Thickness(0, 0, 0, 10) };
        header.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        header.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
        header.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
        var title = new TextBlock
        {
            Text = caption,
            Style = (Style)Application.Current.Resources["TypeWorkspaceTitleStyle"]
        };
        Grid.SetColumn(redockButton, 1);
        Grid.SetColumn(_redockZoneBox, 2);
        header.Children.Add(title);
        header.Children.Add(redockButton);
        header.Children.Add(_redockZoneBox);

        var root = new Grid
        {
            Padding = new Thickness(14),
            Background = (Brush)Application.Current.Resources["SiftBackgroundBrush"]
        };
        root.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        root.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) });
        Grid.SetRow(_dockHost, 1);
        root.Children.Add(header);
        root.Children.Add(_dockHost);
        Content = root;

        _dockHost.BindFloating(session, presenter, site, caption);
        TryPosition(site);
        Closed += DockBoardWindow_Closed;
        SizeChanged += DockBoardWindow_SizeChanged;
    }

    public string SiteId => _siteId;
    public DockHostControl Host => _dockHost;
    public event EventHandler? ClosedByUser;
    public event EventHandler<DockDropZone>? RedockRequested;

    public void ApplyData(object? data) => _dockHost.ApplyData(data);
    public void Refresh() => _dockHost.RefreshFromSession();

    private void TryPosition(FloatingDockSite site)
    {
        try
        {
            var handle = WindowNative.GetWindowHandle(this);
            var id = Microsoft.UI.Win32Interop.GetWindowIdFromWindow(handle);
            var appWindow = AppWindow.GetFromWindowId(id);
            appWindow.Resize(new Windows.Graphics.SizeInt32(
                Math.Max(320, (int)site.Width),
                Math.Max(240, (int)site.Height)));
            appWindow.Move(new Windows.Graphics.PointInt32((int)site.X, (int)site.Y));
        }
        catch (Exception exception)
        {
            System.Diagnostics.Debug.WriteLine(exception);
        }
    }

    private void DockBoardWindow_SizeChanged(object sender, WindowSizeChangedEventArgs args)
    {
        if (_closing) return;
        try
        {
            var handle = WindowNative.GetWindowHandle(this);
            var id = Microsoft.UI.Win32Interop.GetWindowIdFromWindow(handle);
            var appWindow = AppWindow.GetFromWindowId(id);
            var pos = appWindow.Position;
            var size = appWindow.Size;
            _session.UpdateFloatingBounds(_siteId, pos.X, pos.Y, size.Width, size.Height);
        }
        catch (Exception exception)
        {
            System.Diagnostics.Debug.WriteLine(exception);
        }
    }

    private void DockBoardWindow_Closed(object sender, WindowEventArgs args)
    {
        if (_closing) return;
        _closing = true;
        ClosedByUser?.Invoke(this, EventArgs.Empty);
    }

    public void MarkProgrammaticClose() => _closing = true;
}
