using Sift.Models;
using Sift.Services;

return await RunAsync(args);

static async Task<int> RunAsync(string[] args)
{
    string? requestPath = null;
    try
    {
        if (args.Length != 2 || !args[0].Equals("--request", StringComparison.OrdinalIgnoreCase)) return 2;
        requestPath = args[1];
        ElevationOperationFiles.ValidateExactPath(requestPath, ".request.json");
        using var requestLease = ElevationOperationFiles.OpenRequestReadLease(requestPath);
        var request = ElevationOperationFiles.ReadRequest(requestLease);
        var paths = ElevationOperationFiles.PathsBesideRequest(requestPath, request.RequestId);
        if (!string.Equals(Path.GetFullPath(paths.RequestPath), Path.GetFullPath(requestPath),
                StringComparison.OrdinalIgnoreCase)) return 3;

        ElevatedOperationResponse response;
        if (!ElevationHelper.IsElevated())
        {
            response = Failure(request, "The helper did not receive an administrator access token.");
        }
        else if (!ElevatedOperationPolicy.TryValidateRequestShape(request, out var shapeReason))
        {
            response = Failure(request, shapeReason);
        }
        else if (request.Operation != ElevatedOperationKind.ValidateElevation &&
                 !ElevatedOperationConsent.Confirm(request))
        {
            response = new ElevatedOperationResponse(request.RequestId, request.Nonce, false, true,
                "The administrator action was cancelled in the elevation helper; nothing was changed.",
                0, 0, ["CANCELLED at the administrator-side operation summary"]);
        }
        else if (request.Operation == ElevatedOperationKind.ApplyMachineTweaks)
        {
            response = await ApplyMachineTweaksAsync(requestPath, request);
        }
        else if (request.Operation == ElevatedOperationKind.RestoreMachineBackup)
        {
            response = await RestoreMachineBackupAsync(requestPath, request);
        }
        else if (request.Operation == ElevatedOperationKind.ValidateElevation)
        {
            response = new ElevatedOperationResponse(request.RequestId, request.Nonce, true, false,
                "The one-shot helper received an administrator token and completed a non-mutating IPC round trip.",
                0, 0, ["VALIDATED elevation token and nonce-bound response path"]);
        }
        else if (request.Operation == ElevatedOperationKind.ManageService)
        {
            response = ManageService(request);
        }
        else if (request.Operation == ElevatedOperationKind.ChangeScheduledTask)
        {
            response = ChangeScheduledTask(request);
        }
        else if (request.Operation == ElevatedOperationKind.CreateSystemRestorePoint)
        {
            response = CreateSystemRestorePoint(request);
        }
        else if (request.Operation == ElevatedOperationKind.RunCatalogRecipe)
        {
            response = await RunCatalogRecipeAsync(request);
        }
        else response = Failure(request, "The requested elevated operation is not allowlisted.");
        ElevationOperationFiles.WriteResponse(paths.ResponsePath, response);
        return response.Succeeded ? 0 : 1;
    }
    catch (Exception exception)
    {
        try
        {
            if (!string.IsNullOrWhiteSpace(requestPath))
            {
                var request = ElevationOperationFiles.ReadRequest(requestPath);
                var paths = ElevationOperationFiles.PathsBesideRequest(requestPath, request.RequestId);
                ElevationOperationFiles.WriteResponse(paths.ResponsePath,
                    Failure(request, $"The elevation helper rejected the request: {exception.Message}"));
            }
        }
        catch { }
        return 4;
    }
}

static async Task<ElevatedOperationResponse> ApplyMachineTweaksAsync(
    string requestPath,
    ElevatedOperationRequest request)
{
    if (!ElevatedOperationPolicy.TryResolveMachineTweaks(request.TweakIds, out var selected, out var reason))
        return Failure(request, reason);

    var backupDirectory = ElevationOperationFiles.ResolveSiblingBackupDirectory(requestPath);
    var result = await new TweakExecutor(backupDirectory).ApplyAsync(selected, dryRun: false);
    return new ElevatedOperationResponse(request.RequestId, request.Nonce, result.Failed == 0, false,
        result.Failed == 0
            ? $"Applied {result.Succeeded:N0} protected Optimize action(s) through the one-shot elevation helper."
            : $"The elevation helper applied {result.Succeeded:N0} and failed {result.Failed:N0} protected Optimize action(s).",
        result.Succeeded, result.Failed, result.Log, result.BackupPath);
}

