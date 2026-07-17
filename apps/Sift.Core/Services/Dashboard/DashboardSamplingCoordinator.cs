namespace Sift.Services;

public sealed class DashboardSamplingCoordinator
{
    private DateTimeOffset? _lastDecisionUtc;
    private DateTimeOffset _nextMediumUtc;
    private DateTimeOffset _nextSlowUtc;

    public DashboardSampleKind Next(DateTimeOffset nowUtc)
    {
        if (_lastDecisionUtc is null)
        {
            Reset(nowUtc);
            return DashboardSampleKind.Fast;
        }
        if (nowUtc - _lastDecisionUtc > TimeSpan.FromSeconds(30) || nowUtc >= _nextSlowUtc)
        {
            Reset(nowUtc);
            return DashboardSampleKind.Slow;
        }
        _lastDecisionUtc = nowUtc;
        if (nowUtc < _nextMediumUtc) return DashboardSampleKind.Fast;
        _nextMediumUtc = nowUtc.AddSeconds(30);
        return DashboardSampleKind.Medium;
    }

    public void Reset(DateTimeOffset nowUtc)
    {
        _lastDecisionUtc = nowUtc;
        _nextMediumUtc = nowUtc.AddSeconds(30);
        _nextSlowUtc = nowUtc.AddMinutes(5);
    }

    public static TimeSpan Delay(bool batterySaver) =>
        batterySaver ? TimeSpan.FromSeconds(10) : TimeSpan.FromSeconds(2);
}
