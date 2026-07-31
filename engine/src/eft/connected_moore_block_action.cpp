#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/derived_interaction_graph.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <utility>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

std::size_t volume(int L) {
  return L > 0 ? static_cast<std::size_t>(L)*L*L : 0;
}

int wrap(int value, int L) {
  const int remainder = value%L;
  return remainder < 0 ? remainder+L : remainder;
}

std::size_t index(int L, int x, int y, int z) {
  return static_cast<std::size_t>(
      (wrap(x, L)*L+wrap(y, L))*L+wrap(z, L));
}

template <typename Field>
bool finite_field(const Field& field) {
  const std::size_t expected = volume(field.L);
  if (field.L <= 0 || field.x.size() != expected
      || field.y.size() != expected || field.z.size() != expected)
    return false;
  const auto entries_finite = [](const std::vector<double>& entries) {
    return std::all_of(entries.begin(), entries.end(),
        [](double value) { return std::isfinite(value); });
  };
  return entries_finite(field.x) && entries_finite(field.y)
      && entries_finite(field.z);
}

Vec3 effective_position(const MatchedMatterPoint& point) {
  return {point.anchor.x+point.remainder.x,
          point.anchor.y+point.remainder.y,
          point.anchor.z+point.remainder.z};
}

MatchedMatterPoint point_at(const Vec3& position, int L,
                            const Vec3& momentum) {
  MatchedMatterPoint point;
  const long long ax = std::llround(position.x);
  const long long ay = std::llround(position.y);
  const long long az = std::llround(position.z);
  point.anchor = {wrap(static_cast<int>(ax), L),
                  wrap(static_cast<int>(ay), L),
                  wrap(static_cast<int>(az), L)};
  point.remainder = {position.x-ax, position.y-ay, position.z-az};
  point.momentum = momentum;
  return point;
}

bool same_anchor(const Coord& lhs, const Coord& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool site_projection_valid(const ConnectedMooreBlockState& state) {
  if (state.constituents.size() != state.charges.size()) return false;
  std::set<std::tuple<int,int,int>> sites;
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    if (state.charges[a] != -1 && state.charges[a] != +1) return false;
    const auto& anchor = state.constituents[a].anchor;
    sites.emplace(anchor.x, anchor.y, anchor.z);
  }
  return sites.size() == state.constituents.size();
}

bool graph_valid(const ConnectedMooreBlockState& state,
                 bool* connected_out = nullptr,
                 bool* local_out = nullptr) {
  const std::size_t count = state.constituents.size();
  bool local = count > 0;
  std::vector<std::vector<std::size_t>> adjacency(count);
  std::set<std::pair<std::size_t,std::size_t>> unique;
  for (const auto& edge : state.edges) {
    if (edge.first >= count || edge.second >= count
        || edge.first >= edge.second
        || !unique.emplace(edge.first, edge.second).second) {
      local = false;
      continue;
    }
    const Coord d = edge.reference_delta;
    const int chebyshev = std::max({std::abs(d.x), std::abs(d.y),
                                    std::abs(d.z)});
    const int rest = d.x*d.x+d.y*d.y+d.z*d.z;
    local = local && chebyshev == 1 && rest >= 1 && rest <= 3
        && edge.rest_length_squared == static_cast<double>(rest);
    adjacency[edge.first].push_back(edge.second);
    adjacency[edge.second].push_back(edge.first);
  }
  bool connected = count > 0;
  if (connected) {
    std::vector<bool> seen(count, false);
    std::queue<std::size_t> pending;
    seen[0] = true;
    pending.push(0);
    while (!pending.empty()) {
      const auto a = pending.front();
      pending.pop();
      for (std::size_t b : adjacency[a])
        if (!seen[b]) {
          seen[b] = true;
          pending.push(b);
        }
    }
    connected = std::all_of(seen.begin(), seen.end(),
        [](bool value) { return value; });
  }
  if (connected_out != nullptr) *connected_out = connected;
  if (local_out != nullptr) *local_out = local;
  return connected && local;
}

bool matter_metadata_valid(const ConnectedMooreBlockState& state,
                           const ConnectedMooreBlockOptions& options) {
  bool connected = false, local = false;
  const bool derived_pair =
      options.binding_law == ConnectedBindingLaw::DerivedCompactPair;
  const bool graph_ok = derived_pair
      ? state.constituents.size() == 2 && state.edges.empty()
      : graph_valid(state, &connected, &local);
  if ((!derived_pair && (state.width < 1 || state.orientation_axis < 0
                            || state.orientation_axis > 2))
      || state.constituents.empty()
      || state.constituents.size() != state.charges.size()
      || (!options.allow_shared_anchor_chart && !site_projection_valid(state))
      || !graph_ok
      || !(options.dt > 0.0) || !std::isfinite(options.dt)
      || !(options.wave_speed >= 0.0) || !std::isfinite(options.wave_speed)
      || !(options.binding_stiffness >= 0.0)
      || !std::isfinite(options.binding_stiffness)
      || (derived_pair
          && (!(options.compact_pair_well_depth > 0.0)
              || !std::isfinite(options.compact_pair_well_depth)
              || options.compact_pair_cutoff_distance_squared != 1.5))
      || !(options.constituent_mass_scale > 0.0)
      || !std::isfinite(options.constituent_mass_scale)
      || !(options.polarity_scale > 0.0)
      || !std::isfinite(options.polarity_scale)
      || !(options.field_energy_scale > 0.0)
      || !std::isfinite(options.field_energy_scale)
      || !(options.gate_tolerance > 0.0)
      || !(options.solve_tolerance > 0.0)
      || !(options.finite_difference_scale > 0.0)
      || options.max_iterations <= 0) return false;
  if (!options.root_momentum_seed.empty()) {
    if (options.root_momentum_seed.size() != state.constituents.size())
      return false;
    for (const auto& momentum : options.root_momentum_seed)
      if (!finite(momentum)) return false;
  }
  int net = 0;
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    net += state.charges[a];
    if (!finite(state.constituents[a].remainder)
        || !finite(state.constituents[a].momentum)
        || !make_quadratic_polarity_coat(
            effective_position(state.constituents[a]),
            state.charges[a]).valid) return false;
  }
  if (derived_pair)
    return net == 0 && state.charges.size() == 2
        && state.charges[0] == -state.charges[1];
  return net == 0 && connected && local;
}

bool state_valid(const ConnectedMooreBlockState& state,
                 const ConnectedMooreBlockOptions& options,
                 bool fields_prevalidated = false) {
  const std::size_t expected = state.electric.L > 0
      ? volume(state.electric.L) : 0;
  const bool field_shapes = state.electric.L >= 5
      && state.magnetic_half.L == state.electric.L
      && state.electric.x.size() == expected
      && state.electric.y.size() == expected
      && state.electric.z.size() == expected
      && state.magnetic_half.x.size() == expected
      && state.magnetic_half.y.size() == expected
      && state.magnetic_half.z.size() == expected;
  return field_shapes
      && (fields_prevalidated
          || (finite_field(state.electric)
              && finite_field(state.magnetic_half)))
      && matter_metadata_valid(state,options);
}

std::vector<Vec3> positions(const ConnectedMooreBlockState& state) {
  std::vector<Vec3> result(state.constituents.size());
  for (std::size_t a = 0; a < result.size(); ++a)
    result[a] = effective_position(state.constituents[a]);
  return result;
}

double constituent_energy(const Vec3& momentum,
                          const ConnectedMooreBlockOptions& options) {
  const double rest = options.constituent_mass_scale*E_REST;
  return std::sqrt(rest*rest+C_SPEED*C_SPEED*momentum.mag2());
}

DerivedInteractionGraphOptions derived_pair_options(
    const ConnectedMooreBlockOptions& options) {
  DerivedInteractionGraphOptions result;
  result.dt = options.dt;
  result.well_depth = options.compact_pair_well_depth;
  result.cutoff_distance_squared =
      options.compact_pair_cutoff_distance_squared;
  result.gate_tolerance = options.gate_tolerance;
  result.solve_tolerance = options.solve_tolerance;
  result.max_iterations = options.max_iterations;
  return result;
}

double binding_energy(const std::vector<Vec3>& x,
                      const std::vector<MooreBindingEdge>& edges,
                      const ConnectedMooreBlockOptions& options) {
  if (options.binding_law == ConnectedBindingLaw::DerivedCompactPair) {
    if (x.size() != 2 || !edges.empty()) return INFINITY;
    return derived_interaction_potential(
        (x[0]-x[1]).mag2(), derived_pair_options(options));
  }
  long double result = 0.0L;
  for (const auto& edge : edges) {
    const Vec3 delta = x[edge.first]-x[edge.second];
    const long double u = static_cast<long double>(delta.dot(delta))
        -edge.rest_length_squared;
    result += 0.25L*options.binding_stiffness*u*u;
  }
  return static_cast<double>(result);
}

std::vector<Vec3> binding_impulses(
    const std::vector<Vec3>& x0, const std::vector<Vec3>& x1,
    const std::vector<MooreBindingEdge>& edges,
    const ConnectedMooreBlockOptions& options) {
  std::vector<Vec3> result(x0.size());
  if (options.binding_law == ConnectedBindingLaw::DerivedCompactPair) {
    if (x0.size() != 2 || x1.size() != 2 || !edges.empty()) return {};
    const Vec3 d0 = x0[0]-x0[1];
    const Vec3 d1 = x1[0]-x1[1];
    const double squared0 = d0.mag2();
    const double squared1 = d1.mag2();
    const auto pair_options = derived_pair_options(options);
    const double scale = std::abs(squared1-squared0)
            > 1e-13*std::max({1.0,std::abs(squared0),std::abs(squared1)})
        ? (derived_interaction_potential(squared1,pair_options)
           -derived_interaction_potential(squared0,pair_options))
            /(squared1-squared0)
        : derived_interaction_potential_derivative(
            0.5*(squared0+squared1),pair_options);
    const Vec3 impulse = (d0+d1)*(options.dt*scale);
    result[0] -= impulse;
    result[1] += impulse;
    return result;
  }
  for (const auto& edge : edges) {
    const Vec3 d0 = x0[edge.first]-x0[edge.second];
    const Vec3 d1 = x1[edge.first]-x1[edge.second];
    const double u0 = d0.dot(d0)-edge.rest_length_squared;
    const double u1 = d1.dot(d1)-edge.rest_length_squared;
    const Vec3 gradient = (d0+d1)
        *(0.25*options.binding_stiffness*(u0+u1));
    const Vec3 impulse = gradient*options.dt;
    result[edge.first] -= impulse;
    result[edge.second] += impulse;
  }
  return result;
}

std::vector<double> flatten_momenta(
    const std::vector<MatchedMatterPoint>& constituents) {
  std::vector<double> result(3*constituents.size(), 0.0);
  for (std::size_t a = 0; a < constituents.size(); ++a)
    for (int axis = 0; axis < 3; ++axis)
      result[3*a+axis] = component(constituents[a].momentum, axis);
  return result;
}

std::vector<double> flatten_momenta(const std::vector<Vec3>& momenta) {
  std::vector<double> result(3*momenta.size(), 0.0);
  for (std::size_t a = 0; a < momenta.size(); ++a)
    for (int axis = 0; axis < 3; ++axis)
      result[3*a+axis] = component(momenta[a], axis);
  return result;
}

std::vector<Vec3> unflatten_momenta(const std::vector<double>& values) {
  std::vector<Vec3> result(values.size()/3);
  for (std::size_t a = 0; a < result.size(); ++a)
    result[a] = {values[3*a], values[3*a+1], values[3*a+2]};
  return result;
}

double infinity_norm(const std::vector<double>& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

MatchedFaceFlux midpoint(const MatchedFaceFlux& lhs,
                         const MatchedFaceFlux& rhs) {
  MatchedFaceFlux result(lhs.L);
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    result.x[i] = 0.5*(lhs.x[i]+rhs.x[i]);
    result.y[i] = 0.5*(lhs.y[i]+rhs.y[i]);
    result.z[i] = 0.5*(lhs.z[i]+rhs.z[i]);
  }
  return result;
}

void add_current(MatchedFaceFlux& field,
                 const QuadraticCoatFaceCurrent& segment,
                 double scale) {
  if (!segment.dense_materialized) {
    for (const auto& entry : segment.sparse_current) {
      const std::size_t i = static_cast<std::size_t>(segment.index(
          entry.face.x, entry.face.y, entry.face.z));
      auto& component = entry.axis == 0 ? field.x
          : (entry.axis == 1 ? field.y : field.z);
      component[i] += scale*entry.value;
    }
    return;
  }
  for (std::size_t i = 0; i < field.x.size(); ++i) {
    field.x[i] += scale*segment.current_x[i];
    field.y[i] += scale*segment.current_y[i];
    field.z[i] += scale*segment.current_z[i];
  }
}

void add_density(std::vector<double>& density,
                 const std::vector<double>& addition) {
  for (std::size_t i = 0; i < density.size(); ++i)
    density[i] += addition[i];
}

struct PreparedForwardFields {
  MatchedEdgeField magnetic_later;
  MatchedFaceFlux electric_pre_current;
  explicit PreparedForwardFields(int L)
      : magnetic_later(L), electric_pre_current(L) {}
};

PreparedForwardFields prepare_forward_fields(
    const ConnectedMooreBlockState& earlier, double lambda) {
  PreparedForwardFields result(earlier.electric.L);
  result.magnetic_later = earlier.magnetic_half;
  const auto curl_adjoint = matched_curl_adjoint(earlier.electric);
  for (std::size_t i = 0; i < result.magnetic_later.x.size(); ++i) {
    result.magnetic_later.x[i] -= lambda*curl_adjoint.x[i];
    result.magnetic_later.y[i] -= lambda*curl_adjoint.y[i];
    result.magnetic_later.z[i] -= lambda*curl_adjoint.z[i];
  }
  result.electric_pre_current = earlier.electric;
  const auto curl = matched_curl(result.magnetic_later);
  for (std::size_t i = 0; i < result.electric_pre_current.x.size(); ++i) {
    result.electric_pre_current.x[i] += lambda*curl.x[i];
    result.electric_pre_current.y[i] += lambda*curl.y[i];
    result.electric_pre_current.z[i] += lambda*curl.z[i];
  }
  return result;
}

