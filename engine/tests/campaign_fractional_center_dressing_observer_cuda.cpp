/** FTD-0763: untouched CUDA replay through the fractional-center observer. */

#pragma push_macro("main")
#undef main
#define main ftd0762_reference_main
#include "campaign_m4_moving_dressing_observer_forensics_cuda.cpp"
#undef main
#pragma pop_macro("main")

namespace {

constexpr char kFtd0763ProtocolSha256[] =
    "FB78C2688A90E18D01071DA390BFE230FFD76CF340FD2CB56AD6D545CDD8C63A";
constexpr int kFtd0763Volume = 321;
constexpr int kFtd0763TransportTicks = 64;
constexpr double kFtd0763Gate = 1e-12;

struct FractionalCheckpoint {
  bool valid = false;
  bool observer_valid = false;
  bool boundary_ledger_valid = false;
  bool ladder_valid = false;
  bool cuda_scalar_only = false;
  ftd::Vec3 center{};
  ftd::Vec3 support_center{};
  ftd::Vec3 center_offset{};
  ftd::Vec3 matter_momentum{};
  double kinetic_energy = INFINITY;
  double internal_energy = INFINITY;
  double bound_energy = INFINITY;
  double residual_energy = INFINITY;
  double outgoing_energy = INFINITY;
  double incoming_energy = INFINITY;
  double radial_energy = INFINITY;
  double background_energy = INFINITY;
  double signed_radial_poynting = INFINITY;
  double primitive_interference = INFINITY;
  double induced_boundary_interference = INFINITY;
  double boundary_flux_sum = INFINITY;
  double boundary_identity_residual = INFINITY;
  double readout_reconstruction_residual = INFINITY;
  double maximum_reconstruction_residual = INFINITY;
  double actual_gauss_residual = INFINITY;
  double energy_partition_residual = INFINITY;
  double characteristic_flux_residual = INFINITY;
  double ladder_energy_residual = INFINITY;
  double ladder_projection_residual = INFINITY;
  std::vector<ftd::eft::StateOnlyCharacteristicShell> shells;
  std::string observer_error;
  std::string ladder_error;
  std::size_t host_to_device_bytes = 0;
  std::size_t device_to_host_bytes = 0;
  double kernel_ms = 0.0;
};

ftd::Vec3 matter_momentum(
    const ftd::eft::ConnectedMooreBlockState& state) {
  ftd::Vec3 result{};
  for(const auto& point:state.constituents) result+=point.momentum;
  return result;
}

FractionalCheckpoint observe_fractional_checkpoint(
    const ftd::eft::ConnectedMooreBlockState& state,
    const ftd::eft::ConnectedMooreBlockOptions& action) {
  using namespace ftd::eft;
  FractionalCheckpoint result;
  result.matter_momentum=matter_momentum(state);
  CudaMatchedFieldPipeline pipeline(state.electric.L);
  if(!pipeline.valid()
      ||!pipeline.upload(state.electric,state.magnetic_half)) return result;
  const auto views=pipeline.resident_views();
  if(!views.electric_before.valid()||!views.magnetic_before.valid())
    return result;
  const auto matter=geometry_only(state);
  StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width=4;
  observer.shell_radii={8,12,16,24,32,48};
  observer.wave_speed=action.wave_speed;
  observer.dt=action.dt;
  observer.allow_fractional_center=true;
  CudaStateOnlySupportLadderTelemetry observer_telemetry;
  const auto field=observe_state_only_matter_field_cuda_resident(
      matter,action,views.electric_before,views.magnetic_before,
      observer,&observer_telemetry);
  CudaStateOnlySupportLadderTelemetry ladder_telemetry;
  const auto ladder=observe_state_only_support_ladder_cuda_resident(
      matter,action,views.electric_before,{4,6,8},1e-13,4096,
      kFtd0763Gate,&ladder_telemetry,true);
  result.observer_valid=field.valid;
  result.boundary_ledger_valid=field.boundary_energy_ledger_valid;
  result.ladder_valid=ladder.valid;
  result.cuda_scalar_only=observer_telemetry.complete_field_downloads==0
      &&ladder_telemetry.complete_field_downloads==0;
  result.center=field.center;
  result.support_center=field.support_center;
  result.center_offset=field.fractional_center_offset;
  result.kinetic_energy=field.constituent_kinetic_energy;
  result.internal_energy=field.pair_internal_energy;
  result.bound_energy=field.bound_energy;
  result.residual_energy=field.residual_energy;
  result.outgoing_energy=field.outgoing_energy;
  result.incoming_energy=field.incoming_energy;
  result.radial_energy=field.radial_energy;
  result.background_energy=field.background_energy;
  result.signed_radial_poynting=field.signed_radial_poynting;
  result.primitive_interference=field.primitive_face_interference;
  result.induced_boundary_interference=
      field.induced_boundary_interference;
  result.boundary_flux_sum=field.boundary_flux_sum;
  result.boundary_identity_residual=
      field.primitive_boundary_identity_residual;
  result.readout_reconstruction_residual=
      field.readout_interference_reconstruction_residual;
  result.maximum_reconstruction_residual=
      field.maximum_reconstruction_residual;
  result.actual_gauss_residual=field.actual_gauss_compatibility_residual;
  result.energy_partition_residual=field.energy_partition_residual;
  result.characteristic_flux_residual=field.characteristic_flux_residual;
  result.ladder_energy_residual=
      ladder.maximum_energy_reconstruction_residual;
  result.ladder_projection_residual=ladder.maximum_projection_residual;
  result.shells=field.shells;
  result.observer_error=observer_telemetry.error;
  result.ladder_error=ladder_telemetry.error;
  result.host_to_device_bytes=observer_telemetry.host_to_device_bytes
      +ladder_telemetry.host_to_device_bytes;
  result.device_to_host_bytes=observer_telemetry.device_to_host_bytes
      +ladder_telemetry.device_to_host_bytes;
  result.kernel_ms=observer_telemetry.kernel_ms+ladder_telemetry.kernel_ms;
  result.valid=result.observer_valid&&result.boundary_ledger_valid
      &&result.ladder_valid&&result.cuda_scalar_only
      &&result.center_offset.mag()<=std::sqrt(3.0)*0.5+kFtd0763Gate
      &&(result.center-result.support_center-result.center_offset).mag()
          <=kFtd0763Gate;
  return result;
}

struct FractionalReplay {
  std::string slug;
  std::string direction;
  bool parent_valid = false;
  bool replay_executed = false;
  bool common_action = false;
  FractionalCheckpoint tick160;
  FractionalCheckpoint tick224;
  ftd::Vec3 boosted_initial_momentum{};
  ftd::Vec3 momentum_defect{};
  ftd::Vec3 displacement{};
  bool pass = false;
};

FractionalReplay run_fractional_replay(const std::string& slug,int L,
                                       int ticks) {
  using namespace ftd;
  using namespace ftd::eft;
  FractionalReplay result;
  result.slug=slug;
  ForensicDirection direction;
  if(!select_direction(slug,direction)) return result;
  result.direction=direction.label;
  const auto normalization=measure_face_flux_normalization();
  if(!normalization.valid) return result;
  auto options=forensic_options();
  auto parent=build_parent(
      L,direction,options,normalization.mapped_field_work_coefficient);
  result.parent_valid=parent.valid;
  if(!parent.valid) return result;
  result.tick160=observe_fractional_checkpoint(parent.state,options);
  const Vec3 initial_center=object_center(parent.state);
  auto boosted=parent.state;
  for(auto& point:boosted.constituents)
    point.momentum+=direction_unit(direction)*kBoost;
  result.boosted_initial_momentum=matter_momentum(boosted);
  ForensicCudaStepper stepper(
      std::move(boosted),options,
      normalization.mapped_field_work_coefficient);
  if(!stepper.valid()) return result;
  bool common=true;
  int completed=0;
  for(int tick=1;tick<=ticks;++tick) {
    const auto step=stepper.advance();
    common=common&&step.valid&&step.common;
    if(!step.valid) break;
    ++completed;
  }
  result.replay_executed=completed==ticks;
  result.common_action=result.replay_executed&&common;
  const auto evolved=stepper.release_state();
  result.tick224=observe_fractional_checkpoint(evolved,options);
  result.displacement=object_center(evolved)-initial_center;
  result.momentum_defect=matter_momentum(evolved)
      -result.boosted_initial_momentum;
  result.pass=result.parent_valid&&result.replay_executed
      &&result.common_action&&result.tick160.valid&&result.tick224.valid
      &&result.tick224.center_offset.mag()>kFtd0763Gate;
  return result;
}

std::filesystem::path ftd0763_results_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results"/"ftd_0763";
}

