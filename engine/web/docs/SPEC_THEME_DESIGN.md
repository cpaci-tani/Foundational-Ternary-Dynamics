# Theme Design Specification

Status: `[SELECTION]` canonical design spec for the FTD web UI palette system
Version: 1.0 (2026-04-16)
Applies to: `engine/web/css/**`, all JS template strings in `engine/web/js/**`

---

## 1. Goals

1. **Every UI pixel is themeable.** No hardcoded color, shadow, or elevation that bypasses the token system.
2. **One theme file switches the whole app.** A theme override re-skins shell, panels, overlays, primitives, and scale-specific widgets without touching component CSS.
3. **Semantic over literal.** Components never pick a literal color; they pick a *role* (`--bg-card`, `--text-muted`, `--state-hover-bg`). Themes decide what the role looks like.
4. **Accessibility by default.** Every text/background pair in every theme passes WCAG 2.1 AA (≥4.5:1 for body, ≥3:1 for inert / non-text UI).
5. **Responsive to OS preferences.** Themes adapt to `prefers-color-scheme` and `prefers-contrast` when the user hasn't made an explicit choice.

---

## 2. Architecture

### 2.1 The two-region `:root` model

Every design token lives in one of two regions inside [tokens.css](../css/tokens.css):

- **Theme Override Surface** — tokens that themes **MUST** redefine. Defined first, reset by each `html[data-theme="*"]` selector.
- **Fixed System Tokens** — tokens themes inherit unchanged: geometry (radii, spacing, font sizes), motion (durations, easings), z-layers, legend colors, reference frame context slots.

```css
:root {
    /* ╔══════════════════════════════════════╗
       ║  THEME OVERRIDE SURFACE              ║
       ╚══════════════════════════════════════╝ */
    --bg-deep: ...; --bg-surface: ...; --bg-card: ...; ...
    --text-primary: ...; --text-secondary: ...; ...
    --accent: ...; --card-shadow: ...; --viewport-bg: ...; ...

    /* ╔══════════════════════════════════════╗
       ║  FIXED SYSTEM TOKENS                 ║
       ╚══════════════════════════════════════╝ */
    --radius-sm: 4px; --radius: 6px; --radius-md: 8px; ...
    --fs-xs: ...; --fs-sm: ...; --fs-base: ...; ...
    --dur-fast: 150ms; --ease-out: cubic-bezier(...);
}
```

### 2.2 Theme files

Four canonical themes live in [css/themes/](../css/themes/):

| Theme | Activation | Palette summary |
|---|---|---|
| **default** | `:root` (no attribute) | Cyan on deep navy — "Vibrant Physics" |
| **abyss** | `html[data-theme="abyss"]` | Indigo on pitch black — OLED-friendly |
| **nord** | `html[data-theme="nord"]` | Frost on slate gray — Nord-inspired cool |
| **light** | `html[data-theme="light"]` | Blue on white — daylight mode |
| **parchment** | `html[data-theme="parchment"]` | Gold on warm tan — sepia/print feel |

Each theme file **must** override **every token** in the Theme Override Surface. No exceptions.

### 2.3 Responsive triggers (auto-theming)

At the end of `tokens.css`:

1. **`@media (prefers-color-scheme: light)`** — when the user has no `data-theme` set and the OS prefers light, the app auto-adopts the light palette. User's explicit choice always wins via attribute specificity.
2. **`@media (prefers-contrast: more)`** — stacks on top of any active theme; swaps borders/muted text to `CanvasText` system color, thickens focus rings to 3px `Highlight`.

---

## 3. Token Catalog

### 3.1 Theme override surface (every theme must redefine)

#### Backgrounds — elevation ladder (z0 → z4)

