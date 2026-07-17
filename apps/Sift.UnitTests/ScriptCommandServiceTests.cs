using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class ScriptCommandServiceTests
{
    [Fact]
    public void Catalog_contains_at_least_one_hundred_unique_reviewed_recipes()
    {
        var service = new ScriptCommandService();
        Assert.True(service.Catalog.Count >= 100);
        Assert.Equal(service.Catalog.Count, service.Catalog.Select(x => x.Id).Distinct(StringComparer.OrdinalIgnoreCase).Count());
    }

    [Fact]
    public void Catalog_does_not_contain_remote_or_destructive_tokens()
    {
        var service = new ScriptCommandService();
        var blocked = new[] { "invoke-webrequest", "downloadstring", "curl ", "wget ", "remove-item", "reg delete", "format ", "diskpart" };
        foreach (var recipe in service.Catalog)
            Assert.DoesNotContain(blocked, token => recipe.Command.Contains(token, StringComparison.OrdinalIgnoreCase));
    }

    [Fact]
    public void Preflight_rejects_recipe_not_in_exact_catalog()
    {
        var service = new ScriptCommandService();
        var result = service.Preflight(new ScriptRecipe("foreign", "Foreign", "Test", "Test", ScriptShell.Cmd, "whoami"));
        Assert.False(result.Allowed);
    }

    [Fact]
    public void Preflight_rejects_forged_security_metadata()
    {
        var service = new ScriptCommandService();
        var canonical = service.Catalog.First(x => x.RequiresAdministrator);
        var forged = canonical with { RequiresAdministrator = false, Risk = ScriptRisk.ReadOnly };
        var result = service.Preflight(forged);
        Assert.False(result.Allowed);
        Assert.Contains("security metadata", result.BlockReason, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void Shells_and_child_path_are_pinned_to_trusted_windows_locations()
    {
        var service = new ScriptCommandService();
        var recipe = service.Catalog.First(x => x.Shell == ScriptShell.Cmd && !x.RequiresAdministrator);
        var executable = ScriptCommandService.TrustedShellPath(recipe.Shell);
        var startInfo = ScriptCommandService.CreateStartInfo(recipe, executable);
        Assert.Equal(Path.Combine(Environment.SystemDirectory, "cmd.exe"), startInfo.FileName, ignoreCase: true);
        Assert.Equal(Environment.SystemDirectory, startInfo.WorkingDirectory, ignoreCase: true);
        Assert.DoesNotContain(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile),
            startInfo.Environment["PATH"] ?? "", StringComparison.OrdinalIgnoreCase);
        Assert.StartsWith(Environment.SystemDirectory, startInfo.Environment["PATH"] ?? "", StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public async Task Read_only_standard_recipe_executes_with_streamed_output()
    {
        var service = new ScriptCommandService();
        var recipe = service.Catalog.Single(x => x.Id == "sys-ver");
        var preflight = service.Preflight(recipe);
        Assert.True(preflight.Allowed, preflight.BlockReason);
        var output = new List<string>();
        var result = await service.RunAsync(recipe, preflight, (line, _) => output.Add(line),
            TestContext.Current.CancellationToken);
        Assert.False(result.Cancelled);
        Assert.Equal(0, result.ExitCode);
        Assert.NotEmpty(output);
    }

    [Fact]
    public async Task Pre_cancelled_recipe_never_starts()
    {
        var service = new ScriptCommandService();
        var recipe = service.Catalog.Single(x => x.Id == "sys-ver");
        var preflight = service.Preflight(recipe);
        using var cancellation = new CancellationTokenSource();
        cancellation.Cancel();
        await Assert.ThrowsAsync<OperationCanceledException>(() =>
            service.RunAsync(recipe, preflight, (_, _) => { }, cancellation.Token));
    }

    [Fact]
    public void Catalog_uses_current_windows_commands_and_correct_admin_metadata()
    {
        var service = new ScriptCommandService();
        Assert.DoesNotContain(service.Catalog, x => x.Command.Contains("wmic", StringComparison.OrdinalIgnoreCase));
        Assert.Contains("/limitaccess", service.Catalog.Single(x => x.Id == "health-dism-restore").Command,
            StringComparison.OrdinalIgnoreCase);
        Assert.True(service.Catalog.Single(x => x.Id == "disk-bitlocker").RequiresAdministrator);
        Assert.True(service.Catalog.Single(x => x.Id == "sec-audit").RequiresAdministrator);
        Assert.Contains("S-1-5-32-544", service.Catalog.Single(x => x.Id == "sec-admins").Command);
    }

    [Fact]
    public void Network_and_sensitive_evidence_is_recipe_specific()
    {
        var service = new ScriptCommandService();
        var network = service.Preflight(service.Catalog.Single(x => x.Id == "net-ping-cloudflare"));
        var sensitive = service.Preflight(service.Catalog.Single(x => x.Id == "sys-env"));
        Assert.Contains(network.Evidence, line => line.Contains("may contact", StringComparison.OrdinalIgnoreCase));
        Assert.Contains(sensitive.Evidence, line => line.Contains("secret", StringComparison.OrdinalIgnoreCase));
    }

    [Fact]
    public void Administrator_recipes_are_visible_in_standard_sessions_for_typed_elevation()
    {
        var catalog = new ScriptCommandService().Catalog;
        var standard = ScriptRecipeAccessPolicy.VisibleForSession(catalog, isElevated: false);
        var elevated = ScriptRecipeAccessPolicy.VisibleForSession(catalog, isElevated: true);
        Assert.Contains(standard, recipe => recipe.RequiresAdministrator);
        Assert.Contains(elevated, recipe => recipe.RequiresAdministrator);
        Assert.Equal(catalog.Count, standard.Count);
        Assert.Equal(catalog.Count, elevated.Count);
    }

    [Fact]
    public void Administrator_recipe_preflight_marks_elevation_hop_for_standard_sessions()
    {
        if (ElevationHelper.IsElevated()) return;
        var service = new ScriptCommandService();
        var recipe = service.Catalog.First(x => x.RequiresAdministrator);
        var preflight = service.Preflight(recipe);
        Assert.True(preflight.Allowed, preflight.BlockReason);
        Assert.True(preflight.RequiresElevation);
        Assert.False(string.IsNullOrWhiteSpace(preflight.RecipeHash));
        Assert.Equal(ScriptRecipeIdentity.ComputeHash(recipe), preflight.RecipeHash);
    }

    [Fact]
    public void Only_state_changing_recipes_require_confirmation()
    {
        var catalog = new ScriptCommandService().Catalog;

        Assert.All(catalog.Where(recipe => recipe.Risk == ScriptRisk.ReadOnly),
            recipe => Assert.False(ScriptRecipeAccessPolicy.RequiresConfirmation(recipe)));
        Assert.All(catalog.Where(recipe => recipe.Risk != ScriptRisk.ReadOnly),
            recipe => Assert.True(ScriptRecipeAccessPolicy.RequiresConfirmation(recipe)));
        Assert.Contains(catalog, recipe => recipe.Risk != ScriptRisk.ReadOnly);
    }

    [Fact]
    public void Every_recipe_uses_the_complete_ordered_category_taxonomy()
    {
        var catalog = new ScriptCommandService().Catalog;
        Assert.Equal(19, ScriptRecipeTaxonomy.Categories.Count);
        Assert.All(catalog, recipe => Assert.True(ScriptRecipeTaxonomy.IsKnown(recipe.Category), recipe.Category));
        Assert.All(ScriptRecipeTaxonomy.Categories,
            category => Assert.Contains(catalog, recipe => recipe.Category == category.Name));
        var categoryOrders = catalog.Select(recipe => ScriptRecipeTaxonomy.OrderOf(recipe.Category)).ToList();
        Assert.Equal(categoryOrders.Order().ToList(), categoryOrders);
    }

    [Fact]
    public void Mutating_recipes_are_explicitly_labeled()
    {
        var service = new ScriptCommandService();
        Assert.All(service.Catalog.Where(x => x.Id.Contains("reset") || x.Id.Contains("flush") || x.Id.Contains("restore")),
            recipe => Assert.NotEqual(ScriptRisk.ReadOnly, recipe.Risk));
    }
}
