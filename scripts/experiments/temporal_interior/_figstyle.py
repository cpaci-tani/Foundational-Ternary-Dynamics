"""One typographic scheme for every figure in the semantic-ontology paper.

Import this BEFORE matplotlib.pyplot -- it selects the backend, which
cannot be changed afterwards:

    import _figstyle as fs            # or the path shim, see below
    import matplotlib.pyplot as plt
    ...
    fs.save(fig, "clock")

WHY THIS EXISTS.  The paper had two figure sources with different
rcParams: figures/make_figures.py authored at 6.6 in with 9 pt text, and
eight scripts here authored at 7.3 in with 7.5 pt.  Both were included at
\\textwidth = 6.1677 in, so they were rescaled by 0.935 and 0.845
respectively -- printing at 8.41 pt and 6.34 pt.  The type visibly shrank
a third of the way through the paper and never came back.  Fractional
widths (0.62, 0.95) made two further sizes.

The fix is to author every figure at exactly \\textwidth and include it
at width=\\textwidth, so the scale is 1.0 and the rcParams values below
are the printed point sizes.  If a figure should look smaller, give it
whitespace inside the canvas -- never a scale factor outside it.

FONT.  Default is the PGF backend running pdflatex with mathpazo, the
document's own font, so $G^{*}$ in a figure is set identically to $G^{*}$
in a sentence.  Set FTD_FIG_BACKEND=fallback for an Agg/Palatino-Linotype
approximation (much faster, math glyphs differ subtly) on a machine
without a working TeX-in-matplotlib path.
"""

import os
import pathlib

import matplotlib

TEXTWIDTH_IN = 6.1677          # a4 minus 2 x 1.05 in margins
_BACKEND = os.environ.get("FTD_FIG_BACKEND", "pgf").lower()

FIGDIR = (pathlib.Path(__file__).resolve().parents[3] / "dissemination"
          / "papers" / "semantic_ontology" / "figures")

# Palette.  Status is carried by shape and line-rule, never by colour
# alone -- these must stay legible in greyscale and to a colour-blind
# reader.
C1 = "#2a78d6"      # blue
C2 = "#eb6834"      # orange
C3 = "#1baf7a"      # green
C4 = "#7a4bbd"      # purple
CK = "#2b2b2b"      # near-black, for reference curves
CG = "#9a9a9a"      # grey, for guides and de-emphasis

FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = 8.0, 8.5, 9.0, 7.5, 7.5

_COMMON = {
    "font.size": FS_TICK,
    "axes.labelsize": FS_LAB,
    "axes.titlesize": FS_TITLE,
    "legend.fontsize": FS_LEG,
    "xtick.labelsize": FS_TICK,
    "ytick.labelsize": FS_TICK,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "axes.titlepad": 7.0,
    "axes.labelpad": 3.5,
    "lines.linewidth": 0.9,
    "figure.dpi": 160,
    "savefig.dpi": 600,
}


def _select_backend():
    if _BACKEND == "pgf":
        matplotlib.use("pgf")
        matplotlib.rcParams.update({
            "pgf.texsystem": "pdflatex",
            "pgf.rcfonts": False,
            "pgf.preamble": (r"\usepackage[T1]{fontenc}"
                             r"\usepackage{mathpazo}"),
            "font.family": "serif",
        })
    else:
        matplotlib.use("Agg")
        matplotlib.rcParams.update({
            "font.family": "serif",
            "font.serif": ["Palatino Linotype", "TeX Gyre Pagella",
                           "DejaVu Serif"],
            "mathtext.fontset": "custom",
            "mathtext.rm": "Palatino Linotype",
            "mathtext.it": "Palatino Linotype:italic",
            "mathtext.bf": "Palatino Linotype:bold",
        })
    matplotlib.rcParams.update(_COMMON)


_select_backend()


def figsize(height_in, width_in=TEXTWIDTH_IN):
    """Canonical width, free height."""
    return (width_in, height_in)


def save(fig, name, png=True):
    """Write <name>.pdf into the paper's figures/ at the authored size.

    Deliberately no bbox_inches='tight': a tight box changes the saved
    WIDTH, which is exactly the defect this module exists to remove.

    tight_layout is a different thing and is applied: it repacks the axes
    *inside* a fixed canvas and leaves the figure size alone.  Without it
    the axis labels are clipped, since these figures were authored
    expecting it.  Figures that manage their own layout (constrained or
    an explicit subplots_adjust) are left untouched.
    """
    FIGDIR.mkdir(parents=True, exist_ok=True)
    engine = fig.get_layout_engine()
    if engine is None:
        try:
            fig.tight_layout()
        except Exception:
            pass
    w, h = fig.get_size_inches()
    if abs(w - TEXTWIDTH_IN) > 1e-6:
        raise ValueError(
            "%s: authored at %.4f in, must be %.4f in. Add whitespace "
            "inside the canvas, do not scale on include." % (name, w,
                                                             TEXTWIDTH_IN))
    fig.savefig(FIGDIR / ("%s.pdf" % name))
    if png:
        here = pathlib.Path(__file__).resolve().parent / ("%s.png" % name)
        fig.savefig(here, dpi=200)
    return FIGDIR / ("%s.pdf" % name)


def decade_ticks(axis, ticks, fmt="{:g}"):
    """Fixed plain-decimal ticks on a log axis, minor labels suppressed.

    matplotlib's default log locator emits decade AND minor labels on
    sub-decade ranges, which collide (3x10^-1 against 4x10^-1).
    """
    from matplotlib.ticker import (FixedLocator, FixedFormatter,
                                   NullLocator)
    labels = list(fmt) if not isinstance(fmt, str) else [
        fmt.format(t) for t in ticks]
    axis.set_major_locator(FixedLocator(ticks))
    axis.set_major_formatter(FixedFormatter(labels))
    axis.set_minor_locator(NullLocator())


def backend():
    return _BACKEND
