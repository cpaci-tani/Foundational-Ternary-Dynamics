using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class ScriptStudioServiceTests
{
    [Fact]
    public async Task Runtime_discovery_is_finite_and_includes_pinned_windows_hosts()
    {
        var service = new ScriptStudioService();
        var runtimes = await service.DiscoverRuntimesAsync(TestContext.Current.CancellationToken);

        Assert.Contains(runtimes, runtime => runtime.Id == "cmd-system" && runtime.Available);
        Assert.Contains(runtimes, runtime => runtime.Id == "powershell-windows" && runtime.Available);
        Assert.Contains(runtimes, runtime => runtime.Id == "cmd-system" && runtime.TrustedForAnalysis);
        Assert.Contains(runtimes, runtime => runtime.Id == "powershell-windows" && runtime.TrustedForAnalysis);
        Assert.All(runtimes.Where(runtime => !string.IsNullOrWhiteSpace(runtime.ExecutablePath)),
            runtime => Assert.True(Path.IsPathFullyQualified(runtime.ExecutablePath), runtime.ExecutablePath));
        Assert.True(runtimes.Count < 30);
    }

    [Fact]
    public void Python_policy_blocks_dynamic_remote_and_process_execution()
    {
        var document = new ScriptDocument(ScriptLanguage.Python, "python", "scratch.py",
            "import subprocess\nimport requests\nexec('print(1)')\n");

        var diagnostics = ScriptStudioService.AnalyzePolicy(document);

        Assert.Contains(diagnostics, item => item.Code == "CWPY001" && item.Severity == ScriptDiagnosticSeverity.Blocked);
        Assert.Contains(diagnostics, item => item.Code == "CWPY002" && item.Severity == ScriptDiagnosticSeverity.Blocked);
        Assert.Contains(diagnostics, item => item.Code == "CWPY003" && item.Severity == ScriptDiagnosticSeverity.Blocked);
    }

    [Fact]
    public void Policy_reports_source_positions()
    {
        var document = new ScriptDocument(ScriptLanguage.PowerShell, "powershell", "scratch.ps1",
            "Get-Process\nInvoke-WebRequest https://example.invalid\n");

        var diagnostic = Assert.Single(ScriptStudioService.AnalyzePolicy(document));

        Assert.Equal(2, diagnostic.Line);
        Assert.Equal("CWPS001", diagnostic.Code);
    }

    [Fact]
    public async Task PowerShell_analysis_parses_without_executing_document()
    {
        var service = new ScriptStudioService();
        var runtimes = await service.DiscoverRuntimesAsync(TestContext.Current.CancellationToken);
        var runtime = runtimes.Single(item => item.Id == "powershell-windows");
        var marker = Path.Combine(Path.GetTempPath(), $"sift-studio-{Guid.NewGuid():N}.txt");
        var document = new ScriptDocument(ScriptLanguage.PowerShell, runtime.Id, "scratch.ps1",
            $"Set-Content -LiteralPath '{marker.Replace("'", "''")}' -Value unsafe");

        var analysis = await service.AnalyzeAsync(document, TestContext.Current.CancellationToken);

        Assert.True(analysis.SyntaxChecked);
        Assert.False(analysis.CanExecute);
        Assert.False(File.Exists(marker));
        Assert.Contains(analysis.Diagnostics, item => item.Code == "CWPS003");
    }

    [Fact]
    public async Task Python_analysis_reports_invalid_syntax_when_runtime_is_available()
    {
        var service = new ScriptStudioService();
        var runtimes = await service.DiscoverRuntimesAsync(TestContext.Current.CancellationToken);
        var runtime = runtimes.FirstOrDefault(item =>
            item.Language == ScriptLanguage.Python && item.Available && item.TrustedForAnalysis);
        if (runtime is null) return;

        var document = new ScriptDocument(ScriptLanguage.Python, runtime.Id, "scratch.py", "if True print('broken')");
        var analysis = await service.AnalyzeAsync(document, TestContext.Current.CancellationToken);

        Assert.True(analysis.SyntaxChecked);
        Assert.False(analysis.CanExecute);
        Assert.Contains(analysis.Diagnostics, item => item.Source == "Python" && item.Severity == ScriptDiagnosticSeverity.Error);
    }

    [Fact]
    public async Task Missing_runtime_is_visible_and_never_enables_execution()
    {
        var service = new ScriptStudioService();
        var document = new ScriptDocument(ScriptLanguage.Python, "missing", "scratch.py", "print('hello')");

        var analysis = await service.AnalyzeAsync(document, TestContext.Current.CancellationToken);

        Assert.False(analysis.SyntaxChecked);
        Assert.False(analysis.CanExecute);
        Assert.Contains(analysis.Diagnostics, item => item.Code == "CW1001");
    }

    [Fact]
    public async Task Runtime_id_cannot_be_reinterpreted_as_another_language()
    {
        var service = new ScriptStudioService();
        var document = new ScriptDocument(
            ScriptLanguage.Python,
            "powershell-windows",
            "scratch.py",
            "print('must not reach PowerShell')");

        var analysis = await service.AnalyzeAsync(document, TestContext.Current.CancellationToken);

        Assert.False(analysis.SyntaxChecked);
        Assert.False(analysis.CanExecute);
        Assert.Contains(analysis.Diagnostics, item => item.Code == "CW1005");
    }

    [Fact]
    public async Task Pre_cancelled_analysis_never_discovers_or_starts_a_runtime()
    {
        var service = new ScriptStudioService();
        using var cancellation = new CancellationTokenSource();
        cancellation.Cancel();
        var document = new ScriptDocument(ScriptLanguage.PowerShell, "powershell-windows", "scratch.ps1", "Get-Process");

        await Assert.ThrowsAsync<OperationCanceledException>(() => service.AnalyzeAsync(document, cancellation.Token));
    }

    [Fact]
    public async Task TypeScript_without_core_diagnostic_bridge_is_not_reported_as_syntax_clean()
    {
        var service = new ScriptStudioService();
        var document = new ScriptDocument(
            ScriptLanguage.TypeScript,
            "typescript-language-service",
            "scratch.ts",
            "const broken: = 1;");

        var analysis = await service.AnalyzeAsync(document, TestContext.Current.CancellationToken);

        Assert.False(analysis.SyntaxChecked);
        Assert.False(analysis.CanExecute);
        Assert.Contains("did not perform a syntax check", analysis.Summary, StringComparison.OrdinalIgnoreCase);
    }
}
