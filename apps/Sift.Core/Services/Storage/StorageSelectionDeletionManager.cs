using System.Collections.Concurrent;
using System.Security.Cryptography;
using System.Text;
using Sift.Models;

namespace Sift.Services;

public interface IStorageSelectionDeletionManager
{
    Task<StorageSelectionDeletePreflight> PreflightAsync(StorageTree tree, int nodeIndex,
        CancellationToken cancellationToken = default);
    Task<StorageSelectionDeleteResult> ExecuteAsync(string ticketId,
        CancellationToken cancellationToken = default);
    void Revoke(string ticketId);
}

public sealed class StorageSelectionDeletionManager(IStorageDeleter deleter) : IStorageSelectionDeletionManager
{
    private const int MaximumTickets = 8;
    private const int MaximumInventoryEntries = 2_000_000;
    private static readonly TimeSpan TicketLifetime = TimeSpan.FromMinutes(5);
    private readonly ConcurrentDictionary<string, DeleteTicket> _tickets = new(StringComparer.Ordinal);

    public async Task<StorageSelectionDeletePreflight> PreflightAsync(StorageTree tree, int nodeIndex,
        CancellationToken cancellationToken = default)
    {
        PruneTickets();
        if (nodeIndex < 0 || nodeIndex >= tree.Nodes.Count)
            return Blocked("The selected storage node is no longer part of the current map.");
        var node = tree.Nodes[nodeIndex];
        StorageNode root;
        try
        {
            root = FindRoot(tree, node);
        }
        catch (InvalidDataException exception)
        {
            return Blocked(exception.Message, node.FullPath);
        }
        if (node.Index == root.Index)
            return Blocked("The scanned root itself cannot be deleted. Select a child file or folder.", node.FullPath);
        if (node.IsReparsePoint)
            return Blocked("Reparse points, junctions, and symbolic links cannot be deleted from the storage map.", node.FullPath);
        if (!IsStrictlyUnder(node.FullPath, root.FullPath))
            return Blocked("The selected path escaped the exact scanned root.", node.FullPath);
        if (deleter.IsProtected(node.FullPath, out var protectedReason))
            return Blocked($"The selected path is protected: {protectedReason}.", node.FullPath);

        LiveInventory inventory;
        try
        {
            inventory = await Task.Run(() => Inventory(node.FullPath, cancellationToken), cancellationToken);
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException or InvalidDataException)
        {
            return Blocked($"Sift could not completely inventory the selected path: {exception.Message}", node.FullPath);
        }
        if (inventory.IsDirectory != node.IsDirectory)
            return Blocked("The selected path changed between the storage scan and deletion preflight.", node.FullPath);
        if (inventory.SizeBytes != node.Size || inventory.FileCount != node.FileCount)
            return Blocked(
                $"The selected path changed after the map was built. Map: {StorageRow.FormatSize(node.Size)} / {node.FileCount:N0} files; live: {StorageRow.FormatSize(inventory.SizeBytes)} / {inventory.FileCount:N0} files. Scan again before deleting.",
                node.FullPath);

        var ticketId = Guid.NewGuid().ToString("N");
        var expires = DateTime.UtcNow + TicketLifetime;
        _tickets[ticketId] = new DeleteTicket(ticketId, Path.GetFullPath(root.FullPath),
            Path.GetFullPath(node.FullPath), inventory, expires);
        while (_tickets.Count > MaximumTickets)
        {
            var oldest = _tickets.Values.OrderBy(ticket => ticket.ExpiresUtc).FirstOrDefault();
            if (oldest is null || !_tickets.TryRemove(oldest.Id, out _)) break;
        }
        return new StorageSelectionDeletePreflight(true,
            $"Ready to move {node.Name} to the Recycle Bin.",
            $"Live inventory matches the map: {StorageRow.FormatSize(inventory.SizeBytes)}, {inventory.FileCount:N0} files, {inventory.DirectoryCount:N0} folders. The complete path will be re-inventoried immediately before execution.",
            ticketId, node.FullPath, node.IsDirectory, inventory.SizeBytes, inventory.FileCount,
            inventory.DirectoryCount, expires);
    }

    public async Task<StorageSelectionDeleteResult> ExecuteAsync(string ticketId,
        CancellationToken cancellationToken = default)
    {
        if (!_tickets.TryRemove(ticketId, out var ticket))
            return Failed("The deletion authorization is missing or was already used.");
        if (ticket.ExpiresUtc <= DateTime.UtcNow)
            return Failed("The deletion authorization expired. Run preflight again.", ticket.TargetPath);
        cancellationToken.ThrowIfCancellationRequested();
        if (!IsStrictlyUnder(ticket.TargetPath, ticket.RootPath))
            return Failed("The authorized path no longer belongs to the exact scanned root.", ticket.TargetPath);
        if (deleter.IsProtected(ticket.TargetPath, out var protectedReason))
            return Failed($"The path became protected: {protectedReason}.", ticket.TargetPath);

        LiveInventory current;
        try
        {
            current = await Task.Run(() => Inventory(ticket.TargetPath, cancellationToken), cancellationToken);
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException or InvalidDataException)
        {
            return Failed($"Final live inventory failed: {exception.Message}", ticket.TargetPath);
        }
        if (!current.Equals(ticket.Inventory))
            return Failed("The selected path changed after confirmation. Nothing was deleted; run preflight again.",
                ticket.TargetPath);

        // Do not observe cancellation after final validation begins: the shell operation must report its real outcome.
        var result = deleter.MoveToRecycleBin([ticket.TargetPath], dryRun: false);
        return result.Deleted == 1 && result.Failed == 0 && result.Skipped == 0
            ? new StorageSelectionDeleteResult(true,
                $"Moved {Path.GetFileName(ticket.TargetPath.TrimEnd(Path.DirectorySeparatorChar))} to the Recycle Bin.",
                ticket.TargetPath, result.Log)
            : new StorageSelectionDeleteResult(false,
                $"Recycle Bin deletion did not complete: {result.Summary}", ticket.TargetPath, result.Log);
    }