struct PreparedReverseFields {
  MatchedFaceFlux electric_pre_current;
  explicit PreparedReverseFields(int L) : electric_pre_current(L) {}
};

PreparedReverseFields prepare_reverse_fields(
    const ConnectedMooreBlockState& later, double lambda) {
  PreparedReverseFields result(later.electric.L);
  result.electric_pre_current = later.electric;
  const auto magnetic_curl = matched_curl(later.magnetic_half);
  for (std::size_t i = 0; i < result.electric_pre_current.x.size(); ++i) {
    result.electric_pre_current.x[i] -= lambda*magnetic_curl.x[i];
    result.electric_pre_current.y[i] -= lambda*magnetic_curl.y[i];
    result.electric_pre_current.z[i] -= lambda*magnetic_curl.z[i];
  }
  return result;
}

struct Candidate {
  bool valid = false;
  ConnectedMooreBlockState earlier;
  ConnectedMooreBlockState later;
  std::vector<QuadraticCoatFaceCurrent> segments;
  std::vector<QuadraticCoatOrbitGatherResult> gathers;
  std::vector<Vec3> velocities;
  std::vector<Vec3> electric_impulses;
  std::vector<Vec3> magnetic_impulses;
  std::vector<Vec3> binding_impulses;
  std::vector<Vec3> total_impulses;
  std::vector<double> residual;

  Candidate(int L, std::size_t count)
      : earlier(L), later(L), segments(count), gathers(count),
        velocities(count), electric_impulses(count),
        magnetic_impulses(count), binding_impulses(count),
        total_impulses(count), residual(3*count, 0.0) {}
};

void copy_metadata(const ConnectedMooreBlockState& source,
                   ConnectedMooreBlockState& target) {
  target.width = source.width;
  target.orientation_axis = source.orientation_axis;
  target.charges = source.charges;
  target.edges = source.edges;
  target.constituents.resize(source.constituents.size());
}

bool make_segments(Candidate& candidate, bool sparse, int explicit_L = 0) {
  const int L = explicit_L > 0 ? explicit_L : candidate.earlier.electric.L;
  for (std::size_t a = 0; a < candidate.segments.size(); ++a) {
    candidate.segments[a] = make_quadratic_coat_face_current(
        L, effective_position(candidate.earlier.constituents[a]),
        effective_position(candidate.later.constituents[a]),
        candidate.earlier.charges[a], !sparse);
    if (!candidate.segments[a].valid) return false;
  }
  return true;
}

void gather_impulses(Candidate& candidate,
                     const ConnectedMooreBlockOptions& options,
                     double interaction_scale) {
  const auto electric_midpoint = midpoint(
      candidate.earlier.electric, candidate.later.electric);
  if (!finite_field(electric_midpoint)
      || !finite_field(candidate.later.magnetic_half)) return;
  candidate.binding_impulses = binding_impulses(
      positions(candidate.earlier), positions(candidate.later),
      candidate.earlier.edges, options);
  for (std::size_t a = 0; a < candidate.segments.size(); ++a) {
    candidate.gathers[a] =
        evaluate_quadratic_coat_orbit_gather_prevalidated_fields(
        candidate.segments[a], electric_midpoint,
        candidate.later.magnetic_half, candidate.velocities[a],
        options.dt, interaction_scale, options.polarity_scale);
    if (!candidate.gathers[a].valid) return;
    candidate.electric_impulses[a] =
        candidate.gathers[a].electric_force*(options.dt*interaction_scale);
    candidate.magnetic_impulses[a] = candidate.gathers[a].magnetic_impulse;
    candidate.total_impulses[a] = candidate.electric_impulses[a]
        +candidate.magnetic_impulses[a]+candidate.binding_impulses[a];
    const Vec3 delta = candidate.later.constituents[a].momentum
        -candidate.earlier.constituents[a].momentum
        -candidate.total_impulses[a];
    candidate.residual[3*a] = delta.x;
    candidate.residual[3*a+1] = delta.y;
    candidate.residual[3*a+2] = delta.z;
  }
  // Newton's coordinate-wise finite-difference probes may straddle the
  // remainder chart at |f|=1/2 and temporarily give two constituents the
  // same rounded anchor.  The action is evaluated from smooth effective
  // positions, so rejecting those probes makes the Jacobian undefined at a
  // physically regular chart boundary.  Accepted endpoints retain the exact
  // site-projection gate in finalize(); only derivative probes are relaxed.
  candidate.valid = finite(candidate.residual)
      && finite_field(candidate.earlier.electric)
      && finite_field(candidate.earlier.magnetic_half)
      && finite_field(candidate.later.electric)
      && finite_field(candidate.later.magnetic_half);
}

void gather_impulses_local(
    Candidate& candidate,
    const ConnectedMooreBlockOptions& options,
    double interaction_scale,
    const MatchedFaceFlux& fixed_electric,
    const MatchedFaceFlux& electric_pre_current,
    double current_scale,
    const MatchedEdgeField& magnetic) {
  candidate.binding_impulses = binding_impulses(
      positions(candidate.earlier), positions(candidate.later),
      candidate.earlier.edges, options);
  candidate.gathers = options.resident_local_orbit_gather
      ? options.resident_local_orbit_gather(
          candidate.segments,candidate.velocities,current_scale,options.dt,
          interaction_scale,options.polarity_scale)
      : evaluate_quadratic_coat_orbit_gather_sparse_midpoint_batch_prevalidated_fields(
          candidate.segments, fixed_electric, electric_pre_current,
          current_scale, magnetic, candidate.velocities, options.dt,
          interaction_scale, options.polarity_scale);
  bool valid = candidate.gathers.size() == candidate.segments.size();
  for (std::size_t a = 0; a < candidate.segments.size(); ++a) {
    if (!valid || !candidate.gathers[a].valid) {
      valid = false;
      break;
    }
    candidate.electric_impulses[a] =
        candidate.gathers[a].electric_force*(options.dt*interaction_scale);
    candidate.magnetic_impulses[a] = candidate.gathers[a].magnetic_impulse;
    candidate.total_impulses[a] = candidate.electric_impulses[a]
        +candidate.magnetic_impulses[a]+candidate.binding_impulses[a];
    const Vec3 delta = candidate.later.constituents[a].momentum
        -candidate.earlier.constituents[a].momentum
        -candidate.total_impulses[a];
    candidate.residual[3*a] = delta.x;
    candidate.residual[3*a+1] = delta.y;
    candidate.residual[3*a+2] = delta.z;
  }
  candidate.valid = valid && finite(candidate.residual);
}

Candidate evaluate_forward_local_residual(
    const ConnectedMooreBlockState& earlier,
    const ConnectedMooreBlockOptions& options,
    const PreparedForwardFields& prepared,
    double interaction_scale,
    const std::vector<double>& unknown) {
  const int L = earlier.electric.L;
  const std::size_t count = earlier.constituents.size();
  Candidate candidate(0, count);
  copy_metadata(earlier, candidate.earlier);
  copy_metadata(earlier, candidate.later);
  candidate.earlier.constituents = earlier.constituents;
  const auto p1 = unflatten_momenta(unknown);
  for (std::size_t a = 0; a < count; ++a) {
    const Vec3 p0 = earlier.constituents[a].momentum;
    const double h0 = constituent_energy(p0, options);
    const double h1 = constituent_energy(p1[a], options);
    const double denominator = h0+h1;
    if (!(denominator > 0.0) || !std::isfinite(denominator)) return candidate;
    candidate.velocities[a] = (p0+p1[a])
        *(C_SPEED*C_SPEED/denominator);
    candidate.later.constituents[a] = point_at(
        effective_position(earlier.constituents[a])
            +candidate.velocities[a]*options.dt, L, p1[a]);
  }
  if (!make_segments(candidate, true, L)) return candidate;
  gather_impulses_local(candidate, options, interaction_scale,
      earlier.electric, prepared.electric_pre_current,
      -options.polarity_scale, prepared.magnetic_later);
  return candidate;
}

Candidate evaluate_forward_resident_local_residual(
    const ConnectedMooreBlockState& earlier,
    const ConnectedMooreBlockOptions& options,
    double interaction_scale,
    const std::vector<double>& unknown) {
  const int L=earlier.electric.L;
  const std::size_t count=earlier.constituents.size();
  Candidate candidate(0,count);
  copy_metadata(earlier,candidate.earlier);
  copy_metadata(earlier,candidate.later);
  candidate.earlier.electric.L=L;
  candidate.earlier.magnetic_half.L=L;
  candidate.later.electric.L=L;
  candidate.later.magnetic_half.L=L;
  candidate.earlier.constituents=earlier.constituents;
  const auto p1=unflatten_momenta(unknown);
  for(std::size_t a=0;a<count;++a) {
    const Vec3 p0=earlier.constituents[a].momentum;
    const double h0=constituent_energy(p0,options);
    const double h1=constituent_energy(p1[a],options);
    const double denominator=h0+h1;
    if(!(denominator>0.0)||!std::isfinite(denominator)) return candidate;
    candidate.velocities[a]=(p0+p1[a])
        *(C_SPEED*C_SPEED/denominator);
    candidate.later.constituents[a]=point_at(
        effective_position(earlier.constituents[a])
            +candidate.velocities[a]*options.dt,L,p1[a]);
  }
  if(!make_segments(candidate,true,L)) return candidate;
  gather_impulses_local(candidate,options,interaction_scale,
      candidate.earlier.electric,candidate.later.electric,
      -options.polarity_scale,candidate.later.magnetic_half);
  return candidate;
}

Candidate evaluate_reverse_local_residual(
    const ConnectedMooreBlockState& later,
    const ConnectedMooreBlockOptions& options,
    const PreparedReverseFields& prepared,
    double interaction_scale,
    const std::vector<double>& unknown) {
  const int L = later.electric.L;
  const std::size_t count = later.constituents.size();
  Candidate candidate(0, count);
  copy_metadata(later, candidate.earlier);
  copy_metadata(later, candidate.later);
  candidate.later.constituents = later.constituents;
  const auto p0 = unflatten_momenta(unknown);
  for (std::size_t a = 0; a < count; ++a) {
    const Vec3 p1 = later.constituents[a].momentum;
    const double h0 = constituent_energy(p0[a], options);
    const double h1 = constituent_energy(p1, options);
    const double denominator = h0+h1;
    if (!(denominator > 0.0) || !std::isfinite(denominator)) return candidate;
    candidate.velocities[a] = (p0[a]+p1)
        *(C_SPEED*C_SPEED/denominator);
    candidate.earlier.constituents[a] = point_at(
        effective_position(later.constituents[a])
            -candidate.velocities[a]*options.dt, L, p0[a]);
  }
  if (!make_segments(candidate, true, L)) return candidate;
  gather_impulses_local(candidate, options, interaction_scale,
      later.electric, prepared.electric_pre_current,
      +options.polarity_scale, later.magnetic_half);
  return candidate;
}

Candidate evaluate_forward(const ConnectedMooreBlockState& earlier,
                           const ConnectedMooreBlockOptions& options,
                           const PreparedForwardFields& prepared,
                           double interaction_scale,
                           const std::vector<double>& unknown) {
  const int L = earlier.electric.L;
  const std::size_t count = earlier.constituents.size();
  Candidate candidate(L, count);
  candidate.earlier = earlier;
  copy_metadata(earlier, candidate.later);
  const auto p1 = unflatten_momenta(unknown);
  for (std::size_t a = 0; a < count; ++a) {
    const Vec3 p0 = earlier.constituents[a].momentum;
    const double h0 = constituent_energy(p0, options);
    const double h1 = constituent_energy(p1[a], options);
    const double denominator = h0+h1;
    if (!(denominator > 0.0) || !std::isfinite(denominator)) return candidate;
    candidate.velocities[a] = (p0+p1[a])
        *(C_SPEED*C_SPEED/denominator);
    candidate.later.constituents[a] = point_at(
        effective_position(earlier.constituents[a])
            +candidate.velocities[a]*options.dt, L, p1[a]);
  }
  if (!make_segments(candidate, options.use_sparse_local_current))
    return candidate;
  candidate.later.magnetic_half = prepared.magnetic_later;
  candidate.later.electric = prepared.electric_pre_current;
  for (const auto& segment : candidate.segments)
    add_current(candidate.later.electric, segment,
                -options.polarity_scale);
  gather_impulses(candidate, options, interaction_scale);
  return candidate;
}

