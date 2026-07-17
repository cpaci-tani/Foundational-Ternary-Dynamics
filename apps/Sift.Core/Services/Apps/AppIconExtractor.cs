using System.Collections.Concurrent;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.Versioning;
using Windows.Win32;

namespace Sift.Services;

/// <summary>
/// Extracts a small PNG thumbnail from an exact icon reference (registry <c>DisplayIcon</c>,
/// process module path, service ImagePath, or scheduled-task "Task To Run"). The reference is used
/// verbatim; Sift never synthesizes a path from display text and never sweeps directories.
/// </summary>
[SupportedOSPlatform("windows10.0.17763.0")]
public static class AppIconExtractor
{
    private const int MaxCacheEntries = 512;
    private static readonly ConcurrentDictionary<string, byte[]?> Cache = new(StringComparer.OrdinalIgnoreCase);

    /// <summary>
    /// Parses a registry-style icon reference into a filesystem path and icon index.
    /// Accepts optional surrounding quotes and a trailing <c>,index</c>; expands environment
    /// variables. Negative resource-id indices collapse to the first icon. Existence is not checked.
    /// </summary>
    public static bool TryParseIconReference(string? displayIcon, out string path, out int index)
    {
        path = string.Empty;
        index = 0;
        if (string.IsNullOrWhiteSpace(displayIcon)) return false;

        var raw = displayIcon.Trim();
        var candidate = raw;
        var comma = raw.LastIndexOf(',');
        if (comma > 0 && int.TryParse(raw[(comma + 1)..].Trim(), out var parsedIndex))
        {
            candidate = raw[..comma];
            index = parsedIndex < 0 ? 0 : parsedIndex;
        }

        candidate = candidate.Trim().Trim('"').Trim();
        if (candidate.Length == 0) return false;

        try { candidate = Environment.ExpandEnvironmentVariables(candidate); }
        catch { return false; }

        path = candidate;
        return true;
    }

    /// <summary>
    /// Extracts the leading filesystem path from a quoted or unquoted command line
    /// (service ImagePath / scheduled-task "Task To Run"), then returns its icon PNG when possible.
    /// </summary>
    public static byte[]? TryExtractPngFromCommandLine(string? command)
    {
        if (string.IsNullOrWhiteSpace(command)) return null;
        return TryExtractPng(ExtractLeadingPath(command.Trim()));
    }

    /// <summary>
    /// Returns a PNG-encoded icon thumbnail for the declared reference, or <c>null</c> when the
    /// reference is unusable, the file is missing, or extraction fails. Never throws.
    /// Results are cached by path+index for repeated inventory refreshes.
    /// </summary>
    public static byte[]? TryExtractPng(string? displayIcon)
    {
        if (!TryParseIconReference(displayIcon, out var path, out var index)) return null;
        var key = index == 0 ? path : $"{path}|{index}";
        if (Cache.TryGetValue(key, out var cached)) return cached;

        try
        {
            if (!File.Exists(path))
            {
                Remember(key, null);
                return null;
            }
        }
        catch
        {
            Remember(key, null);
            return null;
        }

        var png = ExtractPngCore(path, index);
        Remember(key, png);
        return png;
    }

    internal static string ExtractLeadingPath(string command)
    {
        if (command.StartsWith('"'))
        {
            var end = command.IndexOf('"', 1);
            return end > 1 ? command[1..end] : command.Trim('"');
        }

        foreach (var extension in new[] { ".exe", ".dll", ".sys", ".com", ".msi", ".scr", ".lnk" })
        {
            var index = command.IndexOf(extension, StringComparison.OrdinalIgnoreCase);
            if (index >= 0) return command[..(index + extension.Length)].Trim();
        }

        return command.Split(' ', 2)[0];
    }

    private static byte[]? ExtractPngCore(string path, int index)
    {
        PInvoke.ExtractIconEx(path, index, out DestroyIconSafeHandle large, out DestroyIconSafeHandle small, 1);
        try
        {
            if (large.IsInvalid && small.IsInvalid && index != 0)
            {
                large.Dispose();
                small.Dispose();
                PInvoke.ExtractIconEx(path, 0, out large, out small, 1);
            }

            var handle = !large.IsInvalid ? large.DangerousGetHandle() : small.DangerousGetHandle();
            if (handle == IntPtr.Zero) return null;

            using var icon = Icon.FromHandle(handle);
            using var bitmap = icon.ToBitmap();
            using var memory = new MemoryStream();
            bitmap.Save(memory, ImageFormat.Png);
            return memory.ToArray();
        }
        catch
        {
            return null;
        }
        finally
        {
            large.Dispose();
            small.Dispose();
        }
    }

    private static void Remember(string key, byte[]? png)
    {
        if (Cache.Count >= MaxCacheEntries) Cache.Clear();
        Cache[key] = png;
    }
}
