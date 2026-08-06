/** FTD-0764: transported-chart morphology CPU/CUDA qualification. */

#define main ftd0763_fractional_observer_reference_main
#include "test_cuda_fractional_center_state_only_observer.cpp"
#undef main

#include "ftd/eft/cuda_transported_chart_morphology.h"
#include "ftd/eft/spline_poynting_momentum.h"

namespace {

using ftd::eft::TransportedChartMode;
using ftd::eft::TransportedChartMorphologyObservation;
using ftd::eft::TransportedChartMorphologyOptions;

std::array<std::array<int,3>,3> basis_for(int family) {
  if(family==0) return {{{0,0,1},{1,0,0},{0,1,0}}};
  if(family==1) return {{{0,1,-1},{1,0,0},{0,1,1}}};
  return {{{1,1,1},{1,-1,0},{1,1,-2}}};
}

ftd::Vec3 direction_for(int family) {
  if(family==0) return {0,0,1};
  if(family==1) return {0,1,-1};
  return {1,1,1};
}

bool close_complex(const std::complex<double>& lhs,
                   const std::complex<double>& rhs,
                   double tolerance=1e-11) {
  return std::abs(lhs-rhs)<=tolerance*std::max({1.0,std::abs(lhs),
                                                std::abs(rhs)});
}

bool close_partition(const ftd::eft::ResidualLongitudinalPartition& lhs,
                     const ftd::eft::ResidualLongitudinalPartition& rhs) {
  return close(lhs.trailing,rhs.trailing,1e-11)
      &&close(lhs.neutral,rhs.neutral,1e-11)
      &&close(lhs.leading,rhs.leading,1e-11);
}

double direct_energy(const ftd::eft::ConnectedMooreBlockState& state,
                     const TransportedChartMorphologyOptions& options) {
  const auto magnetic=ftd::eft::matched_integer_time_magnetic(
      state.electric,state.magnetic_half,options.wave_speed,options.dt);
  long double value=0.0L;
  for(std::size_t i=0;i<state.electric.x.size();++i)
    value+=0.5L*(state.electric.x[i]*state.electric.x[i]
        +state.electric.y[i]*state.electric.y[i]
        +state.electric.z[i]*state.electric.z[i]
        +magnetic.x[i]*magnetic.x[i]
        +magnetic.y[i]*magnetic.y[i]
        +magnetic.z[i]*magnetic.z[i]);
  return static_cast<double>(value);
}

double periodic_delta(double coordinate,double center,int L) {
  double delta=coordinate-center;
  const double half=0.5*static_cast<double>(L);
  if(delta>half) delta-=static_cast<double>(L);
  if(delta<-half) delta+=static_cast<double>(L);
  return delta;
}

int periodic_delta(int coordinate,int center,int L) {
  int delta=coordinate-center;
  if(delta>L/2) delta-=L;
  if(delta<-L/2) delta+=L;
  return delta;
}

struct DirectPartitions {
  ftd::eft::ResidualLongitudinalPartition near{};
  ftd::eft::ResidualLongitudinalPartition outer{};
};

DirectPartitions direct_partitions(
    const ftd::eft::ConnectedMooreBlockState& state,
    const ftd::eft::ConnectedMooreBlockOptions& action,
    const TransportedChartMorphologyOptions& options) {
  DirectPartitions result;
  auto geometry=state;
  geometry.electric=ftd::eft::MatchedFaceFlux(state.electric.L);
  geometry.magnetic_half=ftd::eft::MatchedEdgeField(state.electric.L);
  const auto preparation=ftd::eft::prepare_finite_support_derived_compact_pair(
      geometry,action,options.support_half_width,options.poisson_tolerance,
      options.poisson_max_iterations,true);
  if(!preparation.valid) return result;
  const auto actual_b=ftd::eft::matched_integer_time_magnetic(
      state.electric,state.magnetic_half,options.wave_speed,options.dt);
  const auto bound_b=ftd::eft::matched_integer_time_magnetic(
      preparation.state.electric,preparation.state.magnetic_half,
      options.wave_speed,options.dt);
  const int L=state.electric.L;
  const int cx=static_cast<int>(std::llround(preparation.support_center.x));
  const int cy=static_cast<int>(std::llround(preparation.support_center.y));
  const int cz=static_cast<int>(std::llround(preparation.support_center.z));
  const auto direction=options.longitudinal_direction
      *(1.0/options.longitudinal_direction.mag());
  const std::array<ftd::Vec3,3> faces{{{0.5,0,0},{0,0.5,0},{0,0,0.5}}};
  const std::array<ftd::Vec3,3> edges{{{0,0.5,0.5},{0.5,0,0.5},{0.5,0.5,0}}};
  const auto add=[&](ftd::eft::ResidualLongitudinalPartition& partition,
                     double longitudinal,double energy) {
    if(longitudinal<-options.longitudinal_dead_band)
      partition.trailing+=energy;
    else if(longitudinal>options.longitudinal_dead_band)
      partition.leading+=energy;
    else partition.neutral+=energy;
  };
  for(int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z) {
    const int radius=std::max({std::abs(periodic_delta(x,cx,L)),
        std::abs(periodic_delta(y,cy,L)),
        std::abs(periodic_delta(z,cz,L))});
    const bool near=radius<=options.near_radius;
    const bool outer=radius>options.near_radius&&radius<=options.outer_radius;
    if(!near&&!outer) continue;
    const auto index=static_cast<std::size_t>(state.electric.index(x,y,z));
    for(int family=0;family<2;++family) for(int axis=0;axis<3;++axis) {
      const auto& offset=family==0?faces[axis]:edges[axis];
      const ftd::Vec3 delta{
          periodic_delta(x+offset.x,preparation.center.x,L),
          periodic_delta(y+offset.y,preparation.center.y,L),
          periodic_delta(z+offset.z,preparation.center.z,L)};
      const double actual=family==0
          ?(axis==0?state.electric.x[index]:
              (axis==1?state.electric.y[index]:state.electric.z[index]))
          :(axis==0?actual_b.x[index]:
              (axis==1?actual_b.y[index]:actual_b.z[index]));
      const double bound=family==0
          ?(axis==0?preparation.state.electric.x[index]:
              (axis==1?preparation.state.electric.y[index]:
                       preparation.state.electric.z[index]))
          :(axis==0?bound_b.x[index]:
              (axis==1?bound_b.y[index]:bound_b.z[index]));
      add(near?result.near:result.outer,delta.dot(direction),
          0.5*(actual-bound)*(actual-bound));
    }
  }
  return result;
}

bool add_longitudinal_fixture(
    ftd::eft::ConnectedMooreBlockState& state,const ftd::Vec3& center,
    const ftd::Vec3& support_center,const ftd::Vec3& direction,
    int near_radius,bool add_trailing,bool add_leading) {
  const int L=state.electric.L;
  const int cx=static_cast<int>(std::llround(support_center.x));
  const int cy=static_cast<int>(std::llround(support_center.y));
  const int cz=static_cast<int>(std::llround(support_center.z));
  const ftd::Vec3 unit=direction*(1.0/direction.mag());
  const std::array<ftd::Vec3,3> edges{{{0,0.5,0.5},{0.5,0,0.5},{0.5,0.5,0}}};
  bool trailing_done=!add_trailing,leading_done=!add_leading;
  for(int x=0;x<L&&(!trailing_done||!leading_done);++x)
    for(int y=0;y<L&&(!trailing_done||!leading_done);++y)
      for(int z=0;z<L&&(!trailing_done||!leading_done);++z) {
        const int radius=std::max({std::abs(periodic_delta(x,cx,L)),
            std::abs(periodic_delta(y,cy,L)),
            std::abs(periodic_delta(z,cz,L))});
        if(radius>near_radius) continue;
        const auto index=static_cast<std::size_t>(
            state.magnetic_half.index(x,y,z));
        for(int axis=0;axis<3;++axis) {
          const auto& offset=edges[axis];
          const ftd::Vec3 delta{periodic_delta(x+offset.x,center.x,L),
              periodic_delta(y+offset.y,center.y,L),
              periodic_delta(z+offset.z,center.z,L)};
          const double longitudinal=delta.dot(unit);
          if(!trailing_done&&longitudinal<-0.5) {
            (axis==0?state.magnetic_half.x[index]:
                (axis==1?state.magnetic_half.y[index]:
                         state.magnetic_half.z[index]))+=1e-3;
            trailing_done=true;
          } else if(!leading_done&&longitudinal>0.5) {
            (axis==0?state.magnetic_half.x[index]:
                (axis==1?state.magnetic_half.y[index]:
                         state.magnetic_half.z[index]))+=1e-3;
            leading_done=true;
          }
        }
      }
  return trailing_done&&leading_done;
}

void compare_cpu_cuda(const std::string& label,
                      const TransportedChartMorphologyObservation& cpu,
                      const TransportedChartMorphologyObservation& gpu,
                      const ftd::eft::CudaTransportedChartMorphologyTelemetry& t) {
  check(label+" valid",cpu.valid&&gpu.valid&&t.valid);
  check(label+" centers",(cpu.center-gpu.center).mag()<=1e-13
      &&(cpu.support_center-gpu.support_center).mag()<=1e-13
      &&(cpu.fractional_center_offset
          -gpu.fractional_center_offset).mag()<=1e-13);
  check(label+" scalar parity",
      close(cpu.actual_energy,gpu.actual_energy,1e-11)
      &&close(cpu.bound_energy,gpu.bound_energy,1e-11)
      &&close(cpu.residual_energy,gpu.residual_energy,1e-11)
      &&close(cpu.interference_energy,gpu.interference_energy,1e-11)
      &&close(cpu.near_residual_energy,gpu.near_residual_energy,1e-11)
      &&close(cpu.outer_residual_energy,gpu.outer_residual_energy,1e-11)
      &&(cpu.near_residual_first_moment
          -gpu.near_residual_first_moment).mag()<=1e-11
      &&(cpu.outer_residual_first_moment
          -gpu.outer_residual_first_moment).mag()<=1e-11
      &&close(cpu.near_residual_rms_radius,
               gpu.near_residual_rms_radius,1e-11)
      &&close(cpu.outer_residual_rms_radius,
               gpu.outer_residual_rms_radius,1e-11)
      &&close_partition(cpu.near_longitudinal,gpu.near_longitudinal)
      &&close_partition(cpu.outer_longitudinal,gpu.outer_longitudinal)
      &&close(cpu.longitudinal_partition_residual,
               gpu.longitudinal_partition_residual,1e-12));
  check(label+" coefficient count",
      cpu.coefficients.size()==gpu.coefficients.size());
  for(std::size_t i=0;i<std::min(
      cpu.coefficients.size(),gpu.coefficients.size());++i) {
    const auto& a=cpu.coefficients[i];
    const auto& b=gpu.coefficients[i];
    check(label+" mode "+std::to_string(i),
        a.mode.nx==b.mode.nx&&a.mode.ny==b.mode.ny&&a.mode.nz==b.mode.nz
        &&close_complex(a.actual,b.actual)
        &&close_complex(a.bound,b.bound)
        &&close_complex(a.residual,b.residual)
        &&close_complex(a.interference,b.interference)
        &&close_complex(a.near_residual,b.near_residual));
  }
  check(label+" scalar-only download",
      t.complete_field_downloads==0&&t.device_to_host_bytes<16*1024*1024);
}

void compare_symmetry(const std::string& label,
                      const TransportedChartMorphologyObservation& lhs,
                      const TransportedChartMorphologyObservation& rhs) {
  check(label+" valid",lhs.valid&&rhs.valid);
  check(label+" energies",close(lhs.actual_energy,rhs.actual_energy,1e-11)
      &&close(lhs.bound_energy,rhs.bound_energy,1e-11)
      &&close(lhs.residual_energy,rhs.residual_energy,1e-11)
      &&close(lhs.interference_energy,rhs.interference_energy,1e-11)
      &&close(lhs.near_residual_energy,rhs.near_residual_energy,1e-11)
      &&close(lhs.outer_residual_energy,rhs.outer_residual_energy,1e-11)
      &&close_partition(lhs.near_longitudinal,rhs.near_longitudinal)
      &&close_partition(lhs.outer_longitudinal,rhs.outer_longitudinal));
  check(label+" mode count",lhs.coefficients.size()==rhs.coefficients.size());
  for(std::size_t i=0;i<std::min(
      lhs.coefficients.size(),rhs.coefficients.size());++i)
    check(label+" coefficient "+std::to_string(i),
        close_complex(lhs.coefficients[i].actual,rhs.coefficients[i].actual)
        &&close_complex(lhs.coefficients[i].bound,rhs.coefficients[i].bound)
        &&close_complex(lhs.coefficients[i].residual,
                         rhs.coefficients[i].residual)
        &&close_complex(lhs.coefficients[i].interference,
                         rhs.coefficients[i].interference)
        &&close_complex(lhs.coefficients[i].near_residual,
                         rhs.coefficients[i].near_residual));
}

void qualify_case(int L,int family,int polarity) {
  ftd::eft::ConnectedMooreBlockOptions action;
  action.binding_law=ftd::eft::ConnectedBindingLaw::DerivedCompactPair;
  const ftd::Vec3 offset=polarity>0?ftd::Vec3{0.21,-0.17,0.29}
      :ftd::Vec3{-0.31,0.23,-0.11};
  const auto preparation=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_pair(L,offset,direction_for(family),polarity),
      action,4,1e-13,4096,true);
  const std::string label="L="+std::to_string(L)
      +" f="+std::to_string(family)+" p="+std::to_string(polarity);
  check(label+" preparation",preparation.valid);
  if(!preparation.valid) return;
  auto state=preparation.state;
  add_challenge(state,polarity);
  TransportedChartMorphologyOptions options;
  options.near_radius=L==17?4:6;
  options.outer_radius=L==17?6:12;
  options.longitudinal_direction=direction_for(family);
  options.modes=ftd::eft::make_transport_modes(
      basis_for(family),L==17?std::vector<int>{1,2,4}
                             :std::vector<int>{1,2,4,8});
  const auto cpu=ftd::eft::observe_transported_chart_morphology(
      state,action,options);
  ftd::eft::CudaTransportedChartMorphologyTelemetry telemetry;
  const auto gpu=ftd::eft::observe_transported_chart_morphology_cuda(
      state,action,options,&telemetry);
  compare_cpu_cuda(label,cpu,gpu,telemetry);
  check(label+" zero mode direct energy",
      close(cpu.actual_energy,direct_energy(state,options),1e-12));
  check(label+" energy reconstruction",
      std::abs(cpu.energy_reconstruction_residual)<=1e-12
      &&cpu.maximum_mode_reconstruction_residual<=1e-12
      &&std::abs(gpu.energy_reconstruction_residual)<=1e-12
      &&gpu.maximum_mode_reconstruction_residual<=1e-12);
  check(label+" longitudinal partition",
      cpu.longitudinal_partition_enabled
      &&gpu.longitudinal_partition_enabled
      &&close(cpu.near_longitudinal.total(),
               cpu.near_residual_energy,1e-12)
      &&close(cpu.outer_longitudinal.total(),
               cpu.outer_residual_energy,1e-12)
      &&cpu.longitudinal_partition_residual<=1e-12
      &&gpu.longitudinal_partition_residual<=1e-12);
  const auto direct=direct_partitions(state,action,options);
  check(label+" direct longitudinal scalar",
      close_partition(cpu.near_longitudinal,direct.near)
      &&close_partition(cpu.outer_longitudinal,direct.outer));

