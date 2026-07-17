using System.Security.Cryptography;
using System.Text;
using Sift.Models;

namespace Sift.Services;

/// <summary>
/// Deterministic identity hash for catalog recipes crossing the elevation boundary.
/// Never hashes or transmits raw command text as a free-form elevation payload field —
/// the helper re-resolves the command from the bundled catalog after verifying this hash.
/// </summary>
public static class ScriptRecipeIdentity
{
    public static string ComputeHash(ScriptRecipe recipe)
    {
        ArgumentNullException.ThrowIfNull(recipe);
        var material = string.Join('\n',
            recipe.Id,
            recipe.Title,
            recipe.Category,
            recipe.Shell.ToString(),
            recipe.Command,
            recipe.Risk.ToString(),
            recipe.RequiresAdministrator ? "1" : "0",
            recipe.MayUseNetwork ? "1" : "0",
            recipe.SensitiveOutput ? "1" : "0");
        return Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(material)));
    }

    public static bool IsValidRecipeId(string? recipeId) =>
        !string.IsNullOrWhiteSpace(recipeId) &&
        recipeId.Length <= 64 &&
        recipeId.All(character =>
            character is >= 'a' and <= 'z' or >= '0' and <= '9' or '-' or '_' or '.');

    public static bool IsValidHash(string? hash) =>
        !string.IsNullOrWhiteSpace(hash) &&
        hash.Length == 64 &&
        hash.All(character => character is >= '0' and <= '9' or >= 'A' and <= 'F');
}
