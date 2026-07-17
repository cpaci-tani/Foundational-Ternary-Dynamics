using System.IO;

namespace Sift.Services;

public enum ProductDataMigrationStatus
{
    NoLegacyData,
    MigratedDirectory,
    MergedMissingData,
    CurrentDataPreserved,
    BlockedReparsePoint,
    Failed
}

public sealed record ProductDataMigrationResult(ProductDataMigrationStatus Status, string Detail)
{
    public bool Changed => Status is ProductDataMigrationStatus.MigratedDirectory or ProductDataMigrationStatus.MergedMissingData;
}

public static class ProductPaths
{
    public const string ProductName = "Sift";
    private static readonly string LegacyProductName = string.Concat("Clear", "Win");

    public static string DataRoot => Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), ProductName);

    public static string BackupDirectory => Path.Combine(DataRoot, "Backups");
    public static string ElevationDirectory => Path.Combine(DataRoot, "Elevation");

    public static ProductDataMigrationResult EnsureLegacyDataMigrated() => MigrateLegacyData(
        Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), LegacyProductName),
        DataRoot);

    internal static ProductDataMigrationResult MigrateLegacyData(string legacyRoot, string currentRoot)
    {
        try
        {
            legacyRoot = Path.GetFullPath(legacyRoot);
            currentRoot = Path.GetFullPath(currentRoot);
            if (string.Equals(legacyRoot, currentRoot, StringComparison.OrdinalIgnoreCase) || !Directory.Exists(legacyRoot))
                return new(ProductDataMigrationStatus.NoLegacyData, "No legacy application data was found.");

            if (IsReparsePoint(legacyRoot) || HasTopLevelReparsePoint(legacyRoot) ||
                Directory.Exists(currentRoot) && (IsReparsePoint(currentRoot) || HasTopLevelReparsePoint(currentRoot)))
                return new(ProductDataMigrationStatus.BlockedReparsePoint, "Legacy data migration was blocked because an application data root is a reparse point.");

            if (!Directory.Exists(currentRoot))
            {
                Directory.Move(legacyRoot, currentRoot);
                return new(ProductDataMigrationStatus.MigratedDirectory, $"Moved the legacy application data directory to {currentRoot}.");
            }

            var copied = 0;
            copied += CopyMissingFile(legacyRoot, currentRoot, "settings.json");
            copied += CopyMissingFile(legacyRoot, currentRoot, "activity.json");
            copied += CopyMissingFile(legacyRoot, currentRoot, "winui-startup.log");
            copied += CopyMissingBackups(legacyRoot, currentRoot);
            return copied > 0
                ? new(ProductDataMigrationStatus.MergedMissingData, $"Imported {copied:N0} missing legacy data file(s) without overwriting Sift data.")
                : new(ProductDataMigrationStatus.CurrentDataPreserved, "Sift data already exists; no legacy file was overwritten.");
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException or NotSupportedException)
        {
            return new(ProductDataMigrationStatus.Failed, $"Legacy data migration was skipped: {exception.Message}");
        }
    }

    private static int CopyMissingBackups(string legacyRoot, string currentRoot)
    {
        var legacyBackups = Path.Combine(legacyRoot, "Backups");
        if (!Directory.Exists(legacyBackups) || IsReparsePoint(legacyBackups)) return 0;
        var currentBackups = Path.Combine(currentRoot, "Backups");
        if (Directory.Exists(currentBackups) && IsReparsePoint(currentBackups)) return 0;
        Directory.CreateDirectory(currentBackups);
        var copied = 0;
        foreach (var source in Directory.EnumerateFiles(legacyBackups, "backup-*.json", SearchOption.TopDirectoryOnly))
        {
            if (File.GetAttributes(source).HasFlag(FileAttributes.ReparsePoint)) continue;
            var destination = Path.Combine(currentBackups, Path.GetFileName(source));
            if (File.Exists(destination)) continue;
            File.Copy(source, destination, overwrite: false);
            copied++;
        }
        return copied;
    }

    private static int CopyMissingFile(string legacyRoot, string currentRoot, string fileName)
    {
        var source = Path.Combine(legacyRoot, fileName);
        var destination = Path.Combine(currentRoot, fileName);
        if (!File.Exists(source) || File.Exists(destination) ||
            File.GetAttributes(source).HasFlag(FileAttributes.ReparsePoint)) return 0;
        Directory.CreateDirectory(currentRoot);
        File.Copy(source, destination, overwrite: false);
        return 1;
    }

    private static bool IsReparsePoint(string path) =>
        File.GetAttributes(path).HasFlag(FileAttributes.ReparsePoint);

    private static bool HasTopLevelReparsePoint(string root) =>
        Directory.EnumerateFileSystemEntries(root, "*", SearchOption.TopDirectoryOnly)
            .Any(path => File.GetAttributes(path).HasFlag(FileAttributes.ReparsePoint));
}
