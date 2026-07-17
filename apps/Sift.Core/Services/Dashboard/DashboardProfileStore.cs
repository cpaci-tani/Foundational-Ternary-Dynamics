using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text;
using Sift.Infrastructure.Persistence;
using Sift.Models;

namespace Sift.Services;

public interface IDashboardProfileStore
{
    string ProfilePath { get; }
    DashboardProfileDocument LoadOrCreate(IReadOnlyDictionary<string, bool>? legacyVisibility = null);
    void Save(DashboardProfileDocument document);
    string ExportProfile(DashboardProfile profile);
    DashboardProfile ImportProfile(DashboardProfileDocument document, string json);
}

public sealed class DashboardProfileStore : IDashboardProfileStore
{
    public const int MaximumDocumentBytes = 4 * 1024 * 1024;
    public const int MaximumImportBytes = 512 * 1024;
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNameCaseInsensitive = false,
        UnmappedMemberHandling = JsonUnmappedMemberHandling.Disallow
    };

    public DashboardProfileStore(string? directory = null)
    {
        var root = directory ?? ProductPaths.DataRoot;
        ProfilePath = Path.Combine(root, "dashboard-profiles.json");
    }

    public string ProfilePath { get; }

    public DashboardProfileDocument LoadOrCreate(IReadOnlyDictionary<string, bool>? legacyVisibility = null)
    {
        if (!File.Exists(ProfilePath))
        {
            var created = DashboardProfileDefaults.Create(legacyVisibility);
            Save(created);
            return created;
        }

        try
        {
            if (new FileInfo(ProfilePath).Length > MaximumDocumentBytes)
                throw new InvalidDataException("The dashboard profile document is too large.");
            var document = JsonSerializer.Deserialize<DashboardProfileDocument>(File.ReadAllText(ProfilePath), JsonOptions)
                ?? throw new InvalidDataException("The dashboard profile document is empty.");
            ValidateDocument(document);
            return document;
        }
        catch (Exception exception) when (exception is JsonException or InvalidDataException or IOException or ArgumentException)
        {
            QuarantineCorruptFile();
            var created = DashboardProfileDefaults.Create(legacyVisibility);
            Save(created);
            return created;
        }
    }

    public void Save(DashboardProfileDocument document)
    {
        ArgumentNullException.ThrowIfNull(document);
        ValidateDocument(document);
        var json = JsonSerializer.Serialize(document, JsonOptions);
        if (Encoding.UTF8.GetByteCount(json) > MaximumDocumentBytes)
            throw new InvalidDataException("The dashboard profile document is too large.");
        AtomicFile.WriteAllText(ProfilePath, json);
    }

    public string ExportProfile(DashboardProfile profile)
    {
        ArgumentNullException.ThrowIfNull(profile);
        ValidateProfile(profile, DashboardWidgetCatalog.ById());
        var export = new DashboardProfileDocument
        {
            ActiveProfileId = profile.Id,
            Profiles = [profile]
        };
        return JsonSerializer.Serialize(export, JsonOptions);
    }

    public DashboardProfile ImportProfile(DashboardProfileDocument document, string json)
    {
        ArgumentNullException.ThrowIfNull(document);
        ArgumentException.ThrowIfNullOrWhiteSpace(json);
        if (Encoding.UTF8.GetByteCount(json) > MaximumImportBytes)
            throw new InvalidDataException("The imported dashboard profile is too large.");
        var import = JsonSerializer.Deserialize<DashboardProfileDocument>(json, JsonOptions)
            ?? throw new InvalidDataException("The imported dashboard profile is empty.");
        if (import.SchemaVersion != DashboardProfileDocument.CurrentSchemaVersion)
            throw new InvalidDataException($"Dashboard profile schema {import.SchemaVersion} is not supported.");
        if (import.Profiles.Count != 1)
            throw new InvalidDataException("A dashboard profile export must contain exactly one profile.");
        ValidateProfile(import.Profiles[0], DashboardWidgetCatalog.ById());

        var source = import.Profiles[0];
        var uniqueName = UniqueName(document, source.Name);
        var imported = CloneProfile(source, Guid.NewGuid().ToString("N"), uniqueName, isBuiltIn: false);
        document.Profiles.Add(imported);
        return imported;
    }

    public static DashboardProfile CloneProfile(DashboardProfile source, string id, string name, bool isBuiltIn = false)
    {
        var instanceIds = source.Widgets.Select((widget, index) => new
        {
            widget.InstanceId,
            NewId = $"{id}.{widget.DefinitionId}.{index + 1}"
        }).ToDictionary(pair => pair.InstanceId, pair => pair.NewId, StringComparer.OrdinalIgnoreCase);
        return new DashboardProfile
        {
            Id = id,
            Name = name,
            IsBuiltIn = isBuiltIn,
            Density = source.Density,
            Widgets = source.Widgets.Select(widget => new DashboardWidgetInstance
            {
                InstanceId = instanceIds[widget.InstanceId],
                DefinitionId = widget.DefinitionId,
                TitleOverride = widget.TitleOverride,
                Accent = widget.Accent,
                Settings = new Dictionary<string, string>(widget.Settings, StringComparer.OrdinalIgnoreCase)
            }).ToList(),
            Layouts = source.Layouts.Select(layout => new DashboardBreakpointLayout
            {
                Breakpoint = layout.Breakpoint,
                Columns = layout.Columns,
                Placements = layout.Placements.Select(placement =>
                {
                    var copy = placement.Copy();
                    return new DashboardPlacement
                    {
                        InstanceId = instanceIds[copy.InstanceId],
                        Row = copy.Row,
                        Column = copy.Column,
                        RowSpan = copy.RowSpan,
                        ColumnSpan = copy.ColumnSpan,
                        Visible = copy.Visible
                    };
                }).ToList()
            }).ToList()
        };
    }

    private static void ValidateDocument(DashboardProfileDocument document)
    {
        if (document.SchemaVersion != DashboardProfileDocument.CurrentSchemaVersion)
            throw new InvalidDataException($"Dashboard profile schema {document.SchemaVersion} is not supported.");
        if (document.Profiles is null || document.Profiles.Any(profile => profile is null))
            throw new InvalidDataException("Dashboard profiles are missing.");
        if (document.Profiles.Count is 0 or > 32) throw new InvalidDataException("Dashboard profile count is invalid.");
        if (document.Profiles.Select(profile => profile.Id).Distinct(StringComparer.OrdinalIgnoreCase).Count() != document.Profiles.Count)
            throw new InvalidDataException("Dashboard profile identifiers must be unique.");
        var definitions = DashboardWidgetCatalog.ById();
        foreach (var profile in document.Profiles) ValidateProfile(profile, definitions);
        if (string.IsNullOrWhiteSpace(document.ActiveProfileId) ||
            !document.Profiles.Any(profile => string.Equals(profile.Id, document.ActiveProfileId, StringComparison.OrdinalIgnoreCase)))
            throw new InvalidDataException("The active dashboard profile is missing.");
    }

    private static void ValidateProfile(
        DashboardProfile profile,
        IReadOnlyDictionary<string, DashboardWidgetDefinition> definitions)
    {
        if (string.IsNullOrWhiteSpace(profile.Id) || profile.Id.Length > 80 ||
            string.IsNullOrWhiteSpace(profile.Name) || profile.Name.Length > 80)
            throw new InvalidDataException("Dashboard profile identity is invalid.");
        if (profile.Widgets is null || profile.Layouts is null ||
            profile.Widgets.Any(widget => widget is null) || profile.Layouts.Any(layout => layout is null))
            throw new InvalidDataException("Dashboard profile collections are missing.");
        if (profile.Widgets.Count is 0 or > 64) throw new InvalidDataException("Dashboard widget count is invalid.");
        if (profile.Widgets.Any(widget => string.IsNullOrWhiteSpace(widget.InstanceId)) ||
            profile.Widgets.Select(widget => widget.InstanceId).Distinct(StringComparer.OrdinalIgnoreCase).Count() != profile.Widgets.Count)
            throw new InvalidDataException("Dashboard widget instance identifiers must be present and unique.");
        var instances = profile.Widgets.ToDictionary(widget => widget.InstanceId, StringComparer.OrdinalIgnoreCase);
        foreach (var widget in profile.Widgets)
        {
            if (string.IsNullOrWhiteSpace(widget.InstanceId) || widget.InstanceId.Length > 160 ||
                string.IsNullOrWhiteSpace(widget.DefinitionId) ||
                !definitions.TryGetValue(widget.DefinitionId, out var definition))
                throw new InvalidDataException($"Dashboard widget '{widget.InstanceId}' is not supported.");
            if (widget.TitleOverride is { } title &&
                (title.Length > 120 || title.Any(char.IsControl)))
                throw new InvalidDataException($"Dashboard widget title is invalid: {widget.InstanceId}.");
            if (widget.Accent is not null && widget.Accent is not ("Clay" or "Sage" or "Neutral"))
                throw new InvalidDataException($"Dashboard widget accent is invalid: {widget.InstanceId}.");
            if (widget.Settings is null)
                throw new InvalidDataException($"Dashboard widget settings are missing: {widget.InstanceId}.");
            if (widget.Settings.Count > 32 || widget.Settings.Any(pair => pair.Key is null || pair.Value is null ||
                    pair.Key.Length > 80 || pair.Value.Length > 512))
                throw new InvalidDataException($"Dashboard widget settings are invalid: {widget.InstanceId}.");
            var permittedSettings = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            {
                "preset", "timeRange", "metric", "sensor", "volume", "sort", "count", "filter", "showActions"
            };
            if (widget.Settings.Keys.Any(key => !permittedSettings.Contains(key) ||
                    key.Contains("path", StringComparison.OrdinalIgnoreCase) ||
                    key.Contains("uri", StringComparison.OrdinalIgnoreCase) ||
                    key.Contains("command", StringComparison.OrdinalIgnoreCase) ||
                    key.Contains("executable", StringComparison.OrdinalIgnoreCase)))
                throw new InvalidDataException($"Dashboard widget settings contain an unsupported field: {widget.InstanceId}.");
            foreach (var setting in widget.Settings)
            {
                if (!definition.SettingKeys.Contains(setting.Key, StringComparer.OrdinalIgnoreCase))
                    throw new InvalidDataException($"Dashboard widget setting '{setting.Key}' is not supported by {widget.DefinitionId}.");
                ValidateSetting(widget.InstanceId, setting.Key, setting.Value);
            }
            if (!definition.AllowMultiple && profile.Widgets.Count(candidate =>
                    candidate.DefinitionId.Equals(widget.DefinitionId, StringComparison.OrdinalIgnoreCase)) > 1)
                throw new InvalidDataException($"Dashboard widget '{widget.DefinitionId}' cannot be duplicated.");
        }
        if (profile.Layouts.Select(layout => layout.Breakpoint).Distinct().Count() != 3 || profile.Layouts.Count != 3 ||
            !profile.Layouts.Select(layout => layout.Breakpoint).ToHashSet().SetEquals(Enum.GetValues<DashboardBreakpoint>()))
            throw new InvalidDataException("Every dashboard profile requires Wide, Medium, and Compact layouts.");
        foreach (var layout in profile.Layouts)
        {
            var expectedColumns = layout.Breakpoint switch
            {
                DashboardBreakpoint.Wide => 6,
                DashboardBreakpoint.Medium => 4,
                DashboardBreakpoint.Compact => 2,
                _ => 0
            };
            if (layout.Columns != expectedColumns)
                throw new InvalidDataException($"{layout.Breakpoint} requires exactly {expectedColumns} columns.");
            if (layout.Placements is null || layout.Placements.Any(placement => placement is null ||
                    string.IsNullOrWhiteSpace(placement.InstanceId)))
                throw new InvalidDataException($"Dashboard placements are missing in {layout.Breakpoint}.");
            var errors = DashboardPackingEngine.Validate(layout, instances, definitions);
            if (errors.Count > 0) throw new InvalidDataException(string.Join(" ", errors));
            if (layout.Placements.Count != profile.Widgets.Count ||
                layout.Placements.Select(placement => placement.InstanceId).Distinct(StringComparer.OrdinalIgnoreCase).Count() != profile.Widgets.Count)
                throw new InvalidDataException($"Every widget must have one placement in {layout.Breakpoint}.");
        }
    }

    private static void ValidateSetting(string instanceId, string key, string value)
    {
        var valid = key.ToLowerInvariant() switch
        {
            "preset" => value.Length <= 80 && SafeToken(value),
            "timerange" => value is "30 minutes" or "24 hours" or "7 days" or "30 days" or "90 days",
            "metric" or "sensor" => value.Length <= 160 && value.All(character =>
                char.IsLetterOrDigit(character) || character is '.' or '_' or '-'),
            "volume" => value.Length == 2 && char.IsAsciiLetter(value[0]) && value[1] == ':',
            "sort" => value is "Automatic" or "Highest first" or "Lowest first" or "Name",
            "count" => int.TryParse(value, out var count) && count is >= 1 and <= 20,
            "filter" => value.Length <= 120 && !value.Contains("://", StringComparison.Ordinal) &&
                        !value.Contains('\\') && !value.Contains('/'),
            "showactions" => bool.TryParse(value, out _),
            _ => false
        };
        if (!valid) throw new InvalidDataException($"Dashboard widget setting '{key}' is invalid: {instanceId}.");
    }

    private static bool SafeToken(string value) => value.All(character =>
        char.IsLetterOrDigit(character) || character is '.' or '_' or '-');

    private static string UniqueName(DashboardProfileDocument document, string name)
    {
        if (!document.Profiles.Any(profile => profile.Name.Equals(name, StringComparison.OrdinalIgnoreCase))) return name;
        for (var suffix = 2; suffix <= 99; suffix++)
        {
            var candidate = $"{name} ({suffix})";
            if (!document.Profiles.Any(profile => profile.Name.Equals(candidate, StringComparison.OrdinalIgnoreCase))) return candidate;
        }
        return $"{name} {Guid.NewGuid():N}"[..80];
    }

    private void QuarantineCorruptFile()
    {
        try
        {
            if (!File.Exists(ProfilePath)) return;
            var quarantine = Path.Combine(Path.GetDirectoryName(ProfilePath)!,
                $"dashboard-profiles.corrupt-{DateTime.UtcNow:yyyyMMddHHmmssfff}.json");
            File.Move(ProfilePath, quarantine, overwrite: false);
        }
        catch { /* A fresh validated document is still preferable to failing application startup. */ }
    }
}
