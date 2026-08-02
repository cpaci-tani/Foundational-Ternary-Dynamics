#include "ftd/eft/momentum_transport_current.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>

namespace ftd::eft {
namespace {

using Triplet = std::array<std::vector<double>, 3>;

int wrap_index(int value, int L) {
  const int r = value % L;
  return r < 0 ? r + L : r;
}

std::size_t flat(int L, int x, int y, int z) {
  return (static_cast<std::size_t>(wrap_index(x, L)) * L
          + static_cast<std::size_t>(wrap_index(y, L))) * L
      + static_cast<std::size_t>(wrap_index(z, L));
}

int shortest(int coordinate, int L) {
  int value = wrap_index(coordinate, L);
  if (value > L / 2) value -= L;
  return value;
}

Triplet make_triplet(int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return Triplet{std::vector<double>(count, 0.0),
                 std::vector<double>(count, 0.0),
                 std::vector<double>(count, 0.0)};
}

MatchedFaceFlux as_face(int L, const Triplet& value) {
  MatchedFaceFlux result(L);
  result.x = value[0];
  result.y = value[1];
  result.z = value[2];
  return result;
}

MatchedEdgeField as_edge(int L, const Triplet& value) {
  MatchedEdgeField result(L);
  result.x = value[0];
  result.y = value[1];
  result.z = value[2];
  return result;
}

template <typename Field>
Triplet from_field(const Field& field) {
  return Triplet{field.x, field.y, field.z};
}

Triplet view_triplet(const MomentumFieldView& field) {
  const auto count = static_cast<std::size_t>(field.L) * field.L * field.L;
  Triplet result;
  for (int a = 0; a < 3; ++a)
    result[static_cast<std::size_t>(a)].assign(field.component[a],
                                               field.component[a] + count);
  return result;
}

/// C: edge -> face, transcribed via the frozen engine routine.
Triplet apply_curl(int L, const Triplet& edge) {
  return from_field(matched_curl(as_edge(L, edge)));
}

/// C^T: face -> edge, transcribed via the frozen engine routine.
Triplet apply_curl_adjoint(int L, const Triplet& face) {
  return from_field(matched_curl_adjoint(as_face(L, face)));
}

/// D_i, componentwise.  matched_central_derivative is blind to the staggering
/// of its argument (it is a pure translation difference), so the face carrier
/// serves edge fields identically.
Triplet apply_central(int L, const Triplet& value, int axis) {
  return from_field(matched_central_derivative(as_face(L, value), axis));
}

Triplet subtract(const Triplet& lhs, const Triplet& rhs) {
  Triplet result = lhs;
  for (int a = 0; a < 3; ++a)
    for (std::size_t i = 0; i < result[static_cast<std::size_t>(a)].size(); ++i)
      result[static_cast<std::size_t>(a)][i] -=
          rhs[static_cast<std::size_t>(a)][i];
  return result;
}

constexpr double kStencilThreshold = 1e-12;

struct StencilKey {
  int r[3]{};
  bool operator<(const StencilKey& other) const {
    for (int axis = 0; axis < 3; ++axis) {
      if (r[axis] != other.r[axis]) return r[axis] < other.r[axis];
    }
    return false;
  }
};

using StencilMap = std::map<StencilKey, std::array<std::array<double, 3>, 3>>;

Triplet apply_operator_triplet(MomentumOperatorKind kind, int component,
                               int L, const Triplet& field) {
  if (kind == MomentumOperatorKind::CentralDifference)
    return apply_central(L, field, component);
  if (kind == MomentumOperatorKind::FaceBinding)
    return apply_central(L, apply_curl(L, apply_curl_adjoint(L, field)),
                         component);
  return apply_central(L, apply_curl_adjoint(L, apply_curl(L, field)),
                       component);
}

/**
 * Sec 2.2 R+ selection (frozen with this implementation; Banned move B5):
 * pair (r,a,b) with (-r,b,a) and keep the member whose key
 * (r_x,r_y,r_z,a,b) is lexicographically greater.  The keys are never equal:
 * equality needs r = -r and a = b, and N_0 is antisymmetric so its diagonal
 * vanishes.
 */
bool greater_key(const int r[3], int a, int b) {
  const std::array<int, 5> left{{r[0], r[1], r[2], a, b}};
  const std::array<int, 5> right{{-r[0], -r[1], -r[2], b, a}};
  return left > right;
}

}  // namespace

// ---------------------------------------------------------------------------
// Views
// ---------------------------------------------------------------------------

std::size_t MomentumFieldView::index(int x, int y, int z) const {
  return flat(L, x, y, z);
}

double MomentumFieldView::at(int a, int x, int y, int z) const {
  return component[a][index(x, y, z)];
}

MomentumFieldView momentum_view(const MatchedFaceFlux& field) {
  MomentumFieldView result;
  result.L = field.L;
  result.component[0] = field.x.data();
  result.component[1] = field.y.data();
  result.component[2] = field.z.data();
  return result;
}

MomentumFieldView momentum_view(const MatchedEdgeField& field) {
  MomentumFieldView result;
  result.L = field.L;
  result.component[0] = field.x.data();
  result.component[1] = field.y.data();
  result.component[2] = field.z.data();
  return result;
}

// ---------------------------------------------------------------------------
// Masks
// ---------------------------------------------------------------------------

bool MomentumMask::valid() const {
  if (L <= 0) return false;
  if (!per_component) return universal || radius >= 0;
  const auto count = static_cast<std::size_t>(L) * L * L;
  for (int a = 0; a < 3; ++a)
    if (component[static_cast<std::size_t>(a)].size() != count) return false;
  return true;
}

bool MomentumMask::inside(int a, int x, int y, int z) const {
  bool raw = false;
  if (per_component) {
    raw = component[static_cast<std::size_t>(a)][flat(L, x, y, z)] != 0;
  } else if (universal) {
    raw = true;
  } else {
    const int dx = std::abs(shortest(x - center[0], L));
    const int dy = std::abs(shortest(y - center[1], L));
    const int dz = std::abs(shortest(z - center[2], L));
    raw = std::max({dx, dy, dz}) <= radius;
  }
  return complemented ? !raw : raw;
}

MomentumMask make_momentum_site_mask(int L, int cx, int cy, int cz,
                                     int radius) {
  MomentumMask result;
  result.L = L;
  result.center[0] = wrap_index(cx, std::max(L, 1));
  result.center[1] = wrap_index(cy, std::max(L, 1));
  result.center[2] = wrap_index(cz, std::max(L, 1));
  result.radius = radius;
  // A Chebyshev cube whose half-width reaches the periodic half-diameter is
  // the whole domain; record that explicitly so the Sec 3 whole-domain
  // reference and a degenerate radius are never confused.
  result.universal = L > 0 && radius >= L / 2;
  return result;
}

MomentumMask make_momentum_universal_mask(int L) {
  MomentumMask result;
  result.L = L;
  result.universal = true;
  result.radius = L;
  return result;
}

MomentumMask make_momentum_component_challenge_mask(int L, int cx, int cy,
                                                    int cz, int radius) {
  MomentumMask result;
  result.L = L;
  result.per_component = true;
  const auto count = static_cast<std::size_t>(L) * L * L;
  for (int a = 0; a < 3; ++a) {
    result.component[static_cast<std::size_t>(a)].assign(count, 0);
    for (int x = 0; x < L; ++x)
      for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
          // Component a uses a cube of half-width radius+a, shifted by a along
          // x.  The three component masks therefore differ genuinely, which is
          // what Sec 6.4 requires so that S^(i) contributes non-zero.
          const int dx = std::abs(shortest(x - (cx + a), L));
          const int dy = std::abs(shortest(y - cy, L));
          const int dz = std::abs(shortest(z - cz, L));
          if (std::max({dx, dy, dz}) <= radius + a)
            result.component[static_cast<std::size_t>(a)][flat(L, x, y, z)] = 1;
        }
  }
  return result;
}

