using Sift.Models;

namespace Sift.Services;

public static class TweakCatalog
{
    /// <summary>Exact command tweak IDs that may cross the elevation boundary.</summary>
    public static IReadOnlySet<string> ElevatedCommandIds { get; } = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
    {
        "power.hibernate",
        "repair.dism-component-cleanup",
        "repair.sfc-scan"
    };

    public static List<Tweak> Create()
    {
        List<Tweak> tweaks =
        [
        Reg("privacy.ad-id", "Disable advertising ID", "Stops apps from using a per-user advertising identifier.", "Privacy", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\AdvertisingInfo", "Enabled", 0, true, true),
        Reg("privacy.tailored", "Turn off tailored experiences", "Prevents diagnostic data from personalizing tips and promotions.", "Privacy", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Privacy", "TailoredExperiencesWithDiagnosticDataEnabled", 0, true, true),
        Reg("privacy.feedback", "Reduce feedback prompts", "Stops Windows from periodically asking for feedback.", "Privacy", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Siuf\\Rules", "NumberOfSIUFInPeriod", 0, true, false),
        Reg("privacy.activity", "Disable activity publishing", "Prevents Windows from publishing this account's activity history.", "Privacy", TweakRisk.Safe, "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\System", "PublishUserActivities", 0, true, true),
        Reg("privacy.telemetry", "Required diagnostic data only", "Uses the supported Windows policy to select the lowest broadly available diagnostic level.", "Privacy", TweakRisk.Moderate, "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection", "AllowTelemetry", 1, true, false),
        Reg("privacy.consumer-features", "Disable consumer experiences", "Stops Windows from pushing consumer app suggestions through the supported cloud-content policy.", "Privacy", TweakRisk.Moderate, "HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\CloudContent", "DisableWindowsConsumerFeatures", 1, false, false),
        Reg("privacy.spotlight", "Disable lock-screen Spotlight ads", "Turns off Windows Spotlight consumer content on the lock screen for this user.", "Privacy", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", "RotatingLockScreenOverlayEnabled", 0, false, false),
        Reg("privacy.online-speech", "Disable online speech recognition", "Keeps speech features local by disabling the online speech recognition consent flag.", "Privacy", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Speech_OneCore\\Settings\\OnlineSpeechPrivacy", "HasAccepted", 0, false, false),

        Reg("search.web", "Disable web results in Search", "Keeps Start and taskbar search focused on local files, apps, and settings.", "Search", TweakRisk.Safe, "HKCU\\Software\\Policies\\Microsoft\\Windows\\Explorer", "DisableSearchBoxSuggestions", 1, true, true),
        Reg("search.highlights", "Disable search highlights", "Removes promotional artwork and trending content from Search.", "Search", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\SearchSettings", "IsDynamicSearchBoxEnabled", 0, true, false),
        Reg("search.device-history", "Disable device search history", "Stops Windows Search from saving future searches locally on this device.", "Search", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\SearchSettings", "IsDeviceSearchHistoryEnabled", 0, true, false),
        Reg("shell.widgets", "Hide taskbar Widgets", "Removes the Widgets button; it remains available to restore.", "Interface", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", "TaskbarDa", 0, true, true),
        Reg("shell.chat", "Hide taskbar Chat", "Removes the consumer Chat button from the taskbar.", "Interface", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", "TaskbarMn", 0, true, false),
        Reg("shell.task-view", "Hide the Task View button", "Removes Task View from the taskbar; Win+Tab and virtual desktops keep working.", "Interface", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", "ShowTaskViewButton", 0, false, false),
        Reg("shell.taskbar-left", "Align the taskbar left", "Moves Start and pinned taskbar icons from the center to the left.", "Interface", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", "TaskbarAl", 0, false, false),
        Reg("shell.end-task", "Enable taskbar End task", "Adds End task to app taskbar menus. Ending an app can discard its unsaved work.", "Interface", TweakRisk.Moderate, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\TaskbarDeveloperSettings", "TaskbarEndTask", 1, false, false),
        Reg("shell.extensions", "Show file extensions", "Makes executable and document types visible in File Explorer.", "Interface", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", "HideFileExt", 0, true, true),
        Reg("shell.hidden-files", "Show hidden files", "Shows hidden files and folders using File Explorer's standard option.", "Interface", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", "Hidden", 1, false, false),
        Reg("shell.explorer-this-pc", "Open File Explorer to This PC", "Opens File Explorer at drives and common folders instead of Home.", "Interface", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", "LaunchTo", 1, false, false),
        Reg("shell.drive-letters", "Show drive letters first", "Places drive letters before volume names so drives are easier to scan.", "Interface", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer", "ShowDriveLettersFirst", 4, false, false),
        Reg("shell.recent", "Hide recently opened items", "Stops Start, jump lists, and File Explorer from showing recent files.", "Interface", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", "Start_TrackDocs", 0, false, false),
        Reg("shell.copilot", "Disable Windows Copilot", "Turns off the Windows Copilot surface through its policy.", "AI & Suggestions", TweakRisk.Moderate, "HKCU\\Software\\Policies\\Microsoft\\Windows\\WindowsCopilot", "TurnOffWindowsCopilot", 1, true, false),
        Reg("shell.suggestions", "Disable suggested content", "Reduces app suggestions and promotional content in Windows surfaces.", "AI & Suggestions", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", "SubscribedContent-338388Enabled", 0, true, true),
        Reg("shell.tips", "Disable Windows tips", "Stops contextual Windows tips and suggested setup notifications.", "AI & Suggestions", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", "SoftLandingEnabled", 0, true, false),
        Reg("shell.lock-tips", "Disable lock-screen tips", "Turns off fun facts, tips, and promotional overlays on the lock screen.", "AI & Suggestions", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", "SubscribedContent-338387Enabled", 0, true, false),
        Reg("shell.phone-link-start", "Hide Phone Link in Start", "Hides the mobile-device panel in Start without uninstalling Phone Link.", "AI & Suggestions", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Start\\Companions\\Microsoft.YourPhone_8wekyb3d8bbwe", "IsEnabled", 0, false, false),
        Reg("shell.silent-installed", "Disable silent installed apps", "Stops Windows from silently installing suggested apps.", "AI & Suggestions", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", "SilentInstalledAppsEnabled", 0, false, false),
        Reg("shell.preinstalled", "Disable preinstalled app suggestions", "Reduces OEM and Store app suggestions on first sign-in surfaces.", "AI & Suggestions", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", "PreInstalledAppsEnabled", 0, false, false),
        Reg("shell.pane-suggestions", "Disable Start pane suggestions", "Turns off suggested content in the Start settings pane for this user.", "AI & Suggestions", TweakRisk.Safe, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", "SystemPaneSuggestionsEnabled", 0, false, false),
        Reg("gaming.dvr", "Disable background game recording", "Turns off Game DVR background capture while leaving screenshots intact.", "Gaming", TweakRisk.Safe, "HKCU\\System\\GameConfigStore", "GameDVR_Enabled", 0, true, false),
        Reg("network.delivery", "Keep update sharing local", "Prevents Windows Update uploads to internet peers while retaining local caching.", "Network", TweakRisk.Moderate, "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\DeliveryOptimization\\Config", "DODownloadMode", 0, false, false),

        Cmd("power.hibernate", "Disable hibernation", "Frees the hibernation file. This usually disables Fast Startup too.", "Storage", TweakRisk.Advanced, "powercfg.exe /hibernate off", "powercfg.exe /hibernate on", requiresElevation: true),

        Cmd("repair.dism-component-cleanup", "DISM component cleanup",
            "Runs the Windows DISM StartComponentCleanup job. This can take a long time, needs administrator confirmation, and is not automatically reversible.",
            "Repair", TweakRisk.Advanced,
            "DISM.exe /Online /Cleanup-Image /StartComponentCleanup", null, reversible: false, requiresElevation: true),
        Cmd("repair.sfc-scan", "System File Checker scan",
            "Runs sfc /scannow to verify protected system files. This can take a long time, needs administrator confirmation, and is not automatically reversible.",
            "Repair", TweakRisk.Advanced,
            "sfc.exe /scannow", null, reversible: false, requiresElevation: true),

        App("apps.clipchamp", "Remove Clipchamp", "Uninstalls the current user's Clipchamp package.", "Optional Apps", "Clipchamp.Clipchamp"),
        App("apps.solitaire", "Remove Solitaire Collection", "Uninstalls the current user's Solitaire Collection.", "Optional Apps", "Microsoft.MicrosoftSolitaireCollection"),
        App("apps.feedback", "Remove Feedback Hub", "Uninstalls the current user's Feedback Hub package.", "Optional Apps", "Microsoft.WindowsFeedbackHub"),
        App("apps.quickassist", "Remove Quick Assist", "Uninstalls Quick Assist. Keep it if you use remote support.", "Optional Apps", "MicrosoftCorporationII.QuickAssist"),
        App("apps.3d-viewer", "Remove 3D Viewer", "Uninstalls Microsoft's optional 3D model viewer.", "Optional Apps", "Microsoft.Microsoft3DViewer"),
        App("apps.mixed-reality", "Remove Mixed Reality Portal", "Uninstalls the discontinued Windows Mixed Reality portal.", "Optional Apps", "Microsoft.MixedReality.Portal"),
        App("apps.skype-uwp", "Remove legacy Skype", "Uninstalls the discontinued Microsoft Store version of Skype.", "Optional Apps", "Microsoft.SkypeApp"),
        App("apps.paint3d", "Remove Paint 3D", "Uninstalls the optional Paint 3D Store package.", "Optional Apps", "Microsoft.MSPaint"),
        App("apps.get-help", "Remove Get Help", "Uninstalls the Get Help Store package for this user.", "Optional Apps", "Microsoft.GetHelp"),
        App("apps.get-started", "Remove Get Started", "Uninstalls the Get Started / tips experience package.", "Optional Apps", "Microsoft.Getstarted"),
        App("apps.dev-home", "Remove Dev Home", "Uninstalls the optional Dev Home Store package.", "Optional Apps", "Microsoft.Windows.DevHome"),
        App("apps.power-automate", "Remove Power Automate", "Uninstalls Power Automate Desktop for this user.", "Optional Apps", "Microsoft.PowerAutomateDesktop"),
        App("apps.todos", "Remove Microsoft To Do", "Uninstalls the Microsoft To Do Store package.", "Optional Apps", "Microsoft.Todos"),
        App("apps.sticky-notes", "Remove Sticky Notes", "Uninstalls Microsoft Sticky Notes for this user.", "Optional Apps", "Microsoft.MicrosoftStickyNotes"),
        App("apps.onenote", "Remove OneNote for Windows 10", "Uninstalls the Store OneNote package (desktop Microsoft 365 OneNote is unchanged).", "Optional Apps", "Microsoft.Office.OneNote"),

        App("apps.weather", "Remove Weather", "Uninstalls the Bing Weather Store package.", "Media", "Microsoft.BingWeather"),
        App("apps.news", "Remove News", "Uninstalls the Bing News Store package.", "Media", "Microsoft.BingNews"),
        App("apps.maps", "Remove Maps", "Uninstalls Windows Maps for this user.", "Media", "Microsoft.WindowsMaps"),
        App("apps.groove", "Remove Groove Music", "Uninstalls the legacy Groove / Zune Music package.", "Media", "Microsoft.ZuneMusic"),
        App("apps.movies", "Remove Movies & TV", "Uninstalls the Movies & TV Store package.", "Media", "Microsoft.ZuneVideo"),
        App("apps.sound-recorder", "Remove Sound Recorder", "Uninstalls the Windows Sound Recorder package.", "Media", "Microsoft.WindowsSoundRecorder"),

        App("apps.people", "Remove People", "Uninstalls the People Store package.", "Communication", "Microsoft.People"),
        App("apps.your-phone", "Remove Phone Link", "Uninstalls Phone Link / Your Phone for this user. Irreversible without the Store.", "Communication", "Microsoft.YourPhone"),
        App("apps.teams", "Remove Microsoft Teams (Store)", "Uninstalls the consumer Teams Store package when present.", "Communication", "MicrosoftTeams"),
        App("apps.alarms", "Remove Alarms & Clock", "Uninstalls Windows Alarms & Clock for this user.", "Communication", "Microsoft.WindowsAlarms"),

        App("apps.xbox-app", "Remove Xbox Console Companion", "Uninstalls the legacy Xbox Console Companion package.", "Gaming", "Microsoft.XboxApp"),
        App("apps.gaming-app", "Remove Xbox app", "Uninstalls the modern Xbox / Gaming App package.", "Gaming", "Microsoft.GamingApp"),
        App("apps.xbox-overlay", "Remove Xbox Game Bar overlay", "Uninstalls Xbox Game Overlay helpers for this user.", "Gaming", "Microsoft.XboxGamingOverlay"),
        App("apps.xbox-tcui", "Remove Xbox TCUI", "Uninstalls Xbox TCUI for this user.", "Gaming", "Microsoft.Xbox.TCUI"),
        App("apps.xbox-identity", "Remove Xbox Identity Provider", "Uninstalls Xbox Identity Provider. Xbox sign-in surfaces may stop working.", "Gaming", "Microsoft.XboxIdentityProvider"),
        ];
        Validate(tweaks);
        return tweaks;
    }

    private static void Validate(IReadOnlyCollection<Tweak> tweaks)
    {
        var duplicate = tweaks.GroupBy(x => x.Id).FirstOrDefault(x => x.Count() > 1);
        if (duplicate is not null) throw new InvalidOperationException($"Duplicate tweak id: {duplicate.Key}");
        foreach (var tweak in tweaks)
        {
            if (tweak.Kind == TweakKind.Registry && (!tweak.Target.StartsWith("HKCU\\") && !tweak.Target.StartsWith("HKLM\\")))
                throw new InvalidOperationException($"Invalid registry target for {tweak.Id}.");
            if (tweak.Kind == TweakKind.Command && tweak.Reversible && string.IsNullOrWhiteSpace(tweak.UndoCommand))
                throw new InvalidOperationException($"Reversible command {tweak.Id} has no undo command.");
            if (tweak.Minimal && tweak.Risk == TweakRisk.Advanced)
                throw new InvalidOperationException($"Minimal preset cannot include Advanced tweak {tweak.Id}.");
            if (tweak.Recommended && tweak.Risk == TweakRisk.Advanced)
                throw new InvalidOperationException($"Balanced preset cannot include Advanced tweak {tweak.Id}.");
            if (tweak.RequiresElevation &&
                (tweak.Kind != TweakKind.Command || !ElevatedCommandIds.Contains(tweak.Id)))
                throw new InvalidOperationException($"Elevated command catalog mismatch for {tweak.Id}.");
            if (ElevatedCommandIds.Contains(tweak.Id) && !tweak.RequiresElevation)
                throw new InvalidOperationException($"Elevated command {tweak.Id} must set RequiresElevation.");
        }
    }

    private static Tweak Reg(string id, string title, string description, string category, TweakRisk risk, string target, string name, int value, bool recommended, bool minimal) => new()
    { Id=id, Title=title, Description=description, Category=category, Risk=risk, Kind=TweakKind.Registry, Target=target, ValueName=name, DesiredValue=value, Recommended=recommended, Minimal=minimal };

    private static Tweak Cmd(string id, string title, string description, string category, TweakRisk risk, string apply, string? undo, bool reversible=true, bool requiresElevation=false) => new()
    { Id=id, Title=title, Description=description, Category=category, Risk=risk, Kind=TweakKind.Command, Target=apply, ApplyCommand=apply, UndoCommand=undo, Reversible=reversible && undo is not null, RequiresElevation=requiresElevation };

    private static Tweak App(string id, string title, string description, string category, string package) => new()
    { Id=id, Title=title, Description=description, Category=category, Risk=TweakRisk.Advanced, Kind=TweakKind.AppPackage, Target=package, Reversible=false };
}
