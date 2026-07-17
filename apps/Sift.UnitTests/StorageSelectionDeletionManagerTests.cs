using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class StorageSelectionDeletionManagerTests
{
    [Fact]
    public async Task Exact_child_directory_is_inventoried_twice_and_sent_to_recycle_bin_only()
    {
        var fixture = CreateFixture();
        try
        {
            var storage = new RecordingDeleter();
            var manager = new StorageSelectionDeletionManager(storage);
            var (tree, target) = Scan(fixture.Root, fixture.Target);

            var preflight = await manager.PreflightAsync(tree, target.Index, TestContext.Current.CancellationToken);
            var result = await manager.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);

            Assert.True(preflight.CanDelete, preflight.Detail);
            Assert.Equal(2, preflight.FileCount);
            Assert.True(result.Succeeded, result.Summary);
            Assert.True(storage.RecycleBinCalled);
            Assert.False(storage.DryRun);
            Assert.Equal(fixture.Target, Assert.Single(storage.Paths));
        }
        finally { Directory.Delete(fixture.Root, true); }
    }

    [Fact]
    public async Task Changed_content_after_confirmation_invalidates_the_one_time_ticket()
    {
        var fixture = CreateFixture();
        try
        {
            var storage = new RecordingDeleter();
            var manager = new StorageSelectionDeletionManager(storage);
            var (tree, target) = Scan(fixture.Root, fixture.Target);
            var preflight = await manager.PreflightAsync(tree, target.Index, TestContext.Current.CancellationToken);
            await File.WriteAllTextAsync(Path.Combine(fixture.Target, "late.txt"), "changed",
                TestContext.Current.CancellationToken);

            var result = await manager.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);
            var replay = await manager.ExecuteAsync(preflight.TicketId, TestContext.Current.CancellationToken);

            Assert.False(result.Succeeded);
            Assert.Contains("changed after confirmation", result.Summary, StringComparison.OrdinalIgnoreCase);
            Assert.False(replay.Succeeded);
            Assert.Empty(storage.Paths);
        }
        finally { Directory.Delete(fixture.Root, true); }
    }

    [Fact]
    public async Task Stale_map_and_scanned_root_are_blocked_before_authorization()
    {
        var fixture = CreateFixture();
        try
        {
            var manager = new StorageSelectionDeletionManager(new RecordingDeleter());
            var (tree, target) = Scan(fixture.Root, fixture.Target);
            await File.WriteAllTextAsync(Path.Combine(fixture.Target, "after-scan.txt"), "new",
                TestContext.Current.CancellationToken);

            var stale = await manager.PreflightAsync(tree, target.Index, TestContext.Current.CancellationToken);
            var root = await manager.PreflightAsync(tree, tree.RootIndices[0], TestContext.Current.CancellationToken);

            Assert.False(stale.CanDelete);
            Assert.Contains("changed after the map", stale.Detail, StringComparison.OrdinalIgnoreCase);
            Assert.False(root.CanDelete);
            Assert.Contains("root itself", root.Detail, StringComparison.OrdinalIgnoreCase);
        }
        finally { Directory.Delete(fixture.Root, true); }
    }

    [Fact]
    public async Task Protected_child_never_receives_a_ticket()
    {
        var fixture = CreateFixture();
        try
        {
            var storage = new RecordingDeleter { Protect = true };
            var manager = new StorageSelectionDeletionManager(storage);
            var (tree, target) = Scan(fixture.Root, fixture.Target);

            var preflight = await manager.PreflightAsync(tree, target.Index, TestContext.Current.CancellationToken);

            Assert.False(preflight.CanDelete);
            Assert.Empty(preflight.TicketId);
            Assert.Empty(storage.Paths);
        }
        finally { Directory.Delete(fixture.Root, true); }
    }

    [Theory]
    [InlineData(0)]
    [InlineData(99)]
    public async Task Malformed_parent_chains_are_blocked_without_throwing(int parentIndex)
    {
        var tree = new StorageTree();
        tree.Nodes.Add(new StorageNode
        {
            Index = 0,
            ParentIndex = parentIndex,
            Name = "forged",
            FullPath = Path.Combine(Path.GetTempPath(), "sift-forged-storage-node"),
            IsDirectory = true
        });
        var manager = new StorageSelectionDeletionManager(new RecordingDeleter());

        var preflight = await manager.PreflightAsync(tree, 0, TestContext.Current.CancellationToken);

        Assert.False(preflight.CanDelete);
        Assert.Contains("parent chain", preflight.Detail, StringComparison.OrdinalIgnoreCase);
    }

    private static (StorageTree Tree, StorageNode Target) Scan(string root, string target)
    {
        var tree = new StorageScanner().Scan([root], null, CancellationToken.None);
        return (tree, tree.Nodes.Single(node => node.FullPath.Equals(target, StringComparison.OrdinalIgnoreCase)));
    }

    private static (string Root, string Target) CreateFixture()
    {
        var root = Path.Combine(Path.GetTempPath(), "sift-storage-delete-tests", Guid.NewGuid().ToString("N"));
        var target = Path.Combine(root, "candidate");
        Directory.CreateDirectory(Path.Combine(target, "nested"));
        File.WriteAllText(Path.Combine(target, "one.bin"), new string('a', 128));
        File.WriteAllText(Path.Combine(target, "nested", "two.bin"), new string('b', 256));
        return (root, target);
    }

    private sealed class RecordingDeleter : IStorageDeleter
    {
        public bool Protect { get; init; }
        public List<string> Paths { get; } = [];
        public bool RecycleBinCalled { get; private set; }
        public bool DryRun { get; private set; }

        public bool IsProtected(string path, out string reason)
        {
            reason = Protect ? "test protection" : string.Empty;
            return Protect;
        }

        public StorageDeleteResult MoveToRecycleBin(IEnumerable<string> paths, bool dryRun = false)
        {
            Paths.AddRange(paths);
            RecycleBinCalled = true;
            DryRun = dryRun;
            return new StorageDeleteResult { Deleted = Paths.Count, Log = ["TEST recycle bin"] };
        }
    }
}