| Token | Role | Usage |
|---|---|---|
| `--bg-deep` | z0 — void | `body`, viewport fallback |
| `--bg-surface` | z1 — primary chrome | toolbar, status bar, tab bar, viewport overlays, panel-dock, assistant sidebar |
| `--bg-card` | z2 — card | `.card`, toolbar group pills, panel cells |
| `--bg-input` | z2.5 — input surface | `<input>`, `<select>`, table rows, `.tb-select` |
| `--bg-elevated` | z3 — modal / dropdown | `#settings-modal`, toast, assistant-sidebar content |
| `--bg-raised` | z4 — tooltip | `.cs-info-tooltip`, overlay popovers |

Rule: a container at layer Z uses `--bg-Z`, and children use `--bg-(Z+1)` for perceptible contrast.

#### Borders

| Token | Role |
|---|---|
| `--border` | default 1px divider — card edge, input edge, toolbar pill edge |
| `--border-light` | hover / focus border — lifted card, active select |
| `--border-glow` | accent-tinted — inset shadow on active tab, card-glow |

#### Text hierarchy (contrast-tuned)

| Token | Role | Contrast target |
|---|---|---|
| `--text-primary` | headings, values, active labels | ≥7:1 on `--bg-card` (WCAG AAA) |
| `--text-secondary` | body, labels, status values | ≥4.5:1 AA |
| `--text-muted` | captions, hints, inactive legend | ≥4.5:1 AA |
| `--text-accent` | body-lite inside cards | ≥4.5:1 AA |
| `--text-dim` | disabled, inert | ≥3:1 (WCAG 1.4.11 non-text threshold) |

#### Accents

| Token | Role |
|---|---|
| `--accent` | primary interaction color — brand glyph, active tab text, focus ring, toggle active |
| `--accent-dim` | pressed / active button fill |
| `--accent-glow` | shadow color for active-button bloom |

#### Semantic status

| Token | Role |
|---|---|
| `--positive` | success — green in most themes, aurora-green in nord |
| `--negative` | error — red/rose |
| `--warning` | caution — amber/gold |

#### Interaction state tints

| Token | Role |
|---|---|
| `--state-hover-bg` | ghost hover background on tabs, chips, badges, diagnostics cells |
| `--state-active-bg` | selected-state background on tabs, theme swatches |
| `--state-focus-ring` | `box-shadow` rule for `:focus-visible` on custom widgets |

#### Surface effects

| Token | Role |
|---|---|
| `--viewport-bg` | 3D viewport backdrop gradient (set as theme-specific image) |
| `--card-shadow` | card `box-shadow` (none on dark themes, lifted on light) |
| `--card-hover-bg` | lifted-card background on hover |
| `--chrome-shadow` | toolbar / status bar drop shadow |
| `--dock-shadow` | panel-dock lift shadow |
| `--tabs-shadow` | workspace-tabs + viewport-controls-panel lift shadow |
| `--modal-backdrop` | behind-modal screen (handles focus-within-dark overlay) |

#### Tooltip surfaces

Tooltips float above the main chrome and need their own theme-coordinated palette so text reads clearly over the glass:

| Token | Role |
|---|---|
| `--tooltip-bg` | tooltip body background (opaque or near-opaque so text reads) |
| `--tooltip-border` | tooltip edge (accent-tinted per theme) |
| `--tooltip-text` | primary copy inside the tooltip |
| `--tooltip-shadow` | drop shadow for lift |
| `--tooltip-backdrop` | `backdrop-filter` value (blur + saturate) — kept as a fixed system token in `:root`; themes inherit |
| `--tooltip-kicker` | headline / label color (often theme accent) |
| `--tooltip-muted` | secondary copy inside the tooltip |

Rule: any tooltip component (`.cs-info-tooltip`, custom popovers, the FTD-assistant preview hover) reads from the `--tooltip-*` group — never duplicates the chrome tokens, because chrome is semi-transparent and tooltips must be opaque enough to read at arbitrary stacking depth.

### 3.2 Fixed system tokens (themes inherit)

#### Geometry

```css
--radius-sm: 4px;  --radius: 6px;  --radius-md: 8px;
--radius-lg: 12px; --radius-xl: 16px;

--sp-xs: calc(2px * var(--ui-scale));
--sp-sm: calc(6px * var(--ui-scale));
--sp-md: calc(10px * var(--ui-scale));
--sp-lg: calc(16px * var(--ui-scale));
--sp-xl: calc(20px * var(--ui-scale));
```

