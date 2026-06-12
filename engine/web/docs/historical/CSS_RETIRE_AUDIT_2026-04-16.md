# CSS Retirement Audit

Generated from layout.css and components.css audit (2026-04-16).
Use this to safely delete rules in Phase 9 once the referenced primitive/shell/component
files are confirmed live and tested.

---

## layout.css

### Safe to remove after primitives extracted

- `.tb-btn`, `.tb-btn:hover`, `.tb-btn.active` → `css/ui/primitives/button.css`
- `.tb-btn-settings` → `css/ui/primitives/button.css`
- `@media (max-width:768px) .tb-btn` → `css/ui/primitives/button.css`
- `.tb-select`, `.tb-select:focus` → `css/ui/primitives/select.css`
- `.viewport-select-group`, `.viewport-select-group label`, `.viewport-select-group select`, `.viewport-select-group select:hover/focus`, `.viewport-select-group select option` → `css/ui/primitives/select.css`
- `html[data-theme="light"] .viewport-select-group select` / `parchment` overrides → `css/ui/primitives/select.css`
- `.viewport-select-group select` dark-glass force override block → `css/ui/primitives/select.css`
- `input[type="checkbox"]` accent-color → `css/ui/primitives/checkbox.css`
- `.view-toggle`, `.view-toggle:hover`, `.view-toggle.active` → `css/ui/primitives/toggle.css`
- `html[data-theme="light/parchment"] .view-toggle.active` → `css/ui/primitives/toggle.css`
- `#viewport-overlay .view-toggle`, `#viewport-toggles-universal .view-toggle` (dark glass block) → `css/ui/primitives/toggle.css`
- `#viewport-overlay .view-toggle.active/hover` block → `css/ui/primitives/toggle.css`
- `@media (max-width:768px) .view-toggle` → `css/ui/primitives/toggle.css`
- `.dynamics-toggle`, `.dynamics-toggle.active` → `css/ui/primitives/toggle.css`
- `.style-btn`, `.style-btn:first-child/last-child/not(:first-child)`, `.style-btn.active/hover` → `css/ui/primitives/button.css`
- `.force-style-row` → `css/ui/primitives/segmented-control.css`
- `.tb-slider` + all `::webkit-slider-thumb` / `::moz-range-thumb` variants → `css/ui/primitives/slider.css`

### Safe to remove after shell components are done

- `#app` → `css/ui/shell/` (app shell root)
- `#toolbar`, `#toolbar::-webkit-scrollbar` → `css/ui/components/topbar.css` (already has topbar component file)
- `#toolbar .brand`, `#toolbar .brand span` → `css/ui/components/topbar.css`
- `#toolbar .separator`, `.tb-group`, `.tb-label` → `css/ui/components/topbar.css`
- `#viewport`, `#viewport canvas` → `css/ui/components/viewport-frame.css` (already has viewport-frame component file)
- `#viewport-overlay` (positioning shell) → `css/ui/components/viewport-overlays.css`
- `.viewport-bottom-bar` → `css/ui/components/viewport-overlays.css`
- `#status-bar`, `#status-bar .status-item/value/dot/dot.idle` → `css/ui/shell/status-bar.css`
- `.ae-legend`, `.ae-legend-item`, `.ae-legend-swatch`, `.ae-legend-sym`, `.ae-legend-name`, `.ae-legend-sep`, `.ae-legend-header` → `css/ui/scales/` (Scale 2 component)
- `.tb-value`, `.tb-value-wide`, `.tb-check`, `.tb-check input` → `css/ui/components/topbar.css`
- `.field-swatch`, `.field-sep` → `css/ui/components/viewport-overlays.css` (used next to view-toggles)
- `@media (max-width:1024px)` responsive rules → `css/ui/shell/responsive.css`
- `@media (max-width:768px)` responsive rules (non-primitive parts) → `css/ui/shell/responsive.css`

### Must keep (global)

