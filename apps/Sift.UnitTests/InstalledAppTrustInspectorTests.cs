using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class InstalledAppTrustInspectorTests
{
    [Fact]
    public void Msi_report_does_not_misattribute_the_windows_installer_host_to_the_product_publisher()
    {
        var app = App("Vendor MSI", "Vendor", "MsiExec.exe /X {01234567-89AB-CDEF-0123-456789ABCDEF}");
        var report = new InstalledAppTrustInspector(new FixtureInventory(app)).Inspect(app,
            TestContext.Current.CancellationToken);

        Assert.Equal(InstalledAppSignatureStatus.WindowsInstaller, report.Status);
        Assert.Equal(InstalledAppPublisherMatch.NotAvailable, report.PublisherMatch);
        Assert.Contains("product signer is not exposed", report.Summary, StringComparison.OrdinalIgnoreCase);
        Assert.EndsWith("msiexec.exe", report.ExecutablePath, StringComparison.OrdinalIgnoreCase);
        Assert.Equal(64, report.Sha256.Length);
    }

    [Fact]
    public void Signed_windows_executable_reports_local_authenticode_signer_version_and_hash()
    {
        var ping = Path.Combine(Environment.SystemDirectory, "ping.exe");
        Assert.True(File.Exists(ping));
        var app = App("Microsoft network fixture", "Microsoft Corporation", $"\"{ping}\" -n 1 127.0.0.1");
        var report = new InstalledAppTrustInspector(new FixtureInventory(app)).Inspect(app,
            TestContext.Current.CancellationToken);

        Assert.True(report.Status == InstalledAppSignatureStatus.Trusted,
            $"Expected a trusted catalog signature, got {report.Status}: {report.Detail}");
        Assert.False(string.IsNullOrWhiteSpace(report.Signer));
        Assert.False(string.IsNullOrWhiteSpace(report.FileVersion));
        Assert.Equal(64, report.Sha256.Length);
        Assert.Contains("without network retrieval", report.Detail, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void Changed_registration_is_rejected_before_file_inspection()
    {
        var current = App("Vendor app", "Vendor", "MsiExec.exe /X {01234567-89AB-CDEF-0123-456789ABCDEF}");
        var requested = current with { UninstallString = "MsiExec.exe /X {11111111-1111-1111-1111-111111111111}" };

        var report = new InstalledAppTrustInspector(new FixtureInventory(current)).Inspect(requested,
            TestContext.Current.CancellationToken);

        Assert.Equal(InstalledAppSignatureStatus.Unavailable, report.Status);
        Assert.Contains("changed", report.Detail, StringComparison.OrdinalIgnoreCase);
    }

    private static InstalledApp App(string name, string publisher, string uninstall) => new(
        new InstalledAppRegistryLocation("HKCU", "64-bit",
            @"Software\Microsoft\Windows\CurrentVersion\Uninstall\trust-fixture"),
        name, publisher, "1.0", string.Empty, string.Empty, 0, uninstall, "Current user", true,
        "The registered interactive uninstaller can be opened after confirmation.");

    private sealed class FixtureInventory(InstalledApp fixture) : IInstalledAppInventory
    {
        public IReadOnlyList<InstalledApp> Enumerate(CancellationToken cancellationToken = default) => [fixture];
        public InstalledApp? FindExact(InstalledAppRegistryLocation location) =>
            string.Equals(location.Identity, fixture.RegistryLocation.Identity, StringComparison.OrdinalIgnoreCase)
                ? fixture
                : null;
    }
}
