using Sift.Models;

namespace Sift.Services;

public interface IActivityStore
{
    IReadOnlyList<ActivityEntry> Load();
    void Append(string category, string summary, string? detail = null, string? relatedPath = null);
}
