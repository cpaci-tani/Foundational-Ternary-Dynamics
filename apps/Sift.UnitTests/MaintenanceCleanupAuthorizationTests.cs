using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class MaintenanceCleanupAuthorizationTests
{
    [Fact]
    public void Standard_user_scan_does_not_expose_machine_wide_orphan_cleanup()
    {
        var findings = new MaintenanceScanner(() => false).Scan();

        Assert.DoesNotContain(findings, finding =>
            string.Equals(finding.RegistryHive, "HKLM", StringComparison.OrdinalIgnoreCase));
    }

    [Fact]
    public async Task Standard_user_review_rejects_caller_constructed_HKLM_orphan()
    {
        const string subKey = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\SiftFixture";
        var finding = new MaintenanceFinding
        {
            Id = "orphan-uninstall.machine-fixture",
            Category = MaintenanceCategory.OrphanUninstall,
            Title = "Machine fixture",
            Path = $@"HKLM\{subKey}",
            Detail = "fixture",
            SizeBytes = 0,
            CanClean = true,
            RegistryHive = "HKLM",
            RegistrySubKey = subKey,
            RegistryValues = new Dictionary<string, string?>()
        };

        var review = await new MaintenanceCleaner(isElevated: () => false).ReviewAsync(
            [finding], TestContext.Current.CancellationToken);

        Assert.False(review.CanExecute);
        Assert.Equal(MaintenanceCleanupStatus.Rejected, review.Result.Status);
        Assert.Contains(review.Result.Log, line =>
            line.Contains("administrator permission is required", StringComparison.Ordinal));
    }

    [Fact]
    public async Task Execute_rejects_selection_when_reviewed_contents_change()
    {
        var crashDumpDirectory = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "CrashDumps");
        Directory.CreateDirectory(crashDumpDirectory);
        var reviewedFile = Path.Combine(crashDumpDirectory, $"Sift-reviewed-{Guid.NewGuid():N}.dmp");
        var addedFile = Path.Combine(crashDumpDirectory, $"Sift-added-{Guid.NewGuid():N}.dmp");
        await File.WriteAllTextAsync(reviewedFile, "reviewed", TestContext.Current.CancellationToken);

        try
        {
            var finding = new MaintenanceFinding
            {
                Id = "maintenance.crash-dumps",
                Category = MaintenanceCategory.CrashDumps,
                Title = "User crash dumps",
                Path = Path.Combine(crashDumpDirectory, "*.dmp"),
                Detail = "fixture",
                SizeBytes = new FileInfo(reviewedFile).Length,
                CanClean = true
            };
            var cleaner = new MaintenanceCleaner(isElevated: () => false);
            var review = await cleaner.ReviewAsync([finding], TestContext.Current.CancellationToken);
            Assert.True(review.CanExecute);

            await File.WriteAllTextAsync(addedFile, "added after confirmation", TestContext.Current.CancellationToken);
            var result = await cleaner.ExecuteAsync(review.TicketId!, TestContext.Current.CancellationToken);

            Assert.Equal(MaintenanceCleanupStatus.Invalidated, result.Status);
            Assert.Equal(0, result.Cleaned);
            Assert.True(File.Exists(reviewedFile));
            Assert.True(File.Exists(addedFile));
            Assert.Contains(result.Log, line => line.Contains("changed after review", StringComparison.Ordinal));
        }
        finally
        {
            File.Delete(reviewedFile);
            File.Delete(addedFile);
        }
    }
}
