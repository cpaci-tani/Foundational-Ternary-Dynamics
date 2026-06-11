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
        if (val >= 0 && val < size_) return val;
        if (val >= size_ && val < 2 * size_) return val - size_;
        if (val >= -size_ && val < 0) return val + size_;
        int r = val % size_;
        return r < 0 ? r + size_ : r;
    }

    // Coordinate-based neighbor methods to avoid decoding indices via divisions/modulos
    std::array<int, 6> neighbors_6(int x, int y, int z) const {
        int xm = (x == 0) ? size_ - 1 : x - 1;
        int xp = (x == size_ - 1) ? 0 : x + 1;
        int ym = (y == 0) ? size_ - 1 : y - 1;
        int yp = (y == size_ - 1) ? 0 : y + 1;
        int zm = (z == 0) ? size_ - 1 : z - 1;
        int zp = (z == size_ - 1) ? 0 : z + 1;

        int size2 = size_ * size_;
        return {
            xp * size2 + y * size_ + z,
            xm * size2 + y * size_ + z,
            x * size2 + yp * size_ + z,
            x * size2 + ym * size_ + z,
            x * size2 + y * size_ + zp,
            x * size2 + y * size_ + zm
        };
    }

    std::array<int, 12> neighbors_12(int x, int y, int z) const {
        int xm = (x == 0) ? size_ - 1 : x - 1;
        int xp = (x == size_ - 1) ? 0 : x + 1;
        int ym = (y == 0) ? size_ - 1 : y - 1;
        int yp = (y == size_ - 1) ? 0 : y + 1;
        int zm = (z == 0) ? size_ - 1 : z - 1;
        int zp = (z == size_ - 1) ? 0 : z + 1;

        int size2 = size_ * size_;
        return {
            // xy-plane edges (z fixed)
            xp * size2 + yp * size_ + z, xp * size2 + ym * size_ + z,
            xm * size2 + yp * size_ + z, xm * size2 + ym * size_ + z,
            // xz-plane edges (y fixed)
            xp * size2 + y * size_ + zp, xp * size2 + y * size_ + zm,
            xm * size2 + y * size_ + zp, xm * size2 + y * size_ + zm,
            // yz-plane edges (x fixed)
            x * size2 + yp * size_ + zp, x * size2 + yp * size_ + zm,
            x * size2 + ym * size_ + zp, x * size2 + ym * size_ + zm
        };
    }

    std::array<int, 8> neighbors_8_corner(int x, int y, int z) const {
        int xm = (x == 0) ? size_ - 1 : x - 1;
        int xp = (x == size_ - 1) ? 0 : x + 1;
        int ym = (y == 0) ? size_ - 1 : y - 1;
        int yp = (y == size_ - 1) ? 0 : y + 1;
        int zm = (z == 0) ? size_ - 1 : z - 1;
        int zp = (z == size_ - 1) ? 0 : z + 1;

        int size2 = size_ * size_;
        return {
            xp * size2 + yp * size_ + zp, xp * size2 + yp * size_ + zm,
            xp * size2 + ym * size_ + zp, xp * size2 + ym * size_ + zm,
            xm * size2 + yp * size_ + zp, xm * size2 + yp * size_ + zm,
            xm * size2 + ym * size_ + zp, xm * size2 + ym * size_ + zm
        };
    }

    std::array<int, 26> neighbors_26(int x, int y, int z) const {
        int xm = (x == 0) ? size_ - 1 : x - 1;
        int xp = (x == size_ - 1) ? 0 : x + 1;
        int ym = (y == 0) ? size_ - 1 : y - 1;
        int yp = (y == size_ - 1) ? 0 : y + 1;
        int zm = (z == 0) ? size_ - 1 : z - 1;
        int zp = (z == size_ - 1) ? 0 : z + 1;

        int size2 = size_ * size_;
        return {
            // dx = -1
            xm * size2 + ym * size_ + zm, xm * size2 + ym * size_ + z, xm * size2 + ym * size_ + zp,
            xm * size2 + y * size_ + zm,  xm * size2 + y * size_ + z,  xm * size2 + y * size_ + zp,
            xm * size2 + yp * size_ + zm, xm * size2 + yp * size_ + z, xm * size2 + yp * size_ + zp,

            // dx = 0
            x * size2 + ym * size_ + zm,  x * size2 + ym * size_ + z,  x * size2 + ym * size_ + zp,
            x * size2 + y * size_ + zm,                                x * size2 + y * size_ + zp,
            x * size2 + yp * size_ + zm,  x * size2 + yp * size_ + z,  x * size2 + yp * size_ + zp,

            // dx = 1
            xp * size2 + ym * size_ + zm, xp * size2 + ym * size_ + z, xp * size2 + ym * size_ + zp,
            xp * size2 + y * size_ + zm,  xp * size2 + y * size_ + z,  xp * size2 + y * size_ + zp,
            xp * size2 + yp * size_ + zm, xp * size2 + yp * size_ + z, xp * size2 + yp * size_ + zp
        };
    }

    // 6 face-sharing neighbors (for Laplacian)
    std::array<int, 6> neighbors_6(int idx) const {
        int z = idx % size_;
        int xy = idx / size_;
        int y = xy % size_;
        int x = xy / size_;
        return neighbors_6(x, y, z);
    }

    // 12 edge-sharing neighbors (for isotropic 18-point Laplacian)
    std::array<int, 12> neighbors_12(int idx) const {
        int z = idx % size_;
        int xy = idx / size_;
        int y = xy % size_;
        int x = xy / size_;
        return neighbors_12(x, y, z);
    }

    // 8 body-diagonal (corner) neighbors at (±1, ±1, ±1).
    // This is the BCC sub-stencil where the master quadratic lives
    // (Watson's I_1 integral, see ontic/lemniscate.h:147).
    // See sublattice.h for the BCC-projected Laplacian using these.
    std::array<int, 8> neighbors_8_corner(int idx) const {
        int z = idx % size_;
        int xy = idx / size_;
        int y = xy % size_;
        int x = xy / size_;
        return neighbors_8_corner(x, y, z);
    }

    // 26 Moore neighborhood neighbors
    std::array<int, 26> neighbors_26(int idx) const {
        int z = idx % size_;
        int xy = idx / size_;
        int y = xy % size_;
        int x = xy / size_;
        return neighbors_26(x, y, z);
    }

private:
    int size_;
    int64_t total_;
};

}  // namespace ftd
