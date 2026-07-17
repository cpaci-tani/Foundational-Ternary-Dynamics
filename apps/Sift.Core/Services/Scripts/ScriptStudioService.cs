using System.Diagnostics;
using System.Text.Json;
using System.Text.RegularExpressions;
using Sift.Models;
using Microsoft.Win32;

namespace Sift.Services;

public interface IScriptStudioService
{
    Task<IReadOnlyList<ScriptRuntime>> DiscoverRuntimesAsync(CancellationToken cancellationToken = default);
    Task<ScriptAnalysis> AnalyzeAsync(ScriptDocument document, CancellationToken cancellationToken = default);
}

public sealed partial class ScriptStudioService : IScriptStudioService
{
    private const int MaximumDocumentCharacters = 1_000_000;
    private static readonly TimeSpan AnalyzerTimeout = TimeSpan.FromSeconds(8);
    private static readonly string SystemDirectory = Environment.SystemDirectory;

    public Task<IReadOnlyList<ScriptRuntime>> DiscoverRuntimesAsync(CancellationToken cancellationToken = default) =>
        Task.Run<IReadOnlyList<ScriptRuntime>>(() =>
        {
            cancellationToken.ThrowIfCancellationRequested();
            var runtimes = new List<ScriptRuntime>
            {
                Runtime("cmd-system", ScriptLanguage.CommandPrompt, "Windows Command Processor",
                    Path.Combine(SystemDirectory, "cmd.exe"), "Windows system directory"),
                Runtime("powershell-windows", ScriptLanguage.PowerShell, "Windows PowerShell 5.1",
                    Path.Combine(SystemDirectory, "WindowsPowerShell", "v1.0", "powershell.exe"),
                    "Windows system directory"),
                Runtime("bash-wsl", ScriptLanguage.Bash, "Bash through WSL",
                    Path.Combine(SystemDirectory, "wsl.exe"), "Windows Subsystem for Linux")
            };

            AddKnownExecutable(runtimes, "powershell-7", ScriptLanguage.PowerShell, "PowerShell 7",
                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "PowerShell", "7", "pwsh.exe"),
                "Program Files");
            AddKnownExecutable(runtimes, "node-program-files", ScriptLanguage.JavaScript, "Node.js",
                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "nodejs", "node.exe"),
                "Program Files");

            foreach (var python in DiscoverRegisteredPython()) runtimes.Add(python);

            runtimes.Add(new ScriptRuntime("typescript-language-service", ScriptLanguage.TypeScript,
                "TypeScript language service", string.Empty, "Bundled editor tooling", true, true,
                "Bundled Monaco TypeScript worker; Core diagnostics are not connected in this phase."));

