# Web UI Refactor Implementation Spec

Status: `[SELECTION]` behavior-preserving refactor plan for the browser UI

Scope: `engine/web`

Primary goals:
- Convert UI surfaces from monolithic HTML and global CSS buckets into component-owned resources.
- Make the shell modular so each simulation scale contributes UI through stable registration points.
- Make the app intentionally responsive across phone, tablet, desktop, and ultrawide layouts.
- Preserve current features and identifiers during the migration unless explicitly replaced by an equivalent component contract.

## 1. Current State

The current UI is still centered on a large root document in [index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html), with broad shared styles in [layout.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/layout.css) and [components.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/components.css). The main issues to solve are:

- `index.html` owns too much markup:
  - loading overlay
  - toolbar
  - scale-specific scenario selectors
  - viewport overlays
  - tab bar
  - all major panels
  - modal roots and floating widgets
- CSS ownership is coarse:
  - `layout.css` mixes shell, overlays, toolbar, responsive rules, and theme-specific fixes
  - `components.css` mixes cards, tabs, charts, modals, overlays, inspector, and miscellaneous panels
- Layout is desktop-first and absolute-positioned:
  - `#toolbar`, `#tab-bar`, `#panel-area`, and several overlays are positioned as floating layers
  - small-screen behavior is derived from compression rather than from explicit mobile layouts
- Scale-specific UI is not fully isolated from the shell:
  - the root page still contains scale-specific controls and panels
  - app shell concerns and scale concerns are interleaved

This spec replaces that structure with a shell plus component-resource model.

## 2. Target Architecture

### 2.1 Shell contract

The application shell becomes responsible for:

- mounting the viewport
- rendering the global shell chrome
- hosting drawers, sheets, modals, and toasts
- switching layout modes by breakpoint
- mounting scale-provided controls, overlays, and panels

The shell must not hardcode scale-specific control markup.

### 2.2 Component-resource contract

Every UI component owns:

- its template
- its stylesheet
- its binding/controller logic
- its registration metadata if it is shell-mounted

Component rules:

- no component markup authored inline in `index.html` beyond shell mount roots
- no component-specific CSS in shared global buckets
- no scale-specific markup in shell resources
- no desktop-only layout assumptions in shared primitives

### 2.3 Scale UI contract

Each scale contributes UI through a registration API:

- toolbar contributions
- panel contributions
- viewport overlay contributions
- quick actions
- modal contributions if needed

Target interface:

```js
{
  toolbar: ToolbarItemDefinition[],
  panels: PanelDefinition[],
  overlays: OverlayDefinition[],
  actions: ActionDefinition[],
}
```

`app.js` remains the composition root and scheduler host, but not the owner of scale markup.

## 3. Target Folder Structure

The refactor introduces a dedicated UI tree under `engine/web/js/ui` and a matching style tree under `engine/web/css/ui`.

