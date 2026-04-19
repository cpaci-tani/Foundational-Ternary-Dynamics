# FTD Palette Reference

Semantic color tokens for the web engine. Source of truth: `engine/web/css/tokens.css`. JSON export: `palette.json` in this directory.

## Philosophy

- **No raw hex in component CSS.** Every color maps to a `--token` in `tokens.css` so themes can re-skin without touching components.
- **Four themes** ship in `engine/web/css/themes/`: `abyss` (default), `nord`, `light`, `parchment`. Each re-defines the override-surface tokens.
- **Legend palette is literal.** The seven Lagrangian / consciousness term colors are fixed across themes — a red "Born-Infeld" curve should always be the same red.

## Background ladder (dark theme defaults)

| Token | Hex / rgba | Used for |
|---|---|---|
| `--bg-deep` | `#05080f` | viewport, body |
| `--bg-surface` | `rgba(10,15,26,0.45)` | toolbar, tabs, primary glass |
| `--bg-card` | `rgba(18,26,47,0.55)` | cards, panels |
| `--bg-input` | `rgba(25,36,60,0.65)` | inputs, selects |
| `--bg-elevated` | `rgba(25,36,60,0.75)` | modals, dropdowns, settings popover |
| `--bg-raised` | `rgba(35,50,80,0.85)` | tooltips, toasts |

## Text hierarchy

| Token | Hex / rgba | Used for |
|---|---|---|
| `--text-primary` | `#ffffff` | headings, values |
| `--text-secondary` | `rgba(255,255,255,0.82)` | body, labels |
| `--text-muted` | `rgba(255,255,255,0.75)` | captions, hints (AA on `--bg-card`) |
| `--text-accent` | `#e2e8f0` | body-lite on cards |
| `--text-dim` | `#94a3b8` | disabled, inert |

## Interaction accents

| Token | Hex | Used for |
|---|---|---|
| `--accent` | `#00e5ff` (cyan) | active states, focus rings, playheads |
| `--accent-dim` | `#0088aa` | primary buttons, active backgrounds |
| `--accent-glow` | `#00e5ff88` | hover halos, active-state glows |

## Semantic status

| Token | Hex | Meaning |
|---|---|---|
| `--positive` | `#34d399` | green — good / pass / render complete |
| `--negative` | `#fb7185` | pink — error / fail / warning state |
| `--warning` | `#fbbf24` | amber — scenario-override indicator, drift warning |

## Lagrangian / consciousness term legend (theme-invariant)

Seven colors, permanently assigned to Lagrangian density terms so charts stay consistent across themes:

| Token | Hex | Term |
|---|---|---|
| `--legend-field-kinetic` | `#66bb6a` | field kinetic term |
| `--legend-field-gradient` | `#26a69a` | field gradient term |
| `--legend-bi` | `#ef5350` | Born-Infeld core |
| `--legend-coupling` | `#fb8c00` | state-flux coupling |
| `--legend-velocity` | `#fdd835` | velocity term |
| `--legend-gauss` | `#42a5f5` | Gauss constraint penalty |
| `--legend-dissipation` | `#78909c` | dissipation term |

## Overlay swatches (scale-0 topology sheets)

Preview gradients shown in the overlay panel. Defined in `engine/web/css/ui/scales/scale0/toolbar.css`:

| Overlay | Gradient |
|---|---|
| `Φ potential` | `linear-gradient(90deg, #000033, #0066cc, #ffff00)` |
| `EM energy u` | `linear-gradient(90deg, #0d8c8c, #f9a133)` |
| `Charge ρ` | `linear-gradient(90deg, #2159d9, #f2f2f2, #e51a33)` |
| `Vorticity ω` | `linear-gradient(90deg, #070114, #7a0d87, #ffd933)` |
| `|ψ|²` | viridis: `#440154 → #21918c → #fde725` |
| `Phase φ` | full conic HSV wheel |
| `ℒ(x)` | diverging blue-red: `#2166ac → #f7f7f7 → #b2182b` |
| `Entropy s` | grayscale |

## Consciousness mode overrides

When Scale 11 is active, these tokens take over:

| Token | Hex |
|---|---|
| `--consciousness-primary` | `#00e5ff` |
| `--consciousness-secondary` | `#7c4dff` |
| `--consciousness-glow` | `#00bcd4` |
| `--consciousness-gold` | `#ffd700` |

## Rules

1. **Never hardcode hex in component CSS.** Use `var(--token)`.
2. **Themes re-skin by re-defining override-surface tokens.** See `engine/web/css/THEMING.md` §4.
3. **Legend colors are sacred.** Don't re-purpose a `--legend-*` token for something other than its term.
4. **New color needed?** Add a `--token` to `tokens.css` first, not a hex literal to a component.

## Cross-references

- Full theming policy: `engine/web/css/THEMING.md`
- Token source: `engine/web/css/tokens.css`
- Theme overrides: `engine/web/css/themes/*.css`
- Global spacing policy (margin/padding): `engine/web/css/THEMING.md` §5
