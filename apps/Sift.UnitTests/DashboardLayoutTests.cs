using System.Text.Json;
using Sift.Models;
using Sift.Services;

namespace Sift.UnitTests;

public sealed class DashboardLayoutTests
{
    [Fact]
    public void Place_pushes_only_colliding_widgets_and_preserves_gaps()
    {
        var source = Layout(6,
            Placement("moving", 0, 0, 2, 2),
            Placement("collision", 0, 2, 2, 2),
            Placement("gap", 5, 4, 2, 2));

        var result = DashboardPackingEngine.Place(source, "moving", 0, 2, 2, 2);

        Assert.Equal((0, 2), Position(result, "moving"));
        Assert.Equal((2, 2), Position(result, "collision"));
        Assert.Equal((5, 4), Position(result, "gap"));
        Assert.Equal((0, 0), Position(source, "moving"));
    }

    [Theory]
    [InlineData(1)]
    [InlineData(2)]
    [InlineData(3)]
    [InlineData(4)]
    [InlineData(5)]
    [InlineData(6)]
    public void Place_supports_every_wide_column_span(int span)
    {
        var result = DashboardPackingEngine.Place(
            Layout(6, Placement("widget", 0, 0, 1, 1)), "widget", 2, 6, 1, span);

        var placement = Assert.Single(result.Placements);
        Assert.Equal(span, placement.ColumnSpan);
        Assert.Equal(6 - span, placement.Column);
    }

    [Fact]
    public void Tidy_compacts_visible_widgets_but_keeps_hidden_placement()
    {
        var hidden = Placement("hidden", 20, 5, 1, 1);
        hidden.Visible = false;
        var result = DashboardPackingEngine.Tidy(Layout(6,
            Placement("first", 10, 3, 2, 1), Placement("second", 12, 1, 3, 1), hidden));

        Assert.Equal((0, 0), Position(result, "first"));
        Assert.Equal((0, 2), Position(result, "second"));
        Assert.Equal((20, 5), Position(result, "hidden"));
    }

    [Fact]
    public void Default_profiles_have_three_valid_independent_breakpoints()
    {
        var document = DashboardProfileDefaults.Create();
        var definitions = DashboardWidgetCatalog.ById();

        Assert.Equal(4, document.Profiles.Count);
        foreach (var profile in document.Profiles)
        {
            var instances = profile.Widgets.ToDictionary(widget => widget.InstanceId, StringComparer.OrdinalIgnoreCase);
            Assert.Equal(3, profile.Layouts.Count);
            Assert.All(profile.Layouts, layout => Assert.Empty(
                DashboardPackingEngine.Validate(layout, instances, definitions)));
        }

        var overview = document.Profiles.Single(profile => profile.Id == "overview");
        var wide = overview.Layouts.Single(layout => layout.Breakpoint == DashboardBreakpoint.Wide);
        var compact = overview.Layouts.Single(layout => layout.Breakpoint == DashboardBreakpoint.Compact);
        var changed = DashboardPackingEngine.Place(wide, wide.Placements[0].InstanceId, 15, 0, 1, 6);
        Assert.NotEqual(changed.Placements[0].Row, wide.Placements[0].Row);
        Assert.NotEqual(changed.Placements[0].Row, compact.Placements[0].Row);
    }

