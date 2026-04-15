#pragma once

#include "voxel.h"
#include "lattice.h"
#include <vector>
#include <array>
#include <cassert>
#include <cstdint>
#include <unordered_map>

namespace ftd {

struct DagNodeHash {
    std::size_t operator()(const std::array<uint32_t, 8>& arr) const {
        std::size_t seed = 0;
        for (uint32_t val : arr) {
            seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

/**
 * SparseVoxelDAG
 *
 * A Directed Acyclic Graph that compresses identical octree nodes.
 * Used for scale 0 macroscopic limit logic where vast fields of the lattice
 * exist purely in the zero-flux (void) state.
 */
class SparseVoxelDAG {
public:
    explicit SparseVoxelDAG(int size) : size_(size) {
        // Precondition: size must be a power of 2 and non-zero.
        // Was throw std::invalid_argument; replaced with assert so the
        // header compiles cleanly under -fno-exceptions for the WASM build.
        assert(size != 0 && (size & (size - 1)) == 0
               && "DAG size must be a non-zero power of 2");
        depth_ = 0;
        int temp = size;
        while (temp > 1) {
            temp >>= 1;
            depth_++;
        }

        // Initialize the universal zero-leaf
        leaf_pool_.push_back(Voxel{});
        zero_leaf_index_ = 0;

        // Build the zero-tree up to the root
        uint32_t current_zero_node = zero_leaf_index_;
        for (int d = 0; d < depth_; ++d) {
            std::array<uint32_t, 8> zero_children;
            zero_children.fill(current_zero_node);
            internal_pool_.push_back(zero_children);
            current_zero_node = internal_pool_.size() - 1;
            zero_internal_indices_.push_back(current_zero_node);
        }
        root_ = current_zero_node;
    }

    int size() const { return size_; }
    int depth() const { return depth_; }
    uint32_t root() const { return root_; }

    // Determines if a pointer structurally maps identically to the universal void block for this depth dimension
    bool is_zero_node(uint32_t node_idx, int current_depth) const {
        if (current_depth == 0) return node_idx == zero_leaf_index_;
        // Inversely mapped depth: depth=0 is leaf, depth=D is root.
        return node_idx == zero_internal_indices_[current_depth - 1];
    }

    const Voxel& get_voxel(int x, int y, int z) const {
        // Wrap coordinates periodically
        x = (x % size_ + size_) % size_;
        y = (y % size_ + size_) % size_;
        z = (z % size_ + size_) % size_;

        uint32_t current_node = root_;
        int current_size = size_ >> 1;

        for (int d = 0; d < depth_; ++d) {
            int cx = (x >= current_size) ? 1 : 0;
            int cy = (y >= current_size) ? 1 : 0;
            int cz = (z >= current_size) ? 1 : 0;
            int child_idx = (cx << 2) | (cy << 1) | cz;

            current_node = internal_pool_[current_node][child_idx];

            if (cx) x -= current_size;
            if (cy) y -= current_size;
            if (cz) z -= current_size;
            current_size >>= 1;
        }

        return leaf_pool_[current_node];
    }
    
    // Set voxel (requires copy-on-write path replacement)
    // NOTE: In the complete implementation, we build new node paths up to root,
    // and deduplicate via a hash map.
    void set_voxel(int x, int y, int z, const Voxel& v) {
        // Wrap coordinates periodically
        x = (x % size_ + size_) % size_;
        y = (y % size_ + size_) % size_;
        z = (z % size_ + size_) % size_;

        // Traverse and record path
        std::vector<uint32_t> path;
        std::vector<int> indices;
        
        uint32_t current_node = root_;
        int current_size = size_ >> 1;

        int px = x, py = y, pz = z;
        for (int d = 0; d < depth_; ++d) {
            path.push_back(current_node);
            
            int cx = (px >= current_size) ? 1 : 0;
            int cy = (py >= current_size) ? 1 : 0;
            int cz = (pz >= current_size) ? 1 : 0;
            int child_idx = (cx << 2) | (cy << 1) | cz;
            
            indices.push_back(child_idx);
            
            current_node = internal_pool_[current_node][child_idx];
            
            if (cx) px -= current_size;
            if (cy) py -= current_size;
            if (cz) pz -= current_size;
            current_size >>= 1;
        }

        // Add new leaf
        uint32_t new_leaf_idx = leaf_pool_.size();
        leaf_pool_.push_back(v);

        // Reconstruct path upwards (Copy-on-Write)
        uint32_t child_val = new_leaf_idx;
        for (int d = depth_ - 1; d >= 0; --d) {
            uint32_t old_node = path[d];
            std::array<uint32_t, 8> new_children = internal_pool_[old_node];
            new_children[indices[d]] = child_val;

            // Simple deduplication strategy (in full production, use hash map pool)
            uint32_t new_internal_idx = internal_pool_.size();
            internal_pool_.push_back(new_children);
            child_val = new_internal_idx;
        }
        
        root_ = child_val;
    }

private:
    int size_;
    int depth_;
    uint32_t root_;

public:
    uint32_t zero_leaf_index_;
    std::vector<uint32_t> zero_internal_indices_;

    std::vector<std::array<uint32_t, 8>> internal_pool_;
    std::vector<Voxel> leaf_pool_;
private:
    std::unordered_map<std::array<uint32_t, 8>, uint32_t, DagNodeHash> internal_cache_;
};

} // namespace ftd
