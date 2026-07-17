using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class SettingsStoreTests
{
    [Fact]
    public void Settings_round_trip_is_versioned_and_normalized()
    {
        var root = Path.Combine(Path.GetTempPath(), "SiftSettings-" + Guid.NewGuid().ToString("N"));
        try
        {
            var store = new SettingsStore(root);
            store.Save(new AppSettings
            {
                RefreshInterval = "invalid",
                ChartHistory = 5000,
                ConsoleWidth = 50,
                UiScale = "large"
            });

            var loaded = store.Load();

            Assert.Equal(AppSettings.CurrentSchemaVersion, loaded.SchemaVersion);
            Assert.Equal("2 seconds", loaded.RefreshInterval);
            Assert.Equal(600, loaded.ChartHistory);
            Assert.Equal(300, loaded.ConsoleWidth);
            Assert.Equal("Large", loaded.UiScale);
        }
        finally { try { Directory.Delete(root, recursive: true); } catch { } }
    }

    [Fact]
    public void Corrupt_or_newer_settings_are_quarantined_instead_of_reused()
    {
        var root = Path.Combine(Path.GetTempPath(), "SiftSettings-" + Guid.NewGuid().ToString("N"));
        try
        {
            Directory.CreateDirectory(root);
            var path = Path.Combine(root, "settings.json");
            File.WriteAllText(path, "{\"SchemaVersion\":999}");

            var loaded = new SettingsStore(root).Load();

            Assert.Equal(AppSettings.CurrentSchemaVersion, loaded.SchemaVersion);
            Assert.False(File.Exists(path));
            Assert.Single(Directory.GetFiles(root, "settings.corrupt-*.json"));
        }
        finally { try { Directory.Delete(root, recursive: true); } catch { } }
    }

    [Fact]
    public void Legacy_and_unknown_non_authority_settings_remain_forward_compatible()
    {
        var root = Path.Combine(Path.GetTempPath(), "SiftSettings-" + Guid.NewGuid().ToString("N"));
        try
        {
            Directory.CreateDirectory(root);
            File.WriteAllText(Path.Combine(root, "settings.json"),
                """
                {
                  "RefreshInterval": "5 seconds",
                  "OfferSystemRestorePoint": false,
                  "OptimizeCategory": "Legacy",
                  "UnknownFutureProperty": { "nested": true }
                }
                """);

            var loaded = new SettingsStore(root).Load();

            Assert.Equal("5 seconds", loaded.RefreshInterval);
            Assert.False(loaded.OfferSystemRestorePoint);
            Assert.Empty(Directory.GetFiles(root, "settings.corrupt-*.json"));
        }
        finally { try { Directory.Delete(root, recursive: true); } catch { } }
    }
}
