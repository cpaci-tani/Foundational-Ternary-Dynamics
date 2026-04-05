#pragma once
/**
 * Particle Tracker — Trajectory recording using persistent particle_id.
 *
 * Physics justification: The engine assigns monotonically increasing
 * particle_id at genesis and transfers it during movement. This tracker
 * uses that existing infrastructure to record (x,y,z,t) trajectories
 * without adding any new dynamics. Trajectories enable:
 *   - Mean free path measurement
 *   - Lifetime distributions
 *   - Effective mass from F=ma (inertial mass)
 *   - Binding time for composite structures
 *   - Scattering cross-section estimates
 *
 * This is pure instrumentation — no physics is added or modified.
 */

#include <vector>
#include <unordered_map>
#include <cmath>
#include "render_bridge.h"

namespace ftd {

struct TrajectoryPoint {
    int x, y, z;
    int tick;
    int8_t state;
    double density;
    double speed;
};

struct ParticleHistory {
    int32_t particle_id;
    int genesis_tick = -1;   // tick when first observed
    int death_tick = -1;     // tick when disappeared (-1 = still alive)
    int8_t initial_state = 0;
    std::vector<TrajectoryPoint> trajectory;

    int lifetime() const {
        if (trajectory.empty()) return 0;
        int last = (death_tick >= 0) ? death_tick : trajectory.back().tick;
        return last - genesis_tick;
    }

    // Minimum-image displacement for periodic boundaries
    static int wrap_delta(int d, int L) {
        if (d >  L/2) d -= L;
        if (d < -L/2) d += L;
        return d;
    }

    // Mean speed over trajectory (with periodic boundary wrapping)
    double mean_speed(int lattice_size = 0) const {
        if (trajectory.size() < 2) return 0.0;
        double sum = 0.0;
        for (size_t i = 1; i < trajectory.size(); ++i) {
            int dx = trajectory[i].x - trajectory[i-1].x;
            int dy = trajectory[i].y - trajectory[i-1].y;
            int dz = trajectory[i].z - trajectory[i-1].z;
            if (lattice_size > 0) {
                dx = wrap_delta(dx, lattice_size);
                dy = wrap_delta(dy, lattice_size);
                dz = wrap_delta(dz, lattice_size);
            }
            sum += std::sqrt(static_cast<double>(dx*dx + dy*dy + dz*dz));
        }
        return sum / (trajectory.size() - 1);
    }

    // Displacement from start to end (with periodic boundary wrapping)
    double net_displacement(int lattice_size = 0) const {
        if (trajectory.size() < 2) return 0.0;
        // Accumulate wrapped displacements for true physical displacement
        double total_dx = 0, total_dy = 0, total_dz = 0;
        for (size_t i = 1; i < trajectory.size(); ++i) {
            int dx = trajectory[i].x - trajectory[i-1].x;
            int dy = trajectory[i].y - trajectory[i-1].y;
            int dz = trajectory[i].z - trajectory[i-1].z;
            if (lattice_size > 0) {
                dx = wrap_delta(dx, lattice_size);
                dy = wrap_delta(dy, lattice_size);
                dz = wrap_delta(dz, lattice_size);
            }
            total_dx += dx;
            total_dy += dy;
            total_dz += dz;
        }
        return std::sqrt(total_dx*total_dx + total_dy*total_dy + total_dz*total_dz);
    }
};

class Tracker {
public:
    // Record current state of all particles in the RenderBridge.
    // Call this once per tick (or at desired sampling interval).
    void record(const RenderBridge& rb) {
        const auto& lat = rb.lattice();
        const auto& vox = rb.voxels();
        int tick = rb.current_tick();

        // Track which particles we see this tick
        std::vector<int32_t> seen;

        for (int i = 0; i < lat.total_sites(); ++i) {
            if (vox[i].state == 0) continue;
            int32_t pid = vox[i].particle_id;
            if (pid < 0) continue;

            seen.push_back(pid);

            auto it = histories_.find(pid);
            if (it == histories_.end()) {
                // New particle — create history
                ParticleHistory h;
                h.particle_id = pid;
                h.genesis_tick = tick;
                h.initial_state = vox[i].state;
                auto [new_it, _] = histories_.emplace(pid, std::move(h));
                it = new_it;
            }

            Coord c = lat.coord(i);
            TrajectoryPoint pt;
            pt.x = c.x;
            pt.y = c.y;
            pt.z = c.z;
            pt.tick = tick;
            pt.state = vox[i].state;
            pt.density = vox[i].density();
            pt.speed = vox[i].speed();
            it->second.trajectory.push_back(pt);
        }

        // Mark deaths: particles seen before but not this tick
        for (auto& [pid, h] : histories_) {
            if (h.death_tick >= 0) continue;  // already dead
            bool found = false;
            for (int32_t s : seen) {
                if (s == pid) { found = true; break; }
            }
            if (!found && !h.trajectory.empty() && h.trajectory.back().tick < tick) {
                h.death_tick = tick;
            }
        }
    }

    // Access all tracked histories
    const std::unordered_map<int32_t, ParticleHistory>& histories() const {
        return histories_;
    }

    // Get history for a specific particle
    const ParticleHistory* history(int32_t particle_id) const {
        auto it = histories_.find(particle_id);
        return (it != histories_.end()) ? &it->second : nullptr;
    }

    // Count of all tracked particles (alive + dead)
    int total_tracked() const { return static_cast<int>(histories_.size()); }

    // Count of currently alive particles
    int alive_count() const {
        int count = 0;
        for (const auto& [_, h] : histories_)
            if (h.death_tick < 0) ++count;
        return count;
    }

    // Lifetime distribution for all completed (dead) particles
    std::vector<int> lifetime_distribution() const {
        std::vector<int> lifetimes;
        for (const auto& [_, h] : histories_) {
            if (h.death_tick >= 0) {
                lifetimes.push_back(h.lifetime());
            }
        }
        std::sort(lifetimes.begin(), lifetimes.end());
        return lifetimes;
    }

    // Mean lifetime of dead particles
    double mean_lifetime() const {
        auto lt = lifetime_distribution();
        if (lt.empty()) return 0.0;
        double sum = 0.0;
        for (int l : lt) sum += l;
        return sum / lt.size();
    }

    void clear() { histories_.clear(); }

private:
    std::unordered_map<int32_t, ParticleHistory> histories_;
};

}  // namespace ftd
