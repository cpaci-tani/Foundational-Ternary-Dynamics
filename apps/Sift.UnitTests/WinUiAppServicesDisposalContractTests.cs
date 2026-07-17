using Sift.Infrastructure.Logging;
using Sift.Infrastructure.Operations;
using Sift.Infrastructure.Settings;
using Sift.Services;

namespace Sift.UnitTests;

/// <summary>
/// Documents which Core types constructed by WinUiAppServices.CreateDefault must be disposed.
/// Keep aligned with WinUiAppServices.OwnedDisposables.
/// </summary>
public sealed class WinUiAppServicesDisposalContractTests
{
    [Fact]
    public void CreateDefault_owned_disposable_core_types_are_known()
    {
        var expected = new[]
        {
            typeof(OperationCoordinator),
            typeof(IHardwareMonitorService),
            typeof(IDashboardHistoryStore),
            typeof(SettingsPersistenceCoordinator),
            typeof(ISiftLog),
            typeof(IDashboardTelemetrySource)
        };

        Assert.All(expected, type => Assert.True(
            typeof(IDisposable).IsAssignableFrom(type) || type.IsInterface,
            $"{type.Name} must remain disposable."));

        Assert.Contains(typeof(IHardwareMonitorService), typeof(HardwareMonitorService).GetInterfaces());
        Assert.Contains(typeof(IDashboardHistoryStore), typeof(DashboardHistoryStore).GetInterfaces());
        Assert.Contains(typeof(ISiftLog), typeof(SiftFileLog).GetInterfaces());
        Assert.Contains(typeof(IDashboardTelemetrySource), typeof(DashboardTelemetryService).GetInterfaces());
        Assert.True(typeof(IDisposable).IsAssignableFrom(typeof(OperationCoordinator)));
        Assert.True(typeof(IDisposable).IsAssignableFrom(typeof(SettingsPersistenceCoordinator)));
        Assert.True(typeof(IDisposable).IsAssignableFrom(typeof(HardwareMonitorService)));
        Assert.True(typeof(IDisposable).IsAssignableFrom(typeof(DashboardHistoryStore)));
        Assert.True(typeof(IDisposable).IsAssignableFrom(typeof(SiftFileLog)));
        Assert.True(typeof(IDisposable).IsAssignableFrom(typeof(DashboardTelemetryService)));
        Assert.True(typeof(IDisposable).IsAssignableFrom(typeof(DashboardMonitorTelemetrySource)));
    }
}
