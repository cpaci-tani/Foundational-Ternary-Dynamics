/** FTD-0768: long-transport paired dynamic-response campaign on CUDA. */

#pragma push_macro("main")
#undef main
#define main ftd0766_reference_main
#include "campaign_aged_wake_entrainment_cuda.cpp"
#undef main
#pragma pop_macro("main")

#include "ftd/eft/cuda_paired_field_response.h"

#include <cstdint>
#include <cstring>

namespace {

using namespace ftd;
using namespace ftd::eft;

constexpr char kFtd0768ProtocolSha256[] =
    "5E4D0E9A81BD8C7E901A765792284E1BEF64129791CC874357D22F9630A2F48F";
constexpr int kFtd0768Volume=321;
constexpr int kFtd0768Age=128;
constexpr int kFtd0768Ticks=768;
constexpr int kFtd0768Stride=64;
constexpr double kFtd0768Boost=0.030;
constexpr double kFtd0768Gate=1e-12;
constexpr double kFtd0768RegionalGate=1e-10;
constexpr double kFtd0768ReverseGate=1e-10;

bool response_checkpoint(int tau) {
  return tau>=0&&tau<=kFtd0768Ticks&&tau%kFtd0768Stride==0;
}

double kinetic_energy(const ConnectedMooreBlockState& state,
                      const ConnectedMooreBlockOptions& options) {
  long double result=0.0L;
  for(const auto& point:state.constituents)
    result+=options.constituent_mass_scale
        *production_flat_energy_from_momentum(point.momentum);
  return static_cast<double>(result);
}

double complete_energy(const ConnectedMooreBlockState& state,
                       const ConnectedMooreBlockOptions& options,
                       double interaction_scale) {
  return kinetic_energy(state,options)
      +connected_moore_block_binding_energy(state,options)
      +interaction_scale*matched_modified_energy(
          state.electric,state.magnetic_half,
          options.wave_speed*options.dt);
}

struct RegionalCumulative {
  double boundary_transport=0.0;
  double boundary_transport_complement=0.0;
  double source_exchange=0.0;
  double energy_change=0.0;
  double mask_sweep=0.0;
  double mask_sweep_complement=0.0;
  double initial_region_energy=0.0;
  double endpoint_region_energy=0.0;
  double maximum_ledger_residual=0.0;
  double maximum_global_source_free_residual=0.0;
  double maximum_boundary_quadrature_residual=0.0;
  double maximum_mask_sweep_quadrature_residual=0.0;
  double maximum_transport_identity_residual=0.0;
  double maximum_endpoint_chain_residual=0.0;
  bool initialized=false;

  void add(const RegionalModifiedEnergyTransportObservation& value,
           const RegionalControlVolumeTransportObservation& transport) {
    if(!initialized) {
      initial_region_energy=transport.previous_energy_before;
      initialized=true;
    } else {
      maximum_endpoint_chain_residual=std::max(
          maximum_endpoint_chain_residual,
          std::abs(transport.previous_energy_before-endpoint_region_energy));
    }
    boundary_transport+=value.boundary_transport_into;
    boundary_transport_complement+=
        value.boundary_transport_into_complement;
    source_exchange+=value.source_exchange_into_field;
    energy_change+=value.energy_change;
    mask_sweep+=transport.mask_sweep_into;
    mask_sweep_complement+=transport.mask_sweep_into_complement;
    endpoint_region_energy=transport.current_energy_after;
    maximum_ledger_residual=std::max(maximum_ledger_residual,
                                     std::abs(value.ledger_residual));
    maximum_global_source_free_residual=std::max(
        maximum_global_source_free_residual,
        std::abs(value.global_source_free_residual));
    maximum_boundary_quadrature_residual=std::max(
        maximum_boundary_quadrature_residual,
        std::abs(value.boundary_quadrature_residual));
    maximum_mask_sweep_quadrature_residual=std::max(
        maximum_mask_sweep_quadrature_residual,
        std::abs(transport.mask_sweep_quadrature_residual));
    maximum_transport_identity_residual=std::max(
        maximum_transport_identity_residual,
        std::abs(transport.transport_identity_residual));
  }

  double accumulated_residual() const {
    return energy_change-boundary_transport-source_exchange;
  }

  double accumulated_boundary_quadrature_residual() const {
    return boundary_transport+boundary_transport_complement;
  }

  double transported_energy_change() const {
    return initialized?endpoint_region_energy-initial_region_energy:0.0;
  }

  double accumulated_transport_identity_residual() const {
    return transported_energy_change()-energy_change-mask_sweep;
  }

  double accumulated_transport_ledger_residual() const {
    return transported_energy_change()-boundary_transport-source_exchange
        -mask_sweep;
  }

  double accumulated_mask_sweep_quadrature_residual() const {
    return mask_sweep+mask_sweep_complement;
  }

  double transport_scale() const {
    return std::max({1.0,std::abs(initial_region_energy),
        std::abs(endpoint_region_energy),std::abs(transported_energy_change()),
        std::abs(boundary_transport),std::abs(source_exchange),
        std::abs(mask_sweep)});
  }

