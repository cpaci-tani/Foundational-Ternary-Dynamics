using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using Sift.Infrastructure.Persistence;
using Sift.Models;

namespace Sift.Services;

public interface IDockLayoutStore
{
    string ShellId { get; }
    string LayoutPath { get; }
    DockLayoutDocument LoadOrCreate();
    void Save(DockLayoutDocument document);
}

public sealed class DockLayoutStore : IDockLayoutStore
{
    public const int MaximumDocumentBytes = 1024 * 1024;
    public const string LegacyHardwareLayoutFileName = "sensor-graph-layout.json";

    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNameCaseInsensitive = false,
        UnmappedMemberHandling = JsonUnmappedMemberHandling.Disallow,
        Converters = { new DockNodeJsonConverter() }
    };

    private readonly string _directory;
    private readonly string _defaultBoardTitle;
    private readonly int _maximumTiles;

    public DockLayoutStore(
        string shellId,
        string? directory = null,
        string defaultBoardTitle = "Board",
        int maximumTiles = DockLayoutDocument.DefaultMaximumTiles)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(shellId);
        ShellId = shellId.Trim();
        _directory = directory ?? ProductPaths.DataRoot;
        _defaultBoardTitle = defaultBoardTitle;
        _maximumTiles = maximumTiles;
        LayoutPath = Path.Combine(_directory, $"dock-{SanitizeFileName(ShellId)}.json");
    }

    public string ShellId { get; }
    public string LayoutPath { get; }

    public DockLayoutDocument LoadOrCreate()
    {
        if (File.Exists(LayoutPath))
            return LoadExisting(LayoutPath) ?? CreateDefault();

        if (ShellId.Equals(DockShellIds.HardwareSensors, StringComparison.OrdinalIgnoreCase))
        {
            var legacyPath = Path.Combine(_directory, LegacyHardwareLayoutFileName);
            if (File.Exists(legacyPath) && TryMigrateLegacyHardware(legacyPath, out var migrated))
            {
                Save(migrated);
                QuarantineFile(legacyPath, "sensor-graph-layout.migrated");
                return migrated;
            }
        }

        var created = CreateDefault();
        Save(created);
        return created;
    }

    public void Save(DockLayoutDocument document)
    {
        ArgumentNullException.ThrowIfNull(document);
        document.ShellId = ShellId;
        document.MaximumTiles = Math.Max(1, document.MaximumTiles);
        var errors = DockLayoutEngine.Validate(document);
        if (errors.Count > 0)
            throw new InvalidDataException(string.Join(" ", errors));
        var json = JsonSerializer.Serialize(document, JsonOptions);
        if (Encoding.UTF8.GetByteCount(json) > MaximumDocumentBytes)
            throw new InvalidDataException("The dock layout document is too large.");
        AtomicFile.WriteAllText(LayoutPath, json);
    }

    private DockLayoutDocument CreateDefault()
    {
        var created = DockLayoutDocument.Create(ShellId, _defaultBoardTitle, _maximumTiles);
        created.ActiveBoardId = DockLayoutEngine.ResolveActiveBoard(created).Id;
        return created;
    }

    private DockLayoutDocument? LoadExisting(string path)
    {
        try
        {
            if (new FileInfo(path).Length > MaximumDocumentBytes)
                throw new InvalidDataException("The dock layout document is too large.");
            var document = JsonSerializer.Deserialize<DockLayoutDocument>(File.ReadAllText(path), JsonOptions)
                ?? throw new InvalidDataException("The dock layout document is empty.");
            document.ShellId = ShellId;
            if (document.MaximumTiles < 1) document.MaximumTiles = _maximumTiles;
            var errors = DockLayoutEngine.Validate(document);
            if (errors.Count > 0)
                throw new InvalidDataException(string.Join(" ", errors));
            document.ActiveBoardId ??= DockLayoutEngine.ResolveActiveBoard(document).Id;
            return document;
        }
        catch (Exception exception) when (exception is JsonException or InvalidDataException or IOException or ArgumentException or NotSupportedException)
        {
            QuarantineFile(path, "dock.corrupt");
            return null;
        }
    }

    private bool TryMigrateLegacyHardware(string legacyPath, out DockLayoutDocument document)
    {
        document = CreateDefault();
        try
        {
            if (new FileInfo(legacyPath).Length > MaximumDocumentBytes) return false;
            using var json = JsonDocument.Parse(File.ReadAllText(legacyPath));
            var root = json.RootElement;
            if (!root.TryGetProperty("EmbeddedRoot", out var embedded) &&
                !root.TryGetProperty("embeddedRoot", out embedded))
                return false;

            document.EmbeddedRoot = MigrateNode(embedded) ?? DockLayoutDocument.CreateDefaultEmbedded(_defaultBoardTitle);
            document.FloatingSites = [];
            if (root.TryGetProperty("FloatingSites", out var floats) ||
                root.TryGetProperty("floatingSites", out floats))
            {
                foreach (var siteElement in floats.EnumerateArray())
                {
                    var site = new FloatingDockSite
                    {
                        Id = ReadString(siteElement, "Id", "id") ?? Guid.NewGuid().ToString("N"),
                        X = ReadDouble(siteElement, "X", "x") ?? 120,
                        Y = ReadDouble(siteElement, "Y", "y") ?? 120,
                        Width = ReadDouble(siteElement, "Width", "width") ?? 960,
                        Height = ReadDouble(siteElement, "Height", "height") ?? 640,
                        Root = siteElement.TryGetProperty("Root", out var siteRoot) ||
                               siteElement.TryGetProperty("root", out siteRoot)
                            ? MigrateNode(siteRoot) ?? DockLayoutDocument.CreateDefaultEmbedded(_defaultBoardTitle)
                            : DockLayoutDocument.CreateDefaultEmbedded(_defaultBoardTitle)
                    };
                    document.FloatingSites.Add(site);
                }
            }

            document.ActiveBoardId = ReadString(root, "ActiveBoardId", "activeBoardId");
            document.ActiveBoardId ??= DockLayoutEngine.ResolveActiveBoard(document).Id;
            var errors = DockLayoutEngine.Validate(document);
            return errors.Count == 0;
        }
        catch
        {
            return false;
        }
    }

    private static DockNode? MigrateNode(JsonElement element)
    {
        var kind = ReadString(element, "kind", "Kind");
        return kind switch
        {
            "board" => new DockBoardNode
            {
                Id = ReadString(element, "id", "Id") ?? Guid.NewGuid().ToString("N"),
                Title = ReadString(element, "title", "Title") ?? "Board",
                Columns = (int)(ReadDouble(element, "columns", "Columns") ?? 4),
                Tiles = MigrateTiles(element)
            },
            "tabs" => new DockTabGroupNode
            {
                Id = ReadString(element, "id", "Id") ?? Guid.NewGuid().ToString("N"),
                ActiveIndex = (int)(ReadDouble(element, "activeIndex", "ActiveIndex") ?? 0),
                Tabs = ReadArray(element, "tabs", "Tabs")
                    .Select(MigrateNode)
                    .Where(node => node is not null)
                    .Cast<DockNode>()
                    .ToList()
            },
            "split" => new DockSplitNode
            {
                Id = ReadString(element, "id", "Id") ?? Guid.NewGuid().ToString("N"),
                Orientation = Enum.TryParse<DockSplitOrientation>(
                    ReadString(element, "orientation", "Orientation"), true, out var orientation)
                    ? orientation
                    : DockSplitOrientation.Horizontal,
                Ratio = ReadDouble(element, "ratio", "Ratio") ?? 0.5,
                First = element.TryGetProperty("first", out var first) || element.TryGetProperty("First", out first)
                    ? MigrateNode(first) ?? new DockBoardNode { Id = Guid.NewGuid().ToString("N") }
                    : new DockBoardNode { Id = Guid.NewGuid().ToString("N") },
                Second = element.TryGetProperty("second", out var second) || element.TryGetProperty("Second", out second)
                    ? MigrateNode(second) ?? new DockBoardNode { Id = Guid.NewGuid().ToString("N") }
                    : new DockBoardNode { Id = Guid.NewGuid().ToString("N") }
            },
            _ => null
        };
    }

    private static List<DockTile> MigrateTiles(JsonElement board)
    {
        var tiles = new List<DockTile>();
        foreach (var tile in ReadArray(board, "tiles", "Tiles"))
        {
            var sensorId = ReadString(tile, "SensorId", "sensorId") ?? ReadString(tile, "ContentKey", "contentKey");
            if (string.IsNullOrWhiteSpace(sensorId)) continue;
            var unit = ReadString(tile, "Unit", "unit");
            var metadata = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            if (!string.IsNullOrWhiteSpace(unit)) metadata["unit"] = unit;
            tiles.Add(new DockTile
            {
                InstanceId = ReadString(tile, "InstanceId", "instanceId") ?? Guid.NewGuid().ToString("N"),
                ContentType = DockContentTypes.HardwareSensor,
                ContentKey = sensorId,
                Title = ReadString(tile, "Title", "title") ?? sensorId,
                Metadata = metadata,
                Row = (int)(ReadDouble(tile, "Row", "row") ?? 0),
                Column = (int)(ReadDouble(tile, "Column", "column") ?? 0),
                RowSpan = (int)(ReadDouble(tile, "RowSpan", "rowSpan") ?? 2),
                ColumnSpan = (int)(ReadDouble(tile, "ColumnSpan", "columnSpan") ?? 2)
            });
        }
        return tiles;
    }

    private static IEnumerable<JsonElement> ReadArray(JsonElement element, params string[] names)
    {
        foreach (var name in names)
        {
            if (!element.TryGetProperty(name, out var array) || array.ValueKind != JsonValueKind.Array)
                continue;
            foreach (var child in array.EnumerateArray()) yield return child;
            yield break;
        }
    }

    private static string? ReadString(JsonElement element, params string[] names)
    {
        foreach (var name in names)
        {
            if (element.TryGetProperty(name, out var value) && value.ValueKind == JsonValueKind.String)
                return value.GetString();
        }
        return null;
    }

    private static double? ReadDouble(JsonElement element, params string[] names)
    {
        foreach (var name in names)
        {
            if (!element.TryGetProperty(name, out var value)) continue;
            if (value.ValueKind == JsonValueKind.Number && value.TryGetDouble(out var number)) return number;
        }
        return null;
    }

    private static void QuarantineFile(string path, string prefix)
    {
        try
        {
            if (!File.Exists(path)) return;
            var stamp = DateTime.UtcNow.ToString("yyyyMMddHHmmss");
            File.Move(path, Path.Combine(Path.GetDirectoryName(path)!, $"{prefix}-{stamp}.json"));
        }
        catch (IOException) { }
    }

    private static string SanitizeFileName(string shellId)
    {
        var invalid = Path.GetInvalidFileNameChars();
        return new string(shellId.Select(ch => invalid.Contains(ch) ? '-' : ch).ToArray());
    }
}