void write_vec(std::ostream& output,const ftd::Vec3& value) {
  output<<'['<<json_number(value.x)<<", "<<json_number(value.y)
        <<", "<<json_number(value.z)<<']';
}

void write_checkpoint(std::ostream& output,const char* name,
                      const FractionalCheckpoint& value) {
  output<<"  \""<<name<<"\": {\n"
        <<"    \"valid\": "<<value.valid<<",\n"
        <<"    \"observer_valid\": "<<value.observer_valid<<",\n"
        <<"    \"boundary_ledger_valid\": "
        <<value.boundary_ledger_valid<<",\n"
        <<"    \"ladder_valid\": "<<value.ladder_valid<<",\n"
        <<"    \"cuda_scalar_only\": "<<value.cuda_scalar_only<<",\n"
        <<"    \"center\": "; write_vec(output,value.center);
  output<<",\n    \"support_center\": ";
  write_vec(output,value.support_center);
  output<<",\n    \"fractional_center_offset\": ";
  write_vec(output,value.center_offset);
  output<<",\n    \"fractional_center_norm\": "
        <<json_number(value.center_offset.mag())<<",\n"
        <<"    \"matter_momentum\": ";
  write_vec(output,value.matter_momentum);
  output<<",\n    \"kinetic_energy\": "<<json_number(value.kinetic_energy)
        <<",\n    \"internal_energy\": "<<json_number(value.internal_energy)
        <<",\n    \"bound_energy\": "<<json_number(value.bound_energy)
        <<",\n    \"residual_energy\": "<<json_number(value.residual_energy)
        <<",\n    \"outgoing_energy\": "<<json_number(value.outgoing_energy)
        <<",\n    \"incoming_energy\": "<<json_number(value.incoming_energy)
        <<",\n    \"radial_energy\": "<<json_number(value.radial_energy)
        <<",\n    \"background_energy\": "<<json_number(value.background_energy)
        <<",\n    \"signed_radial_poynting\": "
        <<json_number(value.signed_radial_poynting)
        <<",\n    \"primitive_interference\": "
        <<json_number(value.primitive_interference)
        <<",\n    \"induced_boundary_interference\": "
        <<json_number(value.induced_boundary_interference)
        <<",\n    \"boundary_flux_sum\": "
        <<json_number(value.boundary_flux_sum)
        <<",\n    \"boundary_identity_residual\": "
        <<json_number(value.boundary_identity_residual)
        <<",\n    \"readout_reconstruction_residual\": "
        <<json_number(value.readout_reconstruction_residual)
        <<",\n    \"maximum_reconstruction_residual\": "
        <<json_number(value.maximum_reconstruction_residual)
        <<",\n    \"actual_gauss_residual\": "
        <<json_number(value.actual_gauss_residual)
        <<",\n    \"energy_partition_residual\": "
        <<json_number(value.energy_partition_residual)
        <<",\n    \"characteristic_flux_residual\": "
        <<json_number(value.characteristic_flux_residual)
        <<",\n    \"ladder_energy_residual\": "
        <<json_number(value.ladder_energy_residual)
        <<",\n    \"ladder_projection_residual\": "
        <<json_number(value.ladder_projection_residual)
        <<",\n    \"observer_error\": \""<<value.observer_error
        <<"\",\n    \"ladder_error\": \""<<value.ladder_error
        <<"\",\n    \"host_to_device_bytes\": "
        <<value.host_to_device_bytes
        <<",\n    \"device_to_host_bytes\": "
        <<value.device_to_host_bytes
        <<",\n    \"kernel_ms\": "<<json_number(value.kernel_ms)
        <<",\n    \"shells\": [";
  for(std::size_t i=0;i<value.shells.size();++i) {
    const auto& shell=value.shells[i];
    if(i!=0) output<<',';
    output<<"\n      {\"radius\": "<<shell.radius
          <<", \"samples\": "<<shell.samples
          <<", \"residual_energy\": "<<json_number(shell.residual_energy)
          <<", \"outgoing_energy\": "<<json_number(shell.outgoing_energy)
          <<", \"incoming_energy\": "<<json_number(shell.incoming_energy)
          <<", \"radial_energy\": "<<json_number(shell.radial_energy)
          <<", \"background_energy\": "
          <<json_number(shell.background_energy)
          <<", \"signed_radial_poynting\": "
          <<json_number(shell.signed_radial_poynting)<<'}';
  }
  output<<"\n    ]\n  }";
}