  bool transport_valid() const {
    const double gate=kFtd0768RegionalGate*transport_scale();
    return initialized
        &&std::abs(accumulated_transport_identity_residual())<=gate
        &&std::abs(accumulated_transport_ledger_residual())<=gate
        &&std::abs(accumulated_mask_sweep_quadrature_residual())<=gate
        &&maximum_mask_sweep_quadrature_residual<=gate
        &&maximum_transport_identity_residual<=gate
        &&maximum_endpoint_chain_residual<=gate;
  }
};

struct ArmCumulative {
  RegionalCumulative laboratory;
  RegionalCumulative moving_near;
  double matter_work=0.0;
  double field_work=0.0;
  double current_work=0.0;
  double maximum_common_residual=0.0;
  double maximum_energy_residual=0.0;
  double maximum_speed_excess=0.0;
  double minimum_sigma=INFINITY;
  double maximum_condition=0.0;
  double maximum_inverse_residual=0.0;
  double minimum_graph_margin=INFINITY;
  double minimum_energy_margin=INFINITY;
  int site_hops=0;
  bool valid=true;
};

struct LongResponseStep {
  bool valid=false;
  bool common=false;
  bool member=false;
  bool regularity_measured=false;
  bool inverse_valid=false;
  CudaPairedFieldResponseTelemetry laboratory_telemetry;
  CudaPairedFieldResponseTelemetry near_telemetry;
  CudaPairedFieldResponseTelemetry previous_near_telemetry;
  RegionalModifiedEnergyTransportObservation laboratory;
  RegionalModifiedEnergyTransportObservation moving_near;
  RegionalModifiedEnergyTransportObservation previous_moving_near;
  RegionalControlVolumeTransportObservation laboratory_transport;
  RegionalControlVolumeTransportObservation moving_near_transport;
  Vec3 local_before{},local_after{};
  Vec3 spline_before{},spline_after{};
  Vec3 matter_before{},matter_after{};
  double graph_margin=-INFINITY;
  double energy_margin=-INFINITY;
  double matter_work=INFINITY;
  double field_work=INFINITY;
  double current_work=INFINITY;
  double common_residual=INFINITY;
  double energy_residual=INFINITY;
  double speed_excess=INFINITY;
  double sigma_min=0.0;
  double condition=INFINITY;
  double inverse_residual=INFINITY;
  int site_hops=0;
};

bool discrete_state_equal(const ConnectedMooreBlockState& lhs,
                          const ConnectedMooreBlockState& rhs);

void scale_regional_energy(RegionalModifiedEnergyTransportObservation& value,
                           double scale) {
  value.energy_before*=scale;
  value.energy_pre_current*=scale;
  value.energy_after*=scale;
  value.outside_energy_before*=scale;
  value.outside_energy_pre_current*=scale;
  value.outside_energy_after*=scale;
  value.boundary_transport_into*=scale;
  value.boundary_transport_into_complement*=scale;
  value.source_exchange_into_field*=scale;
  value.energy_change*=scale;
  value.global_source_free_residual*=scale;
  value.boundary_quadrature_residual*=scale;
  value.ledger_residual*=scale;
}

class LongResponseCudaStepper {
 public:
  LongResponseCudaStepper(ConnectedMooreBlockState initial,
                          ConnectedMooreBlockOptions options,
                          double interaction_scale,
                          FieldResponseRegionSpec laboratory)
      : state_(std::move(initial)),options_(std::move(options)),
        interaction_scale_(interaction_scale),laboratory_(laboratory),
        pipeline_(state_.electric.L),prepared_b_(state_.electric.L),
        prepared_e_(state_.electric.L) {
    const double c=static_cast<double>(state_.electric.L/2);
    diagnostic_center_={c,c,c};
    previous_near_.kind=FieldResponseRegionKind::ChebyshevCube;
    previous_near_.center=object_center(state_);
    previous_near_.chebyshev_radius=8.0;
    options_.defer_volume_diagnostics=true;
    valid_=pipeline_.valid()
        &&pipeline_.upload(state_.electric,state_.magnetic_half);
  }

  bool valid() const { return valid_; }
  const ConnectedMooreBlockState& state() const { return state_; }
  ConnectedMooreBlockState release_state() { return std::move(state_); }

  LongResponseStep advance(bool checkpoint) {
    LongResponseStep result;
    if(!valid_) return result;
    options_.measure_final_root_regularity=checkpoint;
    const double lambda=options_.wave_speed*options_.dt;
    if(!pipeline_.prepare_forward(lambda)
        ||!pipeline_.download_prepared(prepared_b_,prepared_e_)) {
      valid_=false; return result;
    }
    auto step=solve_connected_moore_block_forward_prepared(
        state_,std::move(prepared_b_),std::move(prepared_e_),options_,
        &forward_cache_);
    if(!step.volume_diagnostics_pending
        ||!pipeline_.apply_ordered_sparse_current(
            step.segments,options_.polarity_scale)) {
      valid_=false; return result;
    }
    FieldResponseRegionSpec near;
    near.kind=FieldResponseRegionKind::ChebyshevCube;
    near.center=object_center(step.later);
    near.chebyshev_radius=8.0;
    const auto views=pipeline_.resident_views();
    result.laboratory=observe_regional_modified_energy_transport_cuda(
        views,lambda,laboratory_,kFtd0768Gate,&result.laboratory_telemetry);
    result.moving_near=observe_regional_modified_energy_transport_cuda(
        views,lambda,near,kFtd0768Gate,&result.near_telemetry);
    result.previous_moving_near=
        observe_regional_modified_energy_transport_cuda(
            views,lambda,previous_near_,kFtd0768Gate,
            &result.previous_near_telemetry);
    scale_regional_energy(result.laboratory,interaction_scale_);
    scale_regional_energy(result.moving_near,interaction_scale_);
    scale_regional_energy(result.previous_moving_near,interaction_scale_);
    result.laboratory_transport=derive_regional_control_volume_transport(
        result.laboratory,result.laboratory,kFtd0768Gate);
    result.moving_near_transport=derive_regional_control_volume_transport(
        result.previous_moving_near,result.moving_near,kFtd0768Gate);
    const auto profile=pipeline_.observe_deterministic(
        lambda,diagnostic_center_,{8},kFtd0768Gate);
    if(!result.laboratory.valid||!result.moving_near.valid
        ||!result.previous_moving_near.valid
        ||!result.laboratory_transport.valid
        ||!result.moving_near_transport.valid
        ||!result.laboratory_telemetry.valid||!result.near_telemetry.valid
        ||!result.previous_near_telemetry.valid
        ||result.laboratory_telemetry.complete_field_downloads!=0
        ||result.near_telemetry.complete_field_downloads!=0
        ||result.previous_near_telemetry.complete_field_downloads!=0
        ||!profile.valid) {
      valid_=false; return result;
    }
    const auto diagnostics=pipeline_.diagnose_common_action(
        step.segments,options_.polarity_scale,interaction_scale_,
        options_.wave_speed,options_.dt,kFtd0768Gate);
    step=complete_connected_moore_block_volume_diagnostics(
        std::move(step),diagnostics,options_);
    result.valid=step.valid&&step.common_action_gates_pass;
    result.common=step.common_action_gates_pass;
    result.local_before=step.local_field_momentum_before;
    result.local_after=step.local_field_momentum_after;
    result.spline_before=step.spline_field_momentum_before;
    result.spline_after=step.spline_field_momentum_after;
    result.matter_before=step.matter_momentum_before;
    result.matter_after=step.matter_momentum_after;
    result.matter_work=(step.kinetic_energy_after+step.binding_energy_after)
        -(step.kinetic_energy_before+step.binding_energy_before);
    result.field_work=step.field_energy_after-step.field_energy_before;
    result.current_work=step.current_work;
    result.common_residual=common_residual_0764(step);
    result.energy_residual=std::abs(step.total_energy_residual);
    result.speed_excess=step.causal_speed_excess;
    result.regularity_measured=step.solve.final_root_regularity_measured;
    result.sigma_min=step.solve.final_minimum_singular_value;
    result.condition=step.solve.final_condition_number;
    result.site_hops=step.site_hops;
    const auto core=observe_support_invariant_matter(step.later,options_);
    result.member=core.valid&&core.member;
    result.graph_margin=core.graph_margin;
    result.energy_margin=core.energy_margin;
    if(checkpoint&&result.valid) {
      auto reverse_options=options_;
      reverse_options.defer_volume_diagnostics=false;
      reverse_options.measure_final_root_regularity=false;
      ConnectedMooreBlockSolveCache reverse_cache;
      const auto reverse=solve_connected_moore_block_reverse(
          step.later,reverse_options,&reverse_cache);
      result.inverse_valid=reverse.valid&&reverse.common_action_gates_pass
          &&discrete_state_equal(state_,reverse.earlier);
      if(result.inverse_valid)
        result.inverse_residual=connected_moore_block_state_max_difference(
            state_,reverse.earlier);
    } else {
      result.inverse_valid=true;
      result.inverse_residual=0.0;
    }
    state_=std::move(step.later);
    previous_near_=near;
    if(!pipeline_.advance()) valid_=false;
    valid_=valid_&&result.valid&&result.member;
    return result;
  }