internal sealed class DockNodeJsonConverter : JsonConverter<DockNode>
{
    public override DockNode Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        using var document = JsonDocument.ParseValue(ref reader);
        var root = document.RootElement;
        var kind = root.TryGetProperty("kind", out var kindElement) ? kindElement.GetString() : null;
        return kind switch
        {
            "board" => new DockBoardNode
            {
                Id = root.GetProperty("id").GetString() ?? Guid.NewGuid().ToString("N"),
                Title = root.TryGetProperty("title", out var title) ? title.GetString() ?? "Board" : "Board",
                Columns = root.TryGetProperty("columns", out var columns) ? columns.GetInt32() : 4,
                Tiles = root.TryGetProperty("tiles", out var tiles)
                    ? tiles.Deserialize<List<DockTile>>(options) ?? []
                    : []
            },
            "tabs" => new DockTabGroupNode
            {
                Id = root.GetProperty("id").GetString() ?? Guid.NewGuid().ToString("N"),
                ActiveIndex = root.TryGetProperty("activeIndex", out var active) ? active.GetInt32() : 0,
                Tabs = root.TryGetProperty("tabs", out var tabs)
                    ? tabs.EnumerateArray().Select(element =>
                        JsonSerializer.Deserialize<DockNode>(element.GetRawText(), options)
                        ?? new DockBoardNode { Id = Guid.NewGuid().ToString("N") }).ToList()
                    : []
            },
            "split" => new DockSplitNode
            {
                Id = root.GetProperty("id").GetString() ?? Guid.NewGuid().ToString("N"),
                Orientation = root.TryGetProperty("orientation", out var orientation) &&
                              Enum.TryParse<DockSplitOrientation>(orientation.GetString(), true, out var parsed)
                    ? parsed
                    : DockSplitOrientation.Horizontal,
                Ratio = root.TryGetProperty("ratio", out var ratio) ? ratio.GetDouble() : 0.5,
                First = JsonSerializer.Deserialize<DockNode>(root.GetProperty("first").GetRawText(), options)
                    ?? new DockBoardNode { Id = Guid.NewGuid().ToString("N") },
                Second = JsonSerializer.Deserialize<DockNode>(root.GetProperty("second").GetRawText(), options)
                    ?? new DockBoardNode { Id = Guid.NewGuid().ToString("N") }
            },
            _ => throw new JsonException($"Unknown dock node kind '{kind}'.")
        };
    }

    public override void Write(Utf8JsonWriter writer, DockNode value, JsonSerializerOptions options)
    {
        writer.WriteStartObject();
        switch (value)
        {
            case DockBoardNode board:
                writer.WriteString("kind", "board");
                writer.WriteString("id", board.Id);
                writer.WriteString("title", board.Title);
                writer.WriteNumber("columns", board.Columns);
                writer.WritePropertyName("tiles");
                JsonSerializer.Serialize(writer, board.Tiles, options);
                break;
            case DockTabGroupNode tabs:
                writer.WriteString("kind", "tabs");
                writer.WriteString("id", tabs.Id);
                writer.WriteNumber("activeIndex", tabs.ActiveIndex);
                writer.WritePropertyName("tabs");
                JsonSerializer.Serialize(writer, tabs.Tabs, options);
                break;
            case DockSplitNode split:
                writer.WriteString("kind", "split");
                writer.WriteString("id", split.Id);
                writer.WriteString("orientation", split.Orientation.ToString());
                writer.WriteNumber("ratio", split.Ratio);
                writer.WritePropertyName("first");
                JsonSerializer.Serialize(writer, split.First, options);
                writer.WritePropertyName("second");
                JsonSerializer.Serialize(writer, split.Second, options);
                break;
            default:
                throw new NotSupportedException($"Unsupported dock node type {value.GetType().Name}.");
        }
        writer.WriteEndObject();
    }
}
