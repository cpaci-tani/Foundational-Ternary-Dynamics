#pragma once
/**
 * 3D Cubic Lattice with periodic boundary conditions.
 *
 * Provides neighbor access (6-face, 12-edge, and 26-Moore) and
 * coordinate mapping for the FTD simulation grid.
 *
 * Neighbors are computed on-the-fly from coordinates rather than
 * stored in pre-computed tables. This eliminates 176 bytes/site
 * of memory overhead (critical for 64^3+ lattices) with negligible
 * CPU cost — the GPU engine already uses this approach.
 */

#include <array>
#include <cstdint>

namespace ftd {

struct Coord {
    int x, y, z;

    bool operator==(const Coord& o) const {
        return x == o.x && y == o.y && z == o.z;
    }
};

class Lattice {
public:
    explicit Lattice(int size)
        : size_(size), total_(static_cast<int64_t>(size) * size * size) {}

    int size() const { return size_; }
    int64_t total_sites() const { return total_; }

    // Flat index from 3D coordinates (periodic boundary)
    int index(int x, int y, int z) const {
        return wrap(x) * size_ * size_ + wrap(y) * size_ + wrap(z);
    }
    int index(const Coord& c) const { return index(c.x, c.y, c.z); }

    // 3D coordinates from flat index
    Coord coord(int idx) const {
        int z = idx % size_;
        int xy = idx / size_;
        int y = xy % size_;
        int x = xy / size_;
        return {x, y, z};
    }

    // Periodic wrapping
    int wrap(int val) const {
        int r = val % size_;
        return r < 0 ? r + size_ : r;
    }

    // 6 face-sharing neighbors (for Laplacian)
    std::array<int, 6> neighbors_6(int idx) const {
        auto [x, y, z] = coord(idx);
        return {
            index(x+1, y, z), index(x-1, y, z),
            index(x, y+1, z), index(x, y-1, z),
            index(x, y, z+1), index(x, y, z-1)
        };
    }

    // 12 edge-sharing neighbors (for isotropic 18-point Laplacian)
    std::array<int, 12> neighbors_12(int idx) const {
        auto [x, y, z] = coord(idx);
        return {
            // xy-plane edges (z fixed)
            index(x+1, y+1, z), index(x+1, y-1, z),
            index(x-1, y+1, z), index(x-1, y-1, z),
            // xz-plane edges (y fixed)
            index(x+1, y, z+1), index(x+1, y, z-1),
            index(x-1, y, z+1), index(x-1, y, z-1),
            // yz-plane edges (x fixed)
            index(x, y+1, z+1), index(x, y+1, z-1),
            index(x, y-1, z+1), index(x, y-1, z-1)
        };
    }

    // 8 body-diagonal (corner) neighbors at (±1, ±1, ±1).
    // This is the BCC sub-stencil where the master quadratic lives
    // (Watson's I_1 integral, see ontic/lemniscate.h:147).
    // See sublattice.h for the BCC-projected Laplacian using these.
    std::array<int, 8> neighbors_8_corner(int idx) const {
        auto [x, y, z] = coord(idx);
        return {
            index(x+1, y+1, z+1), index(x+1, y+1, z-1),
            index(x+1, y-1, z+1), index(x+1, y-1, z-1),
            index(x-1, y+1, z+1), index(x-1, y+1, z-1),
            index(x-1, y-1, z+1), index(x-1, y-1, z-1)
        };
    }

    // 26 Moore neighborhood neighbors
    std::array<int, 26> neighbors_26(int idx) const {
        auto [x, y, z] = coord(idx);
        std::array<int, 26> result;
        int n = 0;
        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy)
                for (int dz = -1; dz <= 1; ++dz)
                    if (dx != 0 || dy != 0 || dz != 0)
                        result[n++] = index(x+dx, y+dy, z+dz);
        return result;
    }

private:
    int size_;
    int64_t total_;
};

}  // namespace ftd
