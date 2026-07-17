using System.Globalization;
using System.Text.Json;
using Sift.Models;
using Microsoft.Win32;

namespace Sift.Services;

internal static class RegistrySnapshotCodec
{
    internal sealed record CapturedValue(bool KeyExisted, bool ValueExisted, RegistryValueSnapshot? Snapshot);

    public static CapturedValue CaptureValue(RegistryKey hive, string subKey, string valueName)
    {
        using var key = hive.OpenSubKey(subKey);
        if (key is null) return new(false, false, null);
        if (!key.GetValueNames().Contains(valueName, StringComparer.OrdinalIgnoreCase))
            return new(true, false, null);

        var kind = key.GetValueKind(valueName);
        var value = key.GetValue(valueName, null, RegistryValueOptions.DoNotExpandEnvironmentNames);
        return new(true, true, Encode(valueName, kind, value));
    }

    public static RegistryKeySnapshot CaptureTree(RegistryKey key)
    {
        var snapshot = new RegistryKeySnapshot();
        foreach (var name in key.GetValueNames())
        {
            var kind = key.GetValueKind(name);
            var value = key.GetValue(name, null, RegistryValueOptions.DoNotExpandEnvironmentNames);
            snapshot.Values.Add(Encode(name, kind, value));
        }

        foreach (var name in key.GetSubKeyNames())
        {
            using var child = key.OpenSubKey(name);
            if (child is not null) snapshot.SubKeys[name] = CaptureTree(child);
        }
        return snapshot;
    }

    public static void RestoreValue(RegistryKey hive, string subKey, string valueName, BackupEntry entry)
    {
        if (!entry.KeyExisted && !entry.Existed)
        {
            var removeEmptyKey = false;
            using (var created = hive.OpenSubKey(subKey, writable: true))
            {
                if (created is null) return;
                created.DeleteValue(valueName, throwOnMissingValue: false);
                removeEmptyKey = created.ValueCount == 0 && created.SubKeyCount == 0;
            }
            if (removeEmptyKey) DeleteEmptySubKey(hive, subKey);
            return;
        }

        using var key = hive.CreateSubKey(subKey, writable: true)
            ?? throw new InvalidOperationException($"Could not open registry key {subKey} for restore.");
        if (!entry.Existed)
        {
            key.DeleteValue(valueName, throwOnMissingValue: false);
            return;
        }

        if (entry.RegistryValue is not null)
        {
            WriteValue(key, entry.RegistryValue);
            return;
        }

        RestoreLegacyValue(key, valueName, entry);
    }

    public static void RestoreTree(RegistryKey hive, string subKey, RegistryKeySnapshot snapshot)
    {
        using var key = hive.CreateSubKey(subKey, writable: true)
            ?? throw new InvalidOperationException($"Could not recreate registry key {subKey}.");
        foreach (var value in snapshot.Values) WriteValue(key, value);
        foreach (var child in snapshot.SubKeys) RestoreTree(key, child.Key, child.Value);
    }

    private static RegistryValueSnapshot Encode(string name, RegistryValueKind kind, object? value) => kind switch
    {
        RegistryValueKind.DWord => Snapshot(name, kind, "Int32", Convert.ToInt32(value, CultureInfo.InvariantCulture).ToString(CultureInfo.InvariantCulture)),
        RegistryValueKind.QWord => Snapshot(name, kind, "Int64", Convert.ToInt64(value, CultureInfo.InvariantCulture).ToString(CultureInfo.InvariantCulture)),
        RegistryValueKind.MultiString => Snapshot(name, kind, "StringArray", JsonSerializer.Serialize(value as string[] ?? [])),
        RegistryValueKind.Binary or RegistryValueKind.None => Snapshot(name, kind, "Bytes", Convert.ToBase64String(value as byte[] ?? [])),
        _ => Snapshot(name, kind, "String", value?.ToString() ?? string.Empty)
    };

    private static RegistryValueSnapshot Snapshot(string name, RegistryValueKind kind, string encoding, string data) =>
        new() { Name = name, Kind = kind.ToString(), Encoding = encoding, Data = data };

    private static void WriteValue(RegistryKey key, RegistryValueSnapshot snapshot)
    {
        if (!Enum.TryParse<RegistryValueKind>(snapshot.Kind, out var kind))
            throw new InvalidDataException($"Unknown registry kind '{snapshot.Kind}'.");
        object value = snapshot.Encoding switch
        {
            "Int32" => int.Parse(snapshot.Data, CultureInfo.InvariantCulture),
            "Int64" => long.Parse(snapshot.Data, CultureInfo.InvariantCulture),
            "StringArray" => JsonSerializer.Deserialize<string[]>(snapshot.Data) ?? [],
            "Bytes" => Convert.FromBase64String(snapshot.Data),
            "String" => snapshot.Data,
            _ => throw new InvalidDataException($"Unknown registry encoding '{snapshot.Encoding}'.")
        };
        key.SetValue(snapshot.Name, value, kind);
    }

    private static void RestoreLegacyValue(RegistryKey key, string valueName, BackupEntry entry)
    {
        var kind = Enum.TryParse<RegistryValueKind>(entry.RegistryKind, out var parsed) ? parsed : RegistryValueKind.String;
        object value = kind switch
        {
            RegistryValueKind.DWord => int.Parse(entry.Value ?? "0", CultureInfo.InvariantCulture),
            RegistryValueKind.QWord => long.Parse(entry.Value ?? "0", CultureInfo.InvariantCulture),
            _ => entry.Value ?? string.Empty
        };
        key.SetValue(valueName, value, kind);
    }

    private static void DeleteEmptySubKey(RegistryKey hive, string subKey)
    {
        var lastSlash = subKey.LastIndexOf('\\');
        if (lastSlash <= 0)
        {
            hive.DeleteSubKey(subKey, throwOnMissingSubKey: false);
            return;
        }
        using var parent = hive.OpenSubKey(subKey[..lastSlash], writable: true);
        parent?.DeleteSubKey(subKey[(lastSlash + 1)..], throwOnMissingSubKey: false);
    }
}