Candidate evaluate_reverse(const ConnectedMooreBlockState& later,
                           const ConnectedMooreBlockOptions& options,
                           double interaction_scale,
                           const std::vector<double>& unknown) {
  const int L = later.electric.L;
  const std::size_t count = later.constituents.size();
  const double lambda = options.wave_speed*options.dt;
  Candidate candidate(L, count);
  candidate.later = later;
  copy_metadata(later, candidate.earlier);
  const auto p0 = unflatten_momenta(unknown);
  for (std::size_t a = 0; a < count; ++a) {
    const Vec3 p1 = later.constituents[a].momentum;
    const double h0 = constituent_energy(p0[a], options);
    const double h1 = constituent_energy(p1, options);
    const double denominator = h0+h1;
    if (!(denominator > 0.0) || !std::isfinite(denominator)) return candidate;
    candidate.velocities[a] = (p0[a]+p1)
        *(C_SPEED*C_SPEED/denominator);
    candidate.earlier.constituents[a] = point_at(
        effective_position(later.constituents[a])
            -candidate.velocities[a]*options.dt, L, p0[a]);
  }
  if (!make_segments(candidate, options.use_sparse_local_current))
    return candidate;
  candidate.earlier.electric = later.electric;
  for (const auto& segment : candidate.segments)
    add_current(candidate.earlier.electric, segment,
                +options.polarity_scale);
  const auto magnetic_curl = matched_curl(later.magnetic_half);
  for (std::size_t i = 0; i < candidate.earlier.electric.x.size(); ++i) {
    candidate.earlier.electric.x[i] -= lambda*magnetic_curl.x[i];
    candidate.earlier.electric.y[i] -= lambda*magnetic_curl.y[i];
    candidate.earlier.electric.z[i] -= lambda*magnetic_curl.z[i];
  }
  candidate.earlier.magnetic_half = later.magnetic_half;
  const auto electric_curl = matched_curl_adjoint(candidate.earlier.electric);
  for (std::size_t i = 0; i < candidate.earlier.magnetic_half.x.size(); ++i) {
    candidate.earlier.magnetic_half.x[i] += lambda*electric_curl.x[i];
    candidate.earlier.magnetic_half.y[i] += lambda*electric_curl.y[i];
    candidate.earlier.magnetic_half.z[i] += lambda*electric_curl.z[i];
  }
  gather_impulses(candidate, options, interaction_scale);
  return candidate;
}

bool solve_linear(std::vector<double> matrix, std::vector<double> rhs,
                  std::vector<double>& solution, double& minimum_pivot) {
  const std::size_t n = rhs.size();
  minimum_pivot = INFINITY;
  for (std::size_t column = 0; column < n; ++column) {
    std::size_t pivot = column;
    for (std::size_t row = column+1; row < n; ++row)
      if (std::abs(matrix[row*n+column])
          > std::abs(matrix[pivot*n+column])) pivot = row;
    const double pivot_value = matrix[pivot*n+column];
    if (!std::isfinite(pivot_value) || std::abs(pivot_value) < 1e-14)
      return false;
    if (pivot != column) {
      for (std::size_t entry = column; entry < n; ++entry)
        std::swap(matrix[pivot*n+entry], matrix[column*n+entry]);
      std::swap(rhs[pivot], rhs[column]);
    }
    minimum_pivot = std::min(minimum_pivot,
                             std::abs(matrix[column*n+column]));
    for (std::size_t row = column+1; row < n; ++row) {
      const double factor = matrix[row*n+column]
          /matrix[column*n+column];
      for (std::size_t entry = column; entry < n; ++entry)
        matrix[row*n+entry] -= factor*matrix[column*n+entry];
      rhs[row] -= factor*rhs[column];
    }
  }
  for (int row = static_cast<int>(n)-1; row >= 0; --row) {
    double value = rhs[static_cast<std::size_t>(row)];
    for (std::size_t column = static_cast<std::size_t>(row)+1;
         column < n; ++column)
      value -= matrix[static_cast<std::size_t>(row)*n+column]*solution[column];
    solution[static_cast<std::size_t>(row)] = value
        /matrix[static_cast<std::size_t>(row)*n
                +static_cast<std::size_t>(row)];
  }
  return true;
}

struct RootResult {
  Candidate candidate;
  ConnectedMooreBlockSolveDiagnostics diagnostics{};
  std::vector<double> accepted_unknown;
  RootResult(int L, std::size_t count) : candidate(L, count) {}
};

RootResult solve_root_low_rank_identity_broyden(
    int L, std::size_t count, const std::vector<double>& initial,
    const ConnectedMooreBlockOptions& options,
    const std::function<Candidate(const std::vector<double>&)>& evaluate) {
  RootResult result(L, count);
  result.diagnostics.attempted = true;
  result.diagnostics.minimum_abs_jacobian_pivot = INFINITY;
  result.diagnostics.identity_broyden_seeds = 1;
  std::vector<double> unknown = initial;
  const auto evaluate_counted = [&](const std::vector<double>& value) {
    ++result.diagnostics.residual_evaluations;
    return evaluate(value);
  };
  Candidate current = evaluate_counted(unknown);
  if (!current.valid) {
    result.diagnostics.residual = INFINITY;
    result.diagnostics.minimum_abs_jacobian_pivot = 0.0;
    return result;
  }

  const std::size_t n = unknown.size();
  std::vector<std::vector<double>> updates_u;
  std::vector<std::vector<double>> updates_v;
  bool identity_restart_used = false;
  for (int iteration = 0; iteration <= options.max_iterations; ++iteration) {
    const double residual = infinity_norm(current.residual);
    result.diagnostics.iterations = iteration;
    result.diagnostics.residual = residual;
    if (residual <= options.solve_tolerance) {
      result.diagnostics.converged = true;
      break;
    }
    if (iteration == options.max_iterations) break;

    std::vector<double> rhs(n, 0.0), step(n, 0.0);
    for (std::size_t row = 0; row < n; ++row)
      rhs[row] = -current.residual[row];
    step = rhs;

    const std::size_t rank = updates_u.size();
    if (rank > 0) {
      // (I + U V^T)^-1 b = b - U (I + V^T U)^-1 V^T b.
      std::vector<double> small(rank*rank, 0.0), projected(rank, 0.0);
      for (std::size_t row = 0; row < rank; ++row) {
        small[row*rank+row] = 1.0;
        for (std::size_t i = 0; i < n; ++i)
          projected[row] += updates_v[row][i]*rhs[i];
        for (std::size_t column = 0; column < rank; ++column)
          for (std::size_t i = 0; i < n; ++i)
            small[row*rank+column] +=
                updates_v[row][i]*updates_u[column][i];
      }
      std::vector<double> correction(rank, 0.0);
      double minimum_pivot = 0.0;
      if (!solve_linear(small, projected, correction, minimum_pivot)) break;
      result.diagnostics.minimum_abs_jacobian_pivot = std::min(
          result.diagnostics.minimum_abs_jacobian_pivot, minimum_pivot);
      ++result.diagnostics.jacobian_reuses;
      for (std::size_t column = 0; column < rank; ++column)
        for (std::size_t i = 0; i < n; ++i)
          step[i] -= updates_u[column][i]*correction[column];
    } else {
      result.diagnostics.minimum_abs_jacobian_pivot = std::min(
          result.diagnostics.minimum_abs_jacobian_pivot, 1.0);
    }

    bool accepted = false;
    double scale = 1.0;
    for (int line = 0; line < 18; ++line) {
      auto trial = unknown;
      for (std::size_t i = 0; i < n; ++i) trial[i] += scale*step[i];
      Candidate trial_candidate = evaluate_counted(trial);
      if (trial_candidate.valid
          && infinity_norm(trial_candidate.residual) < residual) {
        std::vector<double> change(n, 0.0), jacobian_change(n, 0.0);
        long double denominator = 0.0L;
        for (std::size_t i = 0; i < n; ++i) {
          change[i] = trial[i]-unknown[i];
          jacobian_change[i] = change[i];
          denominator += static_cast<long double>(change[i])*change[i];
        }
        for (std::size_t update = 0; update < rank; ++update) {
          long double projection = 0.0L;
          for (std::size_t i = 0; i < n; ++i)
            projection += static_cast<long double>(updates_v[update][i])
                *change[i];
          for (std::size_t i = 0; i < n; ++i)
            jacobian_change[i] += updates_u[update][i]
                *static_cast<double>(projection);
        }
        if (denominator > 1e-30L) {
          std::vector<double> next_u(n, 0.0), next_v(n, 0.0);
          for (std::size_t i = 0; i < n; ++i) {
            next_u[i] = trial_candidate.residual[i]-current.residual[i]
                -jacobian_change[i];
            next_v[i] = static_cast<double>(
                static_cast<long double>(change[i])/denominator);
          }
          updates_u.push_back(std::move(next_u));
          updates_v.push_back(std::move(next_v));
        }
        result.diagnostics.step_residual = infinity_norm(change);
        unknown = std::move(trial);
        current = std::move(trial_candidate);
        accepted = true;
        break;
      }
      scale *= 0.5;
      ++result.diagnostics.rejected_steps;
    }
    if (!accepted) {
      // A learned secant subspace can become stale.  One restart returns to
      // the exact free-limit identity without changing the root equation.
      if (!updates_u.empty() && !identity_restart_used) {
        updates_u.clear();
        updates_v.clear();
        identity_restart_used = true;
        ++result.diagnostics.identity_broyden_seeds;
        continue;
      }
      break;
    }
  }
  if (!std::isfinite(result.diagnostics.minimum_abs_jacobian_pivot))
    result.diagnostics.minimum_abs_jacobian_pivot = 0.0;
  result.accepted_unknown = unknown;
  result.candidate = std::move(current);
  return result;
}

RootResult solve_root_matrix_free_newton_krylov(
    int L, std::size_t count, const std::vector<double>& initial,
    const ConnectedMooreBlockOptions& options,
    const std::function<Candidate(const std::vector<double>&)>& evaluate) {
  RootResult result(L, count);
  result.diagnostics.attempted = true;
  result.diagnostics.minimum_abs_jacobian_pivot = INFINITY;
  std::vector<double> unknown = initial;
  const auto evaluate_counted = [&](const std::vector<double>& value) {
    ++result.diagnostics.residual_evaluations;
    return evaluate(value);
  };
  Candidate current = evaluate_counted(unknown);
  if (!current.valid) {
    result.diagnostics.residual = INFINITY;
    result.diagnostics.minimum_abs_jacobian_pivot = 0.0;
    return result;
  }

  const std::size_t n = unknown.size();
  const std::size_t restart = std::min<std::size_t>(32, n);
  auto inner_product = [](const std::vector<double>& lhs,
                          const std::vector<double>& rhs) {
    long double value = 0.0L;
    for (std::size_t i = 0; i < lhs.size(); ++i)
      value += static_cast<long double>(lhs[i])*rhs[i];
    return value;
  };
  auto two_norm = [&](const std::vector<double>& value) {
    return std::sqrt(std::max(0.0L, inner_product(value,value)));
  };

  for (int iteration = 0; iteration <= options.max_iterations; ++iteration) {
    const double residual = infinity_norm(current.residual);
    result.diagnostics.iterations = iteration;
    result.diagnostics.residual = residual;
    if (residual <= options.solve_tolerance) {
      result.diagnostics.converged = true;
      break;
    }
    if (iteration == options.max_iterations) break;

    std::vector<double> rhs(n, 0.0);
    for (std::size_t i = 0; i < n; ++i) rhs[i] = -current.residual[i];
    const double beta = two_norm(rhs);
    if (!(beta > 0.0) || !std::isfinite(beta)) break;

    std::vector<std::vector<double>> basis(
        restart+1, std::vector<double>(n, 0.0));
    for (std::size_t i = 0; i < n; ++i) basis[0][i] = rhs[i]/beta;
    std::vector<double> hessenberg((restart+1)*restart, 0.0);
    std::vector<double> cosine(restart, 0.0), sine(restart, 0.0);
    std::vector<double> projected(restart+1, 0.0);
    projected[0] = beta;
    std::size_t used = 0;
    bool krylov_valid = true;
    const double directional_delta = options.finite_difference_scale
        *std::max(1.0, infinity_norm(unknown));
    for (std::size_t column = 0; column < restart; ++column) {
      auto perturbed_unknown = unknown;
      for (std::size_t i = 0; i < n; ++i)
        perturbed_unknown[i] += directional_delta*basis[column][i];
      const Candidate perturbed = evaluate_counted(perturbed_unknown);
      ++result.diagnostics.krylov_matvecs;
      if (!perturbed.valid) {
        krylov_valid = false;
        break;
      }
      std::vector<double> image(n, 0.0);
      for (std::size_t i = 0; i < n; ++i)
        image[i] = (perturbed.residual[i]-current.residual[i])
            /directional_delta;

      for (std::size_t row = 0; row <= column; ++row) {
        const double projection = static_cast<double>(
            inner_product(basis[row],image));
        hessenberg[row*restart+column] = projection;
        for (std::size_t i = 0; i < n; ++i)
          image[i] -= projection*basis[row][i];
      }
      const double next_norm = two_norm(image);
      hessenberg[(column+1)*restart+column] = next_norm;
      if (next_norm > 1e-14)
        for (std::size_t i = 0; i < n; ++i)
          basis[column+1][i] = image[i]/next_norm;

      for (std::size_t rotation = 0; rotation < column; ++rotation) {
        const double upper = hessenberg[rotation*restart+column];
        const double lower = hessenberg[(rotation+1)*restart+column];
        hessenberg[rotation*restart+column] =
            cosine[rotation]*upper+sine[rotation]*lower;
        hessenberg[(rotation+1)*restart+column] =
            -sine[rotation]*upper+cosine[rotation]*lower;
      }
      const double upper = hessenberg[column*restart+column];
      const double lower = hessenberg[(column+1)*restart+column];
      const double radius = std::hypot(upper,lower);
      if (!(radius > 1e-14) || !std::isfinite(radius)) {
        krylov_valid = false;
        break;
      }
      cosine[column] = upper/radius;
      sine[column] = lower/radius;
      hessenberg[column*restart+column] = radius;
      hessenberg[(column+1)*restart+column] = 0.0;
      projected[column+1] = -sine[column]*projected[column];
      projected[column] = cosine[column]*projected[column];
      used = column+1;
      const double inner_tolerance = std::max(
          0.05*beta, 0.1*options.solve_tolerance);
      if (std::abs(projected[column+1]) <= inner_tolerance) break;
    }
    if (!krylov_valid || used == 0) break;

    std::vector<double> coefficients(used, 0.0);
    double minimum_pivot = INFINITY;
    for (int row = static_cast<int>(used)-1; row >= 0; --row) {
      double value = projected[static_cast<std::size_t>(row)];
      for (std::size_t column = static_cast<std::size_t>(row)+1;
           column < used; ++column)
        value -= hessenberg[static_cast<std::size_t>(row)*restart+column]
            *coefficients[column];
      const double pivot = hessenberg[static_cast<std::size_t>(row)*restart
          +static_cast<std::size_t>(row)];
      if (!(std::abs(pivot) > 1e-14) || !std::isfinite(pivot)) {
        krylov_valid = false;
        break;
      }
      minimum_pivot = std::min(minimum_pivot,std::abs(pivot));
      coefficients[static_cast<std::size_t>(row)] = value/pivot;
    }
    if (!krylov_valid) break;
    result.diagnostics.minimum_abs_jacobian_pivot = std::min(
        result.diagnostics.minimum_abs_jacobian_pivot, minimum_pivot);
    std::vector<double> step(n, 0.0);
    for (std::size_t column = 0; column < used; ++column)
      for (std::size_t i = 0; i < n; ++i)
        step[i] += basis[column][i]*coefficients[column];

    bool accepted = false;
    double scale = 1.0;
    for (int line = 0; line < 18; ++line) {
      auto trial = unknown;
      for (std::size_t i = 0; i < n; ++i) trial[i] += scale*step[i];
      Candidate trial_candidate = evaluate_counted(trial);
      if (trial_candidate.valid
          && infinity_norm(trial_candidate.residual) < residual) {
        std::vector<double> change(n, 0.0);
        for (std::size_t i = 0; i < n; ++i)
          change[i] = trial[i]-unknown[i];
        result.diagnostics.step_residual = infinity_norm(change);
        unknown = std::move(trial);
        current = std::move(trial_candidate);
        accepted = true;
        break;
      }
      scale *= 0.5;
      ++result.diagnostics.rejected_steps;
    }
    if (!accepted) break;
  }
  if (!std::isfinite(result.diagnostics.minimum_abs_jacobian_pivot))
    result.diagnostics.minimum_abs_jacobian_pivot = 0.0;
  result.accepted_unknown = unknown;
  result.candidate = std::move(current);
  return result;
}

