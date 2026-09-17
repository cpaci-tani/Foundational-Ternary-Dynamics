/**
 * CHARACTERIZATION TEST — weak_transmutation CPU vs CUDA neighbour-order
 * semantics. This test is NOT a bug-free-contract assertion.
 *
 * ── What is being characterized ─────────────────────────────────────────
 * TermToggles::weak_transmutation is declared ToggleBackend::ANY and is ON by
 * default, but the two backends implement two DIFFERENT, mutually incompatible
 * update semantics for the same rule:
 *
 *   CPU  (engine/src/transmutation_phases.cpp, weak_transmutation_cpu)
 *        SEQUENTIAL-IN-PLACE. One pass over ordered_active_indices() (ascending
 *        lattice index). For each site it calls compute_stress_left(i), which
 *        reads the SIX FACE NEIGHBOURS' flux_L live, and on a fire it
 *        immediately does `std::swap(v.flux_L, v.flux_R)` at that site. A site
 *        processed later therefore sees the post-swap flux_L of any earlier
 *        neighbour that fired. Order is load-bearing.
 *
 *   CUDA (engine/cuda/kernels_aux.cu, weak_transmutation_decide_kernel +
 *        weak_transmutation_apply_kernel)
 *        SNAPSHOT. The decide kernel is pure-read over the phase-entry flux_L
 *        and writes only a per-site flip flag; the apply kernel then performs
 *        every swap cell-locally. No site can observe another site's swap.
 *
 * The CUDA file already discloses this in its own header comment (kernels_aux.cu
 * ~L426-436: "Semantics are now snapshot (every site decides against the
 * phase-entry flux) rather than the CPU reference's sequential-in-place order").
 * The split was a deliberate fix for a real read-after-write race that made the
 * dual-substrate GPU path nondeterministic run-to-run; it traded a race for a
 * semantic divergence. Nothing measured it.
 *
 * ── Why the existing guard cannot see it ────────────────────────────────
 * The only pre-existing weak-transmutation parity check is GPC-17
 * (engine/tests/test_gpu_parity_complete.cpp ~L541-562). It injects exactly ONE
 * particle into an L=32 lattice and compares energy audits at a 10% band. With
 * a single manifested site there is no second eligible site whose stress could
 * read the first site's swap, so the sequential/snapshot distinction is
 * structurally unreachable there. It is not a weak guard — it is a guard of a
 * different property.
 *
 * ── What this test does ─────────────────────────────────────────────────
 * Seeds TWO ADJACENT face-neighbour particles, both above WEAK_THRESHOLD in the
 * same tick, so the neighbour-order dependency is actually exercised, and runs
 * exactly one tick with every other rule disabled. Three independent
 * transcriptions are compared, none of them copied from either backend:
 *
 *   reference(Sequential)  — the CPU rule of record
 *   reference(Snapshot)    — the CUDA rule of record
 *   the two live backends
 *
 * so agreement cannot be agreement on a shared mistake, and the divergence is
 * reported site by site and field by field rather than as a bare inequality.
 *
 * ── Standing expectation ────────────────────────────────────────────────
 * Cases 1-4 are EXPECTED TO DIVERGE and the test asserts the exact, measured
 * divergence set. A future maintainer reading a failure here should read it as
 * "the divergence changed shape", not "weak transmutation broke". Case 5 is the
 * control: same firing pattern, same stress magnitudes, but the firing site's
 * L/R swap is a no-op, so the two semantics must coincide — if case 5 ever
 * fails, something other than the disclosed ordering is wrong.
 *
 * Closing the divergence (picking one semantics for both backends) is out of
 * scope here and needs its own decision: the CPU order is not parallelizable as
 * written, and the GPU order is the one that is race-free.
 */

#include "ftd/constants.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel_rng.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {

using ftd::K_MANIFEST;
using ftd::RenderBridge;
using ftd::TermToggles;
using ftd::Vec3;
using ftd::Voxel;
using ftd::WEAK_THRESHOLD;

