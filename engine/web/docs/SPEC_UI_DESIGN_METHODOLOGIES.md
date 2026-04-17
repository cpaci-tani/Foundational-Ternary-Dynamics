# Web UI Design Methodologies

Status: `[SELECTION]` design-system and component methodology for `engine/web`

Scope:
- [engine/web/index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html)
- [engine/web/js/ui](/C:/Users/cpaci/Desktop/ftd/engine/web/js/ui)
- [engine/web/css/ui](/C:/Users/cpaci/Desktop/ftd/engine/web/css/ui)

This document defines how custom UI components in the web engine should be designed, evaluated, and evolved. It is grounded in the current shell/component architecture already present in the codebase, especially:

- [SPEC_UI_REFACTOR.md](/C:/Users/cpaci/Desktop/ftd/engine/web/docs/SPEC_UI_REFACTOR.md)
- [app-shell.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/ui/shell/app-shell.css)
- [responsive.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/ui/shell/responsive.css)
- [topbar.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/ui/components/topbar.css)
- [panel-dock.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/ui/components/panel-dock.css)

## 1. Design Intent

The web engine UI should feel:

- dense enough for serious simulation work
- calm enough to not bury the physics under chrome
- modular enough that a scale can contribute its own UI without touching the shell
- responsive enough that the app is usable on phone, tablet, laptop, and ultrawide
- future-facing enough to host assistant workflows, teaching surfaces, and richer inspection tools

The visual target is not “generic dashboard.” It is a compact scientific instrument with a strong shell hierarchy:

- viewport is primary
- topbar is command/navigation
- panel dock is contextual knowledge and tooling
- overlays are transient and task-specific

## 2. Current Audit

### 2.1 What is already working well

- Shell ownership is becoming real:
  - `css/ui/shell`
  - `js/ui/shell`
- Components and panels are separated from the shell:
  - `css/ui/components`
  - `css/ui/panels`
  - `js/ui/components`
  - `js/ui/panels`
- Primitive-level CSS exists and should be treated as the base layer:
  - `button.css`
  - `field.css`
  - `select.css`
  - `slider.css`
  - `toggle.css`
  - `tabs.css`
- Mobile intent is explicit in [responsive.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/ui/shell/responsive.css), which is a big improvement over accidental shrinkage.
- Topbar and panel dock already show a good direction:
  - grouped controls
  - compact glass surfaces
  - better slot structure
  - dedicated mobile behaviors

### 2.2 What still needs tightening

- Several layouts are still ID-driven rather than component-class-driven.
- `responsive.css` still acts partly as a compatibility layer for legacy layout instead of only shell policy.
- Some panel extraction is structurally modular, but not yet visually decomposed enough; `panel-resources` can become the next monolith if left unchecked.
- Legacy/superseded surfaces still exist in the tree:
  - `js/ui/panels/ontic-panel/component.js`
- Some components still inherit too much of their look from old global assumptions instead of explicit primitive composition.

### 2.3 Immediate conclusion

The architecture is ready for a design-system discipline pass. The next stage should not be “add more one-off CSS.” It should be to standardize how custom components are built and reviewed.

## 3. Core Methodologies

### 3.1 Component ownership

Every custom UI element must have one clear owner.

A component owns:

- structure/template
- styling
- interaction bindings
- state boundary
- mount contract

That means:

- shell layout rules belong in `css/ui/shell`
- reusable visual atoms belong in `css/ui/primitives`
- reusable app chrome belongs in `css/ui/components`
- panel-specific content styling belongs in `css/ui/panels`
- scale-specific affordances belong in `css/ui/scales/<scale>`

No component should depend on “mystery styling” from old global files to look correct.

### 3.2 Composition over custom snowflakes

Custom components should be assembled from a small set of shared primitives rather than styled from scratch each time.

Preferred stack:

1. shell region
2. primitive surface
3. component wrapper
4. scale-specific contribution

Example:

- a scenario selector should be a field/select primitive inside a toolbar-group component
- a diagnostics card should be a card primitive inside a diagnostics panel component
- a mobile action drawer should be a modal/sheet primitive, not ad hoc absolute positioning

### 3.3 Progressive disclosure

The UI should default to the smallest useful surface and reveal complexity only when asked.

Rules:

- topbar shows core controls, not every control
- secondary options belong in drawers, sheets, panels, or grouped sections
- mobile should prefer dropdowns, sheets, and drawers over cramming horizontal toolbars
- advanced scientific detail should appear in dock panels and inspectors, not in global chrome

This fits the user request well: mobile dropdowns are good, and future assistant features should live in a summonable sidebar, not permanently occupy top-level space.

### 3.4 Viewport-first hierarchy

The simulation is the product. The UI should frame it, not compete with it.

Therefore:

- avoid wide permanent chrome unless the user is in analysis mode
- prefer semi-transparent, layered surfaces over opaque slabs
- use generous spacing between groups, but compact spacing within groups
- keep the viewport readable even when panels are open

### 3.5 State clarity

Every custom component must visibly communicate state.

Minimum states:

- default
- hover
- focus-visible
- active/selected
- disabled
- loading, if async
- empty, if data-driven
- error, if external dependency can fail

Scientific UIs become frustrating when controls do not explain whether they are inactive, unavailable, or merely hidden.

## 4. Responsive Methodology

The UI must be designed intentionally for each range, not “made responsive” at the end.

### 4.1 Breakpoint philosophy

Use layout mode, not arbitrary shrinkage.

Target modes:

- phone compact: `0-479px`
- phone large: `480-767px`
- tablet: `768-1023px`
- desktop: `1024-1439px`
- ultrawide: `1440px+`

### 4.2 Per-range behavior

Phone compact:

- single primary viewport column
- topbar wraps into compact rows
- panel access via dropdown, drawer, or bottom sheet
- one main panel visible at a time
- assistant opens as full-height drawer/sheet

Phone large:

- same model as compact phone, but permit slightly richer secondary row behavior
- preserve fast access to scenario selection and run controls

Tablet:

- viewport plus one docked contextual panel
- tab rail may collapse into select/dropdown
- assistant may render as a right drawer

Desktop:

- persistent topbar
- panel dock visible
- tab system visible
- overlays can float if they do not obscure the simulation

Ultrawide:

- allow multi-column paneling
- keep viewport central dominance
- do not simply stretch card widths; introduce max-widths inside panels

### 4.3 Responsive rules

All custom components must:

- support `min-width: 0` where flex/grid overflow is possible
- wrap intelligently before truncating
- preserve `44px` minimum touch targets on touch-first layouts
- avoid hover-only functionality
- define how they behave when horizontal space collapses

## 5. Visual Design Methodology

### 5.1 Surfaces

Use a layered surface model:

- shell background
- chrome surface
- panel/card surface
- overlay surface
- accent/interactive surface

This codebase already trends toward translucent “instrument glass.” Keep that, but systematize it.

Rules:

- major surfaces get stable radii and borders
- background blur is a privilege, not a default
- shadows should establish depth, not drama
- component surfaces should look related even when implemented by different modules

### 5.2 Density

The right target is compact, not cramped.

Guidelines:

- small internal spacing within grouped controls
- larger spacing between groups
- use typography and alignment to communicate hierarchy instead of adding extra borders everywhere
- prefer one strong container over three weak nested containers

### 5.3 Typography

Typography should encode function:

- headings: simulation/tool context
- labels: short and muted
- values: mono or high-contrast where numeric precision matters
- helper text: lighter and quieter

Do not use multiple competing type scales inside one component without a reason.

### 5.4 Color

Color should map to meaning:

- accent for active or primary actions
- muted tones for labels and scaffolding
- warning/error colors only for actual system states
- scale-specific colors only where they support cognition

Avoid arbitrary color drift between components.

## 6. Interaction Methodology

### 6.1 One obvious action per cluster

Every control cluster should answer:

- what is the primary action?
- what is the current state?
- what can I safely ignore?

If a group cannot answer those three questions, it is too dense or badly structured.

### 6.2 Fewer floating widgets

Floating widgets should be rare and purposeful.

Allowed uses:

- transient viewport overlays
- direct object inspection affordances
- context-following info chips

Not allowed:

- permanent controls that would work better as dock panels
- duplicated actions that already exist in the topbar or dock

### 6.3 Assistant as first-class but non-invasive

The future FTD model/assistant should be designed as a summonable collaborator.

Recommended pattern:

- desktop: right sidebar drawer
- tablet: right drawer or split pane
- phone: full-height modal sheet

Assistant UX rules:

- open from a single topbar affordance
- never block core run/step/reset controls
- preserve current simulation context while open
- support pinned prompts, current-scale context, and selected-object context

## 7. Accessibility Methodology

All custom components should be accessible by construction.

Minimum requirements:

- keyboard operable
- visible focus ring
- semantic labeling
- contrast-safe text
- no information conveyed by color alone
- reduced-motion-safe transitions
- pointer target size appropriate for touch

Required behavior:

- drawers and modals trap focus
- close buttons are always present and labeled
- dropdowns remain usable without hover
- panel tabs expose selected state clearly

## 8. CSS Architecture Rules

### 8.1 Allowed CSS ownership

`css/ui/shell`:

- layout regions
- z-index policy
- breakpoint policy
- shell-only spacing contracts

`css/ui/primitives`:

- reusable buttons, fields, cards, tabs, toggles, modal shells

`css/ui/components`:

- app chrome such as topbar, tabs, dock, status bar, overlays, assistant sidebar

`css/ui/panels`:

- panel-specific internals

`css/ui/scales`:

- scale-owned extras like legends, telemetry, pedagogy, toolbar-specific treatments

### 8.2 What to avoid

- styling component internals from unrelated CSS folders
- ID-only styling for reusable component behaviors
- component-specific breakpoint hacks in old global files
- absolute positioning as the default layout mechanism
- adding another “catch-all” stylesheet

### 8.3 Naming and selector policy

Prefer:

- component classes
- state classes
- data attributes for shell state

Use IDs only when the runtime genuinely needs a stable DOM hook.

Visual styling should target:

- `.component-name`
- `.component-name__slot`
- `.is-active`
- `[data-layout-mode="tablet"]`

instead of depending on app-wide ID chains wherever possible.

## 9. Custom Component Acceptance Checklist

A custom component is ready only if all of these are true:

- it has one owner for template, style, and behavior
- it composes primitives instead of re-inventing them
- it works at phone, tablet, desktop, and ultrawide sizes
- it defines empty, loading, disabled, and selected states as needed
- it is keyboard and touch usable
- it does not require unrelated global CSS to look correct
- it does not force `index.html` or `app_dag.js` to know its internal DOM structure
- it has a clear mount contract
- it can be removed or replaced without breaking the shell

## 10. Recommended Design Reviews for Existing Surfaces

### 10.1 Topbar

Keep:

- slot-based layout
- grouped command clusters
- compact two-row structure

Improve:

- unify group treatment with primitive-level field/button patterns
- ensure mobile behavior is a deliberate sheet/dropdown strategy, not only wrap behavior

### 10.2 Panel dock

Keep:

- single shared dock shell
- compact head/body structure
- hide button on mobile

Improve:

- move more visual patterns into panel-specific CSS so `panel-resources` does not accumulate too much responsibility

### 10.3 Inspector

Keep:

- simplified, mode-aware inspector concept
- clear empty states
- scale-specific internals

Improve:

- continue separating scale-specific render templates/styles
- align inspector cards and rows with primitive field/card conventions

### 10.4 Assistant sidebar

Keep:

- sidebar/drawer entrypoint

Improve:

- formalize it as a drawer component contract
- define compact/mobile sheet behavior explicitly
- give it standard header, body, footer, and context-chip slots

### 10.5 Ontic panel

Recommendation:

- keep retired as a first-class dock panel unless a clearer product purpose is defined
- do not spend new design effort on it until it has a real user workflow

## 11. Suggested Next Steps

1. Convert remaining ID-driven component styling to class-oriented component styling.
2. Split heavyweight panel resource styling into panel-owned CSS where needed.
3. Formalize the assistant sidebar as a shared drawer/sheet primitive plus component wrapper.
4. Create a lightweight visual QA checklist for each breakpoint before merging UI work.
5. Treat `responsive.css` as shell policy only; move component-specific responsive rules back into component-owned files when possible.

## 12. Definition of Done for the UI Refactor

The UI refactor is design-complete when:

- `index.html` is mostly shell mounts and stable hooks
- each custom surface has a clear component owner
- shell, primitive, component, panel, and scale CSS responsibilities are cleanly separated
- mobile behavior is deliberate, not fallback behavior
- assistant, inspector, and panel dock all behave like first-class components
- retired surfaces stay retired unless they regain a clear workflow purpose

