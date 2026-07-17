using Sift.Models;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Automation;
using Microsoft.Web.WebView2.Core;

namespace Sift.WinUI.Views;

/// <summary>
/// Owns the local WebView2 host lifetime. DisposeStudio is the single teardown
/// boundary for the host handlers and pending bridge requests.
/// </summary>
public sealed partial class ScriptCenterWorkspaceView
{
    private IReadOnlyList<ScriptRuntime> _studioRuntimes = [];
    private bool _studioInitialized;
    private bool _studioReady;
    private bool _studioSuspended;
    private int _studioAnalysisSequence;

    public async Task InitializeStudioAsync()
    {
        if (_studioInitialized) return;
        _studioInitialized = true;
        var assetRoot = Path.Combine(AppContext.BaseDirectory, "WebAssets", "dist");
        if (!File.Exists(Path.Combine(assetRoot, "index.html")))
        {
            ShowStudioError("Local Script Studio assets are missing from this build.");
            return;
        }

        try
        {
            // Pass browser arguments through the environment options rather than mutating the
            // process-global WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS variable, which both raced other
            // callers and clobbered any flags they intended to set.
            var options = new CoreWebView2EnvironmentOptions { AdditionalBrowserArguments = "--disable-gpu" };
            var environment = await CoreWebView2Environment.CreateWithOptionsAsync(
                browserExecutableFolder: string.Empty, userDataFolder: string.Empty, options);
            await StudioWebView.EnsureCoreWebView2Async(environment);
            var core = StudioWebView.CoreWebView2;
            core.Settings.AreDevToolsEnabled = false;
            core.Settings.AreDefaultContextMenusEnabled = false;
            core.Settings.AreHostObjectsAllowed = false;
            core.Settings.IsWebMessageEnabled = true;
            core.SetVirtualHostNameToFolderMapping(StudioHost, assetRoot, CoreWebView2HostResourceAccessKind.DenyCors);
            core.WebMessageReceived += StudioWebView_WebMessageReceived;
            core.NavigationStarting += StudioWebView_NavigationStarting;
            core.NewWindowRequested += StudioWebView_NewWindowRequested;
            core.PermissionRequested += StudioWebView_PermissionRequested;
            core.DownloadStarting += StudioWebView_DownloadStarting;
            core.Navigate($"https://{StudioHost}/index.html");
            StudioStatusText.Text = "Loading Monaco and xterm.js from bundled local assets…";
        }
        catch (Exception exception)
        {
            // Allow a transient initialization failure to be retried on the next visit instead of
            // permanently wedging the guard at the top of this method.
            _studioInitialized = false;
            ShowStudioError($"Script Studio could not initialize: {exception.Message}");
        }
    }

    public void BindStudioRuntimes(IReadOnlyList<ScriptRuntime> runtimes)
    {
        _studioRuntimes = runtimes;
        var preferred = runtimes.Any(item => item.Language == ScriptLanguage.Python && item.Available)
            ? ScriptLanguage.Python
            : ScriptLanguage.PowerShell;
        StudioLanguageBox.SelectedItem = preferred;
        ApplyRuntimeFilter();
        AnalyzeButton.IsEnabled = _studioReady;
    }

    public async Task SuspendStudioAsync()
    {
        if (StudioWebView.CoreWebView2 is not { } core) return;
        try { _studioSuspended = await core.TrySuspendAsync(); }
        catch { /* The WebView may already be closing during application shutdown. */ }
    }

    public void ResumeStudio()
    {
        if (!_studioSuspended) return;
        try
        {
            StudioWebView.CoreWebView2?.Resume();
            _studioSuspended = false;
        }
        catch { /* Closing WebViews cannot be resumed. */ }
    }

    public void ShowStudioAnalysis(ScriptAnalysis analysis)
    {
        SetStudioBusy(false, analysis.Summary);
        var analysisSequence = ++_studioAnalysisSequence;
        AutomationProperties.SetItemStatus(StudioStatusText, $"{analysis.Summary} Analysis pass {analysisSequence}.");
        AutomationProperties.SetItemStatus(AnalyzeButton, $"Analysis pass {analysisSequence} completed.");
        PostStudio(new { type = "diagnostics.set", diagnostics = analysis.Diagnostics });
        PostStudio(new { type = "terminal.write", text = $"[{DateTime.Now:T}] {analysis.Summary}", error = analysis.Diagnostics.Any(item => item.Severity is ScriptDiagnosticSeverity.Error or ScriptDiagnosticSeverity.Blocked) });
        foreach (var diagnostic in analysis.Diagnostics.Take(100))
            PostStudio(new { type = "terminal.write", text = $"{diagnostic.Source} {diagnostic.Code} ({diagnostic.Line},{diagnostic.Column}): {diagnostic.Message}", error = diagnostic.Severity is ScriptDiagnosticSeverity.Error or ScriptDiagnosticSeverity.Blocked });
    }