RootResult solve_root_dense(
    int L, std::size_t count, const std::vector<double>& initial,
    const ConnectedMooreBlockOptions& options,
    const std::function<Candidate(const std::vector<double>&)>& evaluate,
    ConnectedMooreBlockSolveCache* cache) {
  RootResult result(L, count);
  result.diagnostics.attempted = true;
  result.diagnostics.minimum_abs_jacobian_pivot = INFINITY;
  std::vector<double> unknown = initial;
  const auto evaluate_counted = [&](const std::vector<double>& value) {
    ++result.diagnostics.residual_evaluations;
    return evaluate(value);
  };
  Candidate current = evaluate_counted(unknown);
  if (!current.valid) {
    result.diagnostics.residual = INFINITY;
    result.diagnostics.minimum_abs_jacobian_pivot = 0.0;
    return result;
  }
  const std::size_t n = unknown.size();
  for (int iteration = 0; iteration <= options.max_iterations; ++iteration) {
    const double residual = infinity_norm(current.residual);
    result.diagnostics.iterations = iteration;
    result.diagnostics.residual = residual;
    if (residual <= options.solve_tolerance) {
      result.diagnostics.converged = true;
      break;
    }
    if (iteration == options.max_iterations) break;
    std::vector<double> jacobian;
    const bool reused_cached_jacobian = cache && cache->valid
        && cache->dimension == n && cache->jacobian.size() == n*n;
    bool usable = reused_cached_jacobian;
    if (usable) {
      jacobian = cache->jacobian;
      ++result.diagnostics.jacobian_reuses;
    } else {
      jacobian.assign(n*n, 0.0);
      usable = true;
      for (std::size_t column = 0; column < n; ++column) {
        const double delta = options.finite_difference_scale
            *std::max(1.0, std::abs(unknown[column]));
        auto high = unknown;
        auto low = unknown;
        high[column] += delta;
        low[column] -= delta;
        const Candidate high_candidate = evaluate_counted(high);
        const Candidate low_candidate = evaluate_counted(low);
        if (!high_candidate.valid || !low_candidate.valid) {
          usable = false;
          break;
        }
        for (std::size_t row = 0; row < n; ++row)
          jacobian[row*n+column] = (high_candidate.residual[row]
              -low_candidate.residual[row])/(2.0*delta);
      }
      if (usable) {
        ++result.diagnostics.jacobian_refreshes;
        if (cache) {
          cache->valid = true;
          cache->dimension = n;
          cache->jacobian = jacobian;
        }
      }
    }
    std::vector<double> rhs(n, 0.0), step(n, 0.0);
    for (std::size_t row = 0; row < n; ++row)
      rhs[row] = -current.residual[row];
    double minimum_pivot = 0.0;
    if (!usable || !solve_linear(jacobian, rhs, step, minimum_pivot)) break;
    result.diagnostics.minimum_abs_jacobian_pivot = std::min(
        result.diagnostics.minimum_abs_jacobian_pivot, minimum_pivot);
    bool accepted = false;
    double scale = 1.0;
    for (int line = 0; line < 18; ++line) {
      auto trial = unknown;
      for (std::size_t i = 0; i < n; ++i) trial[i] += scale*step[i];
      Candidate trial_candidate = evaluate_counted(trial);
      if (trial_candidate.valid
          && infinity_norm(trial_candidate.residual) < residual) {
        std::vector<double> change(n, 0.0);
        for (std::size_t i = 0; i < n; ++i)
          change[i] = trial[i]-unknown[i];
        // Good Broyden update of the cached residual Jacobian.  This preserves
        // the exact root equation while following slow state-to-state drift.
        if (cache) {
          std::vector<double> secant(n, 0.0);
          long double denominator = 0.0L;
          for (std::size_t row = 0; row < n; ++row) {
            long double image = 0.0L;
            for (std::size_t column = 0; column < n; ++column)
              image += static_cast<long double>(jacobian[row*n+column])
                  *change[column];
            secant[row] = trial_candidate.residual[row]
                -current.residual[row]-static_cast<double>(image);
            denominator += static_cast<long double>(change[row])*change[row];
          }
          if (denominator > 1e-30L) {
            for (std::size_t row = 0; row < n; ++row)
              for (std::size_t column = 0; column < n; ++column)
                jacobian[row*n+column] += static_cast<double>(
                    static_cast<long double>(secant[row])*change[column]
                    /denominator);
            cache->valid = true;
            cache->dimension = n;
            cache->jacobian = jacobian;
          }
        }
        result.diagnostics.step_residual = infinity_norm(change);
        unknown = std::move(trial);
        current = std::move(trial_candidate);
        accepted = true;
        break;
      }
      scale *= 0.5;
      ++result.diagnostics.rejected_steps;
    }
    if (!accepted) {
      // A stale secant matrix is an acceleration miss, not a failed physical
      // solve.  Discard it and let the next iteration rebuild the same locked
      // central-difference Jacobian used by the uncached production path.
      if (reused_cached_jacobian && cache) {
        cache->reset();
        continue;
      }
      break;
    }
  }
  if (!std::isfinite(result.diagnostics.minimum_abs_jacobian_pivot))
    result.diagnostics.minimum_abs_jacobian_pivot = 0.0;
  result.accepted_unknown = unknown;
  result.candidate = std::move(current);
  return result;
}

struct SingularExtrema {
  bool valid = false;
  int evaluations = 0;
  double minimum = 0.0;
  double maximum = 0.0;
};

SingularExtrema residual_jacobian_singular_extrema(
    const std::vector<double>& root, double scale,
    const std::function<Candidate(const std::vector<double>&)>& evaluate) {
  SingularExtrema result;
  const std::size_t n = root.size();
  if (n == 0 || !(scale > 0.0) || !std::isfinite(scale)) return result;
  std::vector<double> jacobian(n*n, 0.0);
  for (std::size_t column = 0; column < n; ++column) {
    const double delta = scale*std::max(1.0, std::abs(root[column]));
    auto high = root;
    auto low = root;
    high[column] += delta;
    low[column] -= delta;
    const Candidate high_candidate = evaluate(high);
    const Candidate low_candidate = evaluate(low);
    result.evaluations += 2;
    if (!high_candidate.valid || !low_candidate.valid
        || high_candidate.residual.size() != n
        || low_candidate.residual.size() != n) return result;
    for (std::size_t row = 0; row < n; ++row)
      jacobian[row*n+column] = (high_candidate.residual[row]
          -low_candidate.residual[row])/(2.0*delta);
  }

  // The eigenvalues of J^T J are the squared singular values.  The unknown
  // space is only 3N (six coordinates for the FTD-0735 pair), so a symmetric
  // Jacobi diagonalization is both transparent and independent of the root
  // solver's LU/Broyden acceleration path.
  std::vector<double> gram(n*n, 0.0);
  for (std::size_t row = 0; row < n; ++row)
    for (std::size_t column = 0; column < n; ++column) {
      long double value = 0.0L;
      for (std::size_t k = 0; k < n; ++k)
        value += static_cast<long double>(jacobian[k*n+row])
            *jacobian[k*n+column];
      gram[row*n+column] = static_cast<double>(value);
    }
  for (std::size_t sweep = 0; sweep < 64; ++sweep) {
    double largest = 0.0, diagonal_scale = 0.0;
    for (std::size_t i = 0; i < n; ++i)
      diagonal_scale = std::max(diagonal_scale,
                                std::abs(gram[i*n+i]));
    for (std::size_t p = 0; p < n; ++p)
      for (std::size_t q = p+1; q < n; ++q) {
        const double apq = gram[p*n+q];
        largest = std::max(largest, std::abs(apq));
        if (std::abs(apq)
            <= 1e-16*std::max(1.0, diagonal_scale)) continue;
        const double app = gram[p*n+p];
        const double aqq = gram[q*n+q];
        const double angle = 0.5*std::atan2(2.0*apq, aqq-app);
        const double c = std::cos(angle);
        const double s = std::sin(angle);
        for (std::size_t k = 0; k < n; ++k) {
          if (k == p || k == q) continue;
          const double akp = gram[k*n+p];
          const double akq = gram[k*n+q];
          const double next_kp = c*akp-s*akq;
          const double next_kq = s*akp+c*akq;
          gram[k*n+p] = gram[p*n+k] = next_kp;
          gram[k*n+q] = gram[q*n+k] = next_kq;
        }
        gram[p*n+p] = c*c*app-2.0*s*c*apq+s*s*aqq;
        gram[q*n+q] = s*s*app+2.0*s*c*apq+c*c*aqq;
        gram[p*n+q] = gram[q*n+p] = 0.0;
      }
    if (largest <= 1e-14*std::max(1.0, diagonal_scale)) break;
  }
  double minimum_squared = INFINITY;
  double maximum_squared = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    if (!std::isfinite(gram[i*n+i]) || gram[i*n+i] < -1e-12)
      return result;
    const double value = std::max(0.0, gram[i*n+i]);
    minimum_squared = std::min(minimum_squared, value);
    maximum_squared = std::max(maximum_squared, value);
  }
  result.minimum = std::sqrt(minimum_squared);
  result.maximum = std::sqrt(maximum_squared);
  result.valid = std::isfinite(result.minimum)
      && std::isfinite(result.maximum) && result.maximum > 0.0;
  return result;
}

void measure_final_root_regularity(
    RootResult& result, const ConnectedMooreBlockOptions& options,
    const std::function<Candidate(const std::vector<double>&)>& evaluate) {
  if (!options.measure_final_root_regularity
      || !result.diagnostics.converged || !result.candidate.valid
      || result.accepted_unknown.empty()) return;
  const auto coarse = residual_jacobian_singular_extrema(
      result.accepted_unknown, options.finite_difference_scale, evaluate);
  const auto fine = residual_jacobian_singular_extrema(
      result.accepted_unknown, 0.5*options.finite_difference_scale, evaluate);
  result.diagnostics.regularity_residual_evaluations =
      coarse.evaluations+fine.evaluations;
  if (!coarse.valid || !fine.valid || !(fine.minimum > 0.0)) return;
  result.diagnostics.final_root_regularity_measured = true;
  result.diagnostics.final_minimum_singular_value = fine.minimum;
  result.diagnostics.final_maximum_singular_value = fine.maximum;
  result.diagnostics.final_condition_number = fine.maximum/fine.minimum;
  result.diagnostics.regularity_scale_relative_difference =
      std::abs(coarse.minimum-fine.minimum)
      /std::max({coarse.minimum, fine.minimum, 1e-30});
}

RootResult solve_root(
    int L, std::size_t count, const std::vector<double>& initial,
    const ConnectedMooreBlockOptions& options,
    const std::function<Candidate(const std::vector<double>&)>& evaluate,
    ConnectedMooreBlockSolveCache* cache) {
  RootResult result = options.use_matrix_free_newton_krylov
      ? solve_root_matrix_free_newton_krylov(
          L, count, initial, options, evaluate)
      : (options.use_low_rank_identity_broyden
          ? solve_root_low_rank_identity_broyden(
              L, count, initial, options, evaluate)
          : solve_root_dense(L, count, initial, options, evaluate, cache));
  measure_final_root_regularity(result, options, evaluate);
  return result;
}