```text
engine/web/
├── index.html
├── css/
│   ├── tokens.css
│   ├── scale-visibility.css
│   ├── themes/
│   │   ├── abyss.css
│   │   ├── light.css
│   │   ├── nord.css
│   │   └── parchment.css
│   └── ui/
│       ├── shell/
│       │   ├── app-shell.css
│       │   ├── responsive.css
│       │   ├── regions.css
│       │   └── z-layers.css
│       ├── primitives/
│       │   ├── button.css
│       │   ├── card.css
│       │   ├── checkbox.css
│       │   ├── drawer.css
│       │   ├── field.css
│       │   ├── modal.css
│       │   ├── segmented-control.css
│       │   ├── select.css
│       │   ├── sheet.css
│       │   ├── slider.css
│       │   ├── tabs.css
│       │   ├── toast.css
│       │   └── toggle.css
│       ├── components/
│       │   ├── loading-overlay.css
│       │   ├── panel-dock.css
│       │   ├── status-bar.css
│       │   ├── topbar.css
│       │   ├── viewport-frame.css
│       │   ├── viewport-overlay.css
│       │   └── workspace-tabs.css
│       ├── panels/
│       │   ├── charts-panel.css
│       │   ├── controls-panel.css
│       │   ├── diagnostics-panel.css
│       │   ├── hierarchy-panel.css
│       │   ├── inspector-panel.css
│       │   ├── lagrangian-panel.css
│       │   ├── ontic-panel.css
│       │   ├── physics-panel.css
│       │   ├── planetary-panel.css
│       │   ├── reference frame context-panel.css
│       │   ├── cosmic-info-panel.css
│       │   ├── meta-info-panel.css
│       │   ├── quantum-lab-panel.css
│       │   └── zoo-panel.css
│       └── scales/
│           ├── scale0/
│           ├── scale1/
│           ├── scale2/
│           ├── scale3/
│           ├── scale4/
│           ├── scale5/
│           ├── scale11/
│           └── scale12/
└── js/
    ├── app.js
    ├── dom-utils.js
    ├── viewport.js
    ├── ui/
    │   ├── shell/
    │   │   ├── app-shell.js
    │   │   ├── breakpoint-service.js
    │   │   ├── layout-state.js
    │   │   ├── mount-registry.js
    │   │   ├── panel-dock-controller.js
    │   │   ├── shell-events.js
    │   │   └── shell-template.js
    │   ├── primitives/
    │   │   ├── button.js
    │   │   ├── card.js
    │   │   ├── checkbox.js
    │   │   ├── drawer.js
    │   │   ├── field.js
    │   │   ├── modal.js
    │   │   ├── segmented-control.js
    │   │   ├── select.js
    │   │   ├── sheet.js
    │   │   ├── slider.js
    │   │   ├── tabs.js
    │   │   ├── toast.js
    │   │   └── toggle.js
    │   ├── components/
    │   │   ├── loading-overlay/
    │   │   │   ├── component.js
    │   │   │   └── template.js
    │   │   ├── panel-dock/
    │   │   │   ├── component.js
    │   │   │   └── template.js
    │   │   ├── topbar/
    │   │   │   ├── component.js
    │   │   │   └── template.js
    │   │   ├── viewport-frame/
    │   │   │   ├── component.js
    │   │   │   └── template.js
    │   │   ├── viewport-overlay/
    │   │   │   ├── component.js
    │   │   │   └── template.js
    │   │   └── workspace-tabs/
    │   │       ├── component.js
    │   │       └── template.js
    │   ├── panels/
    │   │   ├── charts-panel/
    │   │   ├── controls-panel/
    │   │   ├── diagnostics-panel/
    │   │   ├── hierarchy-panel/
    │   │   ├── inspector-panel/
    │   │   ├── lagrangian-panel/
    │   │   ├── ontic-panel/
    │   │   ├── physics-panel/
    │   │   ├── planetary-panel/
    │   │   ├── reference frame context-panel/
    │   │   ├── cosmic-info-panel/
    │   │   ├── meta-info-panel/
    │   │   ├── quantum-lab-panel/
    │   │   └── zoo-panel/
    │   └── scale-registry/
    │       ├── register-scale-ui.js
    │       ├── toolbar-registry.js
    │       ├── panel-registry.js
    │       └── overlay-registry.js
    └── scales/
        ├── scale0/
        │   ├── controller.js
        │   ├── scenario-registry.js
        │   ├── viewport-adapter.js
        │   ├── ui/
        │   │   ├── register-scale0-ui.js
        │   │   ├── toolbar/
        │   │   ├── overlays/
        │   │   ├── panels/
        │   │   └── controls/
        ├── scale1/
        │   └── ui/
        ├── scale2/
        │   └── ui/
        ├── scale3/
        │   └── ui/
        ├── scale4/
        │   └── ui/
        ├── scale5/
        │   └── ui/
        ├── scale11/
        │   └── ui/
        └── scale12/
            └── ui/
```

Notes:
- `css/layout.css` and `css/components.css` are transitional files and will be retired.
- `tokens.css`, `scale-visibility.css`, and theme files remain global.
- If later desired, component CSS can be imported by JS instead of linked directly; this spec assumes plain linked CSS for minimal tooling risk.

## 4. Responsive Layout System

The shell must implement explicit layout modes.

### 4.1 Breakpoints

```text
compact-sm   0   - 479px
compact-lg   480 - 767px
tablet       768 - 1023px
desktop      1024 - 1439px
wide         1440px+
```

### 4.2 Layout behavior by breakpoint

#### compact-sm

- topbar collapses to two rows or one row plus overflow drawer
- scenario selectors move into contextual sheets
- panel dock becomes a bottom sheet
- only one major panel visible at a time
- viewport overlays become chips or drawers, not floating stacks
- panel resizer is disabled
- touch target minimum `44px`

#### compact-lg

- same core layout as compact-sm
- quick actions may remain visible as a secondary horizontal action row
- inspector and diagnostics may use taller sheets

#### tablet

- shell becomes two-region layout: viewport plus one dock
- dock can be side-mounted in landscape and bottom-mounted in portrait
- tabs become a compact rail or segmented control
- floating overlays allowed only for minimal non-blocking items

#### desktop

- shell uses persistent topbar, tab rail, and side dock
- multi-section controls panel allowed
- viewport overlays can remain docked inside the frame edges

#### wide

