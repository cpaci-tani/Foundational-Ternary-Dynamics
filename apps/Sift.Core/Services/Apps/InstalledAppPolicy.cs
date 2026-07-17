using System.Text.RegularExpressions;
using Sift.Models;

namespace Sift.Services;

public sealed record InstalledAppRegistryValues(
    string DisplayName,
    string Publisher,
    string DisplayVersion,
    string InstallLocation,
    string InstallDate,
    long EstimatedSizeBytes,
    string UninstallString,
    bool WindowsInstaller,
    bool SystemComponent,
    string ReleaseType,
    string ParentKeyName);

public sealed record InstalledAppLaunchPlan(string FileName, string Arguments)
{
    public string DisplayCommand => $"{FileName} {Arguments}".Trim();
}

public static partial class InstalledAppPolicy
{
    private static readonly string[] ProtectedNameTokens =
    [
        "sift", "security update", "hotfix", "firmware", "chipset", "driver",
        "microsoft visual c++", ".net runtime", ".net host", "windows software development kit",
        "windows sdk", "webview2 runtime", "defender", "antivirus", "endpoint", "firewall"
    ];

    private static readonly HashSet<string> RejectedLaunchers = new(StringComparer.OrdinalIgnoreCase)
    {
        "cmd.exe", "powershell.exe", "pwsh.exe", "wscript.exe", "cscript.exe",
        "rundll32.exe", "regsvr32.exe", "mshta.exe", "reg.exe"
    };

    public static (bool Allowed, string Reason) Evaluate(InstalledAppRegistryValues values)
    {
        if (string.IsNullOrWhiteSpace(values.DisplayName)) return (false, "The registry entry has no display name.");
        if (values.SystemComponent) return (false, "Windows marks this entry as a system component.");
        if (!string.IsNullOrWhiteSpace(values.ReleaseType) || !string.IsNullOrWhiteSpace(values.ParentKeyName))
            return (false, "Windows identifies this entry as an update or child component.");
        if (ProtectedNameTokens.Any(token => values.DisplayName.Contains(token, StringComparison.OrdinalIgnoreCase)))
            return (false, "Sift does not remove runtimes, drivers, security tools, or Windows system components.");
        if (string.IsNullOrWhiteSpace(values.UninstallString)) return (false, "No registered uninstall command is available.");
        return TryParseUninstallCommand(values.UninstallString, out _, out var reason)
            ? (true, "The registered interactive uninstaller can be opened after confirmation.")
            : (false, reason);
    }

    public static bool IsConservativeOrphan(InstalledAppRegistryValues values, out string evidence)
    {
        evidence = string.Empty;
        if (string.IsNullOrWhiteSpace(values.DisplayName) || values.SystemComponent || values.WindowsInstaller ||
            !string.IsNullOrWhiteSpace(values.ReleaseType) || !string.IsNullOrWhiteSpace(values.ParentKeyName)) return false;
        if (ProtectedNameTokens.Any(token => values.DisplayName.Contains(token, StringComparison.OrdinalIgnoreCase))) return false;
        if (values.Publisher.Contains("Microsoft", StringComparison.OrdinalIgnoreCase) ||
            values.Publisher.Contains("Windows", StringComparison.OrdinalIgnoreCase)) return false;

        if (!TryNormalizeExplicitDirectory(values.InstallLocation, out var installLocation) || Directory.Exists(installLocation)) return false;
        if (!TryExtractRegisteredExecutable(values.UninstallString, out var uninstaller, out _)) return false;
        if (File.Exists(uninstaller)) return false;

        evidence = $"Two registered targets are missing: install folder {installLocation}; uninstaller {uninstaller}. No files will be deleted.";
        return true;
    }

    public static bool TryExtractRegisteredExecutable(string raw, out string executable, out string reason)
    {
        executable = string.Empty;
        reason = "The registered uninstall executable is unsupported.";
        if (string.IsNullOrWhiteSpace(raw) || raw.IndexOfAny(['\r', '\n']) >= 0) return false;
        var command = Environment.ExpandEnvironmentVariables(raw.Trim());
        if (Uri.TryCreate(command, UriKind.Absolute, out var uri) && !uri.IsFile) return false;
        if (!TrySplitExecutable(command, out executable, out _)) return false;
        executable = executable.Trim().Trim('"');
        var fileName = Path.GetFileName(executable);
        if (fileName.Equals("msiexec", StringComparison.OrdinalIgnoreCase) ||
            fileName.Equals("msiexec.exe", StringComparison.OrdinalIgnoreCase))
        {
            reason = "MSI registrations are left to Windows Installer.";
            return false;
        }
        if (RejectedLaunchers.Contains(fileName))
        {
            reason = $"Commands routed through {fileName} are protected.";
            return false;
        }
        if (!Path.IsPathRooted(executable) || !executable.EndsWith(".exe", StringComparison.OrdinalIgnoreCase)) return false;
        reason = string.Empty;
        return true;
    }

