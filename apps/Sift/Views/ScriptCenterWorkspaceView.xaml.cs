using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure.Dialogs;
using Sift.WinUI.Infrastructure.Interop;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Automation;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;

namespace Sift.WinUI.Views;

/// <summary>
/// Public view contract for Script Studio. Library, terminal, local WebView host, and
/// responsive layout behavior are implemented in responsibility-specific partials.
/// </summary>
public sealed partial class ScriptCenterWorkspaceView : UserControl
{
    private readonly IClipboardService _clipboard;

    public ScriptCenterWorkspaceView(IClipboardService clipboard)
    {
        ArgumentNullException.ThrowIfNull(clipboard);
        _clipboard = clipboard;
        InitializeComponent();
        AccessSections.ItemsSource = _sections;
        StudioLanguageBox.ItemsSource = Enum.GetValues<ScriptLanguage>();
        StudioWebView.DefaultBackgroundColor = Windows.UI.Color.FromArgb(0, 0, 0, 0);
        WorkspaceTabs.SelectedItem = LibraryTab;
    }

    public event EventHandler? RunRequested;
    public event EventHandler? StopRequested;
    public event EventHandler? StudioAnalyzeRequested;
    public event EventHandler? OpenWorkingDirectoryRequested;

    public ScriptRecipe? Selected => ReferenceEquals(WorkspaceTabs.SelectedItem, LibraryTab) ? _selectedRecipe : null;

    public ScriptLanguage SelectedStudioLanguage => StudioLanguageBox.SelectedItem is ScriptLanguage language
        ? language
        : ScriptLanguage.PowerShell;

    public ScriptRuntime? SelectedStudioRuntime => StudioRuntimeBox.SelectedItem as ScriptRuntime;

    public void FocusSearch()
    {
        if (ReferenceEquals(WorkspaceTabs.SelectedItem, StudioTab)) PostStudio(new { type = "editor.focus" });
        else SearchBox.Focus(FocusState.Programmatic);
    }

    public void SetRunning(bool running, string status)
    {
        BusyRing.IsActive = running;
        StopButton.IsEnabled = running;
        RunButton.IsEnabled = !running && Selected is not null;
        SearchBox.IsEnabled = !running;
        CategoryBox.IsEnabled = !running;
        ExpandAllButton.IsEnabled = !running && _sections.Count > 0;
        CollapseAllButton.IsEnabled = !running && _sections.Count > 0;
        CategoryScroller.IsHitTestVisible = !running;
        TerminalStatus.Text = status;
        StatusText.Text = status;
    }

    public async Task<bool> ConfirmStateChangingAsync(ScriptRecipe recipe, ScriptPreflight preflight)
    {
        if (!ScriptRecipeAccessPolicy.RequiresConfirmation(recipe))
            throw new ArgumentException("Read-only recipes run without confirmation.", nameof(recipe));

        var panel = new StackPanel { Spacing = 9, MaxWidth = 680 };
        panel.Children.Add(new TextBlock { Text = recipe.Description, TextWrapping = TextWrapping.Wrap });
        panel.Children.Add(BuildEvidence(preflight.Evidence));
        panel.Children.Add(Warning("This command changes Windows settings or system state."));
        if (recipe.SensitiveOutput)
            panel.Children.Add(Warning("Output may contain private paths, identifiers, tokens, or other secrets."));
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = $"Run {recipe.Title}?",
            Content = panel,
            PrimaryButtonText = "Run command",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    private static TextBlock Warning(string text) => new()
    {
        Text = text,
        Foreground = (Brush)Application.Current.Resources["SiftWarningBrush"],
        TextWrapping = TextWrapping.Wrap
    };

    private static Border BuildEvidence(IReadOnlyList<string> evidence)
    {
        var lines = new StackPanel { Spacing = 5 };
        foreach (var line in evidence)
            lines.Children.Add(new TextBlock
            {
                Text = line,
                FontFamily = new FontFamily("Cascadia Mono"),
                FontSize = 12,
                TextWrapping = TextWrapping.Wrap
            });
        var scroll = new ScrollViewer
        {
            Content = lines,
            MinHeight = 190,
            MaxHeight = 260,
            VerticalScrollBarVisibility = ScrollBarVisibility.Auto
        };
        AutomationProperties.SetName(scroll, "Command details");
        return new Border
        {
            Background = (Brush)Application.Current.Resources["SiftCardBrush"],
            BorderBrush = (Brush)Application.Current.Resources["SiftLineBrush"],
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(8),
            Padding = new Thickness(12, 10, 12, 10),
            Child = scroll
        };
    }

    private void AnalyzeButton_Click(object sender, RoutedEventArgs e) => StudioAnalyzeRequested?.Invoke(this, EventArgs.Empty);
    private void RunButton_Click(object sender, RoutedEventArgs e) => RunRequested?.Invoke(this, EventArgs.Empty);
    private void StopButton_Click(object sender, RoutedEventArgs e) => StopRequested?.Invoke(this, EventArgs.Empty);
}