MomentumMask complement_momentum_mask(const MomentumMask& mask) {
  MomentumMask result = mask;
  result.complemented = !mask.complemented;
  return result;
}

// ---------------------------------------------------------------------------
// Chord tables
// ---------------------------------------------------------------------------

MomentumTransportCurrentTable build_momentum_transport_current_table(
    MomentumOperatorKind kind, int component, int probe_size) {
  MomentumTransportCurrentTable result;
  result.kind = kind;
  result.component = component;
  result.probe_size = probe_size;
  if (probe_size < 5 || component < 0 || component > 2) return result;
  const int L = probe_size;

  StencilMap stencil;
  for (int b = 0; b < 3; ++b) {
    Triplet impulse = make_triplet(L);
    impulse[static_cast<std::size_t>(b)][flat(L, 0, 0, 0)] = 1.0;
    const auto image = apply_operator_triplet(kind, component, L, impulse);
    for (int x = 0; x < L; ++x)
      for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
          const auto site = flat(L, x, y, z);
          for (int a = 0; a < 3; ++a) {
            const double value = image[static_cast<std::size_t>(a)][site];
            if (std::abs(value) <= kStencilThreshold) continue;
            // (N delta_b)_a(v) = N_{-v}[a][b].
            StencilKey key;
            key.r[0] = -shortest(x, L);
            key.r[1] = -shortest(y, L);
            key.r[2] = -shortest(z, L);
            auto& matrix = stencil[key];
            matrix[static_cast<std::size_t>(a)][static_cast<std::size_t>(b)]
                = value;
          }
        }
  }

  // Sec 2.2 (S): skewness forces N_{-r} = -N_r^T.  Measured, not assumed.
  double skew = 0.0;
  for (const auto& [key, matrix] : stencil) {
    StencilKey mirror;
    mirror.r[0] = -key.r[0];
    mirror.r[1] = -key.r[1];
    mirror.r[2] = -key.r[2];
    std::array<std::array<double, 3>, 3> other{};
    const auto found = stencil.find(mirror);
    if (found != stencil.end()) other = found->second;
    for (int a = 0; a < 3; ++a)
      for (int b = 0; b < 3; ++b)
        skew = std::max(skew, std::abs(
            other[static_cast<std::size_t>(a)][static_cast<std::size_t>(b)]
            + matrix[static_cast<std::size_t>(b)][static_cast<std::size_t>(a)]));
  }
  result.skewness_residual = skew;

  int entries = 0;
  int maximum_l1 = 0;
  int maximum_linf = 0;
  for (const auto& [key, matrix] : stencil) {
    const int l1 = std::abs(key.r[0]) + std::abs(key.r[1]) + std::abs(key.r[2]);
    const int linf = std::max({std::abs(key.r[0]), std::abs(key.r[1]),
                               std::abs(key.r[2])});
    for (int a = 0; a < 3; ++a)
      for (int b = 0; b < 3; ++b) {
        const double value =
            matrix[static_cast<std::size_t>(a)][static_cast<std::size_t>(b)];
        if (std::abs(value) <= kStencilThreshold) continue;
        ++entries;
        maximum_l1 = std::max(maximum_l1, l1);
        maximum_linf = std::max(maximum_linf, linf);
        if (!greater_key(key.r, a, b)) continue;
        MomentumChordClass chord;
        chord.r[0] = key.r[0];
        chord.r[1] = key.r[1];
        chord.r[2] = key.r[2];
        chord.a = a;
        chord.b = b;
        chord.coefficient = value;
        chord.l1 = l1;
        chord.linf = linf;
        result.classes.push_back(chord);
      }
  }
  result.displacement_count = static_cast<int>(stencil.size());
  result.entry_count = entries;
  result.class_count = static_cast<int>(result.classes.size());
  result.maximum_l1 = maximum_l1;
  result.maximum_linf = maximum_linf;
  result.aliasing_margin = static_cast<double>(L / 2) - maximum_linf;

  // Sec 2.3: unit-bond decomposition along the frozen lexicographic path
  // (x steps, then y, then z, from v toward v+r).  Banned move B5.
  for (const auto& chord : result.classes) {
    if (chord.a != chord.b) {
      MomentumSiteGenerator generator;
      generator.r[0] = chord.r[0];
      generator.r[1] = chord.r[1];
      generator.r[2] = chord.r[2];
      generator.a = chord.a;
      generator.b = chord.b;
      generator.coefficient = chord.coefficient;
      result.site.push_back(generator);
    }
    int position[3]{0, 0, 0};
    for (int axis = 0; axis < 3; ++axis) {
      const int total = chord.r[axis];
      const int step = (total > 0) - (total < 0);
      for (int taken = 0; taken < std::abs(total); ++taken) {
        MomentumBondGenerator generator;
        generator.axis = axis;
        generator.r[0] = chord.r[0];
        generator.r[1] = chord.r[1];
        generator.r[2] = chord.r[2];
        generator.a = chord.a;
        generator.b = chord.b;
        if (step > 0) {
          // p_{k+1} = p_k+e_d: base = p_k, sign +1.
          generator.base[0] = position[0];
          generator.base[1] = position[1];
          generator.base[2] = position[2];
          generator.weight = chord.coefficient;
        } else {
          // p_{k+1} = p_k-e_d: base = p_{k+1}, sign -1.
          generator.base[0] = position[0];
          generator.base[1] = position[1];
          generator.base[2] = position[2];
          generator.base[axis] += step;
          generator.weight = -chord.coefficient;
        }
        result.bond.push_back(generator);
        position[axis] += step;
      }
    }
  }
  result.valid = result.skewness_residual <= 1e-12
      && result.aliasing_margin > 0.0 && !result.classes.empty();
  return result;
}

