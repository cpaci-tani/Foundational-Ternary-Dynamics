using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Text.Json;
using Microsoft.Win32.SafeHandles;
using Sift.Models;
using Sift.Presentation;

namespace Sift.Services;

public enum ElevatedOperationKind
{
    ApplyMachineTweaks,
    RestoreMachineBackup,
    ValidateElevation,
    ManageService,
    ChangeScheduledTask,
    CreateSystemRestorePoint,
    RunCatalogRecipe
}

public sealed record ElevatedOperationRequest(
    string RequestId,
    string Nonce,
    ElevatedOperationKind Operation,
    IReadOnlyList<string> TweakIds,
    string? BackupFileName = null,
    string? ServiceName = null,
    ServiceActionKind? ServiceAction = null,
    ServiceObservedState? ExpectedServiceState = null,
    ScheduledTaskId? ScheduledTaskId = null,
    ScheduledTaskChange? ScheduledTaskChange = null,
    bool? ExpectedTaskEnabled = null,
    string? ExpectedTaskDefinitionHash = null,
    string? RecipeId = null,
    string? ExpectedRecipeHash = null);

public sealed record ElevatedOperationResponse(
    string RequestId,
    string Nonce,
    bool Succeeded,
    bool Cancelled,
    string Message,
    int Applied,
    int Failed,
    IReadOnlyList<string> Log,
    string? BackupPath = null);

public interface IElevationBroker
{
    Task<ElevatedOperationResponse> ApplyMachineTweaksAsync(IEnumerable<Tweak> selection,
        CancellationToken cancellationToken = default);
    Task<ElevatedOperationResponse> RestoreMachineBackupAsync(string backupPath,
        CancellationToken cancellationToken = default);
    Task<ElevatedOperationResponse> ManageServiceAsync(ServiceActionTarget target, ServiceActionKind action,
        CancellationToken cancellationToken = default);
    Task<ElevatedOperationResponse> ChangeScheduledTaskAsync(
        ScheduledTaskId id,
        ScheduledTaskChange change,
        bool expectedEnabled,
        string expectedDefinitionHash,
        CancellationToken cancellationToken = default);
    Task<ElevatedOperationResponse> CreateSystemRestorePointAsync(
        CancellationToken cancellationToken = default);
    Task<ElevatedOperationResponse> RunCatalogRecipeAsync(
        string recipeId,
        string expectedRecipeHash,
        CancellationToken cancellationToken = default);
}

public static class ElevatedOperationPolicy
{
    private static readonly JsonSerializerOptions JsonOptions = new() { MaxDepth = 48 };
    public static bool TryResolveMachineTweaks(IEnumerable<string> tweakIds, out IReadOnlyList<Tweak> selection,
        out string reason)
    {
        var ids = tweakIds.Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        selection = [];
        reason = string.Empty;
        if (ids.Count is 0 or > 64 || ids.Any(string.IsNullOrWhiteSpace))
        {
            reason = "Select between 1 and 64 machine settings.";
            return false;
        }
        var catalog = TweakCatalog.Create().ToDictionary(tweak => tweak.Id, StringComparer.OrdinalIgnoreCase);
        var resolved = new List<Tweak>();
        foreach (var id in ids)
        {
            if (!catalog.TryGetValue(id, out var tweak) || !IsElevatedOptimizeTweak(tweak))
            {
                reason = $"This machine setting or repair command is not supported: {id}";
                return false;
            }
            resolved.Add(tweak);
        }
        selection = resolved;
        return true;
    }

    public static bool IsElevatedOptimizeTweak(Tweak tweak) =>
        tweak.Kind == TweakKind.Registry &&
        tweak.Target.StartsWith("HKLM\\", StringComparison.OrdinalIgnoreCase) ||
        (tweak.Kind == TweakKind.Command && tweak.RequiresElevation &&
         TweakCatalog.ElevatedCommandIds.Contains(tweak.Id));

    public static bool TryResolveServiceAction(
        string? serviceName,
        ServiceActionKind action,
        ServiceObservedState expectedState,
        out ServiceActionTarget target, out string reason)
    {
        target = new ServiceActionTarget(string.Empty, string.Empty, expectedState);
        reason = string.Empty;
        if (string.IsNullOrWhiteSpace(serviceName) || serviceName.Length > 256 ||
            serviceName.Any(character => char.IsControl(character) || character is '\\' or '/' or '"') ||
            action is not (ServiceActionKind.Start or ServiceActionKind.Restart) ||
            expectedState is not (ServiceObservedState.Stopped or ServiceObservedState.Running) ||
            action == ServiceActionKind.Start && expectedState != ServiceObservedState.Stopped ||
            action == ServiceActionKind.Restart && expectedState != ServiceObservedState.Running)
        {
            reason = "The service name, action, or current state is invalid.";
            return false;
        }
        var current = WindowsServiceMonitor.FindExact(serviceName);
        if (current is null)
        {
            reason = "The exact registered service is no longer present or readable.";
            return false;
        }
        if (current.StartType.Equals("Disabled", StringComparison.OrdinalIgnoreCase))
        {
            reason = "The service start type is Disabled; Sift will not change its configuration implicitly.";
            return false;
        }
        if (!current.CanManage)
        {
            WindowsServiceMonitor.CanManageName(current.Name, out reason);
            return false;
        }
        if (!current.Status.Equals(expectedState.ToString(), StringComparison.OrdinalIgnoreCase))
        {
            reason = $"The service state changed from {expectedState} to {current.Status}; nothing was changed.";
            return false;
        }
        target = new ServiceActionTarget(current.Name, current.DisplayName, expectedState);
        return true;
    }

