// engine/include/ftd/knot_telemetry.h
#pragma once
/**
 * KnotTracker — observation-only per-knot lifecycle + diagram telemetry.
 *
 * A "knot" = a connected component of same-sign manifested voxels (s = ±1),
 * tracked across ticks with a persistent id (overlap matching, same protocol
 * as ClusterTracker / FTD-0136), enriched with flux + organization
 * (measure_cluster) and fission/fusion events (ClusterGenealogyTracker).
 *
 * OBSERVATION-ONLY: record() reads voxels()/lattice()/current_tick() and never
 * mutates the bridge → golden-hash neutral by construction (gated by
 * test_render_bridge_golden equivalence).
 *
 * Epistemic note: `org` and any fusion coupling are FTD-native proxies, NOT
 * QFT amplitudes. Lifetime/age are INTEGER tick counts (native discreteness).
 */
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <cmath>
#include "render_bridge.h"
#include "cluster_tracker.h"      // ClusterTrackerParams
#include "cluster_observables.h"  // ClusterMeasure, measure_cluster
#include "cluster_genealogy.h"    // ClusterGenealogyTracker, GenealogyEvent

namespace ftd {

// One live knot, flattened for export.
struct KnotRow {
    int32_t id = -1;
    int8_t  sign = 0;
    int birth_tick = -1;
    int age = 0;           // current_tick - birth_tick
    int size = 0;          // this tick
    int max_size = 0;      // peak over life
    double cx=0, cy=0, cz=0;          // centroid
    double vx=0, vy=0, vz=0;          // centroid velocity / tick
    double flux_mag = 0.0;            // |Σ J|
    double fdir_x=0, fdir_y=0, fdir_z=0;  // flux direction (unit)
    double org = 0.0;                 // [FTD-native proxy]
};

struct KnotAggregate {
    int alive = 0;
    int net_charge = 0;    // Σ sign over alive knots (count, not voxel charge)
    int births = 0;
    int deaths = 0;
    int fissions = 0;
    int fusions = 0;
};

class KnotTracker {
public:
    explicit KnotTracker(ClusterTrackerParams params = {}) : params_(params) {}

    void record(const RenderBridge& rb) {
        const auto& lat = rb.lattice();
        const auto& vox = rb.voxels();
        const int tick = rb.current_tick();
        last_tick_ = tick;
        const int64_t total = lat.total_sites();

        // 1) sign-grouped connected components (size-filtered).
        std::vector<int> label(total, -1);
        std::vector<std::vector<int>> comps;
        std::vector<int8_t> comp_sign;
        for (int64_t i = 0; i < total; ++i) {
            if (vox[i].state == 0 || label[i] != -1) continue;
            int8_t sign = vox[i].state;
            int cid = static_cast<int>(comps.size());
            comps.emplace_back(); comp_sign.push_back(sign);
            std::vector<int> q; q.push_back(static_cast<int>(i)); label[i] = cid;
            while (!q.empty()) {
                int idx = q.back(); q.pop_back();
                comps[cid].push_back(idx);
                auto push = [&](int n){ if (n>=0 && label[n]==-1 && vox[n].state==sign){ label[n]=cid; q.push_back(n);} };
                if (params_.use_moore_neighbors) for (int n : lat.neighbors_26(idx)) push(n);
                else                             for (int n : lat.neighbors_6(idx))  push(n);
            }
        }
        std::vector<int> kept;
        for (size_t c=0;c<comps.size();++c)
            if (static_cast<int>(comps[c].size()) >= params_.min_cluster_size) kept.push_back(static_cast<int>(c));

        // 2) overlap-match to previous tick (one-to-one, inherit id; else birth).
        std::vector<int32_t> assigned(kept.size(), -1);
        std::unordered_set<int32_t> claimed;
        for (size_t k=0;k<kept.size();++k) {
            std::unordered_map<int32_t,int> overlap;
            for (int v : comps[kept[k]]) { auto it=prev_voxel_to_id_.find(v); if (it!=prev_voxel_to_id_.end()) overlap[it->second]++; }
            int32_t best=-1; int bestN=0;
            for (auto& [pid,c] : overlap) { if (claimed.count(pid)) continue; if (c>bestN){bestN=c;best=pid;} }
            if (best>=0 && bestN >= params_.overlap_threshold * prev_size_[best]) { assigned[k]=best; claimed.insert(best); }
        }
        for (size_t k=0;k<kept.size();++k) if (assigned[k]<0) assigned[k]=next_id_++;

        // 3) measure each kept component + update histories.
        std::unordered_map<int,int32_t> cur_voxel_to_id;
        std::unordered_map<int32_t,int> cur_size;
        std::unordered_set<int32_t> seen_now;
        for (size_t k=0;k<kept.size();++k) {
            int32_t id = assigned[k];
            const auto& voxels_in = comps[kept[k]];
            int8_t sign = comp_sign[kept[k]];
            ClusterMeasure m = measure_cluster(rb, voxels_in);   // size, org, flux_sum
            double sx=0,sy=0,sz=0;
            for (int v : voxels_in) { Coord c = lat.coord(v); sx+=c.x; sy+=c.y; sz+=c.z; }
            const double n = static_cast<double>(voxels_in.size());

            Hist& h = hist_[id];
            if (h.birth_tick < 0) { h.birth_tick = tick; h.sign = sign; births_++; }
            h.death_tick = -1;
            h.sign = sign;
            h.size = static_cast<int>(voxels_in.size());
            if (h.size > h.max_size) h.max_size = h.size;
            h.prev_cx = h.cx; h.prev_cy = h.cy; h.prev_cz = h.cz; h.has_prev = (h.last_tick == tick - 1);
            h.cx = sx/n; h.cy = sy/n; h.cz = sz/n;
            const double fm = m.flux_sum.mag();
            h.flux_mag = fm;
            if (fm > 1e-12) { h.fdir_x = m.flux_sum.x/fm; h.fdir_y = m.flux_sum.y/fm; h.fdir_z = m.flux_sum.z/fm; }
            else { h.fdir_x = h.fdir_y = h.fdir_z = 0.0; }
            h.org = m.org;
            h.last_tick = tick;

            for (int v : voxels_in) cur_voxel_to_id[v] = id;
            cur_size[id] = static_cast<int>(voxels_in.size());
            seen_now.insert(id);
        }

        // 4) deaths: previously-alive ids with no successor this tick.
        for (auto& [id,h] : hist_) {
            if (h.death_tick >= 0) continue;
            if (!seen_now.count(id) && h.last_tick < tick) { h.death_tick = tick; deaths_++; }
        }

        prev_voxel_to_id_ = std::move(cur_voxel_to_id);
        prev_size_ = std::move(cur_size);

        // 5) events (fission/fusion) — reuse the genealogy tracker wholesale.
        genealogy_.record(rb);
    }

