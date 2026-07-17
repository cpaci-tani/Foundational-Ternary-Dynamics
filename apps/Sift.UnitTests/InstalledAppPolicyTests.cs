using Sift.Models;
using Sift.Services;
using Microsoft.Win32;

namespace Sift.UnitTests;

public sealed class InstalledAppPolicyTests
{
    private const string ProductCode = "{01234567-89AB-CDEF-0123-456789ABCDEF}";

    [Fact]
    public void Parser_accepts_only_plain_interactive_msi_product_commands()
    {
        Assert.True(InstalledAppPolicy.TryParseUninstallCommand($"MsiExec.exe /I{ProductCode}", out var installPlan, out _));
        Assert.NotNull(installPlan);
        Assert.True(InstalledAppPolicy.TryParseUninstallCommand($"msiexec /X {ProductCode}", out var uninstallPlan, out _));
        Assert.NotNull(uninstallPlan);

        Assert.False(InstalledAppPolicy.TryParseUninstallCommand($"MsiExec.exe /X{ProductCode} /qn", out _, out _));
        Assert.False(InstalledAppPolicy.TryParseUninstallCommand("MsiExec.exe /X{not-a-guid}", out _, out _));
    }

    [Fact]
    public void Parser_rejects_script_hosts_uris_relative_and_missing_executables()
    {
        Assert.False(InstalledAppPolicy.TryParseUninstallCommand("powershell.exe -Command Remove-AppxPackage example", out _, out _));
        Assert.False(InstalledAppPolicy.TryParseUninstallCommand("cmd.exe /c vendor.cmd", out _, out _));
        Assert.False(InstalledAppPolicy.TryParseUninstallCommand("ms-settings:appsfeatures", out _, out _));
        Assert.False(InstalledAppPolicy.TryParseUninstallCommand("vendor-uninstall.exe", out _, out _));
        Assert.False(InstalledAppPolicy.TryParseUninstallCommand(@"C:\definitely-missing\uninstall.exe", out _, out _));
    }

    [Fact]
    public void Policy_protects_components_updates_drivers_runtimes_and_missing_commands()
    {
        Assert.False(InstalledAppPolicy.Evaluate(Values("Vendor app", systemComponent: true)).Allowed);
        Assert.False(InstalledAppPolicy.Evaluate(Values("Vendor update", releaseType: "Update")).Allowed);
        Assert.False(InstalledAppPolicy.Evaluate(Values("Example display driver")).Allowed);
        Assert.False(InstalledAppPolicy.Evaluate(Values("Microsoft Visual C++ Runtime")).Allowed);
        Assert.False(InstalledAppPolicy.Evaluate(Values("Vendor app", uninstall: string.Empty)).Allowed);
        Assert.True(InstalledAppPolicy.Evaluate(Values("Vendor app")).Allowed);
    }

    [Fact]
    public void Orphan_policy_requires_two_missing_registered_targets_and_excludes_protected_owners()
    {
        var root = Path.Combine(Path.GetTempPath(), "Sift-OrphanPolicy-" + Guid.NewGuid().ToString("N"));
        var missingInstall = Path.Combine(root, "missing-app");
        var missingUninstaller = Path.Combine(missingInstall, "uninstall.exe");
        var candidate = OrphanValues("Vendor app", "Vendor", missingInstall, $"\"{missingUninstaller}\"");

        Assert.True(InstalledAppPolicy.IsConservativeOrphan(candidate, out var evidence));
        Assert.Contains(missingInstall, evidence);
        Assert.False(InstalledAppPolicy.IsConservativeOrphan(candidate with { Publisher = "Microsoft Corporation" }, out _));
        Assert.False(InstalledAppPolicy.IsConservativeOrphan(candidate with { WindowsInstaller = true }, out _));
        Assert.False(InstalledAppPolicy.IsConservativeOrphan(candidate with { InstallLocation = "relative-folder" }, out _));

        Directory.CreateDirectory(root);
        var existingUninstaller = Path.Combine(root, "still-registered.exe");
        File.WriteAllText(existingUninstaller, "fixture");
        Assert.False(InstalledAppPolicy.IsConservativeOrphan(
            candidate with { UninstallString = $"\"{existingUninstaller}\"" }, out _));
        File.Delete(existingUninstaller);
        Directory.CreateDirectory(missingInstall);
        try { Assert.False(InstalledAppPolicy.IsConservativeOrphan(candidate, out _)); }
        finally { Directory.Delete(root, recursive: true); }
    }

