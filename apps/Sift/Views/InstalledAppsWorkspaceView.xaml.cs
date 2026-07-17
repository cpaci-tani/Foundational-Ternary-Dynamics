using System.Collections.ObjectModel;
using Sift.Models;
using Sift.Infrastructure.Icons;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Automation;
using Microsoft.UI.Xaml.Controls;

namespace Sift.WinUI.Views;

public sealed partial class InstalledAppsWorkspaceView : UserControl
{
    private readonly ObservableCollection<InstalledApp> _visible = [];
    private IReadOnlyList<InstalledApp> _all = [];
    private InstalledApp? _recentUninstall;
    private string _recentUninstallStatus = string.Empty;
    private bool _recentUninstallAuthorized;
    private bool _binding;

    public InstalledAppsWorkspaceView()
    {
        InitializeComponent();
        _binding = true;
        AppsTable.ItemsSource = _visible;
        FileLeftoverList.ItemsSource = _fileLeftovers;
        PolicyFilter.SelectedIndex = 0;
        _binding = false;
        ApplyFilter();
    }

    public event EventHandler? RefreshRequested;
    public event EventHandler? OpenSettingsRequested;
    public event EventHandler? ActionRequested;
    public event EventHandler? ScanFileLeftoversRequested;
    public event EventHandler? DeleteFileLeftoversRequested;
    public event EventHandler? SelectedAppChanged;

    public InstalledApp? SelectedApp => AppsTable.SelectedItem as InstalledApp;
    public InstalledApp? LeftoverTarget => SelectedApp?.IsOrphanedRegistration == true
        ? SelectedApp
        : CanUseRecentUninstall(SelectedApp) ? _recentUninstall : null;

    public void Bind(IReadOnlyList<InstalledApp> apps, string status)
    {
        _binding = true;
        _all = apps;
        RegisteredText.Text = apps.Count.ToString("N0");
        EligibleText.Text = apps.Count(app => app.CanUninstall).ToString("N0");
        LeftoverText.Text = apps.Count(app => app.IsOrphanedRegistration).ToString("N0");
        SizeText.Text = FormatBytes(apps.Sum(app => app.EstimatedSizeBytes));
        StatusText.Text = status;
        _binding = false;
        ApplyFilter();
    }

    public void SetBusy(bool busy, string status)
    {
        BusyRing.IsActive = busy;
        RefreshButton.IsEnabled = !busy;
        AppsTable.IsEnabled = !busy;
        FileLeftoverList.IsEnabled = !busy;
        StatusText.Text = status;
        if (busy && _all.Count == 0) ShowState("Loading installed apps", "Reading the three standard uninstall registry locations…", true);
        UpdateSelection();
        UpdateFileLeftoverSelection();
    }

    public void SetStatus(string status) => StatusText.Text = status;
    public void FocusSearch() => SearchBox.Focus(FocusState.Programmatic);

    public void SetRecentUninstall(InstalledApp? app, string status = "", bool cleanupAuthorized = false)
    {
        _recentUninstall = app;
        _recentUninstallStatus = status;
        _recentUninstallAuthorized = cleanupAuthorized;
        UpdateSelection();
    }

    public void SetTrustLoading(InstalledApp app)
    {
        if (!IsSelected(app)) return;
        TrustPanel.Visibility = Visibility.Visible;
        TrustSummaryText.Text = "Inspecting the registered uninstaller…";
        AutomationProperties.SetHelpText(TrustSummaryText, "Inspection in progress");
        TrustSummaryText.Foreground = (Microsoft.UI.Xaml.Media.Brush)Application.Current.Resources["SiftMutedBrush"];
        TrustDetailText.Text = "Checking the registered uninstaller's signature, publisher, version, and SHA-256 hash.";
        TrustMetadataText.Text = string.Empty;
        ToolTipService.SetToolTip(TrustMetadataText, null);
    }

