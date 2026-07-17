using Microsoft.Win32;
using Sift.Services;
using Windows.ApplicationModel;

namespace Sift.WinUI.Infrastructure.Monitoring;

public sealed record DashboardMonitorState(
    bool StartupEnabled,
    bool Running,
    bool Paused,
    bool Packaged,
    string Detail);

public interface IDashboardMonitorController
{
    Task<DashboardMonitorState> GetStateAsync(CancellationToken cancellationToken = default);
    Task<DashboardMonitorState> SetStartupEnabledAsync(bool enabled, CancellationToken cancellationToken = default);
    Task<DashboardMonitorState> PauseAsync(CancellationToken cancellationToken = default);
    Task<DashboardMonitorState> ResumeAsync(CancellationToken cancellationToken = default);
    Task ReloadPreferencesAsync(CancellationToken cancellationToken = default);
    Task<bool> ClearHistoryAsync(CancellationToken cancellationToken = default);
    Task EnsureRunningAsync(CancellationToken cancellationToken = default);
}

/// <summary>
/// Controls only Sift's fixed, as-invoker monitor payload. The monitor protocol deliberately has
/// no process, script, action, path, or elevation message.
/// </summary>
public sealed class DashboardMonitorController : IDashboardMonitorController
{
    private const string StartupTaskId = "SiftMonitor";
    private const string RunKeyPath = @"Software\Microsoft\Windows\CurrentVersion\Run";
    private const string RunValueName = "SiftMonitor";
    private readonly string _appVersion;
    private readonly string _monitorExecutable;

