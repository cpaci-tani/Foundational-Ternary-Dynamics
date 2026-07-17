using System.IO;
using System.Security.Principal;
using Microsoft.Win32;

namespace Sift.Services;

public static class ElevationHelper
{
    public static bool IsElevated()
    {
        using var identity = WindowsIdentity.GetCurrent();
        return new WindowsPrincipal(identity).IsInRole(WindowsBuiltInRole.Administrator);
    }

    public static bool RequiresElevation(IEnumerable<Models.Tweak> selection) =>
        selection.Any(t => t.Kind == Models.TweakKind.Registry && t.Target.StartsWith("HKLM\\", StringComparison.OrdinalIgnoreCase));
}

public static class StartupEnumerator
{
    public sealed record StartupEntry(string Name, string Command, string Source, string Status, byte[]? IconPng = null);

    public static IReadOnlyList<StartupEntry> Enumerate()
    {
        var approved = LoadStartupApproved();
        var results = new List<StartupEntry>();

        ReadRunKey(Registry.CurrentUser, @"Software\Microsoft\Windows\CurrentVersion\Run", "HKCU Run", approved, results);
        ReadRunKey(Registry.CurrentUser, @"Software\Microsoft\Windows\CurrentVersion\RunOnce", "HKCU RunOnce", approved, results);
        try
        {
            ReadRunKey(Registry.LocalMachine, @"Software\Microsoft\Windows\CurrentVersion\Run", "HKLM Run", approved, results);
            ReadRunKey(Registry.LocalMachine, @"Software\Microsoft\Windows\CurrentVersion\RunOnce", "HKLM RunOnce", approved, results);
            ReadRunKey(Registry.LocalMachine, @"Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Run", "HKLM WOW Run", approved, results);
            ReadRunKey(Registry.LocalMachine, @"Software\WOW6432Node\Microsoft\Windows\CurrentVersion\RunOnce", "HKLM WOW RunOnce", approved, results);
        }
        catch { /* access denied when not elevated */ }

        AddStartupFolder(Environment.GetFolderPath(Environment.SpecialFolder.Startup), "Startup folder", results);
        AddStartupFolder(Environment.GetFolderPath(Environment.SpecialFolder.CommonStartup), "Common Startup", results);

        return results.OrderBy(x => x.Name, StringComparer.OrdinalIgnoreCase).ToList();
    }

    private static void AddStartupFolder(string folder, string source, List<StartupEntry> results)
    {
        if (!Directory.Exists(folder)) return;
        foreach (var file in Directory.EnumerateFiles(folder))
            results.Add(new(Path.GetFileNameWithoutExtension(file), file, source, "Enabled",
                AppIconExtractor.TryExtractPngFromCommandLine(file)));
    }

    private static void ReadRunKey(RegistryKey hive, string path, string source, Dictionary<string, bool> approved, List<StartupEntry> results)
    {
        using var key = hive.OpenSubKey(path);
        if (key is null) return;
        foreach (var name in key.GetValueNames())
        {
            var value = key.GetValue(name)?.ToString() ?? "";
            var entryName = string.IsNullOrWhiteSpace(name) ? "(default)" : name;
            var status = approved.TryGetValue(entryName, out var enabled)
                ? (enabled ? "Enabled" : "Disabled")
                : "Enabled";
            results.Add(new(entryName, value, source, status, AppIconExtractor.TryExtractPngFromCommandLine(value)));
        }
    }

    private static Dictionary<string, bool> LoadStartupApproved()
    {
        var map = new Dictionary<string, bool>(StringComparer.OrdinalIgnoreCase);
        void Read(RegistryKey root, string path)
        {
            try
            {
                using var key = root.OpenSubKey(path);
                if (key is null) return;
                foreach (var name in key.GetValueNames())
                {
                    if (key.GetValue(name) is not byte[] bytes || bytes.Length == 0) continue;
                    // StartupApproved: first byte 0x02/0x03 typically enabled; 0x01 disabled variants — treat non-zero enable bit conservatively
                    var enabled = bytes[0] is 0x02 or 0x03 or 0x06;
                    map[name] = enabled;
                }
            }
            catch { /* ignore */ }
        }

        Read(Registry.CurrentUser, @"Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run");
        Read(Registry.CurrentUser, @"Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run32");
        Read(Registry.CurrentUser, @"Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\StartupFolder");
        try
        {
            Read(Registry.LocalMachine, @"Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run");
            Read(Registry.LocalMachine, @"Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run32");
            Read(Registry.LocalMachine, @"Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\StartupFolder");
        }
        catch { /* ignore */ }
        return map;
    }
}
