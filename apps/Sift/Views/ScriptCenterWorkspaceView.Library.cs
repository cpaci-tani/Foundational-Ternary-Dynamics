using System.Collections.ObjectModel;
using System.ComponentModel;
using Sift.Models;
using Sift.Services;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Automation;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;

namespace Sift.WinUI.Views;

public sealed partial class ScriptCenterWorkspaceView
{
    private readonly ObservableCollection<RecipeAccessSection> _sections = [];
    private readonly Dictionary<string, bool> _categoryExpansion = new(StringComparer.Ordinal);
    private IReadOnlyList<ScriptRecipe> _all = [];
    private bool _isElevated;
    private bool _clearingRecipeSelection;
    private ScriptRecipe? _selectedRecipe;
    private ListView? _selectedRecipeList;

    public void Bind(IReadOnlyList<ScriptRecipe> recipes, bool isElevated)
    {
        _all = recipes;
        _isElevated = isElevated;
        StudioTab.Visibility = isElevated ? Visibility.Collapsed : Visibility.Visible;
        CategoryBox.Items.Clear();
        CategoryBox.Items.Add("All categories");
        foreach (var item in ScriptRecipeTaxonomy.Categories.Where(category => recipes.Any(x => x.Category == category.Name)))
            CategoryBox.Items.Add(item.Name);
        CategoryBox.SelectedIndex = 0;
        ApplySubtitle();
        ApplyFilter();
    }

    private void ApplyFilter()
    {
        if (SearchBox is null) return;
        var query = SearchBox.Text.Trim();
        var category = CategoryBox.SelectedItem?.ToString();
        var rows = ScriptRecipeAccessPolicy.VisibleForSession(_all, _isElevated).Where(recipe =>
            (query.Length == 0 || $"{recipe.Title} {recipe.Description} {recipe.Command} {recipe.Category}"
                .Contains(query, StringComparison.OrdinalIgnoreCase)) &&
            (category is null or "All categories" || recipe.Category == category)).ToList();

        ClearRecipeSelection();
        _sections.Clear();
        var revealMatches = query.Length > 0 || category is not null and not "All categories";
        AddAccessSection(rows.Where(x => !x.RequiresAdministrator), requiresAdministrator: false, revealMatches);
        AddAccessSection(rows.Where(x => x.RequiresAdministrator), requiresAdministrator: true, revealMatches);
        EmptyState.Visibility = rows.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
        CategoryScroller.Visibility = rows.Count == 0 ? Visibility.Collapsed : Visibility.Visible;
        ExpandAllButton.IsEnabled = rows.Count > 0;
        CollapseAllButton.IsEnabled = rows.Count > 0;
        CountText.Text =
            $"{rows.Count:N0} recipes · {rows.Count(x => !x.RequiresAdministrator):N0} standard · {rows.Count(x => x.RequiresAdministrator):N0} administrator";
    }

    private void AddAccessSection(IEnumerable<ScriptRecipe> source, bool requiresAdministrator, bool revealMatches)
    {
        var recipes = source.ToList();
        var groups = new ObservableCollection<RecipeGroup>();
        foreach (var taxonomy in ScriptRecipeTaxonomy.Categories)
        {
            var categoryRecipes = recipes.Where(recipe => recipe.Category == taxonomy.Name).ToList();
            if (categoryRecipes.Count == 0) continue;
            var detail = requiresAdministrator
                ? _isElevated
                    ? $"{taxonomy.Description}. Runs with the current administrator access."
                    : $"{taxonomy.Description}. Windows will ask for administrator permission before this recipe runs."
                : _isElevated
                    ? $"{taxonomy.Description}. Runs with the current administrator access."
                    : taxonomy.Description + ".";
            var expansionKey = $"{requiresAdministrator}:{taxonomy.Name}";
            var isExpanded = revealMatches || (_categoryExpansion.TryGetValue(expansionKey, out var saved)
                ? saved
                : false);
            var group = new RecipeGroup(taxonomy.Name, detail, categoryRecipes, isExpanded);
            group.PropertyChanged += (_, args) =>
            {
                if (args.PropertyName != nameof(RecipeGroup.IsExpanded)) return;
                _categoryExpansion[expansionKey] = group.IsExpanded;
                if (!group.IsExpanded && _selectedRecipe is not null && group.Recipes.Contains(_selectedRecipe))
                    ClearRecipeSelection();
            };
            groups.Add(group);
        }
        if (groups.Count > 0)
            _sections.Add(new RecipeAccessSection(
                requiresAdministrator ? "ADMINISTRATOR COMMANDS" : "STANDARD USER COMMANDS",
                groups));
    }

    private void Filter_Changed(object sender, object e) => ApplyFilter();

