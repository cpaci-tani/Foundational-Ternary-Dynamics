/** FTD-0749 candidate: deterministic unique-face CUDA replay. */

// Preload the declaration before transforming only the inherited runner's
// legacy deposition calls.  Header pragma-once guards prevent macro rewriting
// of the class API itself.
#include "ftd/eft/cuda_matched_field_pipeline.h"

#define apply_sparse_current apply_canonical_sparse_current
#define FTD_0748_MAIN_NAME ftd_0748_frozen_main
#include "campaign_canonical_current_horizon_cuda.cpp"
#undef FTD_0748_MAIN_NAME
#undef apply_sparse_current

namespace {

constexpr char kDeterministicProtocolSha256[]=
    "6C0BE1E8109DBD17451FF3A21F426A75583120810EB8C0C9B9077056AE86BB83";

void write_deterministic_support_records(
    const CanonicalHorizonArm& value,const std::string& verdict) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0749";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0749_deterministic_canonical_current_cuda_v1_"
      +value.horizon.slug+"_support";
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"tick,valid,raw_contributions,net_support,source_radius,raw_l1,"
      "net_l1,cancelled_l1,discarded_l1,moment_residual\n"
      <<std::setprecision(17);
  for(const auto& row:value.support)
    csv<<row.tick<<','<<row.valid<<','<<row.raw_contributions<<','
       <<row.net_support<<','<<row.source_radius<<','<<row.raw_l1<<','
       <<row.net_l1<<','<<row.cancelled_l1<<','<<row.discarded_l1<<','
       <<row.moment_residual<<'\n';
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0749\",\n"
      <<"  \"protocol_sha256\": \""<<kDeterministicProtocolSha256<<"\",\n"
      <<"  \"backend\": \"wsl2_cuda_unique_face_current\",\n"
      <<"  \"arm\": \""<<value.horizon.slug<<"\",\n"
      <<"  \"verdict\": \""<<verdict<<"\",\n"
      <<"  \"aggregation_pass\": "<<value.aggregation_pass<<",\n"
      <<"  \"maximum_net_support\": "<<value.maximum_net_support<<",\n"
      <<"  \"maximum_discarded_l1\": "<<value.maximum_discarded_l1<<",\n"
      <<"  \"maximum_moment_residual\": "
      <<value.maximum_moment_residual<<"\n}\n";
}

}  // namespace

int main(int argc,char** argv) {
  const bool qualification=argc==5&&std::string(argv[1])=="--qualify";
  const bool held_out=argc==3;
  if(!qualification&&!held_out) {
    std::cout<<"FTD-0749 CUDA: face|edge|body a|b; qualification uses "
        "--qualify face|edge|body a|b N\n";
    return argc==1?0:2;
  }
  Direction direction;
  const std::string slug=argv[qualification?2:1];
  const std::string replicate=argv[qualification?3:2];
  if(!select_horizon_direction(slug,direction)
      ||(replicate!="a"&&replicate!="b")) return 2;
  const int ticks=qualification?std::stoi(argv[4]):kHorizonTicks;
  if(qualification&&(ticks<1||ticks>8)) return 2;
  if(held_out&&std::string(kDeterministicProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0749 held-out execution refused before protocol lock\n";
    return 3;
  }
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
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  const auto start=std::chrono::steady_clock::now();
  auto result=run_canonical_horizon_cuda_arm(slug,direction,options,
      normalization.mapped_field_work_coefficient,ticks);
  result.horizon.slug=slug+"_"+replicate;
  const double seconds=std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();
  if(qualification) {
    std::cout<<std::setprecision(17)<<"FTD-0749 qualification "
      <<result.horizon.slug<<" ticks="<<ticks
      <<" rows="<<result.horizon.rows.size()
      <<" aggregation="<<result.aggregation_pass
      <<" max_support="<<result.maximum_net_support
      <<" max_discarded="<<result.maximum_discarded_l1
      <<" max_moment="<<result.maximum_moment_residual
      <<" seconds="<<seconds<<" protocol="
      <<kDeterministicProtocolSha256<<'\n';
    return normalization.valid&&result.horizon.initialized
        &&result.horizon.preparation_pass&&result.horizon.initial_pass
        &&result.horizon.forward_executed&&result.aggregation_pass?0:1;
  }
  if(!normalization.valid) result.horizon.exact_pass=false;
  const auto verdict=canonical_horizon_verdict(result);
  write_horizon_records(result.horizon,verdict,"FTD-0749",
      kDeterministicProtocolSha256,"ftd_0749",
      "ftd_0749_deterministic_canonical_current_cuda_v1",
      "wsl2_cuda_unique_face_current");
  write_deterministic_support_records(result,verdict);
  std::cout<<"FTD-0749 "<<result.horizon.slug<<' '<<verdict
      <<" prefix="<<std::setprecision(8)
      <<result.horizon.prefix_scalar_difference
      <<" support="<<result.maximum_net_support
      <<" r48_tick="<<result.horizon.first_tail_tick[5]
      <<" seconds="<<seconds<<'\n';
  return verdict=="CANONICAL_HORIZON_EXECUTION_INVALID"?1:0;
}