- `:focus-visible` outline rule — global accessibility, stays in `layout.css` or `tokens.css`
- `@media (prefers-reduced-motion: reduce)` — global accessibility override, keep here or in tokens.css
- `body, #toolbar, #tab-bar, ...` transition smoothing block — global theme-transition rule; keep until tokens.css absorbs it
- `#app.mode-*` scale-visibility blocks (all `.scale0-only`, `.scale1-only`, `.scale23-only`, `.scale-ae`, `.scale4-only`, `.scale5-only`, `.scale-cosmic-only`) — these are **global behavioral rules** tied to `#app` state; keep in `layout.css` or move to `css/ui/scale-visibility.css`
- Tab visibility per scale (`#app.mode-* .tab[data-scales="..."]`) — global behavioral, same recommendation
- Mode-specific viewport backgrounds (`#app.mode-cosmic #viewport`, `#app.mode-meta #viewport`, `#app.mode-reference frame context #viewport`) — keep global
- `#app.mode-meta .tab[data-panel="..."]`, `#app.mode-cosmic .tab[data-panel="..."]` — keep global

### Needs investigation

- `.panel-grid`, `.panel-grid-2/3/4` — in components.css too; check which is canonical and consolidate
- Responsive `#app { grid-template-rows }` — only valid if #app uses CSS Grid; verify current shell structure before removing

---

## components.css

### Safe to remove after primitives extracted

- `.toggle-row`, `.toggle-row input[type="checkbox"]`, `.toggle-row input[type="checkbox"]:checked`, `.toggle-row input[type="checkbox"]:checked::after`, `.toggle-row label` → `css/ui/primitives/checkbox.css` + `css/ui/primitives/field.css`
- `.toggle-row.scenario-override` → `css/ui/primitives/field.css`
- `.toggle-advanced`, `.toggle-advanced summary` + `::after` states → `css/ui/primitives/field.css`
- `.ctrl-input`, `.ctrl-input:focus` → `css/ui/primitives/field.css`
- `.ctrl-btn`, `.ctrl-btn:hover` → `css/ui/primitives/button.css`
- `.ctrl-btn-secondary`, `.ctrl-btn-secondary:hover` → `css/ui/primitives/button.css`
- `.ctrl-row`, `.ctrl-label` → `css/ui/primitives/field.css`
- `.pe-ctrl-row`, `.pe-ctrl-label`, `.pe-ctrl-value` → `css/ui/primitives/field.css`
- `.pe-slider` + all thumb variants → `css/ui/primitives/slider.css`
- `.combo-btn-row`, `.combo-btn-row .ctrl-btn-secondary`, `.combo-section-label` → `css/ui/primitives/segmented-control.css`
- `.card`, `.card:hover`, `html[data-theme] .card` → `css/ui/primitives/card.css`
- `.card-title`, `.unit-hint` → `css/ui/primitives/card.css`
- `.chart-container`, `.chart-container canvas` → `css/ui/primitives/card.css`
- `.tab`, `.tab:hover`, `.tab.active`, `@media .tab` → `css/ui/primitives/tabs.css`
- `.cs-subtab-bar`, `.cs-subtab`, `.cs-subtab.active/hover`, `.cs-subpanel`, `.cs-subpanel.active` → `css/ui/primitives/tabs.css`
- `.toast`, `.toast button/hover`, `.toast-warning/error/info` + `::before` → `css/ui/primitives/toast.css`
- `#toast-container` → `css/ui/primitives/toast.css`
- `@keyframes toast-in` → `css/ui/primitives/toast.css`
- `#settings-modal` → `css/ui/primitives/modal.css`
- `.settings-box`, `.settings-header`, `.settings-title`, `.settings-close-btn`, `.settings-section`, `.settings-label`, `.settings-scale-row`, `.settings-scale-val`, `.settings-presets`, `.settings-theme-grid` → `css/ui/primitives/modal.css`
- `.theme-swatch`, `.theme-swatch.active/hover`, `.theme-swatch-colors`, `.theme-swatch-name` → `css/ui/primitives/modal.css`
- `.settings-footer`, `.settings-footer button` + hover → `css/ui/primitives/modal.css`
- `.zoo-inject-btn`, `.zoo-inject-btn:hover/disabled` → `css/ui/primitives/button.css`
- `.qlab-btn-primary`, `.qlab-btn-secondary` + states → `css/ui/primitives/button.css`
- `.cs-slider-row`, `.cs-slider-row input[type=range]`, `.cs-slider-row label` → `css/ui/primitives/slider.css`

### Safe to remove after panels migrated