void materialize_forward_local_candidate(
    RootResult& root,
    const ConnectedMooreBlockState& earlier,
    PreparedForwardFields& prepared,
    const ConnectedMooreBlockOptions& options) {
  if (!root.diagnostics.converged || !root.candidate.valid) return;
  Candidate physical(0, earlier.constituents.size());
  copy_metadata(earlier, physical.earlier);
  physical.earlier.constituents = earlier.constituents;
  // Deferred volume diagnostics are measured against the device-resident
  // before field.  Preserve its lattice identity without duplicating six
  // O(L^3) host arrays into the returned step record.
  physical.earlier.electric.L = earlier.electric.L;
  physical.earlier.magnetic_half.L = earlier.magnetic_half.L;
  copy_metadata(earlier, physical.later);
  physical.later.constituents = root.candidate.later.constituents;
  physical.later.magnetic_half = std::move(prepared.magnetic_later);
  physical.later.electric = std::move(prepared.electric_pre_current);
  physical.segments = root.candidate.segments;
  for (const auto& segment : physical.segments)
    add_current(physical.later.electric, segment, -options.polarity_scale);
  physical.gathers = root.candidate.gathers;
  physical.velocities = root.candidate.velocities;
  physical.electric_impulses = root.candidate.electric_impulses;
  physical.magnetic_impulses = root.candidate.magnetic_impulses;
  physical.binding_impulses = root.candidate.binding_impulses;
  physical.total_impulses = root.candidate.total_impulses;
  physical.residual = root.candidate.residual;
  physical.valid = root.candidate.valid;
  root.candidate = std::move(physical);
  ++root.diagnostics.full_candidate_materializations;
  root.diagnostics.materialized_residual_difference = 0.0;
  root.diagnostics.residual = infinity_norm(root.candidate.residual);
  root.diagnostics.converged =
      root.diagnostics.residual <= options.solve_tolerance;
}

double aggregate_continuity_residual(
    const std::vector<QuadraticCoatFaceCurrent>& segments, int L,
    double polarity_scale) {
  const bool all_sparse = std::all_of(segments.begin(), segments.end(),
      [](const QuadraticCoatFaceCurrent& segment) {
        return !segment.dense_materialized;
      });
  if (all_sparse) {
    std::map<std::size_t,long double> residual;
    for (const auto& segment : segments) {
      for (std::size_t item = 0;
           item < segment.start_coat.weight_count; ++item) {
        const auto& entry = segment.start_coat.weights[item];
        residual[index(L,entry.site.x,entry.site.y,entry.site.z)]
            -= polarity_scale*entry.weight;
      }
      for (std::size_t item = 0;
           item < segment.end_coat.weight_count; ++item) {
        const auto& entry = segment.end_coat.weights[item];
        residual[index(L,entry.site.x,entry.site.y,entry.site.z)]
            += polarity_scale*entry.weight;
      }
      for (const auto& entry : segment.sparse_current) {
        const long double value = polarity_scale*entry.value;
        residual[index(L,entry.face.x,entry.face.y,entry.face.z)] += value;
        Coord next = entry.face;
        if (entry.axis == 0) ++next.x;
        else if (entry.axis == 1) ++next.y;
        else ++next.z;
        residual[index(L,next.x,next.y,next.z)] -= value;
      }
    }
    double maximum = 0.0;
    for (const auto& item : residual)
      maximum = std::max(maximum,
          std::abs(static_cast<double>(item.second)));
    return maximum;
  }
  std::vector<double> rho0(volume(L), 0.0), rho1(volume(L), 0.0);
  MatchedFaceFlux current(L);
  for (const auto& segment : segments) {
    if (segment.dense_materialized) {
      for (std::size_t i = 0; i < rho0.size(); ++i) {
        rho0[i] += polarity_scale*segment.rho_before[i];
        rho1[i] += polarity_scale*segment.rho_after[i];
      }
    } else {
      for (std::size_t item = 0;
           item < segment.start_coat.weight_count; ++item) {
        const auto& entry = segment.start_coat.weights[item];
        rho0[index(L, entry.site.x, entry.site.y, entry.site.z)]
            += polarity_scale*entry.weight;
      }
      for (std::size_t item = 0;
           item < segment.end_coat.weight_count; ++item) {
        const auto& entry = segment.end_coat.weights[item];
        rho1[index(L, entry.site.x, entry.site.y, entry.site.z)]
            += polarity_scale*entry.weight;
      }
    }
    add_current(current, segment, polarity_scale);
  }
  double result = 0.0;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = current.index(x,y,z);
        const double divergence = current.x[static_cast<std::size_t>(i)]
            -current.x[static_cast<std::size_t>(current.index(x-1,y,z))]
            +current.y[static_cast<std::size_t>(i)]
            -current.y[static_cast<std::size_t>(current.index(x,y-1,z))]
            +current.z[static_cast<std::size_t>(i)]
            -current.z[static_cast<std::size_t>(current.index(x,y,z-1))];
        result = std::max(result, std::abs(
            rho1[static_cast<std::size_t>(i)]
            -rho0[static_cast<std::size_t>(i)]+divergence));
      }
  return result;
}

std::vector<double> aggregate_density(
    const std::vector<QuadraticCoatFaceCurrent>& segments, bool later,
    double polarity_scale) {
  if (segments.empty()) return {};
  const int L = segments[0].L;
  std::vector<double> result(volume(L), 0.0);
  for (const auto& segment : segments) {
    if (segment.dense_materialized) {
      const auto& density = later ? segment.rho_after : segment.rho_before;
      for (std::size_t i = 0; i < result.size(); ++i)
        result[i] += polarity_scale*density[i];
    } else {
      const auto& coat = later ? segment.end_coat : segment.start_coat;
      for (std::size_t item = 0; item < coat.weight_count; ++item) {
        const auto& entry = coat.weights[item];
        result[index(L, entry.site.x, entry.site.y, entry.site.z)]
            += polarity_scale*entry.weight;
      }
    }
  }
  return result;
}

Vec3 center(const ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents)
    result += effective_position(point);
  return result*(1.0/static_cast<double>(state.constituents.size()));
}

Vec3 momentum(const ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += point.momentum;
  return result;
}

int site_hops(const ConnectedMooreBlockState& lhs,
              const ConnectedMooreBlockState& rhs) {
  int result = 0;
  for (std::size_t a = 0; a < lhs.constituents.size(); ++a)
    if (!same_anchor(lhs.constituents[a].anchor,
                     rhs.constituents[a].anchor)) ++result;
  return result;
}

double maximum_edge_strain(const ConnectedMooreBlockState& state) {
  const auto x = positions(state);
  double result = 0.0;
  for (const auto& edge : state.edges) {
    const Vec3 delta = x[edge.first]-x[edge.second];
    const double squared = delta.dot(delta);
    result = std::max(result,
        std::abs(squared-edge.rest_length_squared));
  }
  return result;
}

ConnectedMooreBlockStepResult finalize(
    RootResult& root, bool forward,
    const ConnectedMooreBlockOptions& options,
    const FaceFluxNormalization& normalization) {
  ConnectedMooreBlockStepResult result;
  result.forward = forward;
  result.solve = root.diagnostics;
  result.normalization = normalization;
  result.interaction_scale = normalization.mapped_field_work_coefficient
      *options.field_energy_scale;
  result.constituent_mass_scale = options.constituent_mass_scale;
  result.polarity_scale = options.polarity_scale;
  result.field_energy_scale = options.field_energy_scale;
  // A local residual candidate is an algebraic scratch record: it carries the
  // accepted constituent coordinates and face-current segments, but omits the
  // O(L^3) fields intentionally.  A stopped nonlinear solve may therefore
  // leave candidate.valid true without ever materializing a physical state.
  // Do not run continuity/Gauss/energy diagnostics on that scratch record.
  if (!root.diagnostics.converged || !root.candidate.valid) return result;
  Candidate& candidate = root.candidate;
  if (options.defer_volume_diagnostics) {
    result.earlier = std::move(candidate.earlier);
    result.later = std::move(candidate.later);
    result.segments = std::move(candidate.segments);
    result.gathers = std::move(candidate.gathers);
    result.velocities = std::move(candidate.velocities);
    result.electric_impulses = std::move(candidate.electric_impulses);
    result.magnetic_impulses = std::move(candidate.magnetic_impulses);
    result.binding_impulses = std::move(candidate.binding_impulses);
    result.total_impulses = std::move(candidate.total_impulses);
  } else {
    result.earlier = candidate.earlier;
    result.later = candidate.later;
    result.segments = candidate.segments;
    result.gathers = candidate.gathers;
    result.velocities = candidate.velocities;
    result.electric_impulses = candidate.electric_impulses;
    result.magnetic_impulses = candidate.magnetic_impulses;
    result.binding_impulses = candidate.binding_impulses;
    result.total_impulses = candidate.total_impulses;
  }
  for (int charge : result.earlier.charges) result.net_charge += charge;
  result.site_projection_valid = site_projection_valid(result.earlier)
      && site_projection_valid(result.later);
  const bool derived_pair =
      options.binding_law == ConnectedBindingLaw::DerivedCompactPair;
  if (derived_pair) {
    const auto earlier_positions = positions(result.earlier);
    const auto later_positions = positions(result.later);
    result.relational_edge_before = earlier_positions.size() == 2
        && (earlier_positions[0]-earlier_positions[1]).mag2()
            < options.compact_pair_cutoff_distance_squared;
    result.relational_edge_after = later_positions.size() == 2
        && (later_positions[0]-later_positions[1]).mag2()
            < options.compact_pair_cutoff_distance_squared;
    result.relational_graph_changed = result.relational_edge_before
        != result.relational_edge_after;
    result.graph_connected = result.relational_edge_before;
    result.graph_local = true;
  } else {
    graph_valid(result.earlier, &result.graph_connected, &result.graph_local);
  }
  result.site_hops = site_hops(result.earlier, result.later);
  result.root_residual = root.diagnostics.residual;
  result.force_residual = infinity_norm(candidate.residual);
  result.continuity_residual = aggregate_continuity_residual(
      result.segments, result.earlier.electric.L, options.polarity_scale);
  if (!options.defer_volume_diagnostics) {
    const auto density0 = aggregate_density(
        result.segments, false, options.polarity_scale);
    const auto density1 = aggregate_density(
        result.segments, true, options.polarity_scale);
    result.gauss_before_residual = max_fractional_gauss_residual(
        result.earlier.electric, density0);
    result.gauss_after_residual = max_fractional_gauss_residual(
        result.later.electric, density1);
  }

  const double lambda = options.wave_speed*options.dt;
  if (!options.defer_volume_diagnostics) {
    result.field_energy_before = result.interaction_scale
        *matched_modified_energy(result.earlier.electric,
            result.earlier.magnetic_half, lambda);
    result.field_energy_after = result.interaction_scale
        *matched_modified_energy(result.later.electric,
            result.later.magnetic_half, lambda);
  }
  long double kinetic0 = 0.0L, kinetic1 = 0.0L;
  long double magnetic_work = 0.0L, current_work = 0.0L;
  for (std::size_t a = 0; a < result.velocities.size(); ++a) {
    const Vec3 p0 = result.earlier.constituents[a].momentum;
    const Vec3 p1 = result.later.constituents[a].momentum;
    const double h0 = constituent_energy(p0, options);
    const double h1 = constituent_energy(p1, options);
    kinetic0 += h0;
    kinetic1 += h1;
    result.kinetic_discrete_gradient_residual = std::max(
        result.kinetic_discrete_gradient_residual,
        std::abs((h1-h0)-result.velocities[a].dot(p1-p0)));
    result.kinematic_residual = std::max(result.kinematic_residual,
        maximum_component(result.segments[a].end_effective_position
          -result.segments[a].start_effective_position
          -result.velocities[a]*options.dt));
    result.electric_adjoint_residual = std::max(
        result.electric_adjoint_residual,
        std::abs(result.gathers[a].electric_adjoint_residual));
    magnetic_work += result.velocities[a].dot(result.magnetic_impulses[a]);
    current_work += result.interaction_scale*result.gathers[a].current_work;
    result.causal_speed_excess = std::max(result.causal_speed_excess,
        std::max(0.0, result.velocities[a].mag()-C_SPEED));
  }
  result.kinetic_energy_before = static_cast<double>(kinetic0);
  result.kinetic_energy_after = static_cast<double>(kinetic1);
  result.magnetic_work_residual = std::abs(static_cast<double>(magnetic_work));
  result.current_work = static_cast<double>(current_work);
  result.binding_energy_before = binding_energy(
      positions(result.earlier), result.earlier.edges, options);
  result.binding_energy_after = binding_energy(
      positions(result.later), result.later.edges, options);
  Vec3 binding_sum{};
  long double binding_work = 0.0L;
  for (std::size_t a = 0; a < result.binding_impulses.size(); ++a) {
    binding_sum += result.binding_impulses[a];
    binding_work += result.velocities[a].dot(result.binding_impulses[a]);
  }
  result.binding_impulse_sum_residual = binding_sum.mag();
  result.binding_work_residual = std::abs(
      result.binding_energy_after-result.binding_energy_before
      +static_cast<double>(binding_work));
  const double matter0 = result.kinetic_energy_before
      +result.binding_energy_before;
  const double matter1 = result.kinetic_energy_after
      +result.binding_energy_after;
  result.matter_work_residual = std::abs(
      matter1-matter0-result.current_work);
  if (!options.defer_volume_diagnostics) {
    result.field_work_residual = std::abs(
        result.field_energy_after-result.field_energy_before
        +result.current_work);
    result.total_energy_residual = std::abs(
        matter1+result.field_energy_after-matter0-result.field_energy_before);
  }

  result.center_before = center(result.earlier);
  result.center_after = center(result.later);
  result.center_displacement = (result.center_after-result.center_before).mag();
  result.maximum_edge_strain = derived_pair ? 0.0 : std::max(
      maximum_edge_strain(result.earlier), maximum_edge_strain(result.later));
  result.matter_momentum_before = momentum(result.earlier);
  result.matter_momentum_after = momentum(result.later);
  bool spline_valid = true;
  if (!options.defer_volume_diagnostics) {
    result.local_field_momentum_before = matched_local_translation_momentum(
        result.earlier.electric, result.earlier.magnetic_half)
        *result.interaction_scale;
    result.local_field_momentum_after = matched_local_translation_momentum(
        result.later.electric, result.later.magnetic_half)
        *result.interaction_scale;
    const auto spline0 = measure_spline_poynting_momentum(
        result.earlier.electric, result.earlier.magnetic_half,
        options.wave_speed, options.dt, result.interaction_scale);
    const auto spline1 = measure_spline_poynting_momentum(
        result.later.electric, result.later.magnetic_half,
        options.wave_speed, options.dt, result.interaction_scale);
    spline_valid = spline0.valid && spline1.valid;
    result.spline_field_momentum_before = spline0.momentum;
    result.spline_field_momentum_after = spline1.momentum;
    const Vec3 matter_delta = result.matter_momentum_after
        -result.matter_momentum_before;
    result.local_total_defect = matter_delta
        +result.local_field_momentum_after-result.local_field_momentum_before;
    result.spline_total_defect = matter_delta
        +result.spline_field_momentum_after-result.spline_field_momentum_before;
    result.local_defect_norm = result.local_total_defect.mag();
    result.spline_defect_norm = result.spline_total_defect.mag();
    if (result.field_energy_before > 0.0)
      result.normalized_spline_defect = options.wave_speed
          *result.spline_defect_norm/result.field_energy_before;
  } else {
    result.volume_diagnostics_pending = true;
    return result;
  }

  const bool finite_values = std::isfinite(result.root_residual)
      && std::isfinite(result.continuity_residual)
      && std::isfinite(result.gauss_before_residual)
      && std::isfinite(result.gauss_after_residual)
      && std::isfinite(result.total_energy_residual)
      && std::isfinite(result.normalized_spline_defect)
      && spline_valid;
  result.valid = result.solve.converged && finite_values
      && (options.allow_shared_anchor_chart || result.site_projection_valid)
      && result.graph_local && result.net_charge == 0
      && (derived_pair || result.graph_connected);
  const double gate = options.gate_tolerance;
  result.common_action_gates_pass = result.valid
      && result.root_residual <= gate
      && result.continuity_residual <= gate
      && result.gauss_before_residual <= gate
      && result.gauss_after_residual <= gate
      && result.force_residual <= gate
      && result.kinematic_residual <= gate
      && result.kinetic_discrete_gradient_residual <= gate
      && result.electric_adjoint_residual <= gate
      && result.magnetic_work_residual <= gate
      && result.binding_work_residual <= gate
      && result.binding_impulse_sum_residual <= gate
      && result.matter_work_residual <= gate
      && result.field_work_residual <= gate
      && result.total_energy_residual <= gate
      && result.causal_speed_excess <= 1e-12;
  return result;
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i])*rhs[i];
  return result;
}

