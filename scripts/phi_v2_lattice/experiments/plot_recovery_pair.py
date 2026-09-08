"""Plot exact prepared finite-map pair paths; no new trajectory campaign."""
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

from .. import channels as C, recovery_pair_sector as Pair


def positions(interacting):
    pair, layer = (0, 1), 0
    position = [0, 0, 0]
    result = [tuple(position)]
    for tick in range(48):
        if tick % 4 == 1:
            if interacting:
                pair = Pair.collide(pair, layer)
            layer = (layer-1) % 3
        elif tick % 4 == 2:
            assert C.tangent(pair[0]) == C.tangent(pair[1])
            position = [a+b for a, b in zip(position, C.tangent(pair[0]))]
            pair = tuple(sorted(C.U(c) for c in pair))
        result.append(tuple(position))
    return result


def main():
    actual, free = positions(True), positions(False)
    assert actual[-1] == (-4, 0, 4) and free[-1] == (-4, 4, 4)
    fig, axes = plt.subplots(3, 1, figsize=(9, 7), sharex=True, constrained_layout=True)
    for axis, name in enumerate("xyz"):
        axes[axis].step(range(49), [p[axis] for p in actual], where="post", lw=2,
                        color="#126c8c", label="Actual repeated-collision map")
        axes[axis].step(range(49), [p[axis] for p in free], where="post", lw=1.7,
                        color="#b8592b", linestyle="--", label="Collision-disabled comparison")
        axes[axis].set_ylabel(f"{name} displacement\n(nodes)")
        axes[axis].grid(alpha=.2)
    axes[0].legend(loc="lower left", frameon=False, fontsize=9)
    axes[-1].set_xlabel("Elapsed physical microticks")
    fig.suptitle("Two co-located fields: collisions change their common transport\n"
                 "Prepared channels {0,1}, initial layer 0; localization does not establish binding", fontsize=11)
    target = Path(__file__).resolve().parents[3] / "engine/docs/evidence/strict-pair-transport-2026-09-05.png"
    target.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(target, dpi=160)
    plt.close(fig)
    print(target)


if __name__ == "__main__":
    main()
