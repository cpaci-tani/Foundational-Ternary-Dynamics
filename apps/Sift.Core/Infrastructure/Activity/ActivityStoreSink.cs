using Sift.Services;

namespace Sift.Infrastructure.Activity;

public sealed class ActivityStoreSink(IActivityStore store) : IActivitySink
{
    public void Publish(ActivityEvent activity)
    {
        if (!activity.Persist) return;
        store.Append(activity.Category, activity.Summary, activity.Detail, activity.RelatedPath);
    }
}