    [Fact]
    public async Task Manager_preview_revalidates_exact_identity_without_launching()
    {
        var fixture = App("Vendor app", $"MsiExec.exe /I{ProductCode}");
        var manager = new InstalledAppManager(new FixtureInventory(fixture));

        var result = await manager.UninstallAsync(fixture, preview: true, TestContext.Current.CancellationToken);

        Assert.True(result.Previewed);
        Assert.False(result.Executed);
        Assert.False(result.Blocked);
    }

    [Fact]
    public async Task Manager_confirmed_action_launches_the_exact_registered_uninstaller()
    {
        var fixture = App("Vendor app", $"MsiExec.exe /X {ProductCode}");
        var launcher = new RecordingLauncher();
        var manager = new InstalledAppManager(new FixtureInventory(fixture), launcher: launcher,
            uninstallSettleWindow: TimeSpan.Zero);

        var preflight = await manager.UninstallAsync(fixture, preview: true, TestContext.Current.CancellationToken);
        Assert.True(preflight.Previewed);
        Assert.Empty(launcher.Plans);

        var actual = await manager.UninstallAsync(fixture, preview: false, TestContext.Current.CancellationToken);

        Assert.True(actual.Executed);
        Assert.False(actual.Blocked);
        Assert.Null(actual.ContinuationToken);
        Assert.False(string.IsNullOrWhiteSpace(actual.UninstallSessionId));
        var plan = Assert.Single(launcher.Plans);
        Assert.Equal(Path.Combine(Environment.SystemDirectory, "msiexec.exe"), plan.FileName);
        Assert.Equal($"/X {ProductCode}", plan.Arguments);
    }

    [Fact]
    public async Task Manager_authorizes_leftovers_only_after_process_exit_and_confirmed_registration_removal()
    {
        var fixture = App("Vendor app", $"MsiExec.exe /X {ProductCode}");
        var inventory = new MutableInventory(fixture);
        var launcher = new RecordingLauncher();
        var manager = new InstalledAppManager(inventory, launcher: launcher,
            uninstallSettleWindow: TimeSpan.Zero);
        var launched = await manager.UninstallAsync(fixture, preview: false, TestContext.Current.CancellationToken);

        var waiting = manager.WaitForUninstallCompletionAsync(fixture, launched.UninstallSessionId!,
            TestContext.Current.CancellationToken);
        Assert.False(waiting.IsCompleted);
        Assert.Null(launched.ContinuationToken);

        inventory.Current = null;
        launcher.LastHandle.Complete();
        var completed = await waiting;

        Assert.True(completed.Completed);
        Assert.False(completed.Blocked);
        Assert.False(string.IsNullOrWhiteSpace(completed.ContinuationToken));
    }

    [Fact]
    public async Task Manager_keeps_leftovers_locked_when_uninstaller_exits_but_registration_remains()
    {
        var fixture = App("Vendor app", $"MsiExec.exe /X {ProductCode}");
        var launcher = new RecordingLauncher();
        var manager = new InstalledAppManager(new FixtureInventory(fixture), launcher: launcher,
            uninstallSettleWindow: TimeSpan.Zero);
        var launched = await manager.UninstallAsync(fixture, preview: false, TestContext.Current.CancellationToken);

        launcher.LastHandle.Complete();
        var completion = await manager.WaitForUninstallCompletionAsync(fixture, launched.UninstallSessionId!,
            TestContext.Current.CancellationToken);

        Assert.False(completion.Completed);
        Assert.False(completion.Blocked);
        Assert.Null(completion.ContinuationToken);
        Assert.Contains("still registered", completion.Message);
    }

    [Fact]
    public async Task Manager_manual_check_can_confirm_removal_after_a_delegating_uninstaller_exits()
    {
        var fixture = App("Vendor app", $"MsiExec.exe /X {ProductCode}");
        var inventory = new MutableInventory(fixture);
        var launcher = new RecordingLauncher();
        var manager = new InstalledAppManager(inventory, launcher: launcher,
            uninstallSettleWindow: TimeSpan.Zero);
        var launched = await manager.UninstallAsync(fixture, preview: false, TestContext.Current.CancellationToken);
        launcher.LastHandle.Complete();
        var first = await manager.WaitForUninstallCompletionAsync(fixture, launched.UninstallSessionId!,
            TestContext.Current.CancellationToken);
        Assert.False(first.Completed);

        inventory.Current = null;
        var checkedAgain = await manager.CheckUninstallCompletionAsync(fixture, launched.UninstallSessionId!,
            TestContext.Current.CancellationToken);

        Assert.True(checkedAgain.Completed);
        Assert.False(string.IsNullOrWhiteSpace(checkedAgain.ContinuationToken));
    }

