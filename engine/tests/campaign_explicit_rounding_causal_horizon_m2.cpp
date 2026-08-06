/** FTD-0753: fresh explicit-rounding causal-horizon M2 witness. */

#include "ftd/eft/cuda_matched_field_pipeline.h"

#define apply_sparse_current apply_ordered_sparse_current
#define observe observe_deterministic
#define FTD_0748_MAIN_NAME ftd_0753_frozen_parent_main
#include "campaign_canonical_current_horizon_cuda.cpp"
#undef FTD_0748_MAIN_NAME
#undef observe
#undef apply_sparse_current

namespace {

constexpr char kM2ProtocolSha256[]=
    "66D64B1A09AAB3243C5BA06991B9979C10C03EA8B8B4A01BA3803260BF3822A4";

std::string m2_horizon_verdict(const CanonicalHorizonArm& value) {
  const auto& arm=value.horizon;
  const bool infrastructure=arm.initialized&&arm.preparation_pass
      &&arm.initial_pass&&arm.forward_executed&&arm.exact_pass
      &&arm.support_pass;
  if(!infrastructure) return "M2_HORIZON_EXECUTION_INVALID";
  if(!value.aggregation_pass)
    return "M2_HORIZON_CURRENT_AGGREGATION_INVALID";
  if(!arm.core_pass) return "M2_HORIZON_CORE_NOT_PERSISTENT";
  if(!arm.near_field_pass) return "M2_HORIZON_NEAR_FIELD_NOT_STABLE";
  if(!arm.arrival_pass) return "M2_HORIZON_R48_ARRIVAL_FAIL";
  if(!arm.post_arrival_pass)
    return "M2_HORIZON_POST_ARRIVAL_NOT_PERSISTENT";
  return "M2_CAUSAL_HORIZON_WITNESS_CONSTRUCTIVE";
}

void write_m2_support_records(
    const CanonicalHorizonArm& value,const std::string& verdict) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0753";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0753_explicit_rounding_causal_horizon_m2_v1_"
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
      <<"{\n  \"ftd_id\": \"FTD-0753\",\n"
      <<"  \"protocol_sha256\": \""<<kM2ProtocolSha256<<"\",\n"
      <<"  \"backend\": \"wsl2_cuda_explicit_rounding_ordered\",\n"
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
  const bool qualification=argc==4&&std::string(argv[1])=="--qualify";
  const bool registered=argc==2;
  if(!qualification&&!registered) {
    std::cout<<"FTD-0753 CUDA: registered face|edge|body; qualification uses "
        "--qualify face|edge|body N\n";
    return argc==1?0:2;
  }
  Direction direction;
  const std::string slug=argv[qualification?2:1];
  if(!select_horizon_direction(slug,direction)) return 2;
  const int ticks=qualification?std::stoi(argv[3]):kHorizonTicks;
  if(qualification&&(ticks<1||ticks>8)) return 2;
  if(registered&&std::string(kM2ProtocolSha256)=="UNLOCKED") {
    std::cerr<<"FTD-0753 registered execution refused before protocol lock\n";
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
  const double seconds=std::chrono::duration<double>(
      std::chrono::steady_clock::now()-start).count();

  if(qualification) {
    std::cout<<std::setprecision(17)<<"FTD-0753 qualification "<<slug
      <<" ticks="<<ticks<<" rows="<<result.horizon.rows.size()
      <<" aggregation="<<result.aggregation_pass
      <<" max_support="<<result.maximum_net_support
      <<" max_discarded="<<result.maximum_discarded_l1
      <<" max_moment="<<result.maximum_moment_residual
      <<" seconds="<<seconds<<" protocol="<<kM2ProtocolSha256<<'\n';
    return normalization.valid&&result.horizon.initialized
        &&result.horizon.preparation_pass&&result.horizon.initial_pass
        &&result.horizon.forward_executed&&result.aggregation_pass?0:1;
  }

  if(!normalization.valid) result.horizon.exact_pass=false;
  const auto verdict=m2_horizon_verdict(result);
  write_horizon_records(result.horizon,verdict,"FTD-0753",
      kM2ProtocolSha256,"ftd_0753",
      "ftd_0753_explicit_rounding_causal_horizon_m2_v1",
      "wsl2_cuda_explicit_rounding_ordered");
  write_m2_support_records(result,verdict);
  std::cout<<"FTD-0753 "<<slug<<' '<<verdict
      <<" support="<<result.maximum_net_support
      <<" onset="<<result.horizon.energetic_onset_tick
      <<" r48_tick="<<result.horizon.first_tail_tick[5]
      <<" seconds="<<seconds<<'\n';
  return verdict=="M2_HORIZON_EXECUTION_INVALID"?1:0;
}
