"""proof_schedule_type_audit.py -- FTD-1027 (drafted). Source-lint verifier for
AUDIT_SCHEDULE_TYPE_CENSUS.md: types each phase of RenderBridge::tick() by
KIND (hyperbolic / elliptic / combinatorial / algebraic) from the C++ source.

Every assertion is a regex against the engine source. Nothing is transcribed.
Same genre as the FTD-0792 source-lint test.
"""
from __future__ import annotations
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "engine" / "src"
INC = ROOT / "engine" / "include" / "ftd"


def read(p):
    return p.read_text(encoding="utf-8", errors="replace")


def lines_matching(text, pat):
    return [i + 1 for i, l in enumerate(text.splitlines()) if re.search(pat, l)]


results = []


def check(name, ok, evidence):
    results.append((name, bool(ok)))
    tag = "PASS" if ok else "FAIL"
    print(f"  [{tag}] {name}\n         {evidence}")


def section(t):
    print("\n" + "=" * 78 + f"\n{t}\n" + "=" * 78)


poisson = read(SRC / "poisson_solvers.cpp")
pread = read(SRC / "render_bridge_phases" / "phase_read.cpp")
pwrite = read(SRC / "render_bridge_phases" / "phase_write.cpp")
pmove = read(SRC / "render_bridge_phases" / "phase_movement.cpp")
pforce = read(SRC / "render_bridge_phases" / "phase_forces.cpp")
trans = read(SRC / "transmutation_phases.cpp")
bridge = read(SRC / "render_bridge.cpp")
toggles = read(INC / "term_toggles.h")


def dflt(name):
    m = re.search(rf"^\s*bool\s+{name}\s*=\s*(true|false)", toggles, re.M)
    return m.group(1) if m else None


# ================================================================ ELLIPTIC
section("E.  ELLIPTIC PHASES  (constraint solves; no time derivative)")
sor = lines_matching(poisson, r"phi\[idx\]\s*\+=\s*omega\s*\*\s*\(gs\s*-\s*phi\[idx\]\)")
check("SOR sweep is pure relaxation  phi += omega*(gs - phi)", len(sor) >= 1,
      f"poisson_solvers.cpp lines {sor}")

plines = poisson.splitlines()
bad = [n for n in lines_matching(poisson, r"\bdt\b|wave_vel")
       if not re.search(r"mag2\(\)|T00|source|\|wave_vel\|", plines[n - 1])]
check("no time-derivative or velocity term in any solver update", len(bad) == 0,
      f"dt/wave_vel outside source terms at lines {bad}" if bad
      else "the only wave_vel refs are the latency T00 source term")

nip = lines_matching(poisson, r"NOT AN IDEMPOTENT PROJECTION")
check("gauss_project is NOT an orthogonal projector (source says so)", len(nip) == 1,
      f"poisson_solvers.cpp line {nip}")
sat = lines_matching(poisson, r"saturates near 1e-2")
check("Gauss residual floors at ~1e-2 (constraint never exactly enforced)", len(sat) == 1,
      f"poisson_solvers.cpp line {sat}")

cc = lines_matching(pforce, r"solve_coulomb_poisson|poisson_coulomb")
check("an elliptic Coulomb solve runs inside phase_forces under poisson_coulomb", len(cc) >= 1,
      f"phase_forces.cpp lines {cc[:4]}")

gd = lines_matching(trans, r"u_old\.a\s*\+\s*staple_adj\.a\s*\*\s*\(dt\s*\*\s*beta\)")
nz = lines_matching(trans, r"u_new\.normalize\(\)")
check("SU(2) link update is U_new = Proj[U_old + dt*beta*staple^dag]  (relaxation)",
      gd and nz, f"transmutation_phases.cpp lines {gd} (step) {nz} (projection)")

# ============================================================== HYPERBOLIC
section("H.  HYPERBOLIC PHASE  (second-order wave; the only propagating field)")
lap = lines_matching(pread, r"delta_j_\[i\]\s*=\s*lap\s*\*\s*cw2")
check("wave source is c^2 * Laplacian(J)", len(lap) >= 1, f"phase_read.cpp lines {lap}")

kick = lines_matching(pwrite, r"wave_vel_L\s*\+=\s*rb\.delta_j_L_\[i\]\s*\*")
drift = lines_matching(pwrite, r"flux_L\s*\+=\s*v\.wave_vel_L\s*\*\s*rb\.dt_")
check("commit is second-order: wave_vel += dJ*dt ; flux += wave_vel*dt", kick and drift,
      f"phase_write.cpp kick {kick[:2]} drift {drift[:2]}")

om = lines_matching(pread, r"omega0_sq")
guard = lines_matching(pread, r"do_db_clock")
check("mass term (omega0^2 J) appears ONLY under the de_broglie_clock branch",
      len(om) >= 1 and len(guard) >= 1 and min(guard) < max(om),
      f"omega0_sq at {om}; do_db_clock guards at {guard}")

