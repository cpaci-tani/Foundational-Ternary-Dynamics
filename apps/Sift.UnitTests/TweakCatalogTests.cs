using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class TweakCatalogTests
{
    [Fact]
    public void Catalog_ids_are_unique_and_presets_exclude_advanced()
    {
        var catalog = TweakCatalog.Create();
        Assert.Equal(catalog.Count, catalog.Select(tweak => tweak.Id).Distinct(StringComparer.OrdinalIgnoreCase).Count());
        Assert.DoesNotContain(catalog.Where(tweak => tweak.Minimal), tweak => tweak.Risk == TweakRisk.Advanced);
        Assert.DoesNotContain(catalog.Where(tweak => tweak.Recommended), tweak => tweak.Risk == TweakRisk.Advanced);
        Assert.Contains(catalog, tweak => tweak.Id == "repair.sfc-scan" && tweak.RequiresElevation);
        Assert.Contains(catalog, tweak => tweak.Id == "apps.weather" && tweak.Kind == TweakKind.AppPackage);
        Assert.True(catalog.Count >= 50);
    }

    [Fact]
    public void App_packages_are_advanced_and_irreversible()
    {
        var packages = TweakCatalog.Create().Where(tweak => tweak.Kind == TweakKind.AppPackage).ToList();
        Assert.NotEmpty(packages);
        Assert.All(packages, tweak =>
        {
            Assert.Equal(TweakRisk.Advanced, tweak.Risk);
            Assert.False(tweak.Reversible);
            Assert.False(tweak.Minimal);
            Assert.False(tweak.Recommended);
        });
    }
}
