using System.Collections.ObjectModel;
using Sift.Models;
using Sift.Services;
using Sift.WinUI.Infrastructure.Dialogs;
using Sift.WinUI.Models;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;

namespace Sift.WinUI.Views;

public sealed partial class OptimizeWorkspaceView : UserControl, IOptimizeMutationInteraction
{
    private const double CategoryColumnWidth = 268;
    private const double CategoryColumnGap = 28;
    private const int CategoryChunkSize = 8;

    private static readonly string[] CategoryOrder =
    [
        "Privacy", "Search", "Interface", "AI & Suggestions", "Gaming", "Network",
        "Storage", "Repair", "Optional Apps", "Communication", "Media"
    ];

    private readonly ObservableCollection<OptimizeCategoryGroup> _groups = [];
    private readonly Dictionary<string, CheckBox> _checksById = new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<string, TextBlock> _countLabelsByCategory = new(StringComparer.OrdinalIgnoreCase);
    private IReadOnlyList<Tweak> _all = [];
    private bool _binding;
    private bool _layoutHooked;
    private bool _forceLayout;
    private int _lastColumnsPerRow = -1;

    public OptimizeWorkspaceView()
    {
        InitializeComponent();
    }

    public event EventHandler? RunRequested;
    public event EventHandler? OpenBackupsRequested;
    public IReadOnlyList<Tweak> Selected => _all.Where(x => x.IsSelected).ToList();

    public void Bind(IReadOnlyList<Tweak> tweaks)
    {
        _binding = true;
        _all = tweaks;
        foreach (var tweak in tweaks) tweak.PropertyChanged += (_, args) =>
        {
            if (args.PropertyName is nameof(Tweak.IsSelected) or nameof(Tweak.IsApplied) or null)
                UpdateSelection();
        };
        _binding = false;
        HookLayout();
        ApplyFilter();
    }

    public void RefreshRows() => ApplyFilter();

    public void SetBusy(bool busy, string status)
    {
        BusyRing.IsActive = busy;
        RunButton.IsEnabled = !busy && _all.Count(x => x.IsSelected) > 0;
        BackupsButton.IsEnabled = !busy;
        CategoryScroller.IsEnabled = !busy;
        StatusText.Text = status;
        if (!busy) UpdateSelection();
    }

    public void FocusSearch() => SearchBox.Focus(FocusState.Programmatic);

    public Task<bool> ConfirmReviewedBatchAsync(OptimizeMutationReview review, CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        return ConfirmMutationAsync(review);
    }