 private:
  ConnectedMooreBlockState state_;
  ConnectedMooreBlockOptions options_;
  double interaction_scale_=0.0;
  FieldResponseRegionSpec laboratory_{};
  FieldResponseRegionSpec previous_near_{};
  Vec3 diagnostic_center_{};
  CudaMatchedFieldPipeline pipeline_;
  MatchedEdgeField prepared_b_;
  MatchedFaceFlux prepared_e_;
  ConnectedMooreBlockSolveCache forward_cache_;
  bool valid_=false;
};

void accumulate(ArmCumulative& cumulative,const LongResponseStep& step) {
  cumulative.laboratory.add(step.laboratory,step.laboratory_transport);
  cumulative.moving_near.add(step.moving_near,step.moving_near_transport);
  cumulative.matter_work+=step.matter_work;
  cumulative.field_work+=step.field_work;
  cumulative.current_work+=step.current_work;
  cumulative.maximum_common_residual=std::max(
      cumulative.maximum_common_residual,step.common_residual);
  cumulative.maximum_energy_residual=std::max(
      cumulative.maximum_energy_residual,step.energy_residual);
  cumulative.maximum_speed_excess=std::max(
      cumulative.maximum_speed_excess,step.speed_excess);
  cumulative.minimum_graph_margin=std::min(
      cumulative.minimum_graph_margin,step.graph_margin);
  cumulative.minimum_energy_margin=std::min(
      cumulative.minimum_energy_margin,step.energy_margin);
  cumulative.site_hops+=step.site_hops;
  if(step.regularity_measured) {
    cumulative.minimum_sigma=std::min(cumulative.minimum_sigma,
                                      step.sigma_min);
    cumulative.maximum_condition=std::max(cumulative.maximum_condition,
                                          step.condition);
  }
  cumulative.maximum_inverse_residual=std::max(
      cumulative.maximum_inverse_residual,step.inverse_residual);
  cumulative.valid=cumulative.valid&&step.valid&&step.common&&step.member
      &&step.common_residual<=kFtd0768Gate
      &&step.energy_residual<=kFtd0768Gate
      &&step.speed_excess<=kFtd0768Gate
      &&step.graph_margin>=1e-6&&step.energy_margin>=1e-6
      &&cumulative.laboratory.transport_valid()
      &&cumulative.moving_near.transport_valid()
      &&(!step.regularity_measured
         ||(step.sigma_min>=1e-3&&step.condition<=1e4
            &&step.inverse_valid
            &&step.inverse_residual<=kFtd0768Gate));
}

std::uint64_t fnv_mix(std::uint64_t hash,std::uint64_t value) {
  hash^=value;
  return hash*1099511628211ULL;
}

std::uint64_t double_bits(double value) {
  std::uint64_t result=0;
  static_assert(sizeof(result)==sizeof(value));
  std::memcpy(&result,&value,sizeof(result));
  return result;
}

std::string state_hash(const ConnectedMooreBlockState& state) {
  std::uint64_t hash=1469598103934665603ULL;
  hash=fnv_mix(hash,static_cast<std::uint64_t>(state.electric.L));
  hash=fnv_mix(hash,static_cast<std::uint64_t>(state.width));
  hash=fnv_mix(hash,static_cast<std::uint64_t>(state.orientation_axis));
  for(const auto& point:state.constituents) {
    hash=fnv_mix(hash,static_cast<std::uint64_t>(point.anchor.x));
    hash=fnv_mix(hash,static_cast<std::uint64_t>(point.anchor.y));
    hash=fnv_mix(hash,static_cast<std::uint64_t>(point.anchor.z));
    for(const double value:{point.remainder.x,point.remainder.y,
                            point.remainder.z,point.momentum.x,
                            point.momentum.y,point.momentum.z})
      hash=fnv_mix(hash,double_bits(value));
  }
  for(const int charge:state.charges)
    hash=fnv_mix(hash,static_cast<std::uint64_t>(charge));
  for(const auto& edge:state.edges) {
    hash=fnv_mix(hash,static_cast<std::uint64_t>(edge.first));
    hash=fnv_mix(hash,static_cast<std::uint64_t>(edge.second));
    hash=fnv_mix(hash,static_cast<std::uint64_t>(edge.reference_delta.x));
    hash=fnv_mix(hash,static_cast<std::uint64_t>(edge.reference_delta.y));
    hash=fnv_mix(hash,static_cast<std::uint64_t>(edge.reference_delta.z));
    hash=fnv_mix(hash,double_bits(edge.rest_length_squared));
  }
  for(const auto* values:{&state.electric.x,&state.electric.y,
                          &state.electric.z,&state.magnetic_half.x,
                          &state.magnetic_half.y,&state.magnetic_half.z})
    for(const double value:*values) hash=fnv_mix(hash,double_bits(value));
  std::ostringstream out;
  out<<std::hex<<std::setw(16)<<std::setfill('0')<<hash;
  return out.str();
}

bool discrete_state_equal(const ConnectedMooreBlockState& lhs,
                          const ConnectedMooreBlockState& rhs) {
  if(lhs.electric.L!=rhs.electric.L||lhs.charges!=rhs.charges
      ||lhs.constituents.size()!=rhs.constituents.size()
      ||lhs.edges.size()!=rhs.edges.size()||lhs.width!=rhs.width
      ||lhs.orientation_axis!=rhs.orientation_axis) return false;
  for(std::size_t i=0;i<lhs.constituents.size();++i) {
    const auto& a=lhs.constituents[i].anchor;
    const auto& b=rhs.constituents[i].anchor;
    if(a.x!=b.x||a.y!=b.y||a.z!=b.z) return false;
  }
  for(std::size_t i=0;i<lhs.edges.size();++i) {
    const auto& a=lhs.edges[i];
    const auto& b=rhs.edges[i];
    if(a.first!=b.first||a.second!=b.second
        ||a.reference_delta.x!=b.reference_delta.x
        ||a.reference_delta.y!=b.reference_delta.y
        ||a.reference_delta.z!=b.reference_delta.z) return false;
  }
  return true;
}

bool finite_vec(const Vec3& value) {
  return std::isfinite(value.x)&&std::isfinite(value.y)
      &&std::isfinite(value.z);
}

struct LongCheckpoint {
  int tau=0;
  bool valid=false;
  bool clearing=false;
  Vec3 rest_center{},moving_center{};
  double displacement=0.0;
  std::string rest_state_hash,moving_state_hash;
  double rest_kinetic=0.0,rest_binding=0.0,rest_field=0.0;
  double moving_kinetic=0.0,moving_binding=0.0,moving_field=0.0;
  Vec3 rest_matter_p{},moving_matter_p{};
  Vec3 rest_local_p{},moving_local_p{};
  Vec3 rest_spline_p{},moving_spline_p{};
  double rest_local_defect=0.0,moving_local_defect=0.0;
  double rest_spline_defect=0.0,moving_spline_defect=0.0;
  ArmCumulative rest_cumulative,moving_cumulative;
  PairedFieldResponseObservation paired;
  CudaPairedFieldResponseTelemetry paired_telemetry;
};

struct LongTransportCampaign {
  bool parent_valid=false;
  bool aging_valid=false;
  bool rest_initialized=false;
  bool moving_initialized=false;
  bool forward_valid=false;
  double maximum_rest_displacement=0.0;
  bool boundary_clear=true;
  double boundary_margin=-INFINITY;
  double interaction_scale=0.0;
  double initial_pair_energy_scale=0.0;
  Vec3 laboratory_center{};
  std::string moving_initial_hash;
  std::string moving_forward_final_hash;
  std::string moving_reversed_hash;
  bool reverse_valid=false;
  bool reverse_discrete_exact=false;
  double reverse_recovery=INFINITY;
  double reverse_maximum_common=0.0;
  int reverse_steps=0;
  bool clearing_reached=false;
  int first_clearing_tau=-1;
  std::string outcome="LONG_TRANSPORT_EXECUTION_INVALID";
  std::vector<LongCheckpoint> checkpoints;
};

LongCheckpoint make_checkpoint(
    int tau,const ConnectedMooreBlockState& rest,
    const ConnectedMooreBlockState& moving,
    const ConnectedMooreBlockOptions& action,double interaction_scale,
    const Vec3& laboratory_center,const Vec3& direction,
    const ArmCumulative& rest_cumulative,
    const ArmCumulative& moving_cumulative,
    const Vec3& rest_local_p,const Vec3& moving_local_p,
    const Vec3& rest_spline_p,const Vec3& moving_spline_p,
    const Vec3& rest_initial_local_total,
    const Vec3& moving_initial_local_total,
    const Vec3& rest_initial_spline_total,
    const Vec3& moving_initial_spline_total) {
  LongCheckpoint result;
  result.tau=tau;
  result.rest_center=object_center(rest);
  result.moving_center=object_center(moving);
  result.displacement=(result.moving_center-laboratory_center).dot(direction);
  result.clearing=result.displacement>=9.0;
  result.rest_state_hash=state_hash(rest);
  result.moving_state_hash=state_hash(moving);
  result.rest_kinetic=kinetic_energy(rest,action);
  result.rest_binding=connected_moore_block_binding_energy(rest,action);
  result.rest_field=interaction_scale*matched_modified_energy(
      rest.electric,rest.magnetic_half,action.wave_speed*action.dt);
  result.moving_kinetic=kinetic_energy(moving,action);
  result.moving_binding=connected_moore_block_binding_energy(moving,action);
  result.moving_field=interaction_scale*matched_modified_energy(
      moving.electric,moving.magnetic_half,action.wave_speed*action.dt);
  result.rest_matter_p=matter_momentum(rest);
  result.moving_matter_p=matter_momentum(moving);
  result.rest_local_p=rest_local_p;
  result.moving_local_p=moving_local_p;
  result.rest_spline_p=rest_spline_p;
  result.moving_spline_p=moving_spline_p;
  result.rest_local_defect=(result.rest_matter_p+result.rest_local_p
      -rest_initial_local_total).mag();
  result.moving_local_defect=(result.moving_matter_p+result.moving_local_p
      -moving_initial_local_total).mag();
  result.rest_spline_defect=(result.rest_matter_p+result.rest_spline_p
      -rest_initial_spline_total).mag();
  result.moving_spline_defect=(result.moving_matter_p+result.moving_spline_p
      -moving_initial_spline_total).mag();
  result.rest_cumulative=rest_cumulative;
  result.moving_cumulative=moving_cumulative;
  PairedFieldResponseOptions observer;
  observer.laboratory_center=laboratory_center;
  observer.moving_center=result.moving_center;
  observer.longitudinal=direction;
  observer.transverse_u={1,0,0};
  observer.transverse_v={0,1,0};
  observer.wave_speed=action.wave_speed;
  observer.dt=action.dt;
  result.paired=observe_paired_field_response_cuda(
      moving,rest,action,observer,&result.paired_telemetry);
  result.valid=result.paired.valid&&result.paired_telemetry.valid
      &&result.paired_telemetry.complete_field_downloads==0
      &&result.paired.maximum_energy_identity_residual<=kFtd0768Gate
      &&rest_cumulative.valid&&moving_cumulative.valid
      &&finite_vec(result.rest_matter_p)&&finite_vec(result.moving_matter_p)
      &&finite_vec(result.rest_local_p)&&finite_vec(result.moving_local_p)
      &&finite_vec(result.rest_spline_p)&&finite_vec(result.moving_spline_p)
      &&std::isfinite(result.rest_local_defect)
      &&std::isfinite(result.moving_local_defect)
      &&std::isfinite(result.rest_spline_defect)
      &&std::isfinite(result.moving_spline_defect);
  return result;
}

bool monotone_nonincreasing(const std::vector<double>& values) {
  if(values.size()<2) return false;
  for(std::size_t i=1;i<values.size();++i)
    if(values[i]>values[i-1]+1e-15*std::max(1.0,values[i-1])) return false;
  return true;
}

void classify(LongTransportCampaign& result) {
  const bool execution=result.parent_valid&&result.aging_valid
      &&result.rest_initialized&&result.moving_initialized
      &&result.forward_valid&&result.boundary_clear&&result.reverse_valid
      &&result.reverse_discrete_exact
      &&result.reverse_recovery<=kFtd0768ReverseGate
      &&result.reverse_steps==kFtd0768Ticks
      &&result.checkpoints.size()==13
      &&std::all_of(result.checkpoints.begin(),result.checkpoints.end(),
          [](const auto& value){return value.valid;});
  if(!execution) {
    result.outcome="LONG_TRANSPORT_EXECUTION_INVALID";
    return;
  }
  const auto first=std::find_if(result.checkpoints.begin(),
      result.checkpoints.end(),[](const auto& value){return value.clearing;});
  result.clearing_reached=first!=result.checkpoints.end();
  result.first_clearing_tau=result.clearing_reached?first->tau:-1;
  if(!result.clearing_reached) {
    result.outcome="CORE_CLEARING_NOT_REACHED";
    return;
  }
  std::vector<double> delta_u,norm;
  for(auto it=first;it!=result.checkpoints.end();++it) {
    delta_u.push_back(std::abs(it->paired.regions[0]
        .actual.energy_difference));
    norm.push_back(it->paired.regions[0]
        .actual.difference_field_energy);
  }
  if(delta_u.size()<2) {
    result.outcome="CORE_CLEARING_REACHED_RESPONSE_UNRESOLVED";
    return;
  }
  const double floor=1e-6*result.initial_pair_energy_scale;
  const bool decays=monotone_nonincreasing(delta_u)
      &&monotone_nonincreasing(norm);
  const bool persistent_delta=std::all_of(delta_u.begin(),delta_u.end(),
      [floor](double value){return value>=floor;});
  const bool persistent_norm=std::all_of(norm.begin(),norm.end(),
      [floor](double value){return value>=floor;});
  if(decays) result.outcome="CLEARED_LOCAL_RESPONSE_DECAYS";
  else if(persistent_delta||persistent_norm)
    result.outcome="CLEARED_LOCAL_RESPONSE_PERSISTS";
  else result.outcome="CLEARED_LOCAL_RESPONSE_MIXED";
}

LongTransportCampaign run_long_transport_campaign() {
  LongTransportCampaign result;
  ForensicDirection direction_record;
  if(!select_direction("face",direction_record)) return result;
  const Vec3 direction=direction_unit(direction_record);
  const auto normalization=measure_face_flux_normalization();
  if(!normalization.valid) return result;
  result.interaction_scale=normalization.mapped_field_work_coefficient;
  auto action=forensic_options();
  auto parent=build_parent(kFtd0768Volume,direction_record,action,
      result.interaction_scale);
  result.parent_valid=parent.valid;
  if(!parent.valid) return result;

  auto aged_state=std::move(parent.state);
  {
    MorphologyCudaStepper aging(std::move(aged_state),action,
                                result.interaction_scale);
    if(!aging.valid()) return result;
    result.aging_valid=true;
    for(int tick=1;tick<=kFtd0768Age;++tick) {
      const auto step=aging.advance(tick==kFtd0768Age);
      result.aging_valid=result.aging_valid&&step.valid&&step.common
          &&step.common_residual<=kFtd0768Gate
          &&step.energy_residual<=kFtd0768Gate
          &&step.speed_excess<=kFtd0768Gate
          &&(tick!=kFtd0768Age
             ||(step.regularity_measured&&step.sigma_min>=1e-3
                &&step.condition<=1e4&&step.inverse_valid
                &&step.inverse_residual<=kFtd0768Gate));
      if(!step.valid) break;
    }
    aged_state=aging.release_state();
  }
  if(!result.aging_valid) return result;
  auto rest_initial=aged_state;
  auto moving_initial=aged_state;
  for(auto& point:moving_initial.constituents)
    point.momentum+=direction*kFtd0768Boost;
  const auto rest_core=observe_support_invariant_matter(rest_initial,action);
  const auto moving_core=observe_support_invariant_matter(moving_initial,action);
  result.rest_initialized=rest_core.valid&&rest_core.member
      &&rest_core.graph_margin>=1e-6&&rest_core.energy_margin>=1e-6;
  result.moving_initialized=moving_core.valid&&moving_core.member
      &&moving_core.graph_margin>=1e-6&&moving_core.energy_margin>=1e-6;
  if(!result.rest_initialized||!result.moving_initialized) return result;
  result.laboratory_center=object_center(moving_initial);
  result.initial_pair_energy_scale=std::abs(
      complete_energy(moving_initial,action,result.interaction_scale)
      -complete_energy(rest_initial,action,result.interaction_scale));
  result.moving_initial_hash=state_hash(moving_initial);
  const Vec3 rest_local_initial=matched_local_translation_momentum(
      rest_initial.electric,rest_initial.magnetic_half)
      *result.interaction_scale;
  const Vec3 moving_local_initial=matched_local_translation_momentum(
      moving_initial.electric,moving_initial.magnetic_half)
      *result.interaction_scale;
  const auto rest_spline_initial_record=measure_spline_poynting_momentum(
      rest_initial.electric,rest_initial.magnetic_half,
      action.wave_speed,action.dt,result.interaction_scale);
  const auto moving_spline_initial_record=measure_spline_poynting_momentum(
      moving_initial.electric,moving_initial.magnetic_half,
      action.wave_speed,action.dt,result.interaction_scale);
  if(!finite_vec(rest_local_initial)||!finite_vec(moving_local_initial)
      ||!rest_spline_initial_record.valid
      ||!moving_spline_initial_record.valid) return result;
  const Vec3 rest_initial_local_total=
      matter_momentum(rest_initial)+rest_local_initial;
  const Vec3 moving_initial_local_total=
      matter_momentum(moving_initial)+moving_local_initial;
  const Vec3 rest_initial_spline_total=
      matter_momentum(rest_initial)+rest_spline_initial_record.momentum;
  const Vec3 moving_initial_spline_total=
      matter_momentum(moving_initial)+moving_spline_initial_record.momentum;
  const double causal_reach=(160+kFtd0768Age+kFtd0768Ticks)
      *action.wave_speed*action.dt;
  result.boundary_margin=0.5*kFtd0768Volume-4.0-causal_reach;
  result.boundary_clear=result.boundary_margin>0.0;

  PairedFieldResponseOptions observer;
  observer.laboratory_center=result.laboratory_center;
  observer.moving_center=result.laboratory_center;
  observer.longitudinal=direction;
  observer.transverse_u={1,0,0};
  observer.transverse_v={0,1,0};
  const auto laboratory=make_ftd0768_response_regions(observer)[0];
  ArmCumulative rest_cumulative,moving_cumulative;
  rest_cumulative.minimum_graph_margin=rest_core.graph_margin;
  rest_cumulative.minimum_energy_margin=rest_core.energy_margin;
  moving_cumulative.minimum_graph_margin=moving_core.graph_margin;
  moving_cumulative.minimum_energy_margin=moving_core.energy_margin;
  result.checkpoints.push_back(make_checkpoint(
      0,rest_initial,moving_initial,action,result.interaction_scale,
      result.laboratory_center,direction,rest_cumulative,moving_cumulative,
      rest_local_initial,moving_local_initial,
      rest_spline_initial_record.momentum,
      moving_spline_initial_record.momentum,
      rest_initial_local_total,moving_initial_local_total,
      rest_initial_spline_total,moving_initial_spline_total));

  ConnectedMooreBlockState rest_final,moving_final;
  {
    auto rest=std::make_unique<LongResponseCudaStepper>(
        std::move(rest_initial),action,result.interaction_scale,laboratory);
    auto moving=std::make_unique<LongResponseCudaStepper>(
        moving_initial,action,result.interaction_scale,laboratory);
    if(!rest->valid()||!moving->valid()) return result;
    result.forward_valid=true;
    for(int tau=1;tau<=kFtd0768Ticks;++tau) {
      const bool checkpoint=response_checkpoint(tau);
      const auto rest_step=rest->advance(checkpoint);
      const auto moving_step=moving->advance(checkpoint);
      accumulate(rest_cumulative,rest_step);
      accumulate(moving_cumulative,moving_step);
      result.maximum_rest_displacement=std::max(
          result.maximum_rest_displacement,
          (object_center(rest->state())-result.laboratory_center).mag());
      result.forward_valid=result.forward_valid&&rest_cumulative.valid
          &&moving_cumulative.valid
          &&result.maximum_rest_displacement<=kFtd0768Gate;
      if(!rest_step.valid||!moving_step.valid) break;
      if(checkpoint) {
        auto record=make_checkpoint(tau,rest->state(),moving->state(),action,
            result.interaction_scale,result.laboratory_center,direction,
            rest_cumulative,moving_cumulative,
            rest_step.local_after,moving_step.local_after,
            rest_step.spline_after,moving_step.spline_after,
            rest_initial_local_total,moving_initial_local_total,
            rest_initial_spline_total,moving_initial_spline_total);
        result.forward_valid=result.forward_valid&&record.valid
            &&(record.rest_center-result.laboratory_center).mag()<=1e-12;
        result.checkpoints.push_back(std::move(record));
        std::cout<<std::setprecision(17)
            <<"FTD-0768 forward tau="<<tau
            <<" d="<<result.checkpoints.back().displacement
            <<" valid="<<std::boolalpha<<result.forward_valid<<'\n';
      }
    }
    rest_final=rest->release_state();
    moving_final=moving->release_state();
  }
  result.forward_valid=result.forward_valid
      &&result.checkpoints.size()==13;
  result.moving_forward_final_hash=state_hash(moving_final);

  if(result.forward_valid) {
    auto reverse_state=std::move(moving_final);
    auto reverse_options=action;
    reverse_options.defer_volume_diagnostics=false;
    reverse_options.measure_final_root_regularity=false;
    ConnectedMooreBlockSolveCache reverse_cache;
    result.reverse_valid=true;
    for(int step_index=1;step_index<=kFtd0768Ticks;++step_index) {
      const auto reverse=solve_connected_moore_block_reverse(
          reverse_state,reverse_options,&reverse_cache);
      const double common=common_residual_0764(reverse);
      result.reverse_maximum_common=std::max(
          result.reverse_maximum_common,common);
      result.reverse_valid=result.reverse_valid&&reverse.valid
          &&reverse.common_action_gates_pass&&common<=kFtd0768Gate;
      if(!reverse.valid) break;
      reverse_state=std::move(reverse.earlier);
      result.reverse_steps=step_index;
      if(step_index%kFtd0768Stride==0)
        std::cout<<"FTD-0768 reverse steps="<<step_index
                 <<" valid="<<std::boolalpha<<result.reverse_valid<<'\n';
    }
    result.reverse_recovery=connected_moore_block_state_max_difference(
        moving_initial,reverse_state);
    result.reverse_discrete_exact=discrete_state_equal(
        moving_initial,reverse_state);
    result.moving_reversed_hash=state_hash(reverse_state);
    result.reverse_valid=result.reverse_valid
        &&result.reverse_steps==kFtd0768Ticks
        &&result.reverse_discrete_exact
        &&result.reverse_recovery<=kFtd0768ReverseGate;
  }
  classify(result);
  return result;
}

void write_channel(std::ostream& out,
                   const QuadraticFieldDifferenceChannel& value) {
  out<<"{\"moving_energy\": "<<json_number(value.moving_energy)
     <<", \"rest_energy\": "<<json_number(value.rest_energy)
     <<", \"energy_difference\": "<<json_number(value.energy_difference)
     <<", \"difference_field_energy\": "
     <<json_number(value.difference_field_energy)
     <<", \"cross_energy\": "<<json_number(value.cross_energy)
     <<", \"energy_identity_residual\": "
     <<json_number(value.energy_identity_residual)
     <<", \"energy_difference_first_moment\": "
     <<json_number(value.energy_difference_first_moment)
     <<", \"difference_field_first_moment\": "
     <<json_number(value.difference_field_first_moment)
     <<", \"cross_first_moment\": "
     <<json_number(value.cross_first_moment)<<'}';
}

void write_cumulative(std::ostream& out,const ArmCumulative& value) {
  const auto write_region=[&](const RegionalCumulative& region) {
    out<<"{\"boundary_transport\": "
       <<json_number(region.boundary_transport)
       <<", \"boundary_transport_complement\": "
       <<json_number(region.boundary_transport_complement)
       <<", \"source_exchange\": "<<json_number(region.source_exchange)
       <<", \"energy_change\": "<<json_number(region.energy_change)
       <<", \"mask_sweep\": "<<json_number(region.mask_sweep)
       <<", \"mask_sweep_complement\": "
       <<json_number(region.mask_sweep_complement)
       <<", \"initial_region_energy\": "
       <<json_number(region.initial_region_energy)
       <<", \"endpoint_region_energy\": "
       <<json_number(region.endpoint_region_energy)
       <<", \"transported_energy_change\": "
       <<json_number(region.transported_energy_change())
       <<", \"accumulated_residual\": "
       <<json_number(region.accumulated_residual())
       <<", \"accumulated_transport_identity_residual\": "
       <<json_number(region.accumulated_transport_identity_residual())
       <<", \"accumulated_transport_ledger_residual\": "
       <<json_number(region.accumulated_transport_ledger_residual())
       <<", \"accumulated_boundary_quadrature_residual\": "
       <<json_number(region.accumulated_boundary_quadrature_residual())
       <<", \"accumulated_mask_sweep_quadrature_residual\": "
       <<json_number(region.accumulated_mask_sweep_quadrature_residual())
       <<", \"maximum_tick_residual\": "
       <<json_number(region.maximum_ledger_residual)
       <<", \"maximum_global_source_free_residual\": "
       <<json_number(region.maximum_global_source_free_residual)
       <<", \"maximum_boundary_quadrature_residual\": "
       <<json_number(region.maximum_boundary_quadrature_residual)
       <<", \"maximum_mask_sweep_quadrature_residual\": "
       <<json_number(region.maximum_mask_sweep_quadrature_residual)
       <<", \"maximum_transport_identity_residual\": "
       <<json_number(region.maximum_transport_identity_residual)
       <<", \"maximum_endpoint_chain_residual\": "
       <<json_number(region.maximum_endpoint_chain_residual)
       <<", \"initialized\": "<<region.initialized<<'}';
  };
  out<<"{\"valid\": "<<value.valid<<", \"laboratory\": ";
  write_region(value.laboratory);
  out<<", \"moving_near\": "; write_region(value.moving_near);
  out<<", \"matter_work\": "<<json_number(value.matter_work)
     <<", \"field_work\": "<<json_number(value.field_work)
     <<", \"current_work\": "<<json_number(value.current_work)
     <<", \"maximum_common_residual\": "
     <<json_number(value.maximum_common_residual)
     <<", \"maximum_energy_residual\": "
     <<json_number(value.maximum_energy_residual)
     <<", \"maximum_speed_excess\": "
     <<json_number(value.maximum_speed_excess)
     <<", \"minimum_sigma\": "<<json_number(value.minimum_sigma)
     <<", \"maximum_condition\": "<<json_number(value.maximum_condition)
     <<", \"maximum_inverse_residual\": "
     <<json_number(value.maximum_inverse_residual)
     <<", \"minimum_graph_margin\": "
     <<json_number(value.minimum_graph_margin)
     <<", \"minimum_energy_margin\": "
     <<json_number(value.minimum_energy_margin)
     <<", \"site_hops\": "<<value.site_hops<<'}';
}

void write_checkpoint(std::ostream& out,const LongCheckpoint& value) {
  out<<"{\"tau\": "<<value.tau<<", \"valid\": "<<value.valid
     <<", \"clearing\": "<<value.clearing
     <<", \"displacement\": "<<json_number(value.displacement)
     <<", \"rest_center\": "; write_vec(out,value.rest_center);
  out<<", \"moving_center\": "; write_vec(out,value.moving_center);
  out<<", \"rest_state_hash\": \""<<value.rest_state_hash
     <<"\", \"moving_state_hash\": \""<<value.moving_state_hash
     <<"\", \"rest_energy\": {\"kinetic\": "
     <<json_number(value.rest_kinetic)<<", \"binding\": "
     <<json_number(value.rest_binding)<<", \"field\": "
     <<json_number(value.rest_field)<<"}, \"moving_energy\": {\"kinetic\": "
     <<json_number(value.moving_kinetic)<<", \"binding\": "
     <<json_number(value.moving_binding)<<", \"field\": "
     <<json_number(value.moving_field)<<"}, \"rest_matter_momentum\": ";
  write_vec(out,value.rest_matter_p);
  out<<", \"moving_matter_momentum\": "; write_vec(out,value.moving_matter_p);
  out<<", \"rest_local_momentum\": "; write_vec(out,value.rest_local_p);
  out<<", \"moving_local_momentum\": "; write_vec(out,value.moving_local_p);
  out<<", \"rest_spline_momentum\": "; write_vec(out,value.rest_spline_p);
  out<<", \"moving_spline_momentum\": "; write_vec(out,value.moving_spline_p);
  out<<", \"rest_local_momentum_defect\": "
     <<json_number(value.rest_local_defect)
     <<", \"moving_local_momentum_defect\": "
     <<json_number(value.moving_local_defect)
     <<", \"rest_spline_momentum_defect\": "
     <<json_number(value.rest_spline_defect)
     <<", \"moving_spline_momentum_defect\": "
     <<json_number(value.moving_spline_defect);
  out<<", \"rest_cumulative\": "; write_cumulative(out,value.rest_cumulative);
  out<<", \"moving_cumulative\": ";
  write_cumulative(out,value.moving_cumulative);
  out<<", \"paired_telemetry\": {\"valid\": "
     <<value.paired_telemetry.valid
     <<", \"host_to_device_bytes\": "
     <<value.paired_telemetry.host_to_device_bytes
     <<", \"device_to_host_bytes\": "
     <<value.paired_telemetry.device_to_host_bytes
     <<", \"complete_field_downloads\": "
     <<value.paired_telemetry.complete_field_downloads
     <<"}, \"maximum_energy_identity_residual\": "
     <<json_number(value.paired.maximum_energy_identity_residual)
     <<", \"regions\": [";
  for(std::size_t i=0;i<value.paired.regions.size();++i) {
    if(i) out<<',';
    out<<"{\"index\": "<<i<<", \"actual\": ";
    write_channel(out,value.paired.regions[i].actual);
    out<<", \"residual\": ";
    write_channel(out,value.paired.regions[i].residual);
    out<<'}';
  }
  out<<"]}";
}

std::filesystem::path ftd0768_results_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results"/"ftd_0768";
}