- allow multi-column panel layouts
- allow inspector plus diagnostics side by side
- expand charts and hierarchy panels
- keep viewport primary; wide mode is not license for uncontrolled spread

### 4.3 Layout invariants

- viewport always remains the primary region
- no horizontal toolbar scrolling as the default compact interaction model
- no critical controls hidden behind hover-only interactions
- every shell region must degrade cleanly to drawer or sheet form

## 5. Component Inventory

This is the required component inventory for the first full refactor pass.

### 5.1 Shell components

- `AppShell`
- `Topbar`
- `ViewportFrame`
- `PanelDock`
- `WorkspaceTabs`
- `StatusBar`
- `LoadingOverlay`
- `ModalRoot`
- `ToastRoot`

### 5.2 Primitive components

- `UiButton`
- `UiIconButton`
- `UiSelect`
- `UiCheckbox`
- `UiToggle`
- `UiSlider`
- `UiSegmentedControl`
- `UiCard`
- `UiDrawer`
- `UiSheet`
- `UiModal`
- `UiTabs`
- `UiField`
- `UiToast`

### 5.3 Shared panel components

- `ControlsPanel`
- `DiagnosticsPanelView`
- `ChartsPanelView`
- `LagrangianPanelView`
- `InspectorPanelView`
- `ZooPanelView`
- `OnticPanelView`
- `PhysicsPanelView`
- `PlanetaryPanelView`
- `HierarchyPanelView`
- `Reference frame contextPanelView`
- `CosmicInfoPanelView`
- `MetaInfoPanelView`
- `QuantumLabPanelView`

### 5.4 Viewport overlay components

- `Scale0ViewportOverlay`
- `Scale1ViewportOverlay`
- `Scale2ViewportOverlay`
- `Scale3ViewportOverlay`
- `Scale11ViewportOverlay`
- `UniversalViewportToggleBar`
- `FloatingSymmetryPanel`

### 5.5 Scale-owned control components

#### Scale 0

- `Scale0ScenarioSelect`
- `Scale0ScenarioEpistemicStatus`
- `Scale0PhysicsTogglesCard`
- `Scale0SubstrateControlsCard`
- `Scale0FluxVolumeCard`
- `Scale0BoundaryControls`
- `Scale0FieldOverlayToggleGroup`
- `Scale0ForceStyleSelector`

#### Scale 1

- `Scale1ScenarioSelect`
- `Scale1PhysicsControlsCard`
- `Scale1FieldVizCard`
- `Scale1CustomParticleCard`
- `Scale1ViewportOverlay`

#### Scale 2

- `Scale2ScenarioSelect`
- `Scale2AtomControlsCard`
- `Scale2BondStyleCard`
- `Scale2FieldVizCard`
- `Scale2ViewportOverlay`

#### Scale 3

- `Scale3ScenarioSelect`
- `Scale3MoleculeControlsCard`
- `Scale3BondAndCloudCard`
- `Scale3ViewportOverlay`

#### Scale 4

- `Scale4ScenarioSelect`
- `Scale4SystemSummaryCard`
- `Scale4PlanetaryControlsPanel`

#### Scale 5

- `Scale5ScenarioSelect`
- `Scale5CosmicSummaryCard`
- `Scale5PopulationCard`
- `Scale5CosmicInfoPanel`

#### Scale 11

- `Scale11ScenarioSelect`
- `Scale11Subtabs`
- `Scale11ObserverPanel`
- `Scale11ViewportOverlay`

#### Scale 12

- `Scale12MetaInfoPanel`
- `Scale12MetaInspectPanel`

## 6. Current Surface Mapping

This section maps current monolithic surfaces to their target owners.

### 6.1 Root document

Current:
- [index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html)

Target split:
- `js/ui/shell/shell-template.js`
- `js/ui/components/loading-overlay/template.js`
- `js/ui/components/topbar/template.js`
- `js/ui/components/workspace-tabs/template.js`
- `js/ui/components/panel-dock/template.js`
- `js/ui/components/viewport-frame/template.js`
- scale-owned UI templates under `js/scales/*/ui/**`

### 6.2 Global shell and component CSS

Current:
- [css/layout.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/layout.css)
- [css/components.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/components.css)
- [css/charts.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/charts.css)

Target split:
- `css/ui/shell/*`
- `css/ui/primitives/*`
- `css/ui/components/*`
- `css/ui/panels/*`
- `css/ui/scales/*`

### 6.3 Tab and panel system

Current:
- tab bar and panel area live in `index.html`
- tab behavior is handled in `app.js`

Target split:
- `js/ui/components/workspace-tabs/component.js`
- `js/ui/components/panel-dock/component.js`
- `js/ui/shell/panel-dock-controller.js`
- `js/ui/scale-registry/panel-registry.js`

