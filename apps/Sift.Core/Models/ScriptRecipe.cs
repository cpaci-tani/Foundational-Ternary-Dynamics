using Sift.Presentation;

namespace Sift.Models;

public enum ScriptShell { Cmd, PowerShell, Bash }
public enum ScriptRisk { ReadOnly, ChangesState, Advanced }

public sealed record ScriptRecipe(
    string Id, string Title, string Category, string Description, ScriptShell Shell,
    string Command, ScriptRisk Risk = ScriptRisk.ReadOnly, bool RequiresAdministrator = false,
    bool MayUseNetwork = false, bool SensitiveOutput = false)
{
    public string RiskLabel => Risk switch
    {
        ScriptRisk.ReadOnly => "READ ONLY",
        ScriptRisk.ChangesState => "CHANGES STATE",
        _ => "ADVANCED"
    };
    public string ShellLabel => Shell.ToString();
    public string AccessLabel => RequiresAdministrator ? "ADMINISTRATOR" : "STANDARD USER";
    public string CopyAutomationName => $"Copy {Title} command";
    public string QuickRunAutomationName => $"Insert and run {Title} command";
}

public sealed record ScriptPreflight(
    bool Allowed,
    string RecipeId,
    string Executable,
    string Arguments,
    IReadOnlyList<string> Evidence,
    string? BlockReason = null,
    bool RequiresElevation = false,
    string? RecipeHash = null,
    SiftReasonCode ReasonCode = SiftReasonCode.Unspecified);

public sealed record ScriptRunResult(int ExitCode, bool Cancelled, TimeSpan Duration);
