using System.IO;
using System.Text.Json;
using Sift.Models;
using Sift.Infrastructure.Persistence;

namespace Sift.Services;

public sealed class SettingsStore : ISettingsStore
{
    private const long MaximumSettingsBytes = 1024 * 1024;
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNameCaseInsensitive = false
    };
    public string SettingsPath { get; }

    public SettingsStore(string? directory = null)
    {
        var root = directory ?? ProductPaths.DataRoot;
        Directory.CreateDirectory(root);
        SettingsPath = Path.Combine(root, "settings.json");
    }

    public AppSettings Load()
    {
        try
        {
            if (!File.Exists(SettingsPath)) return new AppSettings();
            if (new FileInfo(SettingsPath).Length > MaximumSettingsBytes)
                throw new InvalidDataException("The Sift settings document is too large.");
            var settings = JsonSerializer.Deserialize<AppSettings>(File.ReadAllText(SettingsPath), JsonOptions)
                ?? throw new InvalidDataException("The Sift settings document is empty.");
            Normalize(settings);
            return settings;
        }
        catch
        {
            QuarantineCorruptSettings();
            return new AppSettings();
        }
    }

    public void Save(AppSettings settings)
    {
        ArgumentNullException.ThrowIfNull(settings);
        Normalize(settings);
        AtomicFile.WriteAllText(SettingsPath, JsonSerializer.Serialize(settings, JsonOptions));
    }

    private static void Normalize(AppSettings settings)
    {
        if (settings.SchemaVersion != AppSettings.CurrentSchemaVersion)
            throw new InvalidDataException($"Sift settings schema {settings.SchemaVersion} is not supported.");
        settings.RefreshInterval = settings.RefreshInterval is "1 second" or "2 seconds" or "3 seconds" or "5 seconds"
            ? settings.RefreshInterval : "2 seconds";
        settings.ChartHistory = Math.Clamp(settings.ChartHistory, 30, 600);
        settings.ChartSmoothing = ChartSmoothingPolicy.Normalize(settings.ChartSmoothing);
        settings.UiScale = UiScalePolicy.Normalize(settings.UiScale);
        settings.ConsoleWidth = Math.Clamp(settings.ConsoleWidth, 300, 520);
        settings.LastWorkspace = string.IsNullOrWhiteSpace(settings.LastWorkspace) || settings.LastWorkspace.Length > 80
            ? "Home" : settings.LastWorkspace;
        settings.HomeWidgets ??= new Dictionary<string, bool>(StringComparer.OrdinalIgnoreCase);
        if (settings.HomeWidgets.Count > 100 || settings.HomeWidgets.Keys.Any(key =>
                string.IsNullOrWhiteSpace(key) || key.Length > 100))
            throw new InvalidDataException("Legacy Home widget settings are invalid.");
        settings.StorageRoots ??= [];
        settings.StorageRoots = settings.StorageRoots.Where(root => !string.IsNullOrWhiteSpace(root) && root.Length <= 512)
            .Distinct(StringComparer.OrdinalIgnoreCase).Take(16).ToList();
        settings.Dashboard ??= new DashboardPreferences();
        settings.Dashboard.HistoryRetentionDays = Math.Clamp(settings.Dashboard.HistoryRetentionDays, 7, 365);
        settings.Dashboard.AlertRules ??= [];
        if (settings.Dashboard.AlertRules.Count > 64)
            throw new InvalidDataException("Dashboard alert-rule settings are invalid.");
        settings.Dashboard.ChartSmoothing = ChartSmoothingPolicy.Normalize(settings.Dashboard.ChartSmoothing);
        settings.HardwareCharts ??= new HardwareChartPreferences();
        settings.HardwareCharts.RefreshInterval = ChartRefreshIntervalPolicy.Normalize(settings.HardwareCharts.RefreshInterval);
        settings.HardwareCharts.HistorySamples = Math.Clamp(settings.HardwareCharts.HistorySamples, 30, 600);
        settings.HardwareCharts.ChartSmoothing = ChartSmoothingPolicy.Normalize(settings.HardwareCharts.ChartSmoothing);
        settings.RefreshInterval = ChartRefreshIntervalPolicy.Normalize(settings.RefreshInterval);
        settings.ChartHistory = Math.Clamp(settings.ChartHistory, 30, 600);
        settings.ChartSmoothing = ChartSmoothingPolicy.Normalize(settings.ChartSmoothing);
        if (!settings.PerformanceShowCpuSeries && !settings.PerformanceShowMemorySeries)
            settings.PerformanceShowCpuSeries = true;
    }

    private void QuarantineCorruptSettings()
    {
        try
        {
            if (!File.Exists(SettingsPath)) return;
            var quarantine = Path.Combine(Path.GetDirectoryName(SettingsPath)!,
                $"settings.corrupt-{DateTime.UtcNow:yyyyMMddHHmmssfff}.json");
            File.Move(SettingsPath, quarantine, overwrite: false);
        }
        catch { }
    }
}
