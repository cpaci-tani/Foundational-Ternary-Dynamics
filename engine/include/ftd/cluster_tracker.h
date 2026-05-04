#pragma once
/**
 * Cluster Tracker — Connected-component cluster identification + persistence.
 *
 * Implements Class B (cluster persistence) of the Discrete-Native Derivation
 * Program (FTD-0136). See docs/theory/01_reference/SPEC_CLASS_B_CLUSTER_PERSISTENCE.md
 * for the full protocol specification.
 *
 * Per-tick flow:
 *   1. Identify connected components of manifested voxels (state != 0),
 *      grouped by state sign. Default connectivity: 6-face.
 *   2. For each component, compute overlap with previous-tick clusters.
 *   3. If max overlap >= alpha * |C_prev|, inherit C_prev's cluster_id
 *      (the cluster persists across the tick).
 *   4. Else: assign a new cluster_id (cluster birth).
 *   5. Mark previous clusters with no successor as dead.
 *
 * Pre-registered parameters (per SPEC §3.2):
 *   alpha = 0.5         — overlap threshold for persistence
 *   N_min = 4           — minimum cluster size (smallest A_{1g} multiplicity)
 *   tracking window = 1 — only adjacent ticks compared
 *
 * Native discreteness: lifetime is INTEGER tick-count. No continuous-time
 * extraction. The engine produces tau_persist in N directly.
 */

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cstdint>
#include "render_bridge.h"

namespace ftd {

// Pre-registered Class B parameters (hash-locked per FTD-0027 discipline).
// See SPEC_CLASS_B_CLUSTER_PERSISTENCE.md §3.2 + §7.
struct ClusterTrackerParams {
    double overlap_threshold = 0.5;  // alpha: |C ∩ C'| / |C| required for persistence
    int min_cluster_size = 4;        // N_min: ignore clusters smaller than this
    bool use_moore_neighbors = false;// false = 6-face (default); true = 26-Moore
};

// Snapshot of one cluster at one tick.
struct ClusterSnapshot {
    int tick = -1;
    int size = 0;
    int8_t state_sign = 0;           // +1 or -1
    double centroid_x = 0.0;
    double centroid_y = 0.0;
    double centroid_z = 0.0;
};

// Full history of one cluster from birth to death (or to current tick if alive).
struct ClusterHistory {
    int32_t cluster_id = -1;
    int birth_tick = -1;
    int death_tick = -1;             // -1 = still alive
    int8_t state_sign = 0;
    int max_size = 0;
    std::vector<ClusterSnapshot> snapshots;

    bool alive() const { return death_tick < 0; }

    int lifetime() const {
        if (snapshots.empty()) return 0;
        int last = (death_tick >= 0) ? death_tick : snapshots.back().tick;
        return last - birth_tick;
    }
};

class ClusterTracker {
public:
    explicit ClusterTracker(ClusterTrackerParams params = {})
        : params_(params), next_cluster_id_(0) {}

