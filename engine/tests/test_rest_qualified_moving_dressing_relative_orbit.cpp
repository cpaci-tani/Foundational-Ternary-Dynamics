// FTD-0709: rerun the complete v=1/2 relative-orbit test from the qualified
// L=33 full-coordinate rest state.

#define FTD_0704_EMBEDDED
#include "test_connected_dressed_matter_high_speed_preflight.cpp"
#undef FTD_0704_EMBEDDED

namespace {

constexpr char rqorbit_protocol_sha256[] =
    "14AE617CE7D5EA4F4617FAB667F34CFE339309512B2D9E2D1BE97C946D47A74E";
constexpr char rqorbit_parent_protocol_sha256[] =
    "D978E8920D8121CA2FC91F3E6B4F68353B98E7B6285B4A82304511EE4177D007";
constexpr int rqorbit_ticks = 2;

struct RqOrbitRun {
  bool initialized=false,forward=false,reverse=false;
  int hops=0;
  double energy=0,common=0,recovery=INFINITY;
  ftd::eft::ConnectedMooreBlockState initial,final;
};

struct RqOrbitSummary {
  bool parent=false,normalization=false,reconstruction=false;
  bool execution=false,inverse=false,rest=false,covariance=false;
  double position=INFINITY,momentum=INFINITY,electric=INFINITY;
  double magnetic=INFINITY,complete=INFINITY,rest_residual=INFINITY;
  double covariance_residual=INFINITY;
  RqOrbitRun moving;
  std::string verdict=
      "REST_QUALIFIED_MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID";
};

bool rqorbit_parent_fingerprint(){
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0708/ftd_0708_l33_full_impulse_rest_solve_v1.json";
  std::ifstream input(path,std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  return bytes.find(rqorbit_parent_protocol_sha256)!=std::string::npos
      &&bytes.find("L33_FULL_IMPULSE_REST_FIXED_POINT_CONSTRUCTIVE")
          !=std::string::npos;
}

double rqorbit_max_component(const Vec3&v){
  return std::max({std::abs(v.x),std::abs(v.y),std::abs(v.z)});
}

ftd::eft::ConnectedMooreBlockState rqorbit_reference(bool&valid){
  valid=false;auto geometry=preflight_reference();
  if(geometry.electric.L!=preflight_volume)
    return ftd::eft::ConnectedMooreBlockState(0);
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0708/ftd_0708_l33_full_impulse_rest_solve_state_v1.csv";
  std::ifstream input(path);std::string line;std::getline(input,line);int loaded=0;
  while(std::getline(input,line)){
    std::stringstream row(line);std::array<std::string,9> fields;
    for(auto&field:fields)std::getline(row,field,',');
    if(fields[0]!="FTD-0708")continue;
    const int particle=std::stoi(fields[1]);
    if(particle<0||particle>=count||std::stoi(fields[2])!=geometry.charges[particle])
      return ftd::eft::ConnectedMooreBlockState(0);
    const Vec3 x{std::stod(fields[3]),std::stod(fields[4]),std::stod(fields[5])};
    geometry.constituents[particle]=preflight_point_at(x,preflight_volume);
    geometry.constituents[particle].momentum={};++loaded;
  }
  if(loaded!=count)return ftd::eft::ConnectedMooreBlockState(0);
  const auto dressed=ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry,8,1e-13,4096);
  valid=dressed.valid;return dressed.valid?dressed.state:
      ftd::eft::ConnectedMooreBlockState{};
}

ftd::eft::ConnectedMooreBlockState rqorbit_translate(
    const ftd::eft::ConnectedMooreBlockState&source,int dx){
  auto result=source;const int L=source.electric.L;
  for(auto&point:result.constituents)
    point.anchor.x=preflight_wrap(point.anchor.x+dx,L);
  for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z){
    const int from=source.electric.index(x,y,z),to=result.electric.index(x+dx,y,z);
    result.electric.x[to]=source.electric.x[from];
    result.electric.y[to]=source.electric.y[from];
    result.electric.z[to]=source.electric.z[from];
    result.magnetic_half.x[to]=source.magnetic_half.x[from];
    result.magnetic_half.y[to]=source.magnetic_half.y[from];
    result.magnetic_half.z[to]=source.magnetic_half.z[from];
  }return result;
}

double rqorbit_position_difference(
    const ftd::eft::ConnectedMooreBlockState&a,
    const ftd::eft::ConnectedMooreBlockState&b){
  if(a.electric.L!=b.electric.L||a.constituents.size()!=b.constituents.size())
    return INFINITY;double result=0,L=a.electric.L;
  for(std::size_t i=0;i<a.constituents.size();++i){
    Vec3 d=position(a.constituents[i])-position(b.constituents[i]);
    d.x-=std::round(d.x/L)*L;d.y-=std::round(d.y/L)*L;
    d.z-=std::round(d.z/L)*L;result=std::max(result,rqorbit_max_component(d));
  }return result;
}

