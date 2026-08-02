/** FTD-0751: bounded stagewise CPU/CUDA classifier for selected E1 matter. */

#define main ftd_0731_embedded_main
#include "test_multipass_formation_persistence.cpp"
#undef main

#include "ftd/eft/cuda_matched_field_pipeline.h"

#include <cstdint>
#include <cstring>
#include <limits>

namespace {

constexpr char kStagewiseProtocolSha256[] =
    "AD2F4DBD0843152B6398CEB9A9EF7C92B559D3FC171B167994AD5BA509103FB7";
constexpr int kStagewiseTicks = 8;

struct DifferenceRecord {
  bool exact = true;
  std::uint64_t unequal_count = 0;
  double maximum_absolute = 0.0;
  std::uint64_t maximum_ulp = 0;
  std::string first_location;
  double first_cpu = 0.0;
  double first_cuda = 0.0;
};

struct StageRecord {
  int tick = 0;
  std::string stage;
  DifferenceRecord difference;
};

std::uint64_t ordered_bits(double value) {
  std::uint64_t bits = 0;
  static_assert(sizeof(bits) == sizeof(value), "double width");
  std::memcpy(&bits, &value, sizeof(bits));
  constexpr std::uint64_t sign = UINT64_C(1) << 63;
  return (bits & sign) ? ~bits : (bits | sign);
}

std::uint64_t ulp_distance(double left, double right) {
  if (!std::isfinite(left) || !std::isfinite(right))
    return left == right ? 0 : std::numeric_limits<std::uint64_t>::max();
  const auto a = ordered_bits(left);
  const auto b = ordered_bits(right);
  return a > b ? a-b : b-a;
}

void compare_scalar(DifferenceRecord& result, const std::string& location,
                    double cpu, double cuda) {
  std::uint64_t a = 0, b = 0;
  std::memcpy(&a, &cpu, sizeof(a));
  std::memcpy(&b, &cuda, sizeof(b));
  if (a == b) return;
  if (result.exact) {
    result.first_location = location;
    result.first_cpu = cpu;
    result.first_cuda = cuda;
  }
  result.exact = false;
  ++result.unequal_count;
  const double absolute = std::isfinite(cpu) && std::isfinite(cuda)
      ? std::abs(cpu-cuda) : INFINITY;
  result.maximum_absolute = std::max(result.maximum_absolute, absolute);
  result.maximum_ulp = std::max(result.maximum_ulp, ulp_distance(cpu,cuda));
}

void compare_integer(DifferenceRecord& result, const std::string& location,
                     long long cpu, long long cuda) {
  if (cpu == cuda) return;
  compare_scalar(result, location, static_cast<double>(cpu),
                 static_cast<double>(cuda));
}

void merge(DifferenceRecord& target, const DifferenceRecord& source,
           const std::string& prefix = {}) {
  if (source.exact) return;
  if (target.exact) {
    target.first_location = prefix+source.first_location;
    target.first_cpu = source.first_cpu;
    target.first_cuda = source.first_cuda;
  }
  target.exact = false;
  target.unequal_count += source.unequal_count;
  target.maximum_absolute = std::max(
      target.maximum_absolute, source.maximum_absolute);
  target.maximum_ulp = std::max(target.maximum_ulp, source.maximum_ulp);
}

template <typename Field>
DifferenceRecord compare_field(const Field& cpu, const Field& cuda) {
  DifferenceRecord result;
  compare_integer(result, "L", cpu.L, cuda.L);
  if (cpu.x.size() != cuda.x.size() || cpu.y.size() != cuda.y.size()
      || cpu.z.size() != cuda.z.size()) {
    compare_integer(result, "field_size", cpu.x.size(), cuda.x.size());
    return result;
  }
  const std::array<const char*,3> labels{{"x:","y:","z:"}};
  const std::array<const std::vector<double>*,3> a{{&cpu.x,&cpu.y,&cpu.z}};
  const std::array<const std::vector<double>*,3> b{{&cuda.x,&cuda.y,&cuda.z}};
  for (int axis = 0; axis < 3; ++axis)
    for (std::size_t i = 0; i < a[axis]->size(); ++i)
      compare_scalar(result, labels[axis]+std::to_string(i),
                     (*a[axis])[i], (*b[axis])[i]);
  return result;
}

DifferenceRecord compare_matter_state(const ConnectedMooreBlockState& cpu,
                                      const ConnectedMooreBlockState& cuda,
                                      bool include_fields) {
  DifferenceRecord result;
  if (include_fields) {
    merge(result, compare_field(cpu.electric,cuda.electric), "electric/");
    merge(result, compare_field(cpu.magnetic_half,cuda.magnetic_half),
          "magnetic/");
  }
  compare_integer(result,"constituent_count",cpu.constituents.size(),
                  cuda.constituents.size());
  compare_integer(result,"charge_count",cpu.charges.size(),cuda.charges.size());
  compare_integer(result,"edge_count",cpu.edges.size(),cuda.edges.size());
  compare_integer(result,"width",cpu.width,cuda.width);
  compare_integer(result,"orientation_axis",cpu.orientation_axis,
                  cuda.orientation_axis);
  const std::size_t count = std::min(
      cpu.constituents.size(), cuda.constituents.size());
  for (std::size_t i = 0; i < count; ++i) {
    const auto prefix = "constituent:"+std::to_string(i)+"/";
    const auto& a = cpu.constituents[i];
    const auto& b = cuda.constituents[i];
    compare_integer(result,prefix+"anchor_x",a.anchor.x,b.anchor.x);
    compare_integer(result,prefix+"anchor_y",a.anchor.y,b.anchor.y);
    compare_integer(result,prefix+"anchor_z",a.anchor.z,b.anchor.z);
    compare_scalar(result,prefix+"remainder_x",a.remainder.x,b.remainder.x);
    compare_scalar(result,prefix+"remainder_y",a.remainder.y,b.remainder.y);
    compare_scalar(result,prefix+"remainder_z",a.remainder.z,b.remainder.z);
    compare_scalar(result,prefix+"momentum_x",a.momentum.x,b.momentum.x);
    compare_scalar(result,prefix+"momentum_y",a.momentum.y,b.momentum.y);
    compare_scalar(result,prefix+"momentum_z",a.momentum.z,b.momentum.z);
  }
  const std::size_t charge_count = std::min(cpu.charges.size(),cuda.charges.size());
  for (std::size_t i = 0; i < charge_count; ++i)
    compare_integer(result,"charge:"+std::to_string(i),cpu.charges[i],
                    cuda.charges[i]);
  return result;
}

DifferenceRecord compare_segments(
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& cpu,
    const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& cuda) {
  DifferenceRecord result;
  compare_integer(result,"segment_count",cpu.size(),cuda.size());
  const std::size_t count = std::min(cpu.size(),cuda.size());
  for (std::size_t s = 0; s < count; ++s) {
    const auto prefix="segment:"+std::to_string(s)+"/";
    const auto& a=cpu[s];
    const auto& b=cuda[s];
    compare_integer(result,prefix+"L",a.L,b.L);
    compare_integer(result,prefix+"charge",a.charge,b.charge);
    compare_integer(result,prefix+"valid",a.valid,b.valid);
    compare_scalar(result,prefix+"start_x",a.start_effective_position.x,
                   b.start_effective_position.x);
    compare_scalar(result,prefix+"start_y",a.start_effective_position.y,
                   b.start_effective_position.y);
    compare_scalar(result,prefix+"start_z",a.start_effective_position.z,
                   b.start_effective_position.z);
    compare_scalar(result,prefix+"end_x",a.end_effective_position.x,
                   b.end_effective_position.x);
    compare_scalar(result,prefix+"end_y",a.end_effective_position.y,
                   b.end_effective_position.y);
    compare_scalar(result,prefix+"end_z",a.end_effective_position.z,
                   b.end_effective_position.z);
    compare_integer(result,prefix+"entry_count",a.sparse_current.size(),
                    b.sparse_current.size());
    const std::size_t entries=std::min(
        a.sparse_current.size(),b.sparse_current.size());
    for(std::size_t i=0;i<entries;++i) {
      const auto item=prefix+"entry:"+std::to_string(i)+"/";
      const auto& x=a.sparse_current[i];
      const auto& y=b.sparse_current[i];
      compare_integer(result,item+"axis",x.axis,y.axis);
      compare_integer(result,item+"face_x",x.face.x,y.face.x);
      compare_integer(result,item+"face_y",x.face.y,y.face.y);
      compare_integer(result,item+"face_z",x.face.z,y.face.z);
      compare_scalar(result,item+"value",x.value,y.value);
    }
  }
  return result;
}

DifferenceRecord compare_root(
    const ftd::eft::ConnectedMooreBlockStepResult& cpu,
    const ftd::eft::ConnectedMooreBlockStepResult& cuda) {
  DifferenceRecord result;
  compare_integer(result,"solve_attempted",cpu.solve.attempted,cuda.solve.attempted);
  compare_integer(result,"solve_converged",cpu.solve.converged,cuda.solve.converged);
  compare_integer(result,"iterations",cpu.solve.iterations,cuda.solve.iterations);
  compare_integer(result,"residual_evaluations",cpu.solve.residual_evaluations,
                  cuda.solve.residual_evaluations);
  compare_scalar(result,"solve_residual",cpu.solve.residual,cuda.solve.residual);
  compare_scalar(result,"step_residual",cpu.solve.step_residual,
                 cuda.solve.step_residual);
  compare_scalar(result,"root_residual",cpu.root_residual,cuda.root_residual);
  compare_scalar(result,"interaction_scale",cpu.interaction_scale,
                 cuda.interaction_scale);
  compare_scalar(result,"polarity_scale",cpu.polarity_scale,cuda.polarity_scale);
  merge(result,compare_matter_state(cpu.later,cuda.later,false),"later/");
  merge(result,compare_segments(cpu.segments,cuda.segments),"current/");
  return result;
}

DifferenceRecord compare_profile(
    const ftd::eft::BatchedRegionalEnergyProfile& cpu,
    const ftd::eft::BatchedRegionalEnergyProfile& cuda) {
  DifferenceRecord result;
  compare_integer(result,"valid",cpu.valid,cuda.valid);
  compare_integer(result,"region_count",cpu.regions.size(),cuda.regions.size());
  compare_scalar(result,"energy_before",cpu.energy_before,cuda.energy_before);
  compare_scalar(result,"energy_pre_current",cpu.energy_pre_current,
                 cuda.energy_pre_current);
  compare_scalar(result,"energy_after",cpu.energy_after,cuda.energy_after);
  compare_scalar(result,"maximum_scalar_equivalence_residual",
                 cpu.maximum_scalar_equivalence_residual,
                 cuda.maximum_scalar_equivalence_residual);
  const std::size_t count=std::min(cpu.regions.size(),cuda.regions.size());
  for(std::size_t i=0;i<count;++i) {
    const auto prefix="region:"+std::to_string(i)+"/";
    const auto& a=cpu.regions[i];
    const auto& b=cuda.regions[i];
    compare_scalar(result,prefix+"energy_before",a.energy_before,b.energy_before);
    compare_scalar(result,prefix+"energy_pre_current",a.energy_pre_current,
                   b.energy_pre_current);
    compare_scalar(result,prefix+"energy_after",a.energy_after,b.energy_after);
    compare_scalar(result,prefix+"boundary_transport_into",
                   a.boundary_transport_into,b.boundary_transport_into);
    compare_scalar(result,prefix+"source_exchange_into_field",
                   a.source_exchange_into_field,b.source_exchange_into_field);
    compare_scalar(result,prefix+"regional_ledger_residual",
                   a.regional_ledger_residual,b.regional_ledger_residual);
  }
  return result;
}

struct PreparedFields {
  ftd::eft::MatchedEdgeField magnetic;
  ftd::eft::MatchedFaceFlux electric;
  explicit PreparedFields(int L):magnetic(L),electric(L){}
};

PreparedFields cpu_prepare(const ConnectedMooreBlockState& state,
                           double lambda) {
  PreparedFields result(state.electric.L);
  result.magnetic=state.magnetic_half;
  const auto electric_curl=ftd::eft::matched_curl_adjoint(state.electric);
  for(std::size_t i=0;i<result.magnetic.x.size();++i) {
    result.magnetic.x[i]-=lambda*electric_curl.x[i];
    result.magnetic.y[i]-=lambda*electric_curl.y[i];
    result.magnetic.z[i]-=lambda*electric_curl.z[i];
  }
  result.electric=state.electric;
  const auto magnetic_curl=ftd::eft::matched_curl(result.magnetic);
  for(std::size_t i=0;i<result.electric.x.size();++i) {
    result.electric.x[i]+=lambda*magnetic_curl.x[i];
    result.electric.y[i]+=lambda*magnetic_curl.y[i];
    result.electric.z[i]+=lambda*magnetic_curl.z[i];
  }
  return result;
}

std::string csv_escape(const std::string& value) {
  std::string result="\"";
  for(char c:value) {
    if(c=='\"') result+="\"\"";
    else result+=c;
  }
  return result+'\"';
}

std::string classification_for_stage(const std::string& stage) {
  if(stage=="initial_electric"||stage=="initial_magnetic"
      ||stage=="state_transfer") return "STATE_TRANSFER_DIVERGENCE";
  if(stage=="magnetic_prepare")
    return "SOURCE_FREE_MAGNETIC_PREPARE_DIVERGENCE";
  if(stage=="electric_prepare")
    return "SOURCE_FREE_ELECTRIC_PREPARE_DIVERGENCE";
  if(stage=="matter_root") return "MATTER_ROOT_DIVERGENCE";
  if(stage=="ordered_current") return "ORDERED_CURRENT_DIVERGENCE";
  if(stage=="diagnostics") return "DIAGNOSTIC_ONLY_DIVERGENCE";
  return "EXECUTION_INVALID";
}

void write_records(int L,const std::string& direction,
                   const std::vector<StageRecord>& rows,
                   const std::string& classification,bool executed) {
  const auto directory=std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0751";
  std::filesystem::create_directories(directory);
  const auto stem="ftd_0751_stagewise_e1_parity_v1_L"+std::to_string(L)
      +"_"+direction;
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
  json<<"{\n  \"ftd_id\": \"FTD-0751\",\n"
      <<"  \"protocol_sha256\": \""<<kStagewiseProtocolSha256<<"\",\n"
      <<"  \"backend\": \"wsl2_cuda_stagewise_e1\",\n"
      <<"  \"volume\": "<<L<<",\n"
      <<"  \"direction\": \""<<direction<<"\",\n"
      <<"  \"ticks\": "<<kStagewiseTicks<<",\n"
      <<"  \"row_count\": "<<rows.size()<<",\n"
      <<"  \"executed\": "<<(executed?"true":"false")<<",\n"
      <<"  \"classification\": \""<<classification<<"\"\n}\n";
}

int run_arm(int L,const std::string& slug,const Direction& direction) {
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
    write_records(L,slug,{},"EXECUTION_INVALID",false);
    return 1;
  }
  ConnectedMooreBlockState cpu_state=prep.state;
  ConnectedMooreBlockState cuda_state=prep.state;
  CudaMatchedFieldPipeline pipeline(L);
  if(!pipeline.valid()||!pipeline.upload(
      cuda_state.electric,cuda_state.magnetic_half)) {
    write_records(L,slug,{},"EXECUTION_INVALID",false);
    return 1;
  }
  const double lambda=options.wave_speed*options.dt;
  const Vec3 center{static_cast<double>(L/2),static_cast<double>(L/2),
                    static_cast<double>(L/2)};
  const std::vector<int> radii{2,4,8};
  ConnectedMooreBlockSolveCache cpu_cache,cuda_cache;
  std::vector<StageRecord> rows;
  std::string classification="EXACT_STAGE_PARITY";
  bool executed=true;
  auto record=[&](int tick,const std::string& stage,DifferenceRecord difference) {
    if(classification=="EXACT_STAGE_PARITY"&&!difference.exact)
      classification=classification_for_stage(stage);
    rows.push_back({tick,stage,std::move(difference)});
  };

