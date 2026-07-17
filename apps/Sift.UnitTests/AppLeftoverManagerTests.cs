using System.Collections.Concurrent;
using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class AppLeftoverManagerTests
{
    [Fact]
    public void Scan_uses_exact_generated_AppData_paths_and_selects_nothing()
    {
        var name = "AcmeLeftover" + Guid.NewGuid().ToString("N");
        var localPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), name);
        var nearMatch = localPath + " Cache";
        Directory.CreateDirectory(Path.Combine(localPath, "nested"));
        Directory.CreateDirectory(nearMatch);
        File.WriteAllBytes(Path.Combine(localPath, "nested", "data.bin"), new byte[321]);
        try
        {
            var app = App(name, orphaned: true);
            var manager = Manager(new FixtureInventory(app), new RecordingStorageDeleter());

            var scan = manager.ScanLeftovers(app, null, TestContext.Current.CancellationToken);

            Assert.False(scan.Blocked);
            var candidate = Assert.Single(scan.Candidates);
            Assert.Equal(localPath, candidate.Path, ignoreCase: true);
            Assert.Equal(321, candidate.SizeBytes);
            Assert.Equal(1, candidate.FileCount);
            Assert.True(candidate.CanDelete);
            Assert.False(candidate.IsSelected);
            Assert.DoesNotContain(scan.Candidates, item => item.Path.Equals(nearMatch, StringComparison.OrdinalIgnoreCase));
        }
        finally
        {
            Delete(localPath);
            Delete(nearMatch);
        }
    }

    [Fact]
    public void Scan_blocks_apps_that_are_still_registered_as_installed()
    {
        var app = App("Acme active fixture", orphaned: false);
        var manager = Manager(new FixtureInventory(app), new RecordingStorageDeleter());

        var scan = manager.ScanLeftovers(app, null, TestContext.Current.CancellationToken);

        Assert.True(scan.Blocked);
        Assert.Contains("still registered", scan.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void Delete_revalidates_identity_and_uses_Recycle_Bin_only()
    {
        var name = "AcmeDelete" + Guid.NewGuid().ToString("N");
        var localPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), name);
        Directory.CreateDirectory(localPath);
        File.WriteAllText(Path.Combine(localPath, "settings.json"), "keep unless explicitly selected");
        try
        {
            var app = App(name, orphaned: true);
            var storage = new RecordingStorageDeleter();
            var manager = Manager(new FixtureInventory(app), storage);
            var candidate = Assert.Single(manager.ScanLeftovers(app, null, TestContext.Current.CancellationToken).Candidates);
            candidate.IsSelected = true;

            var preview = manager.DeleteLeftovers(app, null, [candidate], preview: true, TestContext.Current.CancellationToken);
            Assert.Equal(1, preview.Previewed);
            Assert.True(Directory.Exists(localPath));
            Assert.True(storage.RecycleBinCalled);
            Assert.True(storage.LastDryRun);

            var actual = manager.DeleteLeftovers(app, null, [candidate], preview: false, TestContext.Current.CancellationToken);
            Assert.Equal(1, actual.Deleted);
            Assert.True(Directory.Exists(localPath)); // The recording deleter never mutates the fixture.
            Assert.True(storage.RecycleBinCalled);
            Assert.False(storage.LastDryRun);

            var forged = new AppLeftoverCandidate
            {
                AppIdentity = candidate.AppIdentity,
                AppDisplayName = app.DisplayName,
                Path = Path.GetTempPath(),
                Scope = "forged",
                Evidence = "forged",
                CanDelete = true,
                IsSelected = true
            };
            var blocked = manager.DeleteLeftovers(app, null, [forged], preview: false, TestContext.Current.CancellationToken);
            Assert.Equal(1, blocked.Skipped);
            Assert.Equal(2, storage.Calls.Count);
        }
        finally { Delete(localPath); }
    }

    [Fact]
    public void Missing_registration_requires_a_matching_unexpired_session_token()
    {
        var app = App("Acme session fixture", orphaned: false);
        var authorizations = new ConcurrentDictionary<string, LeftoverAuthorization>(StringComparer.Ordinal);
        var manager = new AppLeftoverManager(new FixtureInventory(null), new RecordingStorageDeleter(), authorizations);
        Assert.True(manager.ScanLeftovers(app, null, TestContext.Current.CancellationToken).Blocked);

        authorizations["valid"] = new LeftoverAuthorization(AppLeftoverManager.Fingerprint(app), DateTime.UtcNow.AddMinutes(5));
        Assert.False(manager.ScanLeftovers(app, "valid", TestContext.Current.CancellationToken).Blocked);

        authorizations["expired"] = new LeftoverAuthorization(AppLeftoverManager.Fingerprint(app), DateTime.UtcNow.AddSeconds(-1));
        Assert.True(manager.ScanLeftovers(app, "expired", TestContext.Current.CancellationToken).Blocked);
        Assert.False(authorizations.ContainsKey("expired"));
    }

    private static IAppLeftoverManager Manager(IInstalledAppInventory inventory, IStorageDeleter storage) =>
        new AppLeftoverManager(inventory, storage,
            new ConcurrentDictionary<string, LeftoverAuthorization>(StringComparer.Ordinal));

    private static InstalledApp App(string name, bool orphaned) => new(
        new InstalledAppRegistryLocation("HKCU", "64-bit", $@"Software\Microsoft\Windows\CurrentVersion\Uninstall\{name}"),
        name, "Acme", "1.0", string.Empty, string.Empty, 0,
        $@"C:\missing\{name}\uninstall.exe", "Current user", !orphaned, orphaned ? "missing" : "eligible")
    {
        IsOrphanedRegistration = orphaned,
        CanCleanRegistration = orphaned,
        OrphanEvidence = orphaned ? "two targets missing" : string.Empty
    };

    private static void Delete(string path)
    {
        try { if (Directory.Exists(path)) Directory.Delete(path, recursive: true); } catch { }
    }

    private sealed class FixtureInventory(InstalledApp? current) : IInstalledAppInventory
    {
        public IReadOnlyList<InstalledApp> Enumerate(CancellationToken cancellationToken = default) => current is null ? [] : [current];
        public InstalledApp? FindExact(InstalledAppRegistryLocation location) => current;
    }

    private sealed class RecordingStorageDeleter : IStorageDeleter
    {
        public List<string> Calls { get; } = [];
        public bool RecycleBinCalled { get; private set; }
        public bool LastDryRun { get; private set; }
        public bool IsProtected(string path, out string reason) { reason = string.Empty; return false; }
        public StorageDeleteResult MoveToRecycleBin(IEnumerable<string> paths, bool dryRun = false)
        {
            var selected = paths.ToList();
            Calls.AddRange(selected);
            RecycleBinCalled = true;
            LastDryRun = dryRun;
            return new StorageDeleteResult { Deleted = selected.Count, Log = selected.Select(path => $"TEST {path}").ToList() };
        }
    }
}