    public void Revoke(string ticketId)
    {
        if (!string.IsNullOrWhiteSpace(ticketId)) _tickets.TryRemove(ticketId, out _);
    }

    private static LiveInventory Inventory(string targetPath, CancellationToken cancellationToken)
    {
        var full = Path.GetFullPath(targetPath);
        var attributes = File.GetAttributes(full);
        if (attributes.HasFlag(FileAttributes.ReparsePoint))
            throw new InvalidDataException("The target is now a reparse point.");
        var directory = attributes.HasFlag(FileAttributes.Directory);
        if (!directory)
        {
            var file = new FileInfo(full);
            var fingerprint = HashEntry("F", string.Empty, file.Length, file.LastWriteTimeUtc.Ticks, attributes);
            return new LiveInventory(false, file.Length, 1, 0, file.LastWriteTimeUtc.Ticks,
                Convert.ToHexString(fingerprint));
        }

        long bytes = 0;
        var files = 0;
        var directories = 1;
        var entries = 1;
        var aggregate = new byte[32];
        var root = full.TrimEnd(Path.DirectorySeparatorChar);
        var rootInfo = new DirectoryInfo(root);
        Xor(aggregate, HashEntry("D", string.Empty, 0, rootInfo.LastWriteTimeUtc.Ticks, attributes));
        var pending = new Stack<string>();
        pending.Push(root);
        while (pending.Count > 0)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var directoryPath = pending.Pop();
            var children = Directory.EnumerateFileSystemEntries(directoryPath).Order(StringComparer.OrdinalIgnoreCase);
            foreach (var child in children)
            {
                cancellationToken.ThrowIfCancellationRequested();
                if (++entries > MaximumInventoryEntries)
                    throw new InvalidDataException($"The selection contains more than {MaximumInventoryEntries:N0} entries.");
                var childAttributes = File.GetAttributes(child);
                if (childAttributes.HasFlag(FileAttributes.ReparsePoint))
                    throw new InvalidDataException($"A reparse point was found inside the selection: {child}");
                var relative = Path.GetRelativePath(root, child);
                if (childAttributes.HasFlag(FileAttributes.Directory))
                {
                    var info = new DirectoryInfo(child);
                    directories++;
                    Xor(aggregate, HashEntry("D", relative, 0, info.LastWriteTimeUtc.Ticks, childAttributes));
                    pending.Push(child);
                }
                else
                {
                    var info = new FileInfo(child);
                    bytes = checked(bytes + info.Length);
                    files++;
                    Xor(aggregate, HashEntry("F", relative, info.Length, info.LastWriteTimeUtc.Ticks, childAttributes));
                }
            }
        }
        return new LiveInventory(true, bytes, files, directories, rootInfo.LastWriteTimeUtc.Ticks,
            Convert.ToHexString(aggregate));
    }

    private static byte[] HashEntry(string type, string relativePath, long size, long writeTicks,
        FileAttributes attributes) => SHA256.HashData(Encoding.UTF8.GetBytes(
        $"{type}\0{relativePath.ToUpperInvariant()}\0{size}\0{writeTicks}\0{(int)attributes}"));

    private static void Xor(byte[] aggregate, byte[] value)
    {
        for (var index = 0; index < aggregate.Length; index++) aggregate[index] ^= value[index];
    }

    private static StorageNode FindRoot(StorageTree tree, StorageNode node)
    {
        var current = node;
        var seen = 0;
        while (current.ParentIndex >= 0)
        {
            if (++seen > tree.Nodes.Count) throw new InvalidDataException("The storage tree parent chain is invalid.");
            if (current.ParentIndex >= tree.Nodes.Count)
                throw new InvalidDataException("The storage tree parent chain points outside the current map.");
            current = tree.Nodes[current.ParentIndex];
        }
        return current;
    }

    private static bool IsStrictlyUnder(string path, string root)
    {
        var fullPath = Path.GetFullPath(path).TrimEnd(Path.DirectorySeparatorChar);
        var fullRoot = Path.GetFullPath(root).TrimEnd(Path.DirectorySeparatorChar);
        return !fullPath.Equals(fullRoot, StringComparison.OrdinalIgnoreCase) &&
            fullPath.StartsWith(fullRoot + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase);
    }

    private void PruneTickets()
    {
        foreach (var ticket in _tickets.Values.Where(ticket => ticket.ExpiresUtc <= DateTime.UtcNow))
            _tickets.TryRemove(ticket.Id, out _);
    }

    private static StorageSelectionDeletePreflight Blocked(string reason, string path = "") =>
        new(false, "Deletion preflight blocked.", reason, string.Empty, path, false, 0, 0, 0, default);

    private static StorageSelectionDeleteResult Failed(string reason, string path = "") =>
        new(false, reason, path, []);

    private sealed record DeleteTicket(string Id, string RootPath, string TargetPath,
        LiveInventory Inventory, DateTime ExpiresUtc);

    private sealed record LiveInventory(bool IsDirectory, long SizeBytes, int FileCount,
        int DirectoryCount, long RootLastWriteTicks, string Fingerprint);
}
