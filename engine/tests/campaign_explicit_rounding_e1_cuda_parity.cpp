/** FTD-0752: explicit-rounding qualification of the FTD-0751 stage map. */

#define FTD_0751_MAIN_NAME ftd_0751_embedded_main
#include "campaign_stagewise_e1_cpu_cuda_parity.cpp"
#undef FTD_0751_MAIN_NAME

namespace {

constexpr char kRoundingProtocolSha256[] =
    "A12929B5C50CFD5586345BF78C5E943B21C430EDA32ECBFB5B9DE98DD23E791E";
constexpr double kDiagnosticGate = 2e-15;

void write_rounding_records(int L,const std::string& direction,
                            const std::vector<StageRecord>& rows,
                            const std::string& verdict,bool executed,
                            double diagnostic_maximum) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0752";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0752_explicit_rounding_e1_parity_v1_L"
      +std::to_string(L)+"_"+direction;
  std::ofstream csv(directory/(stem+".csv"));
  csv<<"tick,stage,exact,unequal_count,maximum_absolute,maximum_ulp,"
      "first_location,cpu_value,cuda_value\n"<<std::setprecision(17);
  for(const auto& row:rows) {
    const auto& d=row.difference;
    csv<<row.tick<<','<<row.stage<<','<<d.exact<<','<<d.unequal_count<<','
       <<d.maximum_absolute<<','<<d.maximum_ulp<<','
       <<csv_escape(d.first_location)<<','<<d.first_cpu<<','<<d.first_cuda
       <<'\n';
  }
  std::ofstream json(directory/(stem+".json"));
  json<<std::setprecision(17)
      <<"{\n  \"ftd_id\": \"FTD-0752\",\n"
      <<"  \"protocol_sha256\": \""<<kRoundingProtocolSha256<<"\",\n"
      <<"  \"backend\": \"wsl2_cuda_explicit_rounding_e1\",\n"
      <<"  \"volume\": "<<L<<",\n"
      <<"  \"direction\": \""<<direction<<"\",\n"
      <<"  \"ticks\": 8,\n"
      <<"  \"row_count\": "<<rows.size()<<",\n"
      <<"  \"executed\": "<<(executed?"true":"false")<<",\n"
      <<"  \"diagnostic_maximum\": "<<diagnostic_maximum<<",\n"
      <<"  \"verdict\": \""<<verdict<<"\"\n}\n";
}

