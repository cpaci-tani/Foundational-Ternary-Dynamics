/**
 * @file campaign_determinism_gate.cpp
 * @brief GATE: is the langevin-OFF genesis spectroscopy harness deterministic?
 *
 * This is the Phase-0 gate for the FTD-0273 cluster energy-spectroscopy program
 * (mass as flux-energy in flip-quanta). Before ANY energy number is trusted, the
 * canonical measurement harness used by Phases 1-2 must be proven to return
 * BIT-IDENTICAL results for an identical (A, seed) configuration, and to be
 * INVARIANT to the OpenMP thread count (omp1 == full pool). The golden gate was
 * re-pinned 2026-06-11 ("races fixed"); this binary re-proves that invariant for
 * the SUPERCRITICAL-INJECTION path specifically (golden runs genesis with NO
 * injection), which is the path the spectroscopy exercises.
 *
 * Harness (shared verbatim with Phases 1-2):
 *   force_cpu + set_sor_iterations(150); fresh RenderBridge per measurement;
 *   disable_all() then ONLY {wave_propagation, gauss_projection, genesis};
 *   langevin = false (the stochastic driver -- the suspected source of the
 *   observed A=2 -> N=2/3/10 spread in the langevin-ON s0-seed-emergent-ic1
 *   scenario); dual_substrate = false; seed_rng(seed); axial inject; settle.
 *
 * For each (thread_mode, A, seed) we run --repeats identical measurements and
 * record manifested count, full-bit field/wave energy, and two content hashes
 * (sorted manifested-index list; per-voxel flux bits). PASS iff every repeat is
 * identical AND omp1 == pool. FAIL prints the diverging cell -- the bug report
 * is then the deliverable.
 *
 * Output: determinism_gate.csv
 *   thread_mode,A,seed,repeat,manifested,field_energy,wave_energy,idx_hash,flux_hash
 *
 * Usage:
 *   campaign_determinism_gate --L=24 --As=2,10 --seeds=2 --repeats=8 \
 *       --settle=200 --output-dir=PATH --tag=gate
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_diagnostics.h"
#include "ftd/voxel.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// FNV-1a 64-bit over raw bytes -- a content fingerprint, NOT cryptographic.
struct Fnv64 {
    std::uint64_t h = 1469598103934665603ull;
    void bytes(const void* p, std::size_t n) {
        const auto* b = static_cast<const unsigned char*>(p);
        for (std::size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ull; }
    }
    void u64(std::uint64_t v) { bytes(&v, sizeof(v)); }
    void f64(double v) { std::uint64_t u; std::memcpy(&u, &v, sizeof(u)); bytes(&u, sizeof(u)); }
};

std::vector<double> parse_list(const std::string& s) {
    std::vector<double> out; std::size_t i = 0;
    while (i < s.size()) {
        std::size_t j = s.find(',', i); if (j == std::string::npos) j = s.size();
        out.push_back(std::atof(s.substr(i, j - i).c_str())); i = j + 1;
    }
    return out;
}

struct Measure {
    int manifested = 0;
    double field_energy = 0.0;
    double wave_energy = 0.0;
    std::uint64_t idx_hash = 0;
    std::uint64_t flux_hash = 0;
};

// One full measurement on a FRESH bridge: settle the supercritical-injection
// genesis dynamics with langevin OFF, then fingerprint the settled lattice.
Measure run_one(int L, double A, std::uint32_t seed, int settle) {
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    rb.set_sor_iterations(150);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = false;   // deterministic harness
    rb.toggles.dual_substrate   = false;
    rb.seed_rng(seed);

    const int c = L / 2;
    rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});  // axial
    for (int t = 0; t < settle; ++t) rb.tick();

    Measure m;
    const auto& voxels = rb.voxels();
    Fnv64 hi, hf;
    const int N = L * L * L;
    for (int i = 0; i < N; ++i) {
        const auto& v = voxels[i];
        if (v.state == 0) continue;
        ++m.manifested;
        hi.u64(static_cast<std::uint64_t>(i));
        hi.u64(static_cast<std::uint64_t>(static_cast<int>(v.state)) & 0xFFull);
        hf.f64(v.flux.x); hf.f64(v.flux.y); hf.f64(v.flux.z);
    }
    m.idx_hash = hi.h;
    m.flux_hash = hf.h;

    const ftd::EnergyAudit ea = rb.energy_audit();
    m.field_energy = ea.field_energy;
    m.wave_energy  = ea.wave_energy;
    return m;
}

bool same(const Measure& a, const Measure& b) {
    return a.manifested == b.manifested && a.idx_hash == b.idx_hash &&
           a.flux_hash == b.flux_hash &&
           std::memcmp(&a.field_energy, &b.field_energy, sizeof(double)) == 0 &&
           std::memcmp(&a.wave_energy, &b.wave_energy, sizeof(double)) == 0;
}

} // namespace

int main(int argc, char** argv) {
    int L = 24, seeds = 2, repeats = 8, settle = 200;
    std::string As_str = "2,10";
    std::string tag = "gate";
    std::string output_dir = "engine/results/determinism_gate/";
    std::uint32_t seed_base = 0xD1117A7Eu;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--As=", 0) == 0)         As_str = a.substr(5);
        else if (a.rfind("--seeds=", 0) == 0)      seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--repeats=", 0) == 0)    repeats = std::atoi(a.c_str() + 10);
        else if (a.rfind("--settle=", 0) == 0)     settle = std::atoi(a.c_str() + 9);
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    const std::vector<double> As = parse_list(As_str);
    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("determinism_gate_" + tag + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", out_csv.string().c_str()); return 1; }
    std::fprintf(f, "thread_mode,A,seed,repeat,manifested,field_energy,wave_energy,idx_hash,flux_hash\n");

    // Two thread modes: omp1 (single) and pool (max). Without OpenMP only omp1
    // is meaningful; we still emit both labels so the analyzer's omp1==pool
    // check trivially passes (identical code path).
    struct Mode { const char* name; int threads; };
#ifdef _OPENMP
    const int maxth = omp_get_max_threads();
#else
    const int maxth = 1;
#endif
    const Mode modes[] = {{"omp1", 1}, {"pool", maxth}};

    std::printf("determinism_gate: L=%d As=%s seeds=%d repeats=%d settle=%d maxthreads=%d\n",
                L, As_str.c_str(), seeds, repeats, settle, maxth);
    std::fflush(stdout);

    bool all_pass = true;
    // Per (A,seed) reference for the cross-mode (omp1 vs pool) check.
    for (double A : As) {
        for (int s = 0; s < seeds; ++s) {
            const std::uint32_t seed = seed_base + static_cast<std::uint32_t>(s) * 2654435761u;
            Measure ref_omp1{};
            bool have_ref_omp1 = false;
            for (const Mode& md : modes) {
#ifdef _OPENMP
                omp_set_num_threads(md.threads);
#endif
                Measure first{};
                bool have_first = false;
                bool mode_ok = true;
                for (int r = 0; r < repeats; ++r) {
                    Measure m = run_one(L, A, seed, settle);
                    if (!have_first) { first = m; have_first = true; }
                    else if (!same(m, first)) {
                        mode_ok = false; all_pass = false;
                        std::printf("  [DIVERGE] mode=%s A=%.0f seed=%u repeat=%d: "
                                    "N=%d (ref %d) idx=%016llx (ref %016llx)\n",
                                    md.name, A, seed, r, m.manifested, first.manifested,
                                    (unsigned long long)m.idx_hash,
                                    (unsigned long long)first.idx_hash);
                    }
                    std::fprintf(f, "%s,%.0f,%u,%d,%d,%.17g,%.17g,%016llx,%016llx\n",
                                 md.name, A, seed, r, m.manifested, m.field_energy, m.wave_energy,
                                 (unsigned long long)m.idx_hash, (unsigned long long)m.flux_hash);
                }
                std::printf("  mode=%-4s A=%-3.0f seed=%u  N=%d  field=%.10g  %s\n",
                            md.name, A, seed, first.manifested, first.field_energy,
                            mode_ok ? "stable" : "UNSTABLE");
                std::fflush(stdout);
                if (std::string(md.name) == "omp1") { ref_omp1 = first; have_ref_omp1 = true; }
                else if (have_ref_omp1 && !same(first, ref_omp1)) {
                    all_pass = false;
                    std::printf("  [OMP-MISMATCH] A=%.0f seed=%u: pool N=%d vs omp1 N=%d\n",
                                A, seed, first.manifested, ref_omp1.manifested);
                }
            }
        }
    }
#ifdef _OPENMP
    omp_set_num_threads(maxth);  // restore
#endif
    std::fclose(f);
    std::printf("wrote %s\n", out_csv.string().c_str());
    std::printf("DETERMINISM: %s\n", all_pass ? "PASS" : "FAIL");
    return all_pass ? 0 : 2;
}