  auto reversed_options=options;
  reversed_options.longitudinal_direction=
      options.longitudinal_direction*(-1.0);
  const auto reversed=ftd::eft::observe_transported_chart_morphology(
      state,action,reversed_options);
  check(label+" direction reversal",reversed.valid
      &&close(cpu.near_longitudinal.trailing,
               reversed.near_longitudinal.leading,1e-12)
      &&close(cpu.near_longitudinal.leading,
               reversed.near_longitudinal.trailing,1e-12)
      &&close(cpu.near_longitudinal.neutral,
               reversed.near_longitudinal.neutral,1e-12)
      &&close(cpu.outer_longitudinal.trailing,
               reversed.outer_longitudinal.leading,1e-12)
      &&close(cpu.outer_longitudinal.leading,
               reversed.outer_longitudinal.trailing,1e-12)
      &&close(cpu.outer_longitudinal.neutral,
               reversed.outer_longitudinal.neutral,1e-12));

  auto symmetric=preparation.state;
  check(label+" symmetric fixture construction",
      add_longitudinal_fixture(symmetric,preparation.center,
          preparation.support_center,options.longitudinal_direction,
          options.near_radius,true,true));
  const auto symmetric_observation=
      ftd::eft::observe_transported_chart_morphology(
          symmetric,action,options);
  check(label+" symmetric fixture",symmetric_observation.valid
      &&std::abs(symmetric_observation.near_longitudinal.asymmetry())
          <=1e-12);

