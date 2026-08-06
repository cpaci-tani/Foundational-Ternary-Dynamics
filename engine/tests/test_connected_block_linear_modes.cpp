// FTD-0629: generalized linear modes about the FTD-0628 dressed fixed point.

#include "ftd/eft/connected_moore_block_action.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

namespace {

using ftd::Vec3;
using Theta=std::array<double,4>;
using Matrix=std::array<std::array<double,4>,4>;
constexpr char protocol_sha256[]=
    "BF823BB629BFAB7FA385E39AB83E4BDCC2DCA3E857EE424FAF65C3280898CB4F";
constexpr char parent_result_sha256[]=
    "7C4B9A8E71D1D10CE2D5409CB17F25F23695CD54A426812CDA918F2A7A45AA6A";
constexpr char parent_protocol_sha256[]=
    "4B6CA4AD4ACF106124AAF9C791AF4F7B3374DC30DF3A5A9FDEDC784F66D640C6";
constexpr int L=17,width=2,tick_count=64;
constexpr double action_gate=1e-10;
constexpr Theta theta0{{1.4993153663084844,0.4994670538459639,
                        0.50006590532229034,0.50018096647517352}};
constexpr Matrix hessian{{
  {{63.984246918488694,-64.008998464613882,64.005401398219135,64.022814124818296}},
  {{-64.008998464613882,191.66543224202746,-64.016058330196643,63.852374345947993}},
  {{64.005401398219135,-64.016058330196643,288.10845102769656,96.075177717647179}},
  {{64.022814124818296,63.852374345947993,96.075177717647179,480.33216079722968}}
}};
constexpr std::array<double,4> expected_lambda{{4.977203453881947,
    22.45450515280385,61.30026622242707,67.791994122203}};
constexpr std::array<double,4> expected_omega{{2.2309646913122467,
    4.738618485677429,7.829448653795941,8.233589382657055}};
constexpr std::array<double,4> expected_phase{{1.6798663354589545,
    2.3428303450922074,2.6413975634911466,2.66500817389935}};

struct Mode { double lambda=0.0,omega=0.0,phase=0.0;Theta vector{}; };
struct EigenSystem { bool valid=false;std::array<Mode,4> modes{};double orthogonality=INFINITY; };
struct Spec { std::string label;int orientation=0,mode=0,sign=1;double amplitude=0.0; };
struct Tick { int index=0,multiplicity=1,pairs=0;double separation=INFINITY;
  double center=INFINITY,drift=INFINITY,common=INFINITY;Theta theta{},q{}; };
struct Arm {
  Spec spec;bool initialization=false,forward=false,reverse=false,bounded=false;
  double initial_excess=INFINITY,phase=INFINITY,phase_error=INFINITY;
  double leakage=INFINITY,max_center=0.0,max_drift=0.0,max_common=0.0;
  double recovery=INFINITY,min_separation=INFINITY;int max_multiplicity=1;
  std::vector<Tick> ticks;
};
struct Summary {
  bool parent=false,normalization=false,eigensystem=false,coverage=false;
  bool execution=false,bounded=false,frequency=false,purity=false;
  bool amplitude=false,sign=false,covariance=false;
  double beta=0.0,orthogonality=INFINITY,amplitude_residual=INFINITY;
  double sign_residual=INFINITY,covariance_residual=INFINITY;
  double worst_common=0.0,worst_drift=0.0,worst_recovery=0.0;
  EigenSystem modes;std::vector<Arm> arms;std::string verdict;
};

double component(const Vec3& v,int axis){return axis==0?v.x:(axis==1?v.y:v.z);}
void set_component(Vec3& v,int axis,double x){if(axis==0)v.x=x;else if(axis==1)v.y=x;else v.z=x;}
Vec3 position(const ftd::eft::MatchedMatterPoint& p){return {p.anchor.x+p.remainder.x,p.anchor.y+p.remainder.y,p.anchor.z+p.remainder.z};}
Vec3 center(const ftd::eft::ConnectedMooreBlockState& s){Vec3 c{};for(const auto&p:s.constituents)c+=position(p);return c*(1.0/s.constituents.size());}
double max_component(const Vec3& v){return std::max({std::abs(v.x),std::abs(v.y),std::abs(v.z)});}
int wrap(int x){const int r=x%L;return r<0?r+L:r;}
ftd::eft::MatchedMatterPoint point_at(const Vec3& x){ftd::eft::MatchedMatterPoint p;
  const long long ax=std::llround(x.x),ay=std::llround(x.y),az=std::llround(x.z);
  p.anchor={wrap(static_cast<int>(ax)),wrap(static_cast<int>(ay)),wrap(static_cast<int>(az))};
  p.remainder={x.x-ax,x.y-ay,x.z-az};return p;}

ftd::eft::ConnectedMooreBlockState geometry_from(
    const ftd::eft::ConnectedMooreBlockState& base,const Theta& theta,int axis){
  auto result=base;const Vec3 c=center(base);
  for(std::size_t i=0;i<result.constituents.size();++i){const Vec3 d0=position(base.constituents[i])-c;
    const bool outer=std::abs(component(d0,axis))>1.0;Vec3 d{};
    set_component(d,axis,std::copysign(outer?theta[0]:theta[1],component(d0,axis)));
    const double t=outer?theta[2]:theta[3];for(int k=0;k<3;++k)if(k!=axis)
      set_component(d,k,std::copysign(t,component(d0,k)));
    result.constituents[i]=point_at(c+d);
  }return result;
}

Theta coordinates(const ftd::eft::ConnectedMooreBlockState& reference,
                  const ftd::eft::ConnectedMooreBlockState& state,int axis){
  Theta sums{{0,0,0,0}};std::array<int,4> counts{{0,0,0,0}};
  const Vec3 c0=center(reference),c=center(state);
  for(std::size_t i=0;i<state.constituents.size();++i){
    const bool outer=std::abs(component(position(reference.constituents[i])-c0,axis))>1.0;
    sums[outer?0:1]+=std::abs(component(position(state.constituents[i])-c,axis));
    ++counts[outer?0:1];
    for(int k=0;k<3;++k)if(k!=axis){sums[outer?2:3]+=std::abs(component(position(state.constituents[i])-c,k));++counts[outer?2:3];}
  }
  for(int i=0;i<4;++i)sums[i]/=counts[i];return sums;
}

double mass(int coordinate){return ftd::M_INERTIAL*(coordinate<2?8.0:16.0);}

EigenSystem modes(){
  Matrix a{},vectors{};for(int i=0;i<4;++i){vectors[i][i]=1.0;for(int j=0;j<4;++j)a[i][j]=hessian[i][j]/std::sqrt(mass(i)*mass(j));}
  for(int iteration=0;iteration<96;++iteration){int p=0,q=1;double largest=std::abs(a[p][q]);
    for(int i=0;i<4;++i)for(int j=i+1;j<4;++j)if(std::abs(a[i][j])>largest){largest=std::abs(a[i][j]);p=i;q=j;}
    if(largest<1e-13)break;const double angle=.5*std::atan2(2*a[p][q],a[q][q]-a[p][p]);
    const double c=std::cos(angle),s=std::sin(angle);for(int k=0;k<4;++k)if(k!=p&&k!=q){
      const double kp=a[k][p],kq=a[k][q];a[k][p]=a[p][k]=c*kp-s*kq;a[k][q]=a[q][k]=s*kp+c*kq;}
    const double pp=a[p][p],qq=a[q][q],pq=a[p][q];a[p][p]=c*c*pp-2*c*s*pq+s*s*qq;
    a[q][q]=s*s*pp+2*c*s*pq+c*c*qq;a[p][q]=a[q][p]=0;
    for(int k=0;k<4;++k){const double kp=vectors[k][p],kq=vectors[k][q];vectors[k][p]=c*kp-s*kq;vectors[k][q]=s*kp+c*kq;}}
  std::array<int,4> order{{0,1,2,3}};std::sort(order.begin(),order.end(),[&](int i,int j){return a[i][i]<a[j][j];});
  EigenSystem result;result.valid=true;result.orthogonality=0;
  for(int m=0;m<4;++m){const int column=order[m];auto& mode=result.modes[m];mode.lambda=a[column][column];
    mode.omega=std::sqrt(mode.lambda);mode.phase=2*std::atan(mode.omega/2);
    for(int i=0;i<4;++i)mode.vector[i]=vectors[i][column]/std::sqrt(mass(i));
    int pivot=0;for(int i=1;i<4;++i)if(std::abs(mode.vector[i])>std::abs(mode.vector[pivot]))pivot=i;
    if(mode.vector[pivot]<0)for(double& value:mode.vector)value=-value;
    result.valid=result.valid&&mode.lambda>0&&std::abs(mode.lambda-expected_lambda[m])/expected_lambda[m]<=1e-9
      &&std::abs(mode.omega-expected_omega[m])/expected_omega[m]<=1e-9
      &&std::abs(mode.phase-expected_phase[m])/expected_phase[m]<=1e-9;
  }
  for(int m=0;m<4;++m)for(int n=0;n<4;++n){double inner=0;for(int i=0;i<4;++i)
    inner+=result.modes[m].vector[i]*mass(i)*result.modes[n].vector[i];
    result.orthogonality=std::max(result.orthogonality,std::abs(inner-(m==n?1.0:0.0)));}
  result.valid=result.valid&&result.orthogonality<=1e-12;return result;
}

Theta project(const Theta& theta,const EigenSystem& eig){Theta q{};for(int m=0;m<4;++m)
  for(int i=0;i<4;++i)q[m]+=eig.modes[m].vector[i]*mass(i)*(theta[i]-theta0[i]);return q;}

double energy(const ftd::eft::ConnectedMooreBlockState& state,double beta,
              const ftd::eft::ConnectedMooreBlockOptions& options){long double kinetic=0;
  for(const auto&p:state.constituents)kinetic+=ftd::eft::production_flat_energy_from_momentum(p.momentum);
  return static_cast<double>(kinetic)+ftd::eft::connected_moore_block_binding_energy(state,options)
      +beta*ftd::eft::matched_modified_energy(state.electric,state.magnetic_half,ftd::C_SPEED);}

double common(const ftd::eft::ConnectedMooreBlockStepResult&s){return std::max({s.root_residual,s.continuity_residual,
  s.gauss_before_residual,s.gauss_after_residual,s.force_residual,s.kinematic_residual,
  s.kinetic_discrete_gradient_residual,s.electric_adjoint_residual,s.magnetic_work_residual,
  s.binding_work_residual,s.binding_impulse_sum_residual,s.matter_work_residual,
  s.field_work_residual,s.total_energy_residual,s.causal_speed_excess});}

std::tuple<int,int,double> fibre(const ftd::eft::ConnectedMooreBlockState& state){
  std::map<std::tuple<int,int,int>,std::vector<std::size_t>> groups;for(std::size_t i=0;i<state.constituents.size();++i){const auto&a=state.constituents[i].anchor;groups[{a.x,a.y,a.z}].push_back(i);}
  int maximum=1,pairs=0;double separation=INFINITY;for(const auto&g:groups){maximum=std::max(maximum,static_cast<int>(g.second.size()));
    for(std::size_t i=0;i<g.second.size();++i)for(std::size_t j=i+1;j<g.second.size();++j){++pairs;separation=std::min(separation,
      (position(state.constituents[g.second[i]])-position(state.constituents[g.second[j]])).mag());}}
  return {maximum,pairs,separation};}

double estimate_phase(const std::vector<Tick>& ticks,int mode){long double numerator=0,denominator=0;
  for(int n=1;n<tick_count-1;++n){const double q=ticks[n].q[mode];numerator+=q*(ticks[n+1].q[mode]+ticks[n-1].q[mode]);denominator+=2*q*q;}
  if(!(denominator>0))return INFINITY;const double c=std::clamp(static_cast<double>(numerator/denominator),-1.0,1.0);return std::acos(c);}

bool parent_fingerprint(){const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/"results/ftd_0628/ftd_0628_connected_block_static_dressing_refinement_v1.json";
  std::ifstream in(path,std::ios::binary);const std::string bytes((std::istreambuf_iterator<char>(in)),{});
  return bytes.find(parent_protocol_sha256)!=std::string::npos&&bytes.find("CONNECTED_BLOCK_STATIC_DRESSED_FIXED_POINT_CONSTRUCTIVE")!=std::string::npos;}

Arm run(const Spec& spec,const EigenSystem& eig,double beta,const ftd::eft::ConnectedMooreBlockOptions& options){
  Arm a;a.spec=spec;const auto parent=ftd::eft::initialize_connected_moore_block(L,width,spec.orientation,spec.orientation,.5);
  if(!parent.valid)return a;const auto fixed=ftd::eft::redress_connected_moore_block(geometry_from(parent.state,theta0,spec.orientation),true);
  Theta displaced=theta0;for(int i=0;i<4;++i)displaced[i]+=spec.sign*spec.amplitude*eig.modes[spec.mode].vector[i];
  const auto initial=ftd::eft::redress_connected_moore_block(geometry_from(parent.state,displaced,spec.orientation),true);
  a.initialization=fixed.valid&&initial.valid;if(!a.initialization)return a;
  auto state=initial.state;const auto reference=fixed.state;const Vec3 c0=center(state);const double e0=energy(state,beta,options);
  a.initial_excess=e0-energy(reference,beta,options);a.forward=true;
  for(int tick=1;tick<=tick_count;++tick){const auto step=ftd::eft::solve_connected_moore_block_forward(state,options);const double residual=common(step);
    if(!step.common_action_gates_pass||residual>action_gate){a.forward=false;break;}state=step.later;
    int multiplicity=1,pairs=0;double separation=INFINITY;std::tie(multiplicity,pairs,separation)=fibre(state);
    const Theta shape=coordinates(reference,state,spec.orientation),q=project(shape,eig);const double c=(center(state)-c0).mag();const double drift=std::abs(energy(state,beta,options)-e0);
    a.max_multiplicity=std::max(a.max_multiplicity,multiplicity);if(std::isfinite(separation))a.min_separation=std::min(a.min_separation,separation);
    a.max_center=std::max(a.max_center,c);a.max_drift=std::max(a.max_drift,drift);a.max_common=std::max(a.max_common,residual);
    a.ticks.push_back({tick,multiplicity,pairs,separation,c,drift,residual,shape,q});}
  a.forward=a.forward&&a.ticks.size()==tick_count;if(a.forward){a.phase=estimate_phase(a.ticks,spec.mode);a.phase_error=std::abs(a.phase-eig.modes[spec.mode].phase)/eig.modes[spec.mode].phase;
    long double target=0,leak=0;for(const auto&t:a.ticks){target+=t.q[spec.mode]*t.q[spec.mode];for(int m=0;m<4;++m)if(m!=spec.mode)leak=std::max(leak,static_cast<long double>(t.q[m]*t.q[m]));}
    const double target_rms=std::sqrt(static_cast<double>(target/tick_count));double max_leak=0;for(int m=0;m<4;++m)if(m!=spec.mode){long double sum=0;for(const auto&t:a.ticks)sum+=t.q[m]*t.q[m];max_leak=std::max(max_leak,std::sqrt(static_cast<double>(sum/tick_count)));}a.leakage=max_leak/target_rms;}
  a.reverse=a.forward;for(int tick=tick_count;a.reverse&&tick>=1;--tick){const auto step=ftd::eft::solve_connected_moore_block_reverse(state,options);a.max_common=std::max(a.max_common,common(step));
    if(!step.common_action_gates_pass||common(step)>action_gate){a.reverse=false;break;}state=step.earlier;}
  if(a.reverse)a.recovery=ftd::eft::connected_moore_block_state_max_difference(initial.state,state);
  a.bounded=a.forward&&a.reverse&&a.max_center<=1e-8&&a.max_drift<=1e-12&&a.recovery<=1e-10
    &&a.max_multiplicity<=2&&(!std::isfinite(a.min_separation)||a.min_separation>=.9);return a;}

const Arm* find(const Summary&s,int orientation,int mode,int sign,double amplitude){for(const auto&a:s.arms)if(a.spec.orientation==orientation&&a.spec.mode==mode&&a.spec.sign==sign&&a.spec.amplitude==amplitude)return &a;return nullptr;}
double rel(double a,double b){return std::abs(a-b)/std::max({1e-300,std::abs(a),std::abs(b)});}

void evaluate(Summary&s){s.coverage=s.arms.size()==16;for(int m=0;m<4;++m)s.coverage=s.coverage
  &&find(s,0,m,+1,1e-4)&&find(s,0,m,+1,2e-4)&&find(s,0,m,-1,1e-4)&&find(s,1,m,+1,1e-4);
  s.execution=s.coverage&&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm&a){return a.initialization&&a.forward&&a.reverse;});
  s.bounded=s.execution&&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm&a){return a.bounded;});
  s.frequency=s.execution&&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm&a){return std::isfinite(a.phase)&&a.phase_error<=.02;});
  s.purity=s.execution&&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm&a){return std::isfinite(a.leakage)&&a.leakage<=.10;});
  s.amplitude=s.sign=s.covariance=s.execution;s.amplitude_residual=s.sign_residual=s.covariance_residual=0;
  for(int m=0;m<4;++m){const Arm*a=find(s,0,m,+1,1e-4),*b=find(s,0,m,+1,2e-4),*n=find(s,0,m,-1,1e-4),*y=find(s,1,m,+1,1e-4);
    if(!a||!b||!n||!y){s.amplitude=s.sign=s.covariance=false;continue;}
    const double ar=rel(a->phase,b->phase),er=std::abs(b->initial_excess/a->initial_excess-4.0);
    s.amplitude_residual=std::max({s.amplitude_residual,ar,er});s.amplitude=s.amplitude&&ar<=.005&&b->initial_excess/a->initial_excess>=3.90&&b->initial_excess/a->initial_excess<=4.10;
    double sr=0,cr=0;for(int t=0;t<tick_count;++t){sr=std::max(sr,std::abs(a->ticks[t].q[m]+n->ticks[t].q[m])/1e-4);cr=std::max(cr,std::abs(a->ticks[t].q[m]-y->ticks[t].q[m])/1e-4);}
    sr=std::max(sr,rel(a->phase,n->phase));cr=std::max(cr,rel(a->phase,y->phase));s.sign_residual=std::max(s.sign_residual,sr);s.covariance_residual=std::max(s.covariance_residual,cr);
    s.sign=s.sign&&sr<=.05&&rel(a->phase,n->phase)<=.005;s.covariance=s.covariance&&cr<=.05&&rel(a->phase,y->phase)<=.005;}
  for(const auto&a:s.arms){s.worst_common=std::max(s.worst_common,a.max_common);s.worst_drift=std::max(s.worst_drift,a.max_drift);if(std::isfinite(a.recovery))s.worst_recovery=std::max(s.worst_recovery,a.recovery);}
  if(!s.parent||!s.normalization||!s.eigensystem||!s.coverage)s.verdict="CONNECTED_BLOCK_LINEAR_MODES_EXECUTION_INVALID";
  else if(!s.bounded)s.verdict="CONNECTED_BLOCK_LINEAR_MODE_STABILITY_CLOSED_NEGATIVE";
  else if(s.frequency&&s.purity&&s.amplitude&&s.sign&&s.covariance)s.verdict="CONNECTED_BLOCK_ADIABATIC_LINEAR_MODES_CONSTRUCTIVE";
  else s.verdict="CONNECTED_BLOCK_BOUNDED_HYBRID_MODES_OPEN";}

