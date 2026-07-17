using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Sift.Models;

namespace Sift.WinUI.Models;

public sealed class OptimizeCategoryGroup : INotifyPropertyChanged
{
    private string _countLabel = "0 selected · 0";

    public required string Category { get; init; }
    public ObservableCollection<Tweak> Tweaks { get; } = [];
    public string Header => Category;
    public string CountLabel
    {
        get => _countLabel;
        private set
        {
            if (_countLabel == value) return;
            _countLabel = value;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(CountLabel)));
        }
    }

    public void RefreshCount() =>
        CountLabel = $"{Tweaks.Count(tweak => tweak.IsSelected):N0} selected · {Tweaks.Count:N0}";

    public event PropertyChangedEventHandler? PropertyChanged;
}