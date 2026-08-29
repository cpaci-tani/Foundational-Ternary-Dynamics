# CSS Maintainability Refactor Plan

**Status:** active, behavior-preserving implementation plan
**Scope:** `engine/web/css`, stylesheet loading in `engine/web/index.html`, and static presentation authored in web UI templates
**Primary constraint:** every batch must preserve the validated Scale 0 lifecycle, rendered theme contrast, and the Telemetry Grid's 60 FPS budget.

## 1. Settled baseline

The 2026-08-29 audit closed the immediate correctness and performance defects before this refactor begins:

- every authored and computed UI font is at least 16 CSS pixels;
- `transition: all` is prohibited and all current transitions name their properties;
- all UI `z-index` declarations use the canonical registry;
- Scale 0 visualization styling has one visual owner: `css/ui/scales/scale0/overlay-panel.css`;
- semantic foreground/on-fill tokens meet WCAG AA in Default, Abyss, Light, Nord, and Parchment;
- glass-off panel roots are opaque and glass filters are opt-in;
- theme changes are atomic and OS reduced-motion is honored;
- the Telemetry Grid has an absolute 16.9 ms p95 frame-time gate;
- static CSS architecture, rendered contrast, responsive geometry, panel races, and telemetry performance are covered by Playwright.

Current size baseline:

| Metric | Baseline |
|---|---:|
| CSS files | 70 |
| CSS lines | 10,443 |
| Static/dynamic `style="..."` occurrences in web JS/HTML | 253 |
| `!important` occurrences | 89 |
| Largest stylesheet | Scale 0 overlay, 718 lines |

These counts are inventory, not automatic defects. Visualization palettes, dynamic geometry, visibility arbitration, and accessibility overrides can legitimately remain local exceptions.

## 2. Target ownership model

Every rule must have one owner:

1. `tokens.css` and `themes/`: design values, semantic colors, glass/motion contracts, and global accessibility floors.
2. `ui/shell/`: application geometry, breakpoints, shell regions, and stacking layers.
3. `ui/primitives/`: reusable controls with no scale or panel knowledge.
4. `ui/components/`: one reusable component per file.
5. `ui/panels/`: panel-specific presentation only.
6. `ui/scales/<scale>/`: scale-owned interfaces and visualization palettes.
7. `ui/primitives/utilities.css`: a deliberately small allowlist of cross-owner utilities.

A selector must not be restated in a broader layer to patch a narrower owner. Shared geometry and scale-specific visuals are separate contracts, as established for `viewport-overlays.css` and the Scale 0 overlay owner.

## 3. Refactor phases

### R1 — Make stylesheet delivery declarative

Create a checked stylesheet manifest that records:

- critical versus deferred delivery;
- dependency tier and order;
- owning component or scale;
- cache revision;
- whether the file is permitted to define tokens, breakpoints, or z-layers.

Generate or validate the `<link>` and deferred-loader lists from that manifest. Do not change runtime order in the same batch that moves selectors.

**Exit gates:** no duplicate manifest entries, no empty loaded sheets, all local sheets load successfully, and the existing order assertions remain green.

### R2 — Formalize the token contract

Split token responsibilities without changing computed values:

- palette/surface tokens;
- foreground and on-fill roles;
- typography and spacing;
- motion and glass behavior;
- component-independent performance hints.

Add a static undefined-token check and require every named theme to implement the theme override surface. Literal colors remain allowed for scientific ramps and swatches, but not for ordinary text, controls, or panel roots.

**Exit gates:** zero undefined custom properties, all semantic pairs meet 4.5:1, and glass-off roots compute to alpha 1.

### R3 — Reduce large files by interface ownership

Move one interface at a time, in this order:

1. Scale 0 visualization command/header/search/active rail;
2. Scale 0 category and inline sheet controls;
3. Scale 0 floating toolbar panels and scientific swatches;
4. Settings sections;
5. play bar and panel-mount geometry;
6. diagnostics and meta pedagogy panels;
7. remaining files over 250 lines.

The Scale 0 overlay may use several owner files loaded consecutively, but shared selectors must not return to `viewport-overlays.css`. Avoid `@import` waterfalls; the delivery manifest owns order.

