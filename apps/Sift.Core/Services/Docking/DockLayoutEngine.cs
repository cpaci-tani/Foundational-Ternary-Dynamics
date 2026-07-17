using Sift.Models;

namespace Sift.Services;

public static class DockLayoutEngine
{
    public const int MaximumSplitDepth = 3;

    public static int CountTiles(DockLayoutDocument document)
    {
        ArgumentNullException.ThrowIfNull(document);
        return EnumerateBoards(document).Sum(board => board.Tiles.Count);
    }

    public static IEnumerable<DockBoardNode> EnumerateBoards(DockLayoutDocument document)
    {
        ArgumentNullException.ThrowIfNull(document);
        foreach (var board in EnumerateBoards(document.EmbeddedRoot))
            yield return board;
        foreach (var site in document.FloatingSites)
        foreach (var board in EnumerateBoards(site.Root))
            yield return board;
    }

    public static IEnumerable<DockBoardNode> EnumerateBoards(DockNode node) => node switch
    {
        DockBoardNode board => [board],
        DockTabGroupNode tabs => tabs.Tabs.SelectMany(EnumerateBoards),
        DockSplitNode split => EnumerateBoards(split.First).Concat(EnumerateBoards(split.Second)),
        _ => []
    };

    public static DockBoardNode? FindBoard(DockLayoutDocument document, string boardId)
    {
        ArgumentNullException.ThrowIfNull(document);
        ArgumentException.ThrowIfNullOrWhiteSpace(boardId);
        return EnumerateBoards(document)
            .FirstOrDefault(board => board.Id.Equals(boardId, StringComparison.OrdinalIgnoreCase));
    }

    public static DockBoardNode ResolveActiveBoard(DockLayoutDocument document)
    {
        ArgumentNullException.ThrowIfNull(document);
        if (!string.IsNullOrWhiteSpace(document.ActiveBoardId) &&
            FindBoard(document, document.ActiveBoardId) is { } active)
            return active;

        var first = EnumerateBoards(document).FirstOrDefault()
            ?? throw new InvalidOperationException("The dock layout has no boards.");
        document.ActiveBoardId = first.Id;
        return first;
    }

    public static bool TryAddTile(
        DockLayoutDocument document,
        string contentType,
        string contentKey,
        string title,
        IReadOnlyDictionary<string, string>? metadata,
        out DockTile? tile,
        out string? error)
    {
        ArgumentNullException.ThrowIfNull(document);
        ArgumentException.ThrowIfNullOrWhiteSpace(contentType);
        ArgumentException.ThrowIfNullOrWhiteSpace(contentKey);
        tile = null;
        error = null;

        var max = Math.Max(1, document.MaximumTiles);
        if (CountTiles(document) >= max)
        {
            error = $"At most {max} dock tiles can be open.";
            return false;
        }

        var board = ResolveActiveBoard(document);
        if (board.Tiles.Any(existing =>
                existing.ContentType.Equals(contentType, StringComparison.OrdinalIgnoreCase) &&
                existing.ContentKey.Equals(contentKey, StringComparison.OrdinalIgnoreCase)))
        {
            error = "That content is already open on the active board.";
            return false;
        }

        tile = new DockTile
        {
            InstanceId = Guid.NewGuid().ToString("N"),
            ContentType = contentType.Trim(),
            ContentKey = contentKey.Trim(),
            Title = string.IsNullOrWhiteSpace(title) ? contentKey : title.Trim(),
            Metadata = metadata is null
                ? new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
                : new Dictionary<string, string>(metadata, StringComparer.OrdinalIgnoreCase),
            RowSpan = 2,
            ColumnSpan = Math.Min(2, board.Columns)
        };

        var layout = ToBreakpointLayout(board);
        layout.Placements.Add(new DashboardPlacement
        {
            InstanceId = tile.InstanceId,
            Row = 0,
            Column = 0,
            RowSpan = tile.RowSpan,
            ColumnSpan = tile.ColumnSpan,
            Visible = true
        });
        var tidied = DashboardPackingEngine.Tidy(layout);
        ApplyBreakpointLayout(board, tidied, board.Tiles.Append(tile).ToList());
        document.ActiveBoardId = board.Id;
        return true;
    }