#### Typography

```css
--fs-xs: 12px;  --fs-sm: 13px;  --fs-base: 15px;
--fs-md: 16px;  --fs-lg: 17px;  --fs-xl: 20px;
--fs-2xl: 24px; --fs-3xl: 32px;  /* all multiply through --ui-scale */

--lh-tight: 1.25; --lh-base: 1.5; --lh-loose: 1.7;

--font-heading: 'Outfit', 'Inter', system-ui, sans-serif;
--font-body:    'Inter', system-ui, -apple-system, sans-serif;
--font-mono:    'JetBrains Mono', 'Fira Code', monospace;
```

#### Motion

```css
--dur-fast: 150ms;  --dur-base: 240ms;  --dur-slow: 360ms;
--ease-out: cubic-bezier(0.16, 1, 0.3, 1);
```

#### Shadows (elevation — not themed)

```css
--shadow-sm: 0 1px 3px  rgba(0,0,0,0.18);
--shadow-md: 0 4px 12px rgba(0,0,0,0.28);
--shadow-lg: 0 12px 32px rgba(0,0,0,0.38);
--shadow-xl: 0 20px 80px rgba(0,0,0,0.45);
```

Themeable shadows (`--chrome-shadow`, `--dock-shadow`, `--tabs-shadow`, `--card-shadow`) live in the override surface and use theme-appropriate opacity.

#### Z-layers — see [z-layers.css](../css/ui/shell/z-layers.css)

```css
--z-viewport: 1;  --z-field-overlay: 5;  --z-overlay: 10;
--z-panel: 90;  --z-toolbar: 100;  --z-tabs: 100; --z-status: 100;  --z-panel-toggle: 101;
--z-sidebar-backdrop: 149;  --z-sidebar: 150;
--z-toast: 9000;  --z-modal: 9500;
--z-loading: 9999;  --z-error: 10000;
```

#### Legend palette — data colors, constant across themes

```css
--legend-field-kinetic:  #66bb6a;  --legend-field-gradient: #26a69a;
--legend-bi:             #ef5350;  --legend-coupling:       #fb8c00;
--legend-velocity:       #fdd835;  --legend-gauss:          #42a5f5;
--legend-dissipation:    #78909c;
```

Rationale: these represent physical *terms* (Born-Infeld, coupling, Gauss). They must stay stable across themes so users learn the mapping.

#### Reference frame context slots — Scale 11 semantic colors

```css
--reference frame context-primary:   #00e5ff;
--reference frame context-secondary: #7c4dff;
--reference frame context-glow:      #00bcd4;
--reference frame context-gold:      #ffd700;
```

---

## 4. Component → Token Mapping Matrix

Use this table when writing or reviewing a component. Left column is the element class, right column is the canonical token for each styleable property.

### 4.1 Shell chrome

| Element | background | border | text | shadow |
|---|---|---|---|---|
| `#toolbar` | `--bg-surface` | `--border` (bottom) | `--text-primary` | `--chrome-shadow` |
| `#status-bar` | `--bg-surface` | `--border` (top) | `--text-muted` | — |
| `#tab-bar` | `--bg-surface` | `--border` | — | `--tabs-shadow` |
| `#panel-area` | `--bg-surface` | `--border-light` | `--text-primary` | `--dock-shadow` |
| `#viewport` | `--viewport-bg` | — | — | — |
| `#error-overlay` | `--modal-backdrop` | `--negative` | `--negative` | — |

### 4.2 Primitives