void write_fractional_result(const FractionalReplay& value) {
  const auto directory=ftd0763_results_directory();
  std::filesystem::create_directories(directory);
  std::ofstream output(directory/
      ("ftd_0763_fractional_center_dressing_observer_v1_"
       +value.slug+".json"));
  output<<std::boolalpha<<std::setprecision(17)
        <<"{\n  \"ftd_id\": \"FTD-0763\",\n"
        <<"  \"protocol_sha256\": \""<<kFtd0763ProtocolSha256
        <<"\",\n  \"slug\": \""<<value.slug
        <<"\",\n  \"direction\": \""<<value.direction
        <<"\",\n  \"volume\": "<<kFtd0763Volume
        <<",\n  \"formation_tick\": "<<kFormationTick
        <<",\n  \"transport_ticks\": "<<kFtd0763TransportTicks
        <<",\n  \"boost\": "<<kBoost
        <<",\n  \"parent_valid\": "<<value.parent_valid
        <<",\n  \"replay_executed\": "<<value.replay_executed
        <<",\n  \"common_action\": "<<value.common_action
        <<",\n  \"boosted_initial_momentum\": ";
  write_vec(output,value.boosted_initial_momentum);
  output<<",\n  \"matter_momentum_defect\": ";
  write_vec(output,value.momentum_defect);
  output<<",\n  \"center_displacement\": ";
  write_vec(output,value.displacement);
  output<<",\n  \"pass\": "<<value.pass<<",\n";
  write_checkpoint(output,"tick_160",value.tick160);
  output<<",\n";
  write_checkpoint(output,"tick_224",value.tick224);
  output<<",\n  \"production_changed\": false,\n"
        <<"  \"dynamics_changed\": false,\n"
        <<"  \"co_moving_dressing_claimed\": false\n}\n";
}

