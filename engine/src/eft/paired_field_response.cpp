#include "ftd/eft/paired_field_response.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft {
namespace {

template <typename Field>
bool valid_field(const Field& field, int L) {
  const auto count = static_cast<std::size_t>(L)*L*L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count;
}

bool finite_field(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

template <typename Field>
bool finite_field(const Field& field) {
  return finite_field(field.x) && finite_field(field.y)
      && finite_field(field.z);
}

double periodic_delta(double coordinate, double center, int L) {
  double result = coordinate-center;
  const double half = 0.5*static_cast<double>(L);
  if (result > half) result -= static_cast<double>(L);
  if (result < -half) result += static_cast<double>(L);
  return result;
}

Vec3 periodic_delta(const Vec3& position, const Vec3& center, int L) {
  return {periodic_delta(position.x,center.x,L),
          periodic_delta(position.y,center.y,L),
          periodic_delta(position.z,center.z,L)};
}

bool finite_vec(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

bool valid_region(const FieldResponseRegionSpec& region) {
  if (!finite_vec(region.center) || !finite_vec(region.longitudinal)
      || !finite_vec(region.transverse_u) || !finite_vec(region.transverse_v))
    return false;
  if (region.kind == FieldResponseRegionKind::ChebyshevCube)
    return region.chebyshev_radius >= 0.0
        && std::isfinite(region.chebyshev_radius);
  const double dl = region.longitudinal.mag();
  const double du = region.transverse_u.mag();
  const double dv = region.transverse_v.mag();
  return region.longitudinal_half_width >= 0.0
      && region.transverse_half_width >= 0.0
      && std::isfinite(region.longitudinal_half_width)
      && std::isfinite(region.transverse_half_width)
      && std::abs(dl-1.0) <= 1e-12
      && std::abs(du-1.0) <= 1e-12
      && std::abs(dv-1.0) <= 1e-12
      && std::abs(region.longitudinal.dot(region.transverse_u)) <= 1e-12
      && std::abs(region.longitudinal.dot(region.transverse_v)) <= 1e-12
      && std::abs(region.transverse_u.dot(region.transverse_v)) <= 1e-12;
}

bool contains(const FieldResponseRegionSpec& region,
              const Vec3& position, int L) {
  const Vec3 delta=periodic_delta(position,region.center,L);
  if (region.kind == FieldResponseRegionKind::ChebyshevCube)
    return std::max({std::abs(delta.x),std::abs(delta.y),std::abs(delta.z)})
        <=region.chebyshev_radius;
  return std::abs(delta.dot(region.longitudinal))
          <=region.longitudinal_half_width
      &&std::abs(delta.dot(region.transverse_u))
          <=region.transverse_half_width
      &&std::abs(delta.dot(region.transverse_v))
          <=region.transverse_half_width;
}

double longitudinal_coordinate(const FieldResponseRegionSpec& region,
                               const Vec3& position,int L) {
  return periodic_delta(position,region.center,L).dot(region.longitudinal);
}

const std::vector<double>& component(const MatchedFaceFlux& field,int axis) {
  return axis==0?field.x:(axis==1?field.y:field.z);
}

const std::vector<double>& component(const MatchedEdgeField& field,int axis) {
  return axis==0?field.x:(axis==1?field.y:field.z);
}

Vec3 component_position(int family,int axis,int x,int y,int z) {
  if (family==0) {
    if (axis==0) return {x+0.5,static_cast<double>(y),static_cast<double>(z)};
    if (axis==1) return {static_cast<double>(x),y+0.5,static_cast<double>(z)};
    return {static_cast<double>(x),static_cast<double>(y),z+0.5};
  }
  if (axis==0) return {static_cast<double>(x),y+0.5,z+0.5};
  if (axis==1) return {x+0.5,static_cast<double>(y),z+0.5};
  return {x+0.5,y+0.5,static_cast<double>(z)};
}

MatchedEdgeField integer_magnetic(const MatchedFaceFlux& electric,
                                  const MatchedEdgeField& magnetic_half,
                                  double half_step_scale) {
  MatchedEdgeField result=magnetic_half;
  const auto curl=matched_curl_adjoint(electric);
  for (std::size_t i=0;i<result.x.size();++i) {
    result.x[i]+=half_step_scale*curl.x[i];
    result.y[i]+=half_step_scale*curl.y[i];
    result.z[i]+=half_step_scale*curl.z[i];
  }
  return result;
}

ConnectedMooreBlockState geometry_only(const ConnectedMooreBlockState& state) {
  ConnectedMooreBlockState result;
  result.electric.L=state.electric.L;
  result.magnetic_half.L=state.magnetic_half.L;
  result.constituents=state.constituents;
  result.charges=state.charges;
  result.edges=state.edges;
  result.width=state.width;
  result.orientation_axis=state.orientation_axis;
  return result;
}

void add_channel(QuadraticFieldDifferenceChannel& result,
                 double moving,double rest,double longitudinal) {
  const double difference=moving-rest;
  const double moving_energy=0.5*moving*moving;
  const double rest_energy=0.5*rest*rest;
  const double difference_energy=0.5*difference*difference;
  const double cross=rest*difference;
  const double energy_difference=moving_energy-rest_energy;
  result.moving_energy+=moving_energy;
  result.rest_energy+=rest_energy;
  result.difference_field_energy+=difference_energy;
  result.cross_energy+=cross;
  result.energy_difference_first_moment+=energy_difference*longitudinal;
  result.difference_field_first_moment+=difference_energy*longitudinal;
  result.cross_first_moment+=cross*longitudinal;
}

void finalize_channel(QuadraticFieldDifferenceChannel& result) {
  result.energy_difference=result.moving_energy-result.rest_energy;
  result.energy_identity_residual=result.energy_difference
      -result.cross_energy-result.difference_field_energy;
}

double maximum_identity(const PairedFieldResponseObservation& value) {
  double result=0.0;
  for (const auto& region:value.regions)
    result=std::max({result,
        std::abs(region.actual.energy_identity_residual),
        std::abs(region.residual.energy_identity_residual)});
  return result;
}

double maximum_scale(const PairedFieldResponseObservation& value) {
  double result=1.0;
  for (const auto& region:value.regions) for (const auto* channel:
      {&region.actual,&region.residual})
    result=std::max({result,std::abs(channel->moving_energy),
        std::abs(channel->rest_energy),
        std::abs(channel->difference_field_energy),
        std::abs(channel->cross_energy)});
  return result;
}

double regional_modified_energy(const MatchedFaceFlux& electric,
                                const MatchedEdgeField& magnetic,
                                const MatchedFaceFlux& curl_magnetic,
                                const MatchedEdgeField& curl_electric,
                                double lambda,
                                const FieldResponseRegionSpec& region,
                                bool select_inside) {
  long double result=0.0L;
  const int L=electric.L;
  for (int x=0;x<L;++x) for (int y=0;y<L;++y) for (int z=0;z<L;++z) {
    const auto index=static_cast<std::size_t>(electric.index(x,y,z));
    for (int axis=0;axis<3;++axis) {
      const auto e_position=component_position(0,axis,x,y,z);
      if (contains(region,e_position,L)==select_inside) {
        const double e=component(electric,axis)[index];
        const double cb=component(curl_magnetic,axis)[index];
        result+=0.5L*e*e-0.25L*lambda*e*cb;
      }
      const auto b_position=component_position(1,axis,x,y,z);
      if (contains(region,b_position,L)==select_inside) {
        const double b=component(magnetic,axis)[index];
        const double cte=component(curl_electric,axis)[index];
        result+=0.5L*b*b-0.25L*lambda*b*cte;
      }
    }
  }
  return static_cast<double>(result);
}

}  // namespace

std::array<FieldResponseRegionSpec,4> make_ftd0768_response_regions(
    const PairedFieldResponseOptions& options) {
  std::array<FieldResponseRegionSpec,4> result{};
  result[0].kind=FieldResponseRegionKind::OrientedSlab;
  result[0].center=options.laboratory_center;
  result[0].longitudinal=options.longitudinal;
  result[0].transverse_u=options.transverse_u;
  result[0].transverse_v=options.transverse_v;
  result[0].longitudinal_half_width=0.5;
  result[0].transverse_half_width=4.0;
  for (std::size_t i=1;i<result.size();++i) {
    result[i].kind=FieldResponseRegionKind::ChebyshevCube;
    result[i].center=options.moving_center;
    result[i].longitudinal=options.longitudinal;
    result[i].transverse_u=options.transverse_u;
    result[i].transverse_v=options.transverse_v;
  }
  result[1].chebyshev_radius=options.support_half_width;
  result[2].chebyshev_radius=options.near_radius;
  result[3].chebyshev_radius=options.outer_radius;
  return result;
}

PairedFieldResponseObservation observe_paired_field_response(
    const ConnectedMooreBlockState& moving,
    const ConnectedMooreBlockState& rest,
    const ConnectedMooreBlockOptions& action_options,
    const PairedFieldResponseOptions& options) {
  PairedFieldResponseObservation result;
  result.L=moving.electric.L;
  const int L=result.L;
  if (L<=0 || rest.electric.L!=L || moving.magnetic_half.L!=L
      || rest.magnetic_half.L!=L || moving.constituents.size()!=2
      || rest.constituents.size()!=2 || moving.charges.size()!=2
      || rest.charges.size()!=2 || !valid_field(moving.electric,L)
      || !valid_field(rest.electric,L)
      || !valid_field(moving.magnetic_half,L)
      || !valid_field(rest.magnetic_half,L)
      || !finite_field(moving.electric) || !finite_field(rest.electric)
      || !finite_field(moving.magnetic_half)
      || !finite_field(rest.magnetic_half)
      || options.outer_radius>L/2 || !(options.wave_speed>0.0)
      || !(options.dt>0.0) || !(options.gate_tolerance>0.0)) return result;
  result.regions={};
  const auto regions=make_ftd0768_response_regions(options);
  for (std::size_t i=0;i<regions.size();++i) {
    if (!valid_region(regions[i])) return result;
    result.regions[i].spec=regions[i];
  }

  const auto moving_bound=prepare_finite_support_derived_compact_pair(
      geometry_only(moving),action_options,options.support_half_width,
      options.poisson_tolerance,options.poisson_max_iterations,true);
  const auto rest_bound=prepare_finite_support_derived_compact_pair(
      geometry_only(rest),action_options,options.support_half_width,
      options.poisson_tolerance,options.poisson_max_iterations,true);
  if (!moving_bound.valid || !rest_bound.valid
      || !moving_bound.compact_support || !rest_bound.compact_support
      || !moving_bound.zero_boundary_crossing
      || !rest_bound.zero_boundary_crossing) return result;
  result.moving_bound_center=moving_bound.center;
  result.rest_bound_center=rest_bound.center;
  result.moving_bound_gauss_residual=moving_bound.gauss_residual;
  result.rest_bound_gauss_residual=rest_bound.gauss_residual;

  const double half_step_scale=-0.5*options.wave_speed*options.dt;
  const auto moving_actual_b=integer_magnetic(
      moving.electric,moving.magnetic_half,half_step_scale);
  const auto rest_actual_b=integer_magnetic(
      rest.electric,rest.magnetic_half,half_step_scale);
  const auto moving_bound_b=integer_magnetic(
      moving_bound.state.electric,moving_bound.state.magnetic_half,
      half_step_scale);
  const auto rest_bound_b=integer_magnetic(
      rest_bound.state.electric,rest_bound.state.magnetic_half,
      half_step_scale);
  const std::size_t plane=static_cast<std::size_t>(L)*L;
  const std::size_t count=plane*L;
  for (std::size_t linear=0;linear<count;++linear) {
    const int x=static_cast<int>(linear/plane);
    const auto rem=linear-static_cast<std::size_t>(x)*plane;
    const int y=static_cast<int>(rem/L);
    const int z=static_cast<int>(rem-static_cast<std::size_t>(y)*L);
    for (int family=0;family<2;++family) for (int axis=0;axis<3;++axis) {
      const Vec3 position=component_position(family,axis,x,y,z);
      const double moving_actual=family==0
          ?component(moving.electric,axis)[linear]
          :component(moving_actual_b,axis)[linear];
      const double rest_actual=family==0
          ?component(rest.electric,axis)[linear]
          :component(rest_actual_b,axis)[linear];
      const double moving_selected=family==0
          ?component(moving_bound.state.electric,axis)[linear]
          :component(moving_bound_b,axis)[linear];
      const double rest_selected=family==0
          ?component(rest_bound.state.electric,axis)[linear]
          :component(rest_bound_b,axis)[linear];
      for (auto& region:result.regions) if (contains(region.spec,position,L)) {
        const double longitudinal=longitudinal_coordinate(
            region.spec,position,L);
        add_channel(region.actual,moving_actual,rest_actual,longitudinal);
        add_channel(region.residual,moving_actual-moving_selected,
            rest_actual-rest_selected,longitudinal);
      }
    }
  }
  for (auto& region:result.regions) {
    finalize_channel(region.actual);
    finalize_channel(region.residual);
  }
  result.maximum_energy_identity_residual=maximum_identity(result);
  result.valid=result.maximum_energy_identity_residual
          <=options.gate_tolerance*maximum_scale(result)
      &&result.moving_bound_gauss_residual<=options.gate_tolerance
      &&result.rest_bound_gauss_residual<=options.gate_tolerance;
  return result;
}

RegionalModifiedEnergyTransportObservation
observe_regional_modified_energy_transport(
    const MatchedFaceFlux& electric_before,
    const MatchedEdgeField& magnetic_before,
    const MatchedFaceFlux& electric_pre_current,
    const MatchedEdgeField& magnetic_after,
    const MatchedFaceFlux& electric_after,
    double lambda,const FieldResponseRegionSpec& region,double tolerance) {
  RegionalModifiedEnergyTransportObservation result;
  result.spec=region;
  const int L=electric_before.L;
  if (L<=0 || !(lambda>0.0) || !(tolerance>0.0)
      || !valid_region(region) || !valid_field(electric_before,L)
      || !valid_field(magnetic_before,L)
      || !valid_field(electric_pre_current,L)
      || !valid_field(magnetic_after,L)
      || !valid_field(electric_after,L)) return result;
  const auto curl_b0=matched_curl(magnetic_before);
  const auto curl_e0=matched_curl_adjoint(electric_before);
  const auto curl_b1=matched_curl(magnetic_after);
  const auto curl_epre=matched_curl_adjoint(electric_pre_current);
  const auto curl_e1=matched_curl_adjoint(electric_after);
  result.energy_before=regional_modified_energy(electric_before,
      magnetic_before,curl_b0,curl_e0,lambda,region,true);
  result.energy_pre_current=regional_modified_energy(electric_pre_current,
      magnetic_after,curl_b1,curl_epre,lambda,region,true);
  result.energy_after=regional_modified_energy(electric_after,
      magnetic_after,curl_b1,curl_e1,lambda,region,true);
  result.outside_energy_before=regional_modified_energy(electric_before,
      magnetic_before,curl_b0,curl_e0,lambda,region,false);
  result.outside_energy_pre_current=regional_modified_energy(
      electric_pre_current,magnetic_after,curl_b1,curl_epre,lambda,
      region,false);
  result.outside_energy_after=regional_modified_energy(electric_after,
      magnetic_after,curl_b1,curl_e1,lambda,region,false);
  result.boundary_transport_into=result.energy_pre_current-result.energy_before;
  result.boundary_transport_into_complement=
      result.outside_energy_pre_current-result.outside_energy_before;
  result.source_exchange_into_field=result.energy_after
      -result.energy_pre_current;
  result.energy_change=result.energy_after-result.energy_before;
  result.global_source_free_residual=
      (result.energy_pre_current+result.outside_energy_pre_current)
      -(result.energy_before+result.outside_energy_before);
  result.boundary_quadrature_residual=result.boundary_transport_into
      +result.boundary_transport_into_complement;
  result.ledger_residual=result.energy_change
      -result.boundary_transport_into-result.source_exchange_into_field;
  const double scale=std::max({1.0,std::abs(result.energy_before),
      std::abs(result.energy_pre_current),std::abs(result.energy_after),
      std::abs(result.outside_energy_before),
      std::abs(result.outside_energy_pre_current),
      std::abs(result.outside_energy_after)});
  result.valid=std::abs(result.ledger_residual)<=tolerance*scale
      &&std::abs(result.global_source_free_residual)<=tolerance*scale
      &&std::abs(result.boundary_quadrature_residual)<=tolerance*scale
      &&std::isfinite(result.energy_before)
      &&std::isfinite(result.energy_pre_current)
      &&std::isfinite(result.energy_after)
      &&std::isfinite(result.outside_energy_before)
      &&std::isfinite(result.outside_energy_pre_current)
      &&std::isfinite(result.outside_energy_after);
  return result;
}

RegionalControlVolumeTransportObservation
derive_regional_control_volume_transport(
    const RegionalModifiedEnergyTransportObservation& previous_region,
    const RegionalModifiedEnergyTransportObservation& current_region,
    double tolerance) {
  RegionalControlVolumeTransportObservation result;
  if(!previous_region.valid||!current_region.valid||!(tolerance>0.0))
    return result;
  result.previous_energy_before=previous_region.energy_before;
  result.current_energy_before=current_region.energy_before;
  result.current_energy_after=current_region.energy_after;
  result.mask_sweep_into=current_region.energy_before
      -previous_region.energy_before;
  result.mask_sweep_into_complement=current_region.outside_energy_before
      -previous_region.outside_energy_before;
  result.mask_sweep_quadrature_residual=result.mask_sweep_into
      +result.mask_sweep_into_complement;
  result.transported_energy_change=current_region.energy_after
      -previous_region.energy_before;
  result.transport_identity_residual=result.transported_energy_change
      -current_region.energy_change-result.mask_sweep_into;
  const double scale=std::max({1.0,
      std::abs(result.previous_energy_before),
      std::abs(result.current_energy_before),
      std::abs(result.current_energy_after),
      std::abs(previous_region.outside_energy_before),
      std::abs(current_region.outside_energy_before)});
  result.valid=std::abs(result.mask_sweep_quadrature_residual)
          <=tolerance*scale
      &&std::abs(result.transport_identity_residual)<=tolerance*scale;
  return result;
}

}  // namespace ftd::eft
