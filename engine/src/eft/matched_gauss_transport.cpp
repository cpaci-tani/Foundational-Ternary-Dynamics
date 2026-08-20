#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>

namespace ftd::eft {
namespace {

int wrap(int value, int L) {
    const int r = value % L;
    return r < 0 ? r + L : r;
}

int flat_index(int L, int x, int y, int z) {
    return wrap(x, L) * L * L + wrap(y, L) * L + wrap(z, L);
}

void coordinates(int L, int index, int& x, int& y, int& z) {
    z = index % L;
    const int xy = index / L;
    y = xy % L;
    x = xy / L;
}

int shortest_delta(int from, int to, int L) {
    int delta = to - from;
    if (delta > L / 2) delta -= L;
    if (delta < -L / 2) delta += L;
    return delta;
}

void add_face_step(MatchedFaceFlux& field,
                   int x, int y, int z,
                   int axis, int direction, double amount) {
    if (direction == 0) return;
    if (axis == 0) {
        if (direction > 0) field.x[field.index(x, y, z)] += amount;
        else field.x[field.index(x - 1, y, z)] -= amount;
    } else if (axis == 1) {
        if (direction > 0) field.y[field.index(x, y, z)] += amount;
        else field.y[field.index(x, y - 1, z)] -= amount;
    } else {
        if (direction > 0) field.z[field.index(x, y, z)] += amount;
        else field.z[field.index(x, y, z - 1)] -= amount;
    }
}

long double dot_product(const std::vector<double>& a,
                        const std::vector<double>& b) {
    long double out = 0.0L;
    for (std::size_t i = 0; i < a.size(); ++i)
        out += static_cast<long double>(a[i]) *
               static_cast<long double>(b[i]);
    return out;
}

void apply_ddt(int L, const std::vector<double>& scalar,
               std::vector<double>& out) {
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int i = flat_index(L, x, y, z);
                out[static_cast<std::size_t>(i)] =
                    6.0 * scalar[static_cast<std::size_t>(i)] -
                    scalar[static_cast<std::size_t>(flat_index(L, x + 1, y, z))] -
                    scalar[static_cast<std::size_t>(flat_index(L, x - 1, y, z))] -
                    scalar[static_cast<std::size_t>(flat_index(L, x, y + 1, z))] -
                    scalar[static_cast<std::size_t>(flat_index(L, x, y - 1, z))] -
                    scalar[static_cast<std::size_t>(flat_index(L, x, y, z + 1))] -
                    scalar[static_cast<std::size_t>(flat_index(L, x, y, z - 1))];
            }
        }
    }
}

double max_abs(const std::vector<double>& values) {
    double out = 0.0;
    for (double value : values) out = std::max(out, std::abs(value));
    return out;
}

bool finite(const MatchedFaceFlux& field) {
    const auto all_finite = [](const std::vector<double>& values) {
        return std::all_of(values.begin(), values.end(),
                           [](double value) { return std::isfinite(value); });
    };
    return all_finite(field.x) && all_finite(field.y) && all_finite(field.z);
}

bool finite(const MatchedEdgeField& field) {
    const auto all_finite = [](const std::vector<double>& values) {
        return std::all_of(values.begin(), values.end(),
                           [](double value) { return std::isfinite(value); });
    };
    return all_finite(field.x) && all_finite(field.y) && all_finite(field.z);
}

}  // namespace

MatchedEdgeField::MatchedEdgeField(int size)
    : L(size),
      x(static_cast<std::size_t>(size * size * size), 0.0),
      y(static_cast<std::size_t>(size * size * size), 0.0),
      z(static_cast<std::size_t>(size * size * size), 0.0) {}

int MatchedEdgeField::index(int x_coord, int y_coord, int z_coord) const {
    return flat_index(L, x_coord, y_coord, z_coord);
}

MatchedFaceFlux::MatchedFaceFlux(int size)
    : L(size),
      x(static_cast<std::size_t>(size * size * size), 0.0),
      y(static_cast<std::size_t>(size * size * size), 0.0),
      z(static_cast<std::size_t>(size * size * size), 0.0) {}

