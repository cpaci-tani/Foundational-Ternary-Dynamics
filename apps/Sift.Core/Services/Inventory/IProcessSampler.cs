using Sift.Models;

namespace Sift.Services;

public interface IProcessSampler
{
    SystemSnapshot Sample(CancellationToken cancellationToken = default);
}
