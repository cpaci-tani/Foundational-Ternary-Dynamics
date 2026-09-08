# Comprehensive UI/UX + CSS Completion Audit

Status: `[SELECTION]` consolidated findings from 3 parallel audits (2026-04-16)
Scope: everything not already tracked in [SPEC_UI_REFACTOR.md](./SPEC_UI_REFACTOR.md) or [SPEC_CSS_REVAMP.md](./SPEC_CSS_REVAMP.md)

---

## Priority Matrix

| # | Finding | Priority | Type | Effort |
|---|---|---|---|---|
| 1 | `--text-muted` fails WCAG 4.5:1 contrast on dark themes | **P0** | a11y | 15 min |
| 2 | Scale 1 wiring still in `app.js` (10 listeners + 2 sliders + 1 btn) | **P1** | arch parity | 45 min |
| 3 | Scale 2 wiring still in `app.js` (6 listeners + 2 sliders + 1 btn; `SCALE2_TOGGLES` unused) | **P1** | arch parity | 45 min |
| 4 | Scale 4 has no `controls/` — 17 `planetary-*` DOM lookups inline in `app.js` | **P1** | arch parity | 1 hr |
| 5 | Scale 11 has no `controls/` — cs-scenario-select, cs-figure-select, cs-audio lookups in `app.js` | **P1** | arch parity | 45 min |
| 6 | Settings modal has no Escape-key handler / focus trap | **P1** | a11y | 20 min |
| 7 | `showToast()` referenced but implementation may be missing; silent failures on inject-full / clear-empty | **P1** | UX | 30 min |
| 8 | `meta-pedagogy.js` (48) + `ontic-observatory.js` (40) inline styles — mix of static + dynamic | **P2** | code quality | 1.5 hr |
| 9 | Tab → panel `aria-controls` association missing | **P2** | a11y | 15 min |
| 10 | 10 unused design tokens in `tokens.css` (`--bg-raised`, `--shadow-sm/lg`, `--sp-xs`, `--fs-3xl`, `--dur-slow`, `--reference frame context-glow/gold`, `--state-*` pair) | **P2** | hygiene | 10 min |
| 11 | Light/parchment themes don't override `--text-accent` / `--text-dim` — inherited dark-mode grays on white | **P2** | theme parity | 20 min |
| 12 | `scale23/toolbar.css` uses raw hex `#ff4444` etc. instead of `--legend-*` tokens | **P2** | token adoption | 10 min |
| 13 | No responsive `wide` layout in `responsive.css` — spec calls for multi-column at 1440+ | **P2** | responsive | 45 min |
| 14 | No keyboard shortcut help/discoverability (`?` key → cheat sheet) | **P2** | UX discovery | 1 hr |
| 15 | Panels have no loading/empty state placeholders before first data | **P2** | UX polish | 1 hr |
| 16 | Scale 12 missing `controller.js` + overlay template | **P3** | arch parity (meta mostly static) | — |
| 17 | Scale 5 `controls/` minimal (scenario-select only) — optional | **P3** | arch parity | — |
| 18 | `modal.css` (2L stub) + `layout.css` (23L retired stub) can be deleted once cache cycles | **P3** | hygiene | 5 min |
| 19 | Mobile 360px: field-swatch grids have no `overflow-x: auto` safety | **P3** | responsive edge | 15 min |
| 20 | Skip link never tested in Playwright | **P3** | a11y verification | 20 min |

---

## Cross-Audit Summary

### Architecture parity (Scale 0 is the reference)

| Scale | `controller.js` | `register-ui.js` | `toolbar/` | `overlays/template.js` | `controls/component.js` | `controls/wire.js` | `bindings.js` | `dom.js` | Wiring location |
|---|---|---|---|---|---|---|---|---|---|
| 0 |  |  |  |  |  |  |  |  | **owned by scale** |
| 1 |  |  |  |  |  |  |  |  | **app.js** |
| 2 |  |  |  |  |  |  |  |  | **app.js** |
| 3 |  |  |  |  | stub (inherits 2) | N/A |  |  | app.js (via scale 2) |
| 4 |  |  |  |  |  |  |  |  | **app.js (17 lookups)** |
| 5 |  |  |  |  |  |  |  |  | app.js (minimal) |
| 11 |  |  |  |  |  |  |  |  | **app.js** |
| 12 |  |  |  |  |  |  |  |  | minimal, mostly static |