int passed = 0;
int failed = 0;

void check(const char* name, bool ok) {
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", name);
    ok ? ++passed : ++failed;
}

constexpr int L = 16;
// Flux magnitude used for every seeded source. WEAK_THRESHOLD = K_GENESIS
// = N_c * K_MANIFEST ~ 1.51639 and K_MANIFEST ~ 0.50546, so a site whose stress
// evaluates to FLUX_MAG fires with p = 1 - exp(-(8 - 1.5164)/0.50546)
// = 1 - 2.69e-6 (the run prints the measured value). The
// firing decisions below are therefore not RNG-marginal: every "eligible"
// site in this file fires unless its stress collapses BELOW the threshold, in
// which case it cannot fire at all. That keeps the comparison about ordering
// and not about a coin flip. (Asserted explicitly by the p-margin check in
// main().)
constexpr double FLUX_MAG = 8.0;
constexpr unsigned int SEED = 1;  // TermToggles::langevin_seed default

int index_of(int x, int y, int z) { return x * L * L + y * L + z; }

struct Site {
    int x, y, z;
    int state;       // 0 = unmanifested flux source (never a transmutation candidate)
    Vec3 flux_L;
    Vec3 flux_R;
};

std::vector<Voxel> seed_lattice(const std::vector<Site>& sites) {
    std::vector<Voxel> seed(static_cast<std::size_t>(L) * L * L);
    for (const auto& s : sites) {
        auto& v = seed[static_cast<std::size_t>(index_of(s.x, s.y, s.z))];
        v.state = static_cast<int8_t>(s.state);
        v.flux_L = s.flux_L;
        v.flux_R = s.flux_R;
        v.flux = s.flux_L + s.flux_R;  // dual registers are authoritative
    }
    return seed;
}

// ---------------------------------------------------------------------------
// Independent transcription of the stress functional, written from the
// specification (stress = |div F| + |curl F| + |grad |F||, central differences
// at half-weight over the six face neighbours, periodic wrap) rather than
// copied from field_operators.h::stress_field or the CUDA decide kernel.
// ---------------------------------------------------------------------------
double reference_stress(const std::vector<Vec3>& f, int x, int y, int z) {
    auto wrap = [](int c) { return (c % L + L) % L; };
    auto at = [&](int dx, int dy, int dz) -> const Vec3& {
        return f[static_cast<std::size_t>(
            index_of(wrap(x + dx), wrap(y + dy), wrap(z + dz)))];
    };
    const Vec3& xp = at(+1, 0, 0);
    const Vec3& xm = at(-1, 0, 0);
    const Vec3& yp = at(0, +1, 0);
    const Vec3& ym = at(0, -1, 0);
    const Vec3& zp = at(0, 0, +1);
    const Vec3& zm = at(0, 0, -1);

    const double div = 0.5 * ((xp.x - xm.x) + (yp.y - ym.y) + (zp.z - zm.z));

    const double cx = 0.5 * (yp.z - ym.z) - 0.5 * (zp.y - zm.y);
    const double cy = 0.5 * (zp.x - zm.x) - 0.5 * (xp.z - xm.z);
    const double cz = 0.5 * (xp.y - xm.y) - 0.5 * (yp.x - ym.x);

    const double gx = 0.5 * (xp.mag() - xm.mag());
    const double gy = 0.5 * (yp.mag() - ym.mag());
    const double gz = 0.5 * (zp.mag() - zm.mag());

    return std::abs(div) + std::sqrt(cx * cx + cy * cy + cz * cz)
         + std::sqrt(gx * gx + gy * gy + gz * gz);
}

enum class Semantics { Sequential, Snapshot };

struct Outcome {
    std::vector<int8_t> state;
    std::vector<Vec3> flux_L;
    std::vector<Vec3> flux_R;
};