    public async Task<bool> ConfirmContinueWithoutRestorePointAsync(SystemRestorePointResult failure,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = "Continue without restore point?",
            Content = new TextBlock
            {
                Text = failure.Message + Environment.NewLine + Environment.NewLine +
                       "No restore point was created. Continue with the selected changes?",
                TextWrapping = TextWrapping.Wrap,
                MaxWidth = 620
            },
            PrimaryButtonText = "Continue",
            CloseButtonText = "Cancel all changes",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    public async Task<bool> ConfirmMutationAsync(OptimizeMutationReview review)
    {
        var dialog = new ContentDialog
        {
            XamlRoot = XamlRoot,
            Title = $"Apply {review.TweakPreflight.Previewed:N0} selected change(s)?",
            Content = BuildConfirmationContent(review),
            PrimaryButtonText = "Apply",
            CloseButtonText = "Cancel",
            DefaultButton = ContentDialogButton.Close
        };
        ConfirmationDialogStyle.Apply(dialog);
        return await dialog.ShowAsync() == ContentDialogResult.Primary;
    }

    private static StackPanel BuildConfirmationContent(OptimizeMutationReview review)
    {
        var panel = new StackPanel { Spacing = 10, MaxWidth = 620 };
        panel.Children.Add(new TextBlock
        {
            Text = $"{review.TweakPreflight.Previewed:N0} selected change(s) are ready. Close affected apps first. Repair actions can take a long time, and some changes may require sign-out.",
            TextWrapping = TextWrapping.Wrap
        });
        if (review.AdministratorActions.Count > 0)
        {
            panel.Children.Add(Warning(review.AdministratorActions.Count == 1
                ? $"Windows will ask for administrator permission to {review.AdministratorActions[0]}."
                : $"Windows will ask for administrator permission {review.AdministratorActions.Count:N0} times: " +
                  string.Join("; then ", review.AdministratorActions) + "."));
        }
        var details = new TextBox
        {
            Text = string.Join(Environment.NewLine, review.TweakPreflight.Log),
            IsReadOnly = true,
            AcceptsReturn = true,
            MinHeight = 120,
            MaxHeight = 210,
            TextWrapping = TextWrapping.Wrap,
            FontFamily = new FontFamily("Consolas")
        };
        ScrollViewer.SetVerticalScrollBarVisibility(details, ScrollBarVisibility.Auto);
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(details, "Selected optimization change details");
        panel.Children.Add(details);

        if (review.RestorePointPreflight is { } restore)
        {
            var restorePanel = new StackPanel { Spacing = 8 };
            restorePanel.Children.Add(new TextBlock
            {
                Text = "Windows restore point",
                FontWeight = Microsoft.UI.Text.FontWeights.SemiBold
            });
            var restoreDetails = new TextBox
            {
                Text = restore.Evidence,
                IsReadOnly = true,
                AcceptsReturn = true,
                MinHeight = 100,
                MaxHeight = 180,
                TextWrapping = TextWrapping.Wrap,
                FontFamily = new FontFamily("Consolas")
            };
            ScrollViewer.SetVerticalScrollBarVisibility(restoreDetails, ScrollBarVisibility.Auto);
            Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(restoreDetails,
                "Windows restore point details");
            restorePanel.Children.Add(new Border
            {
                Background = (Brush)Application.Current.Resources["SiftPanelBrush"],
                BorderBrush = (Brush)Application.Current.Resources["SiftLineBrush"],
                BorderThickness = new Thickness(1),
                CornerRadius = new CornerRadius(7),
                Padding = new Thickness(12, 10, 12, 10),
                Child = restoreDetails
            });
            panel.Children.Add(restorePanel);
        }

        return panel;
    }

    private static TextBlock Warning(string text) => new()
    {
        Text = text,
        Foreground = (Brush)Application.Current.Resources["SiftWarningBrush"],
        TextWrapping = TextWrapping.Wrap
    };

    private void HookLayout()
    {
        if (_layoutHooked || CategoryScroller is null) return;
        CategoryScroller.SizeChanged += (_, _) => RelayoutCategoryColumns();
        _layoutHooked = true;
    }

    private void ApplyFilter()
    {
        if (_binding || CategoryHost is null) return;
        var query = SearchBox.Text.Trim();
        var risk = (RiskBox.SelectedItem as ComboBoxItem)?.Content?.ToString() ?? "All risks";
        var rows = _all.Where(x =>
            (string.IsNullOrWhiteSpace(query) || x.Title.Contains(query, StringComparison.OrdinalIgnoreCase) ||
             x.Description.Contains(query, StringComparison.OrdinalIgnoreCase) ||
             x.Category.Contains(query, StringComparison.OrdinalIgnoreCase) ||
             x.Id.Contains(query, StringComparison.OrdinalIgnoreCase)) &&
            (risk == "All risks" ||
             risk == "Standard" && x.Risk == TweakRisk.Safe ||
             risk == "Moderate" && x.Risk == TweakRisk.Moderate ||
             risk == "Advanced" && x.Risk == TweakRisk.Advanced)).ToList();

        _groups.Clear();
        foreach (var category in OrderCategories(rows.Select(x => x.Category).Distinct()))
        {
            var group = new OptimizeCategoryGroup { Category = category };
            foreach (var tweak in rows.Where(x => x.Category == category)
                         .OrderBy(x => x.Risk)
                         .ThenBy(x => x.Title, StringComparer.CurrentCultureIgnoreCase))
                group.Tweaks.Add(tweak);
            group.RefreshCount();
            _groups.Add(group);
        }

        _forceLayout = true;
        RelayoutCategoryColumns();
        EmptyState.Visibility = rows.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
        CategoryScroller.Visibility = rows.Count == 0 ? Visibility.Collapsed : Visibility.Visible;
        UpdateSelection();
    }

    private void RelayoutCategoryColumns()
    {
        if (CategoryHost is null) return;
        if (_groups.Count == 0)
        {
            CategoryHost.Children.Clear();
            _checksById.Clear();
            _countLabelsByCategory.Clear();
            _lastColumnsPerRow = -1;
            return;
        }

        var available = CategoryScroller.ActualWidth;
        if (available < 80) available = 900;
        var columnsPerRow = Math.Max(1, (int)((available + CategoryColumnGap) / (CategoryColumnWidth + CategoryColumnGap)));
        if (!_forceLayout && columnsPerRow == _lastColumnsPerRow && CategoryHost.Children.Count > 0)
            return;

        _forceLayout = false;
        _lastColumnsPerRow = columnsPerRow;
        CategoryHost.Children.Clear();
        _checksById.Clear();
        _countLabelsByCategory.Clear();

        StackPanel? row = null;
        var inRow = 0;
        foreach (var group in _groups)
        {
            var chunks = ChunkTweaks(group.Tweaks, CategoryChunkSize);
            for (var chunkIndex = 0; chunkIndex < chunks.Count; chunkIndex++)
            {
                if (row is null || inRow >= columnsPerRow)
                {
                    row = new StackPanel
                    {
                        Orientation = Orientation.Horizontal,
                        Spacing = CategoryColumnGap,
                        Margin = new Thickness(0, 0, 0, 16)
                    };
                    CategoryHost.Children.Add(row);
                    inRow = 0;
                }

                row.Children.Add(BuildCategoryColumn(group, chunks[chunkIndex], chunkIndex == 0));
                inRow++;
            }
        }
    }

    private static List<IReadOnlyList<Tweak>> ChunkTweaks(IReadOnlyList<Tweak> tweaks, int size)
    {
        var chunks = new List<IReadOnlyList<Tweak>>();
        if (tweaks.Count == 0)
        {
            chunks.Add(tweaks);
            return chunks;
        }

        for (var index = 0; index < tweaks.Count; index += size)
            chunks.Add(tweaks.Skip(index).Take(size).ToList());
        return chunks;
    }

    private UIElement BuildCategoryColumn(OptimizeCategoryGroup group, IReadOnlyList<Tweak> tweaks, bool isPrimary)
    {
        var column = new StackPanel
        {
            Width = CategoryColumnWidth,
            Spacing = 4
        };

        if (isPrimary)
        {
            column.Children.Add(new TextBlock
            {
                Text = group.Header,
                Style = (Style)Application.Current.Resources["TypeSectionTitleStyle"],
                Margin = new Thickness(0, 0, 0, 2)
            });
            var count = new TextBlock
            {
                Text = group.CountLabel,
                Style = (Style)Application.Current.Resources["TypeMetaStyle"],
                Margin = new Thickness(0, 0, 0, 8)
            };
            _countLabelsByCategory[group.Category] = count;
            column.Children.Add(count);
        }
        else
        {
            column.Children.Add(new TextBlock
            {
                Text = $"{group.Header} · continued",
                Style = (Style)Application.Current.Resources["TypeMetaStyle"],
                Margin = new Thickness(0, 0, 0, 8)
            });
        }

        foreach (var tweak in tweaks)
            column.Children.Add(BuildPickerRow(tweak));
        return column;
    }

    private FrameworkElement BuildPickerRow(Tweak tweak)
    {
        var title = new TextBlock
        {
            Text = tweak.Title,
            Style = (Style)Application.Current.Resources["TypeBodyStyle"],
            TextTrimming = TextTrimming.CharacterEllipsis,
            MaxWidth = 210,
            VerticalAlignment = VerticalAlignment.Center,
            Padding = new Thickness(4, 0, 2, 0)
        };
        var check = new CheckBox
        {
            IsChecked = tweak.IsSelected,
            MinWidth = 0,
            MinHeight = 0,
            Padding = new Thickness(0, 0, 0, 0),
            VerticalAlignment = VerticalAlignment.Center,
            VerticalContentAlignment = VerticalAlignment.Center,
            Content = title
        };
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(check, $"Select {tweak.Title}");

        check.Checked += (_, _) =>
        {
            if (!tweak.IsSelected) tweak.IsSelected = true;
        };
        check.Unchecked += (_, _) =>
        {
            if (tweak.IsSelected) tweak.IsSelected = false;
        };
        _checksById[tweak.Id] = check;

        var row = new Border
        {
            MinHeight = 40,
            Padding = new Thickness(6, 8, 8, 8),
            CornerRadius = new CornerRadius(6),
            BorderThickness = new Thickness(1),
            BorderBrush = new SolidColorBrush(Microsoft.UI.Colors.Transparent),
            Background = new SolidColorBrush(Microsoft.UI.Colors.Transparent),
            Child = check,
            Tag = tweak
        };
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(row, tweak.Title);
        ToolTipService.SetToolTip(row, BuildDetailedTooltip(tweak));
        row.PointerPressed += PickerRow_PointerPressed;
        row.PointerEntered += (_, _) =>
        {
            row.Background = (Brush)Application.Current.Resources["SiftElevatedBrush"];
            row.BorderBrush = (Brush)Application.Current.Resources["SiftLineBrush"];
        };
        row.PointerExited += (_, _) =>
        {
            row.Background = new SolidColorBrush(Microsoft.UI.Colors.Transparent);
            row.BorderBrush = new SolidColorBrush(Microsoft.UI.Colors.Transparent);
        };
        return row;
    }

    private static UIElement BuildDetailedTooltip(Tweak tweak)
    {
        var panel = new StackPanel { Spacing = 8, MaxWidth = 360 };
        panel.Children.Add(new TextBlock
        {
            Text = tweak.Title,
            Style = (Style)Application.Current.Resources["TypeSectionTitleStyle"],
            TextWrapping = TextWrapping.Wrap
        });
        panel.Children.Add(new TextBlock
        {
            Text = tweak.Description,
            Style = (Style)Application.Current.Resources["TypeMetaStyle"],
            TextWrapping = TextWrapping.Wrap
        });

        panel.Children.Add(MetaLine($"{tweak.RiskLabel} · {KindLabel(tweak.Kind)} · {tweak.StateLabel}"));
        panel.Children.Add(MetaLine(tweak.Reversible
            ? "Undo: Sift can restore the prior value from its backup."
            : "Undo: Sift cannot automatically restore this change."));
        if (tweak.RequiresElevation ||
            (tweak.Kind == TweakKind.Registry &&
             tweak.Target.StartsWith("HKLM", StringComparison.OrdinalIgnoreCase)))
            panel.Children.Add(MetaLine("Windows will ask for administrator permission before this action starts."));

        panel.Children.Add(MetaLine($"Action: {ActionSummary(tweak)}"));
        return new Border
        {
            Background = (Brush)Application.Current.Resources["SiftElevatedBrush"],
            BorderBrush = (Brush)Application.Current.Resources["SiftLineBrush"],
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(8),
            Padding = new Thickness(12, 10, 12, 10),
            Child = panel
        };
    }

    private static TextBlock MetaLine(string text) => new()
    {
        Text = text,
        Style = (Style)Application.Current.Resources["TypeBodyStyle"],
        TextWrapping = TextWrapping.Wrap
    };

    private static string KindLabel(TweakKind kind) => kind switch
    {
        TweakKind.Registry => "Windows setting",
        TweakKind.Command => "Repair command",
        TweakKind.AppPackage => "Optional app removal",
        _ => kind.ToString()
    };

    private static string ActionSummary(Tweak tweak) => tweak.Kind switch
    {
        TweakKind.Registry when !string.IsNullOrWhiteSpace(tweak.ValueName) =>
            $"{tweak.Target}\\{tweak.ValueName} → {FormatDesired(tweak.DesiredValue)}",
        TweakKind.Registry => tweak.Target,
        TweakKind.Command => tweak.ApplyCommand ?? tweak.Target,
        TweakKind.AppPackage => $"Remove-AppxPackage family {tweak.Target}",
        _ => tweak.Target
    };

    private static string FormatDesired(object? value) => value switch
    {
        null => "(default)",
        int i => i.ToString(),
        _ => value.ToString() ?? ""
    };

    private static IEnumerable<string> OrderCategories(IEnumerable<string> categories)
    {
        var set = categories.ToHashSet(StringComparer.OrdinalIgnoreCase);
        foreach (var known in CategoryOrder)
            if (set.Remove(known)) yield return known;
        foreach (var rest in set.OrderBy(x => x, StringComparer.CurrentCultureIgnoreCase))
            yield return rest;
    }

    private void SelectPreset(Func<Tweak, bool> predicate)
    {
        foreach (var tweak in _all) tweak.IsSelected = predicate(tweak);
        UpdateSelection();
    }

    private void UpdateSelection()
    {
        if (SelectionText is null) return;
        var count = _all.Count(x => x.IsSelected);
        SelectionText.Text = $"{count:N0} selected";
        RunButton.Label = "Apply selected";
        RunButton.IsEnabled = count > 0 && !BusyRing.IsActive;
        foreach (var tweak in _all)
        {
            if (_checksById.TryGetValue(tweak.Id, out var check) && check.IsChecked != tweak.IsSelected)
                check.IsChecked = tweak.IsSelected;
        }
        foreach (var group in _groups)
        {
            group.RefreshCount();
            if (_countLabelsByCategory.TryGetValue(group.Category, out var label))
                label.Text = group.CountLabel;
        }
    }

    private void PickerRow_PointerPressed(object sender, PointerRoutedEventArgs e)
    {
        if (sender is not FrameworkElement { Tag: Tweak tweak }) return;
        if (e.OriginalSource is CheckBox) return;
        if (e.OriginalSource is DependencyObject node)
        {
            while (node is not null)
            {
                if (node is CheckBox) return;
                node = VisualTreeHelper.GetParent(node);
            }
        }
        tweak.IsSelected = !tweak.IsSelected;
    }

    private void SearchBox_TextChanged(object sender, TextChangedEventArgs e) => ApplyFilter();
    private void RiskBox_SelectionChanged(object sender, SelectionChangedEventArgs e) => ApplyFilter();
    private void MinimalButton_Click(object sender, RoutedEventArgs e) => SelectPreset(x => x.Minimal);
    private void BalancedButton_Click(object sender, RoutedEventArgs e) => SelectPreset(x => x.Recommended);
    private void ClearButton_Click(object sender, RoutedEventArgs e) => SelectPreset(_ => false);
    private void RunButton_Click(object sender, RoutedEventArgs e) => RunRequested?.Invoke(this, EventArgs.Empty);
    private void BackupsButton_Click(object sender, RoutedEventArgs e) => OpenBackupsRequested?.Invoke(this, EventArgs.Empty);
}