double rqorbit_momentum_difference(
    const ftd::eft::ConnectedMooreBlockState&a,
    const ftd::eft::ConnectedMooreBlockState&b){
  if(a.constituents.size()!=b.constituents.size())return INFINITY;double result=0;
  for(std::size_t i=0;i<a.constituents.size();++i)
    result=std::max(result,rqorbit_max_component(
        a.constituents[i].momentum-b.constituents[i].momentum));
  return result;
}

RqOrbitRun rqorbit_run(const ftd::eft::ConnectedMooreBlockState&initial,
    double beta,const ftd::eft::ConnectedMooreBlockOptions&options){
  RqOrbitRun run;run.initial=initial;run.initialized=initial.electric.L==preflight_volume
      &&initial.constituents.size()==count;if(!run.initialized)return run;
  const double e0=preflight_energy(initial,beta,options);auto state=initial;
  run.forward=true;ftd::eft::ConnectedMooreBlockSolveCache fc;
  for(int tick=0;tick<rqorbit_ticks&&run.forward;++tick){
    const auto step=ftd::eft::solve_connected_moore_block_forward(state,options,&fc);
    const double common=common_residual(step);
    if(!step.valid||!step.common_action_gates_pass||common>1e-10){run.forward=false;break;}
    state=step.later;run.hops+=step.site_hops;run.common=std::max(run.common,common);
    run.energy=std::max(run.energy,std::abs(preflight_energy(state,beta,options)-e0));
  }run.final=state;run.reverse=run.forward;ftd::eft::ConnectedMooreBlockSolveCache rc;
  for(int tick=0;tick<rqorbit_ticks&&run.reverse;++tick){
    const auto step=ftd::eft::solve_connected_moore_block_reverse(state,options,&rc);
    const double common=common_residual(step);
    if(!step.valid||!step.common_action_gates_pass||common>1e-10){run.reverse=false;break;}
    state=step.earlier;run.common=std::max(run.common,common);
    run.energy=std::max(run.energy,std::abs(preflight_energy(state,beta,options)-e0));
  }if(run.reverse)run.recovery=ftd::eft::connected_moore_block_state_max_difference(
      initial,state);return run;
}

void rqorbit_evaluate(RqOrbitSummary&s,
    const ftd::eft::ConnectedMooreBlockState&reference,double beta,
    const ftd::eft::ConnectedMooreBlockOptions&options){
  const auto rest_run=rqorbit_run(reference,beta,options);
  if(rest_run.forward&&rest_run.reverse)s.rest_residual=
      ftd::eft::connected_moore_block_state_max_difference(reference,rest_run.final);
  s.rest=rest_run.forward&&rest_run.reverse&&rest_run.energy<=1e-10
      &&rest_run.common<=1e-10&&rest_run.recovery<=1e-9&&s.rest_residual<=1e-9;

  auto moving=reference;const Vec3 p=ftd::eft::production_flat_momentum({.5,0,0});
  for(auto&point:moving.constituents)point.momentum=p;
  s.moving=rqorbit_run(moving,beta,options);
  if(s.moving.forward){const auto target=rqorbit_translate(moving,1);
    s.position=rqorbit_position_difference(s.moving.final,target);
    s.momentum=rqorbit_momentum_difference(s.moving.final,target);
    s.electric=ftd::eft::matched_face_max_difference(s.moving.final.electric,target.electric);
    s.magnetic=ftd::eft::matched_edge_max_difference(s.moving.final.magnetic_half,target.magnetic_half);
    s.complete=ftd::eft::connected_moore_block_state_max_difference(s.moving.final,target);}
  s.inverse=s.moving.reverse&&s.moving.recovery<=1e-9;

  const auto shifted=rqorbit_translate(moving,3);
  const auto shifted_run=rqorbit_run(shifted,beta,options);
  if(s.moving.forward&&shifted_run.forward)s.covariance_residual=
      ftd::eft::connected_moore_block_state_max_difference(
          shifted_run.final,rqorbit_translate(s.moving.final,3));
  s.covariance=shifted_run.forward&&shifted_run.reverse
      &&shifted_run.energy<=1e-10&&shifted_run.common<=1e-10
      &&shifted_run.recovery<=1e-9&&s.covariance_residual<=1e-9;
  s.execution=s.parent&&s.normalization&&s.reconstruction&&s.rest&&s.inverse
      &&s.covariance&&s.moving.forward&&s.moving.energy<=1e-10
      &&s.moving.common<=1e-10;
  if(!s.execution)s.verdict=
      "REST_QUALIFIED_MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID";
  else if(s.position<=1e-9&&s.momentum<=1e-9&&s.electric<=1e-9
      &&s.magnetic<=1e-9&&s.complete<=1e-9)s.verdict=
      "REST_QUALIFIED_COMPLETE_MOVING_DRESSING_RELATIVE_ORBIT_CANDIDATE";
  else if(s.position<=.05&&s.momentum<=.05&&(s.electric>1e-6||s.magnetic>1e-6))
    s.verdict="REST_QUALIFIED_CORE_TRANSLATES_WITHOUT_COMPLETE_MOVING_DRESSING";
  else s.verdict="REST_QUALIFIED_STATIC_BOOST_HAS_NO_RELATIVE_ORBIT";
}