// Independent transcription of Rule 6 under either semantics. Both branches
// share the same candidate set, the same stress functional, the same threshold,
// and the same RNG stream; they differ ONLY in when the swap becomes visible.
Outcome reference_weak(const std::vector<Site>& sites, Semantics sem) {
    const std::size_t N = static_cast<std::size_t>(L) * L * L;
    Outcome o;
    o.state.assign(N, 0);
    o.flux_L.assign(N, Vec3{});
    o.flux_R.assign(N, Vec3{});
    for (const auto& s : sites) {
        const std::size_t i = static_cast<std::size_t>(index_of(s.x, s.y, s.z));
        o.state[i] = static_cast<int8_t>(s.state);
        o.flux_L[i] = s.flux_L;
        o.flux_R[i] = s.flux_R;
    }

    // Candidate set: every manifested site, ascending lattice index
    // (ordered_active_indices() sorts; the CUDA decide kernel covers the same
    // set because it early-returns on state[i] == 0).
    std::vector<int> active;
    for (std::size_t i = 0; i < N; ++i)
        if (o.state[i] != 0) active.push_back(static_cast<int>(i));

    auto fires = [&](const std::vector<Vec3>& field, int i) {
        const int x = i / (L * L), y = (i / L) % L, z = i % L;
        const double stress = reference_stress(field, x, y, z);
        if (stress <= WEAK_THRESHOLD) return false;
        const double p = 1.0 - std::exp(-(stress - WEAK_THRESHOLD) / K_MANIFEST);
        const double r = ftd::voxel_uniform(
            SEED, i, /*tick=*/0,
            static_cast<std::uint64_t>(ftd::VoxelRng::WeakTransmutation));
        return r < p;
    };

    if (sem == Semantics::Sequential) {
        for (int i : active) {
            // Live read: o.flux_L already carries any earlier site's swap.
            if (!fires(o.flux_L, i)) continue;
            o.state[static_cast<std::size_t>(i)] =
                static_cast<int8_t>(-o.state[static_cast<std::size_t>(i)]);
            std::swap(o.flux_L[static_cast<std::size_t>(i)],
                      o.flux_R[static_cast<std::size_t>(i)]);
        }
    } else {
        const std::vector<Vec3> entry = o.flux_L;  // phase-entry snapshot
        std::vector<int> flagged;
        for (int i : active)
            if (fires(entry, i)) flagged.push_back(i);
        for (int i : flagged) {
            o.state[static_cast<std::size_t>(i)] =
                static_cast<int8_t>(-o.state[static_cast<std::size_t>(i)]);
            std::swap(o.flux_L[static_cast<std::size_t>(i)],
                      o.flux_R[static_cast<std::size_t>(i)]);
        }
    }
    return o;
}

// ---------------------------------------------------------------------------
// Rule 6 in isolation. weak_transmutation declares a dependency on
// dual_substrate (TOGGLE_SPECS), and the L/R swap only exists in dual mode, so
// both are on and nothing else is: no wave propagation, no damping, no genesis,
// no Gauss projection, no forces, no movement. Whatever differs after one tick
// is the transmutation rule and nothing else.
//
// dual_substrate is set as a plain field, NOT via set_dual_substrate(), because
// that setter re-lifts flux_L = flux_R = flux/2 and would erase the deliberate
// L/R asymmetry these cases depend on.
// ---------------------------------------------------------------------------
void configure(TermToggles& t) {
    t.disable_all();
    t.dual_substrate = true;
    t.weak_transmutation = true;
    t.langevin_seed = SEED;
}

Outcome from_voxels(const std::vector<Voxel>& voxels) {
    Outcome o;
    o.state.reserve(voxels.size());
    o.flux_L.reserve(voxels.size());
    o.flux_R.reserve(voxels.size());
    for (const auto& v : voxels) {
        o.state.push_back(v.state);
        o.flux_L.push_back(v.flux_L);
        o.flux_R.push_back(v.flux_R);
    }
    return o;
}

