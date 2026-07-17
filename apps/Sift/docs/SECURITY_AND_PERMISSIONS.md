# Sift security and permissions

This document defines how Sift authorizes actions that can change Windows or user data. Customer-facing screens describe the selected action and its consequence; implementation details remain here and in `ARCHITECTURE.md`.

## Permission model

The main application manifest is `asInvoker`, so a normal launch uses the current account's token. Read-only inventory, monitoring, local application settings, and current-user actions do not request administrator access.

Actions that need machine-wide access use one of two models:

| Action | Standard-user session | Administrator session |
|---|---|---|
| Apply supported machine-wide Optimize settings | Windows prompts for administrator permission through `Sift.ElevationHost` | Runs with the existing administrator token |
| Restore supported machine-wide backup entries | Windows prompts for administrator permission through `Sift.ElevationHost` | Runs with the existing administrator token |
| Start or restart a supported service | Windows prompts for administrator permission through `Sift.ElevationHost` | Runs with the existing administrator token |
| Enable or disable a supported scheduled task | Windows prompts for administrator permission through `Sift.ElevationHost` | Runs with the existing administrator token |
| Create an optional pre-change restore point | Windows prompts for administrator permission through `Sift.ElevationHost` | Runs with the existing administrator token |
| Run an administrator Script Studio recipe | Windows prompts for administrator permission through `Sift.ElevationHost` (`RunCatalogRecipe` with recipe ID + hash only) | Runs with the existing administrator token |
| Inspect hardware sensors | Read-only providers run with the current token | Driver-backed sensor access is unavailable until it has a separate explicit permission design |

`Sift.ElevationHost` is a short-lived helper, not a second application shell. Its request contract accepts typed operation identifiers and bounded data, not arbitrary commands, scripts, registry locations, or executable paths. The broker and helper independently validate request location, operation shape, and current target state, and each re-resolves the operation from its typed identifier rather than trusting request contents. The 256-bit nonce correlates a response to its request and rejects a stale one; because the request file is readable by the invoking user it is not treated as a same-user authenticity secret, so integrity rests on that independent re-resolution rather than the nonce. The broker keeps the request file write-locked through helper completion, the app and helper must have matching trusted signatures, and the administrator desktop presents the exact typed operation for a second confirmation before mutation. Builds without trusted matching signatures cannot perform protected machine changes.

## Action sequence

An action that changes Windows or removes data follows this sequence:

1. The user selects a target or a displayed batch.
2. Core inspects the proposed action without changing the target.
3. The application shows the action, target, relevant consequences, and whether Windows will request administrator permission.
4. Core rechecks identity, current state, and policy after confirmation.
5. Restorable prior state is captured before a supported change.
6. Core performs the action and records a typed activity result.

Closing or cancelling confirmation does not start the action. After final confirmation, the operation is committed: navigation, a duplicate click, or view disposal cannot abandon it midway. Cancelling a Windows permission prompt or the helper's administrator-side operation summary is reported separately from an execution failure.

## Boundary ownership

- **Optimize** owns tweak selection and application. Its backup link only navigates to Recovery.
- **Recovery** owns backup selection, inspection, confirmation, reinspection, and restore execution.
- **Task Manager** binds process actions to PID, start time, session, name, and executable path. Service actions remain bound to the reviewed Running or Stopped state.
- **Maintenance** binds a cleanup selection to a short-lived one-use review ticket and rejects the whole selection if reviewed content changes.
- **Storage** can authorize only a selected child of the current map and exposes Recycle Bin cleanup only.
- **Installed Apps** re-reads the selected registration before uninstaller handoff or leftover cleanup. File leftovers also use Recycle Bin cleanup only.
- **Script Studio** runs only canonical built-in recipes. Administrator recipes elevate through typed catalog identity (`RecipeId` + `ExpectedRecipeHash`); authored documents can be analyzed but cannot execute or cross the elevation boundary.

Sift resolves its fixed Windows command tools by absolute System32 path, passes arguments as separate tokens, sanitizes child environments, drains output with a size bound, and terminates command trees that exceed their explicit deadline. Process restart is blocked whenever the Sift shell is running with an administrator token so a selected user program cannot inherit that token.

The detailed invariants, protected-target exclusions, request validation, and persistence design are maintained in [ARCHITECTURE.md](../ARCHITECTURE.md).

## Local data and network behavior

Sift stores settings, activity history, and backups under `%LOCALAPPDATA%\Sift`. It has no analytics or remote command feed and does not download scripting runtimes. Local signature verification disables network retrieval.

Dashboard profiles are atomic versioned JSON. Imports reject newer schemas, unknown fields or widgets, duplicate singleton instances, invalid spans, and executable/path/URI-shaped settings. Dashboard metric history is a local SQLite database containing numeric machine aggregates only; metric keys containing process, command, executable, path, or filename identity are rejected. Profile export never contains history.

“Monitor when Sift is closed” is optional and disabled by default. It starts `Sift.MonitorHost` as the current user, never as a service or administrator, and only when its trusted signature matches the running Sift executable. Its current-user named pipe carries only protocol/version, typed snapshots, alerts, status, pause/resume, preference reload, acknowledgement/snooze, history clearing, and shutdown. It contains no action, script, executable/path/URI, arbitrary payload, or elevation operation. Folder startup registration is exact-path owned and is removed only when its value still matches Sift's monitor. Background hardware sensors and Windows notifications require separate opt-ins. A notification opens signed Sift to the alert context and never invokes a maintenance action.

Some explicitly selected built-in diagnostic recipes use normal Windows network tools. Their descriptions identify the destination or data involved before execution.

## Release trust

An unsigned MSIX is a package-layout test artifact, not a trusted release. A distributable build must use a trusted certificate whose subject matches the package manifest, and the application executable, elevation helper, monitor host, and MSIX must each pass Windows signature verification. The clean-account acceptance gate is documented in [Build and release](BUILD_AND_RELEASE.md).
