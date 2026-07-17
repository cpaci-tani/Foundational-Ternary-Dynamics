# Script Studio architecture

Script Studio extends Command Center without weakening its exact-catalog execution boundary. Command Center recipes remain the only executable scripts until an authored document can be converted into a typed, inspectable `ExecutionPlan` that Core can preflight without mutation and revalidate immediately before execution.

## Runtime flow

```text
Local Monaco editor / xterm.js terminal
  -> typed WebView2 JSON messages
  -> ScriptCenterWorkspaceModule
  -> IScriptStudioService
       -> finite runtime discovery
       -> language-specific non-executing syntax adapter
       -> common Sift policy analyzer
  -> diagnostics returned to Monaco markers and Problems

Exact catalog selection
  -> IScriptCommandService.Preflight
  -> confirmation when the recipe changes system state
  -> immediate canonical recipe/token revalidation
  -> sanitized child process
```

Read-only catalog recipes run after Core review without a second confirmation dialog. Recipes marked `ChangesState` or `Advanced` require confirmation before launch. Both paths use the same canonical catalog lookup, trusted shell resolution, and immediate recipe/token revalidation.

## Administrator commands

Administrator catalog recipes are visible to standard users. They elevate through typed `RunCatalogRecipe` requests that carry only `RecipeId` and `ExpectedRecipeHash`; the helper independently resolves the bundled catalog record, verifies the hash, and runs `ScriptCommandService`. Raw command text never crosses the elevation boundary. Already-elevated Sift sessions keep the in-process run path. Authored Studio documents remain analysis-only and never elevate.

The WebView receives no generic native-object proxy, arbitrary filesystem API, process launcher, or network URL. It is mapped to a fixed local asset directory. Messages are a closed set of typed records and must not contain executable native commands.

## Language-adapter phases

1. **Foundation:** PowerShell, Python, Bash, CMD, JavaScript, and TypeScript document models; finite runtime discovery; syntax parsing; shared policy diagnostics; Monaco and xterm.js hosted entirely from local assets.
2. **Language intelligence:** pinned offline Pyright and Ruff hosts for Python, PowerShell Editor Services and PSScriptAnalyzer, and vetted LSP adapters for other languages. Language servers run out of process with sanitized environments, bounded roots, cancellation, and no profile/plugin autoload.
3. **Debugging:** private named-pipe or loopback-with-nonce DAP adapters, beginning with debugpy. Debug adapters never listen on a public interface.
4. **Planned execution:** adapters translate supported operations into typed Core effects. Unknown calls, dynamic evaluation, runtime downloads, package installation, and opaque child-process launch remain blocked.
5. **Interactive PTY:** ConPTY-backed sessions are permitted only for an execution profile whose effects and token behavior satisfy the same policy. Standard and elevated state remain explicit; no raw script is sent through the bounded elevation helper.

## Runtime discovery

Discovery uses exact Windows system paths, exact Program Files locations, and registered Python `PythonCore` keys. It does not sweep drives, user profiles, virtual-environment trees, or package-manager caches. Project virtual environments will be added only through an explicit folder selection or trusted project file.

## Safety properties

- Analysis reads the in-memory document through standard input and never executes it.
- Python uses isolated, no-site, no-bytecode startup for AST parsing.
- PowerShell uses the parser API with profiles and interactivity disabled.
- Bash uses `bash -n`; JavaScript uses `node --check`.
- Analyzer children receive a minimal environment, an eight-second limit, cancellation, and process-tree termination.
- Static diagnostics are evidence, not proof of harmlessness. They never enable authored-script execution by themselves.
- Runtime and package downloads are absent from the product.
