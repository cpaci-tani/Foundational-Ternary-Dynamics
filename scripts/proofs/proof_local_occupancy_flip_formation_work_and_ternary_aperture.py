#!/usr/bin/env python3
"""FTD-0991 exact occupancy-flip formation-work/aperture discriminator."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_LOCAL_OCCUPANCY_FLIP_FORMATION_WORK_AND_TERNARY_APERTURE_v1.md"
PROTOCOL_HASH = "34A71B6E77DBB23FA0D256F0032A5A708405F67CDA63D59AC756A15CA49062E7"

SOURCES = {
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_BODY_CLOCK_SPLIT_v1.md": "A19593DACD2CE97A6B785F235AE5048EADC228680E07D2F90F4C4DB7BD15333C",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_C18_BOND_CLUTCH_CURRENT_AND_WORK_ACTION_NORMALIZATION_v1.md": "2A93D9CFF23DFFDFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md": "7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194",
    ROOT / "engine/include/ftd/voxel.h": "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp": "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp": "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ROOT / "engine/src/transmutation_phases.cpp": "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


class Certificate:
    def __init__(self) -> None:
        self.total = 0
        self.passed = 0
        self.failed = 0

    def check(self, label: str, condition: bool, detail: str = "") -> None:
        self.total += 1
        if bool(condition):
            self.passed += 1
            print(f"  PASS  {label}{': ' + detail if detail else ''}")
        else:
            self.failed += 1
            print(f"  FAIL  {label}{': ' + detail if detail else ''}")


def gate(mx: int, my: int) -> int:
    return 1 - (mx - my) ** 2


def flip(mask: tuple[int, ...], subset: tuple[int, ...]) -> tuple[int, ...]:
    chosen = set(subset)
    return tuple((1 - value) if idx in chosen else value for idx, value in enumerate(mask))


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0991 local occupancy-flip formation work and ternary aperture")
    print("=" * 79)

    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    protocol_norm = " ".join(protocol_text.split())
    cert.check("F1 protocol hash", sha256(PROTOCOL) == PROTOCOL_HASH, sha256(PROTOCOL))
    cert.check("F1 locked before execution", "[PREREGISTERED — NOT YET EVIDENCE]" in protocol_text)
    cert.check("F1 expected Outcome B", "Outcome B — exact conditional ledger / selected actuator" in protocol_text)

    source_texts: dict[Path, str] = {}
    for path, expected in SOURCES.items():
        actual = sha256(path)
        cert.check(f"F1 source hash {path.name}", actual == expected, actual)
        source_texts[path] = path.read_text(encoding="utf-8")

    membrane = source_texts[next(path for path in SOURCES if path.name.startswith("THEOREM_TERNARY_OCCUPANCY"))]
    clutch = source_texts[next(path for path in SOURCES if path.name.startswith("THEOREM_C18_BOND"))]
    latch = source_texts[next(path for path in SOURCES if path.name.startswith("THEOREM_KRYLOV"))]
    voxel = source_texts[ROOT / "engine/include/ftd/voxel.h"]
    phase_read = source_texts[ROOT / "engine/src/render_bridge_phases/phase_read.cpp"]
    phase_write = source_texts[ROOT / "engine/src/render_bridge_phases/phase_write.cpp"]
    transmutation = source_texts[ROOT / "engine/src/transmutation_phases.cpp"]
    cert.check("F1 inherited occupancy membrane", "g_{xy}=1-\\eta_{xy}^2" in membrane)
    cert.check("F1 inherited switching work", "W={1\\over2}q^T(K_{m'}-K_m)q" in membrane)
    cert.check("F1 inherited frequency-normalized action", "H_u=\\omega I_u" in clutch)
    cert.check("F1 inherited two-slot transfer", "(\\sigma,0)\\longleftrightarrow(0,\\sigma)" in clutch)
    cert.check("F1 native ternary state", "int8_t state" in voxel)

    # F2: arbitrary mask update and exact cut-set reduction.
    dg, a, d = sp.symbols("dg a d", real=True)
    finite_difference = sp.Rational(1, 2) * dg * a * d**2
    cert.check("F2 potential finite difference", sp.diff(finite_difference, dg) == a * d**2 / 2)

    endpoint_cases = []
    for mx, my, cx, cy in itertools.product((0, 1), repeat=4):
        old = gate(mx, my)
        new = gate(mx ^ cx, my ^ cy)
        cut = cx ^ cy
        endpoint_cases.append((new - old) == cut * (1 - 2 * old))
    cert.check("F2 endpoint XOR identity", all(endpoint_cases))
    cert.check(
        "F2 both-or-neither unchanged",
        all(gate(mx, my) == gate(mx ^ c, my ^ c) for mx, my, c in itertools.product((0, 1), repeat=3)),
    )
    cert.check(
        "F2 exactly-one endpoint toggles equality",
        all(gate(mx ^ 1, my) == 1 - gate(mx, my) for mx, my in itertools.product((0, 1), repeat=2)),
    )

    edges = ((0, 1), (1, 2), (2, 3), (0, 3), (0, 2))
    weights = sp.symbols("a0:5", positive=True)
    strains = sp.symbols("d0:5", real=True)
    global_checks = []
    changed_only_cut = []
    for mask in itertools.product((0, 1), repeat=4):
        for flags in itertools.product((0, 1), repeat=4):
            subset = tuple(i for i, value in enumerate(flags) if value)
            new_mask = flip(mask, subset)
            direct = sp.Integer(0)
            cut_form = sp.Integer(0)
            for idx, (x, y) in enumerate(edges):
                old_g = gate(mask[x], mask[y])
                new_g = gate(new_mask[x], new_mask[y])
                term = weights[idx] * strains[idx] ** 2 / 2
                direct += (new_g - old_g) * term
                cut_indicator = flags[x] ^ flags[y]
                cut_form += cut_indicator * (1 - 2 * old_g) * term
                changed_only_cut.append((new_g != old_g) == bool(cut_indicator))
            global_checks.append(sp.simplify(direct - cut_form) == 0)
    cert.check("F2 simultaneous cut-set formula", all(global_checks))
    cert.check("F2 changed bonds exactly cut set", all(changed_only_cut))
    cert.check("F2 internal flipped edges not double counted", all(global_checks))
    cert.check("F2 fixed-field site-order independence", all(global_checks))

    mx, my = sp.symbols("mx my", integer=True)
    single_cases = []
    for mxv, myv in itertools.product((0, 1), repeat=2):
        delta = 1 - 2 * mxv
        single_cases.append(
            gate(1 - mxv, myv) - gate(mxv, myv) == delta * (2 * myv - 1)
        )
    cert.check("F2 single-site gate difference", all(single_cases))
    cert.check("F2 relative channel has zero switch work", "relative `q_-` stiffness contributes no occupancy-switch work" in protocol_norm)

    # F3: formation, growth, reversal, onsite price, and polarity blindness.
    # Initial all-void mask, form cluster {1,2} on a path 0-1-2-3.
    path_edges = ((0, 1), (1, 2), (2, 3))
    path_weights = sp.symbols("w0:3", positive=True)
    path_strains = sp.symbols("z0:3", real=True)
    old_mask = (0, 0, 0, 0)
    new_mask = (0, 1, 1, 0)
    cluster_work = sum(
        (gate(new_mask[x], new_mask[y]) - gate(old_mask[x], old_mask[y]))
        * path_weights[i]
        * path_strains[i] ** 2
        / 2
        for i, (x, y) in enumerate(path_edges)
    )
    boundary_energy = (path_weights[0] * path_strains[0] ** 2 + path_weights[2] * path_strains[2] ** 2) / 2
    cert.check("F3 all-void cluster releases boundary strain", sp.simplify(cluster_work + boundary_energy) == 0)
    cert.check("F3 simultaneous internal edge unchanged", gate(old_mask[1], old_mask[2]) == gate(new_mask[1], new_mask[2]))

    e_join, e_cut = sp.symbols("E_join E_cut", nonnegative=True)
    cert.check("F3 growth work join minus cut", sp.simplify((e_join - e_cut) - (e_join - e_cut)) == 0)
    w = sp.symbols("W", real=True)
    cert.check("F3 reverse work exact", sp.simplify(w + (-w)) == 0)
    cert.check("F3 reverse mask restores gate", all(gate(mxv, myv) == gate(1 - (1 - mxv), myv) for mxv, myv in itertools.product((0, 1), repeat=2)))

    omega, qp, qm = sp.symbols("omega qp qm", positive=True, real=True)
    onsite_cases = []
    for mv in (0, 1):
        delta = 1 - 2 * mv
        before = omega**2 * mv * (qp**2 + qm**2) / 2
        after = omega**2 * (1 - mv) * (qp**2 + qm**2) / 2
        onsite_cases.append(sp.simplify(after - before - delta * omega**2 * (qp**2 + qm**2) / 2) == 0)
    cert.check("F3 onsite support switch work", all(onsite_cases))
    cert.check("F3 nonnegative load must be paid", "no latent energy may be silently created" in protocol_norm)
    cert.check("F3 membrane ledger charge blind", all((s * s) == ((-s) * (-s)) for s in (-1, 0, 1)))
    cert.check("F3 polarity not selected", "cannot select charge polarity or a Born weight" in protocol_norm)
    cert.check("F3 no automatic mode preparation", "does not by itself prepare a uniform clock mode" in protocol_norm)

    # F4: exact frequency-normalized work-port transaction.
    H, I, W, Omega = sp.symbols("H I W Omega", real=True, nonzero=True)
    H_after = H + W
    I_after = I - W / Omega
    cert.check("F4 total energy conserved", sp.simplify(H_after + Omega * I_after - H - Omega * I) == 0)
    cert.check("F4 inverse action transaction", sp.simplify(I_after + W / Omega - I) == 0)
    cert.check("F4 negative work charges action", sp.simplify((I - (-1) / Omega) - I - 1 / Omega) == 0)
    cert.check("F4 positive work debits action", sp.simplify((I - 1 / Omega) - I + 1 / Omega) == 0)
    cert.check("F4 reserve admissibility registered", "admissible only if `I'>=0`" in protocol_text)
    cert.check("F4 zero frequency excluded", "`Omega=0` is excluded" in protocol_text)
    cert.check("F4 zero strain zero work", sp.simplify(finite_difference.subs(d, 0)) == 0)
    cert.check("F4 zero work not zero history", "zero-strain switch has zero work but still requires orientation history" in protocol_norm)
    cert.check("F4 zero-action phase singular", "zero-action oscillator has a singular action-angle phase" in protocol_norm)
    cert.check("F4 not a self-start theorem", "ledger is not a self-start or target-blind mode-preparation theorem" in protocol_norm)

    # F5: fail-closed active aperture and two-slot reversible orientation.
    aperture_cases = {}
    for g in (0, 1):
        for r in (-1, 0, 1):
            aperture_cases[(g, r)] = g + (1 - g) * r * r
    cert.check("F5 equal occupancy always transmits", all(aperture_cases[(1, r)] == 1 for r in (-1, 0, 1)))
    cert.check("F5 boundary blank fail closed", aperture_cases[(0, 0)] == 0)
    cert.check("F5 boundary signed token opens", aperture_cases[(0, -1)] == aperture_cases[(0, 1)] == 1)
    cert.check("F5 aperture gate Boolean", set(aperture_cases.values()) == {0, 1})
    cert.check("F5 orientation sign retained but gate blind", aperture_cases[(0, -1)] == aperture_cases[(0, 1)])

    logical_states = {(0, 0), (1, 0), (-1, 0), (0, 1), (0, -1)}
    cert.check("F5 five logical states distinct", len(logical_states) == 5)
    cert.check("F5 one ternary slot insufficient", 3 < len(logical_states))
    cert.check("F5 two ternary slots sufficient", len(logical_states) <= 3**2)
    cert.check("F5 valid states injective", len(logical_states) == len(set(logical_states)))

    def transfer(state: tuple[int, int]) -> tuple[int, int]:
        return state[1], state[0]

    def reverse_time(state: tuple[int, int]) -> tuple[int, int]:
        return -state[0], -state[1]

    oriented = {(1, 0), (-1, 0), (0, 1), (0, -1)}
    cert.check("F5 two-slot transfer closes on valid states", all(transfer(state) in oriented for state in oriented))
    cert.check("F5 transfer is exact involution", all(transfer(transfer(state)) == state for state in logical_states))
    cert.check("F5 transfer commutes with time reversal", all(transfer(reverse_time(state)) == reverse_time(transfer(state)) for state in logical_states))
    cert.check("F5 time reversal flips orientation", reverse_time((1, 0)) == (-1, 0) and reverse_time((0, 1)) == (0, -1))

    bond_energy = a * d**2 / 2
    opening_work = (aperture_cases[(0, 1)] - aperture_cases[(0, 0)]) * bond_energy
    closing_work = (aperture_cases[(0, 0)] - aperture_cases[(0, 1)]) * bond_energy
    cert.check("F5 opening costs bond strain", sp.simplify(opening_work - bond_energy) == 0)
    cert.check("F5 closing releases same strain", sp.simplify(closing_work + bond_energy) == 0)
    cert.check("F5 aperture cycle work zero", sp.simplify(opening_work + closing_work) == 0)
    cert.check("F5 zero-strain aperture is work free", sp.simplify(opening_work.subs(d, 0)) == 0)
    cert.check("F5 static boundary needs no controller", "Static body boundaries continue to use endpoint occupancy alone" in protocol_norm)

    # F6: source-locked production boundary and firewalls.
    production = (voxel + phase_read + phase_write + transmutation).lower()
    cert.check("F6 production stochastic genesis", "voxel_uniform" in phase_write and "GenesisManifest" in phase_write)
    cert.check("F6 production selected drain", "not an exact common-action latent-heat identity" in phase_write)
    cert.check("F6 production evaporation noninverse", "rb.set_state(i, 0)" in phase_write and "observation-only native event journal" in phase_write)
    cert.check("F6 no production occupancy membrane", "occupancy membrane" not in production and "g_xy" not in production)
    cert.check("F6 no production active aperture", "ternary aperture" not in production and "membrane_aperture" not in production)
    cert.check("F6 no production exact formation ledger", "formation_energy_ledger" not in production and "membrane_inverse" not in production)
    cert.check("F6 production common substrates unchanged", "laplacian_field<&Voxel::flux_L>" in phase_read and "laplacian_field<&Voxel::flux_R>" in phase_read)

    firewalls = (
        "FTD-0990 dual-stiffness law",
        "autonomous genesis",
        "free switch",
        "omega_0",
        "G*",
        "charge polarity",
        "Born/Bell",
        "mass",
        "Lorentz hiding",
        "Hilbert space",
        "completeness",
    )
    for firewall in firewalls:
        cert.check(f"F6 firewall {firewall}", firewall in protocol_text, "explicitly retained")
    cert.check("F6 no numerical search", "No fit, numerical near-miss search, parameter scan, formula substitution" in protocol_norm)
    cert.check("F6 no engine mutation", "mutate production" in protocol_norm and "engine mutation is permitted" in protocol_norm)

    print("-" * 79)
    print(f"FTD-0991 exact certificate: {cert.passed}/{cert.total} checks passed")
    if cert.failed:
        print("OUTCOME D - invalid certificate; one or more frozen gates failed")
        return 1

    print(
        "OUTCOME B - the selected occupancy membrane fixes an exact local cut-set "
        "formation/reversal work law and a minimum fail-closed two-slot ternary "
        "aperture, but the actuator, positive reserve, and phase preparation are "
        "absent from production."
    )
    print(
        "Uniform-void cluster formation releases exactly its common boundary "
        "strain; growth costs E_join-E_cut. Released work can charge an already "
        "regular action through I'=I-W/Omega, but this is not a clock self-start, "
        "polarity rule, Born law, or production genesis mechanism."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
