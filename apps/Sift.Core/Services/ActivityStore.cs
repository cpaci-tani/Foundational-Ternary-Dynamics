using System.IO;
using System.Text.Json;
using Sift.Models;
using Sift.Infrastructure.Persistence;

namespace Sift.Services;

public sealed class ActivityStore : IActivityStore
{
    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };
    private const int MaxEntries = 200;
    private readonly string _path;
    private readonly object _gate = new();

    public ActivityStore(string? directory = null)
    {
        var root = directory ?? ProductPaths.DataRoot;
        Directory.CreateDirectory(root);
        _path = Path.Combine(root, "activity.json");
    }

    public IReadOnlyList<ActivityEntry> Load()
    {
        lock (_gate)
        {
            try
            {
                if (!File.Exists(_path)) return [];
                return JsonSerializer.Deserialize<List<ActivityEntry>>(File.ReadAllText(_path), JsonOptions) ?? [];
            }
            catch
            {
                return [];
            }
        }
    }

    public void Append(string category, string summary, string? detail = null, string? relatedPath = null)
    {
        lock (_gate)
        {
            var list = Load().ToList();
            list.Insert(0, new ActivityEntry
            {
                CreatedUtc = DateTime.UtcNow,
                Category = category,
                Summary = summary,
                Detail = detail,
                RelatedPath = relatedPath
            });
            while (list.Count > MaxEntries) list.RemoveAt(list.Count - 1);
            AtomicFile.WriteAllText(_path, JsonSerializer.Serialize(list, JsonOptions));
        }
    }
}
