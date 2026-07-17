namespace Sift.Infrastructure.Activity;

public sealed class ActivityHub : IActivitySink
{
    private readonly IReadOnlyList<IActivitySink> _sinks;

    public ActivityHub(params IActivitySink[] sinks) => _sinks = sinks;

    public event EventHandler<ActivityEvent>? Published;

    public void Publish(ActivityEvent activity)
    {
        foreach (var sink in _sinks)
        {
            try { sink.Publish(activity); }
            catch { /* Observability must never break the guarded operation. */ }
        }

        if (Published is { } published)
        {
            foreach (EventHandler<ActivityEvent> subscriber in published.GetInvocationList())
            {
                try { subscriber(this, activity); }
                catch { /* A presentation subscriber must not break the guarded operation. */ }
            }
        }
    }

    public void Info(string category, string summary, string? detail = null, bool persist = false, string? operationId = null) =>
        Publish(ActivityEvent.Create(category, summary, detail: detail, persist: persist, operationId: operationId));

    public void Warning(string category, string summary, string? detail = null, bool persist = false, string? operationId = null) =>
        Publish(ActivityEvent.Create(category, summary, ActivitySeverity.Warning, detail, persist: persist, operationId: operationId));

    public void Error(string category, string summary, string? detail = null, bool persist = false, string? operationId = null) =>
        Publish(ActivityEvent.Create(category, summary, ActivitySeverity.Error, detail, persist: persist, operationId: operationId));
}