    public static SiftResult<ScriptRecipe> ResolveCatalogRecipe(string? recipeId, string? expectedRecipeHash)
    {
        if (!ScriptRecipeIdentity.IsValidRecipeId(recipeId) ||
            !ScriptRecipeIdentity.IsValidHash(expectedRecipeHash))
            return SiftResult<ScriptRecipe>.Fail(SiftReasonCode.ElevationRecipeIdentityInvalid);

        var canonical = ScriptRecipeCatalog.Create().SingleOrDefault(item =>
            string.Equals(item.Id, recipeId, StringComparison.OrdinalIgnoreCase));
        if (canonical is null || !canonical.RequiresAdministrator)
            return SiftResult<ScriptRecipe>.Fail(SiftReasonCode.ElevationRecipeNotAdministrator);

        var hash = ScriptRecipeIdentity.ComputeHash(canonical);
        if (!string.Equals(hash, expectedRecipeHash, StringComparison.Ordinal))
            return SiftResult<ScriptRecipe>.Fail(SiftReasonCode.ElevationRecipeHashMismatch);

        return SiftResult<ScriptRecipe>.Ok(canonical);
    }

    public static bool TryResolveCatalogRecipe(
        string? recipeId,
        string? expectedRecipeHash,
        out ScriptRecipe recipe,
        out string reason)
    {
        var resolved = ResolveCatalogRecipe(recipeId, expectedRecipeHash);
        recipe = resolved.Value!;
        reason = resolved.Message;
        return resolved.IsSuccess;
    }

    public static bool TryValidateRequestShape(ElevatedOperationRequest request, out string reason)
    {
        reason = string.Empty;
        if (!Guid.TryParseExact(request.RequestId, "N", out _))
        {
            reason = "The administrator request ID is invalid.";
            return false;
        }
        if (request.Nonce.Length != 64 || !request.Nonce.All(character =>
                character is >= '0' and <= '9' or >= 'A' and <= 'F'))
        {
            reason = "The administrator request token is invalid.";
            return false;
        }
        if (request.TweakIds.Count > 64 || request.TweakIds.Any(string.IsNullOrWhiteSpace))
        {
            reason = "The administrator request contains too many settings or an empty setting ID.";
            return false;
        }
        if (!string.IsNullOrWhiteSpace(request.ServiceName) &&
            (request.ServiceName.Length > 256 ||
             request.ServiceName.Any(character => char.IsControl(character) || character is '\\' or '/' or '"')))
        {
            reason = "The service name is invalid.";
            return false;
        }
        if (!string.IsNullOrWhiteSpace(request.BackupFileName) && request.BackupFileName.Length > 160)
        {
            reason = "The backup file name is too long.";
            return false;
        }
        if (!string.IsNullOrWhiteSpace(request.ExpectedTaskDefinitionHash) &&
            (request.ExpectedTaskDefinitionHash.Length != 64 ||
             !request.ExpectedTaskDefinitionHash.All(character =>
                 character is >= '0' and <= '9' or >= 'A' and <= 'F')))
        {
            reason = "The scheduled task no longer matches the reviewed definition.";
            return false;
        }
        if (!string.IsNullOrWhiteSpace(request.ExpectedRecipeHash) &&
            !ScriptRecipeIdentity.IsValidHash(request.ExpectedRecipeHash))
        {
            reason = "The catalog recipe identity hash is invalid.";
            return false;
        }
        if (!string.IsNullOrWhiteSpace(request.RecipeId) &&
            !ScriptRecipeIdentity.IsValidRecipeId(request.RecipeId))
        {
            reason = "The catalog recipe ID is invalid.";
            return false;
        }

        bool Has(params object?[] values) => values.Any(value => value switch
        {
            null => false,
            string text => !string.IsNullOrWhiteSpace(text),
            IReadOnlyList<string> list => list.Count > 0,
            _ => true
        });

        switch (request.Operation)
        {
            case ElevatedOperationKind.ApplyMachineTweaks:
                if (Has(request.BackupFileName, request.ServiceName, request.ServiceAction,
                        request.ExpectedServiceState, request.ScheduledTaskId,
                        request.ScheduledTaskChange, request.ExpectedTaskEnabled, request.ExpectedTaskDefinitionHash,
                        request.RecipeId, request.ExpectedRecipeHash))
                {
                    reason = "The machine tweak request contains unrelated fields.";
                    return false;
                }
                break;
            case ElevatedOperationKind.RestoreMachineBackup:
                if (Has(request.TweakIds, request.ServiceName, request.ServiceAction,
                        request.ExpectedServiceState, request.ScheduledTaskId,
                        request.ScheduledTaskChange, request.ExpectedTaskEnabled, request.ExpectedTaskDefinitionHash,
                        request.RecipeId, request.ExpectedRecipeHash) ||
                    string.IsNullOrWhiteSpace(request.BackupFileName))
                {
                    reason = "The restore request is incomplete or contains unrelated fields.";
                    return false;
                }
                break;
            case ElevatedOperationKind.ValidateElevation:
                if (Has(request.TweakIds, request.BackupFileName, request.ServiceName, request.ServiceAction,
                        request.ExpectedServiceState,
                        request.ScheduledTaskId, request.ScheduledTaskChange, request.ExpectedTaskEnabled,
                        request.ExpectedTaskDefinitionHash, request.RecipeId, request.ExpectedRecipeHash))
                {
                    reason = "The elevation validation request contains unrelated fields.";
                    return false;
                }
                break;
            case ElevatedOperationKind.ManageService:
                if (Has(request.TweakIds, request.BackupFileName, request.ScheduledTaskId, request.ScheduledTaskChange,
                        request.ExpectedTaskEnabled, request.ExpectedTaskDefinitionHash,
                        request.RecipeId, request.ExpectedRecipeHash) ||
                    string.IsNullOrWhiteSpace(request.ServiceName) ||
                    request.ServiceAction is not (ServiceActionKind.Start or ServiceActionKind.Restart) ||
                    request.ExpectedServiceState is not (ServiceObservedState.Stopped or ServiceObservedState.Running) ||
                    request.ServiceAction == ServiceActionKind.Start &&
                    request.ExpectedServiceState != ServiceObservedState.Stopped ||
                    request.ServiceAction == ServiceActionKind.Restart &&
                    request.ExpectedServiceState != ServiceObservedState.Running)
                {
                    reason = "The service request is incomplete or contains unrelated fields.";
                    return false;
                }
                break;
            case ElevatedOperationKind.ChangeScheduledTask:
                if (Has(request.TweakIds, request.BackupFileName, request.ServiceName, request.ServiceAction,
                        request.ExpectedServiceState, request.RecipeId, request.ExpectedRecipeHash) ||
                    request.ScheduledTaskId is null || request.ScheduledTaskChange is null ||
                    request.ExpectedTaskEnabled is null ||
                    string.IsNullOrWhiteSpace(request.ExpectedTaskDefinitionHash))
                {
                    reason = "The scheduled-task request is incomplete or contains unrelated fields.";
                    return false;
                }
                if (!ScheduledTaskIdentityCatalog.TryResolve(
                        ScheduledTaskIdentityCatalog.Resolve(request.ScheduledTaskId.Value).TaskPath,
                        ScheduledTaskIdentityCatalog.Resolve(request.ScheduledTaskId.Value).TaskName,
                        out _))
                {
                    reason = "This scheduled task is not supported.";
                    return false;
                }
                break;
            case ElevatedOperationKind.CreateSystemRestorePoint:
                if (Has(request.TweakIds, request.BackupFileName, request.ServiceName, request.ServiceAction,
                        request.ExpectedServiceState,
                        request.ScheduledTaskId, request.ScheduledTaskChange, request.ExpectedTaskEnabled,
                        request.ExpectedTaskDefinitionHash, request.RecipeId, request.ExpectedRecipeHash))
                {
                    reason = "The restore-point request contains unrelated fields.";
                    return false;
                }
                break;
            case ElevatedOperationKind.RunCatalogRecipe:
                if (Has(request.TweakIds, request.BackupFileName, request.ServiceName, request.ServiceAction,
                        request.ExpectedServiceState, request.ScheduledTaskId, request.ScheduledTaskChange,
                        request.ExpectedTaskEnabled, request.ExpectedTaskDefinitionHash) ||
                    string.IsNullOrWhiteSpace(request.RecipeId) ||
                    string.IsNullOrWhiteSpace(request.ExpectedRecipeHash))
                {
                    reason = ReasonMessages.Format(SiftReasonCode.ElevationRequestShapeInvalid);
                    return false;
                }
                if (!TryResolveCatalogRecipe(request.RecipeId, request.ExpectedRecipeHash, out _, out reason))
                    return false;
                break;
            default:
                reason = "This administrator operation is not supported.";
                return false;
        }

        return true;
    }

