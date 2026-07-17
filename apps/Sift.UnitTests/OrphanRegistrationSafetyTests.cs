using Sift.Models;
using Sift.Services;
using Microsoft.Win32;

namespace Sift.UnitTests;

public sealed class OrphanRegistrationSafetyTests
{
    [Fact]
    public async Task Maintenance_preview_blocks_stale_or_forged_orphan_evidence()
    {
        var child = "Sift-StaleOrphan-" + Guid.NewGuid().ToString("N");
        var subKey = $@"Software\Microsoft\Windows\CurrentVersion\Uninstall\{child}";
        var root = Path.Combine(Path.GetTempPath(), "Sift-StaleOrphan-" + Guid.NewGuid().ToString("N"));
        var missingInstall = Path.Combine(root, "missing-app");
        var missingUninstaller = Path.Combine(missingInstall, "uninstall.exe");
        try
        {
            using (var key = Registry.CurrentUser.CreateSubKey(subKey, writable: true))
            {
                key!.SetValue("DisplayName", "Acme stale orphan fixture", RegistryValueKind.String);
                key.SetValue("Publisher", "Acme", RegistryValueKind.String);
                key.SetValue("InstallLocation", missingInstall, RegistryValueKind.String);
                key.SetValue("UninstallString", $"\"{missingUninstaller}\"", RegistryValueKind.String);
            }

            var finding = new MaintenanceFinding
            {
                Id = "orphan-uninstall.stale-fixture",
                Category = MaintenanceCategory.OrphanUninstall,
                Title = "Stale fixture",
                Path = $@"HKCU\{subKey}",
                Detail = "fixture",
                SizeBytes = 0,
                CanClean = true,
                RequiresAdvancedConfirm = true,
                RegistryHive = "HKCU",
                RegistrySubKey = subKey,
                RegistryValues = new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase)
                {
                    ["DisplayName"] = "Acme stale orphan fixture",
                    ["InstallLocation"] = missingInstall + "-forged",
                    ["UninstallString"] = $"\"{missingUninstaller}\""
                }
            };

            var preview = (await new MaintenanceCleaner(root).ReviewAsync(
                [finding], TestContext.Current.CancellationToken)).Result;
            Assert.Equal(1, preview.Skipped);
            Assert.Contains(preview.Log, line => line.StartsWith("BLOCKED", StringComparison.Ordinal));
            using var stillPresent = Registry.CurrentUser.OpenSubKey(subKey);
            Assert.NotNull(stillPresent);
        }
        finally
        {
            Registry.CurrentUser.DeleteSubKeyTree(subKey, throwOnMissingSubKey: false);
            try { if (Directory.Exists(root)) Directory.Delete(root, recursive: true); } catch { }
        }
    }
}
