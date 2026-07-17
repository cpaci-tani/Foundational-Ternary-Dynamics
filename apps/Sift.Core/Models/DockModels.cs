namespace Sift.Models;

public enum DockSplitOrientation
{
    Horizontal,
    Vertical
}

public enum DockDropZone
{
    Center,
    Tab,
    Left,
    Right,
    Top,
    Bottom,
    Float
}

public static class DockContentTypes
{
    public const string HardwareSensor = "hardware.sensor";
}

public static class DockShellIds
{
    public const string HardwareSensors = "hardware.sensors";
}

public sealed class DockTile
{
    public string InstanceId { get; set; } = string.Empty;
    public string ContentType { get; set; } = string.Empty;
    public string ContentKey { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public Dictionary<string, string> Metadata { get; set; } = new(StringComparer.OrdinalIgnoreCase);
    public int Row { get; set; }
    public int Column { get; set; }
    public int RowSpan { get; set; } = 2;
    public int ColumnSpan { get; set; } = 2;

    public DockTile Copy() => new()
    {
        InstanceId = InstanceId,
        ContentType = ContentType,
        ContentKey = ContentKey,
        Title = Title,
        Metadata = new Dictionary<string, string>(Metadata, StringComparer.OrdinalIgnoreCase),
        Row = Row,
        Column = Column,
        RowSpan = RowSpan,
        ColumnSpan = ColumnSpan
    };
}

public abstract class DockNode
{
    public string Id { get; set; } = string.Empty;
    public abstract DockNode Copy();
}

public sealed class DockBoardNode : DockNode
{
    public string Title { get; set; } = "Board";
    public int Columns { get; set; } = 4;
    public List<DockTile> Tiles { get; set; } = [];

    public override DockNode Copy() => new DockBoardNode
    {
        Id = Id,
        Title = Title,
        Columns = Columns,
        Tiles = Tiles.Select(tile => tile.Copy()).ToList()
    };
}

public sealed class DockTabGroupNode : DockNode
{
    public int ActiveIndex { get; set; }
    public List<DockNode> Tabs { get; set; } = [];

    public override DockNode Copy() => new DockTabGroupNode
    {
        Id = Id,
        ActiveIndex = ActiveIndex,
        Tabs = Tabs.Select(tab => tab.Copy()).ToList()
    };
}

public sealed class DockSplitNode : DockNode
{
    public DockSplitOrientation Orientation { get; set; } = DockSplitOrientation.Horizontal;
    public double Ratio { get; set; } = 0.5;
    public DockNode First { get; set; } = new DockBoardNode { Id = Guid.NewGuid().ToString("N") };
    public DockNode Second { get; set; } = new DockBoardNode { Id = Guid.NewGuid().ToString("N") };

    public override DockNode Copy() => new DockSplitNode
    {
        Id = Id,
        Orientation = Orientation,
        Ratio = Ratio,
        First = First.Copy(),
        Second = Second.Copy()
    };
}

public sealed class FloatingDockSite
{
    public string Id { get; set; } = string.Empty;
    public double X { get; set; } = 120;
    public double Y { get; set; } = 120;
    public double Width { get; set; } = 960;
    public double Height { get; set; } = 640;
    public DockNode Root { get; set; } = new DockTabGroupNode
    {
        Id = Guid.NewGuid().ToString("N"),
        Tabs = [new DockBoardNode { Id = Guid.NewGuid().ToString("N"), Title = "Board" }]
    };

    public FloatingDockSite Copy() => new()
    {
        Id = Id,
        X = X,
        Y = Y,
        Width = Width,
        Height = Height,
        Root = Root.Copy()
    };
}

public sealed class DockLayoutDocument
{
    public const int CurrentSchemaVersion = 1;
    public const int DefaultMaximumTiles = 32;

    public int SchemaVersion { get; set; } = CurrentSchemaVersion;
    public string ShellId { get; set; } = string.Empty;
    public int MaximumTiles { get; set; } = DefaultMaximumTiles;
    public DockNode EmbeddedRoot { get; set; } = CreateDefaultEmbedded();
    public List<FloatingDockSite> FloatingSites { get; set; } = [];
    public string? ActiveBoardId { get; set; }

    public DockLayoutDocument Copy() => new()
    {
        SchemaVersion = SchemaVersion,
        ShellId = ShellId,
        MaximumTiles = MaximumTiles,
        EmbeddedRoot = EmbeddedRoot.Copy(),
        FloatingSites = FloatingSites.Select(site => site.Copy()).ToList(),
        ActiveBoardId = ActiveBoardId
    };

    public static DockLayoutDocument Create(string shellId, string defaultBoardTitle = "Board", int maximumTiles = DefaultMaximumTiles)
    {
        var document = new DockLayoutDocument
        {
            ShellId = shellId,
            MaximumTiles = maximumTiles,
            EmbeddedRoot = CreateDefaultEmbedded(defaultBoardTitle)
        };
        document.ActiveBoardId = DockLayoutEngineIds.DefaultBoardId;
        return document;
    }

    public static DockNode CreateDefaultEmbedded(string boardTitle = "Board")
    {
        var board = new DockBoardNode
        {
            Id = DockLayoutEngineIds.DefaultBoardId,
            Title = boardTitle
        };
        return new DockTabGroupNode
        {
            Id = DockLayoutEngineIds.DefaultTabsId,
            ActiveIndex = 0,
            Tabs = [board]
        };
    }
}

internal static class DockLayoutEngineIds
{
    public const string DefaultBoardId = "board.main";
    public const string DefaultTabsId = "tabs.main";
}