| Element | background | border | text | hover bg |
|---|---|---|---|---|
| `.tb-btn` | `--bg-input` | `--border` | `--text-secondary` | `--card-hover-bg` |
| `.tb-btn.active` | `--accent-dim` | `--accent` | `--text-primary` | — |
| `.tb-select` | `--bg-input` | `--border` | `--text-primary` | — |
| `.view-toggle` | `--bg-surface` | `--border` | `--text-secondary` | `--card-hover-bg` |
| `.view-toggle.active` | `--accent-dim` | `--accent` | `--text-primary` | — |
| `.style-btn` | `--bg-input` | `--border` | `--text-dim` | — |
| `.card` | `--bg-card` | `--border-light` | `--text-primary` | `--card-hover-bg` |
| `.card + :hover` | `--card-hover-bg` | `--border-light` | — | — |
| `.tab` | transparent | — | `--text-muted` | `--state-hover-bg` |
| `.tab.active` | `--state-active-bg` | — | `--accent` | — |
| `.tb-slider` track | `--bg-input` | `--border` (inset) | — | — |
| `.tb-slider` thumb | `--accent` | — | — | — |
| `input`, `textarea` | `--bg-input` | `--border` | `--text-primary` | — |

### 4.3 Components

| Element | background | border | text | shadow |
|---|---|---|---|---|
| `#viewport-overlay` buttons | `--bg-surface` | `--border` | `--text-secondary` | — |
| `#viewport-controls-panel` | `--bg-surface` | `--border-light` (top) | `--text-primary` | `--tabs-shadow` |
| `#settings-modal` backdrop | `--modal-backdrop` | — | — | — |
| `.settings-box` | `--bg-elevated` | `--border-light` | `--text-primary` | `--shadow-xl` |
| `.theme-swatch.active` | `--state-active-bg` | `--accent` | — | `--accent-glow` |
| `#assistant-sidebar` | `--bg-elevated` | `--border-light` | `--text-primary` | `--shadow-xl` |
| `#assistant-sidebar-backdrop` | `--modal-backdrop` | — | — | — |
| `.assistant-chip` | `--state-hover-bg` | `--border` | `--text-primary` | — |
| `#panel-resizer span` | `--border-light` | — | — | — |
| `.toast` | `--bg-elevated` | `--border-light` | `--text-primary` | `--shadow-lg` |

### 4.4 Panel interiors

| Element | background | text | notes |
|---|---|---|---|
| `.chart-container` | `--bg-card` | `--text-primary` | `--border` |
| `.chart-title` | — | `--text-secondary` | — |
| `.chart-legend-item` color swatch | `var(--color)` | — | color comes from `style="--color: var(--legend-*)"` |
| `.diag-badge` | `--state-hover-bg` | inherit | — |
| `.diag-badge-r/g/b` | rgba-tinted positive/negative/accent | `--positive`/`--negative`/`--accent` | semantic, tokenised |
| `.inspector-grid dt` | — | `--text-muted` | — |
| `.inspector-grid dd` | — | `--text-primary` | — |
| `.pe-table tr:hover` | `--state-hover-bg` | — | — |
| `.cs-theory-card` | `--bg-card` | `--text-primary` | `--border` |
| `.cs-theory-card.cs-highlight` | — | `--reference frame context-primary` | `0 0 12px var(--accent-glow)` |
| `.cs-subtab.active` | — | `--reference frame context-primary` | bottom border `--reference frame context-primary` |

### 4.5 Scale-specific widgets

| Element | background | text | notes |
|---|---|---|---|
| `.field-swatch-*` | hardcoded hex | — | data colors — match 3D shader constants, NOT themed |
| `.ae-legend-swatch-*` | hardcoded hex | — | nuclear / orbital data colors |
| `.cs-overlay-status` | — | `--reference frame context-primary` | — |
| `.scale4-overlay-status` | — | `--text-secondary` | — |
| `.sym-panel` (Scale 0 floating) | `--bg-surface` | inherit | border `--positive` |

Hardcoded data colors (field swatches, element legend) are intentional — they match 3D renderer shader constants and remain stable across themes.

---

## 5. Authoring Rules

### 5.1 When writing a new component

1. **Pick semantic tokens from the matrix in §4.** Don't invent new hex codes.
2. **Never use hardcoded rgba/hex except for:**
   - Legend/data colors that map to physical meaning (document the mapping)
   - Theme swatches in the settings modal (they show theme *previews*, not current state)
   - `:focus-visible` when `outline: 2px solid var(--accent)` is insufficient
