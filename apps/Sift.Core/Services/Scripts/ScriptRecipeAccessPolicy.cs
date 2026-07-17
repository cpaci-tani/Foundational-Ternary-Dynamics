using Sift.Models;

namespace Sift.Services;

public static class ScriptRecipeAccessPolicy
{
    /// <summary>
    /// Administrator catalog recipes are visible to standard users; they run through typed
    /// <c>RunCatalogRecipe</c> elevation. Authored Studio analysis remains blocked when elevated.
    /// </summary>
    public static IReadOnlyList<ScriptRecipe> VisibleForSession(
        IEnumerable<ScriptRecipe> catalog, bool isElevated) =>
        catalog.ToList();

    public static bool RequiresConfirmation(ScriptRecipe recipe) =>
        recipe.Risk != ScriptRisk.ReadOnly;
}
