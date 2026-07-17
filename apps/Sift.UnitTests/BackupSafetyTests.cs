using Sift.Models;
using Sift.Services;
using Microsoft.Win32;

namespace Sift.UnitTests;

public sealed class BackupSafetyTests
{
    [Fact]
    public async Task OptimizePreflight_DoesNotCreateBackupDirectory()
    {
        var root = TempDirectory();
        var tweak = FixtureTweak();
        try
        {
            var executor = new TweakExecutor(root, [tweak]);
            var result = await executor.ApplyAsync(
                [tweak],
                dryRun: true,
                cancellationToken: TestContext.Current.CancellationToken);
            Assert.False(Directory.Exists(root));
            Assert.Equal(1, result.Previewed);
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public async Task OptimizeApply_PreCancelledTokenDoesNotCreateBackupOrMutate()
    {
        var root = TempDirectory();
        var subKey = $@"Software\Sift.Tests\{Guid.NewGuid():N}";
        var tweak = FixtureTweak(subKey);
        using var cancellation = new CancellationTokenSource();
        cancellation.Cancel();
        try
        {
            var executor = new TweakExecutor(root, [tweak]);
            await Assert.ThrowsAsync<OperationCanceledException>(() =>
                executor.ApplyAsync([tweak], dryRun: false, cancellation.Token));

            Assert.False(Directory.Exists(root));
            Assert.Null(Registry.CurrentUser.OpenSubKey(subKey));
        }
        finally
        {
            Registry.CurrentUser.DeleteSubKeyTree(subKey, throwOnMissingSubKey: false);
            DeleteDirectory(root);
        }
    }

    [Fact]
    public async Task OptimizeBackup_RoundTripsMultiStringWithOriginalKind()
    {
        var backupRoot = TempDirectory();
        var subKey = $@"Software\Sift.Tests\{Guid.NewGuid():N}";
        var tweak = FixtureTweak(subKey);
        try
        {
            using (var key = Registry.CurrentUser.CreateSubKey(subKey, writable: true))
                key!.SetValue("Fixture", new[] { "alpha", "beta" }, RegistryValueKind.MultiString);

            var executor = new TweakExecutor(backupRoot, [tweak]);
            var applied = await executor.ApplyAsync(
                [tweak],
                dryRun: false,
                cancellationToken: TestContext.Current.CancellationToken);
            using (var key = Registry.CurrentUser.OpenSubKey(subKey))
                Assert.Equal(RegistryValueKind.DWord, key!.GetValueKind("Fixture"));

            var restored = await executor.RestoreFromAsync(applied.BackupPath,
                new Dictionary<string, Tweak>(StringComparer.OrdinalIgnoreCase) { [tweak.Id] = tweak });
            Assert.Equal(1, restored.Restored);
            using (var key = Registry.CurrentUser.OpenSubKey(subKey))
            {
                Assert.Equal(RegistryValueKind.MultiString, key!.GetValueKind("Fixture"));
                Assert.Equal(new[] { "alpha", "beta" }, Assert.IsType<string[]>(key.GetValue("Fixture")));
            }
        }
        finally
        {
            Registry.CurrentUser.DeleteSubKeyTree(subKey, throwOnMissingSubKey: false);
            DeleteDirectory(backupRoot);
        }
    }

    [Fact]
    public async Task BackupNames_AreUniqueWithinOneSecond()
    {
        var backupRoot = TempDirectory();
        var subKey = $@"Software\Sift.Tests\{Guid.NewGuid():N}";
        var tweak = FixtureTweak(subKey);
        try
        {
            var executor = new TweakExecutor(backupRoot, [tweak]);
            var first = await executor.ApplyAsync(
                [tweak],
                dryRun: false,
                cancellationToken: TestContext.Current.CancellationToken);
            var second = await executor.ApplyAsync(
                [tweak],
                dryRun: false,
                cancellationToken: TestContext.Current.CancellationToken);
            Assert.NotEqual(first.BackupPath, second.BackupPath);
            Assert.Equal(2, Directory.GetFiles(backupRoot, "backup-*.json").Length);
        }
        finally
        {
            Registry.CurrentUser.DeleteSubKeyTree(subKey, throwOnMissingSubKey: false);
            DeleteDirectory(backupRoot);
        }
    }

    [Fact]
    public async Task MaintenanceCleaner_BlocksCallerConstructedArbitraryPath()
    {
        var path = Path.Combine(Path.GetTempPath(), "Sift-Arbitrary-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(path);
        try
        {
            var finding = new MaintenanceFinding
            {
                Id = "maintenance.temp.user",
                Category = MaintenanceCategory.TempFiles,
                Title = "Arbitrary",
                Path = path,
                Detail = "fixture",
                SizeBytes = 1,
                CanClean = true
            };
            var result = (await new MaintenanceCleaner().ReviewAsync(
                [finding], TestContext.Current.CancellationToken)).Result;
            Assert.Equal(1, result.Skipped);
            Assert.Contains(result.Log, line => line.StartsWith("BLOCKED"));
        }
        finally { DeleteDirectory(path); }
    }

    [Fact]
    public async Task MaintenanceCleaner_CannotBypassInstalledAppsLeftoverPolicy()
    {
        var path = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "SiftLegacyLeftover-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(path);
        File.WriteAllText(Path.Combine(path, "user-data.txt"), "must remain");
        try
        {
            var finding = new MaintenanceFinding
            {
                Id = "leftover.caller-forged",
                Category = MaintenanceCategory.AppLeftover,
                Title = "Caller-forged leftover",
                Path = path,
                Detail = "fixture",
                SizeBytes = 11,
                CanClean = true,
                IsSelected = true
            };

            var review = await new MaintenanceCleaner().ReviewAsync(
                [finding], TestContext.Current.CancellationToken);
            var result = review.Result;

            Assert.Equal(1, result.Skipped);
            Assert.Contains(result.Log, line => line.StartsWith("BLOCKED", StringComparison.Ordinal));
            Assert.True(File.Exists(Path.Combine(path, "user-data.txt")));
        }
        finally { DeleteDirectory(path); }
    }

    [Fact]
    public async Task OrphanUninstallBackup_RestoresTypedRegistryTree()
    {
        var backupRoot = TempDirectory();
        var childName = "SiftTest-" + Guid.NewGuid().ToString("N");
        var subKey = $@"Software\Microsoft\Windows\CurrentVersion\Uninstall\{childName}";
        var missingInstall = Path.Combine(backupRoot, "missing-app");
        var missingUninstaller = Path.Combine(missingInstall, "uninstall.exe");
        try
        {
            using (var key = Registry.CurrentUser.CreateSubKey(subKey, writable: true))
            {
                key!.SetValue("DisplayName", "Acme orphan fixture", RegistryValueKind.String);
                key.SetValue("Publisher", "Acme Test Vendor", RegistryValueKind.String);
                key.SetValue("InstallLocation", missingInstall, RegistryValueKind.String);
                key.SetValue("UninstallString", $"\"{missingUninstaller}\"", RegistryValueKind.String);
                key!.SetValue("Binary", new byte[] { 1, 2, 3 }, RegistryValueKind.Binary);
                key.SetValue("Names", new[] { "one", "two" }, RegistryValueKind.MultiString);
                using var child = key.CreateSubKey("Child", writable: true);
                child!.SetValue("Count", 42L, RegistryValueKind.QWord);
            }
            var finding = new MaintenanceFinding
            {
                Id = "orphan-uninstall.fixture",
                Category = MaintenanceCategory.OrphanUninstall,
                Title = "Fixture",
                Path = $@"HKCU\{subKey}",
                Detail = "fixture",
                SizeBytes = 0,
                CanClean = true,
                RequiresAdvancedConfirm = true,
                RegistryHive = "HKCU",
                RegistrySubKey = subKey,
                RegistryValues = new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase)
                {
                    ["DisplayName"] = "Acme orphan fixture",
                    ["InstallLocation"] = missingInstall,
                    ["UninstallString"] = $"\"{missingUninstaller}\""
                }
            };

            var cleaner = new MaintenanceCleaner(backupRoot);
            var review = await cleaner.ReviewAsync([finding], TestContext.Current.CancellationToken);
            Assert.True(review.CanExecute);
            var cleaned = await cleaner.ExecuteAsync(review.TicketId!, TestContext.Current.CancellationToken);
            Assert.Equal(1, cleaned.Cleaned);
            Assert.Null(Registry.CurrentUser.OpenSubKey(subKey));

            var backupPath = Assert.Single(Directory.GetFiles(backupRoot, "backup-registry-*.json"));
            var executor = new TweakExecutor(backupRoot);
            var restored = await executor.RestoreFromAsync(backupPath,
                TweakCatalog.Create().ToDictionary(x => x.Id, StringComparer.OrdinalIgnoreCase));
            Assert.Equal(1, restored.Restored);
            using var restoredKey = Registry.CurrentUser.OpenSubKey(subKey);
            Assert.Equal(new byte[] { 1, 2, 3 }, Assert.IsType<byte[]>(restoredKey!.GetValue("Binary")));
            Assert.Equal(new[] { "one", "two" }, Assert.IsType<string[]>(restoredKey.GetValue("Names")));
            using var restoredChild = restoredKey.OpenSubKey("Child");
            Assert.Equal(42L, restoredChild!.GetValue("Count"));
        }
        finally
        {
            Registry.CurrentUser.DeleteSubKeyTree(subKey, throwOnMissingSubKey: false);
            DeleteDirectory(backupRoot);
        }
    }

    private static Tweak FixtureTweak(string? subKey = null) => new()
    {
        Id = "test.registry.fixture",
        Title = "Fixture",
        Description = "Fixture",
        Category = "Test",
        Risk = TweakRisk.Safe,
        Kind = TweakKind.Registry,
        Target = $@"HKCU\{subKey ?? "Software\\Sift.Tests\\Preview"}",
        ValueName = "Fixture",
        DesiredValue = 1,
        Reversible = true
    };

    private static string TempDirectory() => Path.Combine(Path.GetTempPath(), "SiftUnit-" + Guid.NewGuid().ToString("N"));
    private static void DeleteDirectory(string path) { try { if (Directory.Exists(path)) Directory.Delete(path, recursive: true); } catch { } }
}