- `#tab-bar` + `#btn-panel-toggle` + `#btn-panel-toggle:hover` → `css/ui/components/workspace-tabs.css` (component file already exists)
- `#panel-area`, `#panel-area::-webkit-resizer`, `#app.panels-collapsed #panel-area` → `css/ui/components/panel-dock.css` (component file already exists)
- `#panel-resizer`, `#panel-resizer span`, `#panel-resizer:hover span` → `css/ui/components/panel-dock.css`
- `#app.panels-collapsed #tab-bar .tab`, `#app.panels-collapsed #tab-bar #btn-panel-toggle` → `css/ui/components/panel-dock.css`
- `.panel`, `.panel.active`, `.panel-grid`, `.panel-grid-2/3/4` → `css/ui/components/panel-dock.css` / `css/ui/panels/`
- `.panel-scale-header` → `css/ui/panels/` (scale-context header, used per-panel)
- `.viewport-overlay-panel`, `.viewport-overlay-bottom` → `css/ui/components/viewport-overlays.css`
- `.skip-link`, `.skip-link:focus` → `css/ui/shell/` (accessibility, not a panel but shell-level)
- `.insp-pos-input` → `css/ui/components/` (inspector component, move after inspector panel migration)
- `.inspector-empty`, `.inspector-grid`, `.inspector-grid dt/dd` → `css/ui/components/` (inspector)
- `.stat-value`, `.stat-unit`, `.stat-sparkline` → `css/ui/panels/` (stat display helpers)
- `#pe-telemetry` + `.pe-telem-*` block → `css/ui/scales/` (PE telemetry panel, Scale 1/1+)
- `.pe-conservation-row`, `.pe-cons-*` → `css/ui/scales/` (PE telemetry)
- `.pe-table-wrap`, `.pe-table`, `.pe-table th/td/tr` → `css/ui/scales/`
- `.pe-ts-label`, `.pe-ts-chart` → `css/ui/scales/`
- `.pe-insp-selected`, `.pe-insp-catalog-dot`, `.pe-insp-header`, `.pe-insp-name`, `.pe-insp-symbol` → `css/ui/scales/` (PE inspector)
- `.chart-title`, `.chart-legend`, `.chart-legend-item::before` → `css/ui/panels/` (chart helpers)
- `.const-table`, `.const-table th/td` → `css/ui/panels/` (constants table)
- `.zoo-table`, `.zoo-table th/td/tr`, `.zoo-cat-header`, `.zoo-dot`, `.zoo-symbol`, `.zoo-mass`, `.zoo-formula`, `.zoo-accuracy` → `css/ui/scales/` (particle zoo, Scale 1+)
- `.mode-unavailable` → `css/ui/panels/` (generic empty-state)
- `.sym-panel`, `.sym-panel-title`, `.sym-panel-label` → `css/ui/scales/` (floating symmetry panel)
- `.cs-theory-grid`, `.cs-theory-card`, `.cs-theory-title` → `css/ui/scales/` (reference frame context, Scale 4)
- `.cs-walkthrough-*` block → `css/ui/scales/` (reference frame context walkthrough)
- `.cs-info-btn`, `.cs-info-tooltip`, `.cs-info-tooltip.visible` → `css/ui/scales/` (reference frame context tooltips)
- `.cs-scenario-desc` → `css/ui/scales/`
- `#loading-overlay`, `#loading-overlay.hidden/removed` → `css/ui/components/loading-overlay.css` (component file already exists)
- `.load-lattice`, `.load-logo`, `.load-logo .letter`, `.load-logo .dot`, `.load-subtitle`, `.load-progress-wrap`, `.load-bar-bg`, `.load-bar-fill`, `.load-status`, `.load-version` → `css/ui/components/loading-overlay.css`
- `@keyframes letter-in` → `css/ui/components/loading-overlay.css`
- `#error-overlay`, `#error-overlay .error-box`, `#error-overlay h2/p/code` → `css/ui/shell/` (fatal error overlay)

### Must keep (global)

- None identified — all components.css rules are component/panel-scoped.

### Needs investigation

- `.settings-scale-row input[type=range]` — also touched by checkbox.css (accent-color). Decide canonical home: modal.css (contextual) vs checkbox.css (global reset). Recommend modal.css since it's settings-specific.
- `.panel-grid` — appears in both layout.css (responsive overrides) and components.css (base definition). Consolidate to one location (panel-dock.css or a `grid-helpers.css`).
- `.cs-theory-card` uses `.card` base-like styling but is reference frame context-specific — decide whether to extend `.card` or keep standalone.
- `.qlab-btn-primary/secondary` — currently in button.css (extracted), but Quantum Lab is a panel; may belong in `css/ui/scales/scale1/` instead of a global primitive.
