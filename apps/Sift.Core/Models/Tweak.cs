using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Sift.Models;

public enum TweakRisk { Safe, Moderate, Advanced }
public enum TweakKind { Registry, Command, AppPackage }

public sealed class Tweak : INotifyPropertyChanged
{
    private bool _isSelected;
    private bool _isApplied;
    public required string Id { get; init; }
    public required string Title { get; init; }
    public required string Description { get; init; }
    public required string Category { get; init; }
    public required TweakRisk Risk { get; init; }
    public required TweakKind Kind { get; init; }
    public required string Target { get; init; }
    public string? ValueName { get; init; }
    public object? DesiredValue { get; init; }
    public string? ApplyCommand { get; init; }
    public string? UndoCommand { get; init; }
    public bool Reversible { get; init; } = true;
    public bool Recommended { get; init; }
    public bool Minimal { get; init; }

    /// <summary>
    /// When true, Optimize routes this tweak through the one-shot elevation helper while the shell
    /// is standard-user. HKLM registry tweaks are always treated as elevated regardless of this flag.
    /// </summary>
    public bool RequiresElevation { get; init; }
    public bool IsSelected { get => _isSelected; set { _isSelected = value; Changed(); } }
    public bool IsApplied { get => _isApplied; set { _isApplied = value; Changed(); Changed(nameof(StateLabel)); } }
    public string RiskLabel => Risk == TweakRisk.Safe ? "STANDARD" : Risk.ToString().ToUpperInvariant();
    public string StateLabel => IsApplied ? (Kind == TweakKind.AppPackage ? "NOT INSTALLED" : "ACTIVE") : "AVAILABLE";
    public event PropertyChangedEventHandler? PropertyChanged;
    private void Changed([CallerMemberName] string? name = null) => PropertyChanged?.Invoke(this, new(name));
}
