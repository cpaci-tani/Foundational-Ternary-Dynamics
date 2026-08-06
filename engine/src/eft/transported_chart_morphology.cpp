#include "ftd/eft/transported_chart_morphology.h"

#include "ftd/eft/spline_poynting_momentum.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace ftd::eft {
namespace {

const std::array<Vec3, 3> kFaceOffsets{{
    {0.5, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.0, 0.5}}};
const std::array<Vec3, 3> kEdgeOffsets{{
    {0.0, 0.5, 0.5}, {0.5, 0.0, 0.5}, {0.5, 0.5, 0.0}}};

double component(const MatchedFaceFlux& field,int axis,std::size_t index) {
  return axis==0?field.x[index]:(axis==1?field.y[index]:field.z[index]);
}

double component(const MatchedEdgeField& field,int axis,std::size_t index) {
  return axis==0?field.x[index]:(axis==1?field.y[index]:field.z[index]);
}

double shortest_delta(double coordinate,double center,int L) {
  double delta=coordinate-center;
  const double half=0.5*static_cast<double>(L);
  if(delta>half) delta-=static_cast<double>(L);
  if(delta<-half) delta+=static_cast<double>(L);
  return delta;
}

int shortest_delta(int coordinate,int center,int L) {
  int delta=coordinate-center;
  if(delta>L/2) delta-=L;
  if(delta<-L/2) delta+=L;
  return delta;
}

std::complex<double> phase(const TransportedChartMode& mode,
                           const Vec3& position,int L) {
  const double theta=-2.0*PI*(mode.nx*position.x
      +mode.ny*position.y+mode.nz*position.z)/static_cast<double>(L);
  return std::polar(1.0,theta);
}

bool same_mode(const TransportedChartMode& lhs,
               const TransportedChartMode& rhs) {
  return lhs.nx==rhs.nx&&lhs.ny==rhs.ny&&lhs.nz==rhs.nz;
}

double relative_scale(double a,double b,double c,double d=0.0) {
  return std::max({1.0,std::abs(a),std::abs(b),std::abs(c),std::abs(d)});
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x)&&std::isfinite(value.y)
      &&std::isfinite(value.z);
}

std::complex<double> transported_normalized(
    const std::complex<double>& value,double energy,
    const TransportedChartMode& mode,const Vec3& center,int L) {
  const double theta=2.0*PI*(mode.nx*center.x
      +mode.ny*center.y+mode.nz*center.z)/static_cast<double>(L);
  return value*std::polar(1.0,theta)/energy;
}

template <typename Value,typename Energy>
double channel_distance(const TransportedChartMorphologyObservation& reference,
                        const TransportedChartMorphologyObservation& later,
                        Value value,Energy energy,double tolerance) {
  const double e0=energy(reference),e1=energy(later);
  if(!(e0>tolerance)||!(e1>tolerance)) return INFINITY;
  long double numerator=0.0L,denominator=0.0L;
  for(std::size_t i=0;i<reference.coefficients.size();++i) {
    const auto q0=transported_normalized(
        value(reference.coefficients[i]),e0,
        reference.coefficients[i].mode,reference.center,reference.L);
    const auto q1=transported_normalized(
        value(later.coefficients[i]),e1,
        later.coefficients[i].mode,later.center,later.L);
    numerator+=std::norm(q1-q0);
    denominator+=std::norm(q0);
  }
  if(!(denominator>tolerance*tolerance)) return INFINITY;
  return std::sqrt(static_cast<double>(numerator/denominator));
}

}  // namespace

std::vector<TransportedChartMode> make_transport_modes(
    const std::array<std::array<int, 3>, 3>& basis,
    const std::vector<int>& harmonics) {
  std::vector<TransportedChartMode> result;
  result.reserve(basis.size()*harmonics.size());
  for(const auto& direction:basis) for(const int harmonic:harmonics) {
    if(harmonic<=0) return {};
    result.push_back({harmonic*direction[0],harmonic*direction[1],
                      harmonic*direction[2]});
  }
  return result;
}

