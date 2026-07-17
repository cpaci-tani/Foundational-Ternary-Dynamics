using System.Security.Cryptography;
using System.Text;
using Sift.Models;

namespace Sift.Services;

public sealed class ScheduledTaskController : IScheduledTaskController
{
    private const int MaximumDefinitionBytes = 256 * 1024;

    public ScheduledTaskIdentity? Inspect(ScheduledTaskId id)
    {
        var definition = ScheduledTaskIdentityCatalog.Resolve(id);
        if (!TryQueryDefinition(definition, out var xml, out var state)) return null;
        var enabled = IsEnabledState(state);
        return new ScheduledTaskIdentity(id, definition.DisplayName, enabled, state, HashDefinition(xml));
    }

    public ScheduledTaskActionResult SetEnabled(
        ScheduledTaskId id,
        ScheduledTaskChange change,
        bool expectedEnabled,
        string expectedDefinitionHash)
    {
        var definition = ScheduledTaskIdentityCatalog.Resolve(id);
        var current = Inspect(id);
        if (current is null)
            return Failed($"The scheduled task is no longer present or readable: {definition.DisplayName}.");
        if (current.Enabled != expectedEnabled ||
            !string.Equals(current.DefinitionHash, expectedDefinitionHash, StringComparison.Ordinal))
            return Failed("The scheduled task changed after confirmation; nothing was changed.");

        var desiredEnabled = change == ScheduledTaskChange.Enable;
        if (current.Enabled == desiredEnabled)
            return Failed($"The scheduled task is already {(desiredEnabled ? "enabled" : "disabled")}.");

        try
        {
            var full = definition.TaskPath.TrimEnd('\\') + "\\" + definition.TaskName;
            var arguments = desiredEnabled
                ? new[] { "/Change", "/TN", full, "/ENABLE" }
                : new[] { "/Change", "/TN", full, "/DISABLE" };
            _ = TweakExecutor.RunProcessAsync(
                    TweakExecutor.CreateTrustedProcessStartInfo("schtasks.exe", arguments),
                    TimeSpan.FromSeconds(10),
                    CancellationToken.None)
                .GetAwaiter().GetResult();

            var summary = desiredEnabled
                ? $"Enabled {definition.DisplayName}."
                : $"Disabled {definition.DisplayName}.";
            return new ScheduledTaskActionResult(true, false, summary, [summary]);
        }
        catch (Exception exception)
        {
            return Failed($"Failed to change {definition.DisplayName}: {exception.Message}");
        }
    }

    private static bool TryQueryDefinition(ScheduledTaskDefinition definition, out string xml, out string state)
    {
        xml = string.Empty;
        state = "Unknown";
        try
        {
            var full = definition.TaskPath.TrimEnd('\\') + "\\" + definition.TaskName;
            var startInfo = TweakExecutor.CreateTrustedProcessStartInfo(
                "schtasks.exe", ["/Query", "/TN", full, "/XML"]);
            startInfo.StandardOutputEncoding = Encoding.UTF8;
            var result = TweakExecutor.RunProcessAsync(startInfo, TimeSpan.FromSeconds(10), CancellationToken.None)
                .GetAwaiter().GetResult();
            xml = result.StandardOutput;
            if (string.IsNullOrWhiteSpace(xml)) return false;
            if (Encoding.UTF8.GetByteCount(xml) > MaximumDefinitionBytes) return false;
            state = ReadState(xml);
            return true;
        }
        catch
        {
            return false;
        }
    }

    private static string ReadState(string xml)
    {
        var match = System.Text.RegularExpressions.Regex.Match(xml, "<Enabled>(true|false)</Enabled>",
            System.Text.RegularExpressions.RegexOptions.IgnoreCase);
        // A disabled task always emits <Enabled>false</Enabled>; the element is omitted for an enabled
        // task because the Task Scheduler schema defaults <Settings><Enabled> to true. Report the
        // omitted case as the enabled "Ready" state rather than an ambiguous "Unknown" so the state
        // label is honest. A genuinely unreadable definition fails earlier in TryQueryDefinition, which
        // makes Inspect return null and blocks the action; and every mutation is still gated by a full
        // definition-hash match, so a mislabel cannot authorize an unintended change.
        if (!match.Success) return "Ready";
        return match.Groups[1].Value.Equals("true", StringComparison.OrdinalIgnoreCase) ? "Ready" : "Disabled";
    }

    private static bool IsEnabledState(string state) =>
        !state.Equals("Disabled", StringComparison.OrdinalIgnoreCase);

    private static string HashDefinition(string xml)
    {
        var bytes = Encoding.UTF8.GetBytes(xml);
        return Convert.ToHexString(SHA256.HashData(bytes));
    }

    private static ScheduledTaskActionResult Failed(string summary) =>
        new(false, false, summary, [summary]);
}