Seven scales still need some form of migration; Scale 1 and Scale 2 are the highest-value next targets since `SCALE{1,2}_TOGGLES` config exists but isn't wired through, meaning toggle drift risk today.

### Accessibility

- **P0:** `--text-muted` contrast on `--bg-card` is ~3.2:1 (fails 4.5:1). Fix: `--text-muted: rgba(255,255,255,0.75)` or add `--text-muted-solid: #94a3b8`.
- **P1:** Settings modal has `role="dialog"` + `aria-modal="true"` but no `Escape` keydown handler and no focus trap. Users pressing Esc currently hit no-op.
- **P2:** Workspace tabs have `role="tab"` but no `aria-controls="panel-${id}"` linking tab → panel.
- **P3:** Skip link styling present; never Playwright-tested.

### Residual inline `style=`

After Phase B of CSS revamp: `js/ui/` 26 (mostly JS-driven display toggles + CSS var passthrough — acceptable); `js/scales/` 2 (both JS-driven display). **Remaining debt lives in legacy, non-component JS files:**

| File | Count | Static vs dynamic |
|---|---|---|
| `meta-pedagogy.js` | 48 | mostly static color hexes → extractable |
| `ontic-observatory.js` | 40 | ~25 static, ~15 data-driven |
| `aggregation-bridge.js` | 9 | static flex layout |
| `decay-rates.js` | 7 | table header decoration |
| `spectroscopy.js` | 4 | SVG + legend colors |
| `cross-sections.js` | 4 | grid + accent spans |
| `zoo.js` | 3 | already dynamic (cat colors) |

Net: ~100 removable by introducing `.ontic-obs-*`, `.meta-ped-*` classes and a shared `--legend-*` token map.

### CSS hygiene

- 10 dead tokens (declared, never referenced)
- 14 `!important` declarations — all validated as legitimate (visibility, theme viewport bg, `prefers-reduced-motion`)
- Zero unused class selectors (all referenced somewhere in HTML/JS)
- 1 local-stacking z-index literal (zoo table sticky header) — documented, OK

### Responsive coverage gaps

- `compact` (<768) 
- `tablet` (768–1023) 
- `desktop` (default) 
- `wide` (≥1440)  — spec describes multi-column panels and side-by-side inspector+diagnostics at this size but no CSS implements it
- 360–479 sm-compact: field-swatch button grids have no horizontal scroll fallback

---

## Proposed Execution Order

**Batch A (2.5 hr, all P0/P1):**
1. Fix `--text-muted` contrast (#1)
2. Settings modal Escape + focus trap (#6)
3. Scale 1 wire.js extraction (#2)
4. Scale 2 wire.js extraction (#3)

**Batch B (2 hr, P1 continued):**
5. Scale 4 controls migration (#4)
6. Scale 11 controls migration (#5)
7. Toast / silent-failure feedback (#7)

**Batch C (2 hr, P2 polish):**
8. `aria-controls` on tabs (#9)
9. Dead token removal (#10)
10. Theme parity for light/parchment (#11)
11. `scale23/toolbar.css` token adoption (#12)
12. Mobile overflow fallbacks (#19)
13. Delete `modal.css` + `layout.css` stubs (#18)

**Batch D (3 hr, P2 bigger):**
14. `meta-pedagogy` + `ontic-observatory` static-style extraction (#8)
15. Wide responsive layout (#13)
16. Keyboard shortcut help modal (#14)
17. Panel loading/empty states (#15)

**Deferred (P3):**
- Scale 5 / 12 full migration (minimal UI by design)
- Skip-link Playwright test (#20)

Total: ~9.5 hours for full completion.