int MatchedFaceFlux::index(int x_coord, int y_coord, int z_coord) const {
    return flat_index(L, x_coord, y_coord, z_coord);
}

bool seed_dipole_path(MatchedFaceFlux& field,
                      int source_index,
                      int sink_index,
                      double amount) {
    const int total = field.L * field.L * field.L;
    if (field.L <= 0 || source_index < 0 || source_index >= total ||
        sink_index < 0 || sink_index >= total || source_index == sink_index ||
        !std::isfinite(amount) || amount == 0.0) {
        return false;
    }

    int x = 0, y = 0, z = 0;
    int tx = 0, ty = 0, tz = 0;
    coordinates(field.L, source_index, x, y, z);
    coordinates(field.L, sink_index, tx, ty, tz);
    const int deltas[3] = {
        shortest_delta(x, tx, field.L),
        shortest_delta(y, ty, field.L),
        shortest_delta(z, tz, field.L),
    };

    for (int axis = 0; axis < 3; ++axis) {
        const int direction = (deltas[axis] > 0) - (deltas[axis] < 0);
        for (int step = 0; step < std::abs(deltas[axis]); ++step) {
            add_face_step(field, x, y, z, axis, direction, amount);
            if (axis == 0) x = wrap(x + direction, field.L);
            if (axis == 1) y = wrap(y + direction, field.L);
            if (axis == 2) z = wrap(z + direction, field.L);
        }
    }
    return x == tx && y == ty && z == tz;
}

MatchedTransportUpdate apply_conservative_current(
    MatchedFaceFlux& field,
    const DualCellContinuity& history,
    double tolerance) {
    MatchedTransportUpdate out;
    if (field.L <= 0 || history.L != field.L) return out;

    out.reaction_l1 = total_reaction_l1(history);
    out.transport_residual = max_continuity_residual(history);
    out.current_l1 = total_current_l1(history);
    if (out.reaction_l1 != 0 || out.transport_residual > tolerance) return out;

    const std::size_t expected = field.x.size();
    if (history.current_x.size() != expected ||
        history.current_y.size() != expected ||
        history.current_z.size() != expected) {
        return out;
    }
    for (std::size_t i = 0; i < expected; ++i) {
        field.x[i] -= history.current_x[i];
        field.y[i] -= history.current_y[i];
        field.z[i] -= history.current_z[i];
    }
    out.valid = true;
    return out;
}

MatchedFaceFlux matched_curl(const MatchedEdgeField& edge) {
    MatchedFaceFlux out(edge.L);
    if (edge.L <= 0) return out;
    for (int x = 0; x < edge.L; ++x) {
        for (int y = 0; y < edge.L; ++y) {
            for (int z = 0; z < edge.L; ++z) {
                const int i = edge.index(x, y, z);
                const int xm = edge.index(x - 1, y, z);
                const int ym = edge.index(x, y - 1, z);
                const int zm = edge.index(x, y, z - 1);
                out.x[static_cast<std::size_t>(i)] =
                    edge.z[static_cast<std::size_t>(i)] -
                    edge.z[static_cast<std::size_t>(ym)] -
                    edge.y[static_cast<std::size_t>(i)] +
                    edge.y[static_cast<std::size_t>(zm)];
                out.y[static_cast<std::size_t>(i)] =
                    edge.x[static_cast<std::size_t>(i)] -
                    edge.x[static_cast<std::size_t>(zm)] -
                    edge.z[static_cast<std::size_t>(i)] +
                    edge.z[static_cast<std::size_t>(xm)];
                out.z[static_cast<std::size_t>(i)] =
                    edge.y[static_cast<std::size_t>(i)] -
                    edge.y[static_cast<std::size_t>(xm)] -
                    edge.x[static_cast<std::size_t>(i)] +
                    edge.x[static_cast<std::size_t>(ym)];
            }
        }
    }
    return out;
}

