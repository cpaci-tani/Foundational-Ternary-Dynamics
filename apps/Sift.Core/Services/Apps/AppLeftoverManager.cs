using System.Collections.Concurrent;
using System.Text.RegularExpressions;
using Sift.Models;

namespace Sift.Services;

public interface IAppLeftoverManager
{
    AppLeftoverScanResult ScanLeftovers(InstalledApp app, string? continuationToken, CancellationToken cancellationToken = default);
    AppLeftoverDeleteResult DeleteLeftovers(InstalledApp app, string? continuationToken,
        IEnumerable<AppLeftoverCandidate> selection, bool preview, CancellationToken cancellationToken = default);
}

internal sealed record LeftoverAuthorization(string Fingerprint, DateTime ExpiresUtc);

internal sealed partial class AppLeftoverManager(
    IInstalledAppInventory inventory,
    IStorageDeleter storageDeleter,
    ConcurrentDictionary<string, LeftoverAuthorization> authorizations) : IAppLeftoverManager
{
    private static readonly HashSet<string> ProtectedFolderNames = new(StringComparer.OrdinalIgnoreCase)
    {
        "Microsoft", "Packages", "Programs", "Temp", "ConnectedDevicesPlatform", "Publishers",
        "D3DSCache", "CrashDumps", "Sift", "Google", "Mozilla", "NVIDIA Corporation", "AMD"
    };

    private static readonly (string Scope, string Path)[] Roots = CreateRoots();

    public AppLeftoverScanResult ScanLeftovers(InstalledApp app, string? continuationToken,
        CancellationToken cancellationToken = default)
    {
        if (!CanManageLeftovers(app, continuationToken, out var reason))
            return new AppLeftoverScanResult(true, reason, []);

        var candidates = new List<AppLeftoverCandidate>();
        foreach (var candidatePath in CandidatePaths(app))
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (!Directory.Exists(candidatePath.Path)) continue;
            var measured = Measure(candidatePath.Path, cancellationToken);
            candidates.Add(new AppLeftoverCandidate
            {
                AppIdentity = Fingerprint(app),
                AppDisplayName = app.DisplayName,
                Path = candidatePath.Path,
                Scope = candidatePath.Scope,
                Evidence = $"Exact generated folder-name match for {app.DisplayName} under {candidatePath.Scope}; the app is no longer actively registered.",
                SizeBytes = measured.Bytes,
                FileCount = measured.Files,
                CanDelete = measured.Safe,
                BlockReason = measured.Safe ? string.Empty : measured.Reason
            });
        }

        var ordered = candidates.OrderByDescending(candidate => candidate.SizeBytes).ThenBy(candidate => candidate.Path).ToList();
        return new AppLeftoverScanResult(false,
            ordered.Count == 0
                ? $"No exact top-level AppData folder matches were found for {app.DisplayName}."
                : $"Found {ordered.Count:N0} exact AppData folder match(es) for {app.DisplayName}. Nothing is selected or deleted automatically.",
            ordered);
    }

    public AppLeftoverDeleteResult DeleteLeftovers(InstalledApp app, string? continuationToken,
        IEnumerable<AppLeftoverCandidate> selection, bool preview, CancellationToken cancellationToken = default)
    {
        var log = new List<string>();
        var previewed = 0;
        var deleted = 0;
        var skipped = 0;
        var failed = 0;
        var selected = selection.Where(candidate => candidate.IsSelected).ToList();

        foreach (var candidate in selected)
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (!CanManageLeftovers(app, continuationToken, out var registrationReason))
            {
                skipped++;
                log.Add($"BLOCKED  {candidate.Path} · {registrationReason}");
                continue;
            }
            var pathReason = string.Empty;
            if (!candidate.CanDelete || !string.Equals(candidate.AppIdentity, Fingerprint(app), StringComparison.Ordinal) ||
                !IsExactCandidatePath(app, candidate.Path, out pathReason))
            {
                skipped++;
                log.Add($"BLOCKED  {candidate.Path} · {(string.IsNullOrWhiteSpace(pathReason) ? "candidate identity or scan policy changed" : pathReason)}");
                continue;
            }
            if (!Directory.Exists(candidate.Path))
            {
                skipped++;
                log.Add($"SKIPPED  {candidate.Path} · folder no longer exists");
                continue;
            }

            var measured = Measure(candidate.Path, cancellationToken);
            if (!measured.Safe)
            {
                skipped++;
                log.Add($"BLOCKED  {candidate.Path} · {measured.Reason}");
                continue;
            }

            var result = storageDeleter.MoveToRecycleBin([candidate.Path], preview);
            foreach (var line in result.Log) log.Add(line);
            if (preview) previewed += result.Deleted;
            else deleted += result.Deleted;
            skipped += result.Skipped;
            failed += result.Failed;
        }

        return new AppLeftoverDeleteResult(preview, previewed, deleted, skipped, failed, log);
    }

    private bool CanManageLeftovers(InstalledApp app, string? continuationToken, out string reason)
    {
        reason = string.Empty;
        var current = inventory.FindExact(app.RegistryLocation);
        if (current is not null)
        {
            if (!SameRegistration(current, app))
            {
                reason = "The app registration changed. Refresh before scanning leftovers.";
                return false;
            }
            if (!current.IsOrphanedRegistration)
            {
                reason = "The app is still registered as installed. Complete its uninstaller before deleting AppData leftovers.";
                return false;
            }
            return true;
        }

        if (string.IsNullOrWhiteSpace(continuationToken) ||
            !authorizations.TryGetValue(continuationToken, out var authorized) ||
            authorized.ExpiresUtc <= DateTime.UtcNow ||
            !string.Equals(authorized.Fingerprint, Fingerprint(app), StringComparison.Ordinal))
        {
            if (!string.IsNullOrWhiteSpace(continuationToken)) authorizations.TryRemove(continuationToken, out _);
            reason = "Sift cannot verify this app in the current uninstall session. Start from its registered uninstall action or an orphan registration.";
            return false;
        }
        return true;
    }

    private static IEnumerable<(string Scope, string Path)> CandidatePaths(InstalledApp app)
    {
        var names = CandidateFolderNames(app.DisplayName);
        var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (var root in Roots)
        foreach (var name in names)
        {
            if (ProtectedFolderNames.Contains(name)) continue;
            var path = Path.GetFullPath(Path.Combine(root.Path, name));
            if (!Path.GetDirectoryName(path)!.Equals(Path.GetFullPath(root.Path).TrimEnd('\\'), StringComparison.OrdinalIgnoreCase)) continue;
            if (seen.Add(path)) yield return (root.Scope, path);
        }
    }

    private static IReadOnlyList<string> CandidateFolderNames(string displayName)
    {
        var raw = displayName.Trim();
        var stripped = ArchitectureRegex().Replace(raw, string.Empty).Trim();
        stripped = VersionSuffixRegex().Replace(stripped, string.Empty).Trim(' ', '-', '_');
        var values = new[]
        {
            raw,
            stripped,
            stripped.Replace(" ", string.Empty),
            stripped.Replace("-", string.Empty).Replace("_", string.Empty).Replace(" ", string.Empty)
        };
        return values
            .Select(SanitizeFolderName)
            .Where(value => value.Length >= 3 && !ProtectedFolderNames.Contains(value))
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .Take(6)
            .ToList();
    }

    private static string SanitizeFolderName(string value)
    {
        var invalid = Path.GetInvalidFileNameChars().ToHashSet();
        return new string(value.Where(character => !invalid.Contains(character)).ToArray()).Trim().TrimEnd('.');
    }

    private static bool IsExactCandidatePath(InstalledApp app, string path, out string reason)
    {
        reason = string.Empty;
        string full;
        try { full = Path.GetFullPath(path).TrimEnd('\\'); }
        catch (Exception exception) { reason = exception.Message; return false; }
        var allowed = CandidatePaths(app).Any(candidate =>
            string.Equals(Path.GetFullPath(candidate.Path).TrimEnd('\\'), full, StringComparison.OrdinalIgnoreCase));
        if (!allowed) reason = "the folder is outside the exact generated AppData candidates";
        return allowed;
    }

    private static (bool Safe, long Bytes, long Files, string Reason) Measure(string root, CancellationToken cancellationToken)
    {
        try
        {
            var rootAttributes = File.GetAttributes(root);
            if (rootAttributes.HasFlag(FileAttributes.ReparsePoint)) return (false, 0, 0, "root reparse points are blocked");
            long bytes = 0;
            long files = 0;
            var stack = new Stack<string>();
            stack.Push(root);
            while (stack.Count > 0)
            {
                cancellationToken.ThrowIfCancellationRequested();
                var directory = stack.Pop();
                foreach (var entry in Directory.EnumerateFileSystemEntries(directory))
                {
                    cancellationToken.ThrowIfCancellationRequested();
                    var attributes = File.GetAttributes(entry);
                    if (attributes.HasFlag(FileAttributes.ReparsePoint))
                        return (false, bytes, files, $"nested reparse point blocked: {entry}");
                    if (attributes.HasFlag(FileAttributes.Directory)) stack.Push(entry);
                    else
                    {
                        bytes = checked(bytes + new FileInfo(entry).Length);
                        files++;
                    }
                }
            }
            return (true, bytes, files, string.Empty);
        }
        catch (OperationCanceledException) { throw; }
        catch (Exception exception) { return (false, 0, 0, $"folder could not be fully inspected: {exception.Message}"); }
    }

    private static bool SameRegistration(InstalledApp left, InstalledApp right) =>
        string.Equals(left.DisplayName, right.DisplayName, StringComparison.Ordinal) &&
        string.Equals(left.UninstallString, right.UninstallString, StringComparison.Ordinal) &&
        string.Equals(left.InstallLocation, right.InstallLocation, StringComparison.Ordinal);

    internal static string Fingerprint(InstalledApp app) =>
        $"{app.RegistryLocation.Identity}\u001f{app.DisplayName}\u001f{app.UninstallString}\u001f{app.InstallLocation}";

    private static (string Scope, string Path)[] CreateRoots()
    {
        var profile = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
        return
        [
            ("Local AppData", Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)),
            ("Roaming AppData", Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)),
            ("LocalLow AppData", Path.Combine(profile, "AppData", "LocalLow"))
        ];
    }

    [GeneratedRegex(@"\s*(\((x64|x86|arm64|32-bit|64-bit)\)|(x64|x86|arm64|32-bit|64-bit))\s*$", RegexOptions.IgnoreCase)]
    private static partial Regex ArchitectureRegex();

    [GeneratedRegex(@"\s+v?\d+(\.\d+){1,4}([\s_-].*)?$", RegexOptions.IgnoreCase)]
    private static partial Regex VersionSuffixRegex();
}