void number(std::ostream&o,double x){if(std::isfinite(x))o<<x;else o<<"null";}
void write(const Summary&s){const auto dir=std::filesystem::path(__FILE__).parent_path().parent_path()/"results/ftd_0629";std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0629_connected_block_linear_modes_v1.json");json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0629\",\n  \"protocol_sha256\": \""<<protocol_sha256<<"\",\n  \"parent_result_sha256\": \""<<parent_result_sha256<<"\",\n  \"verdict\": \""<<s.verdict<<"\",\n  \"production_changed\": false,\n  \"coverage_pass\": "<<s.coverage<<",\n  \"eigensystem_pass\": "<<s.eigensystem<<",\n  \"execution_pass\": "<<s.execution<<",\n  \"bounded_pass\": "<<s.bounded<<",\n  \"frequency_pass\": "<<s.frequency<<",\n  \"purity_pass\": "<<s.purity<<",\n  \"amplitude_pass\": "<<s.amplitude<<",\n  \"sign_pass\": "<<s.sign<<",\n  \"covariance_pass\": "<<s.covariance<<",\n  \"orthogonality_residual\": ";number(json,s.orthogonality);json<<",\n  \"amplitude_residual\": ";number(json,s.amplitude_residual);json<<",\n  \"sign_residual\": ";number(json,s.sign_residual);json<<",\n  \"covariance_residual\": ";number(json,s.covariance_residual);json<<",\n  \"worst_common_residual\": "<<s.worst_common<<",\n  \"worst_energy_drift\": "<<s.worst_drift<<",\n  \"worst_recovery\": "<<s.worst_recovery<<"\n}\n";
  std::ofstream modes_file(dir/"ftd_0629_connected_block_linear_modes_modes_v1.csv");modes_file<<"ftd_id,mode,lambda,omega,phase,period,v0,v1,v2,v3\n";for(int m=0;m<4;++m){const auto&x=s.modes.modes[m];modes_file<<std::setprecision(17)<<"FTD-0629,"<<m<<','<<x.lambda<<','<<x.omega<<','<<x.phase<<','<<2*std::acos(-1.0)/x.phase;for(double v:x.vector)modes_file<<','<<v;modes_file<<'\n';}
  std::ofstream arms_file(dir/"ftd_0629_connected_block_linear_modes_arms_v1.csv");arms_file<<"ftd_id,label,orientation,mode,sign,amplitude,init,forward,reverse,bounded,initial_excess,phase,predicted_phase,phase_error,leakage,max_center,max_drift,max_common,recovery,max_multiplicity,min_separation\n";for(const auto&a:s.arms)arms_file<<std::setprecision(17)<<"FTD-0629,"<<a.spec.label<<','<<a.spec.orientation<<','<<a.spec.mode<<','<<a.spec.sign<<','<<a.spec.amplitude<<','<<a.initialization<<','<<a.forward<<','<<a.reverse<<','<<a.bounded<<','<<a.initial_excess<<','<<a.phase<<','<<s.modes.modes[a.spec.mode].phase<<','<<a.phase_error<<','<<a.leakage<<','<<a.max_center<<','<<a.max_drift<<','<<a.max_common<<','<<a.recovery<<','<<a.max_multiplicity<<','<<a.min_separation<<'\n';
  std::ofstream ticks_file(dir/"ftd_0629_connected_block_linear_modes_ticks_v1.csv");ticks_file<<"ftd_id,label,tick,multiplicity,pairs,separation,center,drift,common,a,b,t_outer,t_inner,q0,q1,q2,q3\n";for(const auto&a:s.arms)for(const auto&t:a.ticks){ticks_file<<std::setprecision(17)<<"FTD-0629,"<<a.spec.label<<','<<t.index<<','<<t.multiplicity<<','<<t.pairs<<','<<t.separation<<','<<t.center<<','<<t.drift<<','<<t.common;for(double v:t.theta)ticks_file<<','<<v;for(double v:t.q)ticks_file<<','<<v;ticks_file<<'\n';}}

} // namespace

