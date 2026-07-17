using System.Text.Json;
using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class DockLayoutTests
{
    [Fact]
    public void TryAddTile_respects_maximum_tiles()
    {
        var document = DockLayoutDocument.Create(DockShellIds.HardwareSensors, "Hardware graphs");
        for (var index = 0; index < document.MaximumTiles; index++)
            Assert.True(DockLayoutEngine.TryAddTile(
                document, DockContentTypes.HardwareSensor, $"sensor.{index}", $"Sensor {index}",
                new Dictionary<string, string> { ["unit"] = "°C" }, out _, out _));

        Assert.False(DockLayoutEngine.TryAddTile(
            document, DockContentTypes.HardwareSensor, "sensor.overflow", "Overflow", null, out _, out var error));
        Assert.Contains("32", error);
        Assert.Equal(32, DockLayoutEngine.CountTiles(document));
    }

    [Fact]
    public void TryAddTile_stores_hardware_metadata()
    {
        var document = DockLayoutDocument.Create(DockShellIds.HardwareSensors);
        Assert.True(DockLayoutEngine.TryAddTile(
            document, DockContentTypes.HardwareSensor, "cpu.temp", "CPU",
            new Dictionary<string, string> { ["unit"] = "°C" }, out var tile, out _));
        Assert.Equal(DockContentTypes.HardwareSensor, tile!.ContentType);
        Assert.Equal("cpu.temp", tile.ContentKey);
        Assert.Equal("°C", tile.Metadata["unit"]);
    }

    [Fact]
    public void History_store_keeps_bounded_ring_and_purges_stale_sensors()
    {
        var store = new SensorHistoryStore(capacity: 8);
        for (var index = 0; index < 12; index++)
            store.AppendSnapshot([("a", index)]);

        var values = store.GetValues("a");
        Assert.Equal(8, values.Count);
        Assert.Equal(4, values[0]);
        Assert.Equal(11, values[^1]);

        for (var generation = 0; generation < SensorHistoryStore.MissingRetentionSamples; generation++)
            store.AppendSnapshot([("b", generation)]);

        Assert.Empty(store.GetValues("a"));
        Assert.NotEmpty(store.GetValues("b"));
    }

    [Fact]
    public void History_store_set_capacity_retains_recent_samples()
    {
        var store = new SensorHistoryStore(capacity: 16);
        for (var index = 0; index < 16; index++)
            store.AppendSnapshot([("a", index)]);

        store.SetCapacity(8);
        var values = store.GetValues("a");
        Assert.Equal(8, store.Capacity);
        Assert.Equal(8, values.Count);
        Assert.Equal(8, values[0]);
        Assert.Equal(15, values[^1]);
    }

    [Fact]
    public void PopOut_and_redock_preserve_tiles()
    {
        var document = DockLayoutDocument.Create(DockShellIds.HardwareSensors);
        Assert.True(DockLayoutEngine.TryAddTile(
            document, DockContentTypes.HardwareSensor, "cpu.temp", "CPU", null, out var tile, out _));
        var boardId = DockLayoutEngine.ResolveActiveBoard(document).Id;

        var site = DockLayoutEngine.PopOutBoard(document, boardId);
        Assert.Single(document.FloatingSites);
        Assert.Equal(tile!.InstanceId, DockLayoutEngine.EnumerateBoards(site.Root).Single().Tiles.Single().InstanceId);

        DockLayoutEngine.RedockFloatingSite(document, site.Id, DockDropZone.Tab);
        Assert.Empty(document.FloatingSites);
        Assert.Contains(DockLayoutEngine.EnumerateBoards(document), board =>
            board.Tiles.Any(value => value.ContentKey == "cpu.temp"));
    }

    [Fact]
    public void Layout_store_round_trips_boards_and_tiles()
    {
        var root = TempDirectory();
        try
        {
            var store = new DockLayoutStore(DockShellIds.HardwareSensors, root, "Hardware graphs");
            var document = store.LoadOrCreate();
            Assert.True(DockLayoutEngine.TryAddTile(
                document, DockContentTypes.HardwareSensor, "gpu.temp", "GPU",
                new Dictionary<string, string> { ["unit"] = "°C" }, out _, out _));
            store.Save(document);

            var reloaded = new DockLayoutStore(DockShellIds.HardwareSensors, root).LoadOrCreate();
            Assert.Equal(1, DockLayoutEngine.CountTiles(reloaded));
            Assert.Contains(DockLayoutEngine.EnumerateBoards(reloaded), board =>
                board.Tiles.Any(tile => tile.ContentKey == "gpu.temp" && tile.Metadata["unit"] == "°C"));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public void Layout_store_migrates_legacy_sensor_graph_file()
    {
        var root = TempDirectory();
        try
        {
            var legacy = new
            {
                SchemaVersion = 1,
                EmbeddedRoot = new
                {
                    kind = "tabs",
                    id = "tabs.main",
                    activeIndex = 0,
                    tabs = new object[]
                    {
                        new
                        {
                            kind = "board",
                            id = "board.main",
                            title = "Hardware graphs",
                            columns = 4,
                            tiles = new object[]
                            {
                                new
                                {
                                    InstanceId = "tile1",
                                    SensorId = "legacy.cpu",
                                    Title = "Legacy CPU",
                                    Unit = "°C",
                                    Row = 0,
                                    Column = 0,
                                    RowSpan = 2,
                                    ColumnSpan = 2
                                }
                            }
                        }
                    }
                },
                FloatingSites = Array.Empty<object>(),
                ActiveBoardId = "board.main"
            };
            File.WriteAllText(
                Path.Combine(root, DockLayoutStore.LegacyHardwareLayoutFileName),
                JsonSerializer.Serialize(legacy));

            var document = new DockLayoutStore(DockShellIds.HardwareSensors, root, "Hardware graphs").LoadOrCreate();
            Assert.Equal(DockShellIds.HardwareSensors, document.ShellId);
            Assert.Contains(DockLayoutEngine.EnumerateBoards(document), board =>
                board.Tiles.Any(tile =>
                    tile.ContentType == DockContentTypes.HardwareSensor &&
                    tile.ContentKey == "legacy.cpu" &&
                    tile.Metadata["unit"] == "°C"));
            Assert.True(File.Exists(Path.Combine(root, "dock-hardware.sensors.json")));
            Assert.False(File.Exists(Path.Combine(root, DockLayoutStore.LegacyHardwareLayoutFileName)));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public void Session_retain_count_and_remove_tile()
    {
        var root = TempDirectory();
        try
        {
            var session = new DockSession(new DockLayoutStore(DockShellIds.HardwareSensors, root));
            session.Retain();
            session.Retain();
            Assert.Equal(2, session.RetainCount);
            session.Release();
            Assert.Equal(1, session.RetainCount);

            Assert.True(session.TryAddTile(
                DockContentTypes.HardwareSensor, "fan.1", "Fan",
                new Dictionary<string, string> { ["unit"] = "RPM" }, out _));
            var instanceId = DockLayoutEngine.EnumerateBoards(session.Layout).SelectMany(board => board.Tiles).Single().InstanceId;
            Assert.True(session.RemoveTile(instanceId));
            Assert.Equal(0, DockLayoutEngine.CountTiles(session.Layout));
        }
        finally { DeleteDirectory(root); }
    }

    private static string TempDirectory()
    {
        var path = Path.Combine(Path.GetTempPath(), "Sift-Dock-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(path);
        return path;
    }

    private static void DeleteDirectory(string path)
    {
        try { if (Directory.Exists(path)) Directory.Delete(path, recursive: true); }
        catch { }
    }
}