    public void SetTrustReport(InstalledApp app, InstalledAppTrustReport report)
    {
        if (!IsSelected(app)) return;
        TrustPanel.Visibility = Visibility.Visible;
        TrustSummaryText.Text = report.Summary;
        AutomationProperties.SetHelpText(TrustSummaryText, report.Summary);
        var brushKey = report.Status switch
        {
            InstalledAppSignatureStatus.Trusted or InstalledAppSignatureStatus.WindowsInstaller => "SiftSuccessBrush",
            InstalledAppSignatureStatus.Unsigned or InstalledAppSignatureStatus.SignedUntrusted => "SiftAccentBrush",
            _ => "SiftMutedBrush"
        };
        TrustSummaryText.Foreground = (Microsoft.UI.Xaml.Media.Brush)Application.Current.Resources[brushKey];
        TrustDetailText.Text = report.Detail;
        var metadata = new List<string>();
        if (!string.IsNullOrWhiteSpace(report.Signer)) metadata.Add($"Signer: {report.Signer}");
        metadata.Add(report.PublisherMatchDisplay);
        if (!string.IsNullOrWhiteSpace(report.FileVersion)) metadata.Add($"Version: {report.FileVersion}");
        if (!string.IsNullOrWhiteSpace(report.CertificateValidity)) metadata.Add($"Certificate: {report.CertificateValidity}");
        TrustMetadataText.Text = string.Join(" · ", metadata);
        ToolTipService.SetToolTip(TrustMetadataText, string.Join(Environment.NewLine, new[]
        {
            report.ExecutablePath,
            string.IsNullOrWhiteSpace(report.CertificateThumbprint) ? string.Empty : $"Certificate thumbprint: {report.CertificateThumbprint}",
            string.IsNullOrWhiteSpace(report.Sha256) ? string.Empty : $"SHA-256: {report.Sha256}"
        }.Where(value => !string.IsNullOrWhiteSpace(value))));
    }

    private void ApplyFilter()
    {
        if (_binding) return;
        var query = SearchBox.Text.Trim();
        var policy = (PolicyFilter.SelectedItem as ComboBoxItem)?.Content?.ToString() ?? "All apps";
        var rows = _all.Where(app =>
            (string.IsNullOrWhiteSpace(query) || app.DisplayName.Contains(query, StringComparison.OrdinalIgnoreCase) ||
             app.Publisher.Contains(query, StringComparison.OrdinalIgnoreCase) ||
             app.DisplayVersion.Contains(query, StringComparison.OrdinalIgnoreCase) ||
             app.Source.Contains(query, StringComparison.OrdinalIgnoreCase)) &&
            (policy == "All apps" || policy == "Can uninstall" && app.CanUninstall ||
             policy == "Leftover registrations" && app.IsOrphanedRegistration ||
             policy == "Windows Settings only" && !app.CanUninstall && !app.IsOrphanedRegistration))
            .OrderByDescending(app => app.CanUninstall)
            .ThenByDescending(app => app.IsOrphanedRegistration)
            .ThenBy(app => app.DisplayName, StringComparer.CurrentCultureIgnoreCase)
            .ToList();

        _visible.Clear();
        foreach (var row in rows) _visible.Add(row);
        CountText.Text = $"{rows.Count:N0} shown";
        if (rows.Count == 0)
            ShowState(_all.Count == 0 ? "No registered desktop apps" : "No matching apps",
                _all.Count == 0 ? "Use Windows Installed Apps for Store and MSIX packages." : "Try a broader filter or another policy view.", false);
        else EmptyState.Visibility = Visibility.Collapsed;
        UpdateSelection();
    }

