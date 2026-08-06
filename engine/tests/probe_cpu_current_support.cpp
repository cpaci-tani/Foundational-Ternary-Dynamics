/** FTD-0748 pre-lock probe: measure the original FTD-0745 CPU support. */

#define main ftd_0731_embedded_main
#include "test_multipass_formation_persistence.cpp"
#undef main

#include <iomanip>

namespace {

constexpr int kProbeL=193;
constexpr int kProbeTicks=184;
constexpr int kProbeSupportRadius=4;

struct CpuSupportSummary {
  int raw_entries=0;
  double raw_l1=0.0;
  double maximum=0.0;
  double minimum_nonzero=INFINITY;
  double maximum_transverse_component=0.0;
};

CpuSupportSummary summarize_cpu_support(
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
    double polarity_scale) {
  CpuSupportSummary result;
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
      const double magnitude=std::abs(polarity_scale*entry.value);
      result.raw_l1+=magnitude;
      result.maximum=std::max(result.maximum,magnitude);
      result.minimum_nonzero=std::min(result.minimum_nonzero,magnitude);
    }
  }
  if(!std::isfinite(result.minimum_nonzero)) result.minimum_nonzero=0.0;
  return result;
}

}  // namespace

int main(int argc,char** argv) {
  if(argc!=3) {
    std::cerr<<"usage: probe_cpu_current_support face|edge|body ticks\n";
    return 2;
  }
  Direction direction;
  const std::string slug=argv[1];
  if(slug=="face") direction=kDirections[0];
  else if(slug=="edge") direction=kDirections[1];
  else if(slug=="body") direction=kDirections[2];
  else return 2;
  const int tick_limit=std::stoi(argv[2]);
  if(tick_limit<1||tick_limit>kProbeTicks) return 2;

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
  const auto prep=ftd::eft::prepare_finite_support_derived_compact_pair(
      make_geometry(kProbeL,direction,false,1.30,0.0120),options,
      kProbeSupportRadius,1e-13,4096);
  if(!prep.valid) return 1;

  ConnectedMooreBlockState state=prep.state;
  ConnectedMooreBlockSolveCache cache;
  std::cout<<"tick,cpu_raw,raw_l1,max_entry,min_entry,max_transverse\n"
      <<std::setprecision(17);
  int previous=-1;
  for(int tick=1;tick<=tick_limit;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_forward(
        state,options,&cache);
    if(!step.valid) return 1;
    const auto summary=summarize_cpu_support(
        step.segments,options.polarity_scale);
    if(tick>=20||summary.raw_entries!=previous)
      std::cout<<tick<<','<<summary.raw_entries<<','<<summary.raw_l1<<','
          <<summary.maximum<<','<<summary.minimum_nonzero<<','
          <<summary.maximum_transverse_component<<'\n';
    previous=summary.raw_entries;
    state=step.later;
  }
  return 0;
}