    public static bool RemoveTile(DockLayoutDocument document, string instanceId)
    {
        ArgumentNullException.ThrowIfNull(document);
        ArgumentException.ThrowIfNullOrWhiteSpace(instanceId);
        foreach (var board in EnumerateBoards(document))
        {
            var index = board.Tiles.FindIndex(tile =>
                tile.InstanceId.Equals(instanceId, StringComparison.OrdinalIgnoreCase));
            if (index < 0) continue;
            board.Tiles.RemoveAt(index);
            return true;
        }
        return false;
    }

    public static bool PlaceTile(
        DockLayoutDocument document,
        string boardId,
        string instanceId,
        int row,
        int column,
        int rowSpan,
        int columnSpan)
    {
        var board = FindBoard(document, boardId) ?? throw new KeyNotFoundException($"Board '{boardId}' was not found.");
        if (board.Tiles.All(tile => !tile.InstanceId.Equals(instanceId, StringComparison.OrdinalIgnoreCase)))
            throw new KeyNotFoundException($"Tile '{instanceId}' was not found on board '{boardId}'.");

        var layout = ToBreakpointLayout(board);
        var placed = DashboardPackingEngine.Place(layout, instanceId, row, column, rowSpan, columnSpan);
        ApplyBreakpointLayout(board, placed, board.Tiles);
        return true;
    }

    public static void TidyBoard(DockLayoutDocument document, string boardId)
    {
        var board = FindBoard(document, boardId) ?? throw new KeyNotFoundException($"Board '{boardId}' was not found.");
        var tidied = DashboardPackingEngine.Tidy(ToBreakpointLayout(board));
        ApplyBreakpointLayout(board, tidied, board.Tiles);
    }

    public static FloatingDockSite PopOutBoard(DockLayoutDocument document, string boardId)
    {
        ArgumentNullException.ThrowIfNull(document);
        var board = FindBoard(document, boardId)
            ?? throw new KeyNotFoundException($"Board '{boardId}' was not found.");
        if (!TryDetachNode(document, boardId, out var detached) || detached is null)
            throw new InvalidOperationException("The board could not be detached for pop-out.");

        var site = new FloatingDockSite
        {
            Id = Guid.NewGuid().ToString("N"),
            Root = new DockTabGroupNode
            {
                Id = Guid.NewGuid().ToString("N"),
                ActiveIndex = 0,
                Tabs = [detached]
            }
        };
        document.FloatingSites.Add(site);
        document.ActiveBoardId = board.Id;
        EnsureEmbeddedHasBoard(document);
        return site;
    }

    public static void RedockFloatingSite(DockLayoutDocument document, string floatingSiteId, DockDropZone zone)
    {
        ArgumentNullException.ThrowIfNull(document);
        var index = document.FloatingSites.FindIndex(site =>
            site.Id.Equals(floatingSiteId, StringComparison.OrdinalIgnoreCase));
        if (index < 0) throw new KeyNotFoundException($"Floating site '{floatingSiteId}' was not found.");

        var site = document.FloatingSites[index];
        document.FloatingSites.RemoveAt(index);
        document.EmbeddedRoot = Attach(document.EmbeddedRoot, site.Root, zone);
        if (EnumerateBoards(site.Root).FirstOrDefault() is { } board)
            document.ActiveBoardId = board.Id;
    }

    public static DockNode Attach(DockNode host, DockNode incoming, DockDropZone zone)
    {
        ArgumentNullException.ThrowIfNull(host);
        ArgumentNullException.ThrowIfNull(incoming);
        return zone switch
        {
            DockDropZone.Tab or DockDropZone.Center => AttachAsTab(host, incoming),
            DockDropZone.Left => Split(incoming, host, DockSplitOrientation.Horizontal, 0.5),
            DockDropZone.Right => Split(host, incoming, DockSplitOrientation.Horizontal, 0.5),
            DockDropZone.Top => Split(incoming, host, DockSplitOrientation.Vertical, 0.5),
            DockDropZone.Bottom => Split(host, incoming, DockSplitOrientation.Vertical, 0.5),
            DockDropZone.Float => throw new InvalidOperationException("Float is handled by PopOutBoard."),
            _ => AttachAsTab(host, incoming)
        };
    }