TransportedChartMorphologyObservation
observe_transported_chart_morphology(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const TransportedChartMorphologyOptions& options) {
  TransportedChartMorphologyObservation result;
  result.L=state.electric.L;
  const std::size_t expected=result.L>0
      ?static_cast<std::size_t>(result.L)*result.L*result.L:0;
  const bool shapes=result.L>0&&state.magnetic_half.L==result.L
      &&state.electric.x.size()==expected
      &&state.magnetic_half.x.size()==expected;
  const bool radii=options.support_half_width>0&&options.near_radius>0
      &&options.outer_radius>options.near_radius
      &&options.outer_radius<=result.L/2;
  bool modes=!options.modes.empty();
  for(const auto& mode:options.modes)
    modes=modes&&(mode.nx!=0||mode.ny!=0||mode.nz!=0)
        &&std::abs(mode.nx)<=result.L/2
        &&std::abs(mode.ny)<=result.L/2
        &&std::abs(mode.nz)<=result.L/2;
  const double direction_magnitude=options.longitudinal_direction.mag();
  const bool longitudinal_direction_valid=
      finite(options.longitudinal_direction)
      &&std::isfinite(options.longitudinal_dead_band)
      &&options.longitudinal_dead_band>=0.0;
  if(!shapes||!radii||!modes||state.constituents.size()!=2
      ||state.charges.size()!=2||!state.edges.empty()
      ||action_options.binding_law!=ConnectedBindingLaw::DerivedCompactPair
      ||!(options.wave_speed>0.0)||!std::isfinite(options.dt)
      ||!(options.gate_tolerance>0.0)
      ||!longitudinal_direction_valid) return result;

  result.longitudinal_partition_enabled=direction_magnitude>0.0;
  if(result.longitudinal_partition_enabled)
    result.longitudinal_direction=
        options.longitudinal_direction*(1.0/direction_magnitude);

  ConnectedMooreBlockState geometry;
  geometry.electric.L=result.L;
  geometry.magnetic_half.L=result.L;
  geometry.constituents=state.constituents;
  geometry.charges=state.charges;
  geometry.width=state.width;
  geometry.orientation_axis=state.orientation_axis;
  const auto preparation=prepare_finite_support_derived_compact_pair(
      geometry,action_options,options.support_half_width,
      options.poisson_tolerance,options.poisson_max_iterations,true);
  if(!preparation.valid||!preparation.compact_support
      ||!preparation.zero_boundary_crossing) return result;
  result.center=preparation.center;
  result.support_center=preparation.support_center;
  result.fractional_center_offset=preparation.fractional_center_offset;
  result.bound_gauss_residual=preparation.gauss_residual;
  result.coefficients.resize(options.modes.size());
  for(std::size_t i=0;i<options.modes.size();++i)
    result.coefficients[i].mode=options.modes[i];

  const auto actual_b=matched_integer_time_magnetic(
      state.electric,state.magnetic_half,options.wave_speed,options.dt);
  const auto bound_b=matched_integer_time_magnetic(
      preparation.state.electric,preparation.state.magnetic_half,
      options.wave_speed,options.dt);
  const int cx=static_cast<int>(std::llround(result.support_center.x));
  const int cy=static_cast<int>(std::llround(result.support_center.y));
  const int cz=static_cast<int>(std::llround(result.support_center.z));
  const std::size_t plane=static_cast<std::size_t>(result.L)*result.L;
  for(std::size_t index=0;index<expected;++index) {
    const int x=static_cast<int>(index/plane);
    const std::size_t rem=index-static_cast<std::size_t>(x)*plane;
    const int y=static_cast<int>(rem/result.L);
    const int z=static_cast<int>(rem-static_cast<std::size_t>(y)*result.L);
    const int chart_radius=std::max({
        std::abs(shortest_delta(x,cx,result.L)),
        std::abs(shortest_delta(y,cy,result.L)),
        std::abs(shortest_delta(z,cz,result.L))});
    const bool near=chart_radius<=options.near_radius;
    const bool outer=chart_radius>options.near_radius
        &&chart_radius<=options.outer_radius;
    for(int axis=0;axis<3;++axis) for(int family=0;family<2;++family) {
      const auto& offset=family==0?kFaceOffsets[axis]:kEdgeOffsets[axis];
      const Vec3 position{x+offset.x,y+offset.y,z+offset.z};
      const double actual=family==0
          ?component(state.electric,axis,index)
          :component(actual_b,axis,index);
      const double bound=family==0
          ?component(preparation.state.electric,axis,index)
          :component(bound_b,axis,index);
      const double residual=actual-bound;
      const double ua=0.5*actual*actual;
      const double ub=0.5*bound*bound;
      const double ur=0.5*residual*residual;
      const double ui=bound*residual;
      result.actual_energy+=ua;
      result.bound_energy+=ub;
      result.residual_energy+=ur;
      result.interference_energy+=ui;
      const Vec3 delta{
          shortest_delta(position.x,result.center.x,result.L),
          shortest_delta(position.y,result.center.y,result.L),
          shortest_delta(position.z,result.center.z,result.L)};
      if(near) {
        result.near_residual_energy+=ur;
        result.near_residual_first_moment+=delta*ur;
        result.near_residual_second_moment+=ur*delta.mag2();
        if(result.longitudinal_partition_enabled) {
          const double longitudinal=delta.dot(result.longitudinal_direction);
          if(longitudinal<-options.longitudinal_dead_band)
            result.near_longitudinal.trailing+=ur;
          else if(longitudinal>options.longitudinal_dead_band)
            result.near_longitudinal.leading+=ur;
          else result.near_longitudinal.neutral+=ur;
        }
      } else if(outer) {
        result.outer_residual_energy+=ur;
        result.outer_residual_first_moment+=delta*ur;
        result.outer_residual_second_moment+=ur*delta.mag2();
        if(result.longitudinal_partition_enabled) {
          const double longitudinal=delta.dot(result.longitudinal_direction);
          if(longitudinal<-options.longitudinal_dead_band)
            result.outer_longitudinal.trailing+=ur;
          else if(longitudinal>options.longitudinal_dead_band)
            result.outer_longitudinal.leading+=ur;
          else result.outer_longitudinal.neutral+=ur;
        }
      }
      for(auto& coefficient:result.coefficients) {
        const auto p=phase(coefficient.mode,position,result.L);
        coefficient.actual+=ua*p;
        coefficient.bound+=ub*p;
        coefficient.residual+=ur*p;
        coefficient.interference+=ui*p;
        if(near) coefficient.near_residual+=ur*p;
      }
    }
  }
  if(result.near_residual_energy>0.0) {
    result.near_residual_first_moment=
        result.near_residual_first_moment
        *(1.0/result.near_residual_energy);
    result.near_residual_rms_radius=std::sqrt(
        result.near_residual_second_moment/result.near_residual_energy);
  }
  if(result.outer_residual_energy>0.0) {
    result.outer_residual_first_moment=
        result.outer_residual_first_moment
        *(1.0/result.outer_residual_energy);
    result.outer_residual_rms_radius=std::sqrt(
        result.outer_residual_second_moment/result.outer_residual_energy);
  }
  result.energy_reconstruction_residual=result.actual_energy
      -result.bound_energy-result.residual_energy-result.interference_energy;
  if(result.longitudinal_partition_enabled)
    result.longitudinal_partition_residual=std::max(
        std::abs(result.near_residual_energy
            -result.near_longitudinal.total()),
        std::abs(result.outer_residual_energy
            -result.outer_longitudinal.total()));
  for(const auto& coefficient:result.coefficients)
    result.maximum_mode_reconstruction_residual=std::max(
        result.maximum_mode_reconstruction_residual,
        std::abs(coefficient.actual-coefficient.bound
            -coefficient.residual-coefficient.interference));
  const double scale=relative_scale(result.actual_energy,result.bound_energy,
      result.residual_energy,result.interference_energy);
  result.valid=std::abs(result.energy_reconstruction_residual)
          <=options.gate_tolerance*scale
      &&result.maximum_mode_reconstruction_residual
          <=options.gate_tolerance*scale
      &&result.longitudinal_partition_residual
          <=options.gate_tolerance*scale
      &&result.bound_gauss_residual<=options.gate_tolerance;
  return result;
}