// ---------------------------------------------------------------------------
// Masked reductions
// ---------------------------------------------------------------------------

double masked_bond_flux(const MomentumTransportCurrentTable& table,
                        const MomentumFieldView& field,
                        const MomentumMask& mask) {
  if (!field.valid() || !mask.valid() || field.L != mask.L) return NAN;
  const int L = field.L;
  long double total = 0.0L;
  for (const auto& generator : table.bond) {
    const int axis = generator.axis;
    for (int x = 0; x < L; ++x)
      for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
          const int nx = axis == 0 ? x + 1 : x;
          const int ny = axis == 1 ? y + 1 : y;
          const int nz = axis == 2 ? z + 1 : z;
          const int factor =
              static_cast<int>(mask.inside(generator.a, x, y, z))
              - static_cast<int>(mask.inside(generator.a, nx, ny, nz));
          if (factor == 0) continue;  // chord does not straddle dOmega
          const int bx = x - generator.base[0];
          const int by = y - generator.base[1];
          const int bz = z - generator.base[2];
          const double left = field.at(generator.a, bx, by, bz);
          const double right = field.at(generator.b, bx + generator.r[0],
                                        by + generator.r[1],
                                        bz + generator.r[2]);
          total += static_cast<long double>(generator.weight) * left * right
              * factor;
        }
  }
  return static_cast<double>(total);
}

