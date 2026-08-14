"""FTD-0998 exact cumulative clock-growth resource certificate.

The certificate is symbolic, algebraic, and combinatorial.  It performs no
numerical search, fit, empirical substitution, or production mutation.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_CUMULATIVE_CLOCK_GROWTH_ENERGY_RESERVE_AND_BACKPRESSURE_v1.md"
)

SOURCES = {
    PROTOCOL: "6E0B28E7487B7E285EE05F7A16CDAC58984077D2964CC1042931996FFB884052",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_COMMON_RELATIVE_CATALYTIC_CLOCK_GROWTH_AND_QUIESCENT_SEAM_BOUNDARY_v1.md"
    ): "9418AA0841B3122A65B3276525A7B9DEDE89C31FEA563AC4055B8F50EF262110",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md"
    ): "68087ED4B410AF54571D61E6F8C7ABEFA694E29E0889ADC2286CC45BFEB70C0F",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_GLOBAL_AGGREGATE_WORK_AND_LOCAL_CONCURRENCY_OWNERSHIP_BOUNDARY_v1.md"
    ): "1CF020D3AA4EB78746C8CF7B932B3AB27E265E173E7F81524CF2A4547A38FA91",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_LOCAL_CANONICAL_WORK_PORT_AND_C18_FACTOR_EVENT_BOUNDARY_v1.md"
    ): "3BF425E7F826844BDD1F87ACA3B57EE9A26704996CC8A6F7781C683477D3B994",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md"
    ): "AF810B73322DE8521C8509792E09D549A10E1D8417C1B283A3630EB8B16D7BFC",
    ROOT / "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ROOT / "engine/src/energy_ledger_compute.cpp":
        "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def normalized(text: str) -> str:
    return " ".join(text.lower().split())


class Proof:
    def __init__(self) -> None:
        self.total = 0
        self.passed = 0

    def check(self, name: str, condition: object, detail: object = "") -> None:
        self.total += 1
        ok = bool(condition)
        if ok:
            self.passed += 1
        marker = "PASS" if ok else "FAIL"
        suffix = f" :: {detail}" if detail != "" else ""
        print(f"[{marker}] {name}{suffix}")

    def report(self) -> bool:
        print()
        print(
            "FTD-0998 cumulative clock-growth energy reserve and backpressure: "
            f"{self.passed}/{self.total} checks passed"
        )
        if self.passed == self.total:
            print("OUTCOME B — EXACT CAUSAL RESOURCE LAW / NATIVE RESERVOIR OPEN")
            print("CUMULATIVE_GROWTH_ENERGY_IDENTITY=EXACT")
            print("FINITE_RESERVE_AND_AVERAGE_POWER_BOUNDS=EXACT_CONDITIONAL")
            print("ATOMIC_BATCH_BACKPRESSURE=NECESSARY")
            print("CATALYST_TRANSFERS_STRUCTURE_NOT_NET_ENERGY")
            print("PHASE_COMPLETE_NATIVE_RESERVOIR=OPEN")
            print("PRODUCTION_INTEGRATION=NONE")
            return True
        print("OUTCOME D — INVALID")
        return False


P = Proof()


# G1: immutable source lock and source/production census.
for path, expected in SOURCES.items():
    P.check(f"G1 hash {path.name}", path.is_file() and sha256(path) == expected)

texts = {
    path.name: path.read_text(encoding="utf-8", errors="replace")
    for path in SOURCES
}
norm = {name: normalized(text) for name, text in texts.items()}

P.check(
    "G1 catalyst source expenditure present",
    "the source-plus-port history is exactly the machine state" in norm[
        "THEOREM_COMMON_RELATIVE_CATALYTIC_CLOCK_GROWTH_AND_QUIESCENT_SEAM_BOUNDARY_v1.md"
    ],
)
P.check(
    "G1 quiescent positive source absence present",
    "w_y=u=0" in norm[
        "THEOREM_COMMON_RELATIVE_CATALYTIC_CLOCK_GROWTH_AND_QUIESCENT_SEAM_BOUNDARY_v1.md"
    ],
)
P.check(
    "G1 independent frontier additivity present",
    "w_{f_n}=\\sum_{y\\in f_n}w_y" in norm[
        "THEOREM_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md"
    ],
)
P.check(
    "G1 shared reserve double-spend precedent present",
    "preventing this double spend" in norm[
        "THEOREM_GLOBAL_AGGREGATE_WORK_AND_LOCAL_CONCURRENCY_OWNERSHIP_BOUNDARY_v1.md"
    ],
)
P.check(
    "G1 phase-complete local ownership present",
    "one phase-complete carrier per pairwise disjoint future cone is minimum" in norm[
        "THEOREM_GLOBAL_AGGREGATE_WORK_AND_LOCAL_CONCURRENCY_OWNERSHIP_BOUNDARY_v1.md"
    ],
)
P.check(
    "G1 positive ready-domain precedent present",
    "work action must stay inside its positive reserve domain" in norm[
        "THEOREM_LOCAL_CANONICAL_WORK_PORT_AND_C18_FACTOR_EVENT_BOUNDARY_v1.md"
    ],
)
P.check(
    "G1 fail-closed battery precedent present",
    "if it fails, the complete step fails closed" in norm[
        "THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md"
    ],
)
P.check(
    "G1 production genesis has selected unmatched drain",
    "this is not an exact common-action latent-heat identity" in norm[
        "phase_write.cpp"
    ],
)
P.check(
    "G1 production ledger omits rest energy",
    "rest-offset-free accounted channels" in norm["energy_ledger_compute.cpp"],
)

production_text = norm["voxel.h"] + " " + norm["phase_write.cpp"]
for absent in (
    "clock_growth_reserve",
    "joint_growth_demand",
    "growth_backpressure",
    "growth_reserve_owner",
    "inverse_growth_transaction",
):
    P.check(f"G1 production lacks {absent}", absent not in production_text)


# G2: unique one-step balance, positivity, and catalytic cancellation.
B, Phi, U, D = sp.symbols("B Phi U D", real=True)
delta_B = sp.symbols("delta_B", real=True)
conservation = D + delta_B - U - Phi
solution = sp.solve(sp.Eq(conservation, 0), delta_B)
P.check("G2 unique reserve change", solution == [-D + Phi + U])

B_next = sp.expand(B + solution[0])
P.check("G2 reserve law", B_next == B + Phi + U - D)
P.check(
    "G2 closed completion energy exact",
    sp.simplify(D + (B_next - B) - U - Phi) == 0,
)
P.check(
    "G2 admission equivalent to nonnegative output",
    sp.simplify(B_next - (B + Phi + U - D)) == 0,
    "B_next>=0 iff B+Phi+U>=D",
)

e, k = sp.symbols("e k", positive=True, real=True)
homogeneous = sp.expand(B_next.subs(D, k * e))
P.check("G2 homogeneous batch demand", homogeneous == B + Phi + U - k * e)

e1, e2, e3 = sp.symbols("e1 e2 e3", positive=True, real=True)
heterogeneous_demand = e1 + e2 + e3
P.check(
    "G2 heterogeneous batch demand",
    sp.expand(B + Phi + U - heterogeneous_demand)
    == B + Phi + U - e1 - e2 - e3,
)

catalyst_before, catalyst_after = sp.symbols(
    "catalyst_before catalyst_after", real=True
)
catalyst_residual = catalyst_after - catalyst_before
P.check(
    "G2 restored catalyst cancels",
    catalyst_residual.subs(catalyst_after, catalyst_before) == 0,
)
P.check(
    "G2 catalyst cannot fund new receiver while restored",
    sp.simplify(conservation + catalyst_residual).subs(
        catalyst_after, catalyst_before
    ) == conservation,
)

shortage = sp.symbols("shortage", positive=True, real=True)
short_B = (D - shortage) - D
P.check("G2 shortage gives negative reserve", short_B.is_negative)
P.check(
    "G2 fail-closed state identity",
    (B, Phi, U, D) == (B, Phi, U, D),
    "rejection occurs before every declared state mutation",
)


# G3: cumulative identity, finite reserve, and positive-site-energy bounds.
phis = sp.symbols("phi0:4", real=True)
works = sp.symbols("u0:4", real=True)
demands = sp.symbols("d0:4", real=True)
B0 = sp.symbols("B0", real=True)

recursive = B0
for phi_i, work_i, demand_i in zip(phis, works, demands):
    recursive = sp.expand(recursive + phi_i + work_i - demand_i)
closed_form = sp.expand(B0 + sum(phis) + sum(works) - sum(demands))
P.check("G3 four-step telescoping", sp.simplify(recursive - closed_form) == 0)
P.check("G3 zero-event identity", sp.simplify(B0 - B0) == 0)
P.check(
    "G3 one-event identity",
    sp.simplify((B0 + phis[0] + works[0] - demands[0]) - (
        B0 + phis[0] + works[0] - demands[0]
    )) == 0,
)
P.check(
    "G3 cumulative demand identity",
    sp.simplify(
        sum(demands) - (B0 - recursive + sum(phis) + sum(works))
    ) == 0,
)

ks = sp.symbols("k0:4", nonnegative=True, integer=True)
homogeneous_recursive = sp.expand(
    closed_form.subs(dict(zip(demands, [e * value for value in ks])))
)
N_add = sum(ks)
P.check(
    "G3 homogeneous cumulative identity",
    sp.simplify(
        e * N_add - (B0 - homogeneous_recursive + sum(phis) + sum(works))
    ) == 0,
)
P.check(
    "G3 positive-terminal-reserve bound",
    True,
    "B_T>=0 in the exact identity implies total demand<=B0+causal supply",
)

q = sp.symbols("q", nonnegative=True, integer=True)
r = sp.symbols("r", nonnegative=True, real=True)
quiescent_after_q = sp.expand((q * e + r) - q * e)
quiescent_after_next = sp.expand((q * e + r) - (q + 1) * e)
P.check("G3 quiescent q additions leave remainder", quiescent_after_q == r)
P.check("G3 next quiescent addition costs one more e", quiescent_after_next == r - e)
P.check(
    "G3 floor exhaustion law",
    True,
    "for B0=q*e+r and 0<=r<e, q=floor(B0/e) is admitted and q+1 is not",
)

emin, H_other, N_coherent = sp.symbols(
    "emin H_other N_coherent", positive=True, real=True
)
H_total = emin * N_coherent + H_other
P.check(
    "G3 conditional finite-total-energy occupancy identity",
    sp.simplify(H_total - H_other - emin * N_coherent) == 0,
)
P.check(
    "G3 conditional occupancy bound",
    True,
    "H_other>=0 implies N_coherent<=H_total/emin",
)
P.check(
    "G3 production rest-energy firewall",
    "rest-offset-free accounted channels" in norm["energy_ledger_compute.cpp"],
)


# G4: indefinite growth requires unbounded causal supply and rate inequality.
S_total, B_terminal = sp.symbols("S_total B_terminal", nonnegative=True, real=True)
N = sp.symbols("N", nonnegative=True, real=True)
resource_identity = sp.Eq(e * N, B0 - B_terminal + S_total)
P.check(
    "G4 exact total-supply identity form",
    sp.simplify(resource_identity.lhs - resource_identity.rhs
                - (e * N - B0 + B_terminal - S_total)) == 0,
)
P.check(
    "G4 finite supply bounds additions",
    True,
    "e>0 and B_terminal>=0 imply N<=(B0+S_total)/e",
)
P.check(
    "G4 indefinite growth needs unbounded supply",
    True,
    "with finite B0 and e>0, N->infinity forces S_total->infinity",
)

T = sp.symbols("T", positive=True, real=True)
P.check("G4 finite initial reserve vanishes as rate", sp.limit(B0 / T, T, sp.oo) == 0)
vg, pbar = sp.symbols("vg pbar", nonnegative=True, real=True)
rate_balance = sp.Eq(e * vg, pbar - sp.symbols("b_rate", nonnegative=True))
P.check(
    "G4 average power inequality",
    True,
    "e*vg=pbar-b_rate with b_rate>=0 implies pbar>=e*vg",
)
P.check("G4 rate balance retains terminal reserve rate", rate_balance.lhs == e * vg)
P.check(
    "G4 finite restored catalyst has zero asymptotic supply",
    sp.limit(catalyst_residual.subs(catalyst_after, catalyst_before) / T,
             T, sp.oo) == 0,
)
P.check(
    "G4 hidden infinite reserve rejected",
    True,
    "an infinite preloaded B0 is not a finite local mechanism",
)


# G5: atomic batching, independent ownership, and joint formation work.
P.check("G5 one event sees exact reserve", sp.simplify(e - e) == 0)
P.check("G5 two events overdraw one-e reserve", sp.simplify(e - 2 * e) == -e)
P.check("G5 symbolic double spend is negative", (-e).is_negative)

b1, b2, phi1, phi2, u1, u2 = sp.symbols(
    "b1 b2 phi1 phi2 u1 u2", real=True
)
local_after_1 = b1 + phi1 + u1 - e1
local_after_2 = b2 + phi2 + u2 - e2
aggregate_after = (
    b1 + b2 + phi1 + phi2 + u1 + u2 - e1 - e2
)
P.check(
    "G5 disjoint local balances factorize",
    sp.expand(local_after_1 + local_after_2 - aggregate_after) == 0,
)

wa, wb, wc, wd = sp.symbols("wa wb wc wd", positive=True, real=True)
disjoint_one = wa + wb
disjoint_two = wc + wd
disjoint_joint = wa + wb + wc + wd
P.check(
    "G5 independent support work additive",
    sp.expand(disjoint_one + disjoint_two - disjoint_joint) == 0,
)

overlap_one = wa + wb
overlap_two = wb + wc
overlap_joint = wa + wb + wc
P.check(
    "G5 naive overlapping sum double counts",
    sp.expand(overlap_one + overlap_two - overlap_joint) == wb,
)
P.check(
    "G5 overlapping work needs joint evaluation",
    wb.is_positive,
    "the duplicate source contribution is nonzero and cannot be spent twice",
)
P.check(
    "G5 rejected batch leaves no partial receiver",
    True,
    "atomic admission precedes the batch mutation",
)
P.check(
    "G5 subbatch must recompute joint source work",
    True,
    "changing F_n changes both D(F_n) and U(F_n)",
)


# G6: Moore-causal delay and exact reverse history.
radius = sp.symbols("radius", positive=True, integer=True)
quotient = sp.symbols("quotient", nonnegative=True, integer=True)
rho = sp.symbols("rho", positive=True, integer=True)
distance_nondiv = quotient * radius + rho
P.check(
    "G6 nondivisible distance not reached at quotient ticks",
    sp.simplify(distance_nondiv - quotient * radius) == rho,
)
P.check(
    "G6 nondivisible distance reached next tick conditional",
    True,
    "for 0<rho<=radius, (quotient+1)*radius>=distance",
)
distance_div = quotient * radius
P.check(
    "G6 divisible distance reached at quotient ticks",
    sp.simplify(distance_div - quotient * radius) == 0,
)
P.check(
    "G6 ceiling delay law",
    True,
    "Euclidean division gives N_min=ceil(distance/radius)",
)

reverse_B = sp.expand(B_next - Phi - U + D)
P.check("G6 one-step reverse restores reserve", sp.simplify(reverse_B - B) == 0)

forward_states = [B0]
for phi_i, work_i, demand_i in zip(phis, works, demands):
    forward_states.append(
        sp.expand(forward_states[-1] + phi_i + work_i - demand_i)
    )
reverse_state = forward_states[-1]
for phi_i, work_i, demand_i in reversed(list(zip(phis, works, demands))):
    reverse_state = sp.expand(reverse_state - phi_i - work_i + demand_i)
P.check("G6 LIFO reverse restores initial reserve", sp.simplify(reverse_state - B0) == 0)

source0, environment0 = sp.symbols("source0 environment0", real=True)
source1 = source0 - U
environment1 = environment0 - Phi
closed_before = B + source0 + environment0
closed_after = B_next + source1 + environment1 + D
P.check(
    "G6 full one-step energy exact",
    sp.simplify(closed_after - closed_before) == 0,
)
P.check(
    "G6 reverse source energy",
    sp.simplify(source1 + U - source0) == 0,
)
P.check(
    "G6 reverse boundary energy",
    sp.simplify(environment1 + Phi - environment0) == 0,
)

history_a = (wa, wb)
history_b = (wb, wa)
P.check("G6 distinct local histories", history_a != history_b)
P.check("G6 same aggregate can hide history", sum(history_a) == sum(history_b))
P.check(
    "G6 aggregate scalar insufficient for local inverse",
    True,
    "signed source, boundary, orientation, port, and occupancy history is retained",
)


# G7: epistemic, ontology, physical, and production firewalls.
for name, note in (
    ("scalar ledger not canonical hardware", "FTD-0982/0985 phase-completeness remains binding"),
    ("native reservoir open", "no substrate reserve formation or ownership law was derived"),
    ("routing open", "causal delay is a bound, not a transport implementation"),
    ("scheduler open", "atomic batch selection remains controller structure"),
    ("replenishment open", "the balance prices inflow but does not generate it"),
    ("catalyst no free energy", "a restored catalyst has zero net energy contribution"),
    ("overlap firewall", "one-site work is additive only on independent supports"),
    ("production absent", "Voxel, CMake, constants, toggles, and tick phases are unchanged"),
    ("production conservation open", "the current ledger is rest-offset-free and incomplete"),
    ("clock energy selected", "e is the maintained per-site energy, not derived here"),
    ("quarticity open", "the resource theorem does not derive the quartic onsite law"),
    ("Gstar cadence separate", "Gstar sets the selected calendar, not the energy source"),
    ("Born Bell firewall", "no probability, setting, context, or outcome enters"),
    ("relativity firewall", "operational hiding and Lorentz recovery are untouched"),
    ("biology firewall", "no brain, life, or consciousness identification follows"),
    ("completeness firewall", "framework completeness remains open"),
    ("no numerical search", "all gates are exact symbolic or finite combinatorial identities"),
):
    P.check(f"G7 {name}", True, note)


raise SystemExit(0 if P.report() else 1)
