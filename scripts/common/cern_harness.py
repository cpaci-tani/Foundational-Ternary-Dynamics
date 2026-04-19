"""
cern_harness.py — Shared harness for ftd_cern_*.py analysis scripts.

Consolidates three pieces of boilerplate that were duplicated across the
CERN reinvestigation / partial-correlation / MC-comparison scripts:

  1. load_cms_data_with_mc()  — unified loader for ftd_full_enhanced.npz
                                + ftd_mc_cache.npz, returning a named tuple.
  2. MET_EDGES_FINE / MET_EDGES_BROAD  — canonical MET-bin edges.
  3. ResultsLog  — replaces the `results_lines = []; def log(msg): print(msg);
                   results_lines.append(msg)` pattern that every CERN script
                   redefined. Writes to stdout AND accumulates for a
                   single end-of-run dump.

NOTHING here alters scientific logic; it is pure plumbing. The MC sample
list and cross-section weighting are preserved from the original scripts.

Typical use:

    from scripts.common.cern_harness import (
        load_cms_data_with_mc, MET_EDGES_FINE, MET_EDGES_BROAD, ResultsLog,
    )

    log = ResultsLog()
    log("=" * 70)
    log("MY ANALYSIS")
    log("=" * 70)

    bundle = load_cms_data_with_mc()
    met, rcav, dlsig = bundle.met, bundle.rcav, bundle.dlsig
    # ... analysis ...
    log.write("my_results.txt")
"""
from __future__ import annotations

import os
from typing import NamedTuple, Sequence

import numpy as np


# ---------------------------------------------------------------------------
# Canonical MET bin edges (copied verbatim from ftd_cern_reinvestigation.py
# and ftd_cern_partial_correlation_deep.py; the two scripts had identical
# edges, just spelled differently).
# ---------------------------------------------------------------------------

MET_EDGES_FINE: list[int] = [200, 225, 250, 275, 300, 350, 400, 500, 600, 800, 1200]
MET_EDGES_BROAD: list[int] = [200, 300, 400, 600, 1200]


def met_bin_centers(edges: Sequence[float]) -> list[float]:
    """Midpoints of consecutive bin edges. Convenience wrapper."""
    return [(edges[i] + edges[i + 1]) / 2 for i in range(len(edges) - 1)]


# ---------------------------------------------------------------------------
# Unified loader
# ---------------------------------------------------------------------------

# MC sample list + cross-sections, preserved from the original scripts.
# Weights are computed as xsec / n_total (as in the originals); the cached
# npz carries these values, so we do not hardcode them here.
_MC_SAMPLES: list[str] = [
    "WJetsToLNu",
    "ZJetsToNuNu_200toInf",
    "ZJetsToNuNu_100to200",
    "QCD_HT1000to1500",
    "QCD_HT700to1000",
]


class CMSDataBundle(NamedTuple):
    """Unified bundle of CMS data + weighted MC for CERN analysis scripts.

    Fields (data side, from ftd_full_enhanced.npz):
      met, rcav, dlsig, svmass, bjet, ntracks

    Fields (MC side, from ftd_mc_cache.npz, concatenated + xsec-weighted):
      met_mc, rcav_mc, dlsig_mc, svmass_mc, bjet_mc, w_mc

    `mc_weighted` flag is always True for successful loads and exists so
    callers can check `bundle.mc_weighted` without digging further.
    """
    met: np.ndarray
    rcav: np.ndarray
    dlsig: np.ndarray
    svmass: np.ndarray
    bjet: np.ndarray
    ntracks: np.ndarray
    # MC side
    met_mc: np.ndarray
    rcav_mc: np.ndarray
    dlsig_mc: np.ndarray
    svmass_mc: np.ndarray
    bjet_mc: np.ndarray
    w_mc: np.ndarray
    mc_weighted: bool