void write_fractional_aggregate() {
  const auto directory=ftd0763_results_directory();
  const std::array<std::string,3> slugs{{"face","edge","body"}};
  bool complete=true,pass=true;
  for(const auto& slug:slugs) {
    const auto path=directory/
        ("ftd_0763_fractional_center_dressing_observer_v1_"
         +slug+".json");
    complete=complete&&std::filesystem::is_regular_file(path);
    pass=pass&&read_bit(path,"pass");
  }
  const std::string verdict=complete&&pass
      ?"FRACTIONAL_CENTER_OBSERVER_CONSTRUCTED"
      :"FRACTIONAL_CENTER_OBSERVER_CLOSED";
  std::ofstream output(directory/
      "ftd_0763_fractional_center_dressing_observer_v1.json");
  output<<std::boolalpha
        <<"{\n  \"ftd_id\": \"FTD-0763\",\n"
        <<"  \"protocol_sha256\": \""<<kFtd0763ProtocolSha256
        <<"\",\n  \"verdict\": \""<<verdict
        <<"\",\n  \"all_artifacts_present\": "<<complete
        <<",\n  \"all_rays_pass\": "<<pass
        <<",\n  \"qualification_test\": "
          "\"test_cuda_fractional_center_state_only_observer\",\n"
        <<"  \"production_changed\": false,\n"
        <<"  \"dynamics_changed\": false,\n"
        <<"  \"co_moving_dressing_claimed\": false\n}\n";
}

int run_registered_fractional(const std::string& slug) {
  if(std::string(kFtd0763ProtocolSha256)=="UNLOCKED") return 3;
  if(slug=="body") for(const auto& prior:{"face","edge"}) {
    const auto path=ftd0763_results_directory()/
        (std::string("ftd_0763_fractional_center_dressing_observer_v1_")
         +prior+".json");
    if(!std::filesystem::is_regular_file(path)) return 4;
  }
  const auto result=run_fractional_replay(
      slug,kFtd0763Volume,kFtd0763TransportTicks);
  write_fractional_result(result);
  if(slug=="body") write_fractional_aggregate();
  std::cout<<std::boolalpha<<std::setprecision(17)
           <<"FTD-0763 direction="<<slug
           <<" common="<<result.common_action
           <<" tick160="<<result.tick160.valid
           <<" tick224="<<result.tick224.valid
           <<" fractional="<<result.tick224.center_offset.mag()
           <<" displacement="<<result.displacement.mag()
           <<" momentum_defect="<<result.momentum_defect.mag()
           <<" pass="<<result.pass<<'\n';
  return result.pass?0:1;
}

}  // namespace

int main(int argc,char** argv) {
  if(argc==3&&std::string(argv[1])=="--run")
    return run_registered_fractional(argv[2]);
  std::cout<<"FTD-0763 runner: --run face|edge|body\n";
  return argc==1?0:2;
}
