namespace Sift.Infrastructure.Icons;

/// <summary>Vector path data for a single Sift icon (24×24 viewport).</summary>
public sealed record SiftIconGlyph(
    string? StrokePath,
    string? FillPath = null,
    double StrokeThickness = 1.75,
    bool UseFill = false)
{
    public static SiftIconGlyph Stroke(string path, double thickness = 1.75) =>
        new(path, null, thickness, false);

    public static SiftIconGlyph Fill(string path) =>
        new(null, path, 0, true);
}

/// <summary>Original Sift icon geometry — stroke-first, round caps, 24×24 grid.</summary>
public static class SiftIconGlyphs
{
    private static readonly Dictionary<SiftIconKind, SiftIconGlyph> Glyphs = new()
    {
        [SiftIconKind.Close] = SiftIconGlyph.Stroke("M6 6 L18 18 M18 6 L6 18"),
        [SiftIconKind.Drag] = SiftIconGlyph.Stroke(
            "M9 7 H9.01 M15 7 H15.01 M9 12 H9.01 M15 12 H15.01 M9 17 H9.01 M15 17 H15.01", 2.4),
        [SiftIconKind.Resize] = SiftIconGlyph.Stroke("M14 14 L21 21 M17 14 L21 17 M14 17 L17 21"),
        [SiftIconKind.Menu] = SiftIconGlyph.Stroke("M5 7 H19 M5 12 H19 M5 17 H19"),
        [SiftIconKind.More] = SiftIconGlyph.Stroke(
            "M12 6 H12.01 M12 12 H12.01 M12 18 H12.01", 2.4),
        [SiftIconKind.Pin] = SiftIconGlyph.Stroke(
            "M16 4 V12 M12 12 H20 M12 12 V17 C12 19 14 20 16 20 C18 20 20 19 20 17 V12"),
        [SiftIconKind.Refresh] = SiftIconGlyph.Stroke(
            "M20 12 C20 16.42 16.42 20 12 20 C7.58 20 4 16.42 4 12 C4 7.58 7.58 4 12 4 C15.09 4 17.74 5.84 19.12 8.4 M20 4 V8.4 H15.6"),
        [SiftIconKind.Pause] = SiftIconGlyph.Stroke("M8 7 V17 M16 7 V17"),
        [SiftIconKind.Play] = SiftIconGlyph.Fill("M8 6 L18 12 L8 18 Z"),
        [SiftIconKind.Add] = SiftIconGlyph.Stroke("M12 6 V18 M6 12 H18"),
        [SiftIconKind.Save] = SiftIconGlyph.Stroke("M6 4 H16 L18 6 V20 H6 Z M8 4 V8 H16 V4 M8 12 H16"),
        [SiftIconKind.Cancel] = SiftIconGlyph.Stroke("M8 8 L16 16 M16 8 L8 16"),
        [SiftIconKind.Copy] = SiftIconGlyph.Stroke(
            "M8 10 H6 C5.45 10 5 10.45 5 11 V18 C5 18.55 5.45 19 6 19 H13 C13.55 19 14 18.55 14 18 V16 M10 5 H17 C17.55 5 18 5.45 18 6 V13 C18 13.55 17.55 14 17 14 H10 C9.45 14 9 13.55 9 13 V6 C9 5.45 9.45 5 10 5 Z"),
        [SiftIconKind.Clear] = SiftIconGlyph.Stroke(
            "M9 4 H15 M7 4 V6 H17 V4 M8 6 L9 19 H15 L16 6 M10 9 V16 M14 9 V16"),
        [SiftIconKind.Undo] = SiftIconGlyph.Stroke(
            "M8 8 H15 C17.21 8 19 9.79 19 12 C19 14.21 17.21 16 15 16 H10 M8 8 L11 5 M8 8 L11 11"),
        [SiftIconKind.Redo] = SiftIconGlyph.Stroke(
            "M16 8 H9 C6.79 8 5 9.79 5 12 C5 14.21 6.79 16 9 16 H14 M16 8 L13 5 M16 8 L13 11"),
        [SiftIconKind.Tidy] = SiftIconGlyph.Stroke("M4 6 H10 V12 H4 Z M14 6 H20 V12 H14 Z M4 14 H10 V20 H4 Z M14 14 H20 V20 H14 Z"),
        [SiftIconKind.Search] = SiftIconGlyph.Stroke("M10.5 16 A5.5 5.5 0 1 1 10.5 5 A5.5 5.5 0 0 1 10.5 16 M15 15 L20 20"),
        [SiftIconKind.Reset] = SiftIconGlyph.Stroke(
            "M12 5 C8.13 5 5 8.13 5 12 C5 15.87 8.13 19 12 19 C14.76 19 17.17 17.38 18.36 15 M19 5 V9 H15"),
        [SiftIconKind.Apply] = SiftIconGlyph.Stroke("M6 12 L10 16 L18 8"),
        [SiftIconKind.Run] = SiftIconGlyph.Fill("M7 6 L18 12 L7 18 Z"),
        [SiftIconKind.Stop] = SiftIconGlyph.Fill("M7 7 H17 V17 H7 Z"),
        [SiftIconKind.Analyze] = SiftIconGlyph.Stroke(
            "M4 18 L9 11 L13 15 L20 6 M20 6 H15 M20 6 V11"),
        [SiftIconKind.Scan] = SiftIconGlyph.Stroke(
            "M12 12 M5 12 A7 7 0 0 1 19 12 M12 5 V12 L16 14"),
        [SiftIconKind.Clean] = SiftIconGlyph.Stroke(
            "M12 3 L14 8 H19 L15 11 L16.5 16 L12 13.5 L7.5 16 L9 11 L5 8 H10 Z"),
        [SiftIconKind.Select] = SiftIconGlyph.Stroke("M6 12 L10 16 L18 8"),
        [SiftIconKind.ExpandAll] = SiftIconGlyph.Stroke("M8 9 L12 5 L16 9 M8 15 L12 19 L16 15"),
        [SiftIconKind.CollapseAll] = SiftIconGlyph.Stroke("M5 9 L12 16 L19 9 M5 15 L12 8 L19 15"),
        [SiftIconKind.Up] = SiftIconGlyph.Stroke("M6 14 L12 8 L18 14"),
        [SiftIconKind.OpenFolder] = SiftIconGlyph.Stroke("M4 8 H10 L12 6 H20 V18 H4 Z"),
        [SiftIconKind.OpenExternal] = SiftIconGlyph.Stroke(
            "M12 4 H8 C6.9 4 6 4.9 6 6 V16 C6 17.1 6.9 18 8 18 H16 C17.1 18 18 17.1 18 16 V12 M15 4 H20 V9 M20 4 L11 13"),
        [SiftIconKind.Restore] = SiftIconGlyph.Stroke(
            "M12 6 V3 M12 6 C8.13 6 5 9.13 5 13 C5 16.87 8.13 20 12 20 C15.87 20 19 16.87 19 13 M12 20 V17"),
        [SiftIconKind.Restart] = SiftIconGlyph.Stroke(
            "M20 12 C20 16.42 16.42 20 12 20 C7.58 20 4 16.42 4 12 C4 7.58 7.58 4 12 4 M16 4 H20 V8 M20 4 L15 9"),
        [SiftIconKind.EndTask] = SiftIconGlyph.Stroke("M8 8 L16 16 M16 8 L8 16"),
        [SiftIconKind.Start] = SiftIconGlyph.Stroke("M8 7 L17 12 L8 17 Z"),
        [SiftIconKind.Enable] = SiftIconGlyph.Stroke("M6 12 L10 16 L18 8"),
        [SiftIconKind.Disable] = SiftIconGlyph.Stroke("M7 12 H17"),
        [SiftIconKind.Remove] = SiftIconGlyph.Stroke("M6 12 H18"),
        [SiftIconKind.Manage] = SiftIconGlyph.Stroke(
            "M5 7 H19 M5 12 H19 M5 17 H13 M16 16 A2 2 0 1 1 16 12 A2 2 0 0 1 16 16"),
        [SiftIconKind.Customize] = SiftIconGlyph.Stroke(
            "M5 8 H19 M5 12 H15 M5 16 H11 M17 11 V17 M14 14 H20"),
        [SiftIconKind.Graph] = SiftIconGlyph.Stroke("M4 18 L9 11 L13 15 L20 6"),
        [SiftIconKind.Arrange] = SiftIconGlyph.Stroke("M4 6 H10 V12 H4 Z M14 6 H20 V10 H14 Z M14 14 H20 V20 H14 Z M4 16 H10 V20 H4 Z"),
        [SiftIconKind.Done] = SiftIconGlyph.Stroke("M6 12 L10 16 L18 8"),
        [SiftIconKind.PopOut] = SiftIconGlyph.Stroke(
            "M8 4 H4 V8 M16 4 H20 V8 M20 16 V20 H16 M8 20 H4 V16 M14 10 H18 V14 H14 Z"),
        [SiftIconKind.ActivityLog] = SiftIconGlyph.Stroke("M5 7 H19 M5 12 H15 M5 17 H17"),
        [SiftIconKind.Hide] = SiftIconGlyph.Stroke(
            "M4 12 C6 8 9 6 12 6 C15 6 18 8 20 12 C18 16 15 18 12 18 C9 18 6 16 4 12 M9 15 L15 9"),
        [SiftIconKind.Show] = SiftIconGlyph.Stroke(
            "M4 12 C6 8 9 6 12 6 C15 6 18 8 20 12 C18 16 15 18 12 18 C9 18 6 16 4 12 M12 9 A3 3 0 1 1 12 15 A3 3 0 0 1 12 9"),
        [SiftIconKind.NavHome] = SiftIconGlyph.Stroke("M4 11 L12 5 L20 11 V19 H15 V14 H9 V19 H4 Z"),
        [SiftIconKind.NavOptimize] = SiftIconGlyph.Stroke("M13 3 L19 9 L13 15 L7 9 Z M5 17 H19"),
        [SiftIconKind.NavTaskManager] = SiftIconGlyph.Stroke("M5 6 H19 V18 H5 Z M9 6 V18 M15 6 V18"),
        [SiftIconKind.NavPerformance] = SiftIconGlyph.Stroke("M4 18 L8 12 L12 15 L16 8 L20 14"),
        [SiftIconKind.NavHardware] = SiftIconGlyph.Stroke(
            "M8 6 H16 V18 H8 Z M10 9 H14 M10 12 H14 M10 15 H12"),
        [SiftIconKind.NavStartup] = SiftIconGlyph.Stroke("M12 4 V12 M8 8 L12 4 L16 8 M6 18 H18"),
        [SiftIconKind.NavMaintenance] = SiftIconGlyph.Stroke(
            "M14 4 L18 8 L11 15 L7 15 L7 11 Z M6 18 H18"),
        [SiftIconKind.NavScripts] = SiftIconGlyph.Stroke("M8 6 L16 12 L8 18 V6 Z M5 6 H6 V18 H5 Z"),
        [SiftIconKind.NavHealth] = SiftIconGlyph.Stroke("M12 20 C12 20 5 15 5 10 A4 4 0 0 1 12 7 A4 4 0 0 1 19 10 C19 15 12 20 12 20"),
        [SiftIconKind.NavRecovery] = SiftIconGlyph.Stroke(
            "M12 6 V3 M12 6 C8.13 6 5 9.13 5 13 C5 16.87 8.13 20 12 20 C15.87 20 19 16.87 19 13"),
        [SiftIconKind.NavStorage] = SiftIconGlyph.Stroke("M6 8 H18 V18 H6 Z M8 8 V6 H16 V8"),
        [SiftIconKind.NavApps] = SiftIconGlyph.Stroke("M5 5 H10 V10 H5 Z M14 5 H19 V10 H14 Z M5 14 H10 V19 H5 Z M14 14 H19 V19 H14 Z"),
        [SiftIconKind.NavSystemInfo] = SiftIconGlyph.Stroke("M12 16 V12 M12 8 H12.01 M12 4 A8 8 0 1 1 12 20 A8 8 0 0 1 12 4"),
        [SiftIconKind.NavSettings] = SiftIconGlyph.Stroke(
            "M12 8 A4 4 0 1 1 12 16 A4 4 0 0 1 12 8 M12 4 V5.5 M12 18.5 V20 M4 12 H5.5 M18.5 12 H20 M6.1 6.1 L7.2 7.2 M16.8 16.8 L17.9 17.9 M6.1 17.9 L7.2 16.8 M16.8 7.2 L17.9 6.1"),
        [SiftIconKind.EmptyActivity] = SiftIconGlyph.Stroke("M5 7 H19 M5 12 H15 M5 17 H17"),
        [SiftIconKind.EmptyStorage] = SiftIconGlyph.Stroke("M6 8 H18 V18 H6 Z M8 8 V6 H16 V8")
    };

    public static bool TryGet(SiftIconKind kind, out SiftIconGlyph glyph) => Glyphs.TryGetValue(kind, out glyph!);

    public static SiftIconGlyph Get(SiftIconKind kind) =>
        Glyphs.TryGetValue(kind, out var glyph) ? glyph : SiftIconGlyph.Stroke(string.Empty);

    public static string? GetPathData(SiftIconKind kind)
    {
        var glyph = Get(kind);
        return glyph.UseFill ? glyph.FillPath : glyph.StrokePath;
    }
}