    public static bool TryValidateMachineRestore(string backupPath, out string reason)
    {
        reason = string.Empty;
        try
        {
            var info = new FileInfo(backupPath);
            if (!info.Exists || info.Length is <= 0 or > 4 * 1024 * 1024)
            {
                reason = "The recovery backup is missing, empty, or larger than 4 MB.";
                return false;
            }
            var backup = JsonSerializer.Deserialize<Backup>(File.ReadAllText(backupPath), JsonOptions);
            if (backup is null || backup.SchemaVersion is < 1 or > 2 || backup.Entries.Count is 0 or > 256 ||
                !string.Equals(backup.MachineName, Environment.MachineName, StringComparison.OrdinalIgnoreCase))
            {
                reason = "The recovery backup schema, entry count, or machine identity is invalid.";
                return false;
            }

            var catalog = TweakCatalog.Create().ToDictionary(tweak => tweak.Id, StringComparer.OrdinalIgnoreCase);
            var machineCount = 0;
            foreach (var entry in backup.Entries.Where(ShouldRestore))
            {
                if (entry.RegistryTree is not null)
                {
                    if (string.Equals(entry.RegistryHive, "HKLM", StringComparison.OrdinalIgnoreCase))
                    {
                        reason = "Unsigned HKLM registry-tree snapshots cannot cross the elevation boundary.";
                        return false;
                    }
                    continue;
                }
                if (!catalog.TryGetValue(entry.TweakId, out var tweak)) continue;
                var isMachine = tweak.Kind == TweakKind.Command || tweak.Kind == TweakKind.Registry &&
                    tweak.Target.StartsWith("HKLM\\", StringComparison.OrdinalIgnoreCase);
                if (!isMachine) continue;
                machineCount++;
                if (machineCount > 64 || !tweak.Reversible)
                {
                    reason = "The machine recovery batch exceeds its limit or contains an irreversible action.";
                    return false;
                }
                if (tweak.Kind == TweakKind.Command)
                {
                    if (!tweak.Id.Equals("power.hibernate", StringComparison.OrdinalIgnoreCase) ||
                        entry.RegistryTree is not null || entry.RegistryValue is not null)
                    {
                        reason = "This backup contains an unsupported machine recovery command.";
                        return false;
                    }
                    continue;
                }
                if (!ValidateMachineRegistrySnapshot(tweak, entry, out reason)) return false;
            }
            if (machineCount == 0)
            {
                reason = "This backup has no pending machine changes to restore.";
                return false;
            }
            return true;
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException or JsonException)
        {
            reason = $"The recovery backup could not be validated: {exception.Message}";
            return false;
        }
    }

