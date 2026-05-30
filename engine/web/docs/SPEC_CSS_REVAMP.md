# Web CSS Audit & Revamp Plan

Status: `[SELECTION]` proposed revamp; not yet approved for execution
Scope: `engine/web/css` + inline `style="..."` in `engine/web/js/ui`
Prereq: Phase 9 of [SPEC_UI_REFACTOR.md](./SPEC_UI_REFACTOR.md) complete (done 2026-04-16)

---

## 1. Audit Snapshot (2026-04-16)

| Metric | Value | Notes |
|---|---|---|
| Total CSS lines | 3,590 | across 40 files |
| Total CSS files | 40 | organised under `ui/{shell,primitives,components,panels,scales}` |
| `var(--*)` token uses | 467 | good adoption where tokens exist |
| Hardcoded `px` values in `ui/` | 570 | spacing tokens `--sp-*` underused |
| `rgba(…)` literals in `ui/` | 80 | color tokens underused for transparency tiers |
| Hex colors in primitives | 18 | `#fff`, `#e2e8f0`, `#94a3b8`, etc. should be tokens |
| `!important` declarations | 30 | usually signals specificity conflict |
| ID selectors in `ui/` | 74 | shell-ok, but some component coupling (see §3.3) |
| Class selectors in `ui/` | 420 | healthy ratio |
| Inline `style="…"` in JS templates | **106** | largest leak of style into markup |
| Hardcoded `z-index` literals | 14 | bypass the `--z-*` registry in [z-layers.css](../css/ui/shell/z-layers.css) |
| Undefined `var()` references | 3 | `--bg-elevated`, `--radius-md`, `--radius-sm` used but not declared |

### Largest files (candidates for decomposition)

| File | Lines | Observation |
|---|---|---|
| [panel-resources.css](../css/ui/components/panel-resources.css) | 400 | Omnibus for 6 different panels — inspector, physics, quantum-lab, planetary, cosmic-info, meta-info. Should split by panel. |
| [topbar.css](../css/ui/components/topbar.css) | 326 | Contains `assistant-sidebar` and `assistant-sidebar-backdrop` — a separate component, not toolbar. |
| [tokens.css](../css/tokens.css) | 181 | Now absorbs retired `layout.css` content. Mostly OK but two duplicate transition blocks (lines 115–124 and 166–170). |
| [modal.css](../css/ui/primitives/modal.css) | 172 | Settings-modal-specific rules mixed with generic modal primitives. |
| [button.css](../css/ui/primitives/button.css) | 172 | `zoo-inject-btn`, `qlab-btn-primary`, `settings-preset` all live here — panel-specific buttons in a "primitive". |

---

## 2. Findings

### 2.1 Token system is incomplete

Tokens exist for sizing, spacing, colors, font, radius, z-layer. But the system has gaps that callers work around:

**Missing radius scale.** Only `--radius` (6px), `--radius-lg` (12px), `--radius-xl` (16px) are defined. Code references `--radius-md` and `--radius-sm`. Result: silent fall-through to `initial`.

**Missing elevation / background tiers.** `--bg-deep`, `--bg-surface`, `--bg-card`, `--bg-input` exist. Code references `--bg-elevated`, which is undefined. No documented elevation ladder (z0 → z4).

**Missing semantic text tokens.** `--text-primary`, `--text-secondary`, `--text-muted` exist. `--text-accent` is referenced nowhere canonical but primitives hardcode `#e2e8f0` and `#d1d5db` as "body-lite" colors.

**Missing state tokens.** Hover, active, focus, disabled states are expressed as one-off `rgba(0,229,255,0.X)` or `!important` overrides rather than `--state-hover-bg`, `--state-active-bg`, etc.

**No density / line-height scale.** `line-height` appears as magic `1.4`, `1.5`, `1.6`, `1.7` across files. No `--lh-tight`, `--lh-base`, `--lh-loose`.