double masked_site_flux(const MomentumTransportCurrentTable& table,
                        const MomentumFieldView& field,
                        const MomentumMask& mask) {
  if (!field.valid() || !mask.valid() || field.L != mask.L) return NAN;
  const int L = field.L;
  long double total = 0.0L;
  for (const auto& generator : table.site) {
    for (int x = 0; x < L; ++x)
      for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
          const int factor =
              static_cast<int>(mask.inside(generator.a, x, y, z))
              - static_cast<int>(mask.inside(generator.b, x, y, z));
          if (factor == 0) continue;
          const double left = field.at(generator.a, x - generator.r[0],
                                       y - generator.r[1], z - generator.r[2]);
          const double right = field.at(generator.b, x, y, z);
          total += static_cast<long double>(generator.coefficient) * left
              * right * factor;
        }
  }
  return static_cast<double>(total);
}

double masked_chord_flux(const MomentumTransportCurrentTable& table,
                         const MomentumFieldView& field,
                         const MomentumMask& mask) {
  return masked_bond_flux(table, field, mask)
      + masked_site_flux(table, field, mask);
}

std::array<std::vector<double>, 3> apply_momentum_operator(
    MomentumOperatorKind kind, int component,
    const MomentumFieldView& field) {
  if (!field.valid()) return {};
  return apply_operator_triplet(kind, component, field.L,
                                view_triplet(field));
}