    std::vector<KnotRow> alive_knots() const {
        std::vector<KnotRow> out;
        for (const auto& [id,h] : hist_) {
            if (h.death_tick >= 0) continue;
            KnotRow r;
            r.id=id; r.sign=h.sign; r.birth_tick=h.birth_tick; r.age=last_tick_-h.birth_tick;
            r.size=h.size; r.max_size=h.max_size; r.cx=h.cx; r.cy=h.cy; r.cz=h.cz;
            if (h.has_prev) { r.vx=h.cx-h.prev_cx; r.vy=h.cy-h.prev_cy; r.vz=h.cz-h.prev_cz; }
            r.flux_mag=h.flux_mag; r.fdir_x=h.fdir_x; r.fdir_y=h.fdir_y; r.fdir_z=h.fdir_z; r.org=h.org;
            out.push_back(r);
        }
        return out;
    }

    KnotAggregate aggregate() const {
        KnotAggregate a; a.births=births_; a.deaths=deaths_;
        a.fissions=genealogy_.count(EventType::Fission);
        a.fusions =genealogy_.count(EventType::Fusion);
        for (const auto& [id,h] : hist_) if (h.death_tick<0) { a.alive++; a.net_charge += h.sign; }
        return a;
    }

    const std::vector<GenealogyEvent>& events() const { return genealogy_.events(); }
    int current_tick() const { return last_tick_; }

    void clear() {
        hist_.clear(); prev_voxel_to_id_.clear(); prev_size_.clear();
        next_id_=0; births_=0; deaths_=0; last_tick_=-1; genealogy_.clear();
    }

private:
    struct Hist {
        int birth_tick=-1, death_tick=-1, last_tick=-2, size=0, max_size=0;
        int8_t sign=0;
        double cx=0,cy=0,cz=0, prev_cx=0,prev_cy=0,prev_cz=0; bool has_prev=false;
        double flux_mag=0, fdir_x=0,fdir_y=0,fdir_z=0, org=0;
    };
    ClusterTrackerParams params_;
    std::unordered_map<int32_t,Hist> hist_;
    std::unordered_map<int,int32_t> prev_voxel_to_id_;
    std::unordered_map<int32_t,int> prev_size_;
    ClusterGenealogyTracker genealogy_{ GenealogyParams{ params_.min_cluster_size, params_.use_moore_neighbors, 2 } };
    int32_t next_id_=0;
    int births_=0, deaths_=0, last_tick_=-1;
};

}  // namespace ftd