    public DashboardMonitorController(string appVersion)
    {
        _appVersion = appVersion;
        _monitorExecutable = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "MonitorHost", "Sift.MonitorHost.exe"));
    }

    public async Task<DashboardMonitorState> GetStateAsync(CancellationToken cancellationToken = default)
    {
        var packaged = IsPackaged();
        var startupEnabled = packaged ? await IsPackagedStartupEnabledAsync() : IsFolderStartupEnabled();
        var response = await DashboardMonitorTelemetrySource.SendCommandAsync("status", _appVersion, cancellationToken);
        var compatible = IsCompatible(response);
        var detail = response is { Command: "version-mismatch" }
            ? response.Message ?? "A monitor from another Sift version is running."
            : response is not null && !HasRequiredCapabilities(response)
                ? "An incompatible Sift monitor is running; restart it from this Sift release."
            : compatible && response!.LastSampleUtc is { } sampled &&
              DateTimeOffset.UtcNow - sampled > DashboardMonitorTelemetrySource.MaximumHostSampleAge && !response.Paused
                ? $"Monitor is running but its last sample is stale ({(DateTimeOffset.UtcNow - sampled).TotalSeconds:0} seconds old)."
                : response?.Message ?? (File.Exists(_monitorExecutable) ? "Monitor is not running." : "Monitor payload is unavailable.");
        return new DashboardMonitorState(startupEnabled, compatible, compatible && response?.Paused == true, packaged, detail);
    }

    public async Task<DashboardMonitorState> SetStartupEnabledAsync(bool enabled, CancellationToken cancellationToken = default)
    {
        if (IsPackaged()) await SetPackagedStartupEnabledAsync(enabled);
        else SetFolderStartupEnabled(enabled);

        if (enabled) await EnsureRunningAsync(cancellationToken);
        else await DashboardMonitorTelemetrySource.SendCommandAsync("shutdown", _appVersion, cancellationToken);
        return await GetStateAsync(cancellationToken);
    }

    public async Task<DashboardMonitorState> PauseAsync(CancellationToken cancellationToken = default)
    {
        await DashboardMonitorTelemetrySource.SendCommandAsync("pause", _appVersion, cancellationToken);
        return await GetStateAsync(cancellationToken);
    }

    public async Task<DashboardMonitorState> ResumeAsync(CancellationToken cancellationToken = default)
    {
        await EnsureRunningAsync(cancellationToken);
        await DashboardMonitorTelemetrySource.SendCommandAsync("resume", _appVersion, cancellationToken);
        return await GetStateAsync(cancellationToken);
    }

    public async Task ReloadPreferencesAsync(CancellationToken cancellationToken = default) =>
        _ = await DashboardMonitorTelemetrySource.SendCommandAsync("reload", _appVersion, cancellationToken);

    public async Task<bool> ClearHistoryAsync(CancellationToken cancellationToken = default)
    {
        var response = await DashboardMonitorTelemetrySource.SendCommandAsync("clear-history", _appVersion, cancellationToken);
        return IsCompatible(response) && response!.Message == "cleared";
    }

    public async Task EnsureRunningAsync(CancellationToken cancellationToken = default)
    {
        var existing = await DashboardMonitorTelemetrySource.SendCommandAsync("status", _appVersion, cancellationToken);
        if (IsCompatible(existing)) return;
        if (existing is { Command: "version-mismatch" })
            throw new InvalidOperationException(existing.Message ?? "A monitor from another Sift version is already running.");
        if (existing is not null)
            throw new InvalidOperationException("An incompatible Sift monitor is already running. Exit it and retry.");
        if (!File.Exists(_monitorExecutable)) return;
        EnsureTrustedMonitorPayload();
        System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
        {
            FileName = _monitorExecutable,
            UseShellExecute = true,
            WorkingDirectory = Path.GetDirectoryName(_monitorExecutable)!
        });
        for (var attempt = 0; attempt < 12; attempt++)
        {
            await Task.Delay(100, cancellationToken);
            var response = await DashboardMonitorTelemetrySource.SendCommandAsync("status", _appVersion, cancellationToken);
            if (IsCompatible(response)) return;
            if (response is { Command: "version-mismatch" })
                throw new InvalidOperationException(response.Message ?? "A monitor from another Sift version is already running.");
            if (response is not null)
                throw new InvalidOperationException("An incompatible Sift monitor is already running. Exit it and retry.");
        }
    }

    private bool IsCompatible(DashboardMonitorEnvelope? response) =>
        response is { Command: "status" } &&
        response.AppVersion.Equals(_appVersion, StringComparison.OrdinalIgnoreCase) &&
        HasRequiredCapabilities(response);

    private static bool HasRequiredCapabilities(DashboardMonitorEnvelope response) =>
        response.Capabilities?.Contains(DashboardMonitorProtocol.TelemetryOwnershipCapability,
            StringComparer.Ordinal) == true;

    private static bool IsPackaged()
    {
        try { return !string.IsNullOrWhiteSpace(Package.Current.Id.Name); }
        catch { return false; }
    }

    private static async Task<bool> IsPackagedStartupEnabledAsync()
    {
        try
        {
            var startup = await StartupTask.GetAsync(StartupTaskId);
            return startup.State == StartupTaskState.Enabled;
        }
        catch { return false; }
    }

    private static async Task SetPackagedStartupEnabledAsync(bool enabled)
    {
        var startup = await StartupTask.GetAsync(StartupTaskId);
        if (enabled)
        {
            var state = await startup.RequestEnableAsync();
            if (state != StartupTaskState.Enabled)
                throw new InvalidOperationException(state == StartupTaskState.DisabledByUser
                    ? "Windows has disabled Sift Monitor in Startup Apps. Re-enable it in Windows Settings."
                    : $"Windows did not enable the startup task ({state}).");
        }
        else startup.Disable();
    }

    private bool IsFolderStartupEnabled()
    {
        using var key = Registry.CurrentUser.OpenSubKey(RunKeyPath, writable: false);
        var actual = key?.GetValue(RunValueName) as string;
        return string.Equals(actual, Quote(_monitorExecutable), StringComparison.OrdinalIgnoreCase);
    }

    private void SetFolderStartupEnabled(bool enabled)
    {
        using var key = Registry.CurrentUser.CreateSubKey(RunKeyPath, writable: true)
            ?? throw new InvalidOperationException("The current-user Startup registry key is unavailable.");
        if (enabled)
        {
            if (!File.Exists(_monitorExecutable))
                throw new FileNotFoundException("The Sift monitor payload is unavailable.", _monitorExecutable);
            EnsureTrustedMonitorPayload();
            key.SetValue(RunValueName, Quote(_monitorExecutable), RegistryValueKind.String);
        }
        else if (string.Equals(key.GetValue(RunValueName) as string, Quote(_monitorExecutable),
                     StringComparison.OrdinalIgnoreCase))
            key.DeleteValue(RunValueName, throwOnMissingValue: false);
    }

    private void EnsureTrustedMonitorPayload()
    {
        var processPath = Environment.ProcessPath;
        if (!BinaryTrustPolicy.HaveSameTrustedSigner(processPath, _monitorExecutable, out var reason))
            throw new InvalidOperationException(
                $"Background monitoring requires a signed Sift release whose app and monitor signatures match. {reason}");
    }

    private static string Quote(string value) => $"\"{value}\"";
}
