#pragma once
/**
 * Cluster genealogy tracker — merge/split (fusion/fission) event detection.
 *
 * EXPLORATORY instrumentation for the cluster-thermodynamics conjecture
 * (.claude/plans/lazy-conjuring-marble.md). NOT a derived FTD claim, NOT
 * pre-registered.
 *
 * Why a new class and not ClusterTracker: ClusterTracker's overlap matcher is
 * strictly one-to-one (a `claimed_ids` set), so a merge silently kills one
 * parent and a split silently births one child — it cannot represent
 * fusion/fission. This tracker keeps the FULL bipartite overlap structure
 * between consecutive ticks and classifies each connected parent⇄child group
 * as Birth / Death / Persist / Fission / Fusion / Ambiguous, with size +
 * organization + detuning accounting.
 *
 * Classification of a connected (parent-set P, child-set C) group:
 *   |P|=0, |C|≥1  -> Birth
 *   |P|=1, |C|=1  -> Persist   (not recorded, to keep the CSV to events)
 *   |P|=1, |C|≥2  -> Fission   (P2: should be conservative ΣN_child ≈ N_parent)
 *   |P|≥2, |C|=1  -> Fusion    (P2: compatibility-gated; P3: should be lossy ΣN_child < ΣN_parent)
 *   |P|≥2, |C|≥2  -> Ambiguous (dropped from clean P2/P3 stats; counted)
 *   a parent in no child-bearing group -> Death
 *
 * Reuses the sign-grouped BFS connected-component finder of ClusterTracker.
 */

#include "render_bridge.h"
#include "cluster_observables.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cstdint>
#include <string>
#include <fstream>

namespace ftd {

struct GenealogyParams {
    int  min_cluster_size   = 4;
    bool use_moore_neighbors = true;
    int  min_overlap_voxels = 2;  // a parent⇄child link needs >= this many shared voxels
};

enum class EventType { Birth, Death, Persist, Fission, Fusion, Ambiguous };

inline const char* event_name(EventType e) {
    switch (e) {
        case EventType::Birth:     return "Birth";
        case EventType::Death:     return "Death";
        case EventType::Persist:   return "Persist";
        case EventType::Fission:   return "Fission";
        case EventType::Fusion:    return "Fusion";
        case EventType::Ambiguous: return "Ambiguous";
    }
    return "?";
}

struct GenealogyEvent {
    int       tick = -1;
    EventType type = EventType::Persist;
    std::vector<int32_t> parent_ids;
    std::vector<int32_t> child_ids;
    int    sum_parent_size = 0, sum_child_size = 0;
    double sum_parent_org  = 0.0, sum_child_org  = 0.0;
    double detuning_proxy  = 0.0;  // Fusion: angle between the 2 largest parents' flux dirs
    int8_t state_sign = 0;
};

class ClusterGenealogyTracker {
public:
    explicit ClusterGenealogyTracker(GenealogyParams p = {}) : params_(p) {}