### 6.4 Loading overlay

Current:
- markup and animation bootstrap are inline in `index.html`

Target split:
- `js/ui/components/loading-overlay/component.js`
- `js/ui/components/loading-overlay/template.js`
- `css/ui/components/loading-overlay.css`

### 6.5 Settings modal

Current:
- markup embedded in `index.html`

Target split:
- `js/ui/primitives/modal.js`
- `js/ui/components/settings-modal/component.js`
- `js/ui/components/settings-modal/template.js`
- `css/ui/components/settings-modal.css`

## 7. File-by-File Migration Order

This is the implementation order. Each step should leave the app runnable.

### Phase 0: Introduce the UI shell without changing behavior

1. Create `engine/web/docs/SPEC_UI_REFACTOR.md`
2. Add shell JS files:
   - `engine/web/js/ui/shell/app-shell.js`
   - `engine/web/js/ui/shell/shell-template.js`
   - `engine/web/js/ui/shell/layout-state.js`
   - `engine/web/js/ui/shell/breakpoint-service.js`
   - `engine/web/js/ui/shell/mount-registry.js`
3. Add shell CSS files:
   - `engine/web/css/ui/shell/app-shell.css`
   - `engine/web/css/ui/shell/regions.css`
   - `engine/web/css/ui/shell/responsive.css`
   - `engine/web/css/ui/shell/z-layers.css`
4. Update [index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html):
   - keep current IDs alive
   - add shell mount roots
   - load new shell CSS after tokens/theme CSS
   - load new shell JS entry through `app.js`
5. Update [app.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/app.js):
   - instantiate `AppShell`
   - move shell DOM lookup into a shell adapter
   - keep old behavior intact while the shell mirrors existing DOM

Exit criterion:
- app still boots exactly as before
- no visual redesign yet
- shell services exist and are testable

### Phase 1: Extract shared primitives

Files to add:
- `engine/web/js/ui/primitives/button.js`
- `engine/web/js/ui/primitives/select.js`
- `engine/web/js/ui/primitives/checkbox.js`
- `engine/web/js/ui/primitives/toggle.js`
- `engine/web/js/ui/primitives/slider.js`
- `engine/web/js/ui/primitives/card.js`
- `engine/web/js/ui/primitives/tabs.js`
- `engine/web/js/ui/primitives/drawer.js`
- `engine/web/js/ui/primitives/sheet.js`
- `engine/web/js/ui/primitives/modal.js`
- `engine/web/js/ui/primitives/toast.js`
- matching files under `engine/web/css/ui/primitives/`

Files to update:
- [layout.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/layout.css)
- [components.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/components.css)

Work:
- carve primitive styles out of the current CSS buckets
- keep the old classes as compatibility aliases during migration

Exit criterion:
- shared controls use primitive styles
- CSS duplication starts shrinking

### Phase 2: Move shell-owned components out of `index.html`

Files to add:
- `engine/web/js/ui/components/loading-overlay/component.js`
- `engine/web/js/ui/components/loading-overlay/template.js`
- `engine/web/js/ui/components/topbar/component.js`
- `engine/web/js/ui/components/topbar/template.js`
- `engine/web/js/ui/components/workspace-tabs/component.js`
- `engine/web/js/ui/components/workspace-tabs/template.js`
- `engine/web/js/ui/components/panel-dock/component.js`
- `engine/web/js/ui/components/panel-dock/template.js`
- `engine/web/js/ui/components/viewport-frame/component.js`
- `engine/web/js/ui/components/viewport-frame/template.js`
- matching CSS under `engine/web/css/ui/components/`

Files to update:
- [index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html)
- [app.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/app.js)

Work:
- remove inline loading overlay markup and script
- move topbar markup to `Topbar`
- move tab bar to `WorkspaceTabs`
- move panel dock scaffold to `PanelDock`
- move viewport container scaffold to `ViewportFrame`

Exit criterion:
- `index.html` is reduced to shell mounts and script/style entrypoints

### Phase 3: Build the registry-based panel system

Files to add:
- `engine/web/js/ui/scale-registry/register-scale-ui.js`
- `engine/web/js/ui/scale-registry/toolbar-registry.js`
- `engine/web/js/ui/scale-registry/panel-registry.js`
- `engine/web/js/ui/scale-registry/overlay-registry.js`
- `engine/web/js/ui/shell/panel-dock-controller.js`

Files to update:
- [app.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/app.js)

Work:
- convert tab visibility logic into registry-driven definitions
- make panel mounting data-driven
- keep existing panel IDs where practical for compatibility

Exit criterion:
- shell asks registries what to render
- shell no longer hardcodes panel content