    private static bool ValidateMachineRegistrySnapshot(Tweak tweak, BackupEntry entry, out string reason)
    {
        reason = string.Empty;
        if (!entry.Existed) return true;
        int prior;
        if (entry.RegistryValue is not null)
        {
            if (!entry.RegistryValue.Name.Equals(tweak.ValueName, StringComparison.OrdinalIgnoreCase) ||
                !entry.RegistryValue.Kind.Equals("DWord", StringComparison.OrdinalIgnoreCase) ||
                !entry.RegistryValue.Encoding.Equals("Int32", StringComparison.Ordinal) ||
                !int.TryParse(entry.RegistryValue.Data, NumberStyles.Integer, CultureInfo.InvariantCulture, out prior))
            {
                reason = $"The prior registry snapshot for {tweak.Id} is not the expected DWORD value.";
                return false;
            }
        }
        else if (!string.Equals(entry.RegistryKind, "DWord", StringComparison.OrdinalIgnoreCase) ||
                 !int.TryParse(entry.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out prior))
        {
            reason = $"The legacy prior registry snapshot for {tweak.Id} is invalid.";
            return false;
        }
        var allowed = tweak.Id.ToLowerInvariant() switch
        {
            "privacy.activity" => prior is 0 or 1,
            "privacy.telemetry" => prior is >= 0 and <= 3,
            "network.delivery" => prior is 0 or 1 or 2 or 3 or 99 or 100,
            _ => false
        };
        if (!allowed) reason = $"The prior value for {tweak.Id} cannot be restored.";
        return allowed;
    }

    private static bool ShouldRestore(BackupEntry entry) => entry.State != BackupEntryStates.Restored &&
        (entry.AppliedSuccessfully || entry.State is BackupEntryStates.Applying or BackupEntryStates.Applied);
}