**Exit gates:** no authored stylesheet exceeds 400 lines, no selector has competing visual owners, and each moved interface passes its focused Playwright spec before the next interface moves.

### R4 — Retire static inline presentation

Classify all template `style` usage:

- move static layout, spacing, font, and color declarations into the owning stylesheet;
- keep dynamic values as narrowly scoped custom properties, such as `style="--series-color: ..."`;
- replace JS `element.style.display` state with `hidden`, `aria-expanded`, or a state attribute when the value is binary;
- keep measured geometry and canvas/SVG drawing values in JS.

Migrate one template/component per batch. Never combine this with controller logic changes.

**Exit gates:** static inline presentation is eliminated, remaining inline declarations are documented dynamic values, and CSP-ready templates contain no event-handler attributes.

### R5 — Make cascade precedence explicit

After R1–R4 have single ownership, introduce a declared layer order:

```css
@layer reset, tokens, shell, primitives, components, panels, scales, utilities, accessibility;
```

Layer migration must be atomic across authored sheets because unlayered rules outrank layered rules. Reduce `!important` only after layer order is active; retain an allowlist for visibility arbitration, reduced motion, focus mode, and third-party overrides.

**Exit gates:** `!important` is allowlisted and reduced to at most 35 declarations, specificity does not increase during migration, and all component states remain exact.

### R6 — Consolidate responsive policy

Define named breakpoint tokens in documentation and keep viewport breakpoints in the shell. Move component reshaping to container queries where the component is dockable, floatable, or resizable. Eliminate overlapping ranges and duplicated phone/tablet rules.

**Exit gates:** 767/768 and 1023/1024 boundaries have one owner, every public scale passes the width matrix, and no component creates horizontal overflow at its minimum supported container width.

### R7 — Make performance budgets permanent

Retain the existing telemetry and overlay scheduler gates, then add:

- CSS coverage snapshots for the audited interface matrix;
- a long-task count during chart updates and panel resize;
- layout-shift and forced-reflow counters for drag/resize/input bursts;
- a rule that permanent `will-change` is forbidden outside active interaction states;
- a visual screenshot matrix for the five themes at desktop, tablet, and phone widths.

**Exit gates:** Telemetry Grid p95 frame time is at most 16.9 ms, update p95 is at most 4 ms, update max is at most 10 ms, zero panel lifecycle leaks occur, and visual/contrast snapshots are clean.

## 4. Batch protocol

Each refactor batch must follow this sequence:

1. name one interface and its current/target owner files;
2. capture selector and runtime coverage for that interface;
3. move rules without changing values;
4. run Stylelint and `css-maintainability.spec.js`;
5. run the interface's focused Playwright spec;
6. run `theme-contrast.spec.js` if any color or surface rule moved;
7. run `responsive-overflow.spec.js` if geometry moved;
8. run the telemetry/overlay performance gate if frame-visible presentation moved;
9. update the manifest and this plan's progress table.

Do not start the next interface while any gate is red. Do not mix physics/controller changes into a CSS-owner batch.

## 5. Progress table

| Phase | State | Required proof |
|---|---|---|
| Immediate audit fixes | Complete | 36/36 focused Playwright checks; Stylelint clean |
| R1 delivery manifest | Pending | load/order/empty-sheet checks |
| R2 token contract | In progress | semantic contract and rendered contrast gates landed; undefined-token gate pending |
| R3 interface ownership | In progress | Scale 0 visualization owner consolidated; remaining large owners pending |
| R4 inline presentation | Pending | per-template inventory and focused specs |
| R5 cascade layers | Pending | atomic layer migration and `!important` allowlist |
| R6 responsive policy | Pending | width/container matrix |
| R7 permanent budgets | In progress | frame/contrast/lifecycle gates landed; coverage and visual matrix pending |

## 6. Completion criteria

The refactor is complete only when:

- each selector has an unambiguous owner;
- stylesheet order comes from one checked manifest;
- all themes satisfy the semantic token contract;
- all static template presentation is class-owned;
- responsive behavior is shell- or container-owned, never patched ad hoc;
- CSS architecture, contrast, font floor, overflow, lifecycle, and performance gates run in CI;
- the full focused gate remains green after each interface migration.
