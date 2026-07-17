using System.Collections.ObjectModel;
using Microsoft.UI.Xaml;
using Sift.Models;

namespace Sift.WinUI.Views;

public sealed partial class InstalledAppsWorkspaceView
{
    private readonly ObservableCollection<AppLeftoverCandidate> _fileLeftovers = [];
    private InstalledApp? _leftoverTarget;

    public InstalledApp? DisplayedLeftoverTarget => _leftoverTarget;
    public IReadOnlyList<AppLeftoverCandidate> SelectedFileLeftovers =>
        _fileLeftovers.Where(item => item.IsSelected).ToList();

    public void ShowFileLeftovers(InstalledApp app, IReadOnlyList<AppLeftoverCandidate> candidates, string status)
    {
        ReleaseCandidateSubscriptions();
        _fileLeftovers.Clear();
        _leftoverTarget = app;
        foreach (var candidate in candidates)
        {
            candidate.IsSelected = false;
            candidate.PropertyChanged += FileLeftover_PropertyChanged;
            _fileLeftovers.Add(candidate);
        }
        FileLeftoverTitleText.Text = $"Leftovers for {app.DisplayName}";
        FileLeftoverStatusText.Text = status;
        SelectedAppPanel.Visibility = Visibility.Collapsed;
        FileLeftoverPanel.Visibility = Visibility.Visible;
        UpdateFileLeftoverSelection();
    }

    public void SetFileLeftoverStatus(string status)
    {
        FileLeftoverStatusText.Text = status;
        UpdateFileLeftoverSelection();
    }

    public void ReleaseSubscriptions()
    {
        ReleaseCandidateSubscriptions();
        _fileLeftovers.Clear();
        _leftoverTarget = null;
    }

    private void UpdateFileLeftoverSelection()
    {
        if (FileLeftoverSelectionText is null) return;
        var selected = SelectedFileLeftovers;
        FileLeftoverSelectionText.Text = selected.Count == 0
            ? "Nothing selected by default · Recycle Bin only"
            : $"{selected.Count:N0} selected · {FormatBytes(selected.Sum(item => item.SizeBytes))} · Recycle Bin";
        DeleteFileLeftoversButton.Label = "Move to Recycle Bin";
        DeleteFileLeftoversButton.IsEnabled = selected.Count > 0 && !BusyRing.IsActive;
    }

    private void ReleaseCandidateSubscriptions()
    {
        foreach (var candidate in _fileLeftovers)
            candidate.PropertyChanged -= FileLeftover_PropertyChanged;
    }

    private void DeleteFileLeftoversButton_Click(object sender, RoutedEventArgs e) =>
        DeleteFileLeftoversRequested?.Invoke(this, EventArgs.Empty);

    private void CloseFileLeftoversButton_Click(object sender, RoutedEventArgs e)
    {
        FileLeftoverPanel.Visibility = Visibility.Collapsed;
        SelectedAppPanel.Visibility = Visibility.Visible;
        UpdateSelection();
    }

    private void FileLeftover_PropertyChanged(object? sender,
        System.ComponentModel.PropertyChangedEventArgs e) => UpdateFileLeftoverSelection();
}
