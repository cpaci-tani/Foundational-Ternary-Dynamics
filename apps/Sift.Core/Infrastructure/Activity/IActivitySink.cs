namespace Sift.Infrastructure.Activity;

public interface IActivitySink
{
    void Publish(ActivityEvent activity);
}