### Phase 4: Migrate shared panels one by one

Files to add:
- `engine/web/js/ui/panels/controls-panel/component.js`
- `engine/web/js/ui/panels/diagnostics-panel/component.js`
- `engine/web/js/ui/panels/charts-panel/component.js`
- `engine/web/js/ui/panels/lagrangian-panel/component.js`
- `engine/web/js/ui/panels/inspector-panel/component.js`
- `engine/web/js/ui/panels/zoo-panel/component.js`
- `engine/web/js/ui/panels/ontic-panel/component.js`
- `engine/web/js/ui/panels/physics-panel/component.js`
- `engine/web/js/ui/panels/planetary-panel/component.js`
- `engine/web/js/ui/panels/hierarchy-panel/component.js`
- `engine/web/js/ui/panels/reference frame context-panel/component.js`
- `engine/web/js/ui/panels/cosmic-info-panel/component.js`
- `engine/web/js/ui/panels/meta-info-panel/component.js`
- `engine/web/js/ui/panels/quantum-lab-panel/component.js`
- matching CSS under `engine/web/css/ui/panels/`

Files to update:
- [index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html)
- [app.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/app.js)
- [charts.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/charts.js)
- [diagnostics.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/diagnostics.js)
- [inspector.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/inspector.js)
- [lagrangian.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/lagrangian.js)
- [pe-telemetry.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/pe-telemetry.js)
- [ontic-observatory.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/ontic-observatory.js)
- [aggregation-bridge.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/aggregation-bridge.js)
- [meta-pedagogy.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/meta-pedagogy.js)
- [reference frame context-pedagogy.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/reference frame context-pedagogy.js)

Work:
- adapt panel producers to mount into component-owned roots rather than static page markup
- keep existing data/rendering classes intact while swapping host nodes

Exit criterion:
- each panel has a component owner and dedicated stylesheet

### Phase 5: Migrate viewport overlays and floating widgets

Files to add:
- `engine/web/js/ui/components/viewport-overlay/component.js`
- `engine/web/js/ui/components/viewport-overlay/template.js`
- `engine/web/css/ui/components/viewport-overlay.css`

Scale-owned files to add:
- `engine/web/js/scales/scale0/ui/overlays/register-scale0-overlays.js`
- `engine/web/js/scales/scale1/ui/overlays/register-scale1-overlays.js`
- `engine/web/js/scales/scale2/ui/overlays/register-scale2-overlays.js`
- `engine/web/js/scales/scale3/ui/overlays/register-scale3-overlays.js`
- `engine/web/js/scales/scale11/ui/overlays/register-scale11-overlays.js`

Files to update:
- [index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html)
- [viewport.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/viewport.js)
- [app.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/app.js)
- scale controllers under `engine/web/js/scales/*/controller.js`

Work:
- extract `#viewport-overlay`, `#pe-viewport-overlay`, `#ae-viewport-overlay`, `#mol-viewport-overlay`, `#cs-viewport-overlay`, `#viewport-toggles-universal`, and `#floating-symmetry-panel`
- route them through overlay registrations
- allow shell to render overlay content as desktop docked overlays or compact sheets

Exit criterion:
- no viewport overlay markup lives in root HTML

### Phase 6: Migrate toolbar contributions by scale

Files to add:
- `engine/web/js/scales/scale0/ui/register-scale0-ui.js`
- `engine/web/js/scales/scale1/ui/register-scale1-ui.js`
- `engine/web/js/scales/scale2/ui/register-scale2-ui.js`
- `engine/web/js/scales/scale3/ui/register-scale3-ui.js`
- `engine/web/js/scales/scale4/ui/register-scale4-ui.js`
- `engine/web/js/scales/scale5/ui/register-scale5-ui.js`
- `engine/web/js/scales/scale11/ui/register-scale11-ui.js`
- `engine/web/js/scales/scale12/ui/register-scale12-ui.js`

Files to update:
- scale controllers under `engine/web/js/scales/*/controller.js`
- [app.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/app.js)

Work:
- move scenario selectors and scale-specific toolbar controls out of the shell
- shell renders only base transport controls and registered contributions

Exit criterion:
- topbar is scale-agnostic

### Phase 7: Scale 0 pilot migration

Files to add:
- `engine/web/js/scales/scale0/ui/controls/scale0-scenario-select.js`
- `engine/web/js/scales/scale0/ui/controls/scale0-epistemic-status.js`
- `engine/web/js/scales/scale0/ui/controls/scale0-physics-toggles-card.js`
- `engine/web/js/scales/scale0/ui/controls/scale0-substrate-controls-card.js`
- `engine/web/js/scales/scale0/ui/controls/scale0-flux-volume-card.js`
- `engine/web/js/scales/scale0/ui/controls/scale0-boundary-controls.js`
- `engine/web/js/scales/scale0/ui/controls/scale0-force-style-selector.js`
- matching CSS under `engine/web/css/ui/scales/scale0/`