MatchedEdgeField matched_curl_adjoint(const MatchedFaceFlux& face) {
    MatchedEdgeField out(face.L);
    if (face.L <= 0) return out;
    for (int x = 0; x < face.L; ++x) {
        for (int y = 0; y < face.L; ++y) {
            for (int z = 0; z < face.L; ++z) {
                const int i = face.index(x, y, z);
                const int xp = face.index(x + 1, y, z);
                const int yp = face.index(x, y + 1, z);
                const int zp = face.index(x, y, z + 1);
                out.x[static_cast<std::size_t>(i)] =
                    face.z[static_cast<std::size_t>(yp)] -
                    face.z[static_cast<std::size_t>(i)] -
                    face.y[static_cast<std::size_t>(zp)] +
                    face.y[static_cast<std::size_t>(i)];
                out.y[static_cast<std::size_t>(i)] =
                    face.x[static_cast<std::size_t>(zp)] -
                    face.x[static_cast<std::size_t>(i)] -
                    face.z[static_cast<std::size_t>(xp)] +
                    face.z[static_cast<std::size_t>(i)];
                out.z[static_cast<std::size_t>(i)] =
                    face.y[static_cast<std::size_t>(xp)] -
                    face.y[static_cast<std::size_t>(i)] -
                    face.x[static_cast<std::size_t>(yp)] +
                    face.x[static_cast<std::size_t>(i)];
            }
        }
    }
    return out;
}

double apply_transverse_curl(MatchedFaceFlux& field,
                             const MatchedEdgeField& edge,
                             double scale) {
    if (field.L != edge.L || !std::isfinite(scale)) return 0.0;
    const auto curl = matched_curl(edge);
    double norm = 0.0;
    for (std::size_t i = 0; i < field.x.size(); ++i) {
        field.x[i] += scale * curl.x[i];
        field.y[i] += scale * curl.y[i];
        field.z[i] += scale * curl.z[i];
        norm += std::abs(scale * curl.x[i]);
        norm += std::abs(scale * curl.y[i]);
        norm += std::abs(scale * curl.z[i]);
    }
    return norm;
}

double divergence_at(const MatchedFaceFlux& field,
                     int x, int y, int z) {
    const int i = field.index(x, y, z);
    return field.x[static_cast<std::size_t>(i)] -
               field.x[static_cast<std::size_t>(field.index(x - 1, y, z))] +
           field.y[static_cast<std::size_t>(i)] -
               field.y[static_cast<std::size_t>(field.index(x, y - 1, z))] +
           field.z[static_cast<std::size_t>(i)] -
               field.z[static_cast<std::size_t>(field.index(x, y, z - 1))];
}

double max_divergence(const MatchedFaceFlux& field) {
    double out = 0.0;
    for (int x = 0; x < field.L; ++x)
        for (int y = 0; y < field.L; ++y)
            for (int z = 0; z < field.L; ++z)
                out = std::max(out, std::abs(divergence_at(field, x, y, z)));
    return out;
}

double max_curl_adjoint(const MatchedFaceFlux& field) {
    const auto curl = matched_curl_adjoint(field);
    double out = 0.0;
    for (double value : curl.x) out = std::max(out, std::abs(value));
    for (double value : curl.y) out = std::max(out, std::abs(value));
    for (double value : curl.z) out = std::max(out, std::abs(value));
    return out;
}

double max_gauss_residual(const MatchedFaceFlux& field,
                          const std::vector<int>& site_source) {
    const std::size_t expected =
        static_cast<std::size_t>(field.L * field.L * field.L);
    if (field.L <= 0 || site_source.size() != expected) return INFINITY;
    double out = 0.0;
    for (int x = 0; x < field.L; ++x)
        for (int y = 0; y < field.L; ++y)
            for (int z = 0; z < field.L; ++z) {
                const int i = field.index(x, y, z);
                out = std::max(out, std::abs(
                    divergence_at(field, x, y, z) -
                    static_cast<double>(site_source[static_cast<std::size_t>(i)])));
            }
    return out;
}

double l1_norm(const MatchedFaceFlux& field) {
    double out = 0.0;
    for (double value : field.x) out += std::abs(value);
    for (double value : field.y) out += std::abs(value);
    for (double value : field.z) out += std::abs(value);
    return out;
}

