# Sift debloat-source audit

Reviewed 2026-07-12. Sift does not download or execute any third-party script. The projects below were treated as leads; accepted changes were independently checked against Windows documentation and reimplemented as individual, previewable catalog entries with local backup metadata.

## Sources reviewed

- [Raphire/Win11Debloat](https://github.com/Raphire/Win11Debloat), including its current `Regfiles` and `Config/Apps.json` catalogs.
- [ChrisTitusTech/winutil](https://github.com/ChrisTitusTech/winutil), including `config/tweaks.json` and its Standard/Minimal preset design.
- [farag2/Sophia-Script-for-Windows](https://github.com/farag2/Sophia-Script-for-Windows), including its granular function model and documented side-effect warnings.
- [memstechtips/Winhance](https://github.com/memstechtips/Winhance), particularly its version filtering, restore-first workflow, and Settings-like scoping. No Winhance code was copied.
- [undergroundwires/privacy.sexy](https://github.com/undergroundwires/privacy.sexy), reviewed as a source of privacy leads; Defender, hosts-file, firewall, update, and deep service changes were excluded.
- [W4RH4WK/Debloat-Windows-10](https://github.com/W4RH4WK/Debloat-Windows-10) and [LeDragoX/Win-Debloat-Tools](https://github.com/LeDragoX/Win-Debloat-Tools) were reviewed but yielded no accepted actions: the former is archived, Windows-10-specific, and explicitly has no undo; the latter includes broad removals and third-party execution outside Sift's safety boundary.
- [Microsoft: Customize the taskbar](https://support.microsoft.com/en-us/windows/experience/personalization/customize-the-taskbar-in-windows).
- [Microsoft: File Explorer in Windows](https://support.microsoft.com/en-au/windows/show-libraries-in-file-explorer-8b443cb0-e966-55d9-e0d3-f814fe2db95b).
- [Microsoft: Windows Search and privacy](https://support.microsoft.com/en-us/windows/windows-search-and-privacy-99fb8251-7260-1cd6-1bbb-15c2370eb168).
- [Microsoft: Customize the lock screen](https://support.microsoft.com/en-us/windows/experience/personalization/customize-the-lock-screen-in-windows).
- [Microsoft update notes documenting taskbar End task](https://support.microsoft.com/en-au/topic/july-25-2024-kb5040527-os-builds-22621-3958-and-22631-3958-preview-de3e1e24-0c07-4210-9777-8e03a1446bae).

## Accepted in this pass

- Disable device-local Search history.
- Hide Task View while preserving `Win+Tab` and virtual desktops.
- Left-align the taskbar.
- Enable the supported taskbar **End task** option, with an unsaved-work warning.
- Show hidden files.
- Open File Explorer to **This PC**.
- Show drive letters before volume labels.
- Disable lock-screen tips.
- Hide the Phone Link panel in Start without removing Phone Link.
- Add optional removals for 3D Viewer, Mixed Reality Portal, and legacy Skype.
- Disable silent installed apps (`SilentInstalledAppsEnabled`) and preinstalled app suggestions (`PreInstalledAppsEnabled`) via ContentDeliveryManager — ordinary suggestion toggles, not included in Minimal/Balanced until further soak.
- Optional Windows System Restore point before irreversible or HKLM batches (user opt-in via WMI `SystemRestore.CreateRestorePoint`; Advanced safeguard, not automatic).
- Read-only Startup inventory (Run/RunOnce including WOW6432Node, user + Common Startup folders, StartupApproved Enabled/Disabled) — display only; no disable/remove actions in v0.7. Deep-link to `ms-settings:startupapps` allowed.
- **Maintenance workspace (v0.7):** scoped local scans — user temp, LocalAppData temp, Recycle Bin, thumbnail cache (`thumbcache_*.db`), WER ReportQueue, user CrashDumps (`*.dmp`), Delivery Optimization cache (admin), Prefetch (admin; clean is Advanced + confirm), AppData leftovers with stronger token match + High/Medium confidence (Medium unselected by default), and **orphan uninstall registry entries** whose InstallLocation folder is missing (exclude `SystemComponent=1` and Microsoft/Windows publisher families; JSON backup before key delete). Scan-first, user-selected cleanup, preview mode, no automatic sweeps.
- **Storage workspace (v0.8):** interactive disk usage map (parallel FindFirstFileEx walk, squarified treemap, folder drill-down). Guarded delete of user-selected paths only — Recycle Bin by default, permanent behind toggle + confirm. Blocks drive roots, `%SystemRoot%`, Sift itself, and reparse points. Not a silent mass cleaner.
- **Native Performance charts (v0.11):** LiveCharts2 rolling CPU/memory history with ranked consumers; the workspace module owns off-thread sampling and stops its timer when inactive.
- **Health workspace (v0.10):** read-only Checks (disk space, reboot pending, memory, manageable stopped Auto services, Update service status, recent System errors, WMI disk health) with plain-language recommendations and deep links only — no auto-fix. History tab merges Optimize backups, orphan-uninstall registry snapshots, and persisted `activity.json` timeline; Optimize restore from History requires two confirmations.
- **Services / scheduled tasks (v0.9):** runtime Start/Stop/Restart for non-critical services with a hard protected denylist (Defender, Update, firewall, RPC, EventLog, etc.). Scheduled-task enable/disable only for a small documented OEM/updater allowlist; Defender/Update tasks never allowlisted; other tasks open in Task Scheduler only. **v0.10.0:** grouped-grid selection stability and background `schtasks` reload — read-only for non-allowlisted tasks unchanged.
- Retired Optimize duplicate `maintenance.temp` — temp cleanup lives only in Maintenance.

All accepted registry changes are per-setting and automatically reversible through Sift's existing backup engine. None are silently included in the Minimal preset. Search history and lock-screen tips are included in Balanced because both are ordinary privacy/content toggles.

## Explicitly rejected

- Disabling Defender, Windows Update, error reporting, telemetry services, scheduled tasks, or firewall rules.
- Disabling BitLocker/device encryption or changing update/restart policy.
- Removing Edge, Microsoft Store, Web Experience, Xbox identity/accessibility frameworks, Photos, Calculator, Notepad, Terminal, or drivers.
- Broad OEM-app sweeps and removal of software a user may have intentionally installed.
- Deleting Explorer namespace registry trees.
- Disabling accessibility shortcuts such as Sticky Keys.
- Network-stack, DNS, timer, memory, service, and power-plan “performance” recipes.
- Policies documented as Enterprise/Education-only when the same behavior is not reliably available on Home/Pro.
- **Broad “registry cleaner” folklore:** whole-hive invalid-path/DLL sweeps, Explorer shell namespace autofix, COM CLSID mass deletes, “boost PC by cleaning registry,” or any autofix that is not a single previewed uninstall subkey with backup.
- Auto-disable of StartupApproved / Run entries from Sift (Windows Settings remains the supported path in v0.7).
- Storage autofill / scheduled mass delete, duplicate-file campaigns, or deleting junctions/reparse targets without an explicit future design.
- Bulk Disable of arbitrary Windows services, deleting/uninstalling services, “optimize all services” presets, or enable/disable of Defender/Update scheduled tasks.
- Editing scheduled-task triggers/actions, GPU-engine ETW, or per-process network attribution in v0.9.
- Auto-running SFC/DISM, Winsock reset, or other destructive “repair” scripts from Health Checks.

“Harmless” cannot be guaranteed for every workflow. Sift therefore keeps preference changes unselected unless they are conservative, marks package removal irreversible, previews exact targets, and preserves prior registry values for rollback.
