using Sift.Models;

namespace Sift.Services;

public interface IMaintenanceScanner
{
    bool DeliveryOptimizationSkippedForElevation { get; }
    bool PrefetchSkippedForElevation { get; }
    IReadOnlyList<MaintenanceFinding> Scan(IProgress<string>? progress = null);
}

public interface IMaintenanceCleaner
{
    Task<MaintenanceCleanupReview> ReviewAsync(IEnumerable<MaintenanceFinding> selection,
        CancellationToken cancellationToken = default);
    Task<CleanResult> ExecuteAsync(string ticketId, CancellationToken cancellationToken = default);
    void Discard(string ticketId);
}
