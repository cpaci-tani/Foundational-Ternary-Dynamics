using Sift.Services;

namespace Sift.UnitTests;

public sealed class BatteryReportReaderTests
{
    [Fact]
    public void Read_does_not_throw_and_marks_desktop_as_absent_or_present()
    {
        var snapshot = BatteryReportReader.Read();
        if (!snapshot.Present)
        {
            Assert.Null(snapshot.ChargePercent);
            return;
        }

        if (snapshot.ChargePercent is { } charge)
            Assert.InRange(charge, 0, 100);
        if (snapshot.HealthPercent is { } health)
            Assert.InRange(health, 0, 100);
    }
}
