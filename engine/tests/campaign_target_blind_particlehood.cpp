/**
 * @file campaign_target_blind_particlehood.cpp
 * @brief FTD-0399 target-blind particlehood campaign.
 *
 * Test instrument only: no production API or engine behavior changes.
 * The frozen definitions and outcome precedence are in
 * PREREG_TARGET_BLIND_PARTICLEHOOD_v1.md.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>
#include <queue>
#include <string>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/term_toggles.h"
#include "ftd/voxel.h"

namespace {

constexpr int kLocalRadius = 4;
constexpr int kLocalSide = 2 * kLocalRadius + 1;
constexpr int kLocalSites = kLocalSide * kLocalSide * kLocalSide;
constexpr int kProfileComponents = 10;  // J(3), Jdot(3), state/color/spin/flavor
constexpr int kMaxWait = 200;
constexpr int kFreezeTickExpected = 2;
constexpr double kBand = 0.01;
constexpr double kNormFloor = 1e-15;

struct SeedSpec {
    const char* name;
    double ox, oy, oz;
    double amp, sigma, cut_r;
};

const SeedSpec kSeeds[] = {
    {"A_baseline", 0.31, 0.17, 0.07, 3.00, 0.45, 4.0},
    {"C_hot",      0.31, 0.17, 0.07, 5.00, 0.45, 4.0},
    {"E_cold",     0.31, 0.17, 0.07, 2.15, 0.45, 4.0},
};

enum class Protocol { Dissipative, Undamped };

const char* protocol_name(Protocol protocol) {
    return protocol == Protocol::Dissipative ? "dissipative" : "undamped";
}

struct Snapshot {
    Protocol protocol{};
    int L = 0;
    int seed_index = 0;
    int t_post = 0;
    int freeze_tick = -1;
    int center_x = -1, center_y = -1, center_z = -1;
    int cluster_count = 0;
    int N = 0;
    int charge = 0;
    double centroid_x = 0.0, centroid_y = 0.0, centroid_z = 0.0;
    double local_energy = 0.0;
    bool localized = false;
    bool boundary_clear = false;
    bool backend_cpu = false;
    bool toggles_exact = false;
    std::vector<double> profile;
};

struct History {
    Protocol protocol{};
    int L = 0;
    int seed_index = 0;
    int freeze_tick = -1;
    bool manifested = false;
    bool duplicate_identical = false;
    std::vector<Snapshot> snapshots;
};

int horizon_for(int L) { return L == 33 ? 12 : 24; }

int index3(int L, int x, int y, int z) { return (x * L + y) * L + z; }

void configure(ftd::RenderBridge& rb, Protocol protocol) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = true;
    if (protocol == Protocol::Dissipative) {
        rb.toggles.damping = true;
        rb.toggles.selective_damping = true;
    }
}

bool toggles_are_exact(const ftd::TermToggles& toggles, Protocol protocol) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        const std::string name(spec.name);
        bool expected = name == "wave_propagation" || name == "coupling" ||
                        name == "gauss_projection" || name == "genesis";
        if (protocol == Protocol::Dissipative &&
            (name == "damping" || name == "selective_damping")) expected = true;
        if (toggles.*(spec.field) != expected) return false;
    }
    return toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic &&
           toggles.coulomb_charge_coupling == 1.0 &&
           toggles.coulomb_source_scale == 1.0 &&
           toggles.kinetic_drain == 0.5;
}

void seed_radial_pulse(ftd::RenderBridge& rb, const SeedSpec& seed, int L) {
    const double cx = (L - 1) / 2.0 + seed.ox;
    const double cy = (L - 1) / 2.0 + seed.oy;
    const double cz = (L - 1) / 2.0 + seed.oz;
    for (int x = std::max(0, static_cast<int>(cx - seed.cut_r));
         x <= std::min(L - 1, static_cast<int>(cx + seed.cut_r) + 1); ++x)
    for (int y = std::max(0, static_cast<int>(cy - seed.cut_r));
         y <= std::min(L - 1, static_cast<int>(cy + seed.cut_r) + 1); ++y)
    for (int z = std::max(0, static_cast<int>(cz - seed.cut_r));
         z <= std::min(L - 1, static_cast<int>(cz + seed.cut_r) + 1); ++z) {
        const double dx = x - cx, dy = y - cy, dz = z - cz;
        const double r2 = dx * dx + dy * dy + dz * dz;
        if (r2 > seed.cut_r * seed.cut_r) continue;
        const double r = std::sqrt(r2);
        if (r < 1e-9) continue;
        const double amp = seed.amp * std::exp(-r2 / (2.0 * seed.sigma * seed.sigma));
        if (amp < 1e-9) continue;
        rb.inject_flux_add(x, y, z,
            ftd::Vec3(amp * dx / r, amp * dy / r, amp * dz / r));
    }
}

bool first_manifested_site(const ftd::RenderBridge& rb, int& x, int& y, int& z) {
    const int L = rb.lattice().size();
    const auto& voxels = rb.voxels();
    for (int ix = 0; ix < L; ++ix)
    for (int iy = 0; iy < L; ++iy)
    for (int iz = 0; iz < L; ++iz) {
        if (voxels[index3(L, ix, iy, iz)].state != 0) {
            x = ix; y = iy; z = iz;
            return true;
        }
    }
    x = y = z = -1;
    return false;
}

Snapshot capture(const ftd::RenderBridge& rb, Protocol protocol, int seed_index,
                 int t_post, int freeze_tick, int cx, int cy, int cz,
                 bool backend_cpu, bool toggles_exact) {
    Snapshot result;
    result.protocol = protocol;
    result.L = rb.lattice().size();
    result.seed_index = seed_index;
    result.t_post = t_post;
    result.freeze_tick = freeze_tick;
    result.center_x = cx; result.center_y = cy; result.center_z = cz;
    result.backend_cpu = backend_cpu;
    result.toggles_exact = toggles_exact;
    const int L = result.L;
    const auto& voxels = rb.voxels();

    std::vector<unsigned char> manifested(static_cast<std::size_t>(L) * L * L, 0);
    bool all_local = true;
    const int center_margin = std::min({cx, cy, cz, L - 1 - cx, L - 1 - cy, L - 1 - cz});
    const bool boundary_clear = center_margin - kLocalRadius >= t_post;
    double centroid_x_sum = 0.0, centroid_y_sum = 0.0, centroid_z_sum = 0.0;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int idx = index3(L, x, y, z);
        const auto& v = voxels[idx];
        if (v.state != 0) {
            manifested[idx] = 1;
            ++result.N;
            result.charge += static_cast<int>(v.state);
            centroid_x_sum += x; centroid_y_sum += y; centroid_z_sum += z;
            if (std::max({std::abs(x - cx), std::abs(y - cy), std::abs(z - cz)}) > kLocalRadius)
                all_local = false;
        }
    }
    if (result.N > 0) {
        result.centroid_x = centroid_x_sum / result.N;
        result.centroid_y = centroid_y_sum / result.N;
        result.centroid_z = centroid_z_sum / result.N;
    }

    std::vector<unsigned char> visited(manifested.size(), 0);
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int start = index3(L, x, y, z);
        if (!manifested[start] || visited[start]) continue;
        ++result.cluster_count;
        std::queue<int> pending;
        pending.push(start);
        visited[start] = 1;
        while (!pending.empty()) {
            const int idx = pending.front(); pending.pop();
            const int px = idx / (L * L);
            const int rem = idx % (L * L);
            const int py = rem / L;
            const int pz = rem % L;
            for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy)
            for (int dz = -1; dz <= 1; ++dz) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                const int nx = px + dx, ny = py + dy, nz = pz + dz;
                if (nx < 0 || ny < 0 || nz < 0 || nx >= L || ny >= L || nz >= L) continue;
                const int ni = index3(L, nx, ny, nz);
                if (manifested[ni] && !visited[ni]) {
                    visited[ni] = 1;
                    pending.push(ni);
                }
            }
        }
    }

    result.localized = result.N > 0 && result.N <= kLocalSites && all_local;
    result.boundary_clear = boundary_clear;
    result.profile.reserve(kLocalSites * kProfileComponents);
    for (int dx = -kLocalRadius; dx <= kLocalRadius; ++dx)
    for (int dy = -kLocalRadius; dy <= kLocalRadius; ++dy)
    for (int dz = -kLocalRadius; dz <= kLocalRadius; ++dz) {
        const auto& v = voxels[index3(L, cx + dx, cy + dy, cz + dz)];
        result.profile.push_back(v.flux.x);
        result.profile.push_back(v.flux.y);
        result.profile.push_back(v.flux.z);
        result.profile.push_back(v.wave_vel.x);
        result.profile.push_back(v.wave_vel.y);
        result.profile.push_back(v.wave_vel.z);
        result.profile.push_back(static_cast<double>(v.state));
        result.profile.push_back(static_cast<double>(v.color));
        result.profile.push_back(static_cast<double>(v.spin));
        result.profile.push_back(static_cast<double>(v.flavor));
        result.local_energy += 0.5 * (v.flux.mag2() + v.wave_vel.mag2());
    }
    return result;
}

History execute_history(Protocol protocol, int L, int seed_index) {
    History history;
    history.protocol = protocol;
    history.L = L;
    history.seed_index = seed_index;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    configure(rb, protocol);
    const bool backend_cpu = rb.backend_kind() == ftd::Backend::Kind::Cpu;
    const bool exact = toggles_are_exact(rb.toggles, protocol);
    seed_radial_pulse(rb, kSeeds[seed_index], L);

    int cx = -1, cy = -1, cz = -1;
    for (int tick = 0; tick <= kMaxWait; ++tick) {
        if (first_manifested_site(rb, cx, cy, cz)) {
            history.manifested = true;
            history.freeze_tick = tick;
            break;
        }
        rb.tick();
    }
    if (!history.manifested) return history;
    if (cx - kLocalRadius < 0 || cx + kLocalRadius >= L ||
        cy - kLocalRadius < 0 || cy + kLocalRadius >= L ||
        cz - kLocalRadius < 0 || cz + kLocalRadius >= L) return history;

    const int horizon = horizon_for(L);
    history.snapshots.reserve(horizon + 1);
    for (int t_post = 0; t_post <= horizon; ++t_post) {
        history.snapshots.push_back(capture(rb, protocol, seed_index, t_post,
            history.freeze_tick, cx, cy, cz, backend_cpu, exact));
        if (t_post < horizon) rb.tick();
    }
    return history;
}

bool same_snapshot(const Snapshot& a, const Snapshot& b) {
    if (a.protocol != b.protocol || a.L != b.L || a.seed_index != b.seed_index ||
        a.t_post != b.t_post || a.freeze_tick != b.freeze_tick ||
        a.center_x != b.center_x || a.center_y != b.center_y || a.center_z != b.center_z ||
        a.cluster_count != b.cluster_count || a.N != b.N || a.charge != b.charge ||
        a.localized != b.localized || a.boundary_clear != b.boundary_clear ||
        a.backend_cpu != b.backend_cpu || a.toggles_exact != b.toggles_exact ||
        a.profile.size() != b.profile.size()) return false;
    if (std::memcmp(&a.centroid_x, &b.centroid_x, sizeof(double)) != 0 ||
        std::memcmp(&a.centroid_y, &b.centroid_y, sizeof(double)) != 0 ||
        std::memcmp(&a.centroid_z, &b.centroid_z, sizeof(double)) != 0 ||
        std::memcmp(&a.local_energy, &b.local_energy, sizeof(double)) != 0) return false;
    return a.profile.empty() ||
        std::memcmp(a.profile.data(), b.profile.data(), a.profile.size() * sizeof(double)) == 0;
}

bool same_history(const History& a, const History& b) {
    if (a.protocol != b.protocol || a.L != b.L || a.seed_index != b.seed_index ||
        a.freeze_tick != b.freeze_tick || a.manifested != b.manifested ||
        a.snapshots.size() != b.snapshots.size()) return false;
    for (std::size_t i = 0; i < a.snapshots.size(); ++i)
        if (!same_snapshot(a.snapshots[i], b.snapshots[i])) return false;
    return true;
}

double profile_norm(const std::vector<double>& values) {
    double sum = 0.0;
    for (double value : values) sum += value * value;
    return std::sqrt(sum);
}

double raw_distance(const Snapshot& a, const Snapshot& b) {
    double sum = 0.0;
    for (std::size_t i = 0; i < a.profile.size(); ++i) {
        const double delta = a.profile[i] - b.profile[i];
        sum += delta * delta;
    }
    return std::sqrt(sum) /
        std::max({profile_norm(a.profile), profile_norm(b.profile), kNormFloor});
}

double shape_distance(const Snapshot& a, const Snapshot& b) {
    const double na = std::max(profile_norm(a.profile), kNormFloor);
    const double nb = std::max(profile_norm(b.profile), kNormFloor);
    double sum = 0.0;
    for (std::size_t i = 0; i < a.profile.size(); ++i) {
        const double delta = a.profile[i] / na - b.profile[i] / nb;
        sum += delta * delta;
    }
    return std::sqrt(sum);
}

const History& find_history(const std::vector<History>& histories,
                            Protocol protocol, int L, int seed_index) {
    for (const auto& history : histories)
        if (history.protocol == protocol && history.L == L && history.seed_index == seed_index)
            return history;
    std::abort();
}

const Snapshot& find_snapshot(const std::vector<History>& histories,
                              Protocol protocol, int L, int seed_index, int t_post) {
    const auto& history = find_history(histories, protocol, L, seed_index);
    return history.snapshots.at(static_cast<std::size_t>(t_post));
}

double energy_cv(const std::vector<History>& histories,
                 Protocol protocol, int L, int t_post) {
    double values[3];
    double mean = 0.0;
    for (int seed = 0; seed < 3; ++seed) {
        values[seed] = find_snapshot(histories, protocol, L, seed, t_post).local_energy;
        mean += values[seed];
    }
    mean /= 3.0;
    double variance = 0.0;
    for (double value : values) variance += (value - mean) * (value - mean);
    variance /= 3.0;
    return std::sqrt(variance) / std::max(std::fabs(mean), kNormFloor);
}

bool persistent_and_localized(const std::vector<History>& histories) {
    for (const auto& history : histories)
        for (const auto& snapshot : history.snapshots)
            if (snapshot.cluster_count != 1 || snapshot.N < 1 || !snapshot.localized)
                return false;
    return true;
}

bool protocol_converges(const std::vector<History>& histories, Protocol protocol) {
    for (int L : {33, 65})
    for (int t_post = 9; t_post <= 12; ++t_post) {
        if (energy_cv(histories, protocol, L, t_post) > kBand) return false;
        for (int seed = 0; seed < 3; ++seed) {
            const auto& current = find_snapshot(histories, protocol, L, seed, t_post);
            double max_raw = 0.0, max_shape = 0.0;
            for (int other = 0; other < 3; ++other) if (other != seed) {
                const auto& peer = find_snapshot(histories, protocol, L, other, t_post);
                max_raw = std::max(max_raw, raw_distance(current, peer));
                max_shape = std::max(max_shape, shape_distance(current, peer));
            }
            const auto& s33 = find_snapshot(histories, protocol, 33, seed, t_post);
            const auto& s65 = find_snapshot(histories, protocol, 65, seed, t_post);
            const double cross = raw_distance(s33, s65);
            if (max_raw > kBand || max_shape > kBand || cross > kBand) return false;
        }
    }
    return true;
}

bool write_details(const char* path, const std::vector<History>& histories) {
    if (path == nullptr) return true;
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out << "protocol,L,seed,t_post,dx,dy,dz,Jx,Jy,Jz,Vx,Vy,Vz,state,color,spin,flavor,"
           "cluster_count,N,charge,centroid_x,centroid_y,centroid_z,local_energy,localized,boundary_clear\n";
    out << std::setprecision(17);
    for (const auto& history : histories)
    for (const auto& snapshot : history.snapshots) {
        std::size_t offset = 0;
        for (int dx = -kLocalRadius; dx <= kLocalRadius; ++dx)
        for (int dy = -kLocalRadius; dy <= kLocalRadius; ++dy)
        for (int dz = -kLocalRadius; dz <= kLocalRadius; ++dz) {
            out << protocol_name(snapshot.protocol) << ',' << snapshot.L << ','
                << kSeeds[snapshot.seed_index].name << ',' << snapshot.t_post << ','
                << dx << ',' << dy << ',' << dz;
            for (int component = 0; component < kProfileComponents; ++component)
                out << ',' << snapshot.profile[offset++];
            out << ',' << snapshot.cluster_count << ',' << snapshot.N << ',' << snapshot.charge
                << ',' << snapshot.centroid_x << ',' << snapshot.centroid_y << ',' << snapshot.centroid_z
                << ',' << snapshot.local_energy << ',' << (snapshot.localized ? 1 : 0)
                << ',' << (snapshot.boundary_clear ? 1 : 0) << '\n';
        }
    }
    return static_cast<bool>(out);
}

void print_summary(const std::vector<History>& histories) {
    std::printf("protocol,L,seed,t_post,N,charge,centroid,local_energy,raw_distance,shape_distance,energy_cv,cross_L_distance\n");
    for (Protocol protocol : {Protocol::Dissipative, Protocol::Undamped})
    for (int L : {33, 65})
    for (int seed = 0; seed < 3; ++seed) {
        const int horizon = horizon_for(L);
        for (int t_post = 0; t_post <= horizon; ++t_post) {
            const auto& current = find_snapshot(histories, protocol, L, seed, t_post);
            double max_raw = 0.0, max_shape = 0.0;
            for (int other = 0; other < 3; ++other) if (other != seed) {
                const auto& peer = find_snapshot(histories, protocol, L, other, t_post);
                max_raw = std::max(max_raw, raw_distance(current, peer));
                max_shape = std::max(max_shape, shape_distance(current, peer));
            }
            const double cv = energy_cv(histories, protocol, L, t_post);
            double cross = std::numeric_limits<double>::quiet_NaN();
            if (t_post <= 12) {
                const auto& s33 = find_snapshot(histories, protocol, 33, seed, t_post);
                const auto& s65 = find_snapshot(histories, protocol, 65, seed, t_post);
                cross = raw_distance(s33, s65);
            }
            std::printf("%s,%d,%s,%d,%d,%d,%.17g;%.17g;%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n",
                protocol_name(protocol), L, kSeeds[seed].name, t_post,
                current.N, current.charge, current.centroid_x, current.centroid_y,
                current.centroid_z, current.local_energy, max_raw, max_shape, cv, cross);
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 2 || std::getenv("FTD_FORCE_GPU") != nullptr) {
        std::fprintf(stderr, "GATE,arguments_and_FTD_FORCE_GPU,FAIL\n");
        return 2;
    }
    std::vector<History> histories;
    bool correctness = true;
    for (Protocol protocol : {Protocol::Dissipative, Protocol::Undamped})
    for (int L : {33, 65})
    for (int seed = 0; seed < 3; ++seed) {
        History first = execute_history(protocol, L, seed);
        History duplicate = execute_history(protocol, L, seed);
        first.duplicate_identical = same_history(first, duplicate);
        correctness = correctness && first.manifested &&
            first.freeze_tick == kFreezeTickExpected && first.duplicate_identical &&
            !first.snapshots.empty();
        for (const auto& snapshot : first.snapshots)
            correctness = correctness && snapshot.backend_cpu && snapshot.toggles_exact &&
                snapshot.boundary_clear && snapshot.profile.size() == kLocalSites * kProfileComponents;
        std::fprintf(stderr, "GATE,%s,L%d,%s,manifest=%d,freeze=%d,duplicate=%d\n",
            protocol_name(protocol), L, kSeeds[seed].name, first.manifested ? 1 : 0,
            first.freeze_tick, first.duplicate_identical ? 1 : 0);
        histories.push_back(std::move(first));
    }

    if (!correctness) {
        std::fprintf(stderr, "VERDICT,INVALID\n");
        return 2;
    }

    // Comparator non-vacuity: at freeze, each arm must have at least one
    // pair outside the 1% raw or normalized-shape band.
    for (Protocol protocol : {Protocol::Dissipative, Protocol::Undamped})
    for (int L : {33, 65}) {
        double max_raw = 0.0, max_shape = 0.0;
        for (int a = 0; a < 3; ++a)
        for (int b = a + 1; b < 3; ++b) {
            const auto& sa = find_snapshot(histories, protocol, L, a, 0);
            const auto& sb = find_snapshot(histories, protocol, L, b, 0);
            max_raw = std::max(max_raw, raw_distance(sa, sb));
            max_shape = std::max(max_shape, shape_distance(sa, sb));
        }
        const bool nonvacuous = max_raw > kBand || max_shape > kBand;
        correctness = correctness && nonvacuous;
        std::fprintf(stderr, "GATE,%s,L%d,freeze_nonvacuous=%d,max_raw=%.9g,max_shape=%.9g\n",
            protocol_name(protocol), L, nonvacuous ? 1 : 0, max_raw, max_shape);
    }

    if (!write_details(argc == 2 ? argv[1] : nullptr, histories)) correctness = false;
    print_summary(histories);

    std::string verdict;
    if (!correctness) {
        verdict = "INVALID";
    } else if (!persistent_and_localized(histories)) {
        verdict = "NO-STABLE-EXCITATION";
    } else {
        const bool dissipative = protocol_converges(histories, Protocol::Dissipative);
        const bool undamped = protocol_converges(histories, Protocol::Undamped);
        if (dissipative && undamped) verdict = "SPECIES-INVARIANT";
        else if (dissipative && !undamped) verdict = "DISSIPATIVE-ATTRACTOR";
        else verdict = "HISTORY-FAMILY";
    }
    std::fprintf(stderr, "VERDICT,%s\n", verdict.c_str());
    return verdict == "INVALID" ? 2 : 0;
}