public static class ElevationOperationFiles
{
    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };
    public const long MaximumRequestBytes = 64 * 1024;
    public static string RootDirectory => ProductPaths.ElevationDirectory;

    public static (string RequestPath, string ResponsePath) PathsFor(string requestId)
    {
        if (!Guid.TryParseExact(requestId, "N", out _)) throw new ArgumentException("Invalid request id.", nameof(requestId));
        return (Path.Combine(RootDirectory, requestId + ".request.json"),
            Path.Combine(RootDirectory, requestId + ".response.json"));
    }

    public static (string RequestPath, string ResponsePath) PathsBesideRequest(string requestPath, string requestId)
    {
        ValidateExactPath(requestPath, ".request.json");
        if (!Guid.TryParseExact(requestId, "N", out _)) throw new ArgumentException("Invalid request id.", nameof(requestId));
        var directory = Path.GetDirectoryName(Path.GetFullPath(requestPath))!;
        return (Path.Combine(directory, requestId + ".request.json"),
            Path.Combine(directory, requestId + ".response.json"));
    }

    public static void WriteRequest(string path, ElevatedOperationRequest request) =>
        WriteNew(path, JsonSerializer.Serialize(request, JsonOptions), ".request.json");

    public static FileStream WriteRequestLease(string path, ElevatedOperationRequest request)
    {
        ValidateExactPath(path, ".request.json");
        var bytes = System.Text.Encoding.UTF8.GetBytes(JsonSerializer.Serialize(request, JsonOptions));
        if (bytes.LongLength > MaximumRequestBytes) throw new InvalidDataException("The elevation payload is too large.");
        var stream = CreateNewNoFollow(path, FileAccess.ReadWrite, FileShare.Read,
            FILE_FLAG_WRITE_THROUGH | FILE_FLAG_SEQUENTIAL_SCAN);
        try
        {
            stream.Write(bytes);
            stream.Flush(flushToDisk: true);
            stream.Position = 0;
            return stream;
        }
        catch
        {
            stream.Dispose();
            throw;
        }
    }

    public static void WriteResponse(string path, ElevatedOperationResponse response) =>
        WriteNew(path, JsonSerializer.Serialize(response, JsonOptions), ".response.json");

    public static ElevatedOperationRequest ReadRequest(string path)
    {
        using var stream = OpenRequestReadLease(path);
        return ReadRequest(stream);
    }

    public static FileStream OpenRequestReadLease(string path)
    {
        ValidateExactPath(path, ".request.json");
        // The broker intentionally keeps a read/write lease whose share mode denies every other
        // writer. The helper must share that existing write access even though it reads only.
        return new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite, 4096,
            FileOptions.SequentialScan);
    }

    public static ElevatedOperationRequest ReadRequest(FileStream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (stream.Length > MaximumRequestBytes) throw new InvalidDataException("The elevation request is too large.");
        stream.Position = 0;
        using var reader = new StreamReader(stream, System.Text.Encoding.UTF8,
            detectEncodingFromByteOrderMarks: true, leaveOpen: true);
        var json = reader.ReadToEnd();
        return JsonSerializer.Deserialize<ElevatedOperationRequest>(json, JsonOptions)
            ?? throw new InvalidDataException("The elevation request is empty or malformed.");
    }

    public static ElevatedOperationResponse ReadResponse(string path)
    {
        ValidateExactPath(path, ".response.json");
        if (new FileInfo(path).Length > MaximumRequestBytes) throw new InvalidDataException("The elevation response is too large.");
        return JsonSerializer.Deserialize<ElevatedOperationResponse>(File.ReadAllText(path), JsonOptions)
            ?? throw new InvalidDataException("The elevation response is empty or malformed.");
    }

    public static void ValidateExactPath(string path, string suffix)
    {
        var full = Path.GetFullPath(path);
        var root = ValidateRequestDirectory(Path.GetDirectoryName(full));
        var name = Path.GetFileName(full);
        if (!name.EndsWith(suffix, StringComparison.OrdinalIgnoreCase) ||
            !Guid.TryParseExact(name[..^suffix.Length], "N", out _))
            throw new InvalidDataException("The elevation file name is invalid.");
        if (IsReparsePointByName(full))
            throw new InvalidDataException("Elevation request reparse points are blocked.");
    }

    private static string ValidateRequestDirectory(string? directory)
    {
        if (string.IsNullOrWhiteSpace(directory))
            throw new InvalidDataException("The elevation file has no request directory.");
        var root = Path.GetFullPath(directory).TrimEnd(Path.DirectorySeparatorChar);
        var current = Path.GetFullPath(RootDirectory).TrimEnd(Path.DirectorySeparatorChar);
        if (!string.Equals(root, current, StringComparison.OrdinalIgnoreCase))
        {
            var elevation = new DirectoryInfo(root);
            var sift = elevation.Parent;
            var local = sift?.Parent;
            var appData = local?.Parent;
            var profile = appData?.Parent;
            if (!elevation.Name.Equals("Elevation", StringComparison.OrdinalIgnoreCase) ||
                sift is null || !sift.Name.Equals("Sift", StringComparison.OrdinalIgnoreCase) ||
                local is null || !local.Name.Equals("Local", StringComparison.OrdinalIgnoreCase) ||
                appData is null || !appData.Name.Equals("AppData", StringComparison.OrdinalIgnoreCase) ||
                profile is null)
                throw new InvalidDataException("The elevation file is outside an exact user AppData request directory.");
            var driveRoot = Path.GetPathRoot(root);
            if (string.IsNullOrWhiteSpace(driveRoot) || new DriveInfo(driveRoot).DriveType != DriveType.Fixed)
                throw new InvalidDataException("Elevation requests must be on a fixed local profile drive.");
            foreach (var candidate in new[] { profile.FullName, appData.FullName, local.FullName, sift.FullName, elevation.FullName })
                if (Directory.Exists(candidate) && File.GetAttributes(candidate).HasFlag(FileAttributes.ReparsePoint))
                    throw new InvalidDataException("Elevation directory reparse points are blocked.");
        }
        else
        {
            var siftDirectory = Path.GetDirectoryName(root);
            if (Directory.Exists(root) && File.GetAttributes(root).HasFlag(FileAttributes.ReparsePoint) ||
                !string.IsNullOrWhiteSpace(siftDirectory) && Directory.Exists(siftDirectory) &&
                File.GetAttributes(siftDirectory).HasFlag(FileAttributes.ReparsePoint))
                throw new InvalidDataException("Elevation directory reparse points are blocked.");
        }
        RejectIfWorldWritable(root);
        return root;
    }

    // Defense in depth: the directory is authenticated by folder-name pattern, which a crafted tree
    // under a world-writable root could satisfy. A per-user profile directory is never writable by
    // Everyone/Authenticated Users/Users, so reject any request directory that is. Fail open if the
    // DACL cannot be read, so an unusual-but-legitimate per-user directory is never wrongly blocked.
    private static void RejectIfWorldWritable(string directory)
    {
        try
        {
            if (!Directory.Exists(directory)) return;
            var rules = new DirectoryInfo(directory).GetAccessControl()
                .GetAccessRules(true, true, typeof(System.Security.Principal.SecurityIdentifier));
            const FileSystemRights writeMask = FileSystemRights.WriteData | FileSystemRights.CreateFiles |
                FileSystemRights.AppendData | FileSystemRights.CreateDirectories | FileSystemRights.Write |
                FileSystemRights.Modify | FileSystemRights.FullControl;
            foreach (FileSystemAccessRule rule in rules)
            {
                if (rule.AccessControlType != AccessControlType.Allow) continue;
                if (rule.IdentityReference is not System.Security.Principal.SecurityIdentifier sid) continue;
                var untrusted = sid.IsWellKnown(System.Security.Principal.WellKnownSidType.WorldSid) ||
                    sid.IsWellKnown(System.Security.Principal.WellKnownSidType.AuthenticatedUserSid) ||
                    sid.IsWellKnown(System.Security.Principal.WellKnownSidType.BuiltinUsersSid);
                if (untrusted && (rule.FileSystemRights & writeMask) != 0)
                    throw new InvalidDataException("The elevation request directory is writable by untrusted principals.");
            }
        }
        catch (InvalidDataException)
        {
            throw;
        }
        catch
        {
            // The ACL could not be evaluated; do not block a legitimate per-user directory over it.
        }
    }

    public static string ResolveSiblingBackup(string requestPath, string? backupFileName)
    {
        ValidateExactPath(requestPath, ".request.json");
        if (string.IsNullOrWhiteSpace(backupFileName) || backupFileName != Path.GetFileName(backupFileName) ||
            !backupFileName.StartsWith("backup-", StringComparison.OrdinalIgnoreCase) ||
            !backupFileName.EndsWith(".json", StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException("The recovery request contains an invalid backup file name.");
        var elevationDirectory = Path.GetDirectoryName(Path.GetFullPath(requestPath))!;
        var siftDirectory = Directory.GetParent(elevationDirectory)?.FullName
            ?? throw new InvalidDataException("The elevation directory has no Sift parent.");
        var backupDirectory = Path.Combine(siftDirectory, "Backups");
        var backupPath = Path.Combine(backupDirectory, backupFileName);
        if (Directory.Exists(backupDirectory) && File.GetAttributes(backupDirectory).HasFlag(FileAttributes.ReparsePoint) ||
            File.Exists(backupPath) && File.GetAttributes(backupPath).HasFlag(FileAttributes.ReparsePoint))
            throw new InvalidDataException("Recovery backup reparse points are blocked.");
        return backupPath;
    }

    public static string ResolveSiblingBackupDirectory(string requestPath)
    {
        ValidateExactPath(requestPath, ".request.json");
        var elevationDirectory = Path.GetDirectoryName(Path.GetFullPath(requestPath))!;
        var siftDirectory = Directory.GetParent(elevationDirectory)?.FullName
            ?? throw new InvalidDataException("The elevation directory has no Sift parent.");
        var backupDirectory = Path.Combine(siftDirectory, "Backups");
        if (Directory.Exists(backupDirectory) &&
            File.GetAttributes(backupDirectory).HasFlag(FileAttributes.ReparsePoint))
            throw new InvalidDataException("The Sift backup directory is a reparse point.");
        Directory.CreateDirectory(backupDirectory);
        if (File.GetAttributes(backupDirectory).HasFlag(FileAttributes.ReparsePoint))
            throw new InvalidDataException("The Sift backup directory changed during validation.");
        return backupDirectory;
    }

    private static void WriteNew(string path, string content, string suffix)
    {
        ValidateExactPath(path, suffix);
        var bytes = System.Text.Encoding.UTF8.GetBytes(content);
        if (bytes.LongLength > MaximumRequestBytes) throw new InvalidDataException("The elevation payload is too large.");
        using var stream = CreateNewNoFollow(path, FileAccess.Write, FileShare.None, FILE_FLAG_WRITE_THROUGH);
        stream.Write(bytes);
        stream.Flush(flushToDisk: true);
    }

    private const uint GENERIC_READ = 0x80000000;
    private const uint GENERIC_WRITE = 0x40000000;
    private const int FILE_FLAG_WRITE_THROUGH = unchecked((int)0x80000000);
    private const int FILE_FLAG_SEQUENTIAL_SCAN = 0x08000000;
    private const int FILE_FLAG_OPEN_REPARSE_POINT = 0x00200000;
    private const int FILE_ATTRIBUTE_REPARSE_POINT = 0x00000400;
    private const int ERROR_FILE_EXISTS = 80;
    private const int ERROR_ALREADY_EXISTS = 183;
    private static readonly IntPtr InvalidHandleValue = new(-1);

    // Creates a brand-new file while refusing to follow a reparse point at the final component.
    // FILE_FLAG_OPEN_REPARSE_POINT makes CREATE_NEW act on the link itself, so a pre-planted symlink
    // (including a dangling one, which File.Exists misses because it follows the link) fails with
    // ERROR_FILE_EXISTS instead of being resolved to an attacker-chosen target. Validation and open
    // are the same syscall, closing the TOCTOU window on the elevated write path.
    private static FileStream CreateNewNoFollow(string path, FileAccess access, FileShare share, int extraFlags)
    {
        var desired = access switch
        {
            FileAccess.Read => GENERIC_READ,
            FileAccess.Write => GENERIC_WRITE,
            _ => GENERIC_READ | GENERIC_WRITE
        };
        var handle = CreateFileW(path, desired, share, IntPtr.Zero, FileMode.CreateNew,
            FILE_FLAG_OPEN_REPARSE_POINT | extraFlags, IntPtr.Zero);
        if (handle.IsInvalid)
        {
            var error = Marshal.GetLastWin32Error();
            handle.Dispose();
            if (error is ERROR_FILE_EXISTS or ERROR_ALREADY_EXISTS)
                throw new InvalidDataException("The elevation file name already exists or resolves to a reparse point.");
            throw new Win32Exception(error);
        }
        return new FileStream(handle, access, 4096, isAsync: false);
    }

    // Reports whether the final path component is a reparse point (symlink/junction/mount point),
    // reading the parent directory entry without following the link, so dangling links are detected.
    private static bool IsReparsePointByName(string fullPath)
    {
        var handle = FindFirstFileW(fullPath, out var data);
        if (handle == InvalidHandleValue) return false;
        try { return (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0; }
        finally { FindClose(handle); }
    }

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern SafeFileHandle CreateFileW(string lpFileName, uint dwDesiredAccess, FileShare dwShareMode,
        IntPtr lpSecurityAttributes, FileMode dwCreationDisposition, int dwFlagsAndAttributes, IntPtr hTemplateFile);

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern IntPtr FindFirstFileW(string lpFileName, out WIN32_FIND_DATA lpFindFileData);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool FindClose(IntPtr hFindFile);

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct WIN32_FIND_DATA
    {
        public uint dwFileAttributes;
        public uint ftCreationLow, ftCreationHigh;
        public uint ftLastAccessLow, ftLastAccessHigh;
        public uint ftLastWriteLow, ftLastWriteHigh;
        public uint nFileSizeHigh;
        public uint nFileSizeLow;
        public uint dwReserved0;
        public uint dwReserved1;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)] public string cFileName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 14)] public string cAlternateFileName;
    }
}