Files to update:
- [index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html)
- [app.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/app.js)
- [engine/web/js/scales/scale0/controller.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/scales/scale0/controller.js)
- [engine/web/js/scales/scale0/scenario-registry.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/scales/scale0/scenario-registry.js)
- files under `engine/web/js/scales/scale0/ui/`

Work:
- make Scale 0 the reference implementation for scale-owned UI
- bind state through Scale 0 UI modules instead of root-page markup

Exit criterion:
- Scale 0 UI is fully componentized and portable

### Phase 8: Remaining scale migrations

Files to update:
- [engine/web/js/scales/scale1/controller.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/scales/scale1/controller.js)
- [engine/web/js/scales/scale2/controller.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/scales/scale2/controller.js)
- [engine/web/js/scales/scale3/controller.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/scales/scale3/controller.js)
- [engine/web/js/scales/scale4/controller.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/scales/scale4/controller.js)
- [engine/web/js/scales/scale5/controller.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/scales/scale5/controller.js)
- [engine/web/js/scales/scale11/controller.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/scales/scale11/controller.js)

Files to add:
- scale-specific UI resources under each `engine/web/js/scales/*/ui/`
- scale CSS under each `engine/web/css/ui/scales/*/`

Exit criterion:
- all scales register UI instead of editing shell markup

### Phase 9: Remove transitional buckets

Files to shrink or retire:
- [css/layout.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/layout.css)
- [css/components.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/components.css)
- [css/charts.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/charts.css)

Files to update:
- [index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html)

Work:
- move remaining selectors into owned resources
- keep only:
  - tokens
  - themes
  - scale visibility rules
  - minimal compatibility shims if still needed

Exit criterion:
- no broad catch-all component CSS remains

## 8. File Ownership Matrix

### Files that remain global

- [index.html](/C:/Users/cpaci/Desktop/ftd/engine/web/index.html)
- [js/app.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/app.js)
- [js/viewport.js](/C:/Users/cpaci/Desktop/ftd/engine/web/js/viewport.js)
- [css/tokens.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/tokens.css)
- [css/scale-visibility.css](/C:/Users/cpaci/Desktop/ftd/engine/web/css/scale-visibility.css)
- theme CSS files

### Files that become shell-owned

- shell JS under `js/ui/shell/`
- shell CSS under `css/ui/shell/`
- topbar, tab bar, panel dock, loading overlay, viewport frame

### Files that become panel-owned

- diagnostics, charts, lagrangian, inspector, ontic, hierarchy, physics, zoo, cosmic info, meta info, quantum lab

### Files that become scale-owned

- scenario selectors
- scale-specific controls
- scale-specific overlays
- scale-specific control cards
- scale-specific toolbar contributions

## 9. Testing and Validation Plan

### 9.1 Static checks

- no duplicate IDs after component extraction
- each component exports a mount API and cleanup API
- registries validate unique panel IDs and overlay IDs

### 9.2 Playwright coverage additions

Extend [scales.spec.js](/C:/Users/cpaci/Desktop/ftd/engine/web/tests/scales.spec.js) with:

- shell boots with component mounts present
- compact breakpoint renders drawer/sheet mode
- tablet breakpoint renders single dock mode
- desktop breakpoint renders side dock mode
- switching scales does not leak panel listeners
- panel registry reflects correct tabs by scale
- toolbar registry updates scenario controls when engine mode changes

### 9.3 Acceptance criteria

- all screens remain usable at:
  - `360x640`
  - `390x844`
  - `768x1024`
  - `1024x768`
  - `1366x768`
  - `1440x900`
  - `1920x1080`
  - `2560x1440`
- no major UI surface depends on root-document inline markup
- no scale contributes UI by editing shell HTML directly
- viewport remains usable while controls collapse into responsive shells
- keyboard and touch interactions both work

## 10. Immediate Implementation Recommendation

Start with:

1. `Phase 0`
2. `Phase 1`
3. `Phase 2`
4. `Phase 7`

That sequence gives one complete vertical slice:
- real shell
- real primitives
- real component extraction
- one scale fully migrated

Scale 0 should be the pilot because it already has the strongest modularization momentum under `engine/web/js/scales/scale0/`.

---

## 11. Implementation Status

### Completed phases

