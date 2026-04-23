#include "ftd/eft/dual_cell_blocking.h"

#include <algorithm>
#include <cmath>

namespace ftd {
namespace eft {

DualCellFields::DualCellFields(int size)
    : L(size),
      rho_cell(static_cast<size_t>(size * size * size), 0),
      phi_x(static_cast<size_t>(size * size * size), 0.0),
      phi_y(static_cast<size_t>(size * size * size), 0.0),
      phi_z(static_cast<size_t>(size * size * size), 0.0) {}

int DualCellFields::index(int x, int y, int z) const {
    x = (x % L + L) % L;
    y = (y % L + L) % L;
    z = (z % L + L) % L;
    return x * L * L + y * L + z;
}

double div_face_at(const DualCellFields& fields, int x, int y, int z) {
    const int i = fields.index(x, y, z);
    return (fields.phi_x[i] - fields.phi_x[fields.index(x - 1, y, z)]) +
           (fields.phi_y[i] - fields.phi_y[fields.index(x, y - 1, z)]) +
           (fields.phi_z[i] - fields.phi_z[fields.index(x, y, z - 1)]);
}

void set_source_from_divergence(DualCellFields& fields) {
    for (int z = 0; z < fields.L; ++z)
        for (int y = 0; y < fields.L; ++y)
            for (int x = 0; x < fields.L; ++x) {
                fields.rho_cell[fields.index(x, y, z)] =
                    static_cast<int>(std::llround(div_face_at(fields, x, y, z)));
            }
}

int total_source(const DualCellFields& fields) {
    int q = 0;
    for (int source : fields.rho_cell) q += source;
    return q;
}

double max_gauss_residual(const DualCellFields& fields) {
    double out = 0.0;
    for (int z = 0; z < fields.L; ++z)
        for (int y = 0; y < fields.L; ++y)
            for (int x = 0; x < fields.L; ++x) {
                const double residual =
                    div_face_at(fields, x, y, z) -
                    static_cast<double>(fields.rho_cell[fields.index(x, y, z)]);
                out = std::max(out, std::abs(residual));
            }
    return out;
}

DualCellFields block_dual_cell_b2(const DualCellFields& fine) {
    constexpr int b = 2;
    if (fine.L < b || (fine.L % b) != 0) return DualCellFields{};

    const int Lc = fine.L / b;
    DualCellFields coarse(Lc);

    for (int Z = 0; Z < Lc; ++Z)
        for (int Y = 0; Y < Lc; ++Y)
            for (int X = 0; X < Lc; ++X) {
                int q = 0;
                for (int dz = 0; dz < b; ++dz)
                    for (int dy = 0; dy < b; ++dy)
                        for (int dx = 0; dx < b; ++dx) {
                            q += fine.rho_cell[fine.index(
                                b * X + dx, b * Y + dy, b * Z + dz)];
                        }

                double phix = 0.0;
                double phiy = 0.0;
                double phiz = 0.0;

                for (int dz = 0; dz < b; ++dz)
                    for (int dy = 0; dy < b; ++dy) {
                        phix += fine.phi_x[fine.index(
                            b * X + (b - 1), b * Y + dy, b * Z + dz)];
                    }

                for (int dz = 0; dz < b; ++dz)
                    for (int dx = 0; dx < b; ++dx) {
                        phiy += fine.phi_y[fine.index(
                            b * X + dx, b * Y + (b - 1), b * Z + dz)];
                    }

                for (int dy = 0; dy < b; ++dy)
                    for (int dx = 0; dx < b; ++dx) {
                        phiz += fine.phi_z[fine.index(
                            b * X + dx, b * Y + dy, b * Z + (b - 1))];
                    }

                const int ci = coarse.index(X, Y, Z);
                coarse.rho_cell[ci] = q;
                coarse.phi_x[ci] = phix;
                coarse.phi_y[ci] = phiy;
                coarse.phi_z[ci] = phiz;
            }

    return coarse;
}

}  // namespace eft
}  // namespace ftd