void negative_laplacian(int L, const std::vector<double>& input,
                        std::vector<double>& output) {
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto i = index(L,x,y,z);
        output[i] = 6.0*input[i]
            -input[index(L,x+1,y,z)]-input[index(L,x-1,y,z)]
            -input[index(L,x,y+1,z)]-input[index(L,x,y-1,z)]
            -input[index(L,x,y,z+1)]-input[index(L,x,y,z-1)];
      }
}

std::vector<double> density_of(const ConnectedMooreBlockState& state) {
  const int L = state.electric.L;
  std::vector<double> density(volume(L), 0.0);
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    const auto coat = make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]);
    if (!coat.valid) return {};
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& entry = coat.weights[item];
      density[index(L,entry.site.x,entry.site.y,entry.site.z)] += entry.weight;
    }
  }
  return density;
}

bool initialize_longitudinal(ConnectedMooreBlockInitialization& result,
                             double tolerance, int max_iterations) {
  const int L = result.state.electric.L;
  const auto density = density_of(result.state);
  if (density.size() != volume(L)) return false;
  const long double total = std::accumulate(
      density.begin(), density.end(), 0.0L);
  if (std::abs(static_cast<double>(total)) > 1e-12) return false;
  std::vector<double> potential(volume(L), 0.0);
  std::vector<double> residual = density;
  std::vector<double> direction = density;
  std::vector<double> image(volume(L), 0.0);
  long double rr = dot(residual,residual);
  result.poisson_residual = 0.0;
  for (double value : residual)
    result.poisson_residual = std::max(
        result.poisson_residual, std::abs(value));
  for (int iteration = 1;
       result.poisson_residual > tolerance && iteration <= max_iterations;
       ++iteration) {
    negative_laplacian(L,direction,image);
    const long double denominator = dot(direction,image);
    if (!(denominator > 0.0L)) break;
    const long double alpha = rr/denominator;
    for (std::size_t i = 0; i < potential.size(); ++i) {
      potential[i] += static_cast<double>(alpha*direction[i]);
      residual[i] -= static_cast<double>(alpha*image[i]);
    }
    result.poisson_iterations = iteration;
    result.poisson_residual = 0.0;
    for (double value : residual)
      result.poisson_residual = std::max(
          result.poisson_residual, std::abs(value));
    if (result.poisson_residual <= tolerance) break;
    const long double next = dot(residual,residual);
    const long double ratio = next/rr;
    for (std::size_t i = 0; i < direction.size(); ++i)
      direction[i] = residual[i]+static_cast<double>(ratio*direction[i]);
    rr = next;
  }
  if (result.poisson_residual > tolerance) return false;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto i = index(L,x,y,z);
        result.state.electric.x[i] = potential[i]-potential[index(L,x+1,y,z)];
        result.state.electric.y[i] = potential[i]-potential[index(L,x,y+1,z)];
        result.state.electric.z[i] = potential[i]-potential[index(L,x,y,z+1)];
      }
  result.gauss_residual = max_fractional_gauss_residual(
      result.state.electric, density);
  result.curl_adjoint_residual = max_curl_adjoint(result.state.electric);
  return true;
}

}  // namespace

ConnectedMooreBlockInitialization initialize_connected_moore_block(
    int L, int width, int orientation_axis, int phase_axis, double phase,
    double poisson_tolerance, int poisson_max_iterations) {
  ConnectedMooreBlockInitialization result(L);
  if (L < 5 || width < 1 || orientation_axis < 0 || orientation_axis > 2
      || phase_axis < 0 || phase_axis > 2 || !std::isfinite(phase)
      || phase < -0.5 || phase > 0.5 || 2*width+4 >= L
      || !(poisson_tolerance > 0.0) || poisson_max_iterations <= 0)
    return result;
  result.state.width = width;
  result.state.orientation_axis = orientation_axis;
  std::array<int,3> dimensions{{width,width,width}};
  dimensions[orientation_axis] = 2*width;
  const std::array<int,3> origin{{
      L/2-dimensions[0]/2,
      L/2-dimensions[1]/2,
      L/2-dimensions[2]/2}};
  std::vector<Coord> coordinates;
  for (int x = 0; x < dimensions[0]; ++x)
    for (int y = 0; y < dimensions[1]; ++y)
      for (int z = 0; z < dimensions[2]; ++z) {
        const std::array<int,3> local{{x,y,z}};
        MatchedMatterPoint point;
        point.anchor = {origin[0]+x,origin[1]+y,origin[2]+z};
        if (phase_axis == 0) point.remainder.x = phase;
        if (phase_axis == 1) point.remainder.y = phase;
        if (phase_axis == 2) point.remainder.z = phase;
        result.state.constituents.push_back(point);
        result.state.charges.push_back(
            local[orientation_axis] < width ? +1 : -1);
        coordinates.push_back({x,y,z});
      }
  for (std::size_t a = 0; a < coordinates.size(); ++a)
    for (std::size_t b = a+1; b < coordinates.size(); ++b) {
      const Coord delta{coordinates[b].x-coordinates[a].x,
                        coordinates[b].y-coordinates[a].y,
                        coordinates[b].z-coordinates[a].z};
      const int chebyshev = std::max({std::abs(delta.x),
          std::abs(delta.y),std::abs(delta.z)});
      if (chebyshev != 1) continue;
      const int squared = delta.x*delta.x+delta.y*delta.y+delta.z*delta.z;
      result.state.edges.push_back({a,b,delta,static_cast<double>(squared)});
    }
  result.site_projection_valid = site_projection_valid(result.state);
  graph_valid(result.state, &result.graph_connected, &result.graph_local);
  const bool field = initialize_longitudinal(
      result, poisson_tolerance, poisson_max_iterations);
  result.valid = field && result.site_projection_valid
      && result.graph_connected && result.graph_local
      && result.gauss_residual <= 1e-11
      && result.curl_adjoint_residual <= 1e-11;
  return result;
}

ConnectedMooreBlockInitialization redress_connected_moore_block_with_fibre_limit(
    const ConnectedMooreBlockState& geometry,
    int maximum_anchor_multiplicity,
    double poisson_tolerance,
    int poisson_max_iterations) {
  ConnectedMooreBlockInitialization result(geometry.electric.L);
  const int L = geometry.electric.L;
  if (L < 5 || geometry.magnetic_half.L != L || geometry.width < 1
      || geometry.orientation_axis < 0 || geometry.orientation_axis > 2
      || geometry.constituents.empty()
      || geometry.constituents.size() != geometry.charges.size()
      || maximum_anchor_multiplicity < 1
      || !(poisson_tolerance > 0.0) || poisson_max_iterations <= 0)
    return result;

  result.state = geometry;
  result.state.electric = MatchedFaceFlux(L);
  result.state.magnetic_half = MatchedEdgeField(L);
  result.site_projection_valid = site_projection_valid(result.state);
  graph_valid(result.state, &result.graph_connected, &result.graph_local);

  int net_charge = 0;
  bool matter_valid = true;
  std::map<std::tuple<int,int,int>,int> multiplicities;
  for (std::size_t a = 0; a < result.state.constituents.size(); ++a) {
    const auto& point = result.state.constituents[a];
    const int charge = result.state.charges[a];
    net_charge += charge;
    matter_valid = matter_valid && (charge == -1 || charge == +1)
        && finite(point.remainder) && finite(point.momentum)
        && make_quadratic_polarity_coat(
            effective_position(point), charge).valid;
    const auto key = std::make_tuple(
        point.anchor.x, point.anchor.y, point.anchor.z);
    const int count = ++multiplicities[key];
    if (count > maximum_anchor_multiplicity) matter_valid = false;
  }

  const bool field = matter_valid && net_charge == 0
      && result.graph_connected && result.graph_local
      && initialize_longitudinal(result, poisson_tolerance,
                                 poisson_max_iterations);
  result.valid = field
      && (maximum_anchor_multiplicity > 1 || result.site_projection_valid)
      && result.gauss_residual <= 1e-11
      && result.curl_adjoint_residual <= 1e-11;
  return result;
}

ConnectedMooreBlockInitialization redress_connected_moore_block(
    const ConnectedMooreBlockState& geometry,
    bool allow_shared_anchor_chart,
    double poisson_tolerance,
    int poisson_max_iterations) {
  return redress_connected_moore_block_with_fibre_limit(
      geometry, allow_shared_anchor_chart ? 2 : 1,
      poisson_tolerance, poisson_max_iterations);
}

ConnectedMooreBlockInitialization redress_derived_compact_pair(
    const ConnectedMooreBlockState& geometry,
    const ConnectedMooreBlockOptions& options,
    double poisson_tolerance,
    int poisson_max_iterations) {
  ConnectedMooreBlockInitialization result(geometry.electric.L);
  const int L = geometry.electric.L;
  if (options.binding_law != ConnectedBindingLaw::DerivedCompactPair
      || L < 5 || geometry.magnetic_half.L != L
      || geometry.constituents.size() != 2
      || geometry.charges.size() != 2 || !geometry.edges.empty()
      || geometry.charges[0] != -geometry.charges[1]
      || (geometry.charges[0] != -1 && geometry.charges[0] != +1)
      || !(options.compact_pair_well_depth > 0.0)
      || !std::isfinite(options.compact_pair_well_depth)
      || options.compact_pair_cutoff_distance_squared != 1.5
      || !(poisson_tolerance > 0.0) || poisson_max_iterations <= 0)
    return result;

  result.state = geometry;
  result.state.electric = MatchedFaceFlux(L);
  result.state.magnetic_half = MatchedEdgeField(L);
  result.site_projection_valid = site_projection_valid(result.state);
  bool matter_valid = true;
  for (std::size_t a = 0; a < 2; ++a) {
    const auto& point = result.state.constituents[a];
    matter_valid = matter_valid && finite(point.remainder)
        && finite(point.momentum)
        && make_quadratic_polarity_coat(
            effective_position(point), result.state.charges[a]).valid;
  }
  const auto pair_positions = positions(result.state);
  result.graph_connected = matter_valid
      && (pair_positions[0]-pair_positions[1]).mag2()
          < options.compact_pair_cutoff_distance_squared;
  result.graph_local = matter_valid;
  const bool field = matter_valid
      && initialize_longitudinal(result, poisson_tolerance,
                                 poisson_max_iterations);
  result.valid = field
      && (options.allow_shared_anchor_chart || result.site_projection_valid)
      && result.graph_local && result.gauss_residual <= 1e-11
      && result.curl_adjoint_residual <= 1e-11;
  return result;
}

