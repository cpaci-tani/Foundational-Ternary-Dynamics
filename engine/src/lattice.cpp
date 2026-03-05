#include "ftd/lattice.h"

namespace ftd {

Lattice::Lattice(int size) : size_(size), total_(size * size * size) {
    build_neighbor_tables();
}

int Lattice::wrap(int val) const {
    int r = val % size_;
    return r < 0 ? r + size_ : r;
}

int Lattice::index(int x, int y, int z) const {
    return wrap(x) * size_ * size_ + wrap(y) * size_ + wrap(z);
}

Coord Lattice::coord(int idx) const {
    int z = idx % size_;
    idx /= size_;
    int y = idx % size_;
    int x = idx / size_;
    return {x, y, z};
}

void Lattice::build_neighbor_tables() {
    nbr6_.resize(total_);
    nbr12_.resize(total_);
    nbr26_.resize(total_);

    for (int idx = 0; idx < total_; ++idx) {
        auto [x, y, z] = coord(idx);

        // 6 face-sharing neighbors
        nbr6_[idx] = {
            index(x+1, y, z), index(x-1, y, z),
            index(x, y+1, z), index(x, y-1, z),
            index(x, y, z+1), index(x, y, z-1)
        };

        // 12 edge-sharing neighbors (exactly 2 coords differ by ±1)
        // Used by isotropic 18-point Laplacian stencil
        nbr12_[idx] = {
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

        // 26 Moore neighborhood
        int n = 0;
        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy)
                for (int dz = -1; dz <= 1; ++dz)
                    if (dx != 0 || dy != 0 || dz != 0)
                        nbr26_[idx][n++] = index(x+dx, y+dy, z+dz);
    }
}

}  // namespace ftd
