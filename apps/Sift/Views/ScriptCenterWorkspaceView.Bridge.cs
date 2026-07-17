using System.Text.Json;
using System.Text.Json.Serialization;
using Sift.Models;
using Microsoft.UI.Xaml;
using Microsoft.Web.WebView2.Core;

namespace Sift.WinUI.Views;

/// <summary>
/// Owns the finite JSON protocol between the local Studio page and the native view.
/// It accepts no generic native invocation and bounds all document and clipboard text.
/// </summary>
public sealed partial class ScriptCenterWorkspaceView
{
    private const int MaximumBridgeTextCharacters = 1_000_000;
    private const string StudioHost = "sift.local";
    private static readonly JsonSerializerOptions BridgeJson = new(JsonSerializerDefaults.Web)
    {
        Converters = { new JsonStringEnumConverter() }
    };

    private readonly Dictionary<string, TaskCompletionSource<string>> _documentRequests = new(StringComparer.Ordinal);

    public async Task<ScriptDocument> GetStudioDocumentAsync(CancellationToken cancellationToken)
    {
        if (!_studioReady) throw new InvalidOperationException("The local editor is not ready.");
        var requestId = Guid.NewGuid().ToString("N");
        var completion = new TaskCompletionSource<string>(TaskCreationOptions.RunContinuationsAsynchronously);
        _documentRequests.Add(requestId, completion);
        PostStudio(new { type = "document.request", requestId });
        try
        {
            var text = await completion.Task.WaitAsync(TimeSpan.FromSeconds(5), cancellationToken);
            var language = SelectedStudioLanguage;
            return new ScriptDocument(language, SelectedStudioRuntime?.Id ?? string.Empty, FileName(language), text);
        }
        finally
        {
            _documentRequests.Remove(requestId);
        }
    }

    private void PostStudio(object value)
    {
        if (!_studioReady || StudioWebView.CoreWebView2 is null) return;
        StudioWebView.CoreWebView2.PostWebMessageAsJson(JsonSerializer.Serialize(value, BridgeJson));
    }

    private void StudioWebView_WebMessageReceived(CoreWebView2 sender, CoreWebView2WebMessageReceivedEventArgs args)
    {
        if (!Uri.TryCreate(args.Source, UriKind.Absolute, out var source) ||
            !string.Equals(source.Scheme, Uri.UriSchemeHttps, StringComparison.OrdinalIgnoreCase) ||
            !string.Equals(source.Host, StudioHost, StringComparison.OrdinalIgnoreCase)) return;
        var json = args.WebMessageAsJson;
        if (json.Length > MaximumBridgeTextCharacters + 16_384) return;
        try
        {
            using var document = JsonDocument.Parse(json);
            var root = document.RootElement;
            if (!root.TryGetProperty("type", out var typeValue)) return;
            switch (typeValue.GetString())
            {
                case "ready":
                    _studioReady = true;
                    StudioLoadingState.Visibility = Visibility.Collapsed;
                    AnalyzeButton.IsEnabled = true;
                    ApplyRuntimeFilter();
                    break;
                case "analysis.request":
                    StudioAnalyzeRequested?.Invoke(this, EventArgs.Empty);
                    break;
                case "document.response":
                    CompleteDocumentRequest(root);
                    break;
                case "clipboard.copy":
                    CopyFromBridge(root);
                    break;
                case "explorer.open-working-directory":
                    OpenWorkingDirectoryRequested?.Invoke(this, EventArgs.Empty);
                    break;
            }
        }
        catch (JsonException)
        {
            StudioStatusText.Text = "A malformed local editor message was rejected.";
        }
    }

    private void CompleteDocumentRequest(JsonElement root)
    {
        if (!root.TryGetProperty("requestId", out var requestValue) ||
            !root.TryGetProperty("text", out var textValue)) return;
        var requestId = requestValue.GetString();
        var text = textValue.GetString();
        if (requestId is null || text is null || text.Length > MaximumBridgeTextCharacters) return;
        if (_documentRequests.TryGetValue(requestId, out var completion)) completion.TrySetResult(text);
    }

    private void CopyFromBridge(JsonElement root)
    {
        if (!root.TryGetProperty("text", out var value)) return;
        var text = value.GetString();
        if (string.IsNullOrEmpty(text) || text.Length > MaximumBridgeTextCharacters) return;
        _clipboard.CopyText(text, persistAfterExit: false);
    }

    private void CancelPendingDocumentRequests()
    {
        foreach (var request in _documentRequests.Values)
            request.TrySetCanceled();
        _documentRequests.Clear();
    }

    private static string FileName(ScriptLanguage language) => language switch
    {
        ScriptLanguage.PowerShell => "scratch.ps1",
        ScriptLanguage.Python => "scratch.py",
        ScriptLanguage.Bash => "scratch.sh",
        ScriptLanguage.CommandPrompt => "scratch.cmd",
        ScriptLanguage.JavaScript => "scratch.js",
        ScriptLanguage.TypeScript => "scratch.ts",
        _ => "scratch.txt"
    };
}