    public static IReadOnlyList<string> Validate(DockLayoutDocument document)
    {
        ArgumentNullException.ThrowIfNull(document);
        var errors = new List<string>();
        if (document.SchemaVersion != DockLayoutDocument.CurrentSchemaVersion)
            errors.Add($"Unsupported schema version {document.SchemaVersion}.");
        if (string.IsNullOrWhiteSpace(document.ShellId))
            errors.Add("Dock layout is missing ShellId.");
        if (document.MaximumTiles < 1)
            errors.Add("MaximumTiles must be at least 1.");

        var tileIds = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        var boardIds = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        var count = 0;
        foreach (var board in EnumerateBoards(document))
        {
            if (!boardIds.Add(board.Id))
                errors.Add($"Duplicate board id '{board.Id}'.");
            if (board.Columns is < 1 or > 6)
                errors.Add($"Board '{board.Id}' columns must be 1–6.");
            foreach (var tile in board.Tiles)
            {
                count++;
                if (!tileIds.Add(tile.InstanceId))
                    errors.Add($"Duplicate tile id '{tile.InstanceId}'.");
                if (string.IsNullOrWhiteSpace(tile.ContentType) || string.IsNullOrWhiteSpace(tile.ContentKey))
                    errors.Add($"Tile '{tile.InstanceId}' is missing content type or key.");
                if (tile.RowSpan is < 1 or > 12 || tile.ColumnSpan < 1 || tile.ColumnSpan > board.Columns)
                    errors.Add($"Tile '{tile.InstanceId}' has an invalid span.");
            }
        }
        if (count > document.MaximumTiles)
            errors.Add($"Layout exceeds the {document.MaximumTiles}-tile cap.");
        if (SplitDepth(document.EmbeddedRoot) > MaximumSplitDepth)
            errors.Add($"Embedded dock exceeds split depth {MaximumSplitDepth}.");
        foreach (var site in document.FloatingSites)
        {
            if (SplitDepth(site.Root) > MaximumSplitDepth)
                errors.Add($"Floating site '{site.Id}' exceeds split depth {MaximumSplitDepth}.");
            if (site.Width < 320 || site.Height < 240)
                errors.Add($"Floating site '{site.Id}' is too small.");
        }
        return errors;
    }

    private static DockNode AttachAsTab(DockNode host, DockNode incoming)
    {
        if (host is DockTabGroupNode tabs)
        {
            tabs.Tabs.Add(UnwrapToTabContent(incoming));
            tabs.ActiveIndex = tabs.Tabs.Count - 1;
            return tabs;
        }

        return new DockTabGroupNode
        {
            Id = Guid.NewGuid().ToString("N"),
            ActiveIndex = 1,
            Tabs = [host, UnwrapToTabContent(incoming)]
        };
    }

    private static DockNode UnwrapToTabContent(DockNode node) => node switch
    {
        DockTabGroupNode tabs when tabs.Tabs.Count == 1 => tabs.Tabs[0],
        _ => node
    };

    private static DockSplitNode Split(DockNode first, DockNode second, DockSplitOrientation orientation, double ratio) =>
        new()
        {
            Id = Guid.NewGuid().ToString("N"),
            Orientation = orientation,
            Ratio = Math.Clamp(ratio, 0.2, 0.8),
            First = first,
            Second = second
        };

    private static int SplitDepth(DockNode node) => node switch
    {
        DockSplitNode split => 1 + Math.Max(SplitDepth(split.First), SplitDepth(split.Second)),
        DockTabGroupNode tabs => tabs.Tabs.Count == 0 ? 0 : tabs.Tabs.Max(SplitDepth),
        _ => 0
    };

