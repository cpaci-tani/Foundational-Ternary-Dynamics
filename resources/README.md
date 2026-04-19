# FTD Resources

Curated, high-signal goodies for working with Foundational Ternary Dynamics. Nothing here is authoritative — the canonical sources are still `scripts/constants.py`, `docs/theory/`, and `docs/SPEC_FTD.md`. The files in this directory are designed to make those sources **faster to navigate** and **easier to remember**.

## What's inside

| Path | Purpose |
|---|---|
| `cheatsheets/CONSTANTS.md` | One-page summary of every framework constant with its derivation hint |
| `cheatsheets/FORMULAS.md` | The key derivation chains laid out side-by-side |
| `cheatsheets/EPISTEMIC_TAGS.md` | Quick reference for `[AXIOM]` / `[THEOREM]` / `[SELECTION]` / etc. |
| `cheatsheets/WEB_ENGINE_SHORTCUTS.md` | Keyboard + mouse + scrub-bar shortcuts for the browser dashboard |
| `cheatsheets/ENGINE_TICK_CYCLE.md` | The six phases of a tick, one paragraph each |
| `cheatsheets/MOORE_NEIGHBORHOOD.md` | 26-cell polyhedral decomposition reference |
| `glossary/GLOSSARY.md` | FTD-specific vocabulary with one-line definitions |
| `data/constants.json` | Machine-readable canonical constants (safe to import) |
| `data/particle_masses.json` | Derived vs experimental masses for every particle with an FTD formula |
| `data/framework_integers.json` | The four integers {3, 4, 7, 13} with provenance |
| `palettes/PALETTE_REFERENCE.md` | Semantic color tokens with where they're used |
| `palettes/palette.json` | JSON export of the default theme palette |
| `scenarios/RECIPES.md` | Curated Scale-0 scenario walkthroughs with knob settings |
| `templates/DERIVATION_TEMPLATE.md` | Skeleton for a new derivation document |
| `templates/SCENARIO_TEMPLATE.md` | Skeleton for a new Scale-0 scenario |

## Rules

1. **Derived, not fabricated.** Every number in here traces back to `scripts/constants.py` or to a specific theorem in `docs/theory/`. If you see a value that drifted, open an issue — it's a bug.
2. **Short over comprehensive.** Cheatsheets are one page. Formulas appear once in their final form; the derivation lives in the theory docs.
3. **Epistemically tagged.** Claims inherit the tags of the source material. Parametric insertions are never sold as derivations.
4. **Single source of truth.** If a constant or formula lives in both `scripts/constants.py` and a resource file, the Python module wins.

## When to reach for this directory

- **Onboarding** a new contributor — hand them `cheatsheets/` and `glossary/`.
- **Writing a paper** — grab the derived-vs-experimental table from `data/particle_masses.json`.
- **Building a notebook** — import `data/constants.json` to avoid re-declaring values.
- **Running a demo** — follow a walkthrough in `scenarios/RECIPES.md`.
- **Starting a new derivation** — copy `templates/DERIVATION_TEMPLATE.md` into `docs/theory/03_derivations/`.
- **Picking something to work on** — skim `docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md` — every `[OPEN]` across engine code + theory in one file.

If a resource here turns out to be wrong, fix it, then fix whatever upstream source drifted. The point of the directory is to fail loudly when reality and the summary disagree.