FiniteSupportPairPreparation prepare_finite_support_derived_compact_pair(
    const ConnectedMooreBlockState& geometry,
    const ConnectedMooreBlockOptions& options,
    int support_half_width,
    double poisson_tolerance,
    int poisson_max_iterations,
    bool allow_fractional_center) {
  FiniteSupportPairPreparation result(geometry.electric.L);
  const int L = geometry.electric.L;
  if (options.binding_law != ConnectedBindingLaw::DerivedCompactPair
      || L < 5 || geometry.magnetic_half.L != L
      || geometry.constituents.size() != 2
      || geometry.charges.size() != 2 || !geometry.edges.empty()
      || geometry.charges[0] != -geometry.charges[1]
      || (geometry.charges[0] != -1 && geometry.charges[0] != +1)
      || support_half_width < 2 || 2*support_half_width+5 >= L
      || !(poisson_tolerance > 0.0) || poisson_max_iterations <= 0)
    return result;

  result.state = geometry;
  result.state.electric = MatchedFaceFlux(L);
  result.state.magnetic_half = MatchedEdgeField(L);
  result.support_half_width = support_half_width;
  const auto pair_positions = positions(result.state);
  const Vec3 centroid = (pair_positions[0]+pair_positions[1])*0.5;
  const Vec3 integer_center{
      static_cast<double>(std::llround(centroid.x)),
      static_cast<double>(std::llround(centroid.y)),
      static_cast<double>(std::llround(centroid.z))};
  result.fractional_center_offset = centroid-integer_center;
  if (!allow_fractional_center
      && result.fractional_center_offset.mag() > 1e-12) return result;
  result.center = centroid;
  result.support_center = integer_center;
  result.fractional_center_enabled = allow_fractional_center;

  bool matter_valid = true;
  for (std::size_t a = 0; a < 2; ++a) {
    const auto& point = result.state.constituents[a];
    matter_valid = matter_valid && finite(point.remainder)
        && finite(point.momentum)
        && make_quadratic_polarity_coat(
            effective_position(point), result.state.charges[a]).valid;
  }
  if (!matter_valid) return result;
  const auto density = density_of(result.state);
  if (density.size() != volume(L)) return result;

  const int side = 2*support_half_width+1;
  const std::size_t local_count = static_cast<std::size_t>(side)*side*side;
  result.support_site_count = static_cast<int>(local_count);
  const int cx = static_cast<int>(std::llround(integer_center.x));
  const int cy = static_cast<int>(std::llround(integer_center.y));
  const int cz = static_cast<int>(std::llround(integer_center.z));
  const auto local_index = [=](int dx, int dy, int dz) {
    return static_cast<std::size_t>(dx+support_half_width)*side*side
        + static_cast<std::size_t>(dy+support_half_width)*side
        + static_cast<std::size_t>(dz+support_half_width);
  };
  const auto inside_local = [=](int dx, int dy, int dz) {
    return std::abs(dx) <= support_half_width
        && std::abs(dy) <= support_half_width
        && std::abs(dz) <= support_half_width;
  };
  std::vector<double> source(local_count, 0.0);
  std::vector<unsigned char> contained(volume(L), 0);
  long double total = 0.0L;
  for (int dx = -support_half_width; dx <= support_half_width; ++dx)
    for (int dy = -support_half_width; dy <= support_half_width; ++dy)
      for (int dz = -support_half_width; dz <= support_half_width; ++dz) {
        const auto global = index(L,cx+dx,cy+dy,cz+dz);
        const double value = density[global];
        source[local_index(dx,dy,dz)] = value;
        contained[global] = 1;
        total += value;
      }
  result.density_contained = true;
  for (std::size_t i = 0; i < density.size(); ++i)
    if (!contained[i] && std::abs(density[i]) > 1e-15)
      result.density_contained = false;
  result.neutral = std::abs(static_cast<double>(total)) <= 1e-12;
  if (!result.density_contained || !result.neutral) return result;

  const auto apply_local_laplacian = [&](const std::vector<double>& input,
                                         std::vector<double>& output) {
    std::fill(output.begin(), output.end(), 0.0);
    constexpr std::array<std::array<int,3>,6> steps{{
        {{1,0,0}},{{-1,0,0}},{{0,1,0}},{{0,-1,0}},{{0,0,1}},{{0,0,-1}}}};
    for (int dx = -support_half_width; dx <= support_half_width; ++dx)
      for (int dy = -support_half_width; dy <= support_half_width; ++dy)
        for (int dz = -support_half_width; dz <= support_half_width; ++dz) {
          const auto i = local_index(dx,dy,dz);
          int degree = 0;
          for (const auto& step : steps) {
            const int nx = dx+step[0], ny = dy+step[1], nz = dz+step[2];
            if (!inside_local(nx,ny,nz)) continue;
            ++degree;
            output[i] -= input[local_index(nx,ny,nz)];
          }
          output[i] += static_cast<double>(degree)*input[i];
        }
  };
  const auto local_dot = [](const std::vector<double>& lhs,
                            const std::vector<double>& rhs) {
    long double value = 0.0L;
    for (std::size_t i = 0; i < lhs.size(); ++i)
      value += static_cast<long double>(lhs[i])*rhs[i];
    return value;
  };

  std::vector<double> potential(local_count, 0.0);
  std::vector<double> residual = source;
  std::vector<double> direction = source;
  std::vector<double> image(local_count, 0.0);
  long double rr = local_dot(residual,residual);
  result.poisson_residual = 0.0;
  for (double value : residual)
    result.poisson_residual = std::max(
        result.poisson_residual, std::abs(value));
  for (int iteration = 1;
       result.poisson_residual > poisson_tolerance
           && iteration <= poisson_max_iterations;
       ++iteration) {
    apply_local_laplacian(direction,image);
    const long double denominator = local_dot(direction,image);
    if (!(denominator > 0.0L)) break;
    const long double alpha = rr/denominator;
    for (std::size_t i = 0; i < local_count; ++i) {
      potential[i] += static_cast<double>(alpha*direction[i]);
      residual[i] -= static_cast<double>(alpha*image[i]);
    }
    result.poisson_iterations = iteration;
    result.poisson_residual = 0.0;
    for (double value : residual)
      result.poisson_residual = std::max(
          result.poisson_residual, std::abs(value));
    if (result.poisson_residual <= poisson_tolerance) break;
    const long double next = local_dot(residual,residual);
    const long double ratio = next/rr;
    for (std::size_t i = 0; i < local_count; ++i)
      direction[i] = residual[i]+static_cast<double>(ratio*direction[i]);
    rr = next;
  }
  if (result.poisson_residual > poisson_tolerance) return result;

  for (int dx = -support_half_width; dx <= support_half_width; ++dx)
    for (int dy = -support_half_width; dy <= support_half_width; ++dy)
      for (int dz = -support_half_width; dz <= support_half_width; ++dz) {
        const auto local = local_index(dx,dy,dz);
        const auto global = index(L,cx+dx,cy+dy,cz+dz);
        if (dx < support_half_width)
          result.state.electric.x[global] = potential[local]
              -potential[local_index(dx+1,dy,dz)];
        if (dy < support_half_width)
          result.state.electric.y[global] = potential[local]
              -potential[local_index(dx,dy+1,dz)];
        if (dz < support_half_width)
          result.state.electric.z[global] = potential[local]
              -potential[local_index(dx,dy,dz+1)];
      }

  result.gauss_residual = max_fractional_gauss_residual(
      result.state.electric, density);
  result.curl_adjoint_residual = max_curl_adjoint(result.state.electric);
  result.electric_energy = quadratic_energy(result.state.electric);

  const auto periodic_delta = [=](int coordinate, int origin) {
    int value = coordinate-origin;
    while (value > L/2) value -= L;
    while (value < -L/2) value += L;
    return value;
  };
  result.outside_maximum = 0.0;
  result.boundary_crossing_maximum = 0.0;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(
            result.state.electric.index(x,y,z));
        const int dx = periodic_delta(x,cx);
        const int dy = periodic_delta(y,cy);
        const int dz = periodic_delta(z,cz);
        const bool x_internal = inside_local(dx,dy,dz)
            && inside_local(dx+1,dy,dz);
        const bool y_internal = inside_local(dx,dy,dz)
            && inside_local(dx,dy+1,dz);
        const bool z_internal = inside_local(dx,dy,dz)
            && inside_local(dx,dy,dz+1);
        if (!x_internal) result.outside_maximum = std::max(
            result.outside_maximum, std::abs(result.state.electric.x[i]));
        if (!y_internal) result.outside_maximum = std::max(
            result.outside_maximum, std::abs(result.state.electric.y[i]));
        if (!z_internal) result.outside_maximum = std::max(
            result.outside_maximum, std::abs(result.state.electric.z[i]));
        const bool x_cross = inside_local(dx,dy,dz) !=
            inside_local(dx+1,dy,dz);
        const bool y_cross = inside_local(dx,dy,dz) !=
            inside_local(dx,dy+1,dz);
        const bool z_cross = inside_local(dx,dy,dz) !=
            inside_local(dx,dy,dz+1);
        if (x_cross) result.boundary_crossing_maximum = std::max(
            result.boundary_crossing_maximum,
            std::abs(result.state.electric.x[i]));
        if (y_cross) result.boundary_crossing_maximum = std::max(
            result.boundary_crossing_maximum,
            std::abs(result.state.electric.y[i]));
        if (z_cross) result.boundary_crossing_maximum = std::max(
            result.boundary_crossing_maximum,
            std::abs(result.state.electric.z[i]));
      }

  result.internal_circulation_residual = 0.0;
  for (int dx = -support_half_width; dx < support_half_width; ++dx)
    for (int dy = -support_half_width; dy < support_half_width; ++dy)
      for (int dz = -support_half_width; dz < support_half_width; ++dz) {
        const int x = cx+dx, y = cy+dy, z = cz+dz;
        const auto& field = result.state.electric;
        result.internal_circulation_residual = std::max({
            result.internal_circulation_residual,
            std::abs(field.x[field.index(x,y,z)]
                +field.y[field.index(x+1,y,z)]
                -field.x[field.index(x,y+1,z)]
                -field.y[field.index(x,y,z)]),
            std::abs(field.y[field.index(x,y,z)]
                +field.z[field.index(x,y+1,z)]
                -field.y[field.index(x,y,z+1)]
                -field.z[field.index(x,y,z)]),
            std::abs(field.z[field.index(x,y,z)]
                +field.x[field.index(x,y,z+1)]
                -field.z[field.index(x+1,y,z)]
                -field.x[field.index(x,y,z)])});
      }
  result.compact_support = result.outside_maximum == 0.0;
  result.zero_boundary_crossing = result.boundary_crossing_maximum == 0.0;
  result.valid = result.neutral && result.density_contained
      && result.compact_support && result.zero_boundary_crossing
      && result.poisson_residual <= poisson_tolerance
      && result.gauss_residual <= 1e-11
      && result.internal_circulation_residual <= 1e-11;
  return result;
}

double connected_moore_block_binding_energy(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options) {
  return binding_energy(positions(state), state.edges, options);
}

ConnectedMooreBlockStepResult solve_connected_moore_block_forward(
    const ConnectedMooreBlockState& earlier,
    const ConnectedMooreBlockOptions& options,
    ConnectedMooreBlockSolveCache* cache) {
  const auto normalization = measure_face_flux_normalization();
  if (options.defer_volume_diagnostics || !normalization.valid
      || !(normalization.mapped_field_work_coefficient > 0.0)
      || !state_valid(earlier, options))
    return ConnectedMooreBlockStepResult(0);
  const double lambda = options.wave_speed*options.dt;
  const auto prepared = prepare_forward_fields(earlier,lambda);
  if (options.use_local_residual_evaluation
      && (!finite_field(prepared.magnetic_later)
          || !finite_field(prepared.electric_pre_current)))
    return ConnectedMooreBlockStepResult(0);
  const auto evaluate = [&](const std::vector<double>& unknown) {
    const double interaction_scale =
        normalization.mapped_field_work_coefficient
        *options.field_energy_scale;
    return options.use_local_residual_evaluation
        ? evaluate_forward_local_residual(
            earlier, options, prepared, interaction_scale, unknown)
        : evaluate_forward(
            earlier, options, prepared, interaction_scale, unknown);
  };
  const auto initial = options.root_momentum_seed.empty()
      ? flatten_momenta(earlier.constituents)
      : flatten_momenta(options.root_momentum_seed);
  auto root = solve_root(earlier.electric.L,
      earlier.constituents.size(),initial,
      options,evaluate,cache);
  const auto materialize_local_root = [&](RootResult& local_root) {
    if (!options.use_local_residual_evaluation
        || !local_root.diagnostics.converged
        || !local_root.candidate.valid) return;
    const int cache_fallbacks = local_root.diagnostics.cache_fallbacks;
    const int discarded_evaluations =
        local_root.diagnostics.discarded_cache_residual_evaluations;
    const auto local_residual = local_root.candidate.residual;
    const auto accepted = flatten_momenta(
        local_root.candidate.later.constituents);
    local_root.candidate = evaluate_forward(earlier, options, prepared,
        normalization.mapped_field_work_coefficient
            *options.field_energy_scale, accepted);
    ++local_root.diagnostics.full_candidate_materializations;
    local_root.diagnostics.cache_fallbacks = cache_fallbacks;
    local_root.diagnostics.discarded_cache_residual_evaluations =
        discarded_evaluations;
    if (!local_root.candidate.valid
        || local_root.candidate.residual.size() != local_residual.size()) {
      local_root.diagnostics.converged = false;
      local_root.diagnostics.materialized_residual_difference = INFINITY;
    } else {
      double difference = 0.0;
      for (std::size_t i = 0; i < local_residual.size(); ++i)
        difference = std::max(difference,
            std::abs(local_root.candidate.residual[i]-local_residual[i]));
      local_root.diagnostics.materialized_residual_difference = difference;
      local_root.diagnostics.residual = infinity_norm(
          local_root.candidate.residual);
      local_root.diagnostics.converged =
          local_root.diagnostics.residual <= options.solve_tolerance;
    }
  };
  materialize_local_root(root);
  if (cache != nullptr && !root.diagnostics.converged) {
    const int discarded_evaluations = root.diagnostics.residual_evaluations;
    cache->reset();
    root = solve_root(earlier.electric.L, earlier.constituents.size(), initial,
                      options, evaluate, nullptr);
    root.diagnostics.cache_fallbacks = 1;
    root.diagnostics.discarded_cache_residual_evaluations =
        discarded_evaluations;
    materialize_local_root(root);
  }
  return finalize(root,true,options,normalization);
}