int main(){std::cout<<std::setprecision(17);Summary s;s.parent=parent_fingerprint();const auto normalization=ftd::eft::measure_face_flux_normalization();s.normalization=normalization.valid;s.beta=normalization.mapped_field_work_coefficient;s.modes=modes();s.eigensystem=s.modes.valid;s.orthogonality=s.modes.orthogonality;
  ftd::eft::ConnectedMooreBlockOptions options;options.gate_tolerance=action_gate;options.solve_tolerance=2e-11;options.max_iterations=48;options.allow_shared_anchor_chart=true;
  std::vector<Spec> specs;for(int m=0;m<4;++m){specs.push_back({"x_m"+std::to_string(m)+"_p1",0,m,+1,1e-4});specs.push_back({"x_m"+std::to_string(m)+"_p2",0,m,+1,2e-4});specs.push_back({"x_m"+std::to_string(m)+"_n1",0,m,-1,1e-4});specs.push_back({"y_m"+std::to_string(m)+"_p1",1,m,+1,1e-4});}
  if(s.parent&&s.normalization&&s.eigensystem){std::vector<std::future<Arm>> futures;for(const auto&spec:specs)futures.push_back(std::async(std::launch::async,[&,spec](){return run(spec,s.modes,s.beta,options);}));for(std::size_t i=0;i<specs.size();++i){s.arms.push_back(futures[i].get());std::cout<<"completed "<<specs[i].label<<std::endl;}}
  evaluate(s);write(s);std::cout<<"protocol_sha256="<<protocol_sha256<<'\n'<<"verdict="<<s.verdict<<'\n'<<"bounded="<<s.bounded<<" frequency="<<s.frequency<<" purity="<<s.purity<<" amplitude="<<s.amplitude<<" sign="<<s.sign<<" covariance="<<s.covariance<<'\n';for(const auto&a:s.arms)std::cout<<a.spec.label<<" phase="<<a.phase<<" error="<<a.phase_error<<" leakage="<<a.leakage<<" excess="<<a.initial_excess<<" bounded="<<a.bounded<<" recovery="<<a.recovery<<'\n';return s.verdict=="CONNECTED_BLOCK_LINEAR_MODES_EXECUTION_INVALID"?1:0;}