void rqorbit_write(const RqOrbitSummary&s){
  const auto directory=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0709";std::filesystem::create_directories(directory);
  std::ofstream json(directory/
      "ftd_0709_rest_qualified_moving_dressing_relative_orbit_v1.json");
  json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0709\",\n"
      <<"  \"protocol_sha256\": \""<<rqorbit_protocol_sha256<<"\",\n"
      <<"  \"parent_protocol_sha256\": \""<<rqorbit_parent_protocol_sha256<<"\",\n"
      <<"  \"verdict\": \""<<s.verdict<<"\",\n"
      <<"  \"production_changed\": false,\n  \"volume\": "<<preflight_volume<<",\n"
      <<"  \"ticks\": "<<rqorbit_ticks<<",\n  \"parent_pass\": "<<s.parent<<",\n"
      <<"  \"reconstruction_pass\": "<<s.reconstruction<<",\n"
      <<"  \"execution_pass\": "<<s.execution<<",\n"
      <<"  \"rest_pass\": "<<s.rest<<",\n  \"inverse_pass\": "<<s.inverse<<",\n"
      <<"  \"covariance_pass\": "<<s.covariance<<",\n"
      <<"  \"total_hops\": "<<s.moving.hops<<",\n"
      <<"  \"position_residual\": "<<s.position<<",\n"
      <<"  \"momentum_residual\": "<<s.momentum<<",\n"
      <<"  \"electric_residual\": "<<s.electric<<",\n"
      <<"  \"magnetic_residual\": "<<s.magnetic<<",\n"
      <<"  \"complete_residual\": "<<s.complete<<",\n"
      <<"  \"maximum_energy_drift\": "<<s.moving.energy<<",\n"
      <<"  \"maximum_common_residual\": "<<s.moving.common<<",\n"
      <<"  \"inverse_residual\": "<<s.moving.recovery<<",\n"
      <<"  \"rest_residual\": "<<s.rest_residual<<",\n"
      <<"  \"covariance_residual\": "<<s.covariance_residual<<"\n}\n";
  std::ofstream csv(directory/
      "ftd_0709_rest_qualified_moving_dressing_relative_orbit_metrics_v1.csv");
  csv<<"ftd_id,verdict,total_hops,position_residual,momentum_residual,electric_residual,magnetic_residual,complete_residual,max_energy_drift,max_common_residual,inverse_residual,rest_residual,covariance_residual\n"
     <<std::setprecision(17)<<"FTD-0709,"<<s.verdict<<','<<s.moving.hops<<','
     <<s.position<<','<<s.momentum<<','<<s.electric<<','<<s.magnetic<<','
     <<s.complete<<','<<s.moving.energy<<','<<s.moving.common<<','
     <<s.moving.recovery<<','<<s.rest_residual<<','<<s.covariance_residual<<'\n';
}

} // namespace

int main(){RqOrbitSummary s;s.parent=rqorbit_parent_fingerprint();
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  s.normalization=normalization.valid;auto reference=rqorbit_reference(s.reconstruction);
  ftd::eft::ConnectedMooreBlockOptions options;options.allow_shared_anchor_chart=true;
  options.use_sparse_local_current=true;options.use_local_residual_evaluation=true;
  if(s.parent&&s.normalization&&s.reconstruction)rqorbit_evaluate(s,reference,
      normalization.mapped_field_work_coefficient,options);rqorbit_write(s);
  std::cout<<std::setprecision(17)<<"protocol_sha256="<<rqorbit_protocol_sha256<<'\n'
      <<"verdict="<<s.verdict<<'\n'<<"execution="<<s.execution<<" rest="<<s.rest
      <<" inverse="<<s.inverse<<" covariance="<<s.covariance<<'\n'
      <<"hops="<<s.moving.hops<<" position="<<s.position<<" momentum="<<s.momentum
      <<" electric="<<s.electric<<" magnetic="<<s.magnetic<<" complete="<<s.complete<<'\n'
      <<"energy="<<s.moving.energy<<" common="<<s.moving.common
      <<" inverse_residual="<<s.moving.recovery<<" rest_residual="<<s.rest_residual
      <<" covariance_residual="<<s.covariance_residual<<'\n';
  return s.verdict=="REST_QUALIFIED_MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID"?1:0;}