ConnectedMooreBlockStepResult solve_connected_moore_block_forward_prepared(
    const ConnectedMooreBlockState& earlier,
    MatchedEdgeField&& magnetic_later,
    MatchedFaceFlux&& electric_pre_current,
    const ConnectedMooreBlockOptions& options,
    ConnectedMooreBlockSolveCache* cache) {
  const auto normalization = measure_face_flux_normalization();
  const std::size_t expected = volume(earlier.electric.L);
  const bool prepared_shapes = magnetic_later.L == earlier.electric.L
      && electric_pre_current.L == earlier.electric.L
      && magnetic_later.x.size() == expected
      && magnetic_later.y.size() == expected
      && magnetic_later.z.size() == expected
      && electric_pre_current.x.size() == expected
      && electric_pre_current.y.size() == expected
      && electric_pre_current.z.size() == expected;
  if (!options.use_local_residual_evaluation
      || !options.use_sparse_local_current
      || !options.defer_volume_diagnostics
      || !normalization.valid
      || !(normalization.mapped_field_work_coefficient > 0.0)
      || !prepared_shapes || !state_valid(earlier, options, true))
    return ConnectedMooreBlockStepResult(0);
  PreparedForwardFields prepared(earlier.electric.L);
  prepared.magnetic_later = std::move(magnetic_later);
  prepared.electric_pre_current = std::move(electric_pre_current);
  const double interaction_scale =
      normalization.mapped_field_work_coefficient
      *options.field_energy_scale;
  const auto evaluate = [&](const std::vector<double>& unknown) {
    return evaluate_forward_local_residual(
        earlier, options, prepared, interaction_scale, unknown);
  };
  const auto initial = options.root_momentum_seed.empty()
      ? flatten_momenta(earlier.constituents)
      : flatten_momenta(options.root_momentum_seed);
  auto root = solve_root(earlier.electric.L,
      earlier.constituents.size(), initial, options, evaluate, cache);
  materialize_forward_local_candidate(root, earlier, prepared, options);
  if (cache != nullptr && !root.diagnostics.converged) {
    const int discarded_evaluations = root.diagnostics.residual_evaluations;
    cache->reset();
    root = solve_root(earlier.electric.L, earlier.constituents.size(), initial,
                      options, evaluate, nullptr);
    root.diagnostics.cache_fallbacks = 1;
    root.diagnostics.discarded_cache_residual_evaluations =
        discarded_evaluations;
    materialize_forward_local_candidate(root, earlier, prepared, options);
  }
  return finalize(root, true, options, normalization);
}

ConnectedMooreBlockStepResult solve_connected_moore_block_forward_resident(
    const ConnectedMooreBlockState& earlier,
    const ConnectedMooreBlockOptions& options,
    ConnectedMooreBlockSolveCache* cache) {
  const auto normalization=measure_face_flux_normalization();
  const bool field_markers=earlier.electric.L>=5
      &&earlier.magnetic_half.L==earlier.electric.L
      &&earlier.electric.x.empty()&&earlier.electric.y.empty()
      &&earlier.electric.z.empty()&&earlier.magnetic_half.x.empty()
      &&earlier.magnetic_half.y.empty()&&earlier.magnetic_half.z.empty();
  if(!options.use_local_residual_evaluation
      ||!options.use_sparse_local_current
      ||!options.defer_volume_diagnostics
      ||!options.resident_local_orbit_gather
      ||!normalization.valid
      ||!(normalization.mapped_field_work_coefficient>0.0)
      ||!field_markers||!matter_metadata_valid(earlier,options))
    return ConnectedMooreBlockStepResult(0);
  const double interaction_scale=
      normalization.mapped_field_work_coefficient*options.field_energy_scale;
  const auto evaluate=[&](const std::vector<double>& unknown) {
    return evaluate_forward_resident_local_residual(
        earlier,options,interaction_scale,unknown);
  };
  const auto initial=options.root_momentum_seed.empty()
      ?flatten_momenta(earlier.constituents)
      :flatten_momenta(options.root_momentum_seed);
  auto root=solve_root(earlier.electric.L,earlier.constituents.size(),
      initial,options,evaluate,cache);
  if(cache!=nullptr&&!root.diagnostics.converged) {
    const int discarded_evaluations=root.diagnostics.residual_evaluations;
    cache->reset();
    root=solve_root(earlier.electric.L,earlier.constituents.size(),
        initial,options,evaluate,nullptr);
    root.diagnostics.cache_fallbacks=1;
    root.diagnostics.discarded_cache_residual_evaluations=
        discarded_evaluations;
  }
  return finalize(root,true,options,normalization);
}

ConnectedMooreBlockStepResult complete_connected_moore_block_volume_diagnostics(
    ConnectedMooreBlockStepResult step,
    const ConnectedMooreBlockVolumeDiagnostics& diagnostics,
    const ConnectedMooreBlockOptions& options) {
  if (!step.volume_diagnostics_pending || !step.solve.converged
      || !diagnostics.valid) return step;
  step.gauss_before_residual = diagnostics.gauss_before_residual;
  step.gauss_after_residual = diagnostics.gauss_after_residual;
  step.field_energy_before = diagnostics.field_energy_before;
  step.field_energy_after = diagnostics.field_energy_after;
  step.local_field_momentum_before =
      diagnostics.local_field_momentum_before;
  step.local_field_momentum_after =
      diagnostics.local_field_momentum_after;
  step.spline_field_momentum_before =
      diagnostics.spline_field_momentum_before;
  step.spline_field_momentum_after =
      diagnostics.spline_field_momentum_after;
  const double matter0 = step.kinetic_energy_before
      +step.binding_energy_before;
  const double matter1 = step.kinetic_energy_after
      +step.binding_energy_after;
  step.field_work_residual = std::abs(
      step.field_energy_after-step.field_energy_before+step.current_work);
  step.total_energy_residual = std::abs(
      matter1+step.field_energy_after-matter0-step.field_energy_before);
  const Vec3 matter_delta = step.matter_momentum_after
      -step.matter_momentum_before;
  step.local_total_defect = matter_delta+step.local_field_momentum_after
      -step.local_field_momentum_before;
  step.spline_total_defect = matter_delta+step.spline_field_momentum_after
      -step.spline_field_momentum_before;
  step.local_defect_norm = step.local_total_defect.mag();
  step.spline_defect_norm = step.spline_total_defect.mag();
  if (step.field_energy_before > 0.0)
    step.normalized_spline_defect = options.wave_speed
        *step.spline_defect_norm/step.field_energy_before;
  step.volume_diagnostics_pending = false;
  const bool finite_values = std::isfinite(step.root_residual)
      && std::isfinite(step.continuity_residual)
      && std::isfinite(step.gauss_before_residual)
      && std::isfinite(step.gauss_after_residual)
      && std::isfinite(step.total_energy_residual)
      && std::isfinite(step.normalized_spline_defect)
      && finite(step.local_field_momentum_before)
      && finite(step.local_field_momentum_after)
      && finite(step.spline_field_momentum_before)
      && finite(step.spline_field_momentum_after);
  const bool derived_pair =
      options.binding_law == ConnectedBindingLaw::DerivedCompactPair;
  step.valid = diagnostics.valid && step.solve.converged && finite_values
      && (options.allow_shared_anchor_chart || step.site_projection_valid)
      && step.graph_local && step.net_charge == 0
      && (derived_pair || step.graph_connected);
  const double gate = options.gate_tolerance;
  step.common_action_gates_pass = step.valid
      && step.root_residual <= gate
      && step.continuity_residual <= gate
      && step.gauss_before_residual <= gate
      && step.gauss_after_residual <= gate
      && step.force_residual <= gate
      && step.kinematic_residual <= gate
      && step.kinetic_discrete_gradient_residual <= gate
      && step.electric_adjoint_residual <= gate
      && step.magnetic_work_residual <= gate
      && step.binding_work_residual <= gate
      && step.binding_impulse_sum_residual <= gate
      && step.matter_work_residual <= gate
      && step.field_work_residual <= gate
      && step.total_energy_residual <= gate
      && step.causal_speed_excess <= 1e-12;
  return step;
}

ConnectedMooreBlockStepResult solve_connected_moore_block_reverse(
    const ConnectedMooreBlockState& later,
    const ConnectedMooreBlockOptions& options,
    ConnectedMooreBlockSolveCache* cache) {
  const auto normalization = measure_face_flux_normalization();
  if (!normalization.valid
      || !(normalization.mapped_field_work_coefficient > 0.0)
      || !state_valid(later, options))
    return ConnectedMooreBlockStepResult(0);
  const double lambda = options.wave_speed*options.dt;
  const auto prepared = prepare_reverse_fields(later, lambda);
  if (options.use_local_residual_evaluation
      && !finite_field(prepared.electric_pre_current))
    return ConnectedMooreBlockStepResult(0);
  const auto evaluate = [&](const std::vector<double>& unknown) {
    const double interaction_scale =
        normalization.mapped_field_work_coefficient
        *options.field_energy_scale;
    return options.use_local_residual_evaluation
        ? evaluate_reverse_local_residual(
            later, options, prepared, interaction_scale, unknown)
        : evaluate_reverse(later, options, interaction_scale, unknown);
  };
  const auto initial = options.root_momentum_seed.empty()
      ? flatten_momenta(later.constituents)
      : flatten_momenta(options.root_momentum_seed);
  auto root = solve_root(later.electric.L,
      later.constituents.size(),initial,
      options,evaluate,cache);
  const auto materialize_local_root = [&](RootResult& local_root) {
    if (!options.use_local_residual_evaluation
        || !local_root.diagnostics.converged
        || !local_root.candidate.valid) return;
    const int cache_fallbacks = local_root.diagnostics.cache_fallbacks;
    const int discarded_evaluations =
        local_root.diagnostics.discarded_cache_residual_evaluations;
    const auto local_residual = local_root.candidate.residual;
    const auto accepted = flatten_momenta(
        local_root.candidate.earlier.constituents);
    local_root.candidate = evaluate_reverse(later, options,
        normalization.mapped_field_work_coefficient
            *options.field_energy_scale, accepted);
    ++local_root.diagnostics.full_candidate_materializations;
    local_root.diagnostics.cache_fallbacks = cache_fallbacks;
    local_root.diagnostics.discarded_cache_residual_evaluations =
        discarded_evaluations;
    if (!local_root.candidate.valid
        || local_root.candidate.residual.size() != local_residual.size()) {
      local_root.diagnostics.converged = false;
      local_root.diagnostics.materialized_residual_difference = INFINITY;
    } else {
      double difference = 0.0;
      for (std::size_t i = 0; i < local_residual.size(); ++i)
        difference = std::max(difference,
            std::abs(local_root.candidate.residual[i]-local_residual[i]));
      local_root.diagnostics.materialized_residual_difference = difference;
      local_root.diagnostics.residual = infinity_norm(
          local_root.candidate.residual);
      local_root.diagnostics.converged =
          local_root.diagnostics.residual <= options.solve_tolerance;
    }
  };
  materialize_local_root(root);
  if (cache != nullptr && !root.diagnostics.converged) {
    const int discarded_evaluations = root.diagnostics.residual_evaluations;
    cache->reset();
    root = solve_root(later.electric.L, later.constituents.size(), initial,
                      options, evaluate, nullptr);
    root.diagnostics.cache_fallbacks = 1;
    root.diagnostics.discarded_cache_residual_evaluations =
        discarded_evaluations;
    materialize_local_root(root);
  }
  return finalize(root,false,options,normalization);
}

double connected_moore_block_state_max_difference(
    const ConnectedMooreBlockState& lhs,
    const ConnectedMooreBlockState& rhs) {
  if (lhs.electric.L <= 0 || lhs.electric.L != rhs.electric.L
      || lhs.constituents.size() != rhs.constituents.size()
      || lhs.charges != rhs.charges || lhs.edges.size() != rhs.edges.size())
    return INFINITY;
  double result = std::max(
      matched_face_max_difference(lhs.electric,rhs.electric),
      matched_edge_max_difference(lhs.magnetic_half,rhs.magnetic_half));
  const int L = lhs.electric.L;
  for (std::size_t a = 0; a < lhs.constituents.size(); ++a) {
    const auto& p = lhs.constituents[a];
    const auto& q = rhs.constituents[a];
    const Vec3 position_delta{
        (p.anchor.x-q.anchor.x)-std::round(
            static_cast<double>(p.anchor.x-q.anchor.x)/L)*L
            +p.remainder.x-q.remainder.x,
        (p.anchor.y-q.anchor.y)-std::round(
            static_cast<double>(p.anchor.y-q.anchor.y)/L)*L
            +p.remainder.y-q.remainder.y,
        (p.anchor.z-q.anchor.z)-std::round(
            static_cast<double>(p.anchor.z-q.anchor.z)/L)*L
            +p.remainder.z-q.remainder.z};
    result = std::max({result,maximum_component(position_delta),
        maximum_component(p.momentum-q.momentum)});
  }
  for (std::size_t e = 0; e < lhs.edges.size(); ++e) {
    const auto& a = lhs.edges[e];
    const auto& b = rhs.edges[e];
    if (a.first != b.first || a.second != b.second
        || a.reference_delta.x != b.reference_delta.x
        || a.reference_delta.y != b.reference_delta.y
        || a.reference_delta.z != b.reference_delta.z)
      return INFINITY;
    result = std::max(result,
        std::abs(a.rest_length_squared-b.rest_length_squared));
  }
  return result;
}

}  // namespace ftd::eft
