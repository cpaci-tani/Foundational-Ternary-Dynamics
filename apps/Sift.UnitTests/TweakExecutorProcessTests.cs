using System.Diagnostics;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class TweakExecutorProcessTests
{
    [Theory]
    [InlineData("powercfg.exe /hibernate off", "powercfg.exe", "/hibernate", "off")]
    [InlineData("powercfg.exe /hibernate on", "powercfg.exe", "/hibernate", "on")]
    [InlineData("DISM.exe /Online /Cleanup-Image /StartComponentCleanup", "dism.exe", "/Online", "/Cleanup-Image", "/StartComponentCleanup")]
    [InlineData("sfc.exe /scannow", "sfc.exe", "/scannow")]
    public void Catalog_commands_pin_absolute_system_executables_and_argument_tokens(
        string command,
        string executable,
        params string[] arguments)
    {
        var start = TweakExecutor.CreateCatalogCommandStartInfo(command);

        Assert.Equal(Path.Combine(Environment.SystemDirectory, executable), start.FileName,
            ignoreCase: true);
        Assert.Equal(Environment.SystemDirectory, start.WorkingDirectory, ignoreCase: true);
        Assert.False(start.UseShellExecute);
        Assert.True(start.CreateNoWindow);
        Assert.True(start.RedirectStandardOutput);
        Assert.True(start.RedirectStandardError);
        Assert.Equal(arguments, start.ArgumentList.ToArray());
        AssertSanitizedEnvironment(start);
    }

    [Fact]
    public void PowerShell_is_pinned_and_keeps_the_script_in_one_argument()
    {
        const string command = "Get-AppxPackage -Name 'Example.Package' | Remove-AppxPackage -ErrorAction Stop";

        var start = TweakExecutor.CreatePowerShellStartInfo(command);

        Assert.Equal(Path.Combine(Environment.SystemDirectory, "WindowsPowerShell", "v1.0", "powershell.exe"),
            start.FileName, ignoreCase: true);
        Assert.Equal(command, start.ArgumentList.Last());
        Assert.Equal(1, start.ArgumentList.Count(argument => argument == command));
        AssertSanitizedEnvironment(start);
    }

    [Fact]
    public void Arbitrary_catalog_command_is_rejected_before_process_creation()
    {
        Assert.Throws<InvalidOperationException>(() =>
            TweakExecutor.CreateCatalogCommandStartInfo("cmd.exe /c whoami"));
    }

    [Fact]
    public void Scheduled_task_tool_is_pinned_and_uses_argument_tokens()
    {
        var start = TweakExecutor.CreateTrustedProcessStartInfo(
            "schtasks.exe", ["/Query", "/TN", @"\Microsoft\Windows\Defrag\ScheduledDefrag", "/XML"]);

        Assert.Equal(Path.Combine(Environment.SystemDirectory, "schtasks.exe"), start.FileName,
            ignoreCase: true);
        Assert.Equal(new[] { "/Query", "/TN", @"\Microsoft\Windows\Defrag\ScheduledDefrag", "/XML" },
            start.ArgumentList.ToArray());
        AssertSanitizedEnvironment(start);
    }

    private static void AssertSanitizedEnvironment(ProcessStartInfo start)
    {
        Assert.Equal(Environment.SystemDirectory, start.Environment["PATH"]!.Split(Path.PathSeparator)[0],
            ignoreCase: true);
        Assert.Equal(Environment.GetFolderPath(Environment.SpecialFolder.Windows), start.Environment["SystemRoot"],
            ignoreCase: true);
        Assert.Equal(Path.Combine(Environment.SystemDirectory, "cmd.exe"), start.Environment["ComSpec"],
            ignoreCase: true);
        Assert.DoesNotContain(".", start.Environment["PATH"]!.Split(Path.PathSeparator));
        Assert.DoesNotContain("PROMPT", start.Environment.Keys, StringComparer.OrdinalIgnoreCase);
    }
}
