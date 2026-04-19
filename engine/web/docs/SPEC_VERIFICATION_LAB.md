# Verify Panel (formerly Verification Lab)

**Status:** current implementation spec
**Version:** 2.0 — 2026-04-18
**Supersedes:** Version 1.0 (21-experiment Monte Carlo model, retired)
**Design doc:** `docs/superpowers/specs/2026-04-18-verify-panel-redesign-design.md`

## What the Verify tab is

A static evidence scoreboard. Each row poses a falsifiable question and shows
two numbers beneath it: FTD's value and nature's best measurement. Three
tiers make the epistemic status of each row unambiguous:

1. **Hard predictions** — THEOREM or SELECTION derivations. The row lists the
   inputs that went into FTD's value. A pull strip shows the deviation in σ
   of the measurement.
2. **Parametric insertions** — SM formula with FTD numbers plugged in.
   Agreement here is bookkeeping, not a prediction about nature. No pull
   strip; relative error only.
3. **Measurements with no FTD claim** — listed explicitly so that the gaps
   in FTD's coverage are visible instead of hidden.

No PASS/FAIL badges. No verdict colors. The reader reads the numbers and
draws their own conclusion.

## What the Verify tab is not

- Not a test runner. Running tests is the job of `ctest` and `pytest`.
- Not a live simulation. The tab never advances the engine.
- Not a marketing surface. Largest tensions sit above the fold.

## Data flow

```
scripts/constants.py  +  engine/web/data/measurements.json
                    \      /
                     \    /
        scripts/proofs/build_verify_manifest.py
                        |
                        v
         engine/web/data/verify-manifest.json  (committed)
                        |
                        v
         engine/web/js/verify-panel/  (pure render)
```

Regenerate the manifest after changing constants:

```
python -m scripts.proofs.build_verify_manifest
```

## Adding a new row

1. Add a measurement entry to `engine/web/data/measurements.json` with a
   citation URL and access date.
2. Add the matching FTD row to `_ftd_rows_from_constants()` in
   `scripts/proofs/build_verify_manifest.py`. Pick a tier; supply every
   field that tier requires (see the `TierAssertionError` messages —
   they tell you exactly what is missing).
3. Rebuild the manifest, commit the JSON, and the row appears on the tab.

## Tier contracts

Enforced as assertions in the builder:

- `hard` requires `inputs_used` and `theory_ref`.
- `parametric` requires `formula_source: "SM"` and `ftd_inputs`.
- `unpredicted` must **not** carry an `ftd_value`.

A build that violates any contract fails loudly; a silent half-built
manifest is never produced.

## File layout

```
engine/web/
├── data/
│   ├── measurements.json        — curated modern measurements (committed)
│   └── verify-manifest.json     — build output; consumed by the panel
├── js/verify-panel/
│   ├── component.js             — fetches manifest, mounts, renders
│   ├── template.js              — top-level panel skeleton
│   ├── header.js                — build stamp, counts, largest tensions
│   ├── tier.js                  — renders one of the three tiers
│   ├── row.js                   — renders a single evidence row
│   └── pull-strip.js            — inline σ-deviation SVG
├── css/ui/panels/
│   └── verify-panel.css         — three-tier visual language
└── tests/
    └── verify-panel.spec.js     — 6-test Playwright integration suite
```

## Test coverage

The Playwright suite at `engine/web/tests/verify-panel.spec.js` asserts:

- Header renders with FTD version, build stamp, and tier counts.
- Exactly three tier sections render (`hard`, `parametric`, `unpredicted`).
- Hard-tier rows show a pull strip; parametric-tier rows do not.
- Filter pills hide non-matching tiers.
- **No PASS / FAIL / CLOSE strings appear anywhere in the panel** — the
  honesty contract, enforced in the DOM.
- Unpredicted rows read "no prediction" rather than an FTD value.

Run with `cd engine/web/tests && npx playwright test verify-panel.spec.js`.
