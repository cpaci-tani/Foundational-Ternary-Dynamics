using System.IO;
using System.Text;

namespace Sift.Services;

public static class ProcessCsvExporter
{
    public static void Export(string path, IEnumerable<Models.ProcessRow> rows)
    {
        var sb = new StringBuilder();
        sb.AppendLine("Name,PID,CPU%,WorkingMB,PrivateMB,ReadMBs,WriteMBs,Threads,Handles,Status,Priority,Architecture,Session,UptimeSeconds,ExecutablePath,WindowTitle");
        foreach (var row in rows)
        {
            sb.Append(Escape(row.Name)).Append(',')
              .Append(row.Id).Append(',')
              .Append(row.CpuPercent.ToString("0.00")).Append(',')
              .Append(row.MemoryMb.ToString("0.00")).Append(',')
              .Append(row.PrivateMemoryMb.ToString("0.00")).Append(',')
              .Append(row.ReadRateMb.ToString("0.00")).Append(',')
              .Append(row.WriteRateMb.ToString("0.00")).Append(',')
              .Append(row.ThreadCount).Append(',')
              .Append(row.HandleCount).Append(',')
              .Append(Escape(row.Status)).Append(',')
              .Append(Escape(row.Priority)).Append(',')
              .Append(Escape(row.Architecture)).Append(',')
              .Append(row.SessionId).Append(',')
              .Append(row.UptimeSeconds.ToString("0")).Append(',')
              .Append(Escape(row.ExecutablePath)).Append(',')
              .Append(Escape(row.WindowTitle))
              .AppendLine();
        }
        File.WriteAllText(path, sb.ToString(), Encoding.UTF8);
    }

    private static string Escape(string value)
    {
        if (value.Contains('"') || value.Contains(',') || value.Contains('\n') || value.Contains('\r'))
            return $"\"{value.Replace("\"", "\"\"")}\"";
        return value;
    }
}