    void record(const RenderBridge& rb) {
        const auto& lat = rb.lattice();
        const auto& vox = rb.voxels();
        const int tick = rb.current_tick();
        const int64_t total = lat.total_sites();

        // ----- BFS connected components, sign-grouped, size-filtered -----
        std::vector<int> label(total, -1);
        std::vector<std::vector<int>> comps;
        std::vector<int8_t> comp_sign;
        for (int64_t i = 0; i < total; ++i) {
            if (vox[i].state == 0 || label[i] != -1) continue;
            int8_t sign = vox[i].state;
            int cid = static_cast<int>(comps.size());
            comps.emplace_back();
            comp_sign.push_back(sign);
            std::vector<int> q;
            q.push_back(static_cast<int>(i));
            label[i] = cid;
            while (!q.empty()) {
                int idx = q.back(); q.pop_back();
                comps[cid].push_back(idx);
                if (params_.use_moore_neighbors) {
                    for (int n : lat.neighbors_26(idx)) push_if(n, sign, label, cid, vox, q);
                } else {
                    for (int n : lat.neighbors_6(idx)) push_if(n, sign, label, cid, vox, q);
                }
            }
        }
        std::vector<int> kept;
        for (size_t c = 0; c < comps.size(); ++c)
            if (static_cast<int>(comps[c].size()) >= params_.min_cluster_size)
                kept.push_back(static_cast<int>(c));
        const int K = static_cast<int>(kept.size());

        std::vector<ClusterMeasure> cm(K);
        for (int k = 0; k < K; ++k) cm[k] = measure_cluster(rb, comps[kept[k]]);

        // ----- bipartite overlap: prev_id <-> current comp k -----
        std::vector<std::unordered_map<int32_t,int>> comp_overlap(K);
        for (int k = 0; k < K; ++k)
            for (int v : comps[kept[k]]) {
                auto it = prev_voxel_to_id_.find(v);
                if (it != prev_voxel_to_id_.end()) comp_overlap[k][it->second]++;
            }

        std::vector<int32_t> prev_ids;
        prev_ids.reserve(prev_id_size_.size());
        for (auto& kv : prev_id_size_) prev_ids.push_back(kv.first);
        std::unordered_map<int32_t,int> pid_index;
        for (int p = 0; p < static_cast<int>(prev_ids.size()); ++p) pid_index[prev_ids[p]] = p;
        const int P = static_cast<int>(prev_ids.size());

        // ----- DSU over parents [0..P) and children [P..P+K) -----
        std::vector<int> dsu(P + K);
        std::iota(dsu.begin(), dsu.end(), 0);
        std::function<int(int)> find = [&](int a) {
            while (dsu[a] != a) { dsu[a] = dsu[dsu[a]]; a = dsu[a]; }
            return a;
        };
        auto uni = [&](int a, int b) { dsu[find(a)] = find(b); };
        for (int k = 0; k < K; ++k)
            for (auto& [pid, cnt] : comp_overlap[k])
                if (cnt >= params_.min_overlap_voxels) {
                    auto pit = pid_index.find(pid);
                    if (pit != pid_index.end()) uni(pit->second, P + k);
                }

        std::unordered_map<int, std::vector<int>> grp_parents, grp_children;
        for (int p = 0; p < P; ++p) grp_parents[find(p)].push_back(p);
        for (int k = 0; k < K; ++k) grp_children[find(P + k)].push_back(k);
        std::unordered_set<int> groups;
        for (int p = 0; p < P; ++p) groups.insert(find(p));
        for (int k = 0; k < K; ++k) groups.insert(find(P + k));

        std::vector<int32_t> comp_id(K, -1);
        std::unordered_set<int32_t> survived_prev;

        for (int g : groups) {
            auto cit = grp_children.find(g);
            int nc = (cit != grp_children.end()) ? static_cast<int>(cit->second.size()) : 0;
            if (nc == 0) continue;  // pure-parent group -> deaths handled below
            auto pit = grp_parents.find(g);
            int np = (pit != grp_parents.end()) ? static_cast<int>(pit->second.size()) : 0;

            std::vector<int>& kids = cit->second;
            std::sort(kids.begin(), kids.end(),
                      [&](int a, int b) { return cm[a].size > cm[b].size; });

            int32_t inherit_id = -1;
            if (np > 0) {
                int32_t best = -1; int bestsz = -1;
                for (int p : pit->second) {
                    int32_t pid = prev_ids[p];
                    survived_prev.insert(pid);
                    int sz = prev_id_size_[pid];
                    if (sz > bestsz) { bestsz = sz; best = pid; }
                }
                inherit_id = best;
            }
            for (size_t j = 0; j < kids.size(); ++j)
                comp_id[kids[j]] = (j == 0 && inherit_id >= 0) ? inherit_id : next_id_++;

            GenealogyEvent ev;
            ev.tick = tick;
            ev.state_sign = comp_sign[kept[kids[0]]];
            if (np > 0) for (int p : pit->second) ev.parent_ids.push_back(prev_ids[p]);
            for (int k : kids) ev.child_ids.push_back(comp_id[k]);
            for (int32_t pid : ev.parent_ids) {
                ev.sum_parent_size += prev_id_size_[pid];
                ev.sum_parent_org  += prev_id_org_[pid];
            }
            for (int k : kids) { ev.sum_child_size += cm[k].size; ev.sum_child_org += cm[k].org; }

            if      (np == 0 && nc >= 1) ev.type = EventType::Birth;
            else if (np == 1 && nc == 1) ev.type = EventType::Persist;
            else if (np == 1 && nc >= 2) ev.type = EventType::Fission;
            else if (np >= 2 && nc == 1) {
                ev.type = EventType::Fusion;
                std::vector<int>& pars = pit->second;
                std::sort(pars.begin(), pars.end(), [&](int a, int b) {
                    return prev_id_size_[prev_ids[a]] > prev_id_size_[prev_ids[b]];
                });
                if (pars.size() >= 2)
                    ev.detuning_proxy = detuning_proxy(prev_id_fluxdir_[prev_ids[pars[0]]],
                                                       prev_id_fluxdir_[prev_ids[pars[1]]]);
            } else ev.type = EventType::Ambiguous;

            if (ev.type != EventType::Persist) events_.push_back(ev);
        }

        // ----- deaths: prev ids with no surviving successor -----
        for (int p = 0; p < P; ++p) {
            int32_t pid = prev_ids[p];
            if (survived_prev.count(pid)) continue;
            GenealogyEvent ev;
            ev.tick = tick; ev.type = EventType::Death;
            ev.parent_ids = {pid};
            ev.sum_parent_size = prev_id_size_[pid];
            ev.sum_parent_org  = prev_id_org_[pid];
            events_.push_back(ev);
        }

        // ----- roll current -> prev -----
        prev_voxel_to_id_.clear(); prev_id_size_.clear();
        prev_id_org_.clear();      prev_id_fluxdir_.clear();
        for (int k = 0; k < K; ++k) {
            int32_t cid = comp_id[k];
            for (int v : comps[kept[k]]) prev_voxel_to_id_[v] = cid;
            prev_id_size_[cid] = cm[k].size;
            prev_id_org_[cid]  = cm[k].org;
            Vec3 fs = cm[k].flux_sum; double mg = fs.mag();
            prev_id_fluxdir_[cid] = (mg > 1e-12) ? Vec3(fs.x / mg, fs.y / mg, fs.z / mg) : Vec3();
        }
    }

