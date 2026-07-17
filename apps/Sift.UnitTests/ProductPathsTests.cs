using Sift.Services;

namespace Sift.UnitTests;

public sealed class ProductPathsTests
{
    [Fact]
    public void LegacyDirectory_IsMovedWhenSiftDataDoesNotExist()
    {
        var root = NewRoot();
        var legacy = Path.Combine(root, "legacy");
        var current = Path.Combine(root, "Sift");
        try
        {
            Directory.CreateDirectory(Path.Combine(legacy, "Backups"));
            File.WriteAllText(Path.Combine(legacy, "settings.json"), "settings");
            File.WriteAllText(Path.Combine(legacy, "Backups", "backup-one.json"), "backup");

            var result = ProductPaths.MigrateLegacyData(legacy, current);

            Assert.Equal(ProductDataMigrationStatus.MigratedDirectory, result.Status);
            Assert.False(Directory.Exists(legacy));
            Assert.Equal("settings", File.ReadAllText(Path.Combine(current, "settings.json")));
            Assert.Equal("backup", File.ReadAllText(Path.Combine(current, "Backups", "backup-one.json")));
        }
        finally
        {
            DeleteRoot(root);
        }
    }

    [Fact]
    public void ExistingSiftData_IsNeverOverwrittenWhileMissingDataIsImported()
    {
        var root = NewRoot();
        var legacy = Path.Combine(root, "legacy");
        var current = Path.Combine(root, "Sift");
        try
        {
            Directory.CreateDirectory(Path.Combine(legacy, "Backups"));
            Directory.CreateDirectory(current);
            File.WriteAllText(Path.Combine(legacy, "settings.json"), "legacy-settings");
            File.WriteAllText(Path.Combine(legacy, "activity.json"), "legacy-activity");
            File.WriteAllText(Path.Combine(legacy, "Backups", "backup-one.json"), "legacy-backup");
            File.WriteAllText(Path.Combine(current, "settings.json"), "sift-settings");

            var result = ProductPaths.MigrateLegacyData(legacy, current);

            Assert.Equal(ProductDataMigrationStatus.MergedMissingData, result.Status);
            Assert.Equal("sift-settings", File.ReadAllText(Path.Combine(current, "settings.json")));
            Assert.Equal("legacy-activity", File.ReadAllText(Path.Combine(current, "activity.json")));
            Assert.Equal("legacy-backup", File.ReadAllText(Path.Combine(current, "Backups", "backup-one.json")));
        }
        finally
        {
            DeleteRoot(root);
        }
    }

    [Fact]
    public void MissingLegacyData_LeavesSiftDataUntouched()
    {
        var root = NewRoot();
        try
        {
            var result = ProductPaths.MigrateLegacyData(Path.Combine(root, "missing"), Path.Combine(root, "Sift"));
            Assert.Equal(ProductDataMigrationStatus.NoLegacyData, result.Status);
            Assert.False(result.Changed);
        }
        finally
        {
            DeleteRoot(root);
        }
    }

    private static string NewRoot()
    {
        var root = Path.Combine(Path.GetTempPath(), "Sift-ProductPathsTests-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(root);
        return root;
    }

    private static void DeleteRoot(string root)
    {
        if (Directory.Exists(root)) Directory.Delete(root, recursive: true);
    }
}
