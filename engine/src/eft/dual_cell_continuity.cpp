#include "ftd/eft/dual_cell_continuity.h"

#include <algorithm>
#include <cmath>

namespace ftd {
namespace eft {
namespace {

int wrap(int value, int L) {
    const int r = value % L;
    return r < 0 ? r + L : r;
}

int flat_index(int L, int x, int y, int z) {
    return wrap(x, L) * L * L + wrap(y, L) * L + wrap(z, L);
}

void coord_from_index(int L, int idx, int& x, int& y, int& z) {
    z = idx % L;
    const int xy = idx / L;
    y = xy % L;
    x = xy / L;
}

int nearest_step(int from, int to, int L) {
    if (wrap(from + 1, L) == to) return +1;
    if (wrap(from - 1, L) == to) return -1;
    if (wrap(from, L) == to) return 0;
    return 2;
}

void add_oriented_face_current(DualCellContinuity& out,
                               int x, int y, int z,
                               int axis, int dir, int charge) {
    if (dir == 0) return;

    if (axis == 0) {
        if (dir > 0) {
            out.current_x[out.index(x, y, z)] += static_cast<double>(charge);
        } else {
            out.current_x[out.index(x - 1, y, z)] -= static_cast<double>(charge);
        }
    } else if (axis == 1) {
        if (dir > 0) {
            out.current_y[out.index(x, y, z)] += static_cast<double>(charge);
        } else {
            out.current_y[out.index(x, y - 1, z)] -= static_cast<double>(charge);
        }
    } else {
        if (dir > 0) {
            out.current_z[out.index(x, y, z)] += static_cast<double>(charge);
        } else {
            out.current_z[out.index(x, y, z - 1)] -= static_cast<double>(charge);
        }
    }
}

void route_moore_current(DualCellContinuity& out,
                         int sx, int sy, int sz,
                         int dx, int dy, int dz,
                         int charge) {
    int x = sx;
    int y = sy;
    int z = sz;

    add_oriented_face_current(out, x, y, z, 0, dx, charge);
    x = wrap(x + dx, out.L);
    add_oriented_face_current(out, x, y, z, 1, dy, charge);
    y = wrap(y + dy, out.L);
    add_oriented_face_current(out, x, y, z, 2, dz, charge);
}

bool is_moore_neighbor(int L, int src, int dst,
                       int& sx, int& sy, int& sz,
                       int& dx, int& dy, int& dz) {
    int tx = 0;
    int ty = 0;
    int tz = 0;
    coord_from_index(L, src, sx, sy, sz);
    coord_from_index(L, dst, tx, ty, tz);

    dx = nearest_step(sx, tx, L);
    dy = nearest_step(sy, ty, L);
    dz = nearest_step(sz, tz, L);
    return std::abs(dx) <= 1 && std::abs(dy) <= 1 && std::abs(dz) <= 1 &&
           (dx != 0 || dy != 0 || dz != 0);
}

}  // namespace

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

double total_current_l1(const DualCellContinuity& fields) {
    double total = 0.0;
    for (double value : fields.current_x) total += std::abs(value);
    for (double value : fields.current_y) total += std::abs(value);
    for (double value : fields.current_z) total += std::abs(value);
    return total;
}

int total_reaction_l1(const DualCellContinuity& fields) {
    int total = 0;
    for (int value : fields.reaction) total += std::abs(value);
    return total;
}

DualCellOperatorMoments measure_operator_moments(
    const DualCellContinuity& fields) {
    DualCellOperatorMoments moments;
    if (fields.L <= 0) return moments;

    moments.current_l1 = total_current_l1(fields);
    moments.reaction_l1 = total_reaction_l1(fields);

    for (int z = 0; z < fields.L; ++z)
        for (int y = 0; y < fields.L; ++y)
            for (int x = 0; x < fields.L; ++x) {
                const int i = fields.index(x, y, z);
                moments.delta_rho_l1 += std::abs(
                    static_cast<double>(fields.rho_after[i] -
                                        fields.rho_before[i]));
                moments.div_current_l1 += std::abs(
                    div_current_at(fields, x, y, z));
                moments.residual_linf = std::max(
                    moments.residual_linf,
                    std::abs(continuity_residual_at(fields, x, y, z)));
            }
    return moments;
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

bool accumulate_continuity_step(DualCellContinuity& interval,
                                const DualCellContinuity& step) {
    if (step.L <= 0) return false;
    const size_t expected = static_cast<size_t>(step.L * step.L * step.L);
    if (step.rho_before.size() != expected ||
        step.rho_after.size() != expected ||
        step.reaction.size() != expected ||
        step.current_x.size() != expected ||
        step.current_y.size() != expected ||
        step.current_z.size() != expected) {
        return false;
    }

    if (interval.L == 0) {
        interval = step;
        return true;
    }

    if (interval.L != step.L ||
        interval.rho_before.size() != expected ||
        interval.rho_after.size() != expected ||
        interval.reaction.size() != expected ||
        interval.current_x.size() != expected ||
        interval.current_y.size() != expected ||
        interval.current_z.size() != expected) {
        return false;
    }

    interval.rho_after = step.rho_after;
    for (size_t i = 0; i < expected; ++i) {
        interval.reaction[i] += step.reaction[i];
        interval.current_x[i] += step.current_x[i];
        interval.current_y[i] += step.current_y[i];
        interval.current_z[i] += step.current_z[i];
    }
    return true;
}

DualCellHistoryExtraction extract_moore_history_from_snapshots(
    int L,
    const std::vector<int>& rho_before,
    const std::vector<int>& rho_after,
    DualCellContinuity& out) {
    DualCellHistoryExtraction report;
    const size_t expected = static_cast<size_t>(L * L * L);
    if (L <= 0 || rho_before.size() != expected ||
        rho_after.size() != expected) {
        out = DualCellContinuity{};
        return report;
    }

    out = DualCellContinuity(L);
    out.rho_before = rho_before;
    out.rho_after = rho_after;

    std::vector<int> delta(expected, 0);
    for (size_t i = 0; i < expected; ++i) {
        delta[i] = rho_after[i] - rho_before[i];
    }

    // Adjacent opposite charges disappearing in the same tick are the signed
    // snapshot signature of an annihilation event. From states alone this is
    // locally indistinguishable from a charge crossing into an opposite site,
    // so the native history convention records it as reaction, not transport.
    for (int src = 0; src < static_cast<int>(expected); ++src) {
        const int q = rho_before[static_cast<size_t>(src)];
        if (q == 0 || rho_after[static_cast<size_t>(src)] != 0 ||
            delta[static_cast<size_t>(src)] != -q) {
            continue;
        }

        int sx = 0, sy = 0, sz = 0;
        coord_from_index(L, src, sx, sy, sz);
        bool paired = false;
        for (int dz = -1; dz <= 1 && !paired; ++dz)
            for (int dy = -1; dy <= 1 && !paired; ++dy)
                for (int dx = -1; dx <= 1 && !paired; ++dx) {
                    if (dx == 0 && dy == 0 && dz == 0) continue;
                    const int dst = flat_index(L, sx + dx, sy + dy, sz + dz);
                    if (dst <= src) continue;
                    if (rho_before[static_cast<size_t>(dst)] == -q &&
                        rho_after[static_cast<size_t>(dst)] == 0 &&
                        delta[static_cast<size_t>(dst)] == q) {
                        out.reaction[static_cast<size_t>(src)] +=
                            delta[static_cast<size_t>(src)];
                        out.reaction[static_cast<size_t>(dst)] +=
                            delta[static_cast<size_t>(dst)];
                        delta[static_cast<size_t>(src)] = 0;
                        delta[static_cast<size_t>(dst)] = 0;
                        ++report.annihilation_pairs;
                        paired = true;
                    }
                }
    }

    // Pair remaining void-target Moore moves and route each diagonal through a
    // deterministic x/y/z chain of oriented face currents.
    for (int src = 0; src < static_cast<int>(expected); ++src) {
        const int q = rho_before[static_cast<size_t>(src)];
        if (q == 0 || rho_after[static_cast<size_t>(src)] != 0 ||
            delta[static_cast<size_t>(src)] != -q) {
            continue;
        }

        int best = -1;
        int bsx = 0, bsy = 0, bsz = 0;
        int bdx = 0, bdy = 0, bdz = 0;
        for (int dst = 0; dst < static_cast<int>(expected); ++dst) {
            if (rho_before[static_cast<size_t>(dst)] != 0 ||
                rho_after[static_cast<size_t>(dst)] != q ||
                delta[static_cast<size_t>(dst)] != q) {
                continue;
            }

            int sx = 0, sy = 0, sz = 0;
            int dx = 0, dy = 0, dz = 0;
            if (is_moore_neighbor(L, src, dst, sx, sy, sz, dx, dy, dz)) {
                best = dst;
                bsx = sx;
                bsy = sy;
                bsz = sz;
                bdx = dx;
                bdy = dy;
                bdz = dz;
                break;
            }
        }

        if (best >= 0) {
            route_moore_current(out, bsx, bsy, bsz, bdx, bdy, bdz, q);
            delta[static_cast<size_t>(src)] += q;
            delta[static_cast<size_t>(best)] -= q;
            ++report.transported_events;
        }
    }

    for (size_t i = 0; i < expected; ++i) {
        if (delta[i] != 0) {
            out.reaction[i] += delta[i];
            ++report.reaction_sites;
            delta[i] = 0;
        }
    }

    report.valid = true;
    return report;
}

}  // namespace eft
}  // namespace ftd
