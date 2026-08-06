#include "ftd/eft/state_only_matter_field_observer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <numeric>
#include <utility>

namespace ftd::eft {
namespace {

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double sample_energy(const Vec3& electric, const Vec3& magnetic) {
  return 0.5*(electric.mag2()+magnetic.mag2());
}

int shortest_delta(int coordinate, int center, int L) {
  int delta = coordinate-center;
  if (delta > L/2) delta -= L;
  if (delta < -L/2) delta += L;
  return delta;
}

double shortest_delta(int coordinate, double center, int L) {
  double delta = static_cast<double>(coordinate)-center;
  const double half = 0.5*static_cast<double>(L);
  if (delta > half) delta -= static_cast<double>(L);
  if (delta < -half) delta += static_cast<double>(L);
  return delta;
}

Vec3 effective_position(const MatchedMatterPoint& point) {
  return {point.anchor.x+point.remainder.x,
          point.anchor.y+point.remainder.y,
          point.anchor.z+point.remainder.z};
}

double face_component(const MatchedFaceFlux& field, int component,
                      int x, int y, int z) {
  const auto i = static_cast<std::size_t>(field.index(x,y,z));
  return component == 0 ? field.x[i]
      : (component == 1 ? field.y[i] : field.z[i]);
}

double edge_component(const MatchedEdgeField& field, int component,
                      int x, int y, int z) {
  const auto i = static_cast<std::size_t>(field.index(x,y,z));
  return component == 0 ? field.x[i]
      : (component == 1 ? field.y[i] : field.z[i]);
}

double curl_adjoint_component(const MatchedFaceFlux& field,
                              int component, int x, int y, int z) {
  const auto f = [&](int c, int xx, int yy, int zz) {
    return face_component(field,c,xx,yy,zz);
  };
  if (component == 0)
    return f(2,x,y+1,z)-f(2,x,y,z)-f(1,x,y,z+1)+f(1,x,y,z);
  if (component == 1)
    return f(0,x,y,z+1)-f(0,x,y,z)-f(2,x+1,y,z)+f(2,x,y,z);
  return f(1,x+1,y,z)-f(1,x,y,z)-f(0,x,y+1,z)+f(0,x,y,z);
}

double integer_edge_component(const ConnectedMooreBlockState& state,
                              int component, int x, int y, int z,
                              double half_step_scale) {
  return edge_component(state.magnetic_half,component,x,y,z)
      +half_step_scale*curl_adjoint_component(
          state.electric,component,x,y,z);
}

Vec3 centered_face(const MatchedFaceFlux& field, int x, int y, int z) {
  return {
      0.5*(face_component(field,0,x,y,z)+face_component(field,0,x-1,y,z)),
      0.5*(face_component(field,1,x,y,z)+face_component(field,1,x,y-1,z)),
      0.5*(face_component(field,2,x,y,z)+face_component(field,2,x,y,z-1))};
}

Vec3 centered_integer_edge(const ConnectedMooreBlockState& state,
                           int x, int y, int z,
                           double half_step_scale) {
  const auto b = [&](int c, int xx, int yy, int zz) {
    return integer_edge_component(state,c,xx,yy,zz,half_step_scale);
  };
  return {
      0.25*(b(0,x,y,z)+b(0,x,y-1,z)+b(0,x,y,z-1)+b(0,x,y-1,z-1)),
      0.25*(b(1,x,y,z)+b(1,x-1,y,z)+b(1,x,y,z-1)+b(1,x-1,y,z-1)),
      0.25*(b(2,x,y,z)+b(2,x-1,y,z)+b(2,x,y-1,z)+b(2,x-1,y-1,z))};
}

double relative_scale(double a, double b, double c = 0.0) {
  return std::max({1.0,std::abs(a),std::abs(b),std::abs(c)});
}

}  // namespace

CenteredCharacteristicSample decompose_centered_characteristic_sample(
    const Vec3& residual_electric,
    const Vec3& residual_magnetic,
    const Vec3& radial_unit,
    double tolerance) {
  CenteredCharacteristicSample result;
  result.residual_electric = residual_electric;
  result.residual_magnetic = residual_magnetic;
  if (!finite(residual_electric) || !finite(residual_magnetic)
      || !finite(radial_unit) || !(tolerance > 0.0)) return result;

  const double radial_norm = radial_unit.mag();
  if (radial_norm <= tolerance) {
    result.background_electric = residual_electric;
    result.background_magnetic = residual_magnetic;
    result.residual_energy = sample_energy(residual_electric,residual_magnetic);
    result.radial_energy = result.residual_energy;
    result.background_energy = result.residual_energy;
    result.reconstruction_residual = 0.0;
    result.energy_partition_residual = 0.0;
    result.characteristic_flux_residual = 0.0;
    result.valid = true;
    return result;
  }
  const Vec3 n = radial_unit*(1.0/radial_norm);
  const Vec3 electric_radial = n*residual_electric.dot(n);
  const Vec3 magnetic_radial = n*residual_magnetic.dot(n);
  const Vec3 electric_tangent = residual_electric-electric_radial;
  const Vec3 magnetic_tangent = residual_magnetic-magnetic_radial;
  const Vec3 n_cross_b = Vec3::cross(n,magnetic_tangent);
  const Vec3 outgoing_electric = (electric_tangent-n_cross_b)*0.5;
  const Vec3 incoming_electric = (electric_tangent+n_cross_b)*0.5;
  const Vec3 outgoing_magnetic = Vec3::cross(n,outgoing_electric);
  const Vec3 incoming_magnetic = Vec3::cross(n,incoming_electric)*(-1.0);

  result.outgoing_electric = outgoing_electric;
  result.outgoing_magnetic = outgoing_magnetic;
  result.background_electric = incoming_electric+electric_radial;
  result.background_magnetic = incoming_magnetic+magnetic_radial;
  result.residual_energy = sample_energy(residual_electric,residual_magnetic);
  result.outgoing_energy = sample_energy(outgoing_electric,outgoing_magnetic);
  result.incoming_energy = sample_energy(incoming_electric,incoming_magnetic);
  result.radial_energy = sample_energy(electric_radial,magnetic_radial);
  result.background_energy = result.incoming_energy+result.radial_energy;
  result.signed_radial_poynting =
      Vec3::cross(residual_electric,residual_magnetic).dot(n);
  result.reconstruction_residual = std::max(
      maximum_component(residual_electric-result.outgoing_electric
          -result.background_electric),
      maximum_component(residual_magnetic-result.outgoing_magnetic
          -result.background_magnetic));
  result.energy_partition_residual = result.residual_energy
      -result.outgoing_energy-result.background_energy;
  result.characteristic_flux_residual = result.signed_radial_poynting
      -(result.outgoing_energy-result.incoming_energy);
  const double scale = relative_scale(result.residual_energy,
      result.outgoing_energy,result.background_energy);
  result.valid = result.reconstruction_residual <= tolerance*scale
      && std::abs(result.energy_partition_residual) <= tolerance*scale
      && std::abs(result.characteristic_flux_residual) <= tolerance*scale;
  return result;
}

StateOnlyMatterFieldObservation observe_state_only_matter_field(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const StateOnlyMatterFieldObserverOptions& observer_options) {
  StateOnlyMatterFieldObservation result;
  result.L = state.electric.L;
  result.support_half_width = observer_options.support_half_width;
  const std::size_t expected = result.L > 0
      ? static_cast<std::size_t>(result.L)*result.L*result.L : 0;
  const bool shapes = result.L > 0 && state.magnetic_half.L == result.L
      && state.electric.x.size() == expected
      && state.electric.y.size() == expected
      && state.electric.z.size() == expected
      && state.magnetic_half.x.size() == expected
      && state.magnetic_half.y.size() == expected
      && state.magnetic_half.z.size() == expected;
  if (!shapes || result.L%2 == 0 || state.constituents.size() != 2
      || state.charges.size() != 2 || !state.edges.empty()
      || action_options.binding_law != ConnectedBindingLaw::DerivedCompactPair
      || !(observer_options.wave_speed > 0.0)
      || !std::isfinite(observer_options.dt)
      || !(observer_options.gate_tolerance > 0.0)) return result;

  // The preparation consumes only the lattice-size tags and matter metadata;
  // avoid duplicating six zero-filled L^3 arrays merely to describe geometry.
  ConnectedMooreBlockState geometry;
  geometry.electric.L = result.L;
  geometry.magnetic_half.L = result.L;
  geometry.constituents = state.constituents;
  geometry.charges = state.charges;
  geometry.edges = state.edges;
  geometry.width = state.width;
  geometry.orientation_axis = state.orientation_axis;
  const auto preparation = prepare_finite_support_derived_compact_pair(
      geometry,action_options,observer_options.support_half_width,
      observer_options.poisson_tolerance,
      observer_options.poisson_max_iterations,
      observer_options.allow_fractional_center);
  result.bound_poisson_residual = preparation.poisson_residual;
  result.bound_gauss_residual = preparation.gauss_residual;
  result.bound_outside_maximum = preparation.outside_maximum;
  result.bound_boundary_crossing_maximum =
      preparation.boundary_crossing_maximum;
  if (!preparation.valid || !preparation.compact_support
      || !preparation.zero_boundary_crossing) return result;
  result.center = preparation.center;
  result.support_center = preparation.support_center;
  result.fractional_center_offset = preparation.fractional_center_offset;
  result.fractional_center_enabled =
      preparation.fractional_center_enabled;
  result.net_charge = std::accumulate(
      state.charges.begin(),state.charges.end(),0);
  const double rest = action_options.constituent_mass_scale*E_REST;
  for (const auto& point : state.constituents)
    result.constituent_kinetic_energy += std::sqrt(
        rest*rest+C_SPEED*C_SPEED*point.momentum.mag2())-rest;
  result.pair_internal_energy = connected_moore_block_binding_energy(
      state,action_options);

  result.shells.reserve(observer_options.shell_radii.size());
  for (int radius : observer_options.shell_radii) {
    if (radius <= 0 || radius > result.L/2) return result;
    StateOnlyCharacteristicShell shell;
    shell.radius = radius;
    result.shells.push_back(shell);
  }

  const int cx = static_cast<int>(std::llround(result.support_center.x));
  const int cy = static_cast<int>(std::llround(result.support_center.y));
  const int cz = static_cast<int>(std::llround(result.support_center.z));
  const double half_step_scale =
      -0.5*observer_options.wave_speed*observer_options.dt;
  long double actual_energy = 0.0L;
  long double bound_energy = 0.0L;
  long double residual_energy = 0.0L;
  long double outgoing_energy = 0.0L;
  long double incoming_energy = 0.0L;
  long double radial_energy = 0.0L;
  long double background_energy = 0.0L;
  long double signed_flux = 0.0L;
  bool samples_valid = true;
  const std::size_t shell_count=result.shells.size();
  std::vector<long double> actual_by_x(result.L),bound_by_x(result.L);
  std::vector<long double> residual_by_x(result.L),outgoing_by_x(result.L);
  std::vector<long double> incoming_by_x(result.L),radial_by_x(result.L);
  std::vector<long double> background_by_x(result.L),flux_by_x(result.L);
  std::vector<long double> primitive_cross_by_x(result.L);
  std::vector<long double> centered_electric_cross_by_x(result.L);
  std::vector<long double> centered_magnetic_cross_by_x(result.L);
  std::vector<double> reconstruction_by_x(result.L);
  std::vector<double> gauss_by_x(result.L),characteristic_by_x(result.L);
  std::vector<unsigned char> valid_by_x(result.L,1);
  std::vector<StateOnlyCharacteristicShell> shells_by_x(
      static_cast<std::size_t>(result.L)*shell_count);
  for(int x=0;x<result.L;++x) for(std::size_t i=0;i<shell_count;++i)
    shells_by_x[static_cast<std::size_t>(x)*shell_count+i].radius=
        result.shells[i].radius;

#pragma omp parallel for schedule(static)
  for (int x = 0; x < result.L; ++x) {
    long double local_actual=0.0L,local_bound=0.0L,local_residual=0.0L;
    long double local_outgoing=0.0L,local_incoming=0.0L,local_radial=0.0L;
    long double local_background=0.0L,local_flux=0.0L;
    long double local_primitive_cross=0.0L;
    long double local_centered_electric_cross=0.0L;
    long double local_centered_magnetic_cross=0.0L;
    double local_reconstruction=0.0,local_gauss=0.0,local_characteristic=0.0;
    bool local_valid=true;
    auto* local_shells=shell_count==0?nullptr:
        &shells_by_x[static_cast<std::size_t>(x)*shell_count];
    for (int y = 0; y < result.L; ++y)
      for (int z = 0; z < result.L; ++z) {
        const int dx = shortest_delta(x,cx,result.L);
        const int dy = shortest_delta(y,cy,result.L);
        const int dz = shortest_delta(z,cz,result.L);
        const Vec3 radial{
            shortest_delta(x,result.center.x,result.L),
            shortest_delta(y,result.center.y,result.L),
            shortest_delta(z,result.center.z,result.L)};
        local_gauss = std::max(local_gauss,
            std::abs(divergence_at(state.electric,x,y,z)
                -divergence_at(preparation.state.electric,x,y,z)));
        const Vec3 actual_e = centered_face(state.electric,x,y,z);
        const Vec3 bound_e = centered_face(
            preparation.state.electric,x,y,z);
        const Vec3 residual_e = actual_e-bound_e;
        const Vec3 actual_b = centered_integer_edge(
            state,x,y,z,half_step_scale);
        const Vec3 bound_b = centered_integer_edge(
            preparation.state,x,y,z,half_step_scale);
        const Vec3 residual_b = actual_b-bound_b;
        for (int component = 0; component < 3; ++component) {
          const double actual_component = face_component(
              state.electric,component,x,y,z);
          const double bound_component = face_component(
              preparation.state.electric,component,x,y,z);
          local_primitive_cross += static_cast<long double>(bound_component)
              *(actual_component-bound_component);
        }
        local_centered_electric_cross +=
            static_cast<long double>(bound_e.dot(residual_e));
        local_centered_magnetic_cross +=
            static_cast<long double>(bound_b.dot(residual_b));
        const auto sample = decompose_centered_characteristic_sample(
            residual_e,residual_b,radial,observer_options.gate_tolerance);
        local_valid = local_valid && sample.valid;
        local_actual += sample_energy(actual_e,actual_b);
        local_bound += sample_energy(bound_e,bound_b);
        local_residual += sample.residual_energy;
        local_outgoing += sample.outgoing_energy;
        local_incoming += sample.incoming_energy;
        local_radial += sample.radial_energy;
        local_background += sample.background_energy;
        local_flux += sample.signed_radial_poynting;
        local_reconstruction = std::max(local_reconstruction,
            std::max(sample.reconstruction_residual,
              std::max(maximum_component(actual_e-bound_e
                    -sample.outgoing_electric-sample.background_electric),
                  maximum_component(actual_b-bound_b
                    -sample.outgoing_magnetic-sample.background_magnetic))));
        local_characteristic = std::max(local_characteristic,
            std::abs(sample.characteristic_flux_residual));
        const int chebyshev = std::max({std::abs(dx),std::abs(dy),
                                        std::abs(dz)});
        for(std::size_t i=0;i<shell_count;++i) {
          auto& shell=local_shells[i];
          if(chebyshev != shell.radius) continue;
          ++shell.samples;
          shell.residual_energy += sample.residual_energy;
          shell.outgoing_energy += sample.outgoing_energy;
          shell.incoming_energy += sample.incoming_energy;
          shell.radial_energy += sample.radial_energy;
          shell.background_energy += sample.background_energy;
          shell.signed_radial_poynting += sample.signed_radial_poynting;
          shell.outward_characteristic_power += sample.outgoing_energy;
          shell.inward_characteristic_power += sample.incoming_energy;
        }
      }
    actual_by_x[x]=local_actual; bound_by_x[x]=local_bound;
    residual_by_x[x]=local_residual; outgoing_by_x[x]=local_outgoing;
    incoming_by_x[x]=local_incoming; radial_by_x[x]=local_radial;
    background_by_x[x]=local_background; flux_by_x[x]=local_flux;
    primitive_cross_by_x[x]=local_primitive_cross;
    centered_electric_cross_by_x[x]=local_centered_electric_cross;
    centered_magnetic_cross_by_x[x]=local_centered_magnetic_cross;
    reconstruction_by_x[x]=local_reconstruction;
    gauss_by_x[x]=local_gauss;
    characteristic_by_x[x]=local_characteristic;
    valid_by_x[x]=local_valid?1:0;
  }

  for(int x=0;x<result.L;++x) {
    actual_energy+=actual_by_x[x]; bound_energy+=bound_by_x[x];
    residual_energy+=residual_by_x[x]; outgoing_energy+=outgoing_by_x[x];
    incoming_energy+=incoming_by_x[x]; radial_energy+=radial_by_x[x];
    background_energy+=background_by_x[x]; signed_flux+=flux_by_x[x];
    result.primitive_face_interference +=
        static_cast<double>(primitive_cross_by_x[x]);
    result.centered_electric_interference +=
        static_cast<double>(centered_electric_cross_by_x[x]);
    result.centered_magnetic_interference +=
        static_cast<double>(centered_magnetic_cross_by_x[x]);
    samples_valid=samples_valid&&valid_by_x[x]!=0;
    result.maximum_reconstruction_residual=std::max(
        result.maximum_reconstruction_residual,reconstruction_by_x[x]);
    result.actual_gauss_compatibility_residual=std::max(
        result.actual_gauss_compatibility_residual,gauss_by_x[x]);
    result.characteristic_flux_residual=std::max(
        result.characteristic_flux_residual,characteristic_by_x[x]);
    for(std::size_t i=0;i<shell_count;++i) {
      const auto& local=shells_by_x[static_cast<std::size_t>(x)*shell_count+i];
      auto& shell=result.shells[i];
      shell.samples+=local.samples;
      shell.residual_energy+=local.residual_energy;
      shell.outgoing_energy+=local.outgoing_energy;
      shell.incoming_energy+=local.incoming_energy;
      shell.radial_energy+=local.radial_energy;
      shell.background_energy+=local.background_energy;
      shell.signed_radial_poynting+=local.signed_radial_poynting;
      shell.outward_characteristic_power+=local.outward_characteristic_power;
      shell.inward_characteristic_power+=local.inward_characteristic_power;
    }
  }

  result.bound_energy = static_cast<double>(bound_energy);
  result.residual_energy = static_cast<double>(residual_energy);
  result.outgoing_energy = static_cast<double>(outgoing_energy);
  result.incoming_energy = static_cast<double>(incoming_energy);
  result.radial_energy = static_cast<double>(radial_energy);
  result.background_energy = static_cast<double>(background_energy);
  result.signed_radial_poynting = static_cast<double>(signed_flux);
  result.outward_characteristic_power = result.outgoing_energy;
  result.inward_characteristic_power = result.incoming_energy;
  result.bound_residual_interference = static_cast<double>(
      actual_energy-bound_energy-residual_energy);
  result.centering_metric_interference =
      result.centered_electric_interference
      -result.primitive_face_interference;

  // Reconstruct the selected support potential from the primitive internal
  // face gradient.  Its additive constant is immaterial because the net
  // residual flux through the closed support boundary vanishes when the
  // actual and selected fields have the same Gauss source.
  const int support = observer_options.support_half_width;
  const int side = 2*support+1;
  const auto local_index = [=](int dx, int dy, int dz) {
    return static_cast<std::size_t>(dx+support)*side*side
        +static_cast<std::size_t>(dy+support)*side
        +static_cast<std::size_t>(dz+support);
  };
  std::vector<double> support_potential(
      static_cast<std::size_t>(side)*side*side,0.0);
  for (int dx=-support; dx<support; ++dx)
    support_potential[local_index(dx+1,-support,-support)] =
        support_potential[local_index(dx,-support,-support)]
        -face_component(preparation.state.electric,0,
            cx+dx,cy-support,cz-support);
  for (int dx=-support; dx<=support; ++dx)
    for (int dy=-support; dy<support; ++dy)
      support_potential[local_index(dx,dy+1,-support)] =
          support_potential[local_index(dx,dy,-support)]
          -face_component(preparation.state.electric,1,
              cx+dx,cy+dy,cz-support);
  for (int dx=-support; dx<=support; ++dx)
    for (int dy=-support; dy<=support; ++dy)
      for (int dz=-support; dz<support; ++dz)
        support_potential[local_index(dx,dy,dz+1)] =
            support_potential[local_index(dx,dy,dz)]
            -face_component(preparation.state.electric,2,
                cx+dx,cy+dy,cz+dz);

  const auto residual_face = [&](int component,int x,int y,int z) {
    return face_component(state.electric,component,x,y,z)
        -face_component(preparation.state.electric,component,x,y,z);
  };
  long double induced_boundary=0.0L;
  long double boundary_flux=0.0L;
  for (int dx=-support; dx<=support; ++dx)
    for (int dy=-support; dy<=support; ++dy)
      for (int dz=-support; dz<=support; ++dz) {
        double crossing_divergence=0.0;
        if (dx==support)
          crossing_divergence+=residual_face(0,cx+dx,cy+dy,cz+dz);
        if (dx==-support)
          crossing_divergence-=residual_face(0,cx+dx-1,cy+dy,cz+dz);
        if (dy==support)
          crossing_divergence+=residual_face(1,cx+dx,cy+dy,cz+dz);
        if (dy==-support)
          crossing_divergence-=residual_face(1,cx+dx,cy+dy-1,cz+dz);
        if (dz==support)
          crossing_divergence+=residual_face(2,cx+dx,cy+dy,cz+dz);
        if (dz==-support)
          crossing_divergence-=residual_face(2,cx+dx,cy+dy,cz+dz-1);
        const double potential=support_potential[local_index(dx,dy,dz)];
        induced_boundary -= static_cast<long double>(potential)
            *crossing_divergence;
        boundary_flux += crossing_divergence;
      }
  result.induced_boundary_interference=
      static_cast<double>(induced_boundary);
  result.boundary_flux_sum=static_cast<double>(boundary_flux);
  result.primitive_boundary_identity_residual =
      result.primitive_face_interference
      -result.induced_boundary_interference;
  result.readout_interference_reconstruction_residual =
      result.bound_residual_interference
      -result.centered_electric_interference
      -result.centered_magnetic_interference;
  const double boundary_scale=relative_scale(
      result.primitive_face_interference,
      result.induced_boundary_interference,
      result.bound_residual_interference);
  result.boundary_energy_ledger_valid =
      std::abs(result.boundary_flux_sum)
          <= observer_options.gate_tolerance*boundary_scale
      && std::abs(result.primitive_boundary_identity_residual)
          <= observer_options.gate_tolerance*boundary_scale
      && std::abs(result.readout_interference_reconstruction_residual)
          <= observer_options.gate_tolerance*boundary_scale;
  result.energy_partition_residual = result.residual_energy
      -result.outgoing_energy-result.background_energy;
  const double scale = relative_scale(result.residual_energy,
      result.outgoing_energy,result.background_energy);
  result.valid = samples_valid
      && result.maximum_reconstruction_residual
          <= observer_options.gate_tolerance*scale
      && result.actual_gauss_compatibility_residual
          <= observer_options.gate_tolerance
      && std::abs(result.energy_partition_residual)
          <= observer_options.gate_tolerance*scale
      && result.characteristic_flux_residual
          <= observer_options.gate_tolerance*scale
      && std::isfinite(result.pair_internal_energy);
  return result;
}

StateOnlySupportLadderObservation observe_state_only_support_ladder(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const std::vector<int>& support_half_widths,
    double poisson_tolerance,
    int poisson_max_iterations,
    double gate_tolerance,
    bool allow_fractional_center) {
  StateOnlySupportLadderObservation result;
  result.L=state.electric.L;
  const std::size_t expected=result.L>0
      ?static_cast<std::size_t>(result.L)*result.L*result.L:0;
  const bool shapes=result.L>0&&state.magnetic_half.L==result.L
      &&state.electric.x.size()==expected
      &&state.electric.y.size()==expected
      &&state.electric.z.size()==expected;
  const bool ordered=!support_half_widths.empty()
      &&std::adjacent_find(support_half_widths.begin(),
          support_half_widths.end(),std::greater_equal<int>())
          ==support_half_widths.end();
  if(!shapes||!ordered||state.constituents.size()!=2
      ||state.charges.size()!=2||!state.edges.empty()
      ||action_options.binding_law!=ConnectedBindingLaw::DerivedCompactPair
      ||!(poisson_tolerance>0.0)||poisson_max_iterations<=0
      ||!(gate_tolerance>0.0)) return result;

  ConnectedMooreBlockState geometry;
  geometry.electric.L=result.L;
  geometry.magnetic_half.L=result.L;
  geometry.constituents=state.constituents;
  geometry.charges=state.charges;
  geometry.edges=state.edges;
  geometry.width=state.width;
  geometry.orientation_axis=state.orientation_axis;

  const double actual_energy=quadratic_energy(state.electric);
  MatchedFaceFlux previous_bound;
  double previous_energy=0.0;
  bool have_previous=false;
  bool all_valid=true;
  result.scales.reserve(support_half_widths.size());
  if(support_half_widths.size()>1)
    result.transitions.reserve(support_half_widths.size()-1);

  const auto component_accounting=[](
      const std::vector<double>& actual,
      const std::vector<double>& bound,
      long double& residual_squared,long double& interference) {
    for(std::size_t i=0;i<actual.size();++i) {
      const long double b=bound[i];
      const long double r=static_cast<long double>(actual[i])-b;
      residual_squared+=r*r;
      interference+=b*r;
    }
  };
  const auto transition_accounting=[](
      const std::vector<double>& inner,
      const std::vector<double>& outer,
      long double& difference_squared,long double& projection) {
    for(std::size_t i=0;i<inner.size();++i) {
      const long double d=static_cast<long double>(inner[i])-outer[i];
      difference_squared+=d*d;
      projection+=static_cast<long double>(outer[i])*d;
    }
  };

  for(int support:support_half_widths) {
    auto preparation=prepare_finite_support_derived_compact_pair(
        geometry,action_options,support,poisson_tolerance,
        poisson_max_iterations,allow_fractional_center);
    StateOnlySupportScale scale;
    scale.support_half_width=support;
    scale.actual_face_energy=actual_energy;
    scale.poisson_residual=preparation.poisson_residual;
    scale.gauss_residual=preparation.gauss_residual;
    if(!preparation.valid||!preparation.compact_support
        ||!preparation.zero_boundary_crossing) {
      result.scales.push_back(scale);
      return result;
    }
    if(result.scales.empty()) {
      result.center=preparation.center;
      result.support_center=preparation.support_center;
      result.fractional_center_offset=preparation.fractional_center_offset;
      result.fractional_center_enabled=
          preparation.fractional_center_enabled;
    }
    else if((result.center-preparation.center).mag()>gate_tolerance)
      all_valid=false;

    const auto& bound=preparation.state.electric;
    long double residual_squared=0.0L,interference=0.0L;
    component_accounting(state.electric.x,bound.x,
        residual_squared,interference);
    component_accounting(state.electric.y,bound.y,
        residual_squared,interference);
    component_accounting(state.electric.z,bound.z,
        residual_squared,interference);
    scale.bound_face_energy=preparation.electric_energy;
    scale.residual_face_energy=static_cast<double>(0.5L*residual_squared);
    scale.primitive_interference=static_cast<double>(interference);
    scale.energy_reconstruction_residual=scale.actual_face_energy
        -scale.bound_face_energy-scale.residual_face_energy
        -scale.primitive_interference;
    const double scale_norm=relative_scale(scale.actual_face_energy,
        scale.bound_face_energy,scale.residual_face_energy);
    scale.valid=std::abs(scale.energy_reconstruction_residual)
            <=gate_tolerance*scale_norm
        &&scale.gauss_residual<=gate_tolerance;
    result.maximum_energy_reconstruction_residual=std::max(
        result.maximum_energy_reconstruction_residual,
        std::abs(scale.energy_reconstruction_residual));
    all_valid=all_valid&&scale.valid;

    if(have_previous) {
      StateOnlySupportTransition transition;
      transition.inner_half_width=
          result.scales.back().support_half_width;
      transition.outer_half_width=support;
      long double difference_squared=0.0L,projection=0.0L;
      transition_accounting(previous_bound.x,bound.x,
          difference_squared,projection);
      transition_accounting(previous_bound.y,bound.y,
          difference_squared,projection);
      transition_accounting(previous_bound.z,bound.z,
          difference_squared,projection);
      transition.relaxation_energy=
          static_cast<double>(0.5L*difference_squared);
      transition.outer_difference_inner_product=
          static_cast<double>(projection);
      transition.monotonicity_margin=
          previous_energy-scale.bound_face_energy;
      transition.pythagorean_residual=
          transition.monotonicity_margin-transition.relaxation_energy;
      const double transition_norm=relative_scale(previous_energy,
          scale.bound_face_energy,transition.relaxation_energy);
      transition.valid=transition.monotonicity_margin
              >=-gate_tolerance*transition_norm
          &&std::abs(transition.outer_difference_inner_product)
              <=gate_tolerance*transition_norm
          &&std::abs(transition.pythagorean_residual)
              <=gate_tolerance*transition_norm;
      result.maximum_projection_residual=std::max({
          result.maximum_projection_residual,
          std::abs(transition.outer_difference_inner_product),
          std::abs(transition.pythagorean_residual)});
      all_valid=all_valid&&transition.valid;
      result.transitions.push_back(transition);
    }

    previous_bound=std::move(preparation.state.electric);
    previous_energy=scale.bound_face_energy;
    have_previous=true;
    result.scales.push_back(scale);
  }
  result.valid=all_valid
      &&result.scales.size()==support_half_widths.size()
      &&result.transitions.size()+1==result.scales.size();
  return result;
}

}  // namespace ftd::eft
