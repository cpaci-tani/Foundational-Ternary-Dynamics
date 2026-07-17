# Sift

Sift is a Windows 11 system utility for viewing system activity, troubleshooting common problems, applying selected settings, and maintaining a PC from one native application.

Current version: **0.15.0**. Sift uses .NET 8, WinUI 3, and the Windows App SDK.

## What Sift includes

- **Home** — a customizable six-column lifecycle dashboard with independent Wide, Medium, and Compact profiles, drag/resize, live metrics, local trends, alerts, and guarded shortcuts to supported actions.
- **Optimize** — selectable Windows settings, presets, current-state detection, backups, and a link to Recovery.
- **Task Manager** — process, service, and scheduled-task inventories with actions for supported selections.
- **Performance** — live CPU, memory, disk, and process charts.
- **Hardware Monitor** — live sensor readings grouped by device, with filtering, minimum and maximum readings, and selected-sensor history.
- **Startup** — startup-app inventory and a link to Windows Startup Settings.
- **Maintenance** — categorized temporary-file and leftover-registration findings with selectable cleanup.
- **Script Studio** — more than 100 built-in troubleshooting commands plus local syntax analysis for PowerShell, Python, Bash, CMD, JavaScript, and TypeScript documents.
- **Health** — read-only system checks and local activity history.
- **Recovery** — backup inspection and restoration for supported Sift changes.
- **Storage** — folder scanning, a navigable treemap, and selected-item cleanup through the Recycle Bin.
- **Installed Apps** — registered desktop-app inventory, local signature details, uninstall handoff, and eligible leftover cleanup.
- **System Information** — Windows, security, hardware, storage, network, battery, and audio details with search and copy.
- **Settings** — display, lifecycle monitoring, 90-day retention, alert rules, quiet hours, optional notifications, activity-console, and restore-point preferences.

## Permissions and data

Sift starts with the current Windows account. When a selected action requires administrator access, Windows displays its permission prompt before the action begins. Starting Sift as administrator also makes the administrator command catalog available.

Settings, dashboard profiles, numeric metric history, activity history, and Sift backups stay under `%LOCALAPPDATA%\Sift`. Dashboard history never stores process names, commands, executable paths, or filenames. Optional monitoring while Sift is closed is a per-user, as-invoker process with no mutation or elevation API; background monitoring, background hardware sensors, and Windows notifications each begin disabled. Protected machine changes and background startup require a signed release whose Sift, helper, and monitor payloads have matching trusted signatures; builds without those signatures stay read-only for those protected operations. Sift does not use analytics, download commands or runtimes, or send system information to a remote service. Some built-in diagnostic commands contact the service named in their description, such as Microsoft or a configured DNS server.

See [Security and permissions](docs/SECURITY_AND_PERMISSIONS.md) for the action and permission model.

## Build

From the repository root:

```powershell
dotnet build apps\Sift\Sift.csproj --configuration Release
apps\Sift\scripts\validate.ps1
```

For publishing, packaging, signing, and clean-account acceptance, see [Build and release](docs/BUILD_AND_RELEASE.md).

## Project documentation

- [Architecture](ARCHITECTURE.md) — runtime boundaries, composition, and workspace lifecycle.
- [Security and permissions](docs/SECURITY_AND_PERMISSIONS.md) — action authorization, confirmation, and elevation contracts.
- [Roadmap](ROADMAP.md) — current capabilities and next completion gates.
- [Research audit](RESEARCH_AUDIT.md) — accepted and rejected Windows-management behavior.
- [Product language, permission, and architecture audit](docs/audits/AUDIT_PRODUCT_LANGUAGE_PERMISSION_ARCHITECTURE.md) — the active application-wide audit.
- [Feature audit manifest](docs/audits/sift-feature-audit.json) — machine-validated feature ownership and evidence.