3. **Prefer higher-level tokens over primitives.** Use `--card-shadow` instead of `--shadow-sm`. Use `--state-hover-bg` instead of `rgba(0,229,255,0.08)`.
4. **Test on all 4 themes before merging.** The spec doc has a verification checklist — see §7.
5. **Every text/background pair must pass WCAG contrast.** Validate with the browser-side contrast probe (see §7).

### 5.2 When adding a new token

Decide: is this **theme-variable** or **system-fixed**?

**Theme-variable** → add to the override surface in `tokens.css` `:root`, and **override in every theme file** (abyss, nord, light, parchment). Document the role in §3.1.

**System-fixed** → add to the fixed block. Never override per theme.

Rule: if any theme would want a different value, it goes in the override surface.

### 5.3 When adding a new theme

Create `css/themes/<name>.css` and define **every single token** listed in §3.1 under one selector:

```css
html[data-theme="<name>"] {
    /* Backgrounds */
    --bg-deep: ...; --bg-surface: ...; --bg-card: ...; --bg-input: ...;
    --bg-elevated: ...; --bg-raised: ...;

    /* Borders */
    --border: ...; --border-light: ...; --border-glow: ...;

    /* Text hierarchy */
    --text-primary: ...; --text-secondary: ...; --text-muted: ...;
    --text-accent: ...; --text-dim: ...;

    /* Accents */
    --accent: ...; --accent-dim: ...; --accent-glow: ...;

    /* Status */
    --positive: ...; --negative: ...; --warning: ...;

    /* State tints */
    --state-hover-bg: ...; --state-active-bg: ...; --state-focus-ring: ...;

    /* Surface effects */
    --viewport-bg: ...; --card-shadow: ...; --card-hover-bg: ...;
    --chrome-shadow: ...; --dock-shadow: ...; --tabs-shadow: ...;
    --modal-backdrop: ...;

    /* Tooltip surfaces */
    --tooltip-bg: ...; --tooltip-border: ...; --tooltip-text: ...;
    --tooltip-shadow: ...; --tooltip-kicker: ...; --tooltip-muted: ...;
}
```

Then:
1. Link the file in `index.html` (under the `<!-- themes -->` block)
2. Add a `.theme-swatch` entry to [settings-modal/template.js](../js/ui/components/settings-modal/template.js) plus a `.theme-swatch-colors[data-theme="<name>"]` block in [settings-modal.css](../css/ui/components/settings-modal.css)
3. If the new theme is light-on-dark vs dark-on-light, set `color-scheme: light` in the override for native form widgets (see light/parchment theme as reference)
4. Run the §7 contrast probe — fail any pair under 4.5:1 for text or 3:1 for dim/inert
5. Add the theme name to `prefers-color-scheme` / `prefers-contrast` blocks in `tokens.css` if it should auto-activate

### 5.4 Native form controls

Themes that have a light surface **must** set `color-scheme: light` in the theme root, so native `<select>` dropdown options, number-input spinners, and OS radio/checkbox widgets render with the correct UA chrome. Dark themes inherit the default `color-scheme: dark` from `tokens.css`.

---

## 6. Accessibility Contract

Every theme must satisfy:

| Pair | Minimum ratio |
|---|---|
| `--text-primary` on `--bg-card` / `--bg-surface` / `--bg-deep` | **4.5:1 AA** (target AAA 7:1) |
| `--text-secondary` on `--bg-card` | **4.5:1 AA** |
| `--text-muted` on `--bg-card` | **4.5:1 AA** |
| `--text-accent` on `--bg-card` | **4.5:1 AA** |
| `--text-dim` on `--bg-card` | **3:1** (WCAG 1.4.11 non-text; dim is inert by design) |
| `--accent` on `--bg-surface` | **3:1** (non-text UI component) |
| `--positive` / `--negative` / `--warning` on `--bg-card` | **3:1** (status indicators) |