  auto trailing=preparation.state;
  auto leading=preparation.state;
  check(label+" directed fixture construction",
      add_longitudinal_fixture(trailing,preparation.center,
          preparation.support_center,options.longitudinal_direction,
          options.near_radius,true,false)
      &&add_longitudinal_fixture(leading,preparation.center,
          preparation.support_center,options.longitudinal_direction,
          options.near_radius,false,true));
  const auto trailing_observation=
      ftd::eft::observe_transported_chart_morphology(trailing,action,options);
  const auto leading_observation=
      ftd::eft::observe_transported_chart_morphology(leading,action,options);
  check(label+" directed fixture signs",trailing_observation.valid
      &&leading_observation.valid
      &&trailing_observation.near_longitudinal.asymmetry()>0.0
      &&leading_observation.near_longitudinal.asymmetry()<0.0);
  const auto repeated=ftd::eft::compare_transported_chart_morphology(
      cpu,cpu,1e-13);
  check(label+" repeated state",repeated.valid
      &&repeated.actual_distance<=1e-13
      &&repeated.bound_distance<=1e-13
      &&repeated.residual_distance<=1e-13
      &&repeated.near_residual_distance<=1e-13);

  const auto translated=ftd::eft::observe_transported_chart_morphology(
      translate_state(state,2,-1,3),action,options);
  const auto translation=ftd::eft::compare_transported_chart_morphology(
      cpu,translated,1e-13);
  check(label+" integer translation",translation.valid
      &&translation.actual_distance<=1e-11
      &&translation.bound_distance<=1e-11
      &&translation.residual_distance<=1e-11
      &&translation.near_residual_distance<=1e-11);

  const auto conjugated=ftd::eft::observe_transported_chart_morphology(
      conjugate_state(state),action,options);
  compare_symmetry(label+" conjugation",cpu,conjugated);

  std::array<std::array<int,3>,3> rotated_basis=basis_for(family);
  for(auto& mode:rotated_basis)
    mode={mode[2],mode[0],mode[1]};
  auto rotated_options=options;
  rotated_options.longitudinal_direction={
      options.longitudinal_direction.z,
      options.longitudinal_direction.x,
      options.longitudinal_direction.y};
  rotated_options.modes=ftd::eft::make_transport_modes(
      rotated_basis,L==17?std::vector<int>{1,2,4}
                         :std::vector<int>{1,2,4,8});
  const auto rotated=ftd::eft::observe_transported_chart_morphology(
      rotate_state(state),action,rotated_options);
  compare_symmetry(label+" cubic rotation",cpu,rotated);
}

}  // namespace

int main() {
  for(const int L:{17,33}) for(const int family:{0,1,2})
    for(const int polarity:{-1,+1}) qualify_case(L,family,polarity);
  std::cout<<"FTD-0764 transported-chart CUDA qualification failures="
           <<failures<<'\n';
  return failures==0?0:1;
}