TransportedChartMorphologyComparison compare_transported_chart_morphology(
    const TransportedChartMorphologyObservation& reference,
    const TransportedChartMorphologyObservation& later,double tolerance) {
  TransportedChartMorphologyComparison result;
  if(!reference.valid||!later.valid||reference.L!=later.L
      ||reference.coefficients.size()!=later.coefficients.size()
      ||reference.coefficients.empty()||!(tolerance>0.0)) return result;
  for(std::size_t i=0;i<reference.coefficients.size();++i)
    if(!same_mode(reference.coefficients[i].mode,
                  later.coefficients[i].mode)) return result;
  result.actual_distance=channel_distance(reference,later,
      [](const auto& value){return value.actual;},
      [](const auto& value){return value.actual_energy;},tolerance);
  result.bound_distance=channel_distance(reference,later,
      [](const auto& value){return value.bound;},
      [](const auto& value){return value.bound_energy;},tolerance);
  result.residual_distance=channel_distance(reference,later,
      [](const auto& value){return value.residual;},
      [](const auto& value){return value.residual_energy;},tolerance);
  result.near_residual_distance=channel_distance(reference,later,
      [](const auto& value){return value.near_residual;},
      [](const auto& value){return value.near_residual_energy;},tolerance);
  result.actual_energy_ratio=later.actual_energy/reference.actual_energy;
  result.bound_energy_ratio=later.bound_energy/reference.bound_energy;
  result.residual_energy_ratio=later.residual_energy/reference.residual_energy;
  result.near_residual_energy_ratio=
      later.near_residual_energy/reference.near_residual_energy;
  result.near_first_moment_change=later.near_residual_first_moment
      -reference.near_residual_first_moment;
  result.outer_first_moment_change=later.outer_residual_first_moment
      -reference.outer_residual_first_moment;
  result.valid=std::isfinite(result.actual_distance)
      &&std::isfinite(result.bound_distance)
      &&std::isfinite(result.residual_distance)
      &&std::isfinite(result.near_residual_distance)
      &&std::isfinite(result.actual_energy_ratio)
      &&std::isfinite(result.bound_energy_ratio)
      &&std::isfinite(result.residual_energy_ratio)
      &&std::isfinite(result.near_residual_energy_ratio);
  return result;
}

}  // namespace ftd::eft