    private static bool TryDetachNode(DockLayoutDocument document, string nodeId, out DockNode? detached)
    {
        detached = null;
        var embedded = document.EmbeddedRoot;
        if (TryDetach(ref embedded!, nodeId, out detached))
        {
            document.EmbeddedRoot = embedded ?? DockLayoutDocument.CreateDefaultEmbedded();
            return detached is not null;
        }
        for (var index = 0; index < document.FloatingSites.Count; index++)
        {
            var root = document.FloatingSites[index].Root;
            if (!TryDetach(ref root, nodeId, out detached)) continue;
            document.FloatingSites[index].Root = root ?? DockLayoutDocument.CreateDefaultEmbedded();
            return detached is not null;
        }
        return false;
    }

    private static bool TryDetach(ref DockNode? node, string nodeId, out DockNode? detached)
    {
        detached = null;
        if (node is null) return false;
        if (node.Id.Equals(nodeId, StringComparison.OrdinalIgnoreCase))
        {
            detached = node;
            node = null;
            return true;
        }

        switch (node)
        {
            case DockTabGroupNode tabs:
                for (var index = 0; index < tabs.Tabs.Count; index++)
                {
                    var child = tabs.Tabs[index];
                    if (TryDetach(ref child!, nodeId, out detached))
                    {
                        if (child is null) tabs.Tabs.RemoveAt(index);
                        else tabs.Tabs[index] = child;
                        tabs.ActiveIndex = Math.Clamp(tabs.ActiveIndex, 0, Math.Max(0, tabs.Tabs.Count - 1));
                        if (tabs.Tabs.Count == 0) node = new DockBoardNode
                        {
                            Id = Guid.NewGuid().ToString("N"),
                            Title = "Board"
                        };
                        return true;
                    }
                }
                break;
            case DockSplitNode split:
            {
                var first = split.First;
                var second = split.Second;
                if (TryDetach(ref first!, nodeId, out detached))
                {
                    node = first is null ? second : new DockSplitNode
                    {
                        Id = split.Id,
                        Orientation = split.Orientation,
                        Ratio = split.Ratio,
                        First = first,
                        Second = second
                    };
                    return true;
                }
                if (TryDetach(ref second!, nodeId, out detached))
                {
                    node = second is null ? first : new DockSplitNode
                    {
                        Id = split.Id,
                        Orientation = split.Orientation,
                        Ratio = split.Ratio,
                        First = first,
                        Second = second
                    };
                    return true;
                }
                break;
            }
        }
        return false;
    }

    private static void EnsureEmbeddedHasBoard(DockLayoutDocument document)
    {
        if (EnumerateBoards(document.EmbeddedRoot).Any()) return;
        document.EmbeddedRoot = DockLayoutDocument.CreateDefaultEmbedded();
        document.ActiveBoardId = EnumerateBoards(document.EmbeddedRoot).First().Id;
    }

    private static DashboardBreakpointLayout ToBreakpointLayout(DockBoardNode board) => new()
    {
        Breakpoint = DashboardBreakpoint.Wide,
        Columns = board.Columns,
        Placements = board.Tiles.Select(tile => new DashboardPlacement
        {
            InstanceId = tile.InstanceId,
            Row = tile.Row,
            Column = tile.Column,
            RowSpan = tile.RowSpan,
            ColumnSpan = tile.ColumnSpan,
            Visible = true
        }).ToList()
    };

    private static void ApplyBreakpointLayout(
        DockBoardNode board,
        DashboardBreakpointLayout layout,
        IReadOnlyList<DockTile> tiles)
    {
        board.Columns = layout.Columns;
        var byId = tiles.ToDictionary(tile => tile.InstanceId, StringComparer.OrdinalIgnoreCase);
        board.Tiles = layout.Placements
            .Where(placement => byId.ContainsKey(placement.InstanceId))
            .Select(placement =>
            {
                var tile = byId[placement.InstanceId].Copy();
                tile.Row = placement.Row;
                tile.Column = placement.Column;
                tile.RowSpan = placement.RowSpan;
                tile.ColumnSpan = placement.ColumnSpan;
                return tile;
            })
            .ToList();
    }
}