    private void UpdateSelection()
    {
        if (SelectedNameText is null) return;
        var app = SelectedApp;
        var canUseRecent = CanUseRecentUninstall(app);
        if (app is null)
        {
            TrustPanel.Visibility = Visibility.Collapsed;
            SelectedNameText.Text = _recentUninstall is null
                ? "Select an app to view its uninstall information."
                : $"Recent uninstall: {_recentUninstall.DisplayName}";
            SelectedDetailText.Text = _recentUninstall is null
                ? "Registered uninstall details will appear here."
                : _recentUninstallStatus;
            SelectedPathText.Text = string.Empty;
            ActionButton.IsEnabled = false;
            ActionNoteText.Text = _recentUninstall is null
                ? "Select one app"
                : _recentUninstallAuthorized ? "Removal verified · AppData folders available" : "Cleanup becomes available after removal is verified";
        }
        else
        {
            TrustPanel.Visibility = app.IsOrphanedRegistration ? Visibility.Collapsed : Visibility.Visible;
            SelectedNameText.Text = $"{app.DisplayName} · {app.PublisherDisplay} · {app.VersionDisplay}";
            SelectedDetailText.Text = canUseRecent && !string.IsNullOrWhiteSpace(_recentUninstallStatus)
                ? _recentUninstallStatus
                : app.IsOrphanedRegistration ? app.OrphanEvidence : app.PolicyReason;
            SelectedPathText.Text = app.IsOrphanedRegistration
                ? $"{app.RegistryLocation.Hive} · {app.RegistryLocation.View} · {app.RegistryLocation.SubKeyName}"
                : string.IsNullOrWhiteSpace(app.InstallLocation)
                    ? $"{app.RegistryLocation.Hive} · {app.RegistryLocation.View} · {app.Source}"
                    : app.InstallLocation;
            ActionButton.IsEnabled = !canUseRecent &&
                (app.IsOrphanedRegistration ? app.CanCleanRegistration : app.CanUninstall) && !BusyRing.IsActive;
            ActionNoteText.Text = canUseRecent
                ? _recentUninstallAuthorized ? "Removal verified · AppData folders available" : "Waiting for uninstall completion"
                : app.IsOrphanedRegistration
                ? "Registration only · backup created first · app files unchanged"
                : "Opens the registered uninstaller";
        }
        var isOrphan = app?.IsOrphanedRegistration == true;
        ActionButton.Label = isOrphan ? "Remove registration" : "Open uninstaller";
        ActionButton.Icon = isOrphan ? SiftIconKind.Remove : SiftIconKind.OpenExternal;
        var leftoverTarget = LeftoverTarget;
        ScanFileLeftoversButton.IsEnabled = leftoverTarget is not null && !BusyRing.IsActive;
        var scanLabel = app?.IsOrphanedRegistration == true
            ? "Scan file leftovers"
            : canUseRecent && !_recentUninstallAuthorized ? "Check uninstall status"
            : canUseRecent ? "Scan file leftovers" : "Scan file leftovers";
        ScanFileLeftoversButton.Label = scanLabel;
        ScanFileLeftoversButton.Icon = SiftIconKind.Scan;
    }

    private void ShowState(string title, string detail, bool progress)
    {
        StateTitleText.Text = title;
        StateDetailText.Text = detail;
        StateProgressRing.IsActive = progress;
        StateProgressRing.Visibility = progress ? Visibility.Visible : Visibility.Collapsed;
        EmptyState.Visibility = Visibility.Visible;
    }

    private static string FormatBytes(long bytes) => Sift.Presentation.SiftDisplay.BytesOrDash(bytes);

    private void SearchBox_TextChanged(object sender, TextChangedEventArgs e) => ApplyFilter();
    private void PolicyFilter_SelectionChanged(object sender, SelectionChangedEventArgs e) => ApplyFilter();
    private void AppsTable_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        UpdateSelection();
        if (!_binding) SelectedAppChanged?.Invoke(this, EventArgs.Empty);
    }
    private void RefreshButton_Click(object sender, RoutedEventArgs e) => RefreshRequested?.Invoke(this, EventArgs.Empty);
    private void OpenSettingsButton_Click(object sender, RoutedEventArgs e) => OpenSettingsRequested?.Invoke(this, EventArgs.Empty);
    private void ReviewLeftoversButton_Click(object sender, RoutedEventArgs e)
    {
        PolicyFilter.SelectedIndex = 2;
        AppsTable.Focus(FocusState.Programmatic);
    }
    private void ActionButton_Click(object sender, RoutedEventArgs e) => ActionRequested?.Invoke(this, EventArgs.Empty);
    private void ScanFileLeftoversButton_Click(object sender, RoutedEventArgs e) => ScanFileLeftoversRequested?.Invoke(this, EventArgs.Empty);
    private bool CanUseRecentUninstall(InstalledApp? selected) =>
        _recentUninstall is not null && (selected is null ||
        string.Equals(selected.RegistryLocation.Identity, _recentUninstall.RegistryLocation.Identity,
            StringComparison.OrdinalIgnoreCase));

    private bool IsSelected(InstalledApp app) => SelectedApp is { } selected &&
        string.Equals(selected.RegistryLocation.Identity, app.RegistryLocation.Identity,
            StringComparison.OrdinalIgnoreCase) &&
        string.Equals(selected.DisplayName, app.DisplayName, StringComparison.Ordinal);
}
