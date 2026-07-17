using Sift.Services;

namespace Sift.UnitTests;

public sealed class AppIconExtractorTests
{
    [Fact]
    public void Parses_quoted_path_with_index()
    {
        var ok = AppIconExtractor.TryParseIconReference("\"C:\\Program Files\\App\\app.exe\",0", out var path, out var index);
        Assert.True(ok);
        Assert.Equal("C:\\Program Files\\App\\app.exe", path);
        Assert.Equal(0, index);
    }

    [Fact]
    public void Parses_unquoted_path_with_positive_index()
    {
        var ok = AppIconExtractor.TryParseIconReference("C:\\Windows\\System32\\shell32.dll,42", out var path, out var index);
        Assert.True(ok);
        Assert.Equal("C:\\Windows\\System32\\shell32.dll", path);
        Assert.Equal(42, index);
    }

    [Fact]
    public void Parses_path_without_index()
    {
        var ok = AppIconExtractor.TryParseIconReference("C:\\App\\icon.ico", out var path, out var index);
        Assert.True(ok);
        Assert.Equal("C:\\App\\icon.ico", path);
        Assert.Equal(0, index);
    }

    [Fact]
    public void Collapses_negative_resource_index_to_zero()
    {
        var ok = AppIconExtractor.TryParseIconReference("C:\\App\\app.exe,-15", out var path, out var index);
        Assert.True(ok);
        Assert.Equal("C:\\App\\app.exe", path);
        Assert.Equal(0, index);
    }

    [Fact]
    public void Keeps_commas_inside_the_path_when_tail_is_not_an_index()
    {
        var ok = AppIconExtractor.TryParseIconReference("C:\\a,b\\thing.exe", out var path, out var index);
        Assert.True(ok);
        Assert.Equal("C:\\a,b\\thing.exe", path);
        Assert.Equal(0, index);
    }

    [Fact]
    public void Expands_environment_variables()
    {
        var expected = Environment.ExpandEnvironmentVariables("%SystemRoot%\\System32\\imageres.dll");
        var ok = AppIconExtractor.TryParseIconReference("%SystemRoot%\\System32\\imageres.dll,15", out var path, out var index);
        Assert.True(ok);
        Assert.Equal(expected, path);
        Assert.Equal(15, index);
    }

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("   ")]
    [InlineData("\"\"")]
    public void Rejects_blank_or_empty_references(string? reference)
    {
        Assert.False(AppIconExtractor.TryParseIconReference(reference, out _, out _));
    }

    [Fact]
    public void Extracts_png_from_quoted_command_line_pointing_at_notepad()
    {
        var notepad = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "notepad.exe");
        Assert.True(File.Exists(notepad));
        var png = AppIconExtractor.TryExtractPngFromCommandLine($"\"{notepad}\" /A");
        Assert.NotNull(png);
        Assert.True(png!.Length > 32);
        Assert.Equal(0x89, png[0]);
        Assert.Equal((byte)'P', png[1]);
        Assert.Equal((byte)'N', png[2]);
        Assert.Equal((byte)'G', png[3]);
    }

    [Fact]
    public void Command_line_without_existing_file_returns_null()
    {
        Assert.Null(AppIconExtractor.TryExtractPngFromCommandLine(
            "\"C:\\Sift.DoesNotExist.IconFixture\\missing.exe\" -service"));
    }
}
