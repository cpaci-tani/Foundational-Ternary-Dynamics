using Sift.Models;

namespace Sift.Services;

public sealed class DashboardEditSession
{
    private const int MaximumUndoDepth = 50;
    private readonly Stack<DashboardProfile> _undo = [];
    private readonly Stack<DashboardProfile> _redo = [];

    public DashboardEditSession(DashboardProfile source) => WorkingProfile = Copy(source);

    public DashboardProfile WorkingProfile { get; private set; }
    public bool CanUndo => _undo.Count > 0;
    public bool CanRedo => _redo.Count > 0;

    public void Apply(Action<DashboardProfile> mutation)
    {
        ArgumentNullException.ThrowIfNull(mutation);
        _undo.Push(Copy(WorkingProfile));
        while (_undo.Count > MaximumUndoDepth)
        {
            var keep = _undo.Take(MaximumUndoDepth).Reverse().ToArray();
            _undo.Clear();
            foreach (var value in keep) _undo.Push(value);
        }
        _redo.Clear();
        mutation(WorkingProfile);
    }

    public bool Undo()
    {
        if (_undo.Count == 0) return false;
        _redo.Push(Copy(WorkingProfile));
        WorkingProfile = _undo.Pop();
        return true;
    }

    public bool Redo()
    {
        if (_redo.Count == 0) return false;
        _undo.Push(Copy(WorkingProfile));
        WorkingProfile = _redo.Pop();
        return true;
    }

    public DashboardProfile Commit() => Copy(WorkingProfile);

    public static DashboardProfile Copy(DashboardProfile source) => new()
    {
        Id = source.Id,
        Name = source.Name,
        IsBuiltIn = source.IsBuiltIn,
        Density = source.Density,
        Widgets = source.Widgets.Select(widget => new DashboardWidgetInstance
        {
            InstanceId = widget.InstanceId,
            DefinitionId = widget.DefinitionId,
            TitleOverride = widget.TitleOverride,
            Accent = widget.Accent,
            Settings = new Dictionary<string, string>(widget.Settings, StringComparer.OrdinalIgnoreCase)
        }).ToList(),
        Layouts = source.Layouts.Select(layout => layout.Copy()).ToList()
    };
}
