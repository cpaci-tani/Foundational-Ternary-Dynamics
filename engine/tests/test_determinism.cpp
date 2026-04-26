/**
 * @file test_determinism.cpp
 * @brief Bit-identical reproducibility under fixed seed.
 *
 * The audit (test-orchestrator, 2026-04-25) flagged that no test in the
 * engine asserts byte-identical output across two runs with the same seed.
 * Multi-seed campaigns (FTD-0087 etc.) implicitly assume this property; if
 * silent non-determinism creeps in (uninitialized memory, race conditions,
 * std::unordered_map iteration), all multi-seed measurements are corrupt.
 *
 * This test pins determinism for the CPU path across four representative
 * protocols:
 *   D1. Vacuum wave (wave_propagation + gauss_projection only)
 *   D2. Charged pair with Gauss + forces (no genesis)
 *   D3. Genesis-driven manifestation under high flux (stochastic path)
 *   D4. Langevin thermostat (OU on wave_vel)
 *
 * For each: run twice with seed=0xCAFEBABE, compare voxel arrays byte-for-byte
 * after N ticks. Then run once with a different seed and assert outputs DIFFER
 * (negative control — catches a degenerate "everything is zero" pass).
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <utility>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

namespace {

// Bit-identical comparison of voxel arrays. Returns first index of mismatch,
// or -1 if all match.
int first_mismatch(const std::vector<ftd::Voxel>& a, const std::vector<ftd::Voxel>& b) {
    if (a.size() != b.size()) return 0;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::memcmp(&a[i], &b[i], sizeof(ftd::Voxel)) != 0) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Compact byte-by-byte hash for diagnostics
unsigned long long hash_voxels(const std::vector<ftd::Voxel>& v) {
    unsigned long long h = 1469598103934665603ull;  // FNV-1a 64-bit
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(v.data());
    const size_t n = v.size() * sizeof(ftd::Voxel);
    for (size_t i = 0; i < n; ++i) {
        h ^= bytes[i];
        h *= 1099511628211ull;
    }
    return h;
}

void enable_vacuum_wave(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation = true;
    t.gauss_projection = true;
}

void enable_charged_pair(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.forces           = true;
    t.poisson_coulomb  = true;
    t.movement         = true;
}

void enable_genesis(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.genesis          = true;
    t.movement         = true;
}

void enable_langevin(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.langevin         = true;
}

struct Protocol {
    const char* name;
    void (*setup)(ftd::TermToggles&);
    int  ticks;
    bool inject_flux_seed;
    bool inject_charge_pair;
};

std::vector<ftd::Voxel> run_once(const Protocol& p, unsigned seed, int L) {
    ftd::RenderBridge rb(L);
    p.setup(rb.toggles);
    if (p.setup == enable_langevin) {
        rb.toggles.langevin_T = 0.01;
        rb.toggles.langevin_gamma = 0.05;
        rb.toggles.langevin_seed = seed;
    }
    rb.force_cpu();
    rb.seed_rng(seed);

    if (p.inject_flux_seed) {
        // Asymmetric initial flux. For genesis-driven protocols we need amplitude
        // well above K_GENESIS (= K_B*(1-α) ≈ 0.507) at many voxels so the
        // stochastic threshold actually fires within the test's tick budget.
        const double amp = (p.setup == enable_genesis) ? 8.0 : 2.0;
        for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy) {
            rb.inject_flux_add(L/2 + dx, L/2 + dy, L/2,
                               ftd::Vec3{amp, 0.5 * (dx + 1), 0.0});
        }
        rb.inject_flux_add(L/2 + 1, L/2, L/2, ftd::Vec3{-1.0, 0.0, 0.5});
    }
    if (p.inject_charge_pair) {
        // Stamp a +1/-1 pair separated along x
        rb.inject_particle(L/2 - 2, L/2, L/2, +1, ftd::Vec3{0,0,0}, 0, 0);
        rb.inject_particle(L/2 + 2, L/2, L/2, -1, ftd::Vec3{0,0,0}, 0, 0);
    }
    rb.run(p.ticks);
    return rb.voxels();
}

bool test_protocol(const Protocol& p, int L) {
    const unsigned same_seed  = 0xCAFEBABEu;
    const unsigned other_seed = 0xDEADBEEFu;

    auto v1 = run_once(p, same_seed,  L);
    auto v2 = run_once(p, same_seed,  L);
    auto v3 = run_once(p, other_seed, L);

    const int mm12 = first_mismatch(v1, v2);
    const int mm13 = first_mismatch(v1, v3);
    const auto h1 = hash_voxels(v1);
    const auto h2 = hash_voxels(v2);
    const auto h3 = hash_voxels(v3);

    std::printf("  %-22s | hash same-seed: %016llx, %016llx | other-seed: %016llx\n",
                p.name, h1, h2, h3);

    bool same_seed_ok = (mm12 == -1) && (h1 == h2);
    if (!same_seed_ok) {
        std::printf("    FAIL: same-seed voxel arrays differ at index %d\n", mm12);
        return false;
    }

    // Negative control: different seeds should give different output
    // EXCEPT for protocols with no stochastic source (vacuum wave, charged pair
    // with no genesis or langevin — RNG is unused, so seeds don't affect
    // output and equal hashes are correct).
    const bool has_rng = (p.setup == enable_genesis) || (p.setup == enable_langevin);
    if (has_rng && (mm13 == -1 || h1 == h3)) {
        std::printf("    FAIL: different seeds produced identical output (RNG ignored?)\n");
        return false;
    }
    if (!has_rng && (mm13 != -1 || h1 != h3)) {
        std::printf("    FAIL: protocol has no RNG source but seeds gave different outputs\n");
        return false;
    }

    std::printf("    PASS: same-seed bit-identical; seed-sensitivity %s\n",
                has_rng ? "expected and verified" : "correctly absent");
    return true;
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  Bit-Identical Reproducibility (Determinism) Test\n");
    std::printf("================================================================\n");
    std::printf("  Asserts: two runs with the same seed and protocol produce\n");
    std::printf("           byte-identical voxel arrays after N ticks.\n");
    std::printf("           Different seeds produce different output IFF the\n");
    std::printf("           protocol consumes the RNG.\n\n");
    std::printf("  Scope: CPU determinism under any OMP thread count.\n");
    std::printf("         ARCH-7 (2026-04-25) closed particle-ID order race\n");
    std::printf("         via sequential post-pass.\n");
    std::printf("         ARCH-7b (2026-04-25) closed genesis flux read/write\n");
    std::printf("         race via pre-write flux snapshot.\n");
    std::printf("         Test runs at the system default thread count to\n");
    std::printf("         exercise the full multi-thread path.\n\n");

#ifdef _OPENMP
    std::printf("  [OpenMP] using default thread count = %d\n\n",
                omp_get_max_threads());
#endif

    const int L = 8;

    Protocol protocols[] = {
        { "D1 vacuum wave",      enable_vacuum_wave,   8,  true,  false },
        { "D2 charged pair",     enable_charged_pair,  8,  false, true  },
        { "D3 genesis",          enable_genesis,       4,  true,  false },
        { "D4 langevin OU",      enable_langevin,      8,  true,  false },
    };

    int failures = 0;
    for (const auto& p : protocols) {
        if (!test_protocol(p, L)) ++failures;
    }

    std::printf("\n================================================================\n");
    if (failures == 0) {
        std::printf("  RESULT: %d/4 protocols deterministic (PASS)\n",
                    static_cast<int>(sizeof(protocols)/sizeof(protocols[0])));
    } else {
        std::printf("  RESULT: %d FAILURES detected — engine has silent non-determinism\n",
                    failures);
    }
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