    public static bool TryParseUninstallCommand(string raw, out InstalledAppLaunchPlan? plan, out string reason)
    {
        plan = null;
        reason = "The registered uninstall command is unsupported.";
        if (string.IsNullOrWhiteSpace(raw) || raw.IndexOfAny(['\r', '\n']) >= 0) return false;

        var command = Environment.ExpandEnvironmentVariables(raw.Trim());
        if (Uri.TryCreate(command, UriKind.Absolute, out var uri) && !uri.IsFile)
        {
            reason = "URI-based uninstall commands stay in Windows Installed Apps.";
            return false;
        }

        if (!TrySplitExecutable(command, out var executable, out var arguments))
        {
            reason = "Sift could not isolate an executable from the registered command.";
            return false;
        }

        var fileName = Path.GetFileName(executable);
        if (RejectedLaunchers.Contains(fileName))
        {
            reason = $"Commands routed through {fileName} are intentionally blocked.";
            return false;
        }

        if (fileName.Equals("msiexec", StringComparison.OrdinalIgnoreCase) ||
            fileName.Equals("msiexec.exe", StringComparison.OrdinalIgnoreCase))
        {
            if (!MsiArgumentsRegex().IsMatch(arguments))
            {
                reason = "Only a plain interactive MSI product command is supported.";
                return false;
            }

            plan = new InstalledAppLaunchPlan(Path.Combine(Environment.SystemDirectory, "msiexec.exe"), arguments.Trim());
            reason = string.Empty;
            return true;
        }

        if (!Path.IsPathRooted(executable) || !executable.EndsWith(".exe", StringComparison.OrdinalIgnoreCase))
        {
            reason = "The registered uninstaller is not an absolute executable path.";
            return false;
        }

        if (!File.Exists(executable))
        {
            reason = "The registered uninstaller executable no longer exists.";
            return false;
        }

        if (SilentArgumentRegex().IsMatch(arguments))
        {
            reason = "Silent uninstall commands are not launched by Sift.";
            return false;
        }

        plan = new InstalledAppLaunchPlan(executable, arguments.Trim());
        reason = string.Empty;
        return true;
    }

    private static bool TrySplitExecutable(string command, out string executable, out string arguments)
    {
        executable = string.Empty;
        arguments = string.Empty;
        if (command.StartsWith("msiexec ", StringComparison.OrdinalIgnoreCase))
        {
            executable = "msiexec";
            arguments = command["msiexec".Length..].Trim();
            return true;
        }
        if (command.StartsWith('"'))
        {
            var closingQuote = command.IndexOf('"', 1);
            if (closingQuote <= 1) return false;
            executable = command[1..closingQuote].Trim();
            arguments = command[(closingQuote + 1)..].Trim();
            return true;
        }

        var exeEnd = command.IndexOf(".exe", StringComparison.OrdinalIgnoreCase);
        if (exeEnd < 0) return false;
        exeEnd += 4;
        executable = command[..exeEnd].Trim();
        arguments = command[exeEnd..].Trim();
        return true;
    }

    private static bool TryNormalizeExplicitDirectory(string raw, out string path)
    {
        path = string.Empty;
        if (string.IsNullOrWhiteSpace(raw) || raw.IndexOfAny(['\r', '\n']) >= 0) return false;
        try
        {
            var expanded = Environment.ExpandEnvironmentVariables(raw.Trim().Trim('"'));
            if (!Path.IsPathRooted(expanded)) return false;
            path = Path.GetFullPath(expanded).TrimEnd('\\', '/');
            var root = Path.GetPathRoot(path)?.TrimEnd('\\', '/');
            return !string.IsNullOrWhiteSpace(root) && !path.Equals(root, StringComparison.OrdinalIgnoreCase);
        }
        catch
        {
            path = string.Empty;
            return false;
        }
    }

    [GeneratedRegex(@"^\s*/[ix]\s*\{[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}\}\s*$", RegexOptions.IgnoreCase)]
    private static partial Regex MsiArgumentsRegex();

    [GeneratedRegex(@"(^|\s)(/q(n|uiet)?|/s(ilent)?|--silent|-silent)(\s|$)", RegexOptions.IgnoreCase)]
    private static partial Regex SilentArgumentRegex();
}
