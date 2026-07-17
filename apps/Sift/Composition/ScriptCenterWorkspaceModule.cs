using System.Collections.Concurrent;
using Sift.Infrastructure.Activity;
using Sift.Infrastructure.Operations;
using Sift.Models;
using Sift.Presentation;
using Sift.Services;
using Sift.WinUI.Views;
using Microsoft.UI.Xaml.Controls;
using Sift.WinUI.Infrastructure.Interop;
using Sift.WinUI.Infrastructure.Localization;

namespace Sift.WinUI.Composition;

public sealed class ScriptCenterWorkspaceModule : IWorkspaceModule
{
    private const int OutputBatchSize = 200;
    private readonly IScriptCommandService _service;
    private readonly IScriptStudioService _studio;
    private readonly IElevationBroker _elevation;
    private readonly ActivityHub _activity;
    private readonly OperationCoordinator _operations;
    private readonly IWindowsShellLauncher _shellLauncher;
    private readonly ScriptCenterWorkspaceView _view;
    private readonly ConcurrentQueue<(string Line, bool Error)> _outputQueue = new();
    private IReadOnlyList<ScriptRuntime> _runtimes = [];
    private CancellationTokenSource? _run;
    private CancellationTokenSource? _studioRequest;
    private Task _studioSuspension = Task.CompletedTask;
    private bool _active;
    private int _activationVersion;
    private int _studioAnalysisInFlight;
    private int _drainScheduled;

    public ScriptCenterWorkspaceModule(
        IScriptCommandService service,
        IScriptStudioService studio,
        IElevationBroker elevation,
        ActivityHub activity,
        OperationCoordinator operations,
        IClipboardService clipboard,
        IWindowsShellLauncher shellLauncher)
    {
        _service = service;
        _studio = studio;
        _elevation = elevation;
        _activity = activity;
        _operations = operations;
        _shellLauncher = shellLauncher;
        _view = new ScriptCenterWorkspaceView(clipboard);
        _view.RunRequested += Run;
        _view.StopRequested += Stop;
        _view.StudioAnalyzeRequested += AnalyzeStudio;
        _view.OpenWorkingDirectoryRequested += OpenWorkingDirectory;
        _view.Bind(service.Catalog, ElevationHelper.IsElevated());
    }

    public string Key => "Scripts";
    public string Title => "Script studio";
    public Control View => _view;
    public async Task ActivateAsync(CancellationToken cancellationToken = default)
    {
        var activationVersion = Interlocked.Increment(ref _activationVersion);
        _active = true;
        var pendingSuspension = _studioSuspension;
        await pendingSuspension;
        if (!_active || activationVersion != Volatile.Read(ref _activationVersion)) return;
        _view.ResumeStudio();
        if (ElevationHelper.IsElevated()) return;
        if (_runtimes.Count > 0) return;
        _view.SetStudioBusy(true, "Discovering registered local scripting runtimes…");
        var outcome = await _operations.RunLatestAsync(
            "script-studio-runtime-discovery",
            Key,
            "Script Studio runtime discovery",
            _studio.DiscoverRuntimesAsync,
            cancellationToken);
        if (!_active || activationVersion != Volatile.Read(ref _activationVersion)) return;
        if (outcome.Succeeded && outcome.Value is { } runtimes)
        {
            _runtimes = runtimes;
            _view.BindStudioRuntimes(runtimes);
            _view.SetStudioBusy(false, $"Discovered {runtimes.Count(item => item.Available):N0} available local runtime entries. No runtime was downloaded.");
        }
        else if (!outcome.Cancelled)
        {
            _view.ShowStudioError(outcome.Error?.Message ?? "Runtime discovery failed.");
        }
    }
    public Task RefreshAsync(CancellationToken cancellationToken = default) => Task.CompletedTask;
    public void FocusPrimarySearch() => _view.FocusSearch();
    public void Deactivate()
    {
        _active = false;
        Interlocked.Increment(ref _activationVersion);
        Stop(this, EventArgs.Empty);
        _studioRequest?.Cancel();
        _operations.Cancel("script-studio-runtime-discovery");
        _operations.Cancel("script-studio-analysis");
        _studioSuspension = SuspendStudioAfterAsync(_studioSuspension);
    }

    private async Task SuspendStudioAfterAsync(Task previousSuspension)
    {
        await previousSuspension;
        await _view.SuspendStudioAsync();
    }

    public void Dispose()
    {
        Deactivate();
        _view.RunRequested -= Run;
        _view.StopRequested -= Stop;
        _view.StudioAnalyzeRequested -= AnalyzeStudio;
        _view.OpenWorkingDirectoryRequested -= OpenWorkingDirectory;
        _view.DisposeStudio();
    }