public sealed class ElevationBroker(string? helperPath = null) : IElevationBroker
{
    private readonly string _helperPath = helperPath ?? Path.Combine(
        AppContext.BaseDirectory, "ElevationHost", "Sift.ElevationHost.exe");

    public async Task<ElevatedOperationResponse> ApplyMachineTweaksAsync(IEnumerable<Tweak> selection,
        CancellationToken cancellationToken = default)
    {
        var selected = selection.ToList();
        cancellationToken.ThrowIfCancellationRequested();
        if (selected.Count == 0 || selected.Any(tweak => !ElevatedOperationPolicy.IsElevatedOptimizeTweak(tweak)))
            return Failure("The administrator process accepts only supported machine settings and repair commands.");
        var requestId = Guid.NewGuid().ToString("N");
        var nonce = Convert.ToHexString(System.Security.Cryptography.RandomNumberGenerator.GetBytes(32));
        var request = new ElevatedOperationRequest(requestId, nonce, ElevatedOperationKind.ApplyMachineTweaks,
            selected.Select(tweak => tweak.Id).Distinct(StringComparer.OrdinalIgnoreCase).ToList());
        if (!ElevatedOperationPolicy.TryResolveMachineTweaks(request.TweakIds, out _, out var policyReason))
            return Failure(policyReason, requestId, nonce);
        return await ExecuteAsync(request, cancellationToken);
    }

