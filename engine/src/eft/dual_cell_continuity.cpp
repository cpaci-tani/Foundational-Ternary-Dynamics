#include "ftd/eft/dual_cell_continuity.h"

#include <algorithm>
#include <cmath>

namespace ftd {
namespace eft {

DualCellContinuity::DualCellContinuity(int size)
    : L(size),
      rho_before(static_cast<size_t>(size * size * size), 0),
      rho_after(static_cast<size_t>(size * size * size), 0),
      reaction(static_cast<size_t>(size * size * size), 0),
      current_x(static_cast<size_t>(size * size * size), 0.0),
      current_y(static_cast<size_t>(size * size * size), 0.0),
      current_z(static_cast<size_t>(size * size * size), 0.0) {}

int DualCellContinuity::index(int x, int y, int z) const {
    x = (x % L + L) % L;
    y = (y % L + L) % L;
    z = (z % L + L) % L;
    return x * L * L + y * L + z;
}

double div_current_at(const DualCellContinuity& fields, int x, int y, int z) {
    const int i = fields.index(x, y, z);
    return (fields.current_x[i] - fields.current_x[fields.index(x - 1, y, z)]) +
           (fields.current_y[i] - fields.current_y[fields.index(x, y - 1, z)]) +
           (fields.current_z[i] - fields.current_z[fields.index(x, y, z - 1)]);
}

double continuity_residual_at(const DualCellContinuity& fields,
                              int x, int y, int z) {
    const int i = fields.index(x, y, z);
    return static_cast<double>(fields.rho_after[i] - fields.rho_before[i]) +
           div_current_at(fields, x, y, z) -
           static_cast<double>(fields.reaction[i]);
}

double max_continuity_residual(const DualCellContinuity& fields) {
    double out = 0.0;
    for (int z = 0; z < fields.L; ++z)
        for (int y = 0; y < fields.L; ++y)
            for (int x = 0; x < fields.L; ++x) {
                out = std::max(out, std::abs(continuity_residual_at(fields, x, y, z)));
            }
    return out;
}

int total_before(const DualCellContinuity& fields) {
    int total = 0;
    for (int value : fields.rho_before) total += value;
    return total;
}

int total_after(const DualCellContinuity& fields) {
    int total = 0;
    for (int value : fields.rho_after) total += value;
    return total;
}

int total_reaction(const DualCellContinuity& fields) {
    int total = 0;
    for (int value : fields.reaction) total += value;
    return total;
}

DualCellContinuity block_dual_cell_continuity_b2(
    const DualCellContinuity& fine) {
    constexpr int b = 2;
    if (fine.L < b || (fine.L % b) != 0) return DualCellContinuity{};

    const int Lc = fine.L / b;
    DualCellContinuity coarse(Lc);

    for (int Z = 0; Z < Lc; ++Z)
        for (int Y = 0; Y < Lc; ++Y)
            for (int X = 0; X < Lc; ++X) {
                int before = 0;
                int after = 0;
                int reaction = 0;
                for (int dz = 0; dz < b; ++dz)
                    for (int dy = 0; dy < b; ++dy)
                        for (int dx = 0; dx < b; ++dx) {
                            const int fi = fine.index(
                                b * X + dx, b * Y + dy, b * Z + dz);
                            before += fine.rho_before[fi];
                            after += fine.rho_after[fi];
                            reaction += fine.reaction[fi];
                        }

                double ix = 0.0;
                double iy = 0.0;
                double iz = 0.0;

                for (int dz = 0; dz < b; ++dz)
                    for (int dy = 0; dy < b; ++dy) {
                        ix += fine.current_x[fine.index(
                            b * X + (b - 1), b * Y + dy, b * Z + dz)];
                    }

                for (int dz = 0; dz < b; ++dz)
                    for (int dx = 0; dx < b; ++dx) {
                        iy += fine.current_y[fine.index(
                            b * X + dx, b * Y + (b - 1), b * Z + dz)];
                    }

                for (int dy = 0; dy < b; ++dy)
                    for (int dx = 0; dx < b; ++dx) {
                        iz += fine.current_z[fine.index(
                            b * X + dx, b * Y + dy, b * Z + (b - 1))];
                    }

                const int ci = coarse.index(X, Y, Z);
                coarse.rho_before[ci] = before;
                coarse.rho_after[ci] = after;
                coarse.reaction[ci] = reaction;
                coarse.current_x[ci] = ix;
                coarse.current_y[ci] = iy;
                coarse.current_z[ci] = iz;
            }

    return coarse;
}

}  // namespace eft
}  // namespace ftd
