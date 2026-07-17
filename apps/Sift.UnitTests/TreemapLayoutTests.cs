using Sift.Models;

namespace Sift.UnitTests;

public sealed class TreemapLayoutTests
{
    [Fact]
    public void Layout_PreservesAreaAndWeightRatiosInsideBounds()
    {
        var bounds = new TreemapBounds(10, 20, 100, 60);
        var tiles = SquarifiedTreemap.Layout(
            [new(1, 6), new(2, 3), new(3, 1)], bounds);

        Assert.Equal(3, tiles.Count);
        Assert.All(tiles, tile =>
        {
            Assert.True(tile.Bounds.X >= bounds.X - 0.001);
            Assert.True(tile.Bounds.Y >= bounds.Y - 0.001);
            Assert.True(tile.Bounds.X + tile.Bounds.Width <= bounds.X + bounds.Width + 0.001);
            Assert.True(tile.Bounds.Y + tile.Bounds.Height <= bounds.Y + bounds.Height + 0.001);
        });
        Assert.Equal(bounds.Area, tiles.Sum(tile => tile.Bounds.Area), 5);
        Assert.Equal(6, tiles.Single(tile => tile.Id == 1).Bounds.Area /
            tiles.Single(tile => tile.Id == 3).Bounds.Area, 5);
    }

    [Fact]
    public void Layout_DoesNotOverlapTiles()
    {
        var tiles = SquarifiedTreemap.Layout(
            Enumerable.Range(1, 40).Select(id => new TreemapWeightedItem(id, 41 - id)),
            new TreemapBounds(0, 0, 800, 500));

        for (var i = 0; i < tiles.Count; i++)
        for (var j = i + 1; j < tiles.Count; j++)
            Assert.True(IntersectionArea(tiles[i].Bounds, tiles[j].Bounds) < 0.001,
                $"Tiles {tiles[i].Id} and {tiles[j].Id} overlap.");
    }

    [Fact]
    public void Layout_IgnoresInvalidWeightsAndEmptyBounds()
    {
        var tiles = SquarifiedTreemap.Layout(
            [new(1, 10), new(2, 0), new(3, -1), new(4, double.NaN)],
            new TreemapBounds(0, 0, 100, 100));

        Assert.Equal(1, Assert.Single(tiles).Id);
        Assert.Empty(SquarifiedTreemap.Layout([new(1, 1)], new TreemapBounds(0, 0, 0, 100)));
    }

    private static double IntersectionArea(TreemapBounds a, TreemapBounds b)
    {
        var width = Math.Max(0, Math.Min(a.X + a.Width, b.X + b.Width) - Math.Max(a.X, b.X));
        var height = Math.Max(0, Math.Min(a.Y + a.Height, b.Y + b.Height) - Math.Max(a.Y, b.Y));
        return width * height;
    }
}