    [Fact]
    public async Task Verified_uninstall_session_token_unlocks_only_the_matching_exact_leftover_path()
    {
        var displayName = "AcmeVerifiedRemoval" + Guid.NewGuid().ToString("N");
        var fixture = App(displayName, $"MsiExec.exe /X {ProductCode}");
        var inventory = new MutableInventory(fixture);
        var launcher = new RecordingLauncher();
        var manager = new InstalledAppManager(inventory, launcher: launcher,
            uninstallSettleWindow: TimeSpan.Zero);
        var leftoverPath = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), displayName);
        try
        {
            Directory.CreateDirectory(leftoverPath);
            File.WriteAllText(Path.Combine(leftoverPath, "leftover.dat"), "fixture");
            var launched = await manager.UninstallAsync(fixture, preview: false,
                TestContext.Current.CancellationToken);
            inventory.Current = null;
            launcher.LastHandle.Complete();
            var completion = await manager.WaitForUninstallCompletionAsync(fixture, launched.UninstallSessionId!,
                TestContext.Current.CancellationToken);

            var scan = manager.ScanLeftovers(fixture, completion.ContinuationToken,
                TestContext.Current.CancellationToken);

            Assert.False(scan.Blocked);
            Assert.Contains(scan.Candidates, candidate =>
                string.Equals(candidate.Path, leftoverPath, StringComparison.OrdinalIgnoreCase));
        }
        finally
        {
            if (Directory.Exists(leftoverPath)) Directory.Delete(leftoverPath, recursive: true);
        }
    }

    [Fact]
    public async Task Manager_blocks_missing_stale_and_forged_entries()
    {
        var registered = App("Vendor app", $"MsiExec.exe /I{ProductCode}");
        var manager = new InstalledAppManager(new FixtureInventory(registered));
        var stale = registered with { UninstallString = "MsiExec.exe /I{11111111-1111-1111-1111-111111111111}" };
        var forgedName = registered with { DisplayName = "Another app" };
        var missing = registered with
        {
            RegistryLocation = new InstalledAppRegistryLocation("HKCU", "64-bit", @"Software\Microsoft\Windows\CurrentVersion\Uninstall\missing")
        };

        Assert.True((await manager.UninstallAsync(stale, true, TestContext.Current.CancellationToken)).Blocked);
        Assert.True((await manager.UninstallAsync(forgedName, true, TestContext.Current.CancellationToken)).Blocked);
        Assert.True((await manager.UninstallAsync(missing, true, TestContext.Current.CancellationToken)).Blocked);
    }

    [Fact]
    public async Task Manager_cleanup_previews_then_backs_up_and_removes_only_exact_hkcu_orphan()
    {
        var backupRoot = Path.Combine(Path.GetTempPath(), "Sift-OrphanBackup-" + Guid.NewGuid().ToString("N"));
        var child = "Sift-Orphan-" + Guid.NewGuid().ToString("N");
        var subKey = $@"Software\Microsoft\Windows\CurrentVersion\Uninstall\{child}";
        var missingInstall = Path.Combine(backupRoot, "missing-app");
        var missingUninstaller = Path.Combine(missingInstall, "uninstall.exe");
        var sentinel = Path.Combine(backupRoot, "unrelated-user-file.txt");
        try
        {
            Directory.CreateDirectory(backupRoot);
            File.WriteAllText(sentinel, "must remain");
            using (var key = Registry.CurrentUser.CreateSubKey(subKey, writable: true))
            {
                key!.SetValue("DisplayName", "Acme cleanup fixture", RegistryValueKind.String);
                key.SetValue("Publisher", "Acme Test Vendor", RegistryValueKind.String);
                key.SetValue("InstallLocation", missingInstall, RegistryValueKind.String);
                key.SetValue("UninstallString", $"\"{missingUninstaller}\"", RegistryValueKind.String);
                key.SetValue("TypedValue", 17, RegistryValueKind.DWord);
            }

            var inventory = new InstalledAppInventory();
            var location = new InstalledAppRegistryLocation("HKCU", "64-bit", subKey);
            var app = Assert.IsType<InstalledApp>(inventory.FindExact(location));
            Assert.True(app.IsOrphanedRegistration);
            Assert.True(app.CanCleanRegistration);
            var manager = new InstalledAppManager(inventory, backupRoot);

            var preview = await manager.CleanupRegistrationAsync(app, true, TestContext.Current.CancellationToken);
            Assert.True(preview.Previewed);
            using (var previewKey = Registry.CurrentUser.OpenSubKey(subKey)) Assert.NotNull(previewKey);
            Assert.Empty(Directory.Exists(backupRoot) ? Directory.GetFiles(backupRoot, "*.json") : []);

            var cleaned = await manager.CleanupRegistrationAsync(app, false, TestContext.Current.CancellationToken);
            Assert.False(cleaned.Blocked);
            Assert.Null(Registry.CurrentUser.OpenSubKey(subKey));
            Assert.Equal("must remain", File.ReadAllText(sentinel));
            var backup = Assert.Single(Directory.GetFiles(backupRoot, "backup-registry-*.json"));

            var restored = await new TweakExecutor(backupRoot).RestoreFromAsync(backup,
                TweakCatalog.Create().ToDictionary(item => item.Id, StringComparer.OrdinalIgnoreCase));
            Assert.Equal(1, restored.Restored);
            using var restoredKey = Registry.CurrentUser.OpenSubKey(subKey);
            Assert.Equal(17, restoredKey!.GetValue("TypedValue"));
        }
        finally
        {
            Registry.CurrentUser.DeleteSubKeyTree(subKey, throwOnMissingSubKey: false);
            try { if (Directory.Exists(backupRoot)) Directory.Delete(backupRoot, recursive: true); } catch { }
        }
    }

    [Fact]
    public void Inventory_rejects_locations_outside_exact_uninstall_children()
    {
        var inventory = new InstalledAppInventory();
        Assert.Null(inventory.FindExact(new InstalledAppRegistryLocation("HKCU", "64-bit", @"Software\Sift.Tests\fixture")));
        Assert.Null(inventory.FindExact(new InstalledAppRegistryLocation("HKCU", "64-bit", @"Software\Microsoft\Windows\CurrentVersion\Uninstall\one\two")));
        Assert.Null(inventory.FindExact(new InstalledAppRegistryLocation("HKCR", "64-bit", @"Software\Microsoft\Windows\CurrentVersion\Uninstall\fixture")));
    }

    private static InstalledAppRegistryValues Values(string name, bool systemComponent = false,
        string releaseType = "", string uninstall = $"MsiExec.exe /I{ProductCode}") =>
        new(name, "Vendor", "1.0", string.Empty, string.Empty, 0, uninstall, false,
            systemComponent, releaseType, string.Empty);

    private static InstalledAppRegistryValues OrphanValues(string name, string publisher, string installLocation, string uninstall) =>
        new(name, publisher, "1.0", installLocation, string.Empty, 0, uninstall, false, false,
            string.Empty, string.Empty);

    private static InstalledApp App(string name, string uninstall) => new(
        new InstalledAppRegistryLocation("HKCU", "64-bit", @"Software\Microsoft\Windows\CurrentVersion\Uninstall\fixture"),
        name, "Vendor", "1.0", string.Empty, string.Empty, 0, uninstall, "Current user", true,
        "The registered interactive uninstaller can be opened after confirmation.");

    private sealed class FixtureInventory(InstalledApp fixture) : IInstalledAppInventory
    {
        public IReadOnlyList<InstalledApp> Enumerate(CancellationToken cancellationToken = default) => [fixture];
        public InstalledApp? FindExact(InstalledAppRegistryLocation location) =>
            string.Equals(location.Identity, fixture.RegistryLocation.Identity, StringComparison.OrdinalIgnoreCase) ? fixture : null;
    }

    private sealed class MutableInventory(InstalledApp? fixture) : IInstalledAppInventory
    {
        public InstalledApp? Current { get; set; } = fixture;
        public IReadOnlyList<InstalledApp> Enumerate(CancellationToken cancellationToken = default) =>
            Current is null ? [] : [Current];
        public InstalledApp? FindExact(InstalledAppRegistryLocation location) =>
            Current is not null && string.Equals(location.Identity, Current.RegistryLocation.Identity,
                StringComparison.OrdinalIgnoreCase) ? Current : null;
    }

    private sealed class RecordingLauncher : IInstalledAppLauncher
    {
        public List<InstalledAppLaunchPlan> Plans { get; } = [];
        public RecordingLaunchHandle LastHandle { get; private set; } = new();
        public IInstalledAppLaunchHandle Launch(InstalledAppLaunchPlan plan)
        {
            Plans.Add(plan);
            LastHandle = new RecordingLaunchHandle();
            return LastHandle;
        }
    }

    private sealed class RecordingLaunchHandle : IInstalledAppLaunchHandle
    {
        private readonly TaskCompletionSource _completion = new(TaskCreationOptions.RunContinuationsAsynchronously);
        public int? ProcessId => 4242;
        public Task WaitForExitAsync(CancellationToken cancellationToken = default) =>
            _completion.Task.WaitAsync(cancellationToken);
        public void Complete() => _completion.TrySetResult();
        public void Dispose() => _completion.TrySetCanceled();
    }
}
