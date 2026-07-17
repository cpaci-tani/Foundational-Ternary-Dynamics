using System.Runtime.InteropServices;
using System.Runtime.Versioning;

namespace Sift.Services;

/// <summary>
/// Battery metrics from <c>GetSystemPowerStatus</c> plus WinRT <c>AggregateBattery</c> enrichment.
/// Presentation-neutral; never requests elevation or installs drivers.
/// </summary>
[SupportedOSPlatform("windows10.0.17763.0")]
public static class BatteryReportReader
{
    public sealed record Snapshot(
        bool Present,
        double? ChargePercent,
        bool OnAc,
        bool BatterySaver,
        double? RemainingMinutes,
        double? HealthPercent,
        double? RemainingCapacityMwh,
        double? FullChargeCapacityMwh,
        double? DesignCapacityMwh,
        double? ChargeRateMw,
        string? Status);

    public static Snapshot Read()
    {
        if (!GetSystemPowerStatus(out var status) || status.BatteryFlag == 128)
            return new Snapshot(false, null, false, false, null, null, null, null, null, null, null);

        double? charge = status.BatteryLifePercent <= 100 ? status.BatteryLifePercent : null;
        double? remainingMinutes = status.BatteryLifeTime != uint.MaxValue
            ? status.BatteryLifeTime / 60d
            : null;
        var onAc = status.ACLineStatus == 1;
        var saver = status.SystemStatusFlag == 1;

        double? health = null;
        double? remainingMwh = null;
        double? fullMwh = null;
        double? designMwh = null;
        double? chargeRate = null;
        string? reportStatus = null;
        try
        {
            var report = Windows.Devices.Power.Battery.AggregateBattery.GetReport();
            reportStatus = report.Status.ToString();
            if (report.RemainingCapacityInMilliwattHours is { } remaining)
                remainingMwh = remaining;
            if (report.FullChargeCapacityInMilliwattHours is { } full)
                fullMwh = full;
            if (report.DesignCapacityInMilliwattHours is { } design)
            {
                designMwh = design;
                if (fullMwh is { } fullCap && design > 0)
                    health = Math.Clamp(fullCap * 100d / design, 0, 100);
            }
            if (report.ChargeRateInMilliwatts is { } rate)
                chargeRate = rate;
        }
        catch
        {
            // WinRT battery report is best-effort enrichment.
        }

        return new Snapshot(true, charge, onAc, saver, remainingMinutes, health,
            remainingMwh, fullMwh, designMwh, chargeRate, reportStatus);
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct SystemPowerStatus
    {
        public byte ACLineStatus;
        public byte BatteryFlag;
        public byte BatteryLifePercent;
        public byte SystemStatusFlag;
        public uint BatteryLifeTime;
        public uint BatteryFullLifeTime;
    }

    [DllImport("kernel32.dll")]
    private static extern bool GetSystemPowerStatus(out SystemPowerStatus status);
}