    public Task<ElevatedOperationResponse> RestoreMachineBackupAsync(string backupPath,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var full = Path.GetFullPath(backupPath);
        var expectedDirectory = ProductPaths.BackupDirectory;
        if (!string.Equals(Path.GetDirectoryName(full), Path.GetFullPath(expectedDirectory),
                StringComparison.OrdinalIgnoreCase) || !File.Exists(full))
            return Task.FromResult(Failure("Machine recovery is limited to an exact Sift backup file."));
        if (!ElevatedOperationPolicy.TryValidateMachineRestore(full, out var reason))
            return Task.FromResult(Failure(reason));
        var request = new ElevatedOperationRequest(Guid.NewGuid().ToString("N"),
            Convert.ToHexString(System.Security.Cryptography.RandomNumberGenerator.GetBytes(32)),
            ElevatedOperationKind.RestoreMachineBackup, [], Path.GetFileName(full));
        return ExecuteAsync(request, cancellationToken);
    }

    public Task<ElevatedOperationResponse> ManageServiceAsync(ServiceActionTarget confirmedTarget,
        ServiceActionKind action,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (!ElevatedOperationPolicy.TryResolveServiceAction(
                confirmedTarget.Name, action, confirmedTarget.ExpectedState, out var target, out var reason))
            return Task.FromResult(Failure(reason));
        var request = new ElevatedOperationRequest(Guid.NewGuid().ToString("N"),
            Convert.ToHexString(System.Security.Cryptography.RandomNumberGenerator.GetBytes(32)),
            ElevatedOperationKind.ManageService, [], ServiceName: target.Name, ServiceAction: action,
            ExpectedServiceState: target.ExpectedState);
        return ExecuteAsync(request, cancellationToken);
    }

    public Task<ElevatedOperationResponse> ChangeScheduledTaskAsync(
        ScheduledTaskId id,
        ScheduledTaskChange change,
        bool expectedEnabled,
        string expectedDefinitionHash,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (expectedDefinitionHash.Length != 64 ||
            !expectedDefinitionHash.All(character => character is >= '0' and <= '9' or >= 'A' and <= 'F'))
            return Task.FromResult(Failure("The scheduled task no longer matches the reviewed definition."));
        var request = new ElevatedOperationRequest(Guid.NewGuid().ToString("N"),
            Convert.ToHexString(System.Security.Cryptography.RandomNumberGenerator.GetBytes(32)),
            ElevatedOperationKind.ChangeScheduledTask, [],
            ScheduledTaskId: id,
            ScheduledTaskChange: change,
            ExpectedTaskEnabled: expectedEnabled,
            ExpectedTaskDefinitionHash: expectedDefinitionHash);
        if (!ElevatedOperationPolicy.TryValidateRequestShape(request, out var reason))
            return Task.FromResult(Failure(reason));
        return ExecuteAsync(request, cancellationToken);
    }

    public Task<ElevatedOperationResponse> CreateSystemRestorePointAsync(
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var request = new ElevatedOperationRequest(Guid.NewGuid().ToString("N"),
            Convert.ToHexString(System.Security.Cryptography.RandomNumberGenerator.GetBytes(32)),
            ElevatedOperationKind.CreateSystemRestorePoint, []);
        if (!ElevatedOperationPolicy.TryValidateRequestShape(request, out var reason))
            return Task.FromResult(Failure(reason));
        return ExecuteAsync(request, cancellationToken);
    }

