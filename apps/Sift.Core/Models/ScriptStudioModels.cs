namespace Sift.Models;

public enum ScriptLanguage
{
    PowerShell,
    Python,
    Bash,
    CommandPrompt,
    JavaScript,
    TypeScript
}

public enum ScriptDiagnosticSeverity
{
    Information,
    Warning,
    Error,
    Blocked
}

public sealed record ScriptRuntime(
    string Id,
    ScriptLanguage Language,
    string DisplayName,
    string ExecutablePath,
    string Source,
    bool Available,
    bool TrustedForAnalysis,
    string TrustDetail)
{
    public string StatusLabel => !Available ? "NOT FOUND" : TrustedForAnalysis ? "TRUSTED" : "BLOCKED";
    public string DisplayLabel => $"{DisplayName} · {StatusLabel}";
}

public sealed record ScriptDocument(
    ScriptLanguage Language,
    string RuntimeId,
    string FileName,
    string Text);

public sealed record ScriptDiagnostic(
    int Line,
    int Column,
    ScriptDiagnosticSeverity Severity,
    string Source,
    string Code,
    string Message);

public sealed record ScriptAnalysis(
    ScriptLanguage Language,
    string RuntimeId,
    IReadOnlyList<ScriptDiagnostic> Diagnostics,
    bool SyntaxChecked,
    bool CanExecute,
    string Summary);
