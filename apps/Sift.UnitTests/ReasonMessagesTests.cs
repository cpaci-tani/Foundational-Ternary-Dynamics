using Sift.Presentation;

namespace Sift.UnitTests;

public sealed class ReasonMessagesTests
{
    [Fact]
    public void Format_returns_stable_english_for_guard_and_script_codes()
    {
        Assert.Contains("protected", ReasonMessages.Format(SiftReasonCode.ProcessProtected),
            StringComparison.OrdinalIgnoreCase);
        Assert.Contains("interactive session", ReasonMessages.Format(SiftReasonCode.ProcessSessionMismatch),
            StringComparison.OrdinalIgnoreCase);
        Assert.Contains("catalog", ReasonMessages.Format(SiftReasonCode.ScriptCatalogMismatch),
            StringComparison.OrdinalIgnoreCase);
        Assert.Contains("identity", ReasonMessages.Format(SiftReasonCode.ElevationRecipeHashMismatch),
            StringComparison.OrdinalIgnoreCase);
    }
}