double direct_masked_bilinear(MomentumOperatorKind kind, int component,
                              const MomentumFieldView& field,
                              const MomentumMask& mask) {
  if (!field.valid() || !mask.valid() || field.L != mask.L) return NAN;
  const auto image = apply_momentum_operator(kind, component, field);
  const int L = field.L;
  long double total = 0.0L;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto site = flat(L, x, y, z);
        for (int a = 0; a < 3; ++a) {
          if (!mask.inside(a, x, y, z)) continue;
          total += static_cast<long double>(field.component[a][site])
              * image[static_cast<std::size_t>(a)][site];
        }
      }
  return static_cast<double>(total);
}

MomentumStressLedgerArrays build_momentum_stress_ledger_arrays(
    const MomentumTransportCurrentTable& table,
    const MomentumFieldView& field) {
  MomentumStressLedgerArrays result;
  if (!field.valid()) return result;
  const int L = field.L;
  result.L = L;
  const auto count = static_cast<std::size_t>(L) * L * L;
  for (int d = 0; d < 3; ++d)
    for (int a = 0; a < 3; ++a)
      result.bond[static_cast<std::size_t>(d)][static_cast<std::size_t>(a)]
          .assign(count, 0.0);
  for (int a = 0; a < 3; ++a)
    for (int b = 0; b < 3; ++b)
      result.site[static_cast<std::size_t>(a)][static_cast<std::size_t>(b)]
          .assign(count, 0.0);

  for (const auto& generator : table.bond)
    for (int x = 0; x < L; ++x)
      for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
          const int bx = x - generator.base[0];
          const int by = y - generator.base[1];
          const int bz = z - generator.base[2];
          const double value = generator.weight
              * field.at(generator.a, bx, by, bz)
              * field.at(generator.b, bx + generator.r[0],
                         by + generator.r[1], bz + generator.r[2]);
          result.bond[static_cast<std::size_t>(generator.axis)]
                     [static_cast<std::size_t>(generator.a)][flat(L, x, y, z)]
              += value;
        }
  for (const auto& generator : table.site)
    for (int x = 0; x < L; ++x)
      for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
          const double value = generator.coefficient
              * field.at(generator.a, x - generator.r[0], y - generator.r[1],
                         z - generator.r[2])
              * field.at(generator.b, x, y, z);
          result.site[static_cast<std::size_t>(generator.a)]
                     [static_cast<std::size_t>(generator.b)][flat(L, x, y, z)]
              += value;
        }
  return result;
}

double masked_flux_from_arrays(const MomentumStressLedgerArrays& arrays,
                               const MomentumMask& mask) {
  if (arrays.L <= 0 || !mask.valid() || arrays.L != mask.L) return NAN;
  const int L = arrays.L;
  long double total = 0.0L;
  for (int d = 0; d < 3; ++d)
    for (int a = 0; a < 3; ++a)
      for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
          for (int z = 0; z < L; ++z) {
            const int nx = d == 0 ? x + 1 : x;
            const int ny = d == 1 ? y + 1 : y;
            const int nz = d == 2 ? z + 1 : z;
            const int factor = static_cast<int>(mask.inside(a, x, y, z))
                - static_cast<int>(mask.inside(a, nx, ny, nz));
            if (factor == 0) continue;
            total += static_cast<long double>(
                arrays.bond[static_cast<std::size_t>(d)]
                           [static_cast<std::size_t>(a)][flat(L, x, y, z)])
                * factor;
          }
  for (int a = 0; a < 3; ++a)
    for (int b = 0; b < 3; ++b)
      for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
          for (int z = 0; z < L; ++z) {
            const int factor = static_cast<int>(mask.inside(a, x, y, z))
                - static_cast<int>(mask.inside(b, x, y, z));
            if (factor == 0) continue;
            total += static_cast<long double>(
                arrays.site[static_cast<std::size_t>(a)]
                           [static_cast<std::size_t>(b)][flat(L, x, y, z)])
                * factor;
          }
  return static_cast<double>(total);
}

