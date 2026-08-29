# Recorded Experiment Results

Tracked raw outputs from locked or ledger-registered experiment campaigns live
here, grouped by FTD claim ID. Experiment runners resolve this directory from
their own source path, so invoking them from the repository root does not create
new root-level data files.

| Claim | Runner | Recorded outputs |
|---|---|---|
| FTD-0799 | `protonucleus_growth.py`, `protonucleus_controls.py` | `ftd_0799/protonucleus_growth_results.json`, `ftd_0799/protonucleus_controls_t2.json` |
| FTD-0800 | `maxwell_c3_screen.py` | `ftd_0800/maxwell_c3_results.json`; `ftd_0800/maxwell_c3_tierA.json` preserves the earlier Tier-A control run |

These files are evidence artifacts, not independent derivations. Their claim
status remains governed by the canonical LEDGER and associated analysis docs.