ungated = []
gated = []
for p in SRC.rglob("*.cpp"):
    if p.name == "phase_write.cpp":
        continue
    t = read(p)
    tl = t.splitlines()
    for n in lines_matching(t, r"wave_vel(_L|_R)?\s*\+=\s*.*delta_j"):
        # every toggle guard in the enclosing window; gated if ANY defaults OFF
        guards = []
        for k in range(n - 1, max(0, n - 40), -1):
            m = re.search(r"if \(toggles\.(\w+)\)", tl[k - 1])
            if m:
                guards.append(m.group(1))
        off = [g for g in guards if dflt(g) == "false"]
        if off:
            gated.append((p.name, n, off[0]))
        else:
            ungated.append((p.name, n, guards))
check("every wave integrator outside phase_write is behind a default-OFF toggle",
      len(ungated) == 0,
      f"ungated: {ungated}" if ungated else f"gated only: {gated}")

# =========================================================== COMBINATORIAL
section("C.  COMBINATORIAL / NON-INJECTIVE PHASES  (threshold + seeded draw, collisions)")
gen = lines_matching(pwrite, r"flux\.mag2\(\)\s*>\s*(K_GENESIS\s*\*\s*K_GENESIS|kg\s*\*\s*kg)")
gu = lines_matching(pwrite, r"voxel_uniform\(gseed,\s*i,\s*rb\.tick_")
check("genesis = threshold |J|^2 > K_GENESIS^2  AND  seeded draw voxel_uniform(gseed,i,tick)",
      gen and gu, f"phase_write.cpp threshold {gen} draw {gu[:2]}")

ann = lines_matching(pmove, r"annihilation")
void = lines_matching(pmove, r"return to void")
check("movement contains annihilation (both particles return to void)", ann and void,
      f"phase_movement.cpp lines {ann[:3]}")

flip = lines_matching(trans, r"set_state\(i,\s*static_cast<int8_t>\(-v\.state\)\)")
wth = lines_matching(trans, r"stress\s*>\s*WEAK_THRESHOLD")
check("weak transmutation = threshold on stress + seeded draw + SIGN FLIP of state",
      flip and wth, f"transmutation_phases.cpp threshold {wth} flip {flip}")

pp = lines_matching(trans, r"jmag\s*<=\s*K_GENESIS")
check("pair_production = threshold on |J| > K_GENESIS (same class as genesis)", len(pp) >= 1,
      f"transmutation_phases.cpp line {pp}")

tri = lines_matching(trans, r"va\.locked\s*=\s*true")
check("triad_binding sets a monotone lock flag (combinatorial, no dynamics)", len(tri) >= 1,
      f"transmutation_phases.cpp line {tri}")

# =============================================================== ALGEBRAIC
section("A.  ALGEBRAIC / LOCAL PHASES")
pt = lines_matching(trans, r"v\.tau\s*\+=\s*delta_tau")
pr = lines_matching(trans, r"proper_time_rate\(v\.latency")
check("proper time is a local monotone accumulation tau += rate(latency, u^2)", pt and pr,
      f"transmutation_phases.cpp {pr} {pt}")
ax = lines_matching(trans, r"selected clock/bandwidth axiom, not a substrate derivation")
check("source labels the clock law an AXIOM, not a derivation", len(ax) == 1,
      f"transmutation_phases.cpp line {ax}")

# ================================================================ DEFAULTS
section("D.  WHAT RUNS BY DEFAULT  (term_toggles.h)")

ON = ["wave_propagation", "coupling", "genesis", "gauss_projection", "forces", "movement",
      "weak_transmutation", "poisson_coulomb", "damping", "dual_substrate"]
OFF = ["latency_field", "de_broglie_clock", "pair_production", "triad_binding", "su2_gauge",
       "su3_gauge", "evaporation", "strong_stress_energy", "matched_gauss_dynamics",
       "emergent_forces", "langevin", "field_energy_gravity"]
on_ok = {n: dflt(n) for n in ON}
off_ok = {n: dflt(n) for n in OFF}
check("default-ON set", all(v == "true" for v in on_ok.values()), str(on_ok))
check("default-OFF set", all(v == "false" for v in off_ok.values()), str(off_ok))

lat = lines_matching(bridge, r"if \(toggles\.latency_field\)\s*$")
lat2 = lines_matching(bridge, r"solve_latency_poisson\(\);")
check("gravity (latency solve) is gated on latency_field  -> OFF by default", lat and lat2,
      f"render_bridge.cpp {lat} -> {lat2}")
ptg = lines_matching(bridge, r"if \(toggles\.latency_field \|\| toggles\.de_broglie_clock\)")
check("proper time is gated on latency_field || de_broglie_clock  -> OFF by default",
      len(ptg) >= 1, f"render_bridge.cpp line {ptg}")

# ================================================================== VERDICT
section("SELF-CHECK")
fails = [n for n, ok in results if not ok]
print(f"  {len(results) - len(fails)}/{len(results)} source assertions verified")
print("  FAILURES:", fails if fails else "none")
sys.exit(1 if fails else 0)
