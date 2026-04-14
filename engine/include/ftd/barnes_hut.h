#pragma once
/**
 * Universal Barnes-Hut Octree
 *
 * Implements an O(N log N) spatial partitioner that accurately preserves 
 * long-range interactions (1/r^2) via monopole summation (gravity, Coulomb).
 * Enables the FTD engine to maintain simulation legitimacy while rapidly scaling.
 */

#include "voxel.h"
#include <vector>
#include <array>
#include <algorithm>

namespace ftd {

struct BarnesHutNode {
    Vec3 center_of_mass;
    double total_mass = 0.0;
    double total_charge = 0.0; 
    Vec3 bbox_min, bbox_max;
    std::array<int, 8> children;
    std::vector<int> body_indices;
    bool is_leaf = true;

    BarnesHutNode() { children.fill(-1); }

    double width() const { return bbox_max.x - bbox_min.x; }

    int octant(const Vec3& p) const {
        Vec3 mid = {
            0.5 * (bbox_min.x + bbox_max.x),
            0.5 * (bbox_min.y + bbox_max.y),
            0.5 * (bbox_min.z + bbox_max.z)
        };
        int oct = 0;
        if (p.x >= mid.x) oct |= 1;
        if (p.y >= mid.y) oct |= 2;
        if (p.z >= mid.z) oct |= 4;
        return oct;
    }

    void child_bbox(int oct, Vec3& cmin, Vec3& cmax) const {
        Vec3 mid = {
            0.5 * (bbox_min.x + bbox_max.x),
            0.5 * (bbox_min.y + bbox_max.y),
            0.5 * (bbox_min.z + bbox_max.z)
        };
        cmin.x = (oct & 1) ? mid.x : bbox_min.x;
        cmax.x = (oct & 1) ? bbox_max.x : mid.x;
        cmin.y = (oct & 2) ? mid.y : bbox_min.y;
        cmax.y = (oct & 2) ? bbox_max.y : mid.y;
        cmin.z = (oct & 4) ? mid.z : bbox_min.z;
        cmax.z = (oct & 4) ? bbox_max.z : mid.z;
    }
};

template <typename Entity, typename PosFunc, typename MassFunc, typename ChargeFunc>
class BarnesHutTree {
public:
    std::vector<BarnesHutNode> nodes;
    int root = -1;

    void clear() {
        nodes.clear();
        root = -1;
    }

    void build(const std::vector<Entity>& bodies, PosFunc get_pos, MassFunc get_mass, ChargeFunc get_charge) {
        nodes.clear();
        root = -1;
        if (bodies.empty()) return;

        Vec3 bmin = get_pos(bodies[0]);
        Vec3 bmax = bmin;
        for (const auto& b : bodies) {
            Vec3 p = get_pos(b);
            bmin.x = std::min(bmin.x, p.x);
            bmin.y = std::min(bmin.y, p.y);
            bmin.z = std::min(bmin.z, p.z);
            bmax.x = std::max(bmax.x, p.x);
            bmax.y = std::max(bmax.y, p.y);
            bmax.z = std::max(bmax.z, p.z);
        }

        double pad = 0.01 * std::max({bmax.x - bmin.x, bmax.y - bmin.y, bmax.z - bmin.z});
        bmin.x -= pad; bmin.y -= pad; bmin.z -= pad;
        bmax.x += pad; bmax.y += pad; bmax.z += pad;

        double maxspan = std::max({bmax.x - bmin.x, bmax.y - bmin.y, bmax.z - bmin.z});
        // safeguard against 0 span
        if (maxspan < 1e-10) maxspan = 1e-10;
        bmax = {bmin.x + maxspan, bmin.y + maxspan, bmin.z + maxspan};

        nodes.reserve(bodies.size() * 4);
        BarnesHutNode rt;
        rt.bbox_min = bmin;
        rt.bbox_max = bmax;
        nodes.push_back(rt);
        root = 0;

        for (int i = 0; i < (int)bodies.size(); ++i) {
            insert_into_tree(i, root, bodies, get_pos, get_mass, get_charge);
        }
    }

private:
    void insert_into_tree(int body_idx, int node_idx, const std::vector<Entity>& bodies,
                          PosFunc get_pos, MassFunc get_mass, ChargeFunc get_charge) {
        BarnesHutNode& node = nodes[node_idx];

        if (node.is_leaf && node.body_indices.empty()) {
            node.body_indices.push_back(body_idx);
            node.center_of_mass = get_pos(bodies[body_idx]);
            node.total_mass = get_mass(bodies[body_idx]);
            node.total_charge = get_charge(bodies[body_idx]);
            return;
        }

        if (node.is_leaf) {
            if (node.width() < 1e-10) {
                // Overlapping or extremely close - cluster them
                node.body_indices.push_back(body_idx);
                double m_new = get_mass(bodies[body_idx]);
                double q_new = get_charge(bodies[body_idx]);
                double m_total = node.total_mass + m_new;
                if (m_total > 0.0) {
                    node.center_of_mass.x = (node.center_of_mass.x * node.total_mass + get_pos(bodies[body_idx]).x * m_new) / m_total;
                    node.center_of_mass.y = (node.center_of_mass.y * node.total_mass + get_pos(bodies[body_idx]).y * m_new) / m_total;
                    node.center_of_mass.z = (node.center_of_mass.z * node.total_mass + get_pos(bodies[body_idx]).z * m_new) / m_total;
                }
                node.total_mass = m_total;
                node.total_charge += q_new;
                return;
            }

            auto existing = node.body_indices;
            node.body_indices.clear();
            node.is_leaf = false;

            for (int c = 0; c < 8; ++c) {
                BarnesHutNode child;
                child.is_leaf = true;
                node.child_bbox(c, child.bbox_min, child.bbox_max);
                nodes.push_back(child);
            }
            BarnesHutNode& n = nodes[node_idx]; // refetch
            int base = (int)nodes.size() - 8;
            for (int c = 0; c < 8; ++c) {
                n.children[c] = base + c;
            }

            for (int e : existing) {
                int oct_existing = n.octant(get_pos(bodies[e]));
                insert_into_tree(e, n.children[oct_existing], bodies, get_pos, get_mass, get_charge);
            }
        }

        BarnesHutNode& n = nodes[node_idx]; // refetch
        int oct = n.octant(get_pos(bodies[body_idx]));
        if (n.children[oct] == -1) {
            BarnesHutNode child;
            child.is_leaf = true;
            n.child_bbox(oct, child.bbox_min, child.bbox_max);
            nodes.push_back(child);
            nodes[node_idx].children[oct] = (int)nodes.size() - 1;
        }
        insert_into_tree(body_idx, nodes[node_idx].children[oct], bodies, get_pos, get_mass, get_charge);

        BarnesHutNode& nn = nodes[node_idx];
        double m_new = get_mass(bodies[body_idx]);
        double q_new = get_charge(bodies[body_idx]);
        double m_total = nn.total_mass + m_new;
        if (m_total > 0.0) {
            nn.center_of_mass.x = (nn.center_of_mass.x * nn.total_mass + get_pos(bodies[body_idx]).x * m_new) / m_total;
            nn.center_of_mass.y = (nn.center_of_mass.y * nn.total_mass + get_pos(bodies[body_idx]).y * m_new) / m_total;
            nn.center_of_mass.z = (nn.center_of_mass.z * nn.total_mass + get_pos(bodies[body_idx]).z * m_new) / m_total;
        }
        nn.total_mass = m_total;
        nn.total_charge += q_new;
    }
};

} // namespace ftd