// ---------------------------------------------------------------------------
// Densities
// ---------------------------------------------------------------------------

namespace {

bool step_fields_ready(const MomentumStepFields& fields) {
  return fields.electric_before != nullptr && fields.magnetic_before != nullptr
      && fields.magnetic_after != nullptr
      && fields.electric_pre_current != nullptr
      && fields.electric_after != nullptr
      && fields.electric_before->L > 0
      && fields.magnetic_before->L == fields.electric_before->L
      && fields.magnetic_after->L == fields.electric_before->L
      && fields.electric_pre_current->L == fields.electric_before->L
      && fields.electric_after->L == fields.electric_before->L;
}

Triplet product_density(int L, const Triplet& carrier, const Triplet& driver,
                        double sign) {
  Triplet result = make_triplet(L);
  for (int a = 0; a < 3; ++a)
    for (std::size_t i = 0; i < result[static_cast<std::size_t>(a)].size(); ++i)
      result[static_cast<std::size_t>(a)][i] = sign
          * carrier[static_cast<std::size_t>(a)][i]
          * driver[static_cast<std::size_t>(a)][i];
  return result;
}

}  // namespace

std::array<std::vector<double>, 3> momentum_density_before(
    const MomentumStepFields& fields, MomentumLocalization localization,
    int component) {
  if (!step_fields_ready(fields)) return {};
  const int L = fields.electric_before->L;
  if (localization == MomentumLocalization::ECarries) {
    // pi^(1),before = E . (D_i C B)
    const auto curl_b = apply_curl(L, from_field(*fields.magnetic_before));
    const auto driver = apply_central(L, curl_b, component);
    return product_density(L, from_field(*fields.electric_before), driver, 1.0);
  }
  // pi^(2),before = -B . (D_i C^T E)
  const auto w = apply_curl_adjoint(L, from_field(*fields.electric_before));
  const auto driver = apply_central(L, w, component);
  return product_density(L, from_field(*fields.magnetic_before), driver, -1.0);
}

std::array<std::vector<double>, 3> momentum_density_after(
    const MomentumStepFields& fields, MomentumLocalization localization,
    int component) {
  if (!step_fields_ready(fields)) return {};
  const int L = fields.electric_before->L;
  if (localization == MomentumLocalization::ECarries) {
    // pi^(1),after = E'' . (D_i C B')
    const auto u = apply_curl(L, from_field(*fields.magnetic_after));
    const auto driver = apply_central(L, u, component);
    return product_density(L, from_field(*fields.electric_after), driver, 1.0);
  }
  // pi^(2),after = -B' . (D_i C^T E'')
  const auto w = apply_curl_adjoint(L, from_field(*fields.electric_after));
  const auto driver = apply_central(L, w, component);
  return product_density(L, from_field(*fields.magnetic_after), driver, -1.0);
}

std::array<std::vector<double>, 3> momentum_source_density(
    const MomentumStepFields& fields, MomentumLocalization localization,
    int component) {
  if (!step_fields_ready(fields)) return {};
  const int L = fields.electric_before->L;
  const auto current = subtract(from_field(*fields.electric_pre_current),
                                from_field(*fields.electric_after));  // K
  if (localization == MomentumLocalization::ECarries) {
    // Q density = K . (D_i C B')
    const auto u = apply_curl(L, from_field(*fields.magnetic_after));
    const auto driver = apply_central(L, u, component);
    return product_density(L, current, driver, 1.0);
  }
  // Q density = -B' . (D_i C^T K)
  const auto adjoint_current = apply_curl_adjoint(L, current);
  const auto driver = apply_central(L, adjoint_current, component);
  return product_density(L, from_field(*fields.magnetic_after), driver, -1.0);
}

// ---------------------------------------------------------------------------
// Per-tick terms
// ---------------------------------------------------------------------------