static async Task<ElevatedOperationResponse> RestoreMachineBackupAsync(string requestPath,
    ElevatedOperationRequest request)
{
    var backupPath = ElevationOperationFiles.ResolveSiblingBackup(requestPath, request.BackupFileName);
    if (!ElevatedOperationPolicy.TryValidateMachineRestore(backupPath, out var reason))
        return Failure(request, reason);
    var catalog = TweakCatalog.Create().ToDictionary(tweak => tweak.Id, StringComparer.OrdinalIgnoreCase);
    var result = await new TweakExecutor(Path.GetDirectoryName(backupPath)).RestoreFromAsync(
        backupPath, catalog, RestoreScope.ElevatedMachine);
    return new ElevatedOperationResponse(request.RequestId, request.Nonce, result.Failed == 0, false,
        result.Failed == 0
            ? $"Restored {result.Restored:N0} protected backup entr{(result.Restored == 1 ? "y" : "ies")} through the one-shot elevation helper."
            : $"The elevation helper restored {result.Restored:N0} and failed {result.Failed:N0} protected backup entr{(result.Failed == 1 ? "y" : "ies")}.",
        result.Restored, result.Failed, result.Log, result.BackupPath);
}

static ElevatedOperationResponse ManageService(ElevatedOperationRequest request)
{
    if (!ElevatedOperationPolicy.TryValidateRequestShape(request, out var shapeReason))
        return Failure(request, shapeReason);
    if (!ElevatedOperationPolicy.TryResolveServiceAction(
            request.ServiceName,
            request.ServiceAction ?? (ServiceActionKind)(-1),
            request.ExpectedServiceState ?? (ServiceObservedState)(-1),
            out var target,
            out var reason))
        return Failure(request, reason);
    var result = new GuardedSystemActions().ActOnServices([target], request.ServiceAction!.Value);
    return new ElevatedOperationResponse(request.RequestId, request.Nonce,
        result.Succeeded == 1 && result.Failed == 0 && result.Skipped == 0, false,
        result.Succeeded == 1
            ? $"{request.ServiceAction} completed for {target.DisplayName}."
            : $"The elevated service action did not complete for {target.DisplayName}.",
        result.Succeeded, result.Failed + result.Skipped, result.Log);
}

static ElevatedOperationResponse ChangeScheduledTask(ElevatedOperationRequest request)
{
    if (!ElevatedOperationPolicy.TryValidateRequestShape(request, out var shapeReason))
        return Failure(request, shapeReason);
    var controller = new ScheduledTaskController();
    var current = controller.Inspect(request.ScheduledTaskId!.Value);
    if (current is null ||
        current.Enabled != request.ExpectedTaskEnabled!.Value ||
        !string.Equals(current.DefinitionHash, request.ExpectedTaskDefinitionHash, StringComparison.Ordinal))
        return Failure(request, "The scheduled task changed after confirmation; nothing was changed.");

    var result = controller.SetEnabled(
        request.ScheduledTaskId.Value,
        request.ScheduledTaskChange!.Value,
        request.ExpectedTaskEnabled.Value,
        request.ExpectedTaskDefinitionHash!);
    return new ElevatedOperationResponse(request.RequestId, request.Nonce,
        result.Succeeded, result.Cancelled, result.Summary, result.Succeeded ? 1 : 0, result.Succeeded ? 0 : 1,
        result.Log);
}

static ElevatedOperationResponse CreateSystemRestorePoint(ElevatedOperationRequest request)
{
    if (!ElevatedOperationPolicy.TryValidateRequestShape(request, out var shapeReason))
        return Failure(request, shapeReason);
    var controller = new SystemRestorePointController();
    var inspection = controller.Inspect();
    var result = controller.Create(inspection);
    return new ElevatedOperationResponse(request.RequestId, request.Nonce,
        result.Succeeded, result.Cancelled, result.Message, result.Succeeded ? 1 : 0, result.Succeeded ? 0 : 1,
        result.Log);
}