    [Fact]
    public void Store_migrates_legacy_visibility_and_round_trips_profiles()
    {
        var root = TempDirectory();
        try
        {
            var store = new DashboardProfileStore(root);
            var document = store.LoadOrCreate(new Dictionary<string, bool>(StringComparer.OrdinalIgnoreCase)
            {
                ["cpu"] = false,
                ["memory"] = true
            });

            var overview = document.Profiles.Single(profile => profile.Id == "overview");
            Assert.All(overview.Layouts, layout => Assert.False(layout.Placements.Single(placement =>
                placement.InstanceId == "overview.cpu").Visible));
            store.Save(document);
            Assert.Equal(document.Profiles.Count, store.LoadOrCreate().Profiles.Count);
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public void Store_quarantines_corrupt_profiles()
    {
        var root = TempDirectory();
        try
        {
            Directory.CreateDirectory(root);
            File.WriteAllText(Path.Combine(root, "dashboard-profiles.json"), "{not-json");
            var result = new DashboardProfileStore(root).LoadOrCreate();

            Assert.Equal(4, result.Profiles.Count);
            Assert.Single(Directory.GetFiles(root, "dashboard-profiles.corrupt-*.json"));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public void Import_rejects_unknown_widgets_and_creates_unique_copy()
    {
        var root = TempDirectory();
        try
        {
            var store = new DashboardProfileStore(root);
            var document = store.LoadOrCreate();
            var overview = document.Profiles.Single(profile => profile.Id == "overview");
            var exported = store.ExportProfile(overview);
            var imported = store.ImportProfile(document, exported);
            Assert.Equal("Overview (2)", imported.Name);
            Assert.DoesNotContain(imported.Widgets, widget => overview.Widgets.Any(source => source.InstanceId == widget.InstanceId));

            using var parsed = JsonDocument.Parse(exported);
            var tampered = exported.Replace("\"cpu\"", "\"arbitrary.command\"", StringComparison.Ordinal);
            Assert.Throws<InvalidDataException>(() => store.ImportProfile(document, tampered));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public void Import_rejects_unknown_fields_and_path_like_widget_settings()
    {
        var root = TempDirectory();
        try
        {
            var store = new DashboardProfileStore(root);
            var document = store.LoadOrCreate();
            var profile = document.Profiles[0];
            var exported = store.ExportProfile(profile);
            var unknown = exported.Replace("\"SchemaVersion\": 1", "\"SchemaVersion\": 1, \"ExecutablePath\": \"calc.exe\"", StringComparison.Ordinal);
            Assert.ThrowsAny<Exception>(() => store.ImportProfile(document, unknown));

            profile.Widgets[0].Settings["filter"] = @"C:\\Windows";
            Assert.Throws<InvalidDataException>(() => store.ExportProfile(profile));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public void Store_enforces_breakpoint_columns_and_quarantines_semantic_corruption()
    {
        var root = TempDirectory();
        try
        {
            Directory.CreateDirectory(root);
            var document = DashboardProfileDefaults.Create();
            document.Profiles[0].Layouts.Single(layout => layout.Breakpoint == DashboardBreakpoint.Medium).Columns = 5;
            File.WriteAllText(Path.Combine(root, "dashboard-profiles.json"), JsonSerializer.Serialize(document));

            var recovered = new DashboardProfileStore(root).LoadOrCreate();

            Assert.Equal(4, recovered.Profiles.Count);
            Assert.All(recovered.Profiles.SelectMany(profile => profile.Layouts), layout =>
                Assert.Equal(layout.Breakpoint switch
                {
                    DashboardBreakpoint.Wide => 6,
                    DashboardBreakpoint.Medium => 4,
                    _ => 2
                }, layout.Columns));
            Assert.Single(Directory.GetFiles(root, "dashboard-profiles.corrupt-*.json"));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public void Store_rejects_oversized_import_and_invalid_nullable_widget_fields()
    {
        var root = TempDirectory();
        try
        {
            var store = new DashboardProfileStore(root);
            var document = store.LoadOrCreate();
            Assert.Throws<InvalidDataException>(() => store.ImportProfile(document,
                "{" + new string('x', DashboardProfileStore.MaximumImportBytes)));

            var profile = document.Profiles[0];
            profile.Widgets[0].Accent = "Blue";
            Assert.Throws<InvalidDataException>(() => store.ExportProfile(profile));
            profile.Widgets[0].Accent = "Clay";
            profile.Widgets[0].TitleOverride = new string('x', 121);
            Assert.Throws<InvalidDataException>(() => store.ExportProfile(profile));
            profile.Widgets[0].TitleOverride = null;
            profile.Widgets[0].Settings = null!;
            Assert.Throws<InvalidDataException>(() => store.ExportProfile(profile));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public void Store_quarantines_oversized_profile_document_before_deserialization()
    {
        var root = TempDirectory();
        try
        {
            Directory.CreateDirectory(root);
            File.WriteAllText(Path.Combine(root, "dashboard-profiles.json"),
                new string(' ', DashboardProfileStore.MaximumDocumentBytes + 1));

            var recovered = new DashboardProfileStore(root).LoadOrCreate();

            Assert.Equal(4, recovered.Profiles.Count);
            Assert.Single(Directory.GetFiles(root, "dashboard-profiles.corrupt-*.json"));
        }
        finally { DeleteDirectory(root); }
    }

    [Fact]
    public void Edit_session_supports_bounded_undo_redo_without_mutating_source()
    {
        var source = DashboardProfileDefaults.Create().Profiles[0];
        var originalName = source.Name;
        var session = new DashboardEditSession(source);
        session.Apply(profile => profile.Name = "Changed");
        session.Apply(profile => profile.Density = DashboardDensity.Compact);

        Assert.Equal(originalName, source.Name);
        Assert.True(session.Undo());
        Assert.Equal("Changed", session.WorkingProfile.Name);
        Assert.True(session.Undo());
        Assert.Equal(originalName, session.WorkingProfile.Name);
        Assert.True(session.Redo());
        Assert.Equal("Changed", session.WorkingProfile.Name);
    }

    [Theory]
    [InlineData(0.49, 0, 0)]
    [InlineData(0.50, null, 0)]
    [InlineData(0.84, 0, 0)]
    [InlineData(0.85, 0, 1)]
    [InlineData(1.84, 1, 1)]
    [InlineData(1.85, 1, 2)]
    public void SnapIndex_applies_hysteresis_around_half_cell_boundaries(
        double fractional, int? current, int expected)
    {
        Assert.Equal(expected, DashboardGridMath.SnapIndex(fractional, current, 0.35));
    }

    [Fact]
    public void CellFromOffset_keeps_column_until_hysteresis_threshold()
    {
        // cellWidth 100, spacing 0 → stride 100. Current column 1; x=149 is still inside hysteresis.
        var held = DashboardGridMath.CellFromOffset(149, 0, 600, 6, 96, 0, currentRow: 0, currentColumn: 1);
        Assert.Equal((0, 1), held);

        var moved = DashboardGridMath.CellFromOffset(185, 0, 600, 6, 96, 0, currentRow: 0, currentColumn: 1);
        Assert.Equal((0, 2), moved);
    }

    [Fact]
    public void Density_tokens_scale_content_not_only_padding()
    {
        var compact = DashboardDensityTokens.For(DashboardDensity.Compact);
        var comfortable = DashboardDensityTokens.For(DashboardDensity.Comfortable);
        Assert.True(compact.MetricFontSize < comfortable.MetricFontSize);
        Assert.True(compact.HostPadding < comfortable.HostPadding);
        Assert.True(compact.ListRowHeight < comfortable.ListRowHeight);
        Assert.True(compact.ActionMinHeight < comfortable.ActionMinHeight);
    }

    private static DashboardBreakpointLayout Layout(int columns, params DashboardPlacement[] placements) => new()
    {
        Breakpoint = DashboardBreakpoint.Wide,
        Columns = columns,
        Placements = [.. placements]
    };

    private static DashboardPlacement Placement(string id, int row, int column, int columnSpan, int rowSpan) => new()
    {
        InstanceId = id,
        Row = row,
        Column = column,
        ColumnSpan = columnSpan,
        RowSpan = rowSpan
    };

    private static (int Row, int Column) Position(DashboardBreakpointLayout layout, string id)
    {
        var placement = layout.Placements.Single(value => value.InstanceId == id);
        return (placement.Row, placement.Column);
    }

    private static string TempDirectory() => Path.Combine(Path.GetTempPath(), "Sift-Dashboard-" + Guid.NewGuid().ToString("N"));

    private static void DeleteDirectory(string path)
    {
        try { if (Directory.Exists(path)) Directory.Delete(path, recursive: true); }
        catch { }
    }
}