double MomentumLedgerTickTerms::identity_scale(double lambda) const {
  return std::max({1.0, std::abs(material()), std::abs(lambda * phi_plain),
                   std::abs(lambda * phi_binding), std::abs(source)});
}

double MomentumLedgerTickTerms::reynolds_scale() const {
  return std::max({1.0, std::abs(content_after), std::abs(content_old),
                   std::abs(material()), std::abs(sweep)});
}

void scale_momentum_ledger_tick_terms(MomentumLedgerTickTerms& terms,
                                      double scale) {
  terms.phi_plain *= scale;
  terms.phi_binding *= scale;
  terms.phi_plain_complement *= scale;
  terms.phi_binding_complement *= scale;
  terms.sweep *= scale;
  terms.sweep_complement *= scale;
  terms.source *= scale;
  terms.content_after *= scale;
  terms.content_before *= scale;
  terms.content_old *= scale;
}

MomentumLedgerTickTerms observe_momentum_ledger_tick(
    const MomentumStepFields& fields,
    MomentumLocalization localization,
    int component,
    const MomentumMask& previous_mask,
    const MomentumMask& current_mask,
    const MomentumTransportCurrentTable& plain_table,
    const MomentumTransportCurrentTable& binding_table) {
  MomentumLedgerTickTerms result;
  if (!step_fields_ready(fields) || component < 0 || component > 2
      || !plain_table.valid || !binding_table.valid
      || !previous_mask.valid() || !current_mask.valid()) return result;
  const int L = fields.electric_before->L;
  if (previous_mask.L != L || current_mask.L != L) return result;

  // Sec 2.4 / Sec 2.5: the two flux pairs and their carrier fields.
  Triplet plain_field;
  Triplet binding_field;
  if (localization == MomentumLocalization::ECarries) {
    plain_field = apply_curl(L, from_field(*fields.magnetic_after));   // u
    binding_field = from_field(*fields.electric_before);               // E
  } else {
    plain_field = apply_curl_adjoint(L,
        from_field(*fields.electric_before));                          // w
    binding_field = from_field(*fields.magnetic_after);                // B'
  }
  const auto plain_face = as_face(L, plain_field);
  const auto binding_face = as_face(L, binding_field);
  const auto plain_view = momentum_view(plain_face);
  const auto binding_view = momentum_view(binding_face);
  const auto complement = complement_momentum_mask(current_mask);

  result.phi_plain = masked_chord_flux(plain_table, plain_view, current_mask);
  result.phi_binding =
      masked_chord_flux(binding_table, binding_view, current_mask);
  result.phi_plain_complement =
      masked_chord_flux(plain_table, plain_view, complement);
  result.phi_binding_complement =
      masked_chord_flux(binding_table, binding_view, complement);

  const auto before = momentum_density_before(fields, localization, component);
  const auto after = momentum_density_after(fields, localization, component);
  const auto source = momentum_source_density(fields, localization, component);
  const auto previous_complement = complement_momentum_mask(previous_mask);

  long double content_after = 0.0L;
  long double content_before = 0.0L;
  long double content_old = 0.0L;
  long double sweep = 0.0L;
  long double sweep_complement = 0.0L;
  long double source_total = 0.0L;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto site = flat(L, x, y, z);
        for (int a = 0; a < 3; ++a) {
          const bool current_in = current_mask.inside(a, x, y, z);
          const bool previous_in = previous_mask.inside(a, x, y, z);
          const bool current_out = complement.inside(a, x, y, z);
          const bool previous_out = previous_complement.inside(a, x, y, z);
          const double pi_before = before[static_cast<std::size_t>(a)][site];
          const double pi_after = after[static_cast<std::size_t>(a)][site];
          const double q = source[static_cast<std::size_t>(a)][site];
          if (current_in) {
            content_after += pi_after;
            content_before += pi_before;
            source_total += q;
          }
          if (previous_in) content_old += pi_before;
          sweep += (static_cast<int>(current_in)
                    - static_cast<int>(previous_in)) * pi_before;
          sweep_complement += (static_cast<int>(current_out)
                               - static_cast<int>(previous_out)) * pi_before;
        }
      }
  result.content_after = static_cast<double>(content_after);
  result.content_before = static_cast<double>(content_before);
  result.content_old = static_cast<double>(content_old);
  result.sweep = static_cast<double>(sweep);
  result.sweep_complement = static_cast<double>(sweep_complement);
  result.source = static_cast<double>(source_total);
  result.valid = std::isfinite(result.phi_plain)
      && std::isfinite(result.phi_binding)
      && std::isfinite(result.phi_plain_complement)
      && std::isfinite(result.phi_binding_complement)
      && std::isfinite(result.content_after)
      && std::isfinite(result.content_before)
      && std::isfinite(result.content_old) && std::isfinite(result.sweep)
      && std::isfinite(result.sweep_complement) && std::isfinite(result.source);
  return result;
}

