using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class ScriptRecipeElevationTests
{
    [Fact]
    public void Recipe_identity_hash_is_stable_and_sensitive_to_command_drift()
    {
        var recipe = new ScriptCommandService().Catalog.First(x => x.RequiresAdministrator);
        var hash = ScriptRecipeIdentity.ComputeHash(recipe);
        Assert.True(ScriptRecipeIdentity.IsValidHash(hash));
        Assert.Equal(hash, ScriptRecipeIdentity.ComputeHash(recipe));
        var drifted = recipe with { Command = recipe.Command + " " };
        Assert.NotEqual(hash, ScriptRecipeIdentity.ComputeHash(drifted));
    }

    [Fact]
    public void Catalog_recipe_policy_resolves_admin_recipe_and_rejects_hash_mismatch()
    {
        var recipe = new ScriptCommandService().Catalog.First(x => x.RequiresAdministrator);
        var hash = ScriptRecipeIdentity.ComputeHash(recipe);
        Assert.True(ElevatedOperationPolicy.TryResolveCatalogRecipe(recipe.Id, hash, out var resolved, out _));
        Assert.Equal(recipe.Id, resolved.Id);

        Assert.False(ElevatedOperationPolicy.TryResolveCatalogRecipe(
            recipe.Id, new string('A', 64), out _, out var mismatch));
        Assert.Contains("identity", mismatch, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void Catalog_recipe_policy_rejects_non_admin_and_unknown_ids()
    {
        var standard = new ScriptCommandService().Catalog.First(x => !x.RequiresAdministrator);
        var hash = ScriptRecipeIdentity.ComputeHash(standard);
        Assert.False(ElevatedOperationPolicy.TryResolveCatalogRecipe(standard.Id, hash, out _, out var reason));
        Assert.Contains("administrator", reason, StringComparison.OrdinalIgnoreCase);
        Assert.False(ElevatedOperationPolicy.TryResolveCatalogRecipe(
            "not-a-real-recipe", new string('0', 64), out _, out _));
    }

    [Fact]
    public void RunCatalogRecipe_request_shape_accepts_only_recipe_identity_fields()
    {
        var recipe = new ScriptCommandService().Catalog.First(x => x.RequiresAdministrator);
        var hash = ScriptRecipeIdentity.ComputeHash(recipe);
        var id = Guid.NewGuid().ToString("N");
        var nonce = Convert.ToHexString(System.Security.Cryptography.RandomNumberGenerator.GetBytes(32));
        var valid = new ElevatedOperationRequest(id, nonce, ElevatedOperationKind.RunCatalogRecipe, [],
            RecipeId: recipe.Id, ExpectedRecipeHash: hash);
        Assert.True(ElevatedOperationPolicy.TryValidateRequestShape(valid, out _));

        var polluted = valid with { ServiceName = "Acme" };
        Assert.False(ElevatedOperationPolicy.TryValidateRequestShape(polluted, out _));
    }
}