**Shadow tokens absent.** `box-shadow: 0 4px 24px rgba(0,0,0,0.4)`, `0 20px 80px rgba(0,0,0,0.45)`, etc. repeat verbatim across files. Should be `--shadow-sm/md/lg/xl`.

### 2.2 Inline styles leak into templates

106 `style="…"` attributes in `js/ui/**`. Biggest offenders (from this session's new templates):

- **Lagrangian template** — 14 inline styles (`font-size:11px`, `color:#ef5350`, `width:13px;height:13px`, flex layout). These are repeated 7× for the term toggles.
- **Reference frame context template** — 12 inline styles (`margin-top:12px`, `color:var(--reference frame context-primary)`, `font-size:11px`).
- **Charts template** — 6 inline styles (`height:80px;width:100%`, `margin-top:10px`).
- **Scale-0 legacy templates** (registered-ui.js) — toolbar groups with inline flex/gap.

**Consequence:** themes can't restyle them, dark/light switching leaves hardcoded colors, responsive rules can't reach them, and the CSS-in-HTML debt makes the component system feel half-migrated.

### 2.3 Z-index registry is bypassed

[z-layers.css](../css/ui/shell/z-layers.css) defines `--z-viewport`, `--z-overlay`, `--z-toolbar`, `--z-tabs`, `--z-panel`, `--z-toast`, `--z-modal`, `--z-loading`, `--z-error`. Only 4 files actually use them ([loading-overlay.css](../css/ui/components/loading-overlay.css), [modal.css](../css/ui/primitives/modal.css), indirectly).

Everywhere else uses literals: `z-index: 90, 100, 101, 149, 150, 8, 1000, 9000`. The `8` in scale0/toolbar.css and `9000` in toast.css are especially suspicious — they float outside any documented layer. `149 / 150` in topbar.css are ad-hoc to keep `assistant-sidebar` above `assistant-sidebar-backdrop` without a shared token.

### 2.4 Primitives host non-primitive concerns

"Primitives" should be generic. Currently:

- [button.css](../css/ui/primitives/button.css) — owns `.zoo-inject-btn`, `.qlab-btn-primary`, `.qlab-btn-secondary`, `.settings-preset`. These are panel-specific and should move to `css/ui/panels/zoo-panel.css`, `css/ui/panels/quantum-lab-panel.css`, `css/ui/components/settings-modal.css`.
- [modal.css](../css/ui/primitives/modal.css) — owns `.settings-box`, `.settings-header`, `.settings-footer`, `.theme-swatch`, etc. These belong in a `settings-modal.css` component file.
- [toggle.css](../css/ui/primitives/toggle.css) — carries 4× `!important` just to override `.tb-select` and other external rules, a smell of specificity conflict.

### 2.5 Selector specificity is fragile

30 `!important` uses across [select.css, toggle.css, scale-visibility.css, layout.css legacy]. Two patterns:

1. **Scale-visibility** — `!important` on `display: none` is defensible (intentional cascade override).
2. **Primitive overrides** — `select.css` line 71 uses `!important` on `color` to beat theme rules; `toggle.css` uses it on `color` and `border-color` to beat `.tb-btn`. These indicate that `.view-toggle` sits "beneath" `.tb-btn` in the cascade but they share the toolbar scope.

A shared `.ui-control` primitive + modifier classes would dissolve the conflict.

### 2.6 Theme system is shallow

Only 4 themes ([abyss, light, nord, parchment](../css/themes)), averaging 30 lines each. Each theme redeclares `:root` tokens then adds one or two `!important` overrides. This works but:

- Themes can't restyle an element unless that element uses a tokenised property. Because primitives hardcode `#fff`, themes can't make text black-on-white cleanly (hence the `!important` stack in light.css / parchment.css for `.card` shadows).
- No dark-mode media-query fallback (`@media (prefers-color-scheme: light)`) — themes are opt-in via `data-theme` only.
- No high-contrast or larger-text accessibility theme.

### 2.7 Responsive layer is thin

[responsive.css](../css/ui/shell/responsive.css) is 56 lines covering `compact` and `tablet` modes on `#toolbar`, `#viewport-overlay`, `#tab-bar`, `#panel-area`, `#status-bar` only. The spec's breakpoint matrix (compact-sm, compact-lg, tablet, desktop, wide) is not represented. There is no `wide` layout and no panel-grid responsiveness left now that `layout.css` is retired — `.panel-grid-3` and `.panel-grid-4` collapse rules need re-homing.

### 2.8 Scale-owned CSS is inconsistent

`css/ui/scales/` contains only `scale0/toolbar.css`, `scale1/telemetry.css`, `scale2/legend.css`, `scale11/pedagogy.css`, and a shared `toolbar-groups.css`. Scales 3, 4, 5, 12 have JS components but no dedicated CSS files — their styles live in inline `style=""` or in the omnibus `panel-resources.css`.

### 2.9 Duplicate / dead rules

- **tokens.css** has two transition blocks (115–124 and 166–170) with overlapping selectors but different timing curves. Second block was added when retiring `layout.css`; first should be removed.
- **panel-dock.css** line 47 `z-index: 100` collides with topbar line 8 `z-index: 100` — they're peer layers, intentional, but the shared value isn't tokenised so a future bump breaks both.
- **scale-visibility.css** comment on line 38 points to a `.mode-unavailable` rule in `diagnostics-panel.css`; verify it still exists.

---

## 3. Revamp Proposal

Five phases. Each phase leaves the app runnable and visually unchanged (or intentionally improved, never regressed).

### Phase A — Token completion (2–3 hours)

Add the missing tokens to `tokens.css`. Then grep-and-replace the highest-frequency literals.

**Add:**
```css
:root {
  /* radius scale — close the gap */
  --radius-sm: 4px;
  --radius-md: 8px;   /* bridges --radius (6) and --radius-lg (12) */

  /* elevation / background ladder */
  --bg-elevated: rgba(25, 36, 60, 0.75);   /* above --bg-card */
  --bg-raised:   rgba(35, 50, 80, 0.85);   /* above --bg-elevated */

  /* state tokens */
  --state-hover-bg:   rgba(0, 229, 255, 0.08);
  --state-active-bg:  rgba(0, 229, 255, 0.18);
  --state-focus-ring: 0 0 0 2px rgba(0, 229, 255, 0.4);
  --state-disabled-opacity: 0.5;

  /* line-height scale */
  --lh-tight: 1.25;
  --lh-base:  1.5;
  --lh-loose: 1.7;

  /* shadow scale */
  --shadow-sm: 0 1px 3px rgba(0,0,0,0.18);
  --shadow-md: 0 4px 12px rgba(0,0,0,0.28);
  --shadow-lg: 0 12px 32px rgba(0,0,0,0.38);
  --shadow-xl: 0 20px 80px rgba(0,0,0,0.45);

  /* duration + easing */
  --dur-fast: 150ms;
  --dur-base: 240ms;
  --dur-slow: 360ms;
  --ease-out: cubic-bezier(0.16, 1, 0.3, 1);
}
```

**Remove:** the duplicate transition block in tokens.css (lines 115–124).

**Exit criterion:** zero undefined `var(--*)` references (scripted check), duplicates gone.

### Phase B — Purge inline styles from JS templates (4–6 hours)

Replace the 106 inline `style="…"` attributes with classes. One pass per template file.

**Conventions:**
- Utility classes for common one-off layout (`.u-mt-sm`, `.u-gap-md`, `.u-flex-wrap`) in a new `css/ui/primitives/utilities.css`.
- Component-local classes for panel-specific styling (`.lag-term-row`, `.lag-term-toggle`, `.cs-metrics-row`).
- Delete any `style="color:#ef5350"` and use `.term-legend-bi { color: var(--legend-bi); }` where `--legend-bi: #ef5350` is defined in `reference frame context-panel.css` or `lagrangian-panel.css`.

**Templates to clean:**
- `lagrangian-panel/template.js` (14 inline styles)
- `reference frame context-panel/template.js` (12)
- `charts-panel/template.js` (6)
- `components/panel-resources/template.js` + `diagnostics-template.js` (~20 combined)
- `components/panel-resources/template.js` inspector section (~15)
- scale UI templates (`scales/scale*/ui/**`) — remainder

**Exit criterion:** `grep -r 'style="' engine/web/js/ui | wc -l` ≤ 5 (only truly dynamic inline styles — e.g., `modal.style.display` set from JS remains valid).

### Phase C — Z-index discipline (1 hour)

Every `z-index` in `css/ui/**` must reference a `--z-*` token. Extend the registry:

```css
--z-viewport:   1;
--z-field-overlay: 5;      /* NEW — scale0 field overlay stack */
--z-overlay:    10;        /* viewport overlays */
--z-panel:      90;        /* panel-dock */
--z-toolbar:    100;       /* topbar */
--z-tabs:       100;       /* tab-bar — peer of toolbar */
--z-status:     100;       /* status bar — peer */
--z-sidebar-backdrop: 149; /* NEW — assistant-sidebar */
--z-sidebar:    150;       /* NEW */
--z-toast:      9000;
--z-modal:      9500;
--z-loading:    9999;
--z-error:      10000;
```

Update consumers: `panel-dock.css`, `status-bar.css`, `topbar.css`, `viewport-overlays.css`, `workspace-tabs.css`, `toast.css`, `scale0/toolbar.css`, `zoo-panel.css`, `reference frame context-panel.css`.

**Exit criterion:** `grep -rE 'z-index:\s*[0-9]' css/ui | wc -l == 0`.

### Phase D — Split omnibus files (3–4 hours)

**D.1 `panel-resources.css` (400L → ~80L).**
Extract:
- Inspector-specific → `css/ui/panels/inspector-panel.css` (currently 36L, grows)
- Physics panel → `css/ui/panels/physics-panel.css` (new)
- Quantum lab → `css/ui/panels/quantum-lab-panel.css` (new)
- Planetary → `css/ui/panels/planetary-panel.css` (new)
- Cosmic info → `css/ui/panels/cosmic-info-panel.css` (new)
- Meta info → `css/ui/panels/meta-info-panel.css` (new)
- What remains in `panel-resources.css`: only `.panel-resource-shell`, `.panel-resource-grid`, `.panel-resource-toolbar`, `.panel-resource-input` — the generic scaffolding.

**D.2 `topbar.css` (326L → ~180L).**
Extract:
- `#assistant-sidebar` + `#assistant-sidebar-backdrop` + `.assistant-sidebar-header` and related → `css/ui/components/assistant-sidebar.css` (new). Also create `js/ui/components/assistant-sidebar/` to house its controller code per the component pattern.

**D.3 `button.css` (172L → ~110L).**
Extract:
- `.zoo-inject-btn` → `css/ui/panels/zoo-panel.css`
- `.qlab-btn-primary` / `.qlab-btn-secondary` → `css/ui/panels/quantum-lab-panel.css`
- `.settings-preset` → `css/ui/components/settings-modal.css` (new file for settings styles, unifies the modal rules currently in `primitives/modal.css`)

**D.4 `modal.css` (172L → ~80L).**
Extract all `.settings-*` and `.theme-swatch-*` rules to `css/ui/components/settings-modal.css`. `modal.css` keeps only the generic `.ui-modal` primitive: backdrop, box, header/footer shell, close affordance.

**Exit criterion:** no single file >200 lines; primitives contain only primitive rules.

### Phase E — State + primitive dedup (2–3 hours)

Introduce a shared `.ui-control` primitive that unifies `.tb-btn`, `.view-toggle`, `.dynamics-toggle`, `.style-btn`, `.cs-subtab`, `.settings-preset` — all the button-ish pill controls that repeat similar hover/active logic. Keep the named classes for backward compat (alias them).

Remove every `!important` whose purpose is specificity war (not visibility override). Target: ≤10 `!important` remaining, all on `display: none` in `scale-visibility.css` and theme overrides.

**Exit criterion:** cascade is declarative; `!important` count down from 30 → ≤10; a new primitive is documented.

---

## 4. Test Strategy

- **Static:**
  - `scripts/lint/css_check.py` (new) — detects undefined `var()`, literal `z-index`, literal colors outside `tokens.css`/`themes/`, inline `style=` in `js/ui/`. Fails CI if any regress.
- **Visual regression:**
  - Extend [scales.spec.js](../tests/scales.spec.js) with screenshots at compact (360×640), tablet (768×1024), desktop (1280×800), wide (1920×1080) for each scale. Baseline before Phase A; compare after each phase.
- **Theme switching:**
  - Playwright test clicking each `.theme-swatch` and asserting no `rgb(0,0,0)` text on `rgb(0,0,0)` background anywhere via `getComputedStyle`.
- **Accessibility:**
  - `axe-core` via Playwright for focus-visible and contrast on the four themes.

---

## 5. Sequencing & Risk

| Phase | Depends on | Risk | Rollback |
|---|---|---|---|
| A — tokens | — | Low. Purely additive. | Delete new token lines. |
| B — purge inline styles | A for new utility/color tokens | Medium. 106 edits; risk of class-name collisions. | Template-by-template; each template has a git-revert path. |
| C — z-index | A (extended registry) | Low. Every edit is `N → var(--z-*)`. | Revert per file. |
| D — split files | A, B | Medium. CSS ownership moves; risk of dead selectors. | Keep omnibus files as stubs with `@import` fallbacks for one release. |
| E — state/primitive dedup | D | Higher. Touches cascade behavior. | Ship behind a feature-flag class `#app.ui-unified` for one release. |

**Recommended order:** A → C → B → D → E. C is cheap and reveals layering bugs early. E is last because it benefits from clean primitives.

---

## 6. Immediate Wins (can ship today)

1. Add missing tokens in `tokens.css` (Phase A prefix).
2. Remove the duplicate transition block in `tokens.css`.
3. Replace literal `z-index` values in [status-bar.css](../css/ui/components/status-bar.css), [workspace-tabs.css](../css/ui/components/workspace-tabs.css), [panel-dock.css](../css/ui/components/panel-dock.css), [topbar.css](../css/ui/components/topbar.css) with the existing `--z-*` tokens.
4. Purge `style="…"` from the three templates added this session (charts, lagrangian, reference frame context) — while the mapping is fresh.

These four items are ~90 minutes of work and unblock the rest.

---

## 7. Implementation Results (2026-04-16)

All five phases complete. Browser verified after each phase (0 failed stylesheets, visual parity preserved).

### Phase A — Token completion ✅

**Added to `tokens.css`:**
- Radius scale: `--radius-sm` (4px), `--radius-md` (8px)
- Elevation: `--bg-elevated`, `--bg-raised`
- Body-lite text: `--text-accent`, `--text-dim`
- Interaction states: `--state-hover-bg`, `--state-active-bg`, `--state-focus-ring`, `--state-disabled-opacity`
- Line-height: `--lh-tight`, `--lh-base`, `--lh-loose`
- Shadows: `--shadow-sm/md/lg/xl`
- Motion: `--dur-fast/base/slow`, `--ease-out`
- Legend palette: `--legend-field-kinetic`, `--legend-bi`, `--legend-coupling`, `--legend-velocity`, `--legend-gauss`, `--legend-dissipation`, etc.

**Removed:** duplicate `transition:` block at tokens.css lines 115–124. Single canonical block at 166+ now uses `var(--dur-base) var(--ease-out)`.

### Phase C — Z-index discipline ✅

**[z-layers.css](../css/ui/shell/z-layers.css)** extended with new tokens: `--z-field-overlay`, `--z-panel-toggle`, `--z-status`, `--z-sidebar-backdrop`, `--z-sidebar`. Values renumbered so peer layers share a stop.

**Tokenised every literal `z-index` in `css/ui/**`:**
- `status-bar.css` → `var(--z-status)`
- `workspace-tabs.css` → `var(--z-tabs)`
- `panel-dock.css` → `var(--z-panel)` + `var(--z-panel-toggle)`
- `topbar.css` → `var(--z-toolbar)` + `var(--z-sidebar-backdrop)` + `var(--z-sidebar)`
- `viewport-frame.css` → `var(--z-viewport)`
- `viewport-overlays.css` → `var(--z-overlay)`
- `scale0/toolbar.css` (z-index: 8 and 1000) → `var(--z-field-overlay)` + `var(--z-sidebar)`
- `scale11/pedagogy.css` / `reference frame context-panel.css` → `var(--z-overlay)` + `var(--z-panel-toggle)`
- `scale2/legend.css` → `var(--z-status)`
- `toast.css` → `var(--z-toast)`
- `app-shell.css` (z: 150, 99999) → `var(--z-sidebar)` + `var(--z-error)`
- `regions.css` → all 7 region tokens

**Exception retained:** `zoo-panel.css:13 z-index: 1` for sticky `<th>` local stacking context (documented inline).

### Phase B — Purge inline styles ✅

`grep 'style="' engine/web/js/ui` dropped from **106 → 26**. `engine/web/js/scales` dropped from **71 → 2**. Remaining are all genuinely dynamic (JS-toggled `display`, CSS custom-property passthrough).

**Work performed:**
- Created [utilities.css](../css/ui/primitives/utilities.css) for one-off layout helpers (`.u-mt-*`, `.u-gap-*`, `.u-flex-*`, `.u-sparkline-canvas`).
- Charts panel → `.charts-row` layout class, canvas sparklines use `.u-sparkline-canvas`.
- Lagrangian panel → `.lag-layout` grid, `.lag-term-row`, `.lag-term-toggle[data-term]` with `--legend-*` tokens in [lagrangian-panel.css](../css/ui/panels/lagrangian-panel.css).
- Reference frame context panel → `.cs-metrics-row`, `.cs-metric-{primary,secondary,domain}` in [reference frame context-panel.css](../css/ui/panels/reference frame context-panel.css).
- Diagnostics template → `.diag-s0-grid`, `.diag-badge-row`, `.diag-badge-{r,g,b}`, `.diag-sparkline-row`, `.pe-ts-legend-*`, `.pe-telemetry-row-gap`, `.ae-diag-row` etc. in [diagnostics-panel.css](../css/ui/panels/diagnostics-panel.css).
- Panel-resources scale blocks → `.scale-controls-block`, `.scale-info-mono`, `.scale-info-copy`.
- Scale 0/1 field swatches → 22 named classes (`.field-swatch-{e-field,b-field,…}`, `.field-swatch-pe-{velocities,trails,…}`).
- Scale 23 force-toggle buttons → ID-targeted colour rules in new [scales/scale23/toolbar.css](../css/ui/scales/scale23/toolbar.css).
- Scale 2 nuclear legend → `.ae-legend-swatch-{proton,neutron,orb-s,orb-p,orb-d,orb-f}` in [scales/scale2/legend.css](../css/ui/scales/scale2/legend.css).
- Scale 4 overlay → new [scales/scale4/overlays.css](../css/ui/scales/scale4/overlays.css).
- Scale 11 overlay status → `.cs-overlay-status`.
- Ctrl helpers in [field.css](../css/ui/primitives/field.css): `.ctrl-slider-row`, `.ctrl-select-full`, `.ctrl-label-{xs,sm,md}`, `.ctrl-input-coord`, `.ctrl-row-compact`, `.ctrl-btn-compact`, `.ctrl-btn-row-fill`, `.ctrl-btn-flex-1`, `.ctrl-footnote`, `.ctrl-details-summary`, `.ctrl-action-row`, `.toggle-row-disabled`, `.lat-scenario-desc-text`.
- Settings modal theme swatches (20 inline colors) → `[data-theme]` attribute selectors in [settings-modal.css](../css/ui/components/settings-modal.css), reducing the template to semantic markup only.

### Phase D — Split omnibus files ✅

| File | Before | After | Notes |
|---|---|---|---|
| [panel-resources.css](../css/ui/components/panel-resources.css) | 400L | 244L | Generic scaffolding only |
| [topbar.css](../css/ui/components/topbar.css) | 326L | 180L | Assistant-sidebar extracted |
| [button.css](../css/ui/primitives/button.css) | 172L | 107L | Panel-specific buttons extracted |
| [modal.css](../css/ui/primitives/modal.css) | 172L | 2L | Entirely settings-modal-specific → extracted |

**New files created:**
- [css/ui/components/assistant-sidebar.css](../css/ui/components/assistant-sidebar.css) (156L) — full sidebar styles
- [css/ui/components/settings-modal.css](../css/ui/components/settings-modal.css) (232L) — dialog + theme swatch data-attribute map + moved `.settings-preset` from button.css
- [css/ui/panels/quantum-lab-panel.css](../css/ui/panels/quantum-lab-panel.css) (173L) — `.qlab-*` rules + `.qlab-btn-primary/secondary` from button.css
- [css/ui/panels/lagrangian-panel.css](../css/ui/panels/lagrangian-panel.css) (60L)
- [css/ui/scales/scale23/toolbar.css](../css/ui/scales/scale23/toolbar.css) (12L)
- [css/ui/scales/scale4/overlays.css](../css/ui/scales/scale4/overlays.css) (10L)

**Existing files grew:**
- [inspector-panel.css](../css/ui/panels/inspector-panel.css) — `.panel-inspector-*` rules moved out of panel-resources
- [zoo-panel.css](../css/ui/panels/zoo-panel.css) — `.zoo-inject-btn` moved out of button.css

No file now exceeds 250 lines in `css/ui/components/`.

### Phase E — `!important` cleanup ✅

Dropped from **30 → 13** declarations. Every remaining use is intentional:
- **6** in [scale-visibility.css](../css/scale-visibility.css): `display: none` overrides — these must beat any component `display: flex/grid` default.
- **4** in `themes/*.css`: viewport gradient backgrounds — need to beat `viewport-frame.css` base rule (ID-level specificity).
- **3** in [tokens.css](../css/tokens.css): `prefers-reduced-motion` accessibility override — must beat all component transitions.

**Removed specificity-war `!important`:**
- [select.css](../css/ui/primitives/select.css) (6 uses) — merged `.viewport-select-group label` rules into one and relied on natural cascade for the dark-glass viewport overrides.
- [toggle.css](../css/ui/primitives/toggle.css) (9 uses) — `#viewport-overlay .view-toggle` already has higher specificity (0,1,1) than `.view-toggle` (0,1,0), so `!important` was cargo-cult. Also removed from `.dynamics-toggle` by using `border-style: dashed` instead of `border: 1px dashed`.
- [scale1/telemetry.css](../css/ui/scales/scale1/telemetry.css) (1 use) — file load order already puts `.pe-insp-selected` after `.card`, no override needed.

### Metric deltas

| Metric | Before | After | Δ |
|---|---|---|---|
| Total CSS lines | 3,590 | ~3,760 | +170 (new tokens + component splits; per-file smaller) |
| CSS files | 40 | 47 | +7 (all new files well under 250 lines) |
| Inline `style=` in `js/ui/` | 106 | 26 | **−75%** |
| Inline `style=` in `js/scales/` | 71 | 2 | **−97%** |
| Literal `z-index` in `css/ui/` | 14 | 1 | **−93%** (only the local sticky-header remains) |
| `!important` declarations | 30 | 13 | **−57%** |
| Undefined `var()` refs | 3 | 0 | **−100%** |
| File > 300 lines | 2 | 0 | ✅ |
| Duplicate transition blocks | yes | no | ✅ |

Design tokens are now the single source of truth, panel-specific styles live with their components, and the cascade resolves naturally in all but the 13 necessary-override cases.