Additionally:

- `:focus-visible` must be visible on every interactive element — rely on `outline: 2px solid var(--accent)` from `tokens.css` or `--state-focus-ring` box-shadow.
- `prefers-reduced-motion: reduce` disables all transitions/animations (handled in `tokens.css`).
- `prefers-contrast: more` swaps text + borders to `CanvasText` and thickens focus rings (handled in `tokens.css`).
- Text shadows on floating chrome (e.g. `.viewport-select-group label`) must use theme-appropriate opacity — don't hardcode dark-glass shadows on light themes.

---

## 7. Verification

### 7.1 Browser-side contrast probe

Paste into DevTools console while the app is loaded:

```js
(() => {
  const parse = (s) => {
    if (s.startsWith('#')) { const r=parseInt(s.slice(1,3),16), g=parseInt(s.slice(3,5),16), b=parseInt(s.slice(5,7),16); return {r,g,b,a:1}; }
    const m = s.match(/rgba?\(([^)]+)\)/); if(!m) return null;
    const p = m[1].split(',').map(x=>parseFloat(x.trim())); return {r:p[0],g:p[1],b:p[2],a:p[3]??1};
  };
  const lum = (r,g,b) => { const [R,G,B]=[r,g,b].map(v=>{v/=255;return v<=0.03928?v/12.92:Math.pow((v+0.055)/1.055,2.4);}); return 0.2126*R+0.7152*G+0.0722*B; };
  const blend = (f,b) => ({r:f.r*f.a+b.r*(1-f.a), g:f.g*f.a+b.g*(1-f.a), b:f.b*f.a+b.b*(1-f.a)});
  const ratio = (fg, bg) => { const L1=lum(fg.r,fg.g,fg.b), L2=lum(bg.r,bg.g,bg.b); return (Math.max(L1,L2)+0.05)/(Math.min(L1,L2)+0.05); };
  const h = document.documentElement;
  const style = document.createElement('style');
  style.textContent = '*, *::before, *::after { transition: none !important; }';
  document.head.appendChild(style);
  const results = {};
  for (const theme of ['default','abyss','nord','light','parchment']) {
    if (theme === 'default') h.removeAttribute('data-theme'); else h.setAttribute('data-theme', theme);
    void document.body.offsetHeight;
    const cs = getComputedStyle(h);
    const deep = parse(cs.getPropertyValue('--bg-deep').trim());
    const card0 = parse(cs.getPropertyValue('--bg-card').trim());
    const card = card0.a < 1 ? blend(card0, deep) : card0;
    results[theme] = {};
    for (const p of ['--text-primary','--text-secondary','--text-muted','--text-accent','--text-dim']) {
      const c0 = parse(cs.getPropertyValue(p).trim());
      const c = c0.a < 1 ? blend(c0, card) : c0;
      const r = ratio(c, card);
      const threshold = p === '--text-dim' ? 3 : 4.5;
      results[theme][p.slice(7)] = r.toFixed(2) + (r >= threshold ? ' ✓' : ' FAIL');
    }
  }
  h.removeAttribute('data-theme');
  style.remove();
  return results;
})();
```

Every cell must show `✓`. Any `FAIL` blocks merge.

### 7.2 Pre-merge checklist

Before merging a component or theme change:

- [ ] No `rgb(…)`, `rgba(…)`, or `#[0-9a-f]{3,8}` hex values in the diff (except for documented data-color cases)
- [ ] No `z-index: N` literal (use `--z-*` tokens)
- [ ] No `transition: … 0.3s cubic-bezier(…)` inline (use `var(--dur-base) var(--ease-out)`)
- [ ] No `!important` (except the 13 documented uses: `display:none` visibility overrides, `prefers-reduced-motion` accessibility, viewport bg themes)
- [ ] Component tested on default, abyss, nord, light, parchment at the breakpoints (360, 768, 1280, 1920)
- [ ] Contrast probe (§7.1) shows ✓ for every theme
- [ ] Focus ring visible on every interactive element
- [ ] Text readable against its actual background (including any backdrop-filter blur compositing)