double l1_norm(const MatchedEdgeField& field) {
    double out = 0.0;
    for (double value : field.x) out += std::abs(value);
    for (double value : field.y) out += std::abs(value);
    for (double value : field.z) out += std::abs(value);
    return out;
}

double quadratic_energy(const MatchedFaceFlux& field) {
    long double out = 0.0L;
    for (double value : field.x) out += static_cast<long double>(value) * value;
    for (double value : field.y) out += static_cast<long double>(value) * value;
    for (double value : field.z) out += static_cast<long double>(value) * value;
    return static_cast<double>(0.5L * out);
}

double quadratic_energy(const MatchedEdgeField& field) {
    long double out = 0.0L;
    for (double value : field.x) out += static_cast<long double>(value) * value;
    for (double value : field.y) out += static_cast<long double>(value) * value;
    for (double value : field.z) out += static_cast<long double>(value) * value;
    return static_cast<double>(0.5L * out);
}

MatchedSurfaceCharge measure_face_cube_charge(const MatchedFaceFlux& field,
                                              int cx, int cy, int cz,
                                              int radius) {
    MatchedSurfaceCharge out;
    out.radius = radius;
    if (field.L <= 0 || radius < 0 || 2 * radius + 1 >= field.L) return out;
    const int ax = cx - radius;
    const int bx = cx + radius;
    const int ay = cy - radius;
    const int by = cy + radius;
    const int az = cz - radius;
    const int bz = cz + radius;

    for (int y = ay; y <= by; ++y)
        for (int z = az; z <= bz; ++z)
            out.boundary_flux +=
                field.x[static_cast<std::size_t>(field.index(bx, y, z))] -
                field.x[static_cast<std::size_t>(field.index(ax - 1, y, z))];
    for (int x = ax; x <= bx; ++x)
        for (int z = az; z <= bz; ++z)
            out.boundary_flux +=
                field.y[static_cast<std::size_t>(field.index(x, by, z))] -
                field.y[static_cast<std::size_t>(field.index(x, ay - 1, z))];
    for (int x = ax; x <= bx; ++x)
        for (int y = ay; y <= by; ++y)
            out.boundary_flux +=
                field.z[static_cast<std::size_t>(field.index(x, y, bz))] -
                field.z[static_cast<std::size_t>(field.index(x, y, az - 1))];

    for (int x = ax; x <= bx; ++x)
        for (int y = ay; y <= by; ++y)
            for (int z = az; z <= bz; ++z) {
                out.divergence_sum += divergence_at(field, x, y, z);
                ++out.enclosed_sites;
            }
    out.telescope_residual = out.boundary_flux - out.divergence_sum;
    return out;
}

MatchedEdgeField make_transverse_challenge(int L, double amplitude) {
    MatchedEdgeField out(L);
    if (L <= 0 || !std::isfinite(amplitude)) return out;
    constexpr double pi = 3.141592653589793238462643383279502884;
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int i = out.index(x, y, z);
                const double px = 2.0 * pi * static_cast<double>(x) / L;
                const double py = 2.0 * pi * static_cast<double>(y) / L;
                const double pz = 2.0 * pi * static_cast<double>(z) / L;
                out.x[static_cast<std::size_t>(i)] =
                    amplitude * (std::sin(py) + 0.5 * std::cos(pz));
                out.y[static_cast<std::size_t>(i)] =
                    amplitude * (std::sin(pz) + 0.5 * std::cos(px));
                out.z[static_cast<std::size_t>(i)] =
                    amplitude * (std::sin(px) + 0.5 * std::cos(py));
            }
        }
    }
    return out;
}

MatchedGaussDynamics::MatchedGaussDynamics(int size)
    : electric_(size), magnetic_half_(size) {}

void MatchedGaussDynamics::reset(int size) {
    electric_ = MatchedFaceFlux(size);
    magnetic_half_ = MatchedEdgeField(size);
    initialized_ = false;
    initialization_result_ = {};
    last_step_ = {};
}

