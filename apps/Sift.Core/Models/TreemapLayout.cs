namespace Sift.Models;

public readonly record struct TreemapBounds(double X, double Y, double Width, double Height)
{
    public double Area => Math.Max(0, Width) * Math.Max(0, Height);
}

public readonly record struct TreemapWeightedItem(int Id, double Weight);

public readonly record struct TreemapTile(int Id, TreemapBounds Bounds);

/// <summary>
/// Presentation-neutral squarified treemap layout. Callers decide how tiles are
/// rendered and may recursively lay out directory children inside a parent tile.
/// </summary>
public static class SquarifiedTreemap
{
    private readonly record struct AreaItem(int Id, double Area);

    public static IReadOnlyList<TreemapTile> Layout(IEnumerable<TreemapWeightedItem> source, TreemapBounds bounds)
    {
        if (bounds.Width <= 0 || bounds.Height <= 0) return [];
        var weighted = source
            .Where(item => item.Weight > 0 && double.IsFinite(item.Weight))
            .OrderByDescending(item => item.Weight)
            .ToList();
        if (weighted.Count == 0) return [];

        var total = weighted.Sum(item => item.Weight);
        if (total <= 0 || !double.IsFinite(total)) return [];
        var scale = bounds.Area / total;
        var remainingItems = weighted.Select(item => new AreaItem(item.Id, item.Weight * scale)).ToList();
        var result = new List<TreemapTile>(remainingItems.Count);
        var row = new List<AreaItem>();
        var remaining = bounds;
        var index = 0;

        while (index < remainingItems.Count)
        {
            var candidate = remainingItems[index];
            var side = Math.Min(remaining.Width, remaining.Height);
            if (row.Count == 0 || Worst(row.Append(candidate), side) <= Worst(row, side))
            {
                row.Add(candidate);
                index++;
                continue;
            }

            remaining = LayoutRow(row, remaining, result);
            row.Clear();
        }

        if (row.Count > 0) LayoutRow(row, remaining, result);
        return result;
    }

    private static double Worst(IEnumerable<AreaItem> source, double side)
    {
        if (side <= 0) return double.PositiveInfinity;
        var items = source as IReadOnlyCollection<AreaItem> ?? source.ToList();
        if (items.Count == 0) return double.PositiveInfinity;
        var sum = items.Sum(item => item.Area);
        var maximum = items.Max(item => item.Area);
        var minimum = items.Min(item => item.Area);
        if (sum <= 0 || minimum <= 0) return double.PositiveInfinity;
        var sideSquared = side * side;
        var sumSquared = sum * sum;
        return Math.Max(sideSquared * maximum / sumSquared, sumSquared / (sideSquared * minimum));
    }

    private static TreemapBounds LayoutRow(IReadOnlyList<AreaItem> row, TreemapBounds remaining, ICollection<TreemapTile> output)
    {
        var rowArea = row.Sum(item => item.Area);
        if (rowArea <= 0 || remaining.Width <= 0 || remaining.Height <= 0) return remaining;

        if (remaining.Width >= remaining.Height)
        {
            var stripWidth = Math.Min(remaining.Width, rowArea / remaining.Height);
            var y = remaining.Y;
            for (var i = 0; i < row.Count; i++)
            {
                var height = i == row.Count - 1
                    ? remaining.Y + remaining.Height - y
                    : row[i].Area / stripWidth;
                output.Add(new TreemapTile(row[i].Id,
                    new TreemapBounds(remaining.X, y, stripWidth, Math.Max(0, height))));
                y += height;
            }
            return new TreemapBounds(remaining.X + stripWidth, remaining.Y,
                Math.Max(0, remaining.Width - stripWidth), remaining.Height);
        }

        var stripHeight = Math.Min(remaining.Height, rowArea / remaining.Width);
        var x = remaining.X;
        for (var i = 0; i < row.Count; i++)
        {
            var width = i == row.Count - 1
                ? remaining.X + remaining.Width - x
                : row[i].Area / stripHeight;
            output.Add(new TreemapTile(row[i].Id,
                new TreemapBounds(x, remaining.Y, Math.Max(0, width), stripHeight)));
            x += width;
        }
        return new TreemapBounds(remaining.X, remaining.Y + stripHeight,
            remaining.Width, Math.Max(0, remaining.Height - stripHeight));
    }
}