Outcome cpu_run(const std::vector<Site>& sites) {
    RenderBridge bridge(L);
    bridge.force_cpu();
    configure(bridge.toggles);
    bridge.voxels() = seed_lattice(sites);
    bridge.tick();
    return from_voxels(static_cast<const RenderBridge&>(bridge).voxels());
}

Outcome gpu_run(const std::vector<Site>& sites) {
    ftd::gpu::GpuEngine engine(L);
    configure(engine.toggles);
    engine.upload_from_host(seed_lattice(sites));
    engine.tick();
    std::vector<Voxel> out;
    engine.sync_to_host(out);
    return from_voxels(out);
}

bool vec_eq(const Vec3& a, const Vec3& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool same(const Outcome& a, const Outcome& b) {
    if (a.state.size() != b.state.size()) return false;
    for (std::size_t i = 0; i < a.state.size(); ++i) {
        if (a.state[i] != b.state[i]) return false;
        if (!vec_eq(a.flux_L[i], b.flux_L[i])) return false;
        if (!vec_eq(a.flux_R[i], b.flux_R[i])) return false;
    }
    return true;
}

std::string site_name(int i) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "(%d,%d,%d)", i / (L * L), (i / L) % L, i % L);
    return buf;
}

// Every site where the two outcomes differ, with the field that differs.
// This is the measurement the test exists to produce.
std::vector<int> report_divergence(const char* label, const Outcome& a,
                                   const Outcome& b) {
    std::vector<int> sites;
    for (std::size_t i = 0; i < a.state.size(); ++i) {
        const bool ds = a.state[i] != b.state[i];
        const bool dl = !vec_eq(a.flux_L[i], b.flux_L[i]);
        const bool dr = !vec_eq(a.flux_R[i], b.flux_R[i]);
        if (!ds && !dl && !dr) continue;
        sites.push_back(static_cast<int>(i));
        std::printf("      %s %s:", label, site_name(static_cast<int>(i)).c_str());
        if (ds)
            std::printf(" state cpu=%+d gpu=%+d", static_cast<int>(a.state[i]),
                        static_cast<int>(b.state[i]));
        if (dl)
            std::printf(" flux_L cpu=(%g,%g,%g) gpu=(%g,%g,%g)", a.flux_L[i].x,
                        a.flux_L[i].y, a.flux_L[i].z, b.flux_L[i].x,
                        b.flux_L[i].y, b.flux_L[i].z);
        if (dr)
            std::printf(" flux_R cpu=(%g,%g,%g) gpu=(%g,%g,%g)", a.flux_R[i].x,
                        a.flux_R[i].y, a.flux_R[i].z, b.flux_R[i].x,
                        b.flux_R[i].y, b.flux_R[i].z);
        std::printf("\n");
    }
    if (sites.empty()) std::printf("      %s: no differing site\n", label);
    return sites;
}

std::string render_sites(const std::vector<int>& s) {
    std::string out = "{";
    for (std::size_t k = 0; k < s.size(); ++k) {
        if (k) out += ",";
        out += site_name(s[k]);
    }
    return out + "}";
}

// One case: run both backends, pin each to its own rule of record, and assert
// the CPU/GPU difference set is exactly `expected_divergence` (possibly empty).
void run_case(const char* label, const std::vector<Site>& sites,
              const std::vector<int>& expected_divergence) {
    std::printf("\n  -- %s --\n", label);

    // Entry stresses, for the record.
    {
        std::vector<Vec3> f(static_cast<std::size_t>(L) * L * L, Vec3{});
        for (const auto& s : sites)
            f[static_cast<std::size_t>(index_of(s.x, s.y, s.z))] = s.flux_L;
        for (const auto& s : sites) {
            if (s.state == 0) continue;
            std::printf("      entry stress at %s = %.6f  (threshold %.6f)\n",
                        site_name(index_of(s.x, s.y, s.z)).c_str(),
                        reference_stress(f, s.x, s.y, s.z), WEAK_THRESHOLD);
        }
    }

    const Outcome seq = reference_weak(sites, Semantics::Sequential);
    const Outcome snap = reference_weak(sites, Semantics::Snapshot);
    const Outcome cpu = cpu_run(sites);
    const Outcome gpu = gpu_run(sites);

    check((std::string(label) + ": CPU matches the sequential-in-place rule").c_str(),
          same(cpu, seq));
    check((std::string(label) + ": GPU matches the snapshot rule").c_str(),
          same(gpu, snap));

    const auto diff = report_divergence("diff", cpu, gpu);
    const bool as_expected = diff == expected_divergence;
    check((std::string(label) + ": CPU/GPU divergence set is exactly "
           + render_sites(expected_divergence)).c_str(),
          as_expected);
    if (!as_expected)
        std::printf("      measured divergence set: %s\n", render_sites(diff).c_str());
}