    private void RecipeList_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_clearingRecipeSelection || sender is not ListView list) return;
        if (list.SelectedItem is not ScriptRecipe selected)
        {
            if (ReferenceEquals(list, _selectedRecipeList)) ClearRecipeSelection();
            return;
        }
        ApplyRecipeSelection(list, selected);
    }

    private void ApplyRecipeSelection(ListView list, ScriptRecipe selected)
    {
        if (_selectedRecipeList is not null && !ReferenceEquals(_selectedRecipeList, list))
        {
            _clearingRecipeSelection = true;
            _selectedRecipeList.SelectedItem = null;
            _clearingRecipeSelection = false;
        }
        _selectedRecipeList = list;
        _selectedRecipe = selected;
        RunButton.IsEnabled = Selected is not null && !BusyRing.IsActive;
        RunButton.Label = "Run";
        AutomationProperties.SetName(RunButton, "Run selected command");
        TerminalTitle.Text = $"{selected.ShellLabel.ToUpperInvariant()} · {selected.Title.ToUpperInvariant()}";
        TerminalCommandInput.Text = selected.Command;
        TerminalStatus.Text = "Ready";
        StatusText.Text = $"{selected.AccessLabel} · {selected.RiskLabel} · {selected.Command}";
    }

    private void RecipeCommandSurface_PointerEntered(object sender, PointerRoutedEventArgs e) =>
        SetRecipeHoverActions(sender, visible: true);

    private void RecipeCommandSurface_PointerExited(object sender, PointerRoutedEventArgs e) =>
        SetRecipeHoverActions(sender, visible: false);

    private static void SetRecipeHoverActions(object sender, bool visible)
    {
        if (sender is not FrameworkElement surface || surface.FindName("RecipeHoverActions") is not StackPanel actions) return;
        actions.Opacity = visible ? 1 : 0;
        actions.IsHitTestVisible = visible;
    }

    private void CopyRecipeButton_Click(object sender, RoutedEventArgs e)
    {
        if (sender is not Button { DataContext: ScriptRecipe recipe } button) return;
        try
        {
            _clipboard.CopyText(recipe.Command);
            TerminalStatus.Text = $"Copied · {recipe.Title}";
            StatusText.Text = $"Copied command · {recipe.Command}";
            AutomationProperties.SetItemStatus(button, $"Copied {recipe.Title} command.");
        }
        catch (Exception exception)
        {
            TerminalStatus.Text = "COPY FAILED · clipboard is unavailable";
            StatusText.Text = $"Could not copy the command: {exception.Message}";
        }
    }

    private void QuickRunRecipeButton_Click(object sender, RoutedEventArgs e)
    {
        if (BusyRing.IsActive || sender is not Button { DataContext: ScriptRecipe recipe } button) return;
        if (FindVisualAncestor<ListView>(button) is not { } list) return;
        list.SelectedItem = recipe;
        ApplyRecipeSelection(list, recipe);
        RunRequested?.Invoke(this, EventArgs.Empty);
    }

    private static T? FindVisualAncestor<T>(DependencyObject start) where T : DependencyObject
    {
        for (var current = VisualTreeHelper.GetParent(start); current is not null; current = VisualTreeHelper.GetParent(current))
        {
            if (current is T match) return match;
        }
        return null;
    }

    private void ClearRecipeSelection()
    {
        if (_selectedRecipeList is not null)
        {
            _clearingRecipeSelection = true;
            _selectedRecipeList.SelectedItem = null;
            _clearingRecipeSelection = false;
        }
        _selectedRecipeList = null;
        _selectedRecipe = null;
        if (RunButton is not null)
        {
            RunButton.IsEnabled = false;
            RunButton.Label = "Run";
            AutomationProperties.SetName(RunButton, "Run selected command");
        }
        if (TerminalTitle is not null) TerminalTitle.Text = "SIFT TERMINAL";
        if (TerminalCommandInput is not null) TerminalCommandInput.Text = string.Empty;
        if (TerminalStatus is not null) TerminalStatus.Text = "Ready";
        if (StatusText is not null) StatusText.Text = "Select a command.";
    }

    private void SetAllCategoriesExpanded(bool expanded)
    {
        foreach (var group in _sections.SelectMany(section => section.Groups))
            group.IsExpanded = expanded;
    }

    private void ExpandAllButton_Click(object sender, RoutedEventArgs e) => SetAllCategoriesExpanded(true);
    private void CollapseAllButton_Click(object sender, RoutedEventArgs e) => SetAllCategoriesExpanded(false);

    public sealed class RecipeAccessSection
    {
        public RecipeAccessSection(string accessLabel, ObservableCollection<RecipeGroup> groups)
        {
            AccessLabel = accessLabel;
            Groups = groups;
        }

        public string AccessLabel { get; }
        public ObservableCollection<RecipeGroup> Groups { get; }
        public string CountLabel
        {
            get
            {
                var count = Groups.Sum(group => group.Recipes.Count);
                return $"{count:N0} command{(count == 1 ? "" : "s")}";
            }
        }
    }

    public sealed class RecipeGroup : INotifyPropertyChanged
    {
        private bool _isExpanded;

        public RecipeGroup(string category, string detail, IEnumerable<ScriptRecipe> recipes, bool isExpanded)
        {
            Category = category;
            Detail = detail;
            Recipes = new ObservableCollection<ScriptRecipe>(recipes);
            _isExpanded = isExpanded;
        }

        public event PropertyChangedEventHandler? PropertyChanged;
        public string Category { get; }
        public string Detail { get; }
        public ObservableCollection<ScriptRecipe> Recipes { get; }
        public string CountLabel => $"{Recipes.Count:N0} command{(Recipes.Count == 1 ? "" : "s")}";
        public string AutomationName => $"{Category} category";
        public string RecipeListAutomationName => $"{Category} commands";
        public bool IsExpanded
        {
            get => _isExpanded;
            set
            {
                if (_isExpanded == value) return;
                _isExpanded = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsExpanded)));
            }
        }
    }
}