static async Task<ElevatedOperationResponse> RunCatalogRecipeAsync(ElevatedOperationRequest request)
{
    if (!ElevatedOperationPolicy.TryValidateRequestShape(request, out var shapeReason))
        return Failure(request, shapeReason);
    if (!ElevatedOperationPolicy.TryResolveCatalogRecipe(
            request.RecipeId, request.ExpectedRecipeHash, out var recipe, out var reason))
        return Failure(request, reason);

    var service = new ScriptCommandService();
    var preflight = service.Preflight(recipe);
    if (!preflight.Allowed || preflight.RequiresElevation)
        return Failure(request, preflight.BlockReason ?? "The catalog recipe could not be checked under the administrator token.");

    var log = new List<string>
    {
        $"Recipe: {recipe.Title} ({recipe.Id})",
        $"Risk: {recipe.RiskLabel}",
        $"Shell: {recipe.ShellLabel}"
    };
    const int maxLogLines = 200;
    try
    {
        var result = await service.RunAsync(recipe, preflight, (line, error) =>
        {
            if (log.Count >= maxLogLines) return;
            log.Add(error ? $"[stderr] {line}" : line);
        }, CancellationToken.None);
        if (result.Cancelled)
            return new ElevatedOperationResponse(request.RequestId, request.Nonce, false, true,
                "The elevated catalog recipe was cancelled.", 0, 1, log);
        var succeeded = result.ExitCode == 0;
        return new ElevatedOperationResponse(request.RequestId, request.Nonce, succeeded, false,
            succeeded
                ? $"Completed {recipe.Title} with exit code {result.ExitCode}."
                : $"The elevated catalog recipe exited with code {result.ExitCode}.",
            succeeded ? 1 : 0, succeeded ? 0 : 1, log);
    }
    catch (Exception exception)
    {
        log.Add($"[error] {exception.Message}");
        return new ElevatedOperationResponse(request.RequestId, request.Nonce, false, false,
            $"The elevated catalog recipe failed: {exception.Message}", 0, 1, log);
    }
}

static ElevatedOperationResponse Failure(ElevatedOperationRequest request, string message) =>
    new(request.RequestId, request.Nonce, false, false, message, 0, 0, []);

static class ElevatedOperationConsent
{
    private const uint YesNo = 0x00000004;
    private const uint IconWarning = 0x00000030;
    private const uint DefaultButton2 = 0x00000100;
    private const uint SystemModal = 0x00001000;
    private const int Yes = 6;

    public static bool Confirm(ElevatedOperationRequest request)
    {
        var detail = request.Operation switch
        {
            ElevatedOperationKind.ApplyMachineTweaks =>
                "Apply these reviewed machine-wide Sift actions:\n\n" +
                string.Join("\n", request.TweakIds.Select(id => "• " + id)),
            ElevatedOperationKind.RestoreMachineBackup =>
                $"Restore protected entries from this Sift backup:\n\n{request.BackupFileName}",
            ElevatedOperationKind.ManageService =>
                $"{request.ServiceAction} the reviewed service:\n\n{request.ServiceName}",
            ElevatedOperationKind.ChangeScheduledTask =>
                $"{request.ScheduledTaskChange} the reviewed scheduled task:\n\n{request.ScheduledTaskId}",
            ElevatedOperationKind.CreateSystemRestorePoint =>
                "Create the reviewed Windows system restore point.",
            ElevatedOperationKind.RunCatalogRecipe =>
                ElevatedOperationPolicy.TryResolveCatalogRecipe(
                    request.RecipeId, request.ExpectedRecipeHash, out var recipe, out _)
                    ? $"Run this reviewed administrator catalog recipe:\n\n• {recipe.Title}\n• ID: {recipe.Id}\n• Risk: {recipe.RiskLabel}"
                    : $"Run the reviewed administrator catalog recipe:\n\n{request.RecipeId}",
            _ => "Run the reviewed Sift administrator action."
        };
        return MessageBox(IntPtr.Zero,
            detail + "\n\nChoose Yes only if this matches the action you reviewed in Sift.",
            "Confirm Sift administrator action",
            YesNo | IconWarning | DefaultButton2 | SystemModal) == Yes;
    }

    [System.Runtime.InteropServices.DllImport("user32.dll", CharSet = System.Runtime.InteropServices.CharSet.Unicode)]
    private static extern int MessageBox(IntPtr window, string text, string caption, uint type);
}