// ---------------------------------------------------------------------------
// Case geometries.
//
// Stress at a site is a functional of its SIX FACE NEIGHBOURS' flux_L only —
// never of its own. That is what makes the coupling real: two adjacent
// manifested sites each supply the other's stress, so the earlier one's swap
// rewrites the later one's input.
// ---------------------------------------------------------------------------

// Case 1 — two x-adjacent particles, each carrying flux_L = (M,0,0) and
// flux_R = 0. Each is the other's only non-zero neighbour, so each sees
// |div| = M/2 and |grad rho| = M/2, i.e. stress = M, far above threshold.
// Snapshot: both fire. Sequential: (7,7,7) fires first (lower index), its swap
// drives its flux_L to ZERO, (8,7,7)'s stress collapses to 0 < threshold, and
// it cannot fire at all. GPU over-fires relative to CPU.
const std::vector<Site> kCase1 = {
    {7, 7, 7, +1, Vec3{FLUX_MAG, 0, 0}, Vec3{}},
    {8, 7, 7, +1, Vec3{FLUX_MAG, 0, 0}, Vec3{}},
};

// Case 2 — the same construction rotated onto y, to show the effect is not an
// x-axis or index-stride artifact.
const std::vector<Site> kCase2 = {
    {7, 7, 7, +1, Vec3{0, FLUX_MAG, 0}, Vec3{}},
    {7, 8, 7, +1, Vec3{0, FLUX_MAG, 0}, Vec3{}},
};

// Case 3 — x-adjacent particles carrying TRANSVERSE flux_L = (0,M,0). The
// divergence term now vanishes and the same total stress arrives through
// |curl| = M/2 plus |grad rho| = M/2, exercising a different stress channel.
const std::vector<Site> kCase3 = {
    {7, 7, 7, +1, Vec3{0, FLUX_MAG, 0}, Vec3{}},
    {8, 7, 7, +1, Vec3{0, FLUX_MAG, 0}, Vec3{}},
};

// Case 4 — the divergence in the OPPOSITE direction: CPU fires a site the GPU
// does not. (6,7,7) is an unmanifested flux source that gives (7,7,7) its
// stress. (7,7,7) carries its magnitude in flux_R, with flux_L = 0, so at phase
// entry (8,7,7) sees nothing and the snapshot rule leaves it alone. Under the
// sequential rule (7,7,7) fires first, its swap LIFTS flux_L from 0 to (M,0,0),
// and (8,7,7)'s stress jumps from 0 to M and fires. CPU over-fires.
const std::vector<Site> kCase4 = {
    {6, 7, 7, 0, Vec3{FLUX_MAG, 0, 0}, Vec3{}},
    {7, 7, 7, +1, Vec3{}, Vec3{FLUX_MAG, 0, 0}},
    {8, 7, 7, +1, Vec3{}, Vec3{}},
};

