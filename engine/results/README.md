# `engine/results/` — campaign output policy

This directory holds outputs from `engine/tests/campaign_*.cpp`,
`engine/tests/benchmark_*.cpp`, and `tools/scan_*.py` measurement
runs.

## Tracking policy

**Default: subdirectories are local-only (gitignored).** New campaign
runs land here without polluting the repo. The rule lives in the
project's `.gitignore`:

```
engine/results/
```

This means `git status` won't list new subdirs after a measurement run
— they're invisible to git by design.

**To track a specific campaign permanently** (e.g. canonical-result
data cited by a published paper or a tracked AUDIT/ANALYSIS doc),
use force-add:

```sh
git add -f engine/results/<campaign_name>/
git commit -m "..."
```

Already-tracked subtrees (committed before the gitignore rule was
added on 2026-04-27) are preserved — gitignore only blocks NEW
additions.

## When to track vs leave local

**Track** (force-add) when:
- The campaign output is cited by a tracked AUDIT_*.md / ANALYSIS_*.md
  / LEDGER row that future readers need to verify.
- The output supports a pre-registered measurement (per
  [`docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md`](../../docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md))
  whose tagged commit otherwise wouldn't be reproducible without
  the data.
- The output is a published-paper figure source.

**Leave local** when:
- Exploratory sweep / parameter scan with no analysis-doc cite.
- Re-runs of an already-tracked campaign for personal verification.
- Debug output during development.

## Cited-but-untracked links

A tracked AUDIT/ANALYSIS doc may reference a path inside
`engine/results/<dir>/` that doesn't exist on a fresh clone. This is
expected. The reference is reproducible: check out the
pre-registration tag (per `REF_PREREGISTER_MANIFEST.md`), re-run the
campaign, and the output will land at the cited path.

If you find an analysis-doc cite that's load-bearing for a published
result, **force-add the cited subset** so future readers don't need
to re-run.

## Naming convention

Subdirs follow `<campaign_name>_<YYYY-MM-DD>/` for one-shot runs, or
`<campaign_name>_<YYYY-MM-DD>_<sweep_param>/` for parameter sweeps.
Examples:

- `emergent_spectrum_2026-04-27_L64/` — FTD-0107 G1 measurement
- `look_elsewhere_2026-04-27/` — FTD-0097 scan
- `operator_mixing_2026-04-26/L16_b2_inj0.10/` — FTD-0098+ sweep
- `baseline_2026-04-26/` — multi-campaign baseline snapshot

## Cleanup

A `git clean -fdX engine/results/` will remove every gitignored
subdir, leaving only what's force-tracked. Use that to reclaim disk
space without touching anything in git history.