    private async void AnalyzeStudio(object? sender, EventArgs e)
    {
        if (!_active || Interlocked.Exchange(ref _studioAnalysisInFlight, 1) != 0) return;
        _studioRequest = new CancellationTokenSource();
        try
        {
            _view.SetStudioBusy(true, "Reading the in-memory editor document…");
            var document = await _view.GetStudioDocumentAsync(_studioRequest.Token);
            _view.SetStudioBusy(true, $"Analyzing {document.FileName} without executing it…");
            var outcome = await _operations.RunLatestAsync(
                "script-studio-analysis",
                Key,
                $"{document.Language} document analysis",
                token => _studio.AnalyzeAsync(document, token),
                _studioRequest.Token);
            if (outcome.Succeeded && outcome.Value is { } analysis)
            {
                _view.ShowStudioAnalysis(analysis);
                _activity.Info(Key, "Script analysis completed", analysis.Summary);
            }
            else if (!outcome.Cancelled)
            {
                _view.SetStudioBusy(false, outcome.Error?.Message ?? "Script analysis failed.");
                _activity.Error(Key, "Script analysis failed", outcome.Error?.Message);
            }
        }
        catch (OperationCanceledException)
        {
            if (_active) _view.SetStudioBusy(false, "Script analysis was cancelled.");
        }
        catch (Exception exception)
        {
            _view.SetStudioBusy(false, $"Script analysis failed: {exception.Message}");
            _activity.Error(Key, "Script analysis failed", exception.Message);
        }
        finally
        {
            _studioRequest?.Dispose();
            _studioRequest = null;
            Interlocked.Exchange(ref _studioAnalysisInFlight, 0);
        }
    }

    private void OpenWorkingDirectory(object? sender, EventArgs e)
    {
        try
        {
            _shellLauncher.OpenFolder(Environment.SystemDirectory);
            _activity.Info(Key, "Opened terminal working directory", Environment.SystemDirectory);
        }
        catch (Exception exception)
        {
            _view.SetStudioBusy(false, $"Explorer could not open: {exception.Message}");
            _activity.Error(Key, "Explorer handoff failed", exception.Message);
        }
    }

    private async void Run(object? sender, EventArgs e)
    {
        var recipe = _view.Selected;
        if (recipe is null || _run is not null) return;
        var preflight = _service.Preflight(recipe);
        if (!preflight.Allowed)
        {
            var blocked = ReasonPresenter.PresentOrFallback(preflight.ReasonCode, preflight.BlockReason);
            _view.Append($"[BLOCKED] {blocked}", true);
            _activity.Warning(Key, "Command preflight blocked", blocked);
            return;
        }
        if (ScriptRecipeAccessPolicy.RequiresConfirmation(recipe) &&
            !await _view.ConfirmStateChangingAsync(recipe, preflight))
        {
            _activity.Info(Key, "Command confirmation cancelled", recipe.Title);
            return;
        }

        _run = new CancellationTokenSource();
        _view.Append($"> {recipe.Command}");
        _view.SetRunning(true, preflight.RequiresElevation
            ? "Waiting for administrator permission"
            : "Running command");
        try
        {
            if (preflight.RequiresElevation)
            {
                if (string.IsNullOrWhiteSpace(preflight.RecipeHash))
                    throw new InvalidOperationException("The catalog recipe identity was not prepared for elevation.");
                var elevated = await _elevation.RunCatalogRecipeAsync(
                    recipe.Id, preflight.RecipeHash, _run.Token);
                DrainOutput();
                foreach (var line in elevated.Log) _view.Append(line, !elevated.Succeeded);
                if (elevated.Cancelled)
                {
                    _view.Append("[cancelled]");
                    _activity.Info(Key, "Administrator command cancelled", recipe.Title, persist: true);
                }
                else if (elevated.Succeeded)
                {
                    _view.Append($"[ok] {elevated.Message}");
                    _activity.Info(Key, "Administrator command completed", $"{recipe.Title}: {elevated.Message}", persist: true);
                }
                else
                {
                    _view.Append($"[error] {elevated.Message}", true);
                    _activity.Error(Key, "Administrator command failed", elevated.Message, persist: true);
                }
            }
            else
            {
                var result = await _service.RunAsync(recipe, preflight, QueueOutput, _run.Token);
                DrainOutput();
                if (result.Cancelled)
                {
                    _view.Append("[cancelled]");
                    _activity.Info(Key, "Command execution cancelled", recipe.Title, persist: true);
                }
                else
                {
                    _view.Append($"[exit {result.ExitCode}] completed in {result.Duration:g}", result.ExitCode != 0);
                    _activity.Info(Key, "Command completed", $"{recipe.Title}: exit {result.ExitCode}", persist: true);
                }
            }
        }
        catch (OperationCanceledException)
        {
            _view.Append("[cancelled before launch]");
            _activity.Info(Key, "Command execution cancelled", $"{recipe.Title}: cancelled before launch", persist: true);
        }
        catch (Exception exception)
        {
            _view.Append($"[error] {exception.Message}", true);
            _activity.Error(Key, "Command failed", exception.Message, persist: true);
        }
        finally
        {
            _run?.Dispose();
            _run = null;
            _view.SetRunning(false, "Ready");
        }
    }

    private void QueueOutput(string line, bool error)
    {
        _outputQueue.Enqueue((line, error));
        ScheduleDrain();
    }

    private void ScheduleDrain()
    {
        if (Interlocked.Exchange(ref _drainScheduled, 1) != 0) return;
        if (!_view.DispatcherQueue.TryEnqueue(DrainOutput)) Interlocked.Exchange(ref _drainScheduled, 0);
    }

    private void DrainOutput()
    {
        var batch = new List<(string Line, bool Error)>(OutputBatchSize);
        while (batch.Count < OutputBatchSize && _outputQueue.TryDequeue(out var item)) batch.Add(item);
        if (batch.Count > 0) _view.AppendBatch(batch);
        Interlocked.Exchange(ref _drainScheduled, 0);
        if (!_outputQueue.IsEmpty) ScheduleDrain();
    }

    private void Stop(object? sender, EventArgs e) => _run?.Cancel();
}