MatchedMinimumEnergyResult MatchedGaussDynamics::initialize_minimum_energy(
    const std::vector<int>& site_source,
    double tolerance,
    int max_iterations) {
    MatchedMinimumEnergyResult result;
    const int L = electric_.L;
    const std::size_t count = static_cast<std::size_t>(L * L * L);
    if (L <= 0 || site_source.size() != count || tolerance <= 0.0 ||
        !std::isfinite(tolerance)) {
        initialization_result_ = result;
        return result;
    }

    long long total = 0;
    long long source_l1 = 0;
    for (int value : site_source) {
        total += value;
        source_l1 += std::abs(value);
    }
    result.neutral = total == 0;
    if (!result.neutral) {
        initialization_result_ = result;
        return result;
    }

    reset(L);
    if (source_l1 == 0) {
        result.valid = true;
        result.converged = true;
        initialized_ = true;
        initialization_result_ = result;
        return result;
    }

    if (max_iterations <= 0) max_iterations = 12 * L;
    std::vector<double> phi(count, 0.0);
    std::vector<double> residual(count, 0.0);
    std::vector<double> direction(count, 0.0);
    std::vector<double> image(count, 0.0);
    for (std::size_t i = 0; i < count; ++i) {
        residual[i] = static_cast<double>(site_source[i]);
        direction[i] = residual[i];
    }

    long double rr = dot_product(residual, residual);
    for (int iteration = 1; iteration <= max_iterations; ++iteration) {
        apply_ddt(L, direction, image);
        const long double pAp = dot_product(direction, image);
        if (!(pAp > 0.0L) || !std::isfinite(static_cast<double>(pAp))) break;
        const long double alpha = rr / pAp;
        for (std::size_t i = 0; i < count; ++i) {
            phi[i] += static_cast<double>(alpha * direction[i]);
            residual[i] -= static_cast<double>(alpha * image[i]);
        }
        result.iterations = iteration;
        result.solver_residual = max_abs(residual);
        if (result.solver_residual <= tolerance) {
            result.converged = true;
            break;
        }
        const long double rr_next = dot_product(residual, residual);
        if (!(rr_next >= 0.0L) || !std::isfinite(static_cast<double>(rr_next))) break;
        const long double beta = rr_next / rr;
        for (std::size_t i = 0; i < count; ++i)
            direction[i] = residual[i] + static_cast<double>(beta * direction[i]);
        rr = rr_next;
    }

    if (!result.converged) {
        initialization_result_ = result;
        return result;
    }

    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int i = electric_.index(x, y, z);
                electric_.x[static_cast<std::size_t>(i)] =
                    phi[static_cast<std::size_t>(i)] -
                    phi[static_cast<std::size_t>(electric_.index(x + 1, y, z))];
                electric_.y[static_cast<std::size_t>(i)] =
                    phi[static_cast<std::size_t>(i)] -
                    phi[static_cast<std::size_t>(electric_.index(x, y + 1, z))];
                electric_.z[static_cast<std::size_t>(i)] =
                    phi[static_cast<std::size_t>(i)] -
                    phi[static_cast<std::size_t>(electric_.index(x, y, z + 1))];
            }
        }
    }
    result.gauss_residual = max_gauss_residual(electric_, site_source);
    result.curl_adjoint_residual = max_curl_adjoint(electric_);
    result.electric_energy = quadratic_energy(electric_);
    result.valid = finite(electric_) && result.gauss_residual <= 10.0 * tolerance;
    initialized_ = result.valid;
    initialization_result_ = result;
    return result;
}