def load_cms_data_with_mc(
    data_dir: str | None = None,
    mc_samples: Sequence[str] | None = None,
    data_file: str = "ftd_full_enhanced.npz",
    mc_file: str = "ftd_mc_cache.npz",
) -> CMSDataBundle:
    """
    Load CMS enhanced data + MC cache and return a CMSDataBundle.

    Parameters
    ----------
    data_dir : str, optional
        Directory containing the npz files. Defaults to
        scripts/experiments/ (i.e. the directory that holds all the
        ftd_cern_*.py scripts).
    mc_samples : sequence of str, optional
        Sample names to concatenate. Defaults to the canonical five used
        in the reinvestigation / partial-correlation scripts.
    data_file, mc_file : str
        Filenames relative to data_dir.
    """
    if data_dir is None:
        # Default: the experiments/ directory (where the npz caches live).
        data_dir = os.path.join(
            os.path.dirname(os.path.abspath(__file__)), "..", "experiments"
        )
        data_dir = os.path.normpath(data_dir)

    if mc_samples is None:
        mc_samples = _MC_SAMPLES

    # ---- data side ------------------------------------------------------
    data = np.load(os.path.join(data_dir, data_file))
    met = data["met"]
    rcav = data["rcav"]
    dlsig = data["sv_dlsig_max"]
    svmass = data["sv_mass_max"]
    bjet = data["has_bjet"]
    # ntracks is not present in older caches; default to zeros with matching length.
    if "sv_ntracks_max" in data.files:
        ntracks = data["sv_ntracks_max"]
    else:
        ntracks = np.zeros_like(met)

    # ---- MC side --------------------------------------------------------
    mc_raw = np.load(os.path.join(data_dir, mc_file))
    met_lst, rcav_lst, dl_lst, sv_lst, bj_lst, w_lst = [], [], [], [], [], []
    for s in mc_samples:
        m = mc_raw[f"{s}__met"]
        r = mc_raw[f"{s}__rcav"]
        dl = mc_raw[f"{s}__sv_dlsig_max"]
        sv = mc_raw[f"{s}__sv_mass_max"]
        bj = mc_raw[f"{s}__has_bjet"]
        xsec = float(mc_raw[f"{s}__xsec"])
        n_total = float(mc_raw[f"{s}__n_total"])
        w = np.full(len(m), xsec / n_total)
        met_lst.append(m)
        rcav_lst.append(r)
        dl_lst.append(dl)
        sv_lst.append(sv)
        bj_lst.append(bj)
        w_lst.append(w)

    return CMSDataBundle(
        met=met,
        rcav=rcav,
        dlsig=dlsig,
        svmass=svmass,
        bjet=bjet,
        ntracks=ntracks,
        met_mc=np.concatenate(met_lst),
        rcav_mc=np.concatenate(rcav_lst),
        dlsig_mc=np.concatenate(dl_lst),
        svmass_mc=np.concatenate(sv_lst),
        bjet_mc=np.concatenate(bj_lst),
        w_mc=np.concatenate(w_lst),
        mc_weighted=True,
    )


# ---------------------------------------------------------------------------
# ResultsLog — replaces the duplicated log()/results_lines pattern
# ---------------------------------------------------------------------------


class ResultsLog:
    """
    Tee-style logger: prints to stdout AND accumulates an in-memory
    record for later dump to a text file.

    Call the instance as a function to log a line:

        log = ResultsLog()
        log("hello")             # prints + stores
        log.write("out.txt")     # flush to disk

    The class is deliberately minimal; callers may also access
    `log.lines` directly (it is a plain list of strings).
    """

    __slots__ = ("lines",)

    def __init__(self) -> None:
        self.lines: list[str] = []

    def __call__(self, msg: str = "") -> None:
        print(msg)
        self.lines.append(msg)

    # Convenience alias so `log.log("...")` also works, matching the
    # original function-based pattern some scripts reuse.
    def log(self, msg: str = "") -> None:
        self.__call__(msg)

    def write(self, path: str) -> None:
        """Flush accumulated lines to `path` (overwrites)."""
        with open(path, "w", encoding="utf-8") as f:
            f.write("\n".join(self.lines))

    def extend(self, msgs: Sequence[str]) -> None:
        for m in msgs:
            self.__call__(m)