    public void SetStudioBusy(bool busy, string status)
    {
        AnalyzeButton.IsEnabled = !busy && _studioReady;
        StudioLanguageBox.IsEnabled = !busy;
        StudioRuntimeBox.IsEnabled = !busy;
        StudioStatusText.Text = status;
        AutomationProperties.SetItemStatus(StudioStatusText, status);
    }

    public void ShowStudioError(string message)
    {
        StudioLoadingState.Visibility = Visibility.Visible;
        StudioLoadingText.Text = message;
        StudioStatusText.Text = message;
        AutomationProperties.SetItemStatus(StudioStatusText, message);
        AnalyzeButton.IsEnabled = false;
    }

    public void DisposeStudio()
    {
        _studioSuspended = false;
        _studioReady = false;
        _studioInitialized = false;
        CancelPendingDocumentRequests();
        if (StudioWebView.CoreWebView2 is { } core)
        {
            core.WebMessageReceived -= StudioWebView_WebMessageReceived;
            core.NavigationStarting -= StudioWebView_NavigationStarting;
            core.NewWindowRequested -= StudioWebView_NewWindowRequested;
            core.PermissionRequested -= StudioWebView_PermissionRequested;
            core.DownloadStarting -= StudioWebView_DownloadStarting;
        }
        try { StudioWebView.Close(); }
        catch { /* Already closed during tab leave or shutdown. */ }
    }

    private void ApplyRuntimeFilter()
    {
        var runtimes = _studioRuntimes.Where(item => item.Language == SelectedStudioLanguage).ToList();
        StudioRuntimeBox.ItemsSource = runtimes;
        StudioRuntimeBox.SelectedItem = runtimes.FirstOrDefault(item => item.Available && item.TrustedForAnalysis) ??
            runtimes.FirstOrDefault(item => item.Available) ?? runtimes.FirstOrDefault();
        var availability = StudioRuntimeBox.SelectedItem is ScriptRuntime runtime
            ? $"{runtime.DisplayName} · {runtime.StatusLabel} · {runtime.ExecutablePath} · {runtime.Source}."
            : $"No {SelectedStudioLanguage} runtime was discovered. Runtime downloads remain disabled.";
        StudioStatusText.Text = availability;
        if (_studioReady)
            PostStudio(new { type = "language.set", language = SelectedStudioLanguage, preserveText = false });
    }

    private static bool IsLocalStudioUri(string uri) => Uri.TryCreate(uri, UriKind.Absolute, out var parsed) &&
        string.Equals(parsed.Scheme, Uri.UriSchemeHttps, StringComparison.OrdinalIgnoreCase) &&
        string.Equals(parsed.Host, StudioHost, StringComparison.OrdinalIgnoreCase);

    private void StudioWebView_NavigationStarting(CoreWebView2 sender, CoreWebView2NavigationStartingEventArgs args)
    {
        if (!IsLocalStudioUri(args.Uri)) args.Cancel = true;
    }

    private static void StudioWebView_NewWindowRequested(CoreWebView2 sender, CoreWebView2NewWindowRequestedEventArgs args) =>
        args.Handled = true;

    private static void StudioWebView_PermissionRequested(CoreWebView2 sender, CoreWebView2PermissionRequestedEventArgs args) =>
        args.State = CoreWebView2PermissionState.Deny;

    private static void StudioWebView_DownloadStarting(CoreWebView2 sender, CoreWebView2DownloadStartingEventArgs args) =>
        args.Cancel = true;

    private void StudioLanguageBox_SelectionChanged(object sender, Microsoft.UI.Xaml.Controls.SelectionChangedEventArgs e)
    {
        if (StudioRuntimeBox is not null) ApplyRuntimeFilter();
    }
}