MatchedWaveStep MatchedGaussDynamics::advance(
    const DualCellContinuity& history,
    double wave_speed,
    double dt,
    double tolerance) {
    MatchedWaveStep result;
    if (!initialized_ || history.L != electric_.L ||
        !std::isfinite(wave_speed) || wave_speed < 0.0 ||
        !std::isfinite(dt) || dt <= 0.0) {
        last_step_ = result;
        return result;
    }

    result.transport.reaction_l1 = total_reaction_l1(history);
    result.transport.transport_residual = max_continuity_residual(history);
    result.transport.current_l1 = total_current_l1(history);
    if (result.transport.reaction_l1 != 0 ||
        result.transport.transport_residual > tolerance) {
        last_step_ = result;
        return result;
    }

    const double scale = wave_speed * dt;
    result.energy_before = modified_energy(wave_speed, dt);
    const auto electric_curl = matched_curl_adjoint(electric_);
    for (std::size_t i = 0; i < magnetic_half_.x.size(); ++i) {
        magnetic_half_.x[i] -= scale * electric_curl.x[i];
        magnetic_half_.y[i] -= scale * electric_curl.y[i];
        magnetic_half_.z[i] -= scale * electric_curl.z[i];
    }
    const auto magnetic_curl = matched_curl(magnetic_half_);
    for (std::size_t i = 0; i < electric_.x.size(); ++i) {
        electric_.x[i] += scale * magnetic_curl.x[i];
        electric_.y[i] += scale * magnetic_curl.y[i];
        electric_.z[i] += scale * magnetic_curl.z[i];
    }
    result.transport = apply_conservative_current(
        electric_, history, tolerance);
    if (!result.transport.valid) {
        last_step_ = result;
        return result;
    }

    result.gauss_residual = max_gauss_residual(electric_, history.rho_after);
    result.energy_after = modified_energy(wave_speed, dt);
    result.electric_l1 = l1_norm(electric_);
    result.magnetic_l1 = l1_norm(magnetic_half_);
    result.valid = finite(electric_) && finite(magnetic_half_) &&
        std::isfinite(result.energy_after) &&
        result.gauss_residual <= 10.0 * tolerance;
    last_step_ = result;
    return result;
}

bool MatchedGaussDynamics::inject_transverse_edge_potential(
    int x, int y, int z, int axis, double amplitude) {
    if (!initialized_ || axis < 0 || axis > 2 || !std::isfinite(amplitude))
        return false;
    MatchedEdgeField potential(electric_.L);
    const int i = potential.index(x, y, z);
    if (axis == 0) potential.x[static_cast<std::size_t>(i)] = amplitude;
    if (axis == 1) potential.y[static_cast<std::size_t>(i)] = amplitude;
    if (axis == 2) potential.z[static_cast<std::size_t>(i)] = amplitude;
    return apply_transverse_curl(electric_, potential) > 0.0;
}

Vec3 MatchedGaussDynamics::centered_electric_at(int x, int y, int z) const {
    if (!initialized_) return {};
    const int i = electric_.index(x, y, z);
    return {
        0.5 * (electric_.x[static_cast<std::size_t>(i)] +
               electric_.x[static_cast<std::size_t>(electric_.index(x - 1, y, z))]),
        0.5 * (electric_.y[static_cast<std::size_t>(i)] +
               electric_.y[static_cast<std::size_t>(electric_.index(x, y - 1, z))]),
        0.5 * (electric_.z[static_cast<std::size_t>(i)] +
               electric_.z[static_cast<std::size_t>(electric_.index(x, y, z - 1))]),
    };
}

double MatchedGaussDynamics::modified_energy(double wave_speed, double dt) const {
    if (!initialized_) return 0.0;
    const auto electric_curl = matched_curl_adjoint(electric_);
    long double cross = 0.0L;
    for (std::size_t i = 0; i < magnetic_half_.x.size(); ++i) {
        cross += static_cast<long double>(magnetic_half_.x[i]) * electric_curl.x[i];
        cross += static_cast<long double>(magnetic_half_.y[i]) * electric_curl.y[i];
        cross += static_cast<long double>(magnetic_half_.z[i]) * electric_curl.z[i];
    }
    return quadratic_energy(electric_) + quadratic_energy(magnetic_half_) -
           0.5 * wave_speed * dt * static_cast<double>(cross);
}

void MatchedGaussDynamics::adopt_state(MatchedFaceFlux electric,
                                       MatchedEdgeField magnetic,
                                       const MatchedWaveStep& step) {
    electric_ = std::move(electric);
    magnetic_half_ = std::move(magnetic);
    last_step_ = step;
    initialized_ = electric_.L > 0 && electric_.L == magnetic_half_.L;
}

}  // namespace ftd::eft