    const std::vector<GenealogyEvent>& events() const { return events_; }
    std::vector<GenealogyEvent> fissions() const { return filter(EventType::Fission); }
    std::vector<GenealogyEvent> fusions()  const { return filter(EventType::Fusion); }

    int count(EventType t) const {
        int n = 0; for (const auto& e : events_) if (e.type == t) ++n; return n;
    }

    void clear() {
        events_.clear(); prev_voxel_to_id_.clear(); prev_id_size_.clear();
        prev_id_org_.clear(); prev_id_fluxdir_.clear(); next_id_ = 0;
    }

    void write_csv(const std::string& path) const {
        std::ofstream f(path);
        f << "tick,event_type,n_parents,n_children,sum_parent_size,sum_child_size,"
             "sum_parent_org,sum_child_org,detuning_proxy,state_sign\n";
        for (const auto& e : events_) {
            f << e.tick << "," << event_name(e.type) << "," << e.parent_ids.size() << ","
              << e.child_ids.size() << "," << e.sum_parent_size << "," << e.sum_child_size << ","
              << e.sum_parent_org << "," << e.sum_child_org << "," << e.detuning_proxy << ","
              << static_cast<int>(e.state_sign) << "\n";
        }
    }

private:
    GenealogyParams params_;
    int32_t next_id_ = 0;
    std::vector<GenealogyEvent> events_;
    std::unordered_map<int,int32_t>   prev_voxel_to_id_;
    std::unordered_map<int32_t,int>   prev_id_size_;
    std::unordered_map<int32_t,double> prev_id_org_;
    std::unordered_map<int32_t,Vec3>  prev_id_fluxdir_;

    std::vector<GenealogyEvent> filter(EventType t) const {
        std::vector<GenealogyEvent> out;
        for (const auto& e : events_) if (e.type == t) out.push_back(e);
        return out;
    }

    static void push_if(int n, int8_t sign, std::vector<int>& label, int cid,
                        const std::vector<Voxel>& vox, std::vector<int>& q) {
        if (label[n] != -1) return;
        if (vox[n].state != sign) return;
        label[n] = cid;
        q.push_back(n);
    }
};

}  // namespace ftd