// Case 5 — CONTROL. Two x-adjacent particles that both fire in the same tick at
// the same stress magnitude as case 1, but whose stress comes from flanking
// UNMANIFESTED sources instead of from each other, and whose own flux_L and
// flux_R are both zero so the swap is a no-op. Neither semantics can see the
// other site's update, so the two backends must agree exactly. If this case
// ever fails, the cause is NOT the disclosed ordering divergence.
const std::vector<Site> kCase5 = {
    {6, 7, 7, 0, Vec3{FLUX_MAG, 0, 0}, Vec3{}},
    {7, 7, 7, +1, Vec3{}, Vec3{}},
    {8, 7, 7, +1, Vec3{}, Vec3{}},
    {9, 7, 7, 0, Vec3{FLUX_MAG, 0, 0}, Vec3{}},
};

}  // namespace

int main() {
    std::printf("weak_transmutation CPU/CUDA ordering characterization\n");
    std::printf("(EXPECTED divergence — see the file header before treating a\n");
    std::printf(" failure here as a regression in weak transmutation itself)\n");

    // The firing decisions must not be RNG-marginal, or the divergence sets
    // below would be coin flips rather than measurements of the ordering.
    {
        const double p =
            1.0 - std::exp(-(FLUX_MAG - WEAK_THRESHOLD) / K_MANIFEST);
        std::printf("\n  fire probability at stress=%.1f: p = 1 - %.3e\n",
                    FLUX_MAG, 1.0 - p);
        check("seeded stress fires with p > 1 - 1e-5 (not RNG-marginal)",
              p > 1.0 - 1e-5);
        // The other half of the same claim: when a swap zeroes the neighbour
        // flux that supplied a site's stress, the site drops to stress 0, which
        // is strictly below threshold, so it cannot fire whatever the RNG says.
        check("threshold is strictly positive (collapsed stress cannot fire)",
              WEAK_THRESHOLD > 0.0);
    }

    const int a = index_of(7, 7, 7);
    const int b1 = index_of(8, 7, 7);
    const int b2 = index_of(7, 8, 7);

    run_case("case1 x-adjacent co-flux (div+grad channel)", kCase1, {b1});
    run_case("case2 y-adjacent co-flux (axis control)", kCase2, {b2});
    run_case("case3 x-adjacent transverse flux (curl+grad channel)", kCase3, {b1});
    run_case("case4 latent-R source (CPU over-fires)", kCase4, {b1});
    run_case("case5 CONTROL flanking void sources, swap is a no-op", kCase5, {});

    // Both backends must at least agree that the FIRST site fired: if neither
    // fires, the cases above would trivially "diverge by nothing" and prove
    // nothing. (void)a keeps the index meaningful when the checks below change.
    {
        const Outcome cpu = cpu_run(kCase1);
        const Outcome gpu = gpu_run(kCase1);
        check("case1: the lower-index site fired on BOTH backends "
              "(cases are live, not inert)",
              cpu.state[static_cast<std::size_t>(a)] == -1
              && gpu.state[static_cast<std::size_t>(a)] == -1);
        check("case1: the upper-index site fired on GPU only "
              "(this IS the divergence)",
              cpu.state[static_cast<std::size_t>(b1)] == +1
              && gpu.state[static_cast<std::size_t>(b1)] == -1);
    }
    {
        const Outcome cpu = cpu_run(kCase4);
        const Outcome gpu = gpu_run(kCase4);
        check("case4: the upper-index site fired on CPU only "
              "(divergence is bidirectional)",
              cpu.state[static_cast<std::size_t>(b1)] == -1
              && gpu.state[static_cast<std::size_t>(b1)] == +1);
    }

    // Each backend must be self-consistent run to run, or the divergence sets
    // above are noise rather than semantics.
    {
        bool cpu_stable = true, gpu_stable = true;
        const Outcome cpu0 = cpu_run(kCase1);
        const Outcome gpu0 = gpu_run(kCase1);
        for (int r = 0; r < 4; ++r) {
            cpu_stable = cpu_stable && same(cpu_run(kCase1), cpu0);
            gpu_stable = gpu_stable && same(gpu_run(kCase1), gpu0);
        }
        check("CPU result is reproducible across repeated runs", cpu_stable);
        check("GPU result is reproducible across repeated runs", gpu_stable);
    }

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
