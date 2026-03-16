#pragma once
/**
 * 3D Cubic Lattice with periodic boundary conditions.
 *
 * Provides neighbor access (6-face and 26-Moore) and
 * coordinate mapping for the FTD simulation grid.
 */

#include <array>
#include <cstdint>
#include <vector>

namespace ftd {

struct Coord {
    int x, y, z;

    bool operator==(const Coord& o) const {
        return x == o.x && y == o.y && z == o.z;
    }
};

class Lattice {
public:
    explicit Lattice(int size);

    int size() const { return size_; }
    int64_t total_sites() const { return total_; }

    // Flat index from 3D coordinates (periodic boundary)
    int index(int x, int y, int z) const;
    int index(const Coord& c) const { return index(c.x, c.y, c.z); }

    // 3D coordinates from flat index
    Coord coord(int idx) const;

    // Periodic wrapping
    int wrap(int val) const;

    // 6 face-sharing neighbors (for Laplacian) — O(1) table lookup
    const std::array<int, 6>& neighbors_6(int idx) const { return nbr6_[idx]; }

    // 12 edge-sharing neighbors (for isotropic 18-point Laplacian) — O(1) table lookup
    const std::array<int, 12>& neighbors_12(int idx) const { return nbr12_[idx]; }

    // 26 Moore neighborhood neighbors — O(1) table lookup
    const std::array<int, 26>& neighbors_26(int idx) const { return nbr26_[idx]; }

private:
    void build_neighbor_tables();

    int size_;
    int64_t total_;  // int64_t to avoid overflow for size >= 1290 (1290^3 > INT_MAX)
    std::vector<std::array<int, 6>>  nbr6_;   // Pre-computed 6-face neighbors
    std::vector<std::array<int, 12>> nbr12_;   // Pre-computed 12-edge neighbors
    std::vector<std::array<int, 26>> nbr26_;   // Pre-computed 26-Moore neighbors
};

}  // namespace ftd