    // Record current state of all manifested clusters in the RenderBridge.
    // Call this once per tick (or at desired sampling interval).
    void record(const RenderBridge& rb) {
        const auto& lat = rb.lattice();
        const auto& vox = rb.voxels();
        const int tick = rb.current_tick();
        const int64_t total = lat.total_sites();

        // ----- Step 1: connected-component identification -----
        // Per-voxel cluster label (-1 = void or unlabeled).
        std::vector<int> label(total, -1);
        std::vector<std::vector<int>> components;          // voxel-index lists
        std::vector<int8_t> component_sign;                // sign of each component

        for (int64_t i = 0; i < total; ++i) {
            if (vox[i].state == 0 || label[i] != -1) continue;
            // BFS flood-fill from voxel i; only voxels with same state sign join.
            int8_t sign = vox[i].state;
            int comp_id = static_cast<int>(components.size());
            components.emplace_back();
            component_sign.push_back(sign);
            std::vector<int>& comp = components.back();

            std::vector<int> queue;
            queue.push_back(static_cast<int>(i));
            label[i] = comp_id;

            while (!queue.empty()) {
                int idx = queue.back();
                queue.pop_back();
                comp.push_back(idx);

                if (params_.use_moore_neighbors) {
                    auto n26 = lat.neighbors_26(idx);
                    for (int n : n26) push_if_match(n, sign, label, comp_id, vox, queue);
                } else {
                    auto n6 = lat.neighbors_6(idx);
                    for (int n : n6)  push_if_match(n, sign, label, comp_id, vox, queue);
                }
            }
        }

        // Filter components below min_cluster_size — relabel as -1.
        std::vector<int> kept_comp_ids;
        kept_comp_ids.reserve(components.size());
        for (size_t c = 0; c < components.size(); ++c) {
            if (static_cast<int>(components[c].size()) < params_.min_cluster_size) {
                for (int v : components[c]) label[v] = -1;
            } else {
                kept_comp_ids.push_back(static_cast<int>(c));
            }
        }

        // ----- Step 2: overlap matching with previous-tick clusters -----
        // For each kept component, compute overlap with each prev cluster.
        // Inherit cluster_id if best-overlap >= alpha * |C_prev|.
        std::vector<int32_t> assigned_id(kept_comp_ids.size(), -1);
        std::unordered_set<int32_t> claimed_ids;

        for (size_t k = 0; k < kept_comp_ids.size(); ++k) {
            int comp_idx = kept_comp_ids[k];
            const auto& voxels_in_comp = components[comp_idx];

            // Build set of prev-cluster ids hit by this component's voxels.
            std::unordered_map<int32_t, int> overlap_count;
            for (int v : voxels_in_comp) {
                auto it = prev_voxel_to_cluster_.find(v);
                if (it != prev_voxel_to_cluster_.end()) {
                    overlap_count[it->second]++;
                }
            }

            // Find prev cluster with max overlap; check if it satisfies threshold.
            int32_t best_id = -1;
            int best_overlap = 0;
            for (const auto& [pid, count] : overlap_count) {
                if (claimed_ids.count(pid)) continue;  // one-to-one matching
                if (count > best_overlap) {
                    best_overlap = count;
                    best_id = pid;
                }
            }

            if (best_id >= 0) {
                int prev_size = prev_cluster_sizes_[best_id];
                double required = params_.overlap_threshold * prev_size;
                if (best_overlap >= required) {
                    assigned_id[k] = best_id;
                    claimed_ids.insert(best_id);
                }
            }
        }

        // Birth new clusters for unmatched components.
        for (size_t k = 0; k < kept_comp_ids.size(); ++k) {
            if (assigned_id[k] < 0) {
                assigned_id[k] = next_cluster_id_++;
            }
        }

        // ----- Step 3: update histories -----
        std::unordered_map<int, int32_t> current_voxel_to_cluster;
        std::unordered_map<int32_t, int> current_cluster_sizes;

        for (size_t k = 0; k < kept_comp_ids.size(); ++k) {
            int comp_idx = kept_comp_ids[k];
            int32_t cid = assigned_id[k];
            const auto& voxels_in_comp = components[comp_idx];
            int8_t sign = component_sign[comp_idx];

            // Snapshot stats.
            ClusterSnapshot snap;
            snap.tick = tick;
            snap.size = static_cast<int>(voxels_in_comp.size());
            snap.state_sign = sign;

            // Centroid (with periodic wrapping deferred to per-need basis).
            double sx = 0, sy = 0, sz = 0;
            for (int v : voxels_in_comp) {
                Coord c = lat.coord(v);
                sx += c.x; sy += c.y; sz += c.z;
            }
            snap.centroid_x = sx / snap.size;
            snap.centroid_y = sy / snap.size;
            snap.centroid_z = sz / snap.size;

            // Find or create history.
            auto it = histories_.find(cid);
            if (it == histories_.end()) {
                ClusterHistory h;
                h.cluster_id = cid;
                h.birth_tick = tick;
                h.state_sign = sign;
                h.max_size = snap.size;
                h.snapshots.push_back(snap);
                histories_.emplace(cid, std::move(h));
            } else {
                it->second.snapshots.push_back(snap);
                if (snap.size > it->second.max_size) {
                    it->second.max_size = snap.size;
                }
            }

            // Index voxels for next tick's overlap matching.
            for (int v : voxels_in_comp) {
                current_voxel_to_cluster[v] = cid;
            }
            current_cluster_sizes[cid] = snap.size;
        }

        // ----- Step 4: mark previously-alive clusters with no successor as dead -----
        for (auto& [cid, history] : histories_) {
            if (history.death_tick >= 0) continue;          // already dead
            if (current_cluster_sizes.count(cid) == 0) {
                history.death_tick = tick;
            }
        }

        prev_voxel_to_cluster_ = std::move(current_voxel_to_cluster);
        prev_cluster_sizes_ = std::move(current_cluster_sizes);
    }

    // Access all tracked histories.
    const std::unordered_map<int32_t, ClusterHistory>& histories() const {
        return histories_;
    }

    // Get history for a specific cluster.
    const ClusterHistory* history(int32_t cluster_id) const {
        auto it = histories_.find(cluster_id);
        return (it != histories_.end()) ? &it->second : nullptr;
    }

    // Number of currently alive clusters.
    int alive_count() const {
        int count = 0;
        for (const auto& [_, h] : histories_) if (h.alive()) ++count;
        return count;
    }

    // Total tracked (alive + dead).
    int total_tracked() const { return static_cast<int>(histories_.size()); }

    // Lifetime distribution for completed (dead) clusters.
    std::vector<int> lifetime_distribution() const {
        std::vector<int> lifetimes;
        for (const auto& [_, h] : histories_) {
            if (!h.alive()) lifetimes.push_back(h.lifetime());
        }
        std::sort(lifetimes.begin(), lifetimes.end());
        return lifetimes;
    }

    // Mean lifetime of dead clusters (Class B primary observable).
    double mean_lifetime() const {
        auto lt = lifetime_distribution();
        if (lt.empty()) return 0.0;
        double sum = 0.0;
        for (int l : lt) sum += l;
        return sum / lt.size();
    }

    // Maximum cluster size observed across all tracked clusters.
    int max_size_observed() const {
        int m = 0;
        for (const auto& [_, h] : histories_) m = std::max(m, h.max_size);
        return m;
    }

    void clear() {
        histories_.clear();
        prev_voxel_to_cluster_.clear();
        prev_cluster_sizes_.clear();
        next_cluster_id_ = 0;
    }

    const ClusterTrackerParams& params() const { return params_; }

private:
    ClusterTrackerParams params_;
    int32_t next_cluster_id_;
    std::unordered_map<int32_t, ClusterHistory> histories_;

    // Carried across record() calls for overlap matching.
    std::unordered_map<int, int32_t> prev_voxel_to_cluster_;
    std::unordered_map<int32_t, int> prev_cluster_sizes_;

    static void push_if_match(int n, int8_t sign,
                              std::vector<int>& label, int comp_id,
                              const std::vector<Voxel>& vox,
                              std::vector<int>& queue) {
        if (label[n] != -1) return;
        if (vox[n].state != sign) return;
        label[n] = comp_id;
        queue.push_back(n);
    }
};

}  // namespace ftd