// ---------------------------------------------------------------------------
// Accumulators and ratios
// ---------------------------------------------------------------------------

void MomentumLedgerAccumulator::add(const MomentumLedgerTickTerms& terms,
                                    double lambda) {
  if (!terms.valid) return;
  if (!initialized) {
    initialized = true;
    initial_content = terms.content_old;
    content = terms.content_old;
  } else {
    maximum_chain_residual = std::max(maximum_chain_residual,
                                      std::abs(terms.content_old - content));
  }
  flux += static_cast<long double>(terms.flux(lambda));
  flux_complement += static_cast<long double>(terms.flux_complement(lambda));
  sweep += static_cast<long double>(terms.sweep);
  sweep_complement += static_cast<long double>(terms.sweep_complement);
  source += static_cast<long double>(terms.source);
  content = terms.content_after;
  ++ticks;

  const double identity = std::abs(terms.identity_residual(lambda));
  maximum_tick_identity_residual =
      std::max(maximum_tick_identity_residual, identity);
  window_maximum_tick_identity_residual =
      std::max(window_maximum_tick_identity_residual, identity);
  maximum_tick_identity_ratio = std::max(maximum_tick_identity_ratio,
                                         identity / terms.identity_scale(lambda));
  const double reynolds = std::abs(terms.reynolds_residual());
  maximum_reynolds_residual = std::max(maximum_reynolds_residual, reynolds);
  maximum_reynolds_ratio =
      std::max(maximum_reynolds_ratio, reynolds / terms.reynolds_scale());
}

void MomentumLedgerAccumulator::begin_checkpoint_window() {
  window_maximum_tick_identity_residual = 0.0;
}

double MomentumLedgerAccumulator::ledger_residual() const {
  return content_change() - flux_total() - sweep_total() + source_total();
}

double MomentumLedgerAccumulator::ledger_scale() const {
  return std::max({1.0, std::abs(content), std::abs(flux_total()),
                   std::abs(sweep_total()), std::abs(source_total())});
}

MomentumRetentionRatios compute_momentum_retention_ratios(
    const MomentumLedgerAccumulator& region,
    double outer_source,
    double whole_domain_change,
    double matter_defect) {
  MomentumRetentionRatios result;
  const double defect_floor = std::max(std::abs(matter_defect), 1e-9);
  result.rho = std::abs(region.transfer_total()) / defect_floor;
  result.rho_ceiling = std::abs(whole_domain_change) / defect_floor;
  // Sec 6.5 (a): kappa states Q(R,tau) == Q(R_out,tau).  With no accumulated
  // source anywhere the statement is vacuously true, so kappa is 1, not NaN;
  // a non-zero regional source against a zero outer source is a real failure.
  if (outer_source != 0.0)
    result.kappa = region.source_total() / outer_source;
  else
    result.kappa = region.source_total() == 0.0 ? 1.0 : NAN;
  if (whole_domain_change == 0.0 || !std::isfinite(whole_domain_change))
    return result;
  result.resolved = true;
  result.eta = region.content_change() / whole_domain_change;
  result.transfer = region.transfer_total() / whole_domain_change;
  result.identity_residual = result.eta - result.transfer - 1.0;
  return result;
}

}  // namespace ftd::eft
