// FTD-0639: common-action rest and state-only inversion of FTD-0638.

#define FTD_0638_EMBEDDED
#include "test_connected_block_analytic_static_refinement.cpp"
#undef FTD_0638_EMBEDDED

#include <future>
#include <map>
#include <tuple>

namespace {
constexpr char rest_protocol_sha256[] =
    "28B9E9415C49FD989A4FBC60B33D9588E8F6A13A52B78E36881A9379F18D8AF3";
constexpr char rest_parent_result_sha256[] =
    "435493EDC8E5DA5B34CF416EB6445C537A1F6ED9ABFCE02BB032DE2486C1B18C";
constexpr int rest_ticks = 128;

struct TickRecord { int orientation=0,direction=0,tick=0;double state_distance=0,center_distance=0,energy_drift=0,residual=0,impulse=0; };
struct RestArm {
  bool valid=false,coverage=false,sector_preserved=false,no_hops=false;
  int orientation=0,maximum_multiplicity=0,total_hops=0;
  double maximum_impulse=0,maximum_state_excursion=0,
      maximum_center_displacement=0,maximum_energy_drift=0,
      maximum_common_residual=0,recovery=INFINITY;
  std::vector<TickRecord> records;
};
struct RestSummary {
  bool parent=false,normalization=false,covariance=false;
  double covariance_residual=INFINITY;
  std::string verdict="CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_EXECUTION_INVALID";
  std::vector<RestArm> arms;
};

bool rest_parent_fingerprint() {
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_v1.json";
  std::ifstream input(path,std::ios::binary);const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  return bytes.find(refinement_protocol_sha256)!=std::string::npos
      && bytes.find("CONNECTED_BLOCK_ANALYTIC_STATIC_BASIN_CONSTRUCTIVE")!=std::string::npos;
}
int multiplicity(const ftd::eft::ConnectedMooreBlockState& state) {
  std::map<std::tuple<int,int,int>,int> counts;int result=0;for(const auto&p:state.constituents)result=std::max(result,++counts[{p.anchor.x,p.anchor.y,p.anchor.z}]);return result;
}
double common_residual(const ftd::eft::ConnectedMooreBlockStepResult& step) {
  return std::max({step.root_residual,step.continuity_residual,step.gauss_before_residual,
      step.gauss_after_residual,step.force_residual,step.kinematic_residual,
      step.kinetic_discrete_gradient_residual,step.electric_adjoint_residual,
      step.magnetic_work_residual,step.binding_work_residual,
      step.binding_impulse_sum_residual,step.matter_work_residual,
      step.field_work_residual,step.total_energy_residual,step.causal_speed_excess});
}
double total_before(const ftd::eft::ConnectedMooreBlockStepResult& step) { return step.kinetic_energy_before+step.binding_energy_before+step.field_energy_before; }
double total_after(const ftd::eft::ConnectedMooreBlockStepResult& step) { return step.kinetic_energy_after+step.binding_energy_after+step.field_energy_after; }
ftd::eft::ConnectedMooreBlockState load_refined_state(int orientation) {
  const auto initialized=ftd::eft::initialize_connected_moore_block(L,2,orientation,orientation,.5,1e-13,4096);if(!initialized.valid)return ftd::eft::ConnectedMooreBlockState{};auto geometry=initialized.state;
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_states_v1.csv";
  std::ifstream input(path);std::string line;std::getline(input,line);int loaded=0;
  while(std::getline(input,line)){std::stringstream row(line);std::array<std::string,10> fields;for(std::size_t i=0;i<fields.size();++i)std::getline(row,fields[i],',');if(std::stoi(fields[1])!=orientation)continue;const int p=std::stoi(fields[2]);geometry.constituents[p]=point_at({std::stod(fields[7]),std::stod(fields[8]),std::stod(fields[9])});geometry.constituents[p].momentum={};++loaded;}
  if(loaded!=count)return ftd::eft::ConnectedMooreBlockState{};const auto dressed=ftd::eft::redress_connected_moore_block_with_fibre_limit(geometry,8,1e-13,4096);return dressed.valid?dressed.state:ftd::eft::ConnectedMooreBlockState{};
}
void observe(RestArm& arm,const ftd::eft::ConnectedMooreBlockState& initial,
             const ftd::eft::ConnectedMooreBlockState& state,
             const ftd::eft::ConnectedMooreBlockStepResult& step,
             double initial_energy,int direction,int tick,
             const std::vector<int>& initial_sector) {
  TickRecord record;record.orientation=arm.orientation;record.direction=direction;record.tick=tick;record.state_distance=ftd::eft::connected_moore_block_state_max_difference(initial,state);record.center_distance=(center(state)-center(initial)).mag();record.energy_drift=std::abs(total_after(step)-initial_energy);record.residual=common_residual(step);for(const auto& impulse:step.total_impulses)record.impulse=std::max(record.impulse,impulse.mag());arm.maximum_impulse=std::max(arm.maximum_impulse,record.impulse);arm.maximum_state_excursion=std::max(arm.maximum_state_excursion,record.state_distance);arm.maximum_center_displacement=std::max(arm.maximum_center_displacement,record.center_distance);arm.maximum_energy_drift=std::max(arm.maximum_energy_drift,record.energy_drift);arm.maximum_common_residual=std::max(arm.maximum_common_residual,record.residual);arm.maximum_multiplicity=std::max(arm.maximum_multiplicity,multiplicity(state));arm.total_hops+=step.site_hops;arm.sector_preserved=arm.sector_preserved&&sector_signature(state)==initial_sector;arm.records.push_back(record);
}
RestArm run_rest(int orientation,const ftd::eft::ConnectedMooreBlockOptions& options) {
  RestArm arm;arm.orientation=orientation;arm.sector_preserved=true;auto initial=load_refined_state(orientation);if(initial.electric.L!=L)return arm;arm.maximum_multiplicity=multiplicity(initial);const auto initial_sector=sector_signature(initial);auto state=initial;bool common=true;double initial_energy=NAN;
  for(int tick=1;tick<=rest_ticks;++tick){const auto step=ftd::eft::solve_connected_moore_block_forward(state,options);if(!step.valid||!step.common_action_gates_pass){common=false;break;}if(tick==1)initial_energy=total_before(step);state=step.later;observe(arm,initial,state,step,initial_energy,+1,tick,initial_sector);}
  for(int tick=1;common&&tick<=rest_ticks;++tick){const auto step=ftd::eft::solve_connected_moore_block_reverse(state,options);if(!step.valid||!step.common_action_gates_pass){common=false;break;}state=step.earlier;observe(arm,initial,state,step,initial_energy,-1,tick,initial_sector);}
  arm.recovery=ftd::eft::connected_moore_block_state_max_difference(initial,state);arm.coverage=arm.records.size()==2*rest_ticks;arm.no_hops=arm.total_hops==0;arm.valid=common&&arm.coverage&&arm.sector_preserved&&arm.no_hops&&arm.maximum_multiplicity<=8&&arm.maximum_impulse<=1e-9&&arm.maximum_state_excursion<=1e-8&&arm.maximum_center_displacement<=1e-10&&arm.maximum_energy_drift<=1e-12&&arm.maximum_common_residual<=1e-10&&arm.recovery<=1e-10;return arm;
}
void write_rest(const RestSummary& s) {
  const auto dir=std::filesystem::path(__FILE__).parent_path().parent_path()/"results/ftd_0639";std::filesystem::create_directories(dir);std::ofstream json(dir/"ftd_0639_connected_block_analytic_dynamical_rest_v1.json");json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0639\",\n  \"protocol_sha256\": \""<<rest_protocol_sha256<<"\",\n  \"parent_result_sha256\": \""<<rest_parent_result_sha256<<"\",\n  \"verdict\": \""<<s.verdict<<"\",\n  \"production_changed\": false,\n  \"ticks_each_direction\": "<<rest_ticks<<",\n  \"covariance_residual\": "<<s.covariance_residual<<"\n}\n";
  std::ofstream arms(dir/"ftd_0639_connected_block_analytic_dynamical_rest_arms_v1.csv");arms<<"ftd_id,orientation,valid,coverage,sector_preserved,no_hops,max_multiplicity,total_hops,max_impulse,max_state_excursion,max_center_displacement,max_energy_drift,max_common_residual,recovery\n";for(const auto&a:s.arms)arms<<std::setprecision(17)<<"FTD-0639,"<<a.orientation<<','<<a.valid<<','<<a.coverage<<','<<a.sector_preserved<<','<<a.no_hops<<','<<a.maximum_multiplicity<<','<<a.total_hops<<','<<a.maximum_impulse<<','<<a.maximum_state_excursion<<','<<a.maximum_center_displacement<<','<<a.maximum_energy_drift<<','<<a.maximum_common_residual<<','<<a.recovery<<'\n';
  std::ofstream ticks(dir/"ftd_0639_connected_block_analytic_dynamical_rest_ticks_v1.csv");ticks<<"ftd_id,orientation,direction,tick,state_distance,center_distance,energy_drift,residual,impulse\n";for(const auto&a:s.arms)for(const auto&r:a.records)ticks<<std::setprecision(17)<<"FTD-0639,"<<r.orientation<<','<<r.direction<<','<<r.tick<<','<<r.state_distance<<','<<r.center_distance<<','<<r.energy_drift<<','<<r.residual<<','<<r.impulse<<'\n';
}
}

