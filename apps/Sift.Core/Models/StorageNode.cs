using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;

namespace Sift.Models;

public sealed class StorageNode
{
    public int Index { get; init; }
    public int ParentIndex { get; init; } = -1;
    public required string Name { get; init; }
    public required string FullPath { get; init; }
    public bool IsDirectory { get; init; }
    public bool IsReparsePoint { get; init; }
    public long OwnSize { get; set; }
    public long Size { get; set; }
    public int FileCount { get; set; }
    public DateTime LastWriteUtc { get; set; }
    public List<int> Children { get; } = [];
}

public sealed class StorageTree
{
    public List<StorageNode> Nodes { get; } = [];
    public List<int> RootIndices { get; } = [];
    public Dictionary<string, long> ExtensionBytes { get; } = new(StringComparer.OrdinalIgnoreCase);
    public long TotalSize => RootIndices.Sum(i => Nodes[i].Size);
    public int TotalFiles => RootIndices.Sum(i => Nodes[i].FileCount);

    public StorageNode Add(string name, string fullPath, int parentIndex, bool isDirectory, bool isReparse, long ownSize, DateTime lastWriteUtc)
    {
        var node = new StorageNode
        {
            Index = Nodes.Count,
            ParentIndex = parentIndex,
            Name = name,
            FullPath = fullPath,
            IsDirectory = isDirectory,
            IsReparsePoint = isReparse,
            OwnSize = ownSize,
            Size = ownSize,
            FileCount = isDirectory ? 0 : 1,
            LastWriteUtc = lastWriteUtc
        };
        Nodes.Add(node);
        if (parentIndex >= 0)
            Nodes[parentIndex].Children.Add(node.Index);
        else
            RootIndices.Add(node.Index);
        return node;
    }

    public void Rollup()
    {
        for (var i = Nodes.Count - 1; i >= 0; i--)
        {
            var node = Nodes[i];
            if (node.IsDirectory)
            {
                long size = 0;
                var files = 0;
                foreach (var child in node.Children)
                {
                    size += Nodes[child].Size;
                    files += Nodes[child].FileCount;
                }
                node.Size = size;
                node.FileCount = files;
            }
            else if (!string.IsNullOrEmpty(node.Name))
            {
                var ext = ExtensionOf(node.Name);
                ExtensionBytes[ext] = ExtensionBytes.GetValueOrDefault(ext) + node.Size;
            }
        }
    }

    public static string ExtensionOf(string fileName)
    {
        var ext = Path.GetExtension(fileName);
        return string.IsNullOrEmpty(ext) ? "(no extension)" : ext.ToLowerInvariant();
    }

    public static string ColorHexForExtension(string extension)
    {
        var hash = 0;
        foreach (var c in extension.ToLowerInvariant())
            hash = hash * 31 + c;
        var palette = new[]
        {
            "#B97956",
            "#AD915B",
            "#8B9770",
            "#9C6958",
            "#C09C68",
            "#798565",
            "#A98066",
            "#978969",
        };
        return palette[(hash & int.MaxValue) % palette.Length];
    }
}

public sealed class StorageRow : INotifyPropertyChanged
{
    private bool _isSelected;
    public int NodeIndex { get; init; }
    public required string Name { get; init; }
    public required string FullPath { get; init; }
    public bool IsDirectory { get; init; }
    public bool IsReparsePoint { get; init; }
    public long Size { get; init; }
    public int FileCount { get; init; }
    public double Percent { get; init; }
    public DateTime LastWriteUtc { get; init; }
    public string Swatch { get; init; } = "#808080";

    public string SizeLabel => FormatSize(Size);
    public string PercentLabel => $"{Percent:0.0}%";
    public string FilesLabel => FileCount.ToString("N0");
    public string TypeLabel => IsReparsePoint ? "Link" : IsDirectory ? "Folder" : "File";
    public string LastWriteLabel => LastWriteUtc == default ? "—" : LastWriteUtc.ToLocalTime().ToString("g");

    public bool IsSelected
    {
        get => _isSelected;
        set { _isSelected = value; Changed(); }
    }

    public static string FormatSize(long bytes) => Sift.Presentation.SiftDisplay.Bytes(bytes);

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Changed([CallerMemberName] string? name = null) => PropertyChanged?.Invoke(this, new(name));
}

public sealed class StorageScanProgress
{
    public string CurrentPath { get; init; } = "";
    public long FilesSeen { get; init; }
    public long BytesSeen { get; init; }
    public string Message => $"Scanning {CurrentPath} · {FilesSeen:N0} files · {StorageRow.FormatSize(BytesSeen)}";
}

public sealed class StorageExtensionStat
{
    public required string Extension { get; init; }
    public long Bytes { get; init; }
    public required string Swatch { get; init; }
    public string SizeLabel => StorageRow.FormatSize(Bytes);
}