### 7.3 Automated guard (optional future)

Add `scripts/lint/theme_check.py` that:
1. greps for hardcoded colors in `engine/web/css/ui/` and `engine/web/js/ui/` — fails if any unexpected match
2. verifies every theme file defines every token listed in §3.1
3. runs Playwright contrast probe against each theme — fails if any pair < threshold

---

## 8. Common Anti-Patterns (and Fixes)

### 8.1 "This needs to be darker on the viewport"

**Wrong:**
```css
#viewport-overlay .btn {
    background: rgba(17, 24, 39, 0.85) !important;
    color: #d1d5db !important;
}
```

**Right:**
```css
#viewport-overlay .btn {
    background: var(--bg-surface);
    color: var(--text-secondary);
    border: 1px solid var(--border);
    backdrop-filter: blur(12px);
}
```

The theme decides how to present `--bg-surface` against a dark 3D canvas (dark glass) vs a light canvas (light glass).

### 8.2 "Card needs a drop shadow on light themes only"

**Wrong:**
```css
html[data-theme="light"] .card,
html[data-theme="parchment"] .card {
    box-shadow: 0 1px 3px rgba(0,0,0,0.08);
}
```

**Right:**
```css
.card { box-shadow: var(--card-shadow); }
/* tokens.css: --card-shadow: none; */
/* light.css:  --card-shadow: 0 1px 3px rgba(0,0,0,0.08); */
```

Any time you write `html[data-theme=…] .foo`, you're leaking theme knowledge into a component file. Refactor by moving the variable to the theme instead.

### 8.3 "Hover state needs a subtle cyan tint"

**Wrong:**
```css
.card:hover { background: rgba(0, 229, 255, 0.08); }
```

**Right:**
```css
.card:hover { background: var(--state-hover-bg); }
```

`--state-hover-bg` is cyan on the default theme, blue on light, gold on parchment, etc. — always thematic.

### 8.4 "The shadow needs more opacity on dark themes"

**Wrong:** hardcode `rgba(0,0,0,0.7)` in `panel-dock.css`.

**Right:** use `--dock-shadow`, defined per theme (stronger on dark, softer on light).

### 8.5 "Hardcoded hex for this data series"

**Right:** use `--legend-*` tokens (fixed system tokens — stable across themes because they encode physical meaning, not aesthetic choice). If adding a new data series, add a new `--legend-<name>` token in the fixed block with a documented color mapping.

---

## 9. File Layout

```
engine/web/
├── css/
│   ├── tokens.css              — Canonical token catalog
│   │                             • Theme override surface (:root)
│   │                             • Fixed system tokens (:root)
│   │                             • prefers-color-scheme auto-activation
│   │                             • prefers-contrast accessibility boost
│   │                             • prefers-reduced-motion override
│   │
│   ├── themes/
│   │   ├── abyss.css           — Indigo on black
│   │   ├── nord.css            — Frost on slate
│   │   ├── light.css           — Blue on white
│   │   └── parchment.css       — Gold on tan
│   │
│   ├── scale-visibility.css    — Scale mode show/hide (not theme-related)
│   │
│   └── ui/
│       ├── shell/              — app-shell, regions, responsive, z-layers
│       ├── primitives/         — button, card, tabs, toggle, select, slider, …
│       ├── components/         — topbar, status-bar, panel-dock, modal, sidebar, …
│       ├── panels/             — one file per tab panel
│       └── scales/             — scale-specific widgets
│
└── docs/
    ├── SPEC_UI_REFACTOR.md     — Architecture history
    ├── historical/SPEC_CSS_REVAMP.md — Five-phase CSS refactor provenance
    └── SPEC_THEME_DESIGN.md    — THIS FILE
```

---

## 10. Version History

| Version | Date | Change |
|---|---|---|
| 1.0 | 2026-04-16 | Initial spec: two-region token model, 5-theme parity, WCAG AA validated |
