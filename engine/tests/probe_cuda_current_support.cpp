/** FTD-0748 pre-lock probe: characterize sparse-current support semantics. */

#define FTD_CAUSAL_HORIZON_MAIN ftd_0746_embedded_main
#include "test_causal_horizon_environmental_persistence.cpp"

#include "ftd/eft/cuda_matched_field_pipeline.h"

#include <iomanip>
#include <map>
#include <tuple>

namespace {

struct SupportSummary {
  int raw_entries=0;
  int aggregate_exact=0;
  int aggregate_gated=0;
  double raw_l1=0.0;
  double aggregate_l1=0.0;
  double maximum=0.0;
  double minimum_nonzero=INFINITY;
  double maximum_transverse_component=0.0;
};

int wrap_support(int value,int L) {
  value%=L;
  return value<0?value+L:value;
}

SupportSummary summarize_support(
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
    double polarity_scale,double gate) {
  using Key=std::tuple<int,int,int,int>;
  std::map<Key,long double> aggregate;
  SupportSummary result;
  for(const auto& segment:segments) {
    const auto delta=segment.end_effective_position
        -segment.start_effective_position;
    const std::array<double,3> component{delta.x,delta.y,delta.z};
    const double largest=std::max({std::abs(delta.x),std::abs(delta.y),
                                  std::abs(delta.z)});
    for(double value:component)
      if(std::abs(value)<largest)
        result.maximum_transverse_component=std::max(
            result.maximum_transverse_component,std::abs(value));
    for(const auto& entry:segment.sparse_current) {
      if(entry.value==0.0) continue;
      ++result.raw_entries;
      const double value=polarity_scale*entry.value;
      const double magnitude=std::abs(value);
      result.raw_l1+=magnitude;
      result.maximum=std::max(result.maximum,magnitude);
      result.minimum_nonzero=std::min(result.minimum_nonzero,magnitude);
      aggregate[{entry.axis,wrap_support(entry.face.x,segment.L),
          wrap_support(entry.face.y,segment.L),
          wrap_support(entry.face.z,segment.L)}]+=value;
    }
  }
  if(!std::isfinite(result.minimum_nonzero)) result.minimum_nonzero=0.0;
  for(const auto& [key,value_ld]:aggregate) {
    (void)key;
    const double value=static_cast<double>(value_ld);
    const double magnitude=std::abs(value);
    if(value!=0.0) ++result.aggregate_exact;
    if(magnitude>gate) ++result.aggregate_gated;
    result.aggregate_l1+=magnitude;
  }
  return result;
}

}  // namespace

int main(int argc,char** argv) {
  if(argc!=3) {
    std::cerr<<"usage: probe_cuda_current_support face|edge|body ticks\n";
    return 2;
  }
  Direction direction;
  const std::string slug=argv[1];
  if(!select_horizon_direction(slug,direction)) return 2;
  const int tick_limit=std::stoi(argv[2]);
  if(tick_limit<1||tick_limit>184) return 2;

  ConnectedMooreBlockOptions options;
  options.dt=0.25;
  options.binding_law=ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth=0.01;
  options.compact_pair_cutoff_distance_squared=1.5;
  options.allow_shared_anchor_chart=true;
  options.gate_tolerance=kGate;
  options.solve_tolerance=2e-14;
  options.max_iterations=384;
  options.use_sparse_local_current=true;
  options.use_local_residual_evaluation=true;
  options.defer_volume_diagnostics=true;
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  if(!normalization.valid) return 1;
  const double interaction_scale=normalization.mapped_field_work_coefficient;
  const Vec3 center{static_cast<double>(kHorizonL/2),
                    static_cast<double>(kHorizonL/2),
                    static_cast<double>(kHorizonL/2)};
  const auto prep=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_geometry(kHorizonL,direction,false,1.30,0.0120),options,
      kHorizonSupportRadius,1e-13,4096);
  if(!prep.valid) return 1;

  bool baseline_valid=false;
  const auto baseline=load_horizon_baseline(direction.label,baseline_valid);
  if(!baseline_valid) return 1;
  ConnectedMooreBlockState state=prep.state;
  ftd::eft::CudaMatchedFieldPipeline pipeline(kHorizonL);
  if(!pipeline.valid()||!pipeline.upload(state.electric,state.magnetic_half))
    return 1;
  const double lambda=options.wave_speed*options.dt;
  const std::vector<int> radii(kHorizonRadii.begin(),kHorizonRadii.end());
  ftd::eft::MatchedEdgeField prepared_magnetic(kHorizonL);
  ftd::eft::MatchedFaceFlux prepared_electric(kHorizonL);
  ConnectedMooreBlockSolveCache cache;
  std::cout<<"tick,baseline_raw,cuda_raw,aggregate_exact,aggregate_gate_1e-10,"
      "raw_l1,aggregate_l1,max_entry,min_entry,max_transverse\n"
      <<std::setprecision(17);
  for(int tick=1;tick<=tick_limit;++tick) {
    if(!pipeline.prepare_forward(lambda)
        ||!pipeline.download_prepared(prepared_magnetic,prepared_electric))
      return 1;
    auto step=ftd::eft::solve_connected_moore_block_forward_prepared(
        state,std::move(prepared_magnetic),std::move(prepared_electric),
        options,&cache);
    if(!step.volume_diagnostics_pending
        ||!pipeline.apply_sparse_current(step.segments,options.polarity_scale))
      return 1;
    const auto profile=pipeline.observe(
        lambda,center,radii,kHorizonGate);
    if(!profile.valid) return 1;
    const auto diagnostics=pipeline.diagnose_common_action(
        step.segments,options.polarity_scale,interaction_scale,
        options.wave_speed,options.dt,kHorizonGate);
    step=ftd::eft::complete_connected_moore_block_volume_diagnostics(
        std::move(step),diagnostics,options);
    if(!step.valid) return 1;
    const auto summary=summarize_support(
        step.segments,options.polarity_scale,kHorizonGate);
    const auto found=baseline.find(tick);
    if(found==baseline.end()) return 1;
    if(summary.raw_entries!=found->second.source_entries||tick==tick_limit) {
      std::cout<<tick<<','<<found->second.source_entries<<','
          <<summary.raw_entries<<','<<summary.aggregate_exact<<','
          <<summary.aggregate_gated<<','<<summary.raw_l1<<','
          <<summary.aggregate_l1<<','<<summary.maximum<<','
          <<summary.minimum_nonzero<<','
          <<summary.maximum_transverse_component<<'\n';
    }
    std::swap(state.electric,prepared_electric);
    std::swap(state.magnetic_half,prepared_magnetic);
    state=std::move(step.later);
    if(!pipeline.advance()) return 1;
  }
  return 0;
}