    public Task<ElevatedOperationResponse> RunCatalogRecipeAsync(
        string recipeId,
        string expectedRecipeHash,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (!ElevatedOperationPolicy.TryResolveCatalogRecipe(recipeId, expectedRecipeHash, out _, out var reason))
            return Task.FromResult(Failure(reason));
        var request = new ElevatedOperationRequest(Guid.NewGuid().ToString("N"),
            Convert.ToHexString(System.Security.Cryptography.RandomNumberGenerator.GetBytes(32)),
            ElevatedOperationKind.RunCatalogRecipe, [],
            RecipeId: recipeId,
            ExpectedRecipeHash: expectedRecipeHash);
        if (!ElevatedOperationPolicy.TryValidateRequestShape(request, out var shapeReason))
            return Task.FromResult(Failure(shapeReason));
        return ExecuteAsync(request, cancellationToken);
    }

    private async Task<ElevatedOperationResponse> ExecuteAsync(ElevatedOperationRequest request,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (!File.Exists(_helperPath))
            return Failure("The Sift elevation helper is missing. Rebuild the complete release folder before protected operations.",
                request.RequestId, request.Nonce);
        if (!ElevationHelperTrust.TryValidate(_helperPath, out var helperReason))
            return Failure(helperReason, request.RequestId, request.Nonce);
        if (!ElevatedOperationPolicy.TryValidateRequestShape(request, out var shapeReason))
            return Failure(shapeReason, request.RequestId, request.Nonce);
        var requestId = request.RequestId;
        var nonce = request.Nonce;
        var paths = ElevationOperationFiles.PathsFor(requestId);
        Directory.CreateDirectory(ElevationOperationFiles.RootDirectory);
        using var requestLease = ElevationOperationFiles.WriteRequestLease(paths.RequestPath, request);
        using var backupLease = OpenBackupLease(request);

        try
        {
            Process? process;
            try
            {
                process = Process.Start(new ProcessStartInfo(_helperPath)
                {
                    UseShellExecute = true,
                    Verb = "runas",
                    Arguments = $"--request \"{paths.RequestPath}\"",
                    WorkingDirectory = Path.GetDirectoryName(_helperPath) ?? AppContext.BaseDirectory
                });
            }
            catch (Win32Exception exception) when (exception.NativeErrorCode == 1223)
            {
                return new ElevatedOperationResponse(requestId, nonce, false, true,
                    "Windows administrator confirmation was cancelled. No protected operation was performed.", 0, 0, []);
            }
            if (process is null) return Failure("Windows did not start the Sift elevation helper.", requestId, nonce);
            using (process)
                await process.WaitForExitAsync(CancellationToken.None);
            if (!File.Exists(paths.ResponsePath))
                return Failure("The elevation helper exited without a verifiable response.", requestId, nonce);
            var response = ElevationOperationFiles.ReadResponse(paths.ResponsePath);
            if (!string.Equals(response.RequestId, requestId, StringComparison.Ordinal) ||
                !string.Equals(response.Nonce, nonce, StringComparison.Ordinal))
                return Failure("The elevation helper response did not match this request.", requestId, nonce);
            return response;
        }
        finally
        {
            TryDelete(paths.RequestPath);
            TryDelete(paths.ResponsePath);
        }
    }

    private static FileStream? OpenBackupLease(ElevatedOperationRequest request)
    {
        if (request.Operation != ElevatedOperationKind.RestoreMachineBackup ||
            string.IsNullOrWhiteSpace(request.BackupFileName)) return null;
        var path = Path.Combine(ProductPaths.BackupDirectory, request.BackupFileName);
        return new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read, 4096,
            FileOptions.SequentialScan);
    }

    private static ElevatedOperationResponse Failure(string message, string requestId = "", string nonce = "") =>
        new(requestId, nonce, false, false, message, 0, 0, []);

    private static void TryDelete(string path)
    {
        try { if (File.Exists(path)) File.Delete(path); }
        catch { }
    }
}

internal static class ElevationHelperTrust
{
    public static bool TryValidate(string helperPath, out string reason)
    {
        try
        {
            var full = Path.GetFullPath(helperPath);
            var expected = Path.GetFullPath(Path.Combine(
                AppContext.BaseDirectory, "ElevationHost", "Sift.ElevationHost.exe"));
            if (!string.Equals(full, expected, StringComparison.OrdinalIgnoreCase))
            {
                reason = "The elevation helper path does not match Sift's fixed payload location.";
                return false;
            }
            if (File.GetAttributes(full).HasFlag(FileAttributes.ReparsePoint))
            {
                reason = "The Sift elevation helper is a reparse point and was blocked.";
                return false;
            }
            foreach (var directory in new[] { AppContext.BaseDirectory, Path.GetDirectoryName(full)! })
            {
                if (File.GetAttributes(directory).HasFlag(FileAttributes.ReparsePoint))
                {
                    reason = "The Sift elevation helper directory is a reparse point and was blocked.";
                    return false;
                }
            }
            var processPath = Environment.ProcessPath;
            if (!BinaryTrustPolicy.HaveSameTrustedSigner(processPath, full, out var trustReason))
            {
                reason = "Standard-user protected actions require a signed Sift application and matching elevation helper. " +
                         trustReason;
                return false;
            }
            reason = "Trusted Sift elevation helper.";
            return true;
        }
        catch (Exception exception)
        {
            reason = $"The Sift elevation helper could not be verified: {exception.Message}";
            return false;
        }
    }
}
