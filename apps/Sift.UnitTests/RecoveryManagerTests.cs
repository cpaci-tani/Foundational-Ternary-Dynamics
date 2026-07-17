using System.Text.Json;
using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class RecoveryManagerTests
{
    [Fact]
    public void Recovery_inventory_classifies_mixed_backup_and_preserves_exact_path_boundary()
    {
        var root = TempRoot();
        try
        {
            var path = WriteMixedBackup(root);
            var executor = new FakeExecutor(root);
            var manager = new RecoveryManager(executor, new FakeElevation(), () => false);

            var backup = Assert.Single(manager.ListBackups());
            Assert.Equal(path, backup.Path);
            Assert.True(backup.CanRestore);
            Assert.True(backup.RequiresElevation);
            Assert.Equal(2, backup.PendingCount);
            Assert.Throws<InvalidOperationException>(() => manager.InspectExact(Path.Combine(root, "..", "backup-forged.json")));
        }
        finally { Directory.Delete(root, true); }
    }

    [Fact]
    public async Task Mixed_restore_runs_protected_phase_before_current_user_phase()
    {
        var root = TempRoot();
        try
        {
            var path = WriteMixedBackup(root);
            var calls = new List<string>();
            var executor = new FakeExecutor(root, calls);
            var broker = new FakeElevation(calls);
            var manager = new RecoveryManager(executor, broker, () => false);

            var result = await manager.RestoreAsync(path, TestContext.Current.CancellationToken);

            Assert.True(result.Succeeded);
            Assert.Equal(["elevated", "CurrentUser"], calls);
            Assert.Equal(2, result.Restored);
        }
        finally { Directory.Delete(root, true); }
    }

    [Fact]
    public async Task Cancelled_administrator_confirmation_leaves_current_user_phase_unapplied()
    {
        var root = TempRoot();
        try
        {
            var path = WriteMixedBackup(root);
            var calls = new List<string>();
            var manager = new RecoveryManager(new FakeExecutor(root, calls), new FakeElevation(calls, cancel: true),
                () => false);

            var result = await manager.RestoreAsync(path, TestContext.Current.CancellationToken);

            Assert.True(result.Cancelled);
            Assert.Equal(["elevated"], calls);
        }
        finally { Directory.Delete(root, true); }
    }

    [Fact]
    public async Task Cancellation_after_protected_launch_does_not_abandon_the_paired_local_phase()
    {
        var root = TempRoot();
        using var cancellation = new CancellationTokenSource();
        try
        {
            var path = WriteMixedBackup(root);
            var calls = new List<string>();
            var manager = new RecoveryManager(new FakeExecutor(root, calls),
                new FakeElevation(calls, afterCall: cancellation.Cancel), () => false);

            var result = await manager.RestoreAsync(path, cancellation.Token);

            Assert.True(result.Succeeded);
            Assert.Equal(["elevated", "CurrentUser"], calls);
        }
        finally { Directory.Delete(root, true); }
    }

    [Fact]
    public void Elevated_restore_policy_accepts_only_bounded_prior_values_and_blocks_machine_trees()
    {
        var root = TempRoot();
        try
        {
            var valid = WriteBackup(root, "backup-valid.json", [Entry("privacy.telemetry", 3)]);
            Assert.True(ElevatedOperationPolicy.TryValidateMachineRestore(valid, out var validReason), validReason);

            var forged = WriteBackup(root, "backup-forged.json", [Entry("privacy.telemetry", int.MaxValue)]);
            Assert.False(ElevatedOperationPolicy.TryValidateMachineRestore(forged, out var forgedReason));
            Assert.Contains("cannot be restored", forgedReason, StringComparison.OrdinalIgnoreCase);

            var ambiguousLegacy = WriteBackup(root, "backup-legacy.json",
            [
                new BackupEntry
                {
                    TweakId = "privacy.activity", State = BackupEntryStates.Applied,
                    AppliedSuccessfully = true, Existed = true, Value = "1", RegistryKind = null
                }
            ]);
            Assert.False(ElevatedOperationPolicy.TryValidateMachineRestore(ambiguousLegacy, out var legacyReason));
            Assert.Contains("legacy prior registry snapshot", legacyReason, StringComparison.OrdinalIgnoreCase);

            var tree = WriteBackup(root, "backup-tree.json",
            [
                new BackupEntry
                {
                    TweakId = "orphan.test", State = BackupEntryStates.Applied, AppliedSuccessfully = true,
                    RegistryHive = "HKLM", RegistrySubKey = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\fixture",
                    RegistryTree = new RegistryKeySnapshot()
                }
            ]);
            Assert.False(ElevatedOperationPolicy.TryValidateMachineRestore(tree, out var treeReason));
            Assert.Contains("elevation boundary", treeReason, StringComparison.OrdinalIgnoreCase);
        }
        finally { Directory.Delete(root, true); }
    }

    [Fact]
    public void Elevation_request_resolves_only_exact_sibling_backup_names()
    {
        var requestId = Guid.NewGuid().ToString("N");
        var paths = ElevationOperationFiles.PathsFor(requestId);
        Directory.CreateDirectory(Path.GetDirectoryName(paths.RequestPath)!);
        try
        {
            var expected = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                "Sift", "Backups", "backup-safe.json");
            Assert.Equal(expected, ElevationOperationFiles.ResolveSiblingBackup(paths.RequestPath, "backup-safe.json"));
            Assert.Throws<InvalidDataException>(() =>
                ElevationOperationFiles.ResolveSiblingBackup(paths.RequestPath, @"..\backup-forged.json"));
        }
        finally { }
    }

    [Fact]
    public void Elevated_helper_can_reply_beside_a_different_fixed_drive_user_profile_request()
    {
        var root = TempRoot();
        try
        {
            var requestId = Guid.NewGuid().ToString("N");
            var requestDirectory = Path.Combine(root, "AlternateProfile", "AppData", "Local", "Sift", "Elevation");
            Directory.CreateDirectory(requestDirectory);
            var requestPath = Path.Combine(requestDirectory, requestId + ".request.json");

            var paths = ElevationOperationFiles.PathsBesideRequest(requestPath, requestId);

            Assert.Equal(requestPath, paths.RequestPath);
            Assert.Equal(Path.Combine(requestDirectory, requestId + ".response.json"), paths.ResponsePath);
        }
        finally { Directory.Delete(root, true); }
    }

    private static BackupEntry Entry(string id, int prior) => new()
    {
        TweakId = id,
        State = BackupEntryStates.Applied,
        AppliedSuccessfully = true,
        KeyExisted = true,
        Existed = true,
        RegistryValue = new RegistryValueSnapshot
        {
            Name = "AllowTelemetry", Kind = "DWord", Encoding = "Int32", Data = prior.ToString()
        }
    };

    private static string WriteMixedBackup(string root) => WriteBackup(root, "backup-mixed.json",
    [
        new BackupEntry { TweakId = "privacy.ad-id", State = BackupEntryStates.Applied, AppliedSuccessfully = true },
        new BackupEntry
        {
            TweakId = "privacy.activity", State = BackupEntryStates.Applied, AppliedSuccessfully = true,
            KeyExisted = true, Existed = true,
            RegistryValue = new RegistryValueSnapshot
            {
                Name = "PublishUserActivities", Kind = "DWord", Encoding = "Int32", Data = "1"
            }
        }
    ]);

    private static string WriteBackup(string root, string name, IReadOnlyList<BackupEntry> entries)
    {
        var path = Path.Combine(root, name);
        File.WriteAllText(path, JsonSerializer.Serialize(new Backup { Entries = entries.ToList() }));
        return path;
    }

    private static string TempRoot()
    {
        var root = Path.Combine(Path.GetTempPath(), "sift-recovery-tests", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        return root;
    }

    private sealed class FakeExecutor(string root, List<string>? calls = null) : ITweakExecutor
    {
        public string BackupDirectory => root;
        public bool IsApplied(Tweak tweak) => false;
        public Task<ApplyResult> ApplyAsync(IEnumerable<Tweak> selection, bool dryRun, CancellationToken cancellationToken = default) => throw new NotSupportedException();
        public Task<RestoreResult> RestoreFromAsync(string path, IReadOnlyDictionary<string, Tweak> catalog,
            RestoreScope scope = RestoreScope.All)
        {
            calls?.Add(scope.ToString());
            return Task.FromResult(new RestoreResult { BackupPath = path, Restored = scope == RestoreScope.CurrentUser ? 1 : 0 });
        }
        public IReadOnlyList<BackupInfo> ListBackups() => [];
    }

    private sealed class FakeElevation(List<string>? calls = null, bool cancel = false, Action? afterCall = null) : IElevationBroker
    {
        public Task<ElevatedOperationResponse> ApplyMachineTweaksAsync(IEnumerable<Tweak> selection,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> RestoreMachineBackupAsync(string backupPath,
            CancellationToken cancellationToken = default)
        {
            calls?.Add("elevated");
            afterCall?.Invoke();
            return Task.FromResult(cancel
                ? new ElevatedOperationResponse("test", "nonce", false, true, "Cancelled", 0, 0, [])
                : new ElevatedOperationResponse("test", "nonce", true, false, "Restored", 1, 0, ["RESTORED protected"]));
        }

        public Task<ElevatedOperationResponse> ManageServiceAsync(ServiceActionTarget target, ServiceActionKind action,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> ChangeScheduledTaskAsync(
            ScheduledTaskId id,
            ScheduledTaskChange change,
            bool expectedEnabled,
            string expectedDefinitionHash,
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> CreateSystemRestorePointAsync(
            CancellationToken cancellationToken = default) => throw new NotSupportedException();

        public Task<ElevatedOperationResponse> RunCatalogRecipeAsync(
            string recipeId, string expectedRecipeHash, CancellationToken cancellationToken = default) =>
            throw new NotSupportedException();
    }
}