int run_rounding_arm(int L,const std::string& slug,
                     const Direction& direction) {
  using namespace ftd::eft;
  ConnectedMooreBlockOptions options;
  options.dt=0.25;
  options.binding_law=ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth=0.01;
  options.compact_pair_cutoff_distance_squared=1.5;
  options.allow_shared_anchor_chart=true;
  options.gate_tolerance=1e-10;
  options.solve_tolerance=2e-14;
  options.max_iterations=384;
  options.use_sparse_local_current=true;
  options.use_local_residual_evaluation=true;
  options.defer_volume_diagnostics=true;
  const auto prep=prepare_finite_support_derived_compact_pair(
      make_geometry(L,direction,false,1.30,0.0120),options,4,1e-13,4096);
  if(!prep.valid) {
    write_rounding_records(L,slug,{},"EXECUTION_INVALID",false,INFINITY);
    return 1;
  }
  ConnectedMooreBlockState cpu_state=prep.state;
  ConnectedMooreBlockState cuda_state=prep.state;
  CudaMatchedFieldPipeline pipeline(L);
  if(!pipeline.valid()||!pipeline.upload(
      cuda_state.electric,cuda_state.magnetic_half)) {
    write_rounding_records(L,slug,{},"EXECUTION_INVALID",false,INFINITY);
    return 1;
  }
  const double lambda=options.wave_speed*options.dt;
  const Vec3 center{static_cast<double>(L/2),static_cast<double>(L/2),
                    static_cast<double>(L/2)};
  const std::vector<int> radii{2,4,8};
  ConnectedMooreBlockSolveCache cpu_cache,cuda_cache;
  std::vector<StageRecord> rows;
  bool executed=true;
  double diagnostic_maximum=0.0;

  auto record=[&](int tick,const std::string& stage,DifferenceRecord difference) {
    if(stage=="diagnostics") diagnostic_maximum=std::max(
        diagnostic_maximum,difference.maximum_absolute);
    rows.push_back({tick,stage,std::move(difference)});
  };
  for(int tick=1;tick<=8&&executed;++tick) {
    record(tick,"initial_electric",compare_field(
        cpu_state.electric,cuda_state.electric));
    record(tick,"initial_magnetic",compare_field(
        cpu_state.magnetic_half,cuda_state.magnetic_half));
    auto cpu_prepared=cpu_prepare(cpu_state,lambda);
    const auto cpu_magnetic_profile=cpu_prepared.magnetic;
    const auto cpu_electric_profile=cpu_prepared.electric;
    if(!pipeline.prepare_forward(lambda)) { executed=false; break; }
    MatchedEdgeField cuda_magnetic(L);
    MatchedFaceFlux cuda_pre(L);
    if(!pipeline.download_prepared(cuda_magnetic,cuda_pre)) {
      executed=false; break;
    }
    record(tick,"magnetic_prepare",compare_field(
        cpu_prepared.magnetic,cuda_magnetic));
    record(tick,"electric_prepare",compare_field(
        cpu_prepared.electric,cuda_pre));
    const auto cpu_electric_before=cpu_state.electric;
    const auto cpu_magnetic_before=cpu_state.magnetic_half;
    auto cpu_step=solve_connected_moore_block_forward_prepared(
        cpu_state,std::move(cpu_prepared.magnetic),
        std::move(cpu_prepared.electric),options,&cpu_cache);
    auto cuda_step=solve_connected_moore_block_forward_prepared(
        cuda_state,std::move(cuda_magnetic),std::move(cuda_pre),
        options,&cuda_cache);
    if(!cpu_step.solve.converged||!cuda_step.solve.converged
        ||!cpu_step.volume_diagnostics_pending
        ||!cuda_step.volume_diagnostics_pending) {
      executed=false; break;
    }
    record(tick,"matter_root",compare_root(cpu_step,cuda_step));
    if(!pipeline.apply_ordered_sparse_current(
        cuda_step.segments,options.polarity_scale)) {
      executed=false; break;
    }
    MatchedFaceFlux cuda_after(L);
    MatchedEdgeField cuda_after_magnetic(L);
    if(!pipeline.download_after(cuda_after,cuda_after_magnetic)) {
      executed=false; break;
    }
    DifferenceRecord current=compare_field(cpu_step.later.electric,cuda_after);
    merge(current,compare_field(cpu_step.later.magnetic_half,
        cuda_after_magnetic),"magnetic/");
    merge(current,compare_field(cuda_step.later.electric,cuda_after),
          "cuda_host_device/");
    record(tick,"ordered_current",std::move(current));
    auto cuda_transferred=cuda_step.later;
    cuda_transferred.electric=cuda_after;
    cuda_transferred.magnetic_half=cuda_after_magnetic;
    record(tick,"state_transfer",compare_matter_state(
        cpu_step.later,cuda_transferred,true));
    const auto cpu_profile=evaluate_batched_regional_energy_profile(
        cpu_electric_before,cpu_magnetic_before,cpu_electric_profile,
        cpu_magnetic_profile,cpu_step.later.electric,
        lambda,center,radii,1e-10);
    const auto cuda_profile=pipeline.observe_deterministic(
        lambda,center,radii,1e-10);
    record(tick,"diagnostics",compare_profile(cpu_profile,cuda_profile));
    cpu_state=std::move(cpu_step.later);
    cuda_state=std::move(cuda_step.later);
    cuda_state.electric=std::move(cuda_after);
    cuda_state.magnetic_half=std::move(cuda_after_magnetic);
    if(!pipeline.advance()) { executed=false; break; }
  }
  bool dynamic_exact=executed&&rows.size()==64;
  for(const auto& row:rows)
    if(row.stage!="diagnostics"&&!row.difference.exact) dynamic_exact=false;
  const bool diagnostic_pass=diagnostic_maximum<=kDiagnosticGate;
  const std::string verdict=!executed?"EXECUTION_INVALID"
      :!dynamic_exact?"DYNAMIC_PARITY_FAILED"
      :!diagnostic_pass?"DIAGNOSTIC_BOUND_FAILED"
      :diagnostic_maximum==0.0?"EXACT_DYNAMIC_AND_DIAGNOSTIC_PARITY"
      :"EXACT_DYNAMIC_PARITY_DIAGNOSTIC_BOUNDED";
  write_rounding_records(L,slug,rows,verdict,executed,diagnostic_maximum);
  std::cout<<std::setprecision(17)<<"FTD-0752 L="<<L
      <<" direction="<<slug<<" rows="<<rows.size()
      <<" dynamic_exact="<<dynamic_exact
      <<" diagnostic_maximum="<<diagnostic_maximum
      <<" verdict="<<verdict<<'\n';
  return executed&&dynamic_exact&&diagnostic_pass?0:1;
}

}  // namespace

int main(int argc,char** argv) {
  if(argc!=3) {
    std::cout<<"usage: campaign_explicit_rounding_e1_cuda_parity 33|65 "
        "face|edge|body\n";
    return argc==1?0:2;
  }
  const int L=std::stoi(argv[1]);
  if(L!=33&&L!=65) return 2;
  const std::string slug=argv[2];
  Direction direction{};
  if(slug=="face") direction=kDirections[0];
  else if(slug=="edge") direction=kDirections[1];
  else if(slug=="body") direction=kDirections[2];
  else return 2;
  return run_rounding_arm(L,slug,direction);
}