            return runtimes
                .GroupBy(runtime => $"{runtime.Language}|{runtime.ExecutablePath}", StringComparer.OrdinalIgnoreCase)
                .Select(group => group.First())
                .OrderBy(runtime => runtime.Language)
                .ThenByDescending(runtime => runtime.Available)
                .ThenBy(runtime => runtime.DisplayName, StringComparer.OrdinalIgnoreCase)
                .ToList();
        }, cancellationToken);

    public async Task<ScriptAnalysis> AnalyzeAsync(
        ScriptDocument document,
        CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(document);
        cancellationToken.ThrowIfCancellationRequested();

        if (document.Text.Length > MaximumDocumentCharacters)
            return Blocked(document, "CW0001", "The editor document exceeds the 1,000,000 character analysis limit.");
        if (ElevationHelper.IsElevated())
            return Blocked(document, "CW1004",
                "Authored Script Studio analysis is unavailable while Sift is elevated. Restart Sift normally so language tools cannot inherit an administrator token.");

        var diagnostics = AnalyzePolicy(document).ToList();
        var runtimes = await DiscoverRuntimesAsync(cancellationToken);
        var runtime = runtimes.SingleOrDefault(item =>
            string.Equals(item.Id, document.RuntimeId, StringComparison.Ordinal));
        var syntaxChecked = false;

        if (runtime is null || !runtime.Available)
        {
            diagnostics.Add(new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Error, "Runtime", "CW1001",
                "The selected runtime is unavailable. Select an installed runtime before syntax analysis."));
        }
        else if (runtime.Language != document.Language)
        {
            diagnostics.Add(new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Blocked, "Runtime", "CW1005",
                "The selected runtime language does not match the editor document."));
        }
        else if (!runtime.TrustedForAnalysis)
        {
            diagnostics.Add(new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Blocked, "Runtime", "CW1006",
                $"Windows did not establish local signature trust for this runtime. {runtime.TrustDetail}"));
        }
        else
        {
            var syntax = await AnalyzeSyntaxAsync(document, runtime, cancellationToken);
            syntaxChecked = syntax.Checked;
            diagnostics.AddRange(syntax.Diagnostics);
        }

        diagnostics = diagnostics
            .OrderBy(item => item.Line)
            .ThenBy(item => item.Column)
            .ThenByDescending(item => item.Severity)
            .ToList();

        var errors = diagnostics.Count(item => item.Severity is ScriptDiagnosticSeverity.Error or ScriptDiagnosticSeverity.Blocked);
        var warnings = diagnostics.Count(item => item.Severity == ScriptDiagnosticSeverity.Warning);
        var summary = errors == 0 && warnings == 0 && syntaxChecked
            ? "Analysis completed with no syntax or Sift policy findings. Authored-script execution remains disabled until an inspectable execution plan is available."
            : errors == 0 && warnings == 0
                ? "Sift policy analysis completed, but Core did not perform a syntax check for this language. Authored-script execution remains disabled."
            : $"Analysis completed with {errors:N0} blocking/error finding(s) and {warnings:N0} warning(s). Authored-script execution remains disabled.";

        return new ScriptAnalysis(document.Language, document.RuntimeId, diagnostics, syntaxChecked,
            CanExecute: false, summary);
    }

    internal static IReadOnlyList<ScriptDiagnostic> AnalyzePolicy(ScriptDocument document)
    {
        var findings = new List<ScriptDiagnostic>();
        foreach (var rule in PolicyRules.Where(rule => rule.Languages.Contains(document.Language)))
        {
            foreach (Match match in rule.Pattern.Matches(document.Text))
            {
                var (line, column) = PositionOf(document.Text, match.Index);
                findings.Add(new ScriptDiagnostic(line, column, rule.Severity, "Sift policy", rule.Code, rule.Message));
            }
        }

        if (document.Text.Contains('\0'))
            findings.Add(new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Blocked, "Sift policy", "CW0002",
                "NUL characters are not accepted in Script Studio documents."));
        return findings;
    }

    private static async Task<(bool Checked, IReadOnlyList<ScriptDiagnostic> Diagnostics)> AnalyzeSyntaxAsync(
        ScriptDocument document,
        ScriptRuntime runtime,
        CancellationToken cancellationToken)
    {
        if (document.Language == ScriptLanguage.CommandPrompt)
            return (false, [new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Information, "CMD analyzer", "CMD0000",
                "CMD has no non-executing system parser. Sift policy analysis completed, but syntax was not certified.")]);
        if (document.Language == ScriptLanguage.TypeScript)
            return (false, [new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Information, "TypeScript", "TS0000",
                "The Monaco TypeScript language service supplies editor diagnostics; the Core analyzer bridge is scheduled for the LSP phase.")]);

        // Re-verify the interpreter's reparse and Authenticode trust immediately before spawning it.
        // Trust was established at discovery, but auto-discovered runtimes can live under user-writable
        // paths, so the bytes about to run must be re-checked to be the bytes that were verified.
        var current = Runtime(runtime.Id, runtime.Language, runtime.DisplayName, runtime.ExecutablePath, runtime.Source);
        if (!current.Available || !current.TrustedForAnalysis)
            return (false, [new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Blocked, "Runtime", "CW1007",
                $"The selected runtime is no longer trusted for analysis at launch time. {current.TrustDetail}")]);

        var invocation = document.Language switch
        {
            ScriptLanguage.Python => new AnalyzerInvocation(runtime.ExecutablePath,
                ["-I", "-S", "-B", "-c", PythonAnalyzerProgram]),
            ScriptLanguage.PowerShell => new AnalyzerInvocation(runtime.ExecutablePath,
                ["-NoLogo", "-NoProfile", "-NonInteractive", "-Command", PowerShellAnalyzerProgram]),
            ScriptLanguage.Bash => new AnalyzerInvocation(runtime.ExecutablePath,
                ["--exec", "bash", "--noprofile", "--norc", "-n"]),
            ScriptLanguage.JavaScript => new AnalyzerInvocation(runtime.ExecutablePath, ["--check", "-"]),
            _ => throw new ArgumentOutOfRangeException(nameof(document.Language))
        };

        var result = await RunAnalyzerAsync(invocation, document.Text, cancellationToken);
        if (result.TimedOut)
            return (false, [new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Error, "Runtime", "CW1002",
                $"The {document.Language} syntax analyzer exceeded the {AnalyzerTimeout.TotalSeconds:N0}-second limit.")]);

        return document.Language switch
        {
            ScriptLanguage.Python => ParseJsonDiagnostic(result, "Python"),
            ScriptLanguage.PowerShell => ParseJsonDiagnostic(result, "PowerShell"),
            ScriptLanguage.Bash => ParseLineDiagnostic(result, "Bash", "BASH1001"),
            ScriptLanguage.JavaScript => ParseLineDiagnostic(result, "Node.js", "JS1001"),
            _ => (false, Array.Empty<ScriptDiagnostic>())
        };
    }

    private static async Task<AnalyzerResult> RunAnalyzerAsync(
        AnalyzerInvocation invocation,
        string input,
        CancellationToken cancellationToken)
    {
        var info = new ProcessStartInfo(invocation.Executable)
        {
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardInput = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            WorkingDirectory = SystemDirectory
        };
        foreach (var argument in invocation.Arguments) info.ArgumentList.Add(argument);
        info.Environment.Clear();
        info.Environment["SystemRoot"] = Environment.GetFolderPath(Environment.SpecialFolder.Windows);
        info.Environment["windir"] = Environment.GetFolderPath(Environment.SpecialFolder.Windows);
        info.Environment["PATH"] = SystemDirectory;
        info.Environment["PYTHONDONTWRITEBYTECODE"] = "1";
        info.Environment["POWERSHELL_TELEMETRY_OPTOUT"] = "1";

        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(AnalyzerTimeout);
        using var process = new Process { StartInfo = info };
        Task<string>? stdout = null;
        Task<string>? stderr = null;
        var started = false;
        try
        {
            timeout.Token.ThrowIfCancellationRequested();
            process.Start();
            started = true;
            stdout = ReadBoundedAsync(process.StandardOutput, 131_072, timeout.Token);
            stderr = ReadBoundedAsync(process.StandardError, 131_072, timeout.Token);
            await process.StandardInput.WriteAsync(input.AsMemory(), timeout.Token);
            process.StandardInput.Close();
            await process.WaitForExitAsync(timeout.Token);
            return new AnalyzerResult(process.ExitCode, await stdout, await stderr, TimedOut: false);
        }
        catch (OperationCanceledException)
        {
            if (started) await TerminateAnalyzerAsync(process);
            if (cancellationToken.IsCancellationRequested) cancellationToken.ThrowIfCancellationRequested();
            return new AnalyzerResult(-1, CompletedOutput(stdout), CompletedOutput(stderr), TimedOut: true);
        }
        catch
        {
            if (started) await TerminateAnalyzerAsync(process);
            throw;
        }
    }

    private static (bool Checked, IReadOnlyList<ScriptDiagnostic> Diagnostics) ParseJsonDiagnostic(
        AnalyzerResult result,
        string source)
    {
        if (result.ExitCode != 0)
            return RuntimeFailure(source, result);
        var output = result.StandardOutput;
        if (string.IsNullOrWhiteSpace(output)) return (true, Array.Empty<ScriptDiagnostic>());
        try
        {
            var value = JsonSerializer.Deserialize<AnalyzerDiagnostic>(output.Trim());
            if (value is null) throw new JsonException("Analyzer returned an empty diagnostic.");
            return (true, [new ScriptDiagnostic(Math.Max(1, value.Line), Math.Max(1, value.Column),
                ScriptDiagnosticSeverity.Error, source, $"{source.ToUpperInvariant()}1001", value.Message)]);
        }
        catch (JsonException exception)
        {
            return (false, [new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Error, source, "CW1003",
                $"The syntax analyzer returned an invalid response: {exception.Message}")]);
        }
    }

    private static (bool Checked, IReadOnlyList<ScriptDiagnostic> Diagnostics) ParseLineDiagnostic(
        AnalyzerResult result,
        string source,
        string code)
    {
        if (result.ExitCode == 0) return (true, Array.Empty<ScriptDiagnostic>());
        var message = string.IsNullOrWhiteSpace(result.StandardError)
            ? $"{source} reported invalid syntax."
            : result.StandardError.Trim();
        if (source == "Bash" && (message.Contains("Wsl/", StringComparison.OrdinalIgnoreCase) ||
            message.Contains("Windows Subsystem for Linux", StringComparison.OrdinalIgnoreCase) ||
            message.Contains("no installed distributions", StringComparison.OrdinalIgnoreCase)))
            return RuntimeFailure(source, result);
        var match = LineNumberPattern().Match(message);
        var line = match.Success && int.TryParse(match.Groups[1].Value, out var parsed) ? parsed : 1;
        return (true, [new ScriptDiagnostic(line, 1, ScriptDiagnosticSeverity.Error, source, code, message)]);
    }

    private static ScriptRuntime Runtime(
        string id,
        ScriptLanguage language,
        string displayName,
        string path,
        string source)
    {
        try
        {
            if (!File.Exists(path))
                return new ScriptRuntime(id, language, displayName, path, source, false, false, "The executable was not found.");
            if (HasReparsePoint(path))
                return new ScriptRuntime(id, language, displayName, path, source, true, false,
                    "Runtime paths containing reparse points are blocked.");
            var trust = AuthenticodeVerifier.Verify(path);
            return new ScriptRuntime(id, language, displayName, path, source, true,
                trust.Status == InstalledAppSignatureStatus.Trusted, trust.Detail);
        }
        catch (Exception exception)
        {
            return new ScriptRuntime(id, language, displayName, path, source, File.Exists(path), false,
                $"Runtime identity inspection failed: {exception.Message}");
        }
    }

    private static void AddKnownExecutable(
        ICollection<ScriptRuntime> runtimes,
        string id,
        ScriptLanguage language,
        string displayName,
        string path,
        string source)
    {
        if (File.Exists(path)) runtimes.Add(Runtime(id, language, displayName, path, source));
    }

    private static IEnumerable<ScriptRuntime> DiscoverRegisteredPython()
    {
        foreach (var registration in new[]
                 {
                     (Hive: RegistryHive.CurrentUser, Label: "HKEY_CURRENT_USER"),
                     (Hive: RegistryHive.LocalMachine, Label: "HKEY_LOCAL_MACHINE")
                 })
        foreach (var view in new[] { RegistryView.Registry64, RegistryView.Registry32 })
        {
            using var baseKey = RegistryKey.OpenBaseKey(registration.Hive, view);
            using var pythonCore = baseKey.OpenSubKey(@"SOFTWARE\Python\PythonCore", writable: false);
            if (pythonCore is null) continue;
            foreach (var version in pythonCore.GetSubKeyNames().Order(StringComparer.OrdinalIgnoreCase))
            {
                using var install = pythonCore.OpenSubKey($@"{version}\InstallPath", writable: false);
                var executable = install?.GetValue("ExecutablePath") as string;
                if (string.IsNullOrWhiteSpace(executable))
                {
                    var root = install?.GetValue(null) as string;
                    if (!string.IsNullOrWhiteSpace(root)) executable = Path.Combine(root, "python.exe");
                }
                if (string.IsNullOrWhiteSpace(executable) || !File.Exists(executable)) continue;
                executable = Path.GetFullPath(executable);
                var identity = $"{registration.Label}|{view}|{version}|{executable}";
                var suffix = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(
                    System.Text.Encoding.UTF8.GetBytes(identity)))[..16].ToLowerInvariant();
                yield return Runtime($"python-registered-{suffix}", ScriptLanguage.Python,
                    $"Python {version}", executable, $"{registration.Label} registry ({view})");
            }
        }
    }

    private static ScriptAnalysis Blocked(ScriptDocument document, string code, string message) =>
        new(document.Language, document.RuntimeId,
            [new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Blocked, "Sift policy", code, message)],
            SyntaxChecked: false, CanExecute: false, message);

    private static (int Line, int Column) PositionOf(string text, int index)
    {
        var line = 1;
        var lastBreak = -1;
        for (var i = 0; i < index; i++)
        {
            if (text[i] != '\n') continue;
            line++;
            lastBreak = i;
        }
        return (line, index - lastBreak);
    }

    private static async Task<string> ReadBoundedAsync(
        StreamReader reader,
        int maximumCharacters,
        CancellationToken cancellationToken)
    {
        var buffer = new char[8_192];
        var value = new System.Text.StringBuilder(Math.Min(maximumCharacters, 16_384));
        while (value.Length < maximumCharacters)
        {
            var count = await reader.ReadAsync(
                buffer.AsMemory(0, Math.Min(buffer.Length, maximumCharacters - value.Length)),
                cancellationToken);
            if (count == 0) break;
            value.Append(buffer, 0, count);
        }
        return value.ToString();
    }

    private static async Task TerminateAnalyzerAsync(Process process)
    {
        if (!process.HasExited)
        {
            try { process.Kill(entireProcessTree: true); }
            catch (Exception exception)
            {
                throw new InvalidOperationException("The analyzer process tree could not be terminated.", exception);
            }
        }
        try { await process.WaitForExitAsync(CancellationToken.None).WaitAsync(TimeSpan.FromSeconds(5)); }
        catch (TimeoutException exception)
        {
            throw new InvalidOperationException("The analyzer process tree did not exit within five seconds.", exception);
        }
    }

    private static string CompletedOutput(Task<string>? output) =>
        output is { IsCompletedSuccessfully: true } ? output.Result : string.Empty;

    private static (bool Checked, IReadOnlyList<ScriptDiagnostic> Diagnostics) RuntimeFailure(
        string source,
        AnalyzerResult result)
    {
        var detail = string.IsNullOrWhiteSpace(result.StandardError)
            ? $"{source} exited with code {result.ExitCode} without returning a syntax result."
            : result.StandardError.Trim();
        return (false, [new ScriptDiagnostic(1, 1, ScriptDiagnosticSeverity.Error, "Runtime", "CW1007", detail)]);
    }

    private static bool HasReparsePoint(string path)
    {
        var current = Path.GetFullPath(path);
        while (!string.IsNullOrWhiteSpace(current))
        {
            if ((File.GetAttributes(current) & FileAttributes.ReparsePoint) != 0) return true;
            var parent = Path.GetDirectoryName(current);
            if (string.Equals(parent, current, StringComparison.OrdinalIgnoreCase)) break;
            current = parent ?? string.Empty;
        }
        return false;
    }

    private sealed record PolicyRule(
        ScriptLanguage[] Languages,
        Regex Pattern,
        ScriptDiagnosticSeverity Severity,
        string Code,
        string Message);

    private static readonly PolicyRule[] PolicyRules =
    [
        new([ScriptLanguage.Python], new Regex(@"\b(eval|exec|compile|__import__)\s*\(", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Blocked, "CWPY001", "Dynamic Python execution cannot produce an inspectable Sift execution plan."),
        new([ScriptLanguage.Python], new Regex(@"\b(subprocess|ctypes|winreg|shutil)\b|\bos\s*\.\s*(system|remove|unlink|rmdir|rename|replace)\b", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Blocked, "CWPY002", "This Python API can launch opaque commands or mutate Windows state."),
        new([ScriptLanguage.Python], new Regex(@"\b(requests|urllib|httpx|socket)\b|\bpip\s+install\b", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Blocked, "CWPY003", "Network retrieval and runtime package installation are not available in Script Studio."),
        new([ScriptLanguage.Python], new Regex(@"\b(open|Path)\s*\([^\r\n]*(?:['""](?:w|a|x|\+)|write_text|write_bytes)", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Warning, "CWPY004", "This expression may write to the filesystem and requires a typed mutation planner before execution."),
        new([ScriptLanguage.PowerShell], new Regex(@"\b(Invoke-WebRequest|Invoke-RestMethod|Start-BitsTransfer|irm|iwr|curl|wget)\b", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Blocked, "CWPS001", "Remote retrieval is prohibited in Sift Script Studio."),
        new([ScriptLanguage.PowerShell], new Regex(@"\b(Invoke-Expression|iex|Start-Process|Add-Type)\b", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Blocked, "CWPS002", "Opaque or dynamically compiled PowerShell execution cannot be preflighted."),
        new([ScriptLanguage.PowerShell], new Regex(@"\b(Set|Add|Remove|Clear|New|Disable|Enable|Stop|Restart)-[A-Za-z0-9]+\b", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Warning, "CWPS003", "A state-changing PowerShell verb was found; execution requires a typed Core operation."),
        new([ScriptLanguage.Bash], new Regex(@"\b(curl|wget|git\s+clone|apt|dnf|yum|pacman)\b", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Blocked, "CWSH001", "Remote retrieval and package management are not available in Script Studio."),
        new([ScriptLanguage.Bash], new Regex(@"\b(rm|mv|cp|chmod|chown|sudo|systemctl|service|mount|umount)\b", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Warning, "CWSH002", "This shell command may mutate the filesystem or operating-system state."),
        new([ScriptLanguage.CommandPrompt], new Regex(@"\b(del|erase|copy|move|ren|reg\s+(add|delete)|sc\s+(config|delete|stop)|format|diskpart)\b", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Warning, "CWCMD001", "This CMD command may mutate Windows state and requires a typed Core operation."),
        new([ScriptLanguage.JavaScript, ScriptLanguage.TypeScript], new Regex(@"\b(fetch|XMLHttpRequest|axios|https?\s*\.|child_process|eval)\b|\bnpm\s+(install|i)\b", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Blocked, "CWJS001", "Remote, package, child-process, or dynamic execution is prohibited in Script Studio."),
        new([ScriptLanguage.JavaScript, ScriptLanguage.TypeScript], new Regex(@"\b(fs\s*\.|Deno\s*\.)\s*(write|append|rename|remove|mkdir)", RegexOptions.IgnoreCase),
            ScriptDiagnosticSeverity.Warning, "CWJS002", "This API may mutate the filesystem and requires a typed mutation planner.")
    ];

    private const string PythonAnalyzerProgram =
        "import ast,json,sys\n" +
        "source=sys.stdin.read()\n" +
        "try:\n ast.parse(source, filename='<sift>')\n" +
        "except SyntaxError as e:\n print(json.dumps({'line':e.lineno or 1,'column':e.offset or 1,'message':e.msg}))\n";

    private const string PowerShellAnalyzerProgram =
        "$source=[Console]::In.ReadToEnd(); $tokens=$null; $errors=$null; " +
        "[System.Management.Automation.Language.Parser]::ParseInput($source,[ref]$tokens,[ref]$errors)|Out-Null; " +
        "if($errors.Count -gt 0){$e=$errors[0]; @{line=$e.Extent.StartLineNumber;column=$e.Extent.StartColumnNumber;message=$e.Message}|ConvertTo-Json -Compress}";

    private sealed record AnalyzerInvocation(string Executable, IReadOnlyList<string> Arguments);
    private sealed record AnalyzerResult(int ExitCode, string StandardOutput, string StandardError, bool TimedOut);
    private sealed record AnalyzerDiagnostic(int Line, int Column, string Message);

    [GeneratedRegex(@"(?:line\s+|:)(\d+)(?::\d+)?", RegexOptions.IgnoreCase)]
    private static partial Regex LineNumberPattern();
}