| Phase | Description | Status |
|-------|-------------|--------|
| 0 | Shell JS/CSS skeleton, AppShell instantiated in `app.js` | ✅ Done |
| 1 | Shared CSS primitives (button, toggle, slider, card, etc.) | ✅ Done |
| 2 | Shell components extracted (topbar, panel-dock, workspace-tabs, loading-overlay, viewport-frame) | ✅ Done |
| 3 | Registry-based panel system (toolbar-registry, panel-registry, overlay-registry) | ✅ Done |
| 4 | Shared panel wrappers (all 13 panels) | ✅ Done — all panels have `component.js` + owned CSS stub |
| 5 | Viewport overlay component extracted | ✅ Done — `js/ui/components/viewport-overlays/component.js` |
| 6 | Scale toolbar contributions registered | ✅ Done — all scales have `register-scaleN-ui.js` + `toolbar/component.js` |
| 7 | Scale 0 pilot migration | ✅ Done — `scales/scale0/ui/` fully componentized |
| 8 | Remaining scale migrations (1–5, 11, 12) | ✅ Done — all scales have `register-scaleN-ui.js`, toolbar, controls, overlays |
| 9 | Retire transitional buckets (`layout.css`, `components.css`) | ✅ Done — `components.css` deleted earlier; `layout.css` stubbed 2026-04-16 (rules migrated to `tokens.css`, `scale-visibility.css`, `themes/*.css`, `css/ui/shell/responsive.css`, `css/ui/components/*`) |

### Pending work

- Delete the residual `css/layout.css` stub and its `<link>` tag in `index.html` once deployment caches have cycled (non-blocking).

### Completed this session (2026-04-16)

- ✅ Status bar styling — `css/ui/components/status-bar.css`; the unused JS wrapper was retired during the 2026-06-04 dead-code cleanup
- ✅ All 9 missing panel components — `inspector-panel`, `zoo-panel`, `ontic-panel`, `physics-panel`, `planetary-panel`, `hierarchy-panel`, `cosmic-info-panel`, `meta-info-panel`, `quantum-lab-panel` (each with `component.js` + CSS stub in `css/ui/panels/`)
- ✅ `js/ui/panels/index.js` updated to export all 13 panels
- ✅ Playwright breakpoint tests — `tablet` at 768×1024, `desktop` at 1280×800
- ✅ Status bar CSS migrated out of `layout.css` → `css/ui/components/status-bar.css`
- ✅ Telemetry Hub — `js/telemetry-hub.js` owns all ring buffers and bridge calls
- ✅ **Charts panel** componentized — `js/ui/panels/charts-panel/{component.js,template.js}`; `<div id="panel-charts">` in `index.html` reduced to empty mount root
- ✅ **Lagrangian panel** componentized — `js/ui/panels/lagrangian-panel/{component.js,template.js}`; mount root only in `index.html`
- ✅ **Reference frame context panel** componentized — `js/ui/panels/reference frame context-panel/{component.js,template.js}`; 140 lines of subtab/canvas markup removed from `index.html`
- ✅ **Settings modal** componentized — `js/ui/components/settings-modal/{component.js,template.js}`; mounted by `initSettingsModal()` in `app.js`, 80 lines of theme swatches + scale slider removed from `index.html`
- ✅ **`index.html` shrunk** from 516 → 187 lines (−64%)
- ✅ **`layout.css` retired** — content migrated: accessibility + theme transitions + `#app` root → `tokens.css`; theme-specific `.card` rules → `themes/{light,parchment}.css`; legacy class-based scale-visibility rules dropped (redundant with `data-active-scale` canonical rules + JS `applyScaleFilter`). File kept as a stub so cached HTML continues to 200 OK.
- ✅ Browser verification — preview boot clean (0 console errors, 40/40 stylesheets load), scale switching (Scale 0 → Scale 11) preserves correct tab filtering, panel mounts, and viewport rendering
- ✅ **Scale 0 controls fully modularised** — all Scale 0 control-panel wiring moved out of `app.js` into a dedicated [scales/scale0/ui/controls/wire.js](../js/scales/scale0/ui/controls/wire.js):
  - **18 physics toggles** (previously 14 were wired in `app.js` and 4 new ones — `t-color-forces`, `t-strong-force`, `t-exchange`, `t-weak` — existed in the markup but had no event handlers; now all wired from `SCALE0_TOGGLES` config as the single source of truth)
  - **Injection controls** (`inj-state-pos/neg`, `inj-x/y/z`, `btn-center`, `btn-random`, `btn-inject`, `btn-inject-wave`, `btn-inject-flux`, `btn-inject-pair`)
  - **Parameter sliders** (`combo-kb`, `combo-gn`, `combo-damp`)
  - **Flux volume controls** (`flux-shape-select`, `flux-opacity`, `flux-point-scale`, `flux-threshold`, `flux-scenario-scale`)
  - **Field actions** (`btn-clear-field`, `btn-random-flux`)

  Called from `Scale0Controller.bindUI()` via `wireScale0Controls(ctx, { setLatticeNeedsUpload })`. Uses live-accessor `ctx` so bridge reassignment during scale switches stays in sync.

  Also removed **dead wiring** from `app.js` for elements that no longer exist in the markup: `particle-shape`, `size-opacity`, `size-global`, `size-manifested`, `size-void`, `s0-dt-slider`, `btn-enable-all`, `btn-disable-all`, `btn-clear-particles`, `_visualSettings` hook, plus the dead `loadScenario`/`_markScenarioOverrides`/`_syncComboSliders` trio that was commented "DEAD" but never deleted. `app.js`'s `wireControls()` now handles only Scale 1 (PE) and Scale 2/3 (AE) wiring.

  Net effect: `app.js` shrunk ~340 lines; Scale 0 is now architecturally self-contained for its control-panel interactions.