#ifdef FTD_0639_EMBEDDED
int ftd_0639_embedded_main() {
#else
int main() {
#endif
  RestSummary summary;summary.parent=rest_parent_fingerprint();const auto normalization=ftd::eft::measure_face_flux_normalization();summary.normalization=normalization.valid;ftd::eft::ConnectedMooreBlockOptions options;options.allow_shared_anchor_chart=true;
  if(summary.parent&&summary.normalization){auto x=std::async(std::launch::async,[&](){return run_rest(0,options);});auto y=std::async(std::launch::async,[&](){return run_rest(1,options);});summary.arms.push_back(x.get());std::cout<<"completed rest_x\n";summary.arms.push_back(y.get());std::cout<<"completed rest_y\n";}
  if(summary.arms.size()==2){const auto&x=summary.arms[0];const auto&y=summary.arms[1];summary.covariance_residual=std::max({std::abs(x.maximum_impulse-y.maximum_impulse),std::abs(x.maximum_state_excursion-y.maximum_state_excursion),std::abs(x.maximum_center_displacement-y.maximum_center_displacement),std::abs(x.maximum_energy_drift-y.maximum_energy_drift),std::abs(x.recovery-y.recovery)});summary.covariance=summary.covariance_residual<=1e-9;const bool valid=std::all_of(summary.arms.begin(),summary.arms.end(),[](const RestArm&a){return a.valid;});const bool executed=std::all_of(summary.arms.begin(),summary.arms.end(),[](const RestArm&a){return a.coverage;});if(valid&&summary.covariance)summary.verdict="CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_CONSTRUCTIVE";else if(executed&&summary.covariance)summary.verdict="CONNECTED_BLOCK_ANALYTIC_STATIC_ONLY";}
  write_rest(summary);std::cout<<std::setprecision(17)<<"protocol_sha256="<<rest_protocol_sha256<<'\n'<<"verdict="<<summary.verdict<<'\n'<<"covariance="<<summary.covariance_residual<<'\n';for(const auto&a:summary.arms)std::cout<<"orientation="<<a.orientation<<" valid="<<a.valid<<" records="<<a.records.size()<<" impulse="<<a.maximum_impulse<<" state="<<a.maximum_state_excursion<<" center="<<a.maximum_center_displacement<<" energy="<<a.maximum_energy_drift<<" residual="<<a.maximum_common_residual<<" recovery="<<a.recovery<<'\n';return summary.verdict=="CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_EXECUTION_INVALID"?1:0;
}
