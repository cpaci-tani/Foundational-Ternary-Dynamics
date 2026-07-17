using System.Text;
using Sift.Models;

namespace Sift.Services;

public sealed record ScheduledTaskInfo(
    string TaskName,
    string TaskPath,
    string State,
    string Author,
    bool IsAllowlisted,
    string GroupKey,
    ScheduledTaskId? ActionId = null,
    byte[]? IconPng = null);

public static class ScheduledTaskMonitor
{
    public static bool IsAllowlisted(string taskPath, string taskName) =>
        ScheduledTaskIdentityCatalog.IsResolvableIdentity(taskPath, taskName);

    public static IReadOnlyList<ScheduledTaskInfo> Enumerate()
    {
        var results = new List<ScheduledTaskInfo>();
        try
        {
            var startInfo = TweakExecutor.CreateTrustedProcessStartInfo(
                "schtasks.exe", ["/Query", "/FO", "CSV", "/V", "/NH"]);
            startInfo.StandardOutputEncoding = Encoding.UTF8;
            var csv = TweakExecutor.RunProcessAsync(startInfo, TimeSpan.FromSeconds(15), CancellationToken.None)
                .GetAwaiter().GetResult().StandardOutput;
            foreach (var line in csv.Split('\n', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
            {
                var cols = ParseCsv(line);
                if (cols.Count < 4) continue;
                var taskName = cols.Count > 1 ? cols[1].Trim('"') : "";
                if (string.IsNullOrWhiteSpace(taskName) || taskName.Equals("TaskName", StringComparison.OrdinalIgnoreCase))
                    continue;
                var path = "\\";
                var name = taskName;
                var lastSlash = taskName.LastIndexOf('\\');
                if (lastSlash >= 0)
                {
                    path = taskName[..lastSlash];
                    if (string.IsNullOrEmpty(path)) path = "\\";
                    name = taskName[(lastSlash + 1)..];
                }
                var status = cols.Count > 3 ? cols[3].Trim('"') : "";
                var author = cols.Count > 7 ? cols[7].Trim('"') : "";
                var taskToRun = cols.Count > 8 ? cols[8].Trim('"') : "";
                ScheduledTaskId? actionId = null;
                var allow = ScheduledTaskIdentityCatalog.TryResolve(path.EndsWith('\\') ? path : path + "\\", name,
                    out var definition);
                if (allow) actionId = definition.Id;
                results.Add(new ScheduledTaskInfo(name, path, status, author, allow, string.IsNullOrWhiteSpace(path) ? "\\" : path,
                    actionId, AppIconExtractor.TryExtractPngFromCommandLine(taskToRun)));
            }
        }
        catch { /* best effort */ }

        return results
            .GroupBy(x => x.TaskPath + "\\" + x.TaskName, StringComparer.OrdinalIgnoreCase)
            .Select(g => g.First())
            .OrderBy(x => x.TaskPath, StringComparer.OrdinalIgnoreCase)
            .ThenBy(x => x.TaskName, StringComparer.OrdinalIgnoreCase)
            .ToList();
    }

    private static List<string> ParseCsv(string line)
    {
        var cols = new List<string>();
        var sb = new StringBuilder();
        var inQuotes = false;
        foreach (var ch in line)
        {
            if (ch == '"') { inQuotes = !inQuotes; continue; }
            if (ch == ',' && !inQuotes) { cols.Add(sb.ToString()); sb.Clear(); continue; }
            sb.Append(ch);
        }
        cols.Add(sb.ToString());
        return cols;
    }
}
