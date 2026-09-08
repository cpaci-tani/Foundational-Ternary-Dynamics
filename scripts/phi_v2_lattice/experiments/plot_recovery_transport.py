"""Plot the exact prepared transport comparison, not a new measurement campaign."""
from __future__ import annotations

from fractions import Fraction
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

from .. import recovery_transport as R


def main():
    certificate = R.orbit_certificate(0)
    times = [Fraction(k, 8) for k in range(48*8+1)]
    errors = []
    for time in times:
        position = R.lifted_displacement(0, int(time))
        errors.append(float(max(abs(position[a]-time*certificate.velocity[a]) for a in range(3))))
    spacings = [Fraction(1, 2**k) for k in range(5)]
    held = [float(a*certificate.held_time_ripple_linf) for a in spacings]
    ticks = [float(a*certificate.integer_tick_ripple_linf) for a in spacings]
    plt.rcParams.update({"font.size": 10, "axes.spines.top": False, "axes.spines.right": False})
    fig, axes = plt.subplots(1, 2, figsize=(10.5, 4.2), layout="constrained")
    axes[0].plot([float(t) for t in times], errors, color="#156b8a", linewidth=1.5)
    axes[0].axhline(float(certificate.held_time_ripple_linf), color="#b35424", linestyle="--", label="Held-time bound 11/12")
    axes[0].set(xlabel="Elapsed physical microticks", ylabel="Path error in microscopic node units",
                title="Bounded motion within the 48-tick period", xlim=(0,48), ylim=(0,1.1))
    axes[0].legend(frameon=False, loc="upper left")
    axes[1].loglog([float(a) for a in spacings], held, "o-", color="#b35424", label="Held-time bound 11a/12")
    axes[1].loglog([float(a) for a in spacings], ticks, "s--", color="#156b8a", label="Tick-time bound 5a/6")
    axes[1].set(xlabel="Comparison lattice spacing a", ylabel="Uniform comparison error bound",
                title="Error decreases linearly with spacing")
    axes[1].legend(frameon=False)
    for ax in axes:
        ax.grid(True, alpha=.18)
    fig.suptitle("Exact prepared singleton transport: microscopic path versus characteristic advection", fontsize=12)
    root = Path(__file__).resolve().parents[3]
    out = root / "engine/docs/evidence/strict-transport-bound-2026-09-05.png"
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, dpi=180)
    plt.close(fig)
    print(out)


if __name__ == "__main__":
    main()