  for(int tick=1;tick<=kStagewiseTicks&&executed;++tick) {
    record(tick,"initial_electric",compare_field(
        cpu_state.electric,cuda_state.electric));
    record(tick,"initial_magnetic",compare_field(
        cpu_state.magnetic_half,cuda_state.magnetic_half));
    auto cpu_prepared=cpu_prepare(cpu_state,lambda);
    const auto cpu_magnetic_prepared_for_profile=cpu_prepared.magnetic;
    const auto cpu_electric_prepared_for_profile=cpu_prepared.electric;
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
    DifferenceRecord current_difference=compare_field(
        cpu_step.later.electric,cuda_after);
    merge(current_difference,compare_field(
        cpu_step.later.magnetic_half,cuda_after_magnetic),"magnetic/");
    const auto cuda_host_after_difference=compare_field(
        cuda_step.later.electric,cuda_after);
    merge(current_difference,cuda_host_after_difference,"cuda_host_device/");
    record(tick,"ordered_current",std::move(current_difference));

    auto cuda_transferred=cuda_step.later;
    cuda_transferred.electric=cuda_after;
    cuda_transferred.magnetic_half=cuda_after_magnetic;
    record(tick,"state_transfer",compare_matter_state(
        cpu_step.later,cuda_transferred,true));

    const auto cpu_profile=evaluate_batched_regional_energy_profile(
        cpu_electric_before,cpu_magnetic_before,
        cpu_electric_prepared_for_profile,
        cpu_magnetic_prepared_for_profile,cpu_step.later.electric,
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
  if(!executed) classification="EXECUTION_INVALID";
  write_records(L,slug,rows,classification,executed);
  std::cout<<"FTD-0751 L="<<L<<" direction="<<slug
      <<" rows="<<rows.size()<<" classification="<<classification<<'\n';
  return executed?0:1;
}

}  // namespace

#ifndef FTD_0751_MAIN_NAME
#define FTD_0751_MAIN_NAME main
#endif

int FTD_0751_MAIN_NAME(int argc,char** argv) {
  if(argc!=3) {
    std::cout<<"usage: campaign_stagewise_e1_cpu_cuda_parity 33|65 "
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
  return run_arm(L,slug,direction);
}
