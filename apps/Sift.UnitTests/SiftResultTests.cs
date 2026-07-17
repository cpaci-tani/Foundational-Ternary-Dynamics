using Sift.Models;
using Sift.Presentation;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class SiftResultTests
{
    [Fact]
    public void Fail_formats_reason_message_and_preserves_code()
    {
        var fail = SiftResult.Fail(SiftReasonCode.ProcessProtected);
        Assert.True(fail.IsFailure);
        Assert.Equal(SiftReasonCode.ProcessProtected, fail.ReasonCode);
        Assert.Contains("protected", fail.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void Ok_value_result_carries_payload()
    {
        var ok = SiftResult<int>.Ok(42);
        Assert.True(ok.IsSuccess);
        Assert.Equal(42, ok.Value);
        Assert.Equal(SiftReasonCode.Unspecified, ok.ReasonCode);
    }

    [Fact]
    public void ResolveCatalogRecipe_returns_typed_failure_for_hash_mismatch()
    {
        var recipe = new ScriptCommandService().Catalog.First(x => x.RequiresAdministrator);
        var result = ElevatedOperationPolicy.ResolveCatalogRecipe(recipe.Id, new string('A', 64));
        Assert.True(result.IsFailure);
        Assert.Equal(SiftReasonCode.ElevationRecipeHashMismatch, result.ReasonCode);
        Assert.Contains("identity", result.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void ResolveCatalogRecipe_returns_ok_for_admin_recipe()
    {
        var recipe = new ScriptCommandService().Catalog.First(x => x.RequiresAdministrator);
        var hash = ScriptRecipeIdentity.ComputeHash(recipe);
        var result = ElevatedOperationPolicy.ResolveCatalogRecipe(recipe.Id, hash);
        Assert.True(result.IsSuccess);
        Assert.Equal(recipe.Id, result.Value!.Id);
    }
}
