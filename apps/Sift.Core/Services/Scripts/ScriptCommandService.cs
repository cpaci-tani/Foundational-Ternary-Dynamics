using System.Diagnostics;
using Sift.Models;
using Sift.Presentation;

namespace Sift.Services;

public interface IScriptCommandService
{
    IReadOnlyList<ScriptRecipe> Catalog { get; }
    ScriptPreflight Preflight(ScriptRecipe recipe);
    Task<ScriptRunResult> RunAsync(ScriptRecipe recipe, ScriptPreflight approvedPreflight,
        Action<string, bool> output, CancellationToken cancellationToken = default);
}

public sealed class ScriptCommandService : IScriptCommandService
{
    private static readonly string SystemDirectory = Environment.SystemDirectory;
    private static readonly string WindowsDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Windows);
    private static readonly string TrustedPath = string.Join(Path.PathSeparator,
        SystemDirectory,
        Path.Combine(SystemDirectory, "Wbem"),
        Path.Combine(SystemDirectory, "WindowsPowerShell", "v1.0"),
        Path.Combine(SystemDirectory, "OpenSSH"),
        WindowsDirectory);

    private static readonly string[] Forbidden =
    [
        "invoke-webrequest", "curl ", "wget ", "irm ", "iex ", "downloadstring", "bitsadmin",
        "remove-item", " del ", "erase ", "format ", "diskpart", "bcdedit", "reg delete",
        "sc delete", "disable-windowsoptionalfeature", "set-mppreference", "add-mppreference",
        "stop-service windefend", "netsh advfirewall set allprofiles state off"
    ];

    public IReadOnlyList<ScriptRecipe> Catalog { get; } = ScriptRecipeCatalog.Create();

    public ScriptPreflight Preflight(ScriptRecipe recipe)
    {
        var canonical = Catalog.SingleOrDefault(item =>
            string.Equals(item.Id, recipe.Id, StringComparison.OrdinalIgnoreCase));
        if (canonical is null || canonical != recipe)
            return Block(recipe.Id, SiftReasonCode.ScriptCatalogMismatch);
        var normalized = $" {canonical.Command.ToLowerInvariant()} ";
        var forbidden = Forbidden.FirstOrDefault(normalized.Contains);
        if (forbidden is not null)
            return Block(canonical.Id, SiftReasonCode.ScriptForbiddenToken, forbidden.Trim());
        var executable = TrustedShellPath(canonical.Shell);
        if (!File.Exists(executable))
            return Block(canonical.Id, SiftReasonCode.ScriptHostMissing, canonical.ShellLabel, executable);
        var requiresElevationHop = canonical.RequiresAdministrator && !ElevationHelper.IsElevated();
        var arguments = ArgumentDisplay(canonical);
        var recipeHash = ScriptRecipeIdentity.ComputeHash(canonical);
        var networkEvidence = canonical.MayUseNetwork
            ? "Network behavior: this recipe may contact the named endpoint or a configured Windows service; it never downloads or executes scripts."
            : "Network behavior: no network retrieval requested; remote script and runtime retrieval remain blocked.";
        var evidence = new List<string>
        {
            $"Recipe: {canonical.Title} ({canonical.Id})", $"Shell: {canonical.ShellLabel}",
            $"Access: {canonical.AccessLabel}", $"Risk: {canonical.RiskLabel}",
            $"Trusted executable: {executable}", $"Command: {canonical.Command}",
            requiresElevationHop
                ? "Administrator: Windows will ask for administrator permission; the helper rechecks the catalog recipe before launch"
                : canonical.RequiresAdministrator
                    ? "Administrator: the current elevated token was verified and will be rechecked immediately before launch"
                    : $"Administrator: not required; child process inherits the current {(ElevationHelper.IsElevated() ? "elevated" : "standard-user")} token",
            networkEvidence, "Command source: exact bundled local catalog record",
            $"Output sensitivity: {(canonical.SensitiveOutput ? "may contain private or secret values; review before sharing" : "normal diagnostic output")}",
            $"Recipe identity: {recipeHash}"
        };
        return new ScriptPreflight(true, canonical.Id, executable, arguments, evidence,
            RequiresElevation: requiresElevationHop, RecipeHash: recipeHash);
    }

    public async Task<ScriptRunResult> RunAsync(ScriptRecipe recipe, ScriptPreflight approved,
        Action<string, bool> output, CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var canonical = Catalog.SingleOrDefault(item =>
            string.Equals(item.Id, recipe.Id, StringComparison.OrdinalIgnoreCase));
        if (canonical is null || canonical != recipe)
            throw new InvalidOperationException("Recipe security metadata no longer matches the canonical catalog.");
        var current = Preflight(canonical);
        if (!current.Allowed || !string.Equals(current.RecipeId, approved.RecipeId, StringComparison.Ordinal) ||
            !string.Equals(current.Executable, approved.Executable, StringComparison.OrdinalIgnoreCase) ||
            !string.Equals(current.Arguments, approved.Arguments, StringComparison.Ordinal))
            throw new InvalidOperationException("The command changed after review. Select it again.");
        if (canonical.RequiresAdministrator && !ElevationHelper.IsElevated())
            throw new InvalidOperationException("Administrator status changed before execution.");

        var started = Stopwatch.StartNew();
        using var process = new Process
        {
            StartInfo = CreateStartInfo(canonical, current.Executable),
            EnableRaisingEvents = true
        };
        process.OutputDataReceived += (_, e) => { if (e.Data is not null) output(e.Data, false); };
        process.ErrorDataReceived += (_, e) => { if (e.Data is not null) output(e.Data, true); };
        cancellationToken.ThrowIfCancellationRequested();
        process.Start();
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();
        try
        {
            await process.WaitForExitAsync(cancellationToken);
            process.WaitForExit();
        }
        catch (OperationCanceledException)
        {
            if (!process.HasExited)
            {
                try { process.Kill(entireProcessTree: true); }
                catch (Exception exception)
                {
                    throw new InvalidOperationException("Cancellation could not terminate the command process tree.", exception);
                }
                try { await process.WaitForExitAsync(CancellationToken.None).WaitAsync(TimeSpan.FromSeconds(5)); }
                catch (TimeoutException)
                {
                    throw new InvalidOperationException("The command process tree did not exit within five seconds of cancellation.");
                }
            }
            process.WaitForExit();
            return new ScriptRunResult(-1, true, started.Elapsed);
        }
        return new ScriptRunResult(process.ExitCode, false, started.Elapsed);
    }

    internal static ProcessStartInfo CreateStartInfo(ScriptRecipe recipe, string executable)
    {
        var info = new ProcessStartInfo(executable)
        {
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            WorkingDirectory = SystemDirectory
        };
        foreach (var argument in Arguments(recipe)) info.ArgumentList.Add(argument);
        info.Environment.Clear();
        AddEnvironment(info, "SystemRoot", WindowsDirectory);
        AddEnvironment(info, "windir", WindowsDirectory);
        AddEnvironment(info, "SystemDrive", Path.GetPathRoot(WindowsDirectory) ?? "C:\\");
        AddEnvironment(info, "ComSpec", Path.Combine(SystemDirectory, "cmd.exe"));
        AddEnvironment(info, "PATH", TrustedPath);
        AddEnvironment(info, "PATHEXT", ".COM;.EXE;.BAT;.CMD");
        AddEnvironment(info, "TEMP", Path.GetTempPath().TrimEnd(Path.DirectorySeparatorChar));
        AddEnvironment(info, "TMP", Path.GetTempPath().TrimEnd(Path.DirectorySeparatorChar));
        AddEnvironment(info, "USERPROFILE", Environment.GetFolderPath(Environment.SpecialFolder.UserProfile));
        AddEnvironment(info, "HOMEDRIVE", Environment.GetEnvironmentVariable("HOMEDRIVE") ?? "");
        AddEnvironment(info, "HOMEPATH", Environment.GetEnvironmentVariable("HOMEPATH") ?? "");
        AddEnvironment(info, "USERNAME", Environment.UserName);
        AddEnvironment(info, "COMPUTERNAME", Environment.MachineName);
        AddEnvironment(info, "PSModulePath", string.Join(Path.PathSeparator,
            Path.Combine(SystemDirectory, "WindowsPowerShell", "v1.0", "Modules"),
            Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "WindowsPowerShell", "Modules")));
        return info;
    }

    private static void AddEnvironment(ProcessStartInfo info, string name, string value)
    {
        if (!string.IsNullOrWhiteSpace(value)) info.Environment[name] = value;
    }

    internal static string TrustedShellPath(ScriptShell shell) => shell switch
    {
        ScriptShell.Cmd => Path.Combine(SystemDirectory, "cmd.exe"),
        ScriptShell.PowerShell => Path.Combine(SystemDirectory, "WindowsPowerShell", "v1.0", "powershell.exe"),
        ScriptShell.Bash => Path.Combine(SystemDirectory, "wsl.exe"),
        _ => throw new ArgumentOutOfRangeException(nameof(shell))
    };

    private static IReadOnlyList<string> Arguments(ScriptRecipe recipe) => recipe.Shell switch
    {
        ScriptShell.Cmd => ["/d", "/s", "/c", recipe.Command],
        ScriptShell.PowerShell => ["-NoLogo", "-NoProfile", "-NonInteractive", "-ExecutionPolicy", "RemoteSigned", "-Command", recipe.Command],
        ScriptShell.Bash => ["--exec", "bash", "--noprofile", "--norc", "-c", recipe.Command],
        _ => throw new ArgumentOutOfRangeException()
    };

    private static string ArgumentDisplay(ScriptRecipe recipe) =>
        string.Join(" ", Arguments(recipe).Select(argument => argument.Contains(' ') ? $"\"{argument}\"" : argument));

    private static ScriptPreflight Block(string recipeId, SiftReasonCode code, params object?[] args)
    {
        var fail = SiftResult.Fail(code, args);
        return new(false, recipeId, "", "", [fail.Message], fail.Message, ReasonCode: fail.ReasonCode);
    }
}