void write_long_transport_result(const LongTransportCampaign& value) {
  const auto directory=ftd0768_results_directory();
  std::filesystem::create_directories(directory);
  std::ofstream out(directory/
      "ftd_0768_long_transport_dynamic_response_v1.json");
  out<<std::boolalpha<<std::setprecision(17)
     <<"{\n  \"ftd_id\": \"FTD-0768\",\n"
     <<"  \"protocol_sha256\": \""<<kFtd0768ProtocolSha256<<"\",\n"
     <<"  \"run_record_schema\": "
       "\"ftd_0768_long_transport_dynamic_response_v1\",\n"
     <<"  \"field_representation\": "
       "\"matched_oriented_face_electric_edge_magnetic_half\",\n"
     <<"  \"observer_mode\": "
       "\"paired_actual_selected_residual_complementary_boundary_moving_control_volume_sweep\",\n"
     <<"  \"volume\": "<<kFtd0768Volume<<",\n"
     <<"  \"formation_ticks\": 160,\n"
     <<"  \"preparation_age\": "<<kFtd0768Age<<",\n"
     <<"  \"discovery_ticks\": "<<kFtd0768Ticks<<",\n"
     <<"  \"checkpoint_stride\": "<<kFtd0768Stride<<",\n"
     <<"  \"clearing_distance\": 9,\n"
     <<"  \"tolerances\": {\"common\": "
     <<json_number(kFtd0768Gate)<<", \"regional\": "
     <<json_number(kFtd0768RegionalGate)<<", \"reverse\": "
     <<json_number(kFtd0768ReverseGate)
     <<", \"minimum_root_singular_value\": 0.001, "
       "\"maximum_condition_number\": 10000, "
       "\"minimum_core_margin\": 0.000001},\n"
     <<"  \"boost\": "<<json_number(kFtd0768Boost)<<",\n"
     <<"  \"direction\": [0, 0, 1],\n"
     <<"  \"interaction_scale\": "
     <<json_number(value.interaction_scale)<<",\n"
     <<"  \"laboratory_center\": "; write_vec(out,value.laboratory_center);
  out<<",\n  \"parent_valid\": "<<value.parent_valid
     <<",\n  \"aging_valid\": "<<value.aging_valid
     <<",\n  \"rest_initialized\": "<<value.rest_initialized
     <<",\n  \"moving_initialized\": "<<value.moving_initialized
     <<",\n  \"forward_valid\": "<<value.forward_valid
     <<",\n  \"maximum_rest_displacement\": "
     <<json_number(value.maximum_rest_displacement)
     <<",\n  \"boundary_clear\": "<<value.boundary_clear
     <<",\n  \"boundary_margin\": "<<json_number(value.boundary_margin)
     <<",\n  \"initial_pair_energy_scale\": "
     <<json_number(value.initial_pair_energy_scale)
     <<",\n  \"moving_initial_hash\": \""<<value.moving_initial_hash
     <<"\",\n  \"moving_forward_final_hash\": \""
     <<value.moving_forward_final_hash
     <<"\",\n  \"moving_reversed_hash\": \""<<value.moving_reversed_hash
     <<"\",\n  \"reverse_valid\": "<<value.reverse_valid
     <<",\n  \"reverse_discrete_exact\": "<<value.reverse_discrete_exact
     <<",\n  \"reverse_recovery\": "<<json_number(value.reverse_recovery)
     <<",\n  \"reverse_maximum_common\": "
     <<json_number(value.reverse_maximum_common)
     <<",\n  \"reverse_steps\": "<<value.reverse_steps
     <<",\n  \"clearing_reached\": "<<value.clearing_reached
     <<",\n  \"first_clearing_tau\": "<<value.first_clearing_tau
     <<",\n  \"outcome\": \""<<value.outcome<<"\",\n"
     <<"  \"checkpoints\": [";
  for(std::size_t i=0;i<value.checkpoints.size();++i) {
    if(i) out<<',';
    out<<'\n'; write_checkpoint(out,value.checkpoints[i]);
  }
  out<<"\n  ],\n  \"production_changed\": false,\n"
     <<"  \"dynamics_changed\": false,\n"
     <<"  \"new_primitive_added\": false,\n"
     <<"  \"wake_label_available\": false\n}\n";
}

}  // namespace

int main(int argc,char** argv) {
  if(argc!=2||std::string(argv[1])!="--run") {
    std::cout<<"FTD-0768 runner: --run\n";
    return argc==1?0:2;
  }
  if(std::string(kFtd0768ProtocolSha256)=="UNLOCKED") return 3;
  if(std::filesystem::exists(ftd0768_results_directory())) {
    std::cerr<<"FTD-0768 result directory already exists\n";
    return 4;
  }
  const auto result=run_long_transport_campaign();
  write_long_transport_result(result);
  std::cout<<std::boolalpha<<"FTD-0768 forward="<<result.forward_valid
           <<" reverse="<<result.reverse_valid
           <<" clearing="<<result.clearing_reached
           <<" outcome="<<result.outcome<<'\n';
  return result.outcome=="LONG_TRANSPORT_EXECUTION_INVALID"?1:0;
}