---

## 12. Telemetry Hub

`engine/web/js/telemetry-hub.js` is the single JS module that owns all simulation telemetry and calculations. It is **not** part of the original phases above — it is a cross-cutting concern that runs alongside the component refactor.

### Responsibilities

- **Single write path**: all bridge telemetry calls (`getDiagnostics`, `getEnergyAudit`, `getLagrangian`, `peGetDiagnostics`, `peGetExtendedData`, `aeGetDiagnostics`) go through the hub, never scattered across controllers.
- **Canonical ring buffers**: all time-series data lives here. `FluxEnergyChart`, `ParticleChart`, and `LagrangianChart` receive their `RingBuffer` instances from the hub at construction time — they are pure renderers.
- **Derived metrics**: chirality ratio, conservation status, thermodynamics, orbital mechanics, Lagrangian decomposition — computed in one place.
- **Per-scale snapshots**: `hub.s0.diag`, `hub.s0.audit`, `hub.s1.diag`, etc. — latest raw bridge data for any panel to read.

### Buffer map

| Key | Scale | Samples | Contents |
|-----|-------|---------|----------|
| `flux`, `energy`, `manifested`, `entropy`, `positive`, `negative`, `charges` | 0 | 500 | Core lattice diagnostics |
| `ebDiff`, `gauss` | 0 | 500 | Energy audit derived |
| `sp.*` (5 buffers) | 0 | 80 | Sparkline-resolution copies |
| `lag.*` (10 buffers) | 0 | 400 | Lagrangian term decomposition |
| `peKE`, `pePE`, `peTotal`, `peCount`, `peMomentum`, `peAngMom`, `peVirial` | 1 | 200 | Particle engine |
| `aeKE`, `aeTemp`, `aeEnergy`, `aeBonds` | 2/3 | 200 | Atom/molecule engine |
| `csBodies`, `csHubble`, `csDM` | 5 | 200 | Cosmic |
| `csTheta`, `csIntensity`, `csFluxRatio` | 11 | 200 | Reference frame context |

### Collection API

```js
import { telemetryHub } from './telemetry-hub.js';

// Scale 0 — call from scale0/runtime/diagnostics.js
telemetryHub.collectScale0(bridge, fluxMock, useFluxMock);
telemetryHub.collectScale0Audit(bridge, fluxMock, useFluxMock);    // when diag/charts tab
telemetryHub.collectScale0Lagrangian(bridge, fluxMock, useFluxMock); // when lag tab

// Scale 1
telemetryHub.collectScale1(bridge);
telemetryHub.collectScale1Extended(bridge);

// Scale 2/3
telemetryHub.collectScale2(bridge);

// Scale 5
telemetryHub.collectScale5(cosmicBridge);

// Scale 11
telemetryHub.collectScale11(bridge);
```

### Derived metrics API

```js
telemetryHub.getScale0Derived()        // chiralityRatio, colorFraction, spinAsymmetry
telemetryHub.getConservationStatus()   // ok, gaussViolation, maxGaussError
telemetryHub.getScale1OrbitalMetrics() // 2-body Kepler params (if extended data available)
telemetryHub.getScale1Thermo()         // temperature, virialRatio, rmsVelocity
telemetryHub.getLagrangianDecomposition() // field/particle/interaction/constraint fractions
```

### Reset

```js
telemetryHub.resetScale(0);  // clears Scale 0 buffers on scenario change
telemetryHub.resetAll();     // clears all scales on engine mode switch
```

`clearCharts()` in `app.js` calls `telemetryHub.resetScale(0)` — this is the only needed call site for Scale 0 resets.
