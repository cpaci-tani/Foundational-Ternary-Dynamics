// FTD-0631: refine and qualify the fully-half connected matter candidate.

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
#include <string>
#include <tuple>
#include <vector>

#ifndef FTD_FULL_HALF_ID
#define FTD_FULL_HALF_ID "FTD-0631"
#define FTD_FULL_HALF_PROTOCOL_SHA256 "ADB2F73EDF9092C8DA0E0446BD26450432698CCA88A2B2CECF204C105FF00EE8"
#define FTD_FULL_HALF_CANDIDATE_PROTOCOL_SHA256 "4BF3F43F841ABC653611E40FA74B6BF0AB7FEEE14C5F226062D456B34DB76586"
#define FTD_FULL_HALF_PARENT_RESULT_SHA256 "6343684A3427AC5CEA57EFA22C591A4702E1B2D78B82F0461A5F372AE23E91C8"
#define FTD_FULL_HALF_CANDIDATE_RESULT "results/ftd_0630/ftd_0630_connected_block_translation_curvature_v1.json"
#define FTD_FULL_HALF_CANDIDATE_VERDICT "CONNECTED_BLOCK_TRANSLATION_CURVATURE_EXECUTION_INVALID"
#define FTD_FULL_HALF_RESULT_DIRECTORY "results/ftd_0631"
#define FTD_FULL_HALF_RESULT_STEM "ftd_0631_connected_block_full_half"
#define FTD_FULL_HALF_FIBRE_LIMIT 2
#define FTD_FULL_HALF_VERDICT_INVALID "CONNECTED_BLOCK_FULL_HALF_STATIC_REFINEMENT_EXECUTION_INVALID"
#define FTD_FULL_HALF_VERDICT_CONSTRUCTIVE "CONNECTED_BLOCK_FULL_HALF_STATIC_BASIN_CONSTRUCTIVE"
#define FTD_FULL_HALF_VERDICT_STATIONARY "CONNECTED_BLOCK_FULL_HALF_SYMMETRY_STATIONARY_ONLY"
#define FTD_FULL_HALF_VERDICT_NEGATIVE "CONNECTED_BLOCK_FULL_HALF_STATIC_REFINEMENT_CLOSED_NEGATIVE"
#endif

namespace {

using ftd::Vec3;
using Theta = std::array<double,4>;
using Matrix = std::array<std::array<double,4>,4>;
constexpr char protocol_sha256[] =
    FTD_FULL_HALF_PROTOCOL_SHA256;
constexpr char parent_protocol_sha256[] =
    "BF823BB629BFAB7FA385E39AB83E4BDCC2DCA3E857EE424FAF65C3280898CB4F";
constexpr char candidate_protocol_sha256[] =
    FTD_FULL_HALF_CANDIDATE_PROTOCOL_SHA256;
constexpr char parent_result_sha256[] =
    FTD_FULL_HALF_PARENT_RESULT_SHA256;
constexpr int L=17,width=2,tick_count=64;
constexpr double gradient_step=2e-5,hessian_step=2e-4;
constexpr double translation_step=1e-3,action_gate=1e-10;
constexpr double body_half_energy=0.0367648643204065;
constexpr Theta start_theta{{1.4993153663084844,0.4994670538459639,
                             0.50006590532229034,0.50018096647517352}};

struct EnergyEvaluation {
  bool valid=false;
  double energy=INFINITY,binding=INFINITY,field=INFINITY,gauss=INFINITY;
  ftd::eft::ConnectedMooreBlockState state;
  EnergyEvaluation():state(L){}
};
struct EigenSystem { Matrix vectors{}; Theta values{{0,0,0,0}}; };
struct OptimizationRecord {
  int iteration=0;Theta theta{{0,0,0,0}};
  double energy=INFINITY,gradient=INFINITY,minimum_eigenvalue=INFINITY;
  double accepted_scale=0,gauss=INFINITY;
};
struct TranslationRecord {
  int axis=0;double minus=INFINITY,center=INFINITY,plus=INFINITY;
  double gradient=INFINITY,curvature=INFINITY;
};
struct TickRecord {
  int tick=0,multiplicity=1,shared_pairs=0;
  double separation=INFINITY,state_distance=INFINITY;
  double center_displacement=INFINITY,energy_drift=INFINITY,common=INFINITY;
};
struct Arm {
  std::string label;int orientation=0,evaluations=0,iterations=0,max_multiplicity=1;
  bool initialization=false,optimization=false,positive_hessian=false;
  bool translation_basin=false,one_step=false,full_stationarity=false;
  bool forward=false,reverse=false,repeated=false;
  Theta theta{{0,0,0,0}},gradient{{INFINITY,INFINITY,INFINITY,INFINITY}};
  Theta eigenvalues{{INFINITY,INFINITY,INFINITY,INFINITY}};Matrix hessian{};
  double starting_energy=INFINITY,refined_energy=INFINITY,max_impulse=INFINITY;
  double first_displacement=INFINITY,first_momentum=INFINITY;
  double max_center_displacement=0,max_state_distance=0,max_energy_drift=0;
  double max_common=0,recovery=INFINITY,min_separation=INFINITY;
  ftd::eft::ConnectedMooreBlockState initial;
  std::vector<OptimizationRecord> optimization_records;
  std::vector<TranslationRecord> translations;std::vector<TickRecord> ticks;
  Arm():initial(L){}
};
struct Summary {
  bool parent=false,candidate_provenance=false,normalization=false;
  bool coverage=false,initialization=false,reduced=false,translation=false;
  bool full_space=false,repeated=false,covariance=false;
  double beta=0,covariance_residual=INFINITY,rotated_state_residual=INFINITY;
  double worst_common=0,worst_drift=0,worst_recovery=0;
  std::string verdict;std::vector<Arm> arms;
};

double component(const Vec3& v,int axis){return axis==0?v.x:(axis==1?v.y:v.z);}
void set_component(Vec3& v,int axis,double value){if(axis==0)v.x=value;else if(axis==1)v.y=value;else v.z=value;}
double max_component(const Vec3& v){return std::max({std::abs(v.x),std::abs(v.y),std::abs(v.z)});}
Vec3 position(const ftd::eft::MatchedMatterPoint& p){return {p.anchor.x+p.remainder.x,p.anchor.y+p.remainder.y,p.anchor.z+p.remainder.z};}
Vec3 center(const ftd::eft::ConnectedMooreBlockState& s){Vec3 r{};for(const auto&p:s.constituents)r+=position(p);return r*(1.0/s.constituents.size());}
Vec3 total_momentum(const ftd::eft::ConnectedMooreBlockState& s){Vec3 r{};for(const auto&p:s.constituents)r+=p.momentum;return r;}
int wrap(int value){const int r=value%L;return r<0?r+L:r;}
ftd::eft::MatchedMatterPoint point_at(const Vec3& x){ftd::eft::MatchedMatterPoint r;const long long ax=std::llround(x.x),ay=std::llround(x.y),az=std::llround(x.z);r.anchor={wrap(static_cast<int>(ax)),wrap(static_cast<int>(ay)),wrap(static_cast<int>(az))};r.remainder={x.x-ax,x.y-ay,x.z-az};return r;}
bool admissible(const Theta&t){return t[0]>=1.25&&t[0]<=1.75&&t[1]>=0.25&&t[1]<=0.75&&t[0]-t[1]>=0.50&&t[2]>=0.25&&t[2]<=0.75&&t[3]>=0.25&&t[3]<=0.75;}

ftd::eft::ConnectedMooreBlockState geometry_from(const ftd::eft::ConnectedMooreBlockState& base,const Theta&theta,int orientation){auto r=base;const Vec3 c=center(base);Vec3 shift{};for(int axis=0;axis<3;++axis)if(axis!=orientation)set_component(shift,axis,0.5);for(std::size_t i=0;i<r.constituents.size();++i){const Vec3 d0=position(base.constituents[i])-c;const bool outer=std::abs(component(d0,orientation))>1.0;Vec3 d{};set_component(d,orientation,std::copysign(outer?theta[0]:theta[1],component(d0,orientation)));const double t=outer?theta[2]:theta[3];for(int axis=0;axis<3;++axis)if(axis!=orientation)set_component(d,axis,std::copysign(t,component(d0,axis)));r.constituents[i]=point_at(c+shift+d);}return r;}
ftd::eft::ConnectedMooreBlockState translate(const ftd::eft::ConnectedMooreBlockState&s,const Vec3&d){auto r=s;for(auto&p:r.constituents)p=point_at(position(p)+d);return r;}

EnergyEvaluation evaluate_geometry(const ftd::eft::ConnectedMooreBlockState&g,double beta,const ftd::eft::ConnectedMooreBlockOptions&o,int&count){EnergyEvaluation r;++count;const auto d=ftd::eft::redress_connected_moore_block_with_fibre_limit(g,FTD_FULL_HALF_FIBRE_LIMIT,1e-13,4096);if(!d.valid)return r;r.valid=true;r.state=d.state;r.gauss=d.gauss_residual;r.binding=ftd::eft::connected_moore_block_binding_energy(d.state,o);r.field=beta*ftd::eft::matched_modified_energy(d.state.electric,d.state.magnetic_half,ftd::C_SPEED);r.energy=r.binding+r.field;return r;}
EnergyEvaluation evaluate_energy(const ftd::eft::ConnectedMooreBlockState&b,const Theta&t,int orientation,double beta,const ftd::eft::ConnectedMooreBlockOptions&o,int&count){if(!admissible(t))return EnergyEvaluation{};return evaluate_geometry(geometry_from(b,t,orientation),beta,o,count);}

bool gradient_at(const ftd::eft::ConnectedMooreBlockState&b,const Theta&t,int orientation,double beta,const ftd::eft::ConnectedMooreBlockOptions&o,int&count,Theta&g){for(int i=0;i<4;++i){Theta p=t,m=t;p[i]+=gradient_step;m[i]-=gradient_step;const auto ep=evaluate_energy(b,p,orientation,beta,o,count),em=evaluate_energy(b,m,orientation,beta,o,count);if(!ep.valid||!em.valid)return false;g[i]=(ep.energy-em.energy)/(2*gradient_step);}return true;}
bool hessian_at(const ftd::eft::ConnectedMooreBlockState&b,const Theta&t,int orientation,double beta,const ftd::eft::ConnectedMooreBlockOptions&o,int&count,Matrix&h){const auto e0=evaluate_energy(b,t,orientation,beta,o,count);if(!e0.valid)return false;for(int i=0;i<4;++i){Theta p=t,m=t;p[i]+=hessian_step;m[i]-=hessian_step;const auto ep=evaluate_energy(b,p,orientation,beta,o,count),em=evaluate_energy(b,m,orientation,beta,o,count);if(!ep.valid||!em.valid)return false;h[i][i]=(ep.energy-2*e0.energy+em.energy)/(hessian_step*hessian_step);for(int j=i+1;j<4;++j){Theta pp=t,pm=t,mp=t,mm=t;pp[i]+=hessian_step;pp[j]+=hessian_step;pm[i]+=hessian_step;pm[j]-=hessian_step;mp[i]-=hessian_step;mp[j]+=hessian_step;mm[i]-=hessian_step;mm[j]-=hessian_step;const auto epp=evaluate_energy(b,pp,orientation,beta,o,count),epm=evaluate_energy(b,pm,orientation,beta,o,count),emp=evaluate_energy(b,mp,orientation,beta,o,count),emm=evaluate_energy(b,mm,orientation,beta,o,count);if(!epp.valid||!epm.valid||!emp.valid||!emm.valid)return false;h[i][j]=h[j][i]=(epp.energy-epm.energy-emp.energy+emm.energy)/(4*hessian_step*hessian_step);}}return true;}

EigenSystem diagonalize(Matrix a){EigenSystem r;for(int i=0;i<4;++i)r.vectors[i][i]=1;for(int iteration=0;iteration<96;++iteration){int p=0,q=1;double largest=std::abs(a[p][q]);for(int i=0;i<4;++i)for(int j=i+1;j<4;++j)if(std::abs(a[i][j])>largest){largest=std::abs(a[i][j]);p=i;q=j;}if(largest<1e-13)break;const double angle=.5*std::atan2(2*a[p][q],a[q][q]-a[p][p]),c=std::cos(angle),s=std::sin(angle);for(int k=0;k<4;++k)if(k!=p&&k!=q){const double kp=a[k][p],kq=a[k][q];a[k][p]=a[p][k]=c*kp-s*kq;a[k][q]=a[q][k]=s*kp+c*kq;}const double pp=a[p][p],qq=a[q][q],pq=a[p][q];a[p][p]=c*c*pp-2*c*s*pq+s*s*qq;a[q][q]=s*s*pp+2*c*s*pq+c*c*qq;a[p][q]=a[q][p]=0;for(int k=0;k<4;++k){const double kp=r.vectors[k][p],kq=r.vectors[k][q];r.vectors[k][p]=c*kp-s*kq;r.vectors[k][q]=s*kp+c*kq;}}for(int i=0;i<4;++i)r.values[i]=a[i][i];return r;}
double infinity_norm(const Theta&v){return std::max({std::abs(v[0]),std::abs(v[1]),std::abs(v[2]),std::abs(v[3])});}
Theta newton_step(const Theta&g,const EigenSystem&e){Theta p{{0,0,0,0}},s{{0,0,0,0}};for(int m=0;m<4;++m)for(int i=0;i<4;++i)p[m]+=e.vectors[i][m]*g[i];for(int m=0;m<4;++m)p[m]/=std::max(1e-6,e.values[m]);for(int i=0;i<4;++i)for(int m=0;m<4;++m)s[i]-=e.vectors[i][m]*p[m];return s;}

double common_residual(const ftd::eft::ConnectedMooreBlockStepResult&s){return std::max({s.root_residual,s.continuity_residual,s.gauss_before_residual,s.gauss_after_residual,s.force_residual,s.kinematic_residual,s.kinetic_discrete_gradient_residual,s.electric_adjoint_residual,s.magnetic_work_residual,s.binding_work_residual,s.binding_impulse_sum_residual,s.matter_work_residual,s.field_work_residual,s.total_energy_residual,s.causal_speed_excess});}
double kinetic(const ftd::eft::ConnectedMooreBlockState&s){long double r=0;for(const auto&p:s.constituents)r+=ftd::eft::production_flat_energy_from_momentum(p.momentum);return static_cast<double>(r);}
double total_energy(const ftd::eft::ConnectedMooreBlockState&s,double beta,const ftd::eft::ConnectedMooreBlockOptions&o){return kinetic(s)+ftd::eft::connected_moore_block_binding_energy(s,o)+beta*ftd::eft::matched_modified_energy(s.electric,s.magnetic_half,ftd::C_SPEED);}
std::tuple<int,int,double> fibre_metrics(const ftd::eft::ConnectedMooreBlockState&s){std::map<std::tuple<int,int,int>,std::vector<std::size_t>>g;for(std::size_t i=0;i<s.constituents.size();++i){const auto&a=s.constituents[i].anchor;g[{a.x,a.y,a.z}].push_back(i);}int maximum=1,pairs=0;double separation=INFINITY;for(const auto&x:g){maximum=std::max(maximum,static_cast<int>(x.second.size()));for(std::size_t i=0;i<x.second.size();++i)for(std::size_t j=i+1;j<x.second.size();++j){++pairs;separation=std::min(separation,(position(s.constituents[x.second[i]])-position(s.constituents[x.second[j]])).mag());}}return {maximum,pairs,separation};}

Arm run_arm(std::string label,int orientation,double beta,const ftd::eft::ConnectedMooreBlockOptions&o){Arm r;r.label=std::move(label);r.orientation=orientation;const auto parent=ftd::eft::initialize_connected_moore_block(L,width,orientation,orientation,.5,1e-13,4096);r.initialization=parent.valid&&parent.state.constituents.size()==16&&parent.state.edges.size()==72;if(!r.initialization)return r;const auto base=parent.state;const auto start=evaluate_energy(base,start_theta,orientation,beta,o,r.evaluations);if(!start.valid)return r;r.starting_energy=start.energy;Theta theta=start_theta;for(int iteration=0;iteration<16;++iteration){Theta g{};Matrix h{};const auto current=evaluate_energy(base,theta,orientation,beta,o,r.evaluations);if(!current.valid||!gradient_at(base,theta,orientation,beta,o,r.evaluations,g)||!hessian_at(base,theta,orientation,beta,o,r.evaluations,h))break;const auto eig=diagonalize(h);r.optimization_records.push_back({iteration,theta,current.energy,infinity_norm(g),*std::min_element(eig.values.begin(),eig.values.end()),0,current.gauss});r.iterations=iteration+1;if(infinity_norm(g)<=1e-9)break;const Theta step=newton_step(g,eig);bool accepted=false;for(int backtrack=0;backtrack<=10;++backtrack){const double scale=std::ldexp(1.0,-backtrack);Theta trial=theta;for(int i=0;i<4;++i)trial[i]+=scale*step[i];const auto e=evaluate_energy(base,trial,orientation,beta,o,r.evaluations);if(admissible(trial)&&e.valid&&e.energy<current.energy){theta=trial;r.optimization_records.back().accepted_scale=scale;accepted=true;break;}}if(!accepted)break;}
  r.theta=theta;const auto refined=evaluate_energy(base,theta,orientation,beta,o,r.evaluations);if(!refined.valid||!gradient_at(base,theta,orientation,beta,o,r.evaluations,r.gradient)||!hessian_at(base,theta,orientation,beta,o,r.evaluations,r.hessian))return r;const auto eig=diagonalize(r.hessian);r.eigenvalues=eig.values;r.refined_energy=refined.energy;r.initial=refined.state;r.optimization=infinity_norm(r.gradient)<=1e-9&&r.refined_energy<r.starting_energy&&r.refined_energy<body_half_energy;r.positive_hessian=*std::min_element(r.eigenvalues.begin(),r.eigenvalues.end())>1e-6;
  r.translation_basin=r.optimization&&r.positive_hessian;for(int axis=0;axis<3;++axis){Vec3 d{};set_component(d,axis,translation_step);const auto ep=evaluate_geometry(translate(r.initial,d),beta,o,r.evaluations),em=evaluate_geometry(translate(r.initial,d*(-1)),beta,o,r.evaluations);TranslationRecord t;t.axis=axis;t.center=r.refined_energy;if(ep.valid&&em.valid){t.plus=ep.energy;t.minus=em.energy;t.gradient=(ep.energy-em.energy)/(2*translation_step);t.curvature=(ep.energy-2*r.refined_energy+em.energy)/(translation_step*translation_step);r.translation_basin=r.translation_basin&&std::abs(t.gradient)<=1e-9&&t.curvature>1e-4;}else r.translation_basin=false;r.translations.push_back(t);}
  const auto first=ftd::eft::solve_connected_moore_block_forward(r.initial,o);r.one_step=first.common_action_gates_pass&&common_residual(first)<=action_gate;r.max_impulse=0;if(r.one_step)for(const auto&i:first.total_impulses)r.max_impulse=std::max(r.max_impulse,max_component(i));r.first_displacement=r.one_step?ftd::eft::connected_moore_block_state_max_difference(r.initial,first.later):INFINITY;r.first_momentum=r.one_step?max_component(total_momentum(first.later)):INFINITY;r.full_stationarity=r.one_step&&r.max_impulse<=1e-9&&r.first_displacement<=1e-9&&r.first_momentum<=1e-9;
  auto state=r.initial;const Vec3 c0=center(state);const double energy0=total_energy(state,beta,o);r.forward=r.translation_basin&&r.full_stationarity;for(int tick=1;r.forward&&tick<=tick_count;++tick){const auto step=ftd::eft::solve_connected_moore_block_forward(state,o);const double common=common_residual(step);if(!step.common_action_gates_pass||common>action_gate){r.forward=false;break;}state=step.later;int multiplicity=1,pairs=0;double separation=INFINITY;std::tie(multiplicity,pairs,separation)=fibre_metrics(state);const double distance=ftd::eft::connected_moore_block_state_max_difference(r.initial,state),displacement=(center(state)-c0).mag(),drift=std::abs(total_energy(state,beta,o)-energy0);r.max_multiplicity=std::max(r.max_multiplicity,multiplicity);if(std::isfinite(separation))r.min_separation=std::min(r.min_separation,separation);r.max_state_distance=std::max(r.max_state_distance,distance);r.max_center_displacement=std::max(r.max_center_displacement,displacement);r.max_energy_drift=std::max(r.max_energy_drift,drift);r.max_common=std::max(r.max_common,common);r.ticks.push_back({tick,multiplicity,pairs,separation,distance,displacement,drift,common});}r.forward=r.forward&&r.ticks.size()==tick_count;r.reverse=r.forward;for(int tick=tick_count;r.reverse&&tick>=1;--tick){const auto step=ftd::eft::solve_connected_moore_block_reverse(state,o);const double common=common_residual(step);r.max_common=std::max(r.max_common,common);if(!step.common_action_gates_pass||common>action_gate){r.reverse=false;break;}state=step.earlier;}if(r.reverse)r.recovery=ftd::eft::connected_moore_block_state_max_difference(r.initial,state);r.repeated=r.forward&&r.reverse&&r.max_center_displacement<=1e-10&&r.max_state_distance<=1e-8&&r.max_energy_drift<=1e-12&&r.max_common<=action_gate&&r.recovery<=1e-10&&r.max_multiplicity<=2&&(!std::isfinite(r.min_separation)||r.min_separation>=.9);return r;}

double relative(double a,double b){return std::abs(a-b)/std::max({1.0,std::abs(a),std::abs(b)});}
std::size_t index(int x,int y,int z){return static_cast<std::size_t>((wrap(x)*L+wrap(y))*L+wrap(z));}
double rotated_state_covariance(const ftd::eft::ConnectedMooreBlockState&x,const ftd::eft::ConnectedMooreBlockState&y){double r=0;std::vector<bool>used(y.constituents.size(),false);for(std::size_t i=0;i<x.constituents.size();++i){const Vec3 p=position(x.constituents[i]),rotated{p.z,p.x,p.y};double best=INFINITY;std::size_t selected=y.constituents.size();for(std::size_t j=0;j<y.constituents.size();++j)if(!used[j]&&x.charges[i]==y.charges[j]){const double d=(rotated-position(y.constituents[j])).mag();if(d<best){best=d;selected=j;}}if(selected==y.constituents.size())return INFINITY;used[selected]=true;r=std::max(r,best);}for(int ix=0;ix<L;++ix)for(int iy=0;iy<L;++iy)for(int iz=0;iz<L;++iz){const auto old=index(ix,iy,iz),rotated=index(iz,ix,iy);r=std::max({r,std::abs(x.electric.z[old]-y.electric.x[rotated]),std::abs(x.electric.x[old]-y.electric.y[rotated]),std::abs(x.electric.y[old]-y.electric.z[rotated]),std::abs(x.magnetic_half.z[old]-y.magnetic_half.x[rotated]),std::abs(x.magnetic_half.x[old]-y.magnetic_half.y[rotated]),std::abs(x.magnetic_half.y[old]-y.magnetic_half.z[rotated])});}return r;}
bool fingerprint(const std::filesystem::path&path,const char*protocol,const char*verdict){std::ifstream in(path,std::ios::binary);const std::string bytes((std::istreambuf_iterator<char>(in)),{});return bytes.find(protocol)!=std::string::npos&&bytes.find(verdict)!=std::string::npos;}
void evaluate_summary(Summary&s){s.coverage=s.arms.size()==2&&s.arms[0].orientation==0&&s.arms[1].orientation==1;s.initialization=s.coverage&&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm&a){return a.initialization;});s.reduced=s.initialization&&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm&a){return a.optimization&&a.positive_hessian;});s.translation=s.reduced&&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm&a){return a.translation_basin;});s.full_space=s.translation&&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm&a){return a.full_stationarity;});s.repeated=s.full_space&&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm&a){return a.repeated;});if(s.coverage){const auto&x=s.arms[0];const auto&y=s.arms[1];s.rotated_state_residual=rotated_state_covariance(x.initial,y.initial);s.covariance_residual=s.rotated_state_residual;for(int i=0;i<4;++i)s.covariance_residual=std::max({s.covariance_residual,std::abs(x.theta[i]-y.theta[i]),relative(x.gradient[i],y.gradient[i]),relative(x.eigenvalues[i],y.eigenvalues[i])});s.covariance_residual=std::max({s.covariance_residual,relative(x.refined_energy,y.refined_energy),relative(x.max_impulse,y.max_impulse),relative(x.recovery,y.recovery)});s.covariance=s.covariance_residual<=1e-9;}for(const auto&a:s.arms){s.worst_common=std::max(s.worst_common,a.max_common);s.worst_drift=std::max(s.worst_drift,a.max_energy_drift);if(std::isfinite(a.recovery))s.worst_recovery=std::max(s.worst_recovery,a.recovery);}if(!s.parent||!s.candidate_provenance||!s.normalization||!s.coverage||!s.initialization||!s.covariance)s.verdict=FTD_FULL_HALF_VERDICT_INVALID;else if(s.repeated)s.verdict=FTD_FULL_HALF_VERDICT_CONSTRUCTIVE;else if(s.reduced&&s.full_space)s.verdict=FTD_FULL_HALF_VERDICT_STATIONARY;else s.verdict=FTD_FULL_HALF_VERDICT_NEGATIVE;}
void number(std::ostream&o,double x){if(std::isfinite(x))o<<x;else o<<"null";}

void write_records(const Summary&s){const auto dir=std::filesystem::path(__FILE__).parent_path().parent_path()/FTD_FULL_HALF_RESULT_DIRECTORY;std::filesystem::create_directories(dir);std::ofstream json(dir/(std::string(FTD_FULL_HALF_RESULT_STEM)+"_static_refinement_v1.json"));json<<std::setprecision(17)<<"{\n  \"ftd_id\": \""<<FTD_FULL_HALF_ID<<"\",\n  \"protocol_sha256\": \""<<protocol_sha256<<"\",\n  \"parent_result_sha256\": \""<<parent_result_sha256<<"\",\n  \"verdict\": \""<<s.verdict<<"\",\n  \"production_changed\": false,\n  \"fibre_limit\": "<<FTD_FULL_HALF_FIBRE_LIMIT<<",\n  \"coverage_pass\": "<<s.coverage<<",\n  \"initialization_pass\": "<<s.initialization<<",\n  \"reduced_basin_pass\": "<<s.reduced<<",\n  \"translation_basin_pass\": "<<s.translation<<",\n  \"full_stationarity_pass\": "<<s.full_space<<",\n  \"repeated_fixed_point_pass\": "<<s.repeated<<",\n  \"covariance_pass\": "<<s.covariance<<",\n  \"rotated_state_residual\": ";number(json,s.rotated_state_residual);json<<",\n  \"covariance_residual\": ";number(json,s.covariance_residual);json<<",\n  \"worst_common_residual\": "<<s.worst_common<<",\n  \"worst_energy_drift\": "<<s.worst_drift<<",\n  \"worst_recovery\": "<<s.worst_recovery<<"\n}\n";std::ofstream opt(dir/(std::string(FTD_FULL_HALF_RESULT_STEM)+"_optimization_v1.csv"));opt<<"ftd_id,label,iteration,a,b,t_outer,t_inner,energy,gradient_inf,min_eigenvalue,accepted_scale,gauss\n";for(const auto&a:s.arms)for(const auto&r:a.optimization_records)opt<<std::setprecision(17)<<FTD_FULL_HALF_ID<<','<<a.label<<','<<r.iteration<<','<<r.theta[0]<<','<<r.theta[1]<<','<<r.theta[2]<<','<<r.theta[3]<<','<<r.energy<<','<<r.gradient<<','<<r.minimum_eigenvalue<<','<<r.accepted_scale<<','<<r.gauss<<'\n';std::ofstream arms(dir/(std::string(FTD_FULL_HALF_RESULT_STEM)+"_arms_v1.csv"));arms<<"ftd_id,label,orientation,init,optimization,positive_hessian,translation_basin,one_step,full_stationarity,forward,reverse,repeated,evaluations,iterations,starting_energy,refined_energy,a,b,t_outer,t_inner,g0,g1,g2,g3,e0,e1,e2,e3,max_impulse,first_displacement,first_momentum,max_center_displacement,max_state_distance,max_energy_drift,max_common,recovery,max_multiplicity,min_separation\n";for(const auto&a:s.arms)arms<<std::setprecision(17)<<FTD_FULL_HALF_ID<<','<<a.label<<','<<a.orientation<<','<<a.initialization<<','<<a.optimization<<','<<a.positive_hessian<<','<<a.translation_basin<<','<<a.one_step<<','<<a.full_stationarity<<','<<a.forward<<','<<a.reverse<<','<<a.repeated<<','<<a.evaluations<<','<<a.iterations<<','<<a.starting_energy<<','<<a.refined_energy<<','<<a.theta[0]<<','<<a.theta[1]<<','<<a.theta[2]<<','<<a.theta[3]<<','<<a.gradient[0]<<','<<a.gradient[1]<<','<<a.gradient[2]<<','<<a.gradient[3]<<','<<a.eigenvalues[0]<<','<<a.eigenvalues[1]<<','<<a.eigenvalues[2]<<','<<a.eigenvalues[3]<<','<<a.max_impulse<<','<<a.first_displacement<<','<<a.first_momentum<<','<<a.max_center_displacement<<','<<a.max_state_distance<<','<<a.max_energy_drift<<','<<a.max_common<<','<<a.recovery<<','<<a.max_multiplicity<<','<<a.min_separation<<'\n';std::ofstream translations(dir/(std::string(FTD_FULL_HALF_RESULT_STEM)+"_translation_v1.csv"));translations<<"ftd_id,label,orientation,axis,minus,center,plus,gradient,curvature\n";for(const auto&a:s.arms)for(const auto&t:a.translations)translations<<std::setprecision(17)<<FTD_FULL_HALF_ID<<','<<a.label<<','<<a.orientation<<','<<t.axis<<','<<t.minus<<','<<t.center<<','<<t.plus<<','<<t.gradient<<','<<t.curvature<<'\n';std::ofstream ticks(dir/(std::string(FTD_FULL_HALF_RESULT_STEM)+"_ticks_v1.csv"));ticks<<"ftd_id,label,tick,multiplicity,shared_pairs,separation,state_distance,center_displacement,energy_drift,common\n";for(const auto&a:s.arms)for(const auto&t:a.ticks)ticks<<std::setprecision(17)<<FTD_FULL_HALF_ID<<','<<a.label<<','<<t.tick<<','<<t.multiplicity<<','<<t.shared_pairs<<','<<t.separation<<','<<t.state_distance<<','<<t.center_displacement<<','<<t.energy_drift<<','<<t.common<<'\n';}

}  // namespace

int main(){std::cout<<std::setprecision(17);Summary s;const auto root=std::filesystem::path(__FILE__).parent_path().parent_path();s.parent=fingerprint(root/"results/ftd_0629/ftd_0629_connected_block_linear_modes_v1.json",parent_protocol_sha256,"CONNECTED_BLOCK_ADIABATIC_LINEAR_MODES_CONSTRUCTIVE");s.candidate_provenance=fingerprint(root/FTD_FULL_HALF_CANDIDATE_RESULT,candidate_protocol_sha256,FTD_FULL_HALF_CANDIDATE_VERDICT);const auto n=ftd::eft::measure_face_flux_normalization();s.normalization=n.valid;s.beta=n.mapped_field_work_coefficient;ftd::eft::ConnectedMooreBlockOptions options;options.gate_tolerance=action_gate;options.solve_tolerance=2e-11;options.max_iterations=48;options.allow_shared_anchor_chart=true;if(s.parent&&s.candidate_provenance&&s.normalization){auto x=std::async(std::launch::async,[&](){return run_arm("full_half_x",0,s.beta,options);});auto y=std::async(std::launch::async,[&](){return run_arm("full_half_y",1,s.beta,options);});s.arms.push_back(x.get());std::cout<<"completed full_half_x\n";s.arms.push_back(y.get());std::cout<<"completed full_half_y\n";}evaluate_summary(s);write_records(s);std::cout<<"protocol_sha256="<<protocol_sha256<<'\n'<<"verdict="<<s.verdict<<'\n'<<"reduced="<<s.reduced<<" translation="<<s.translation<<" full="<<s.full_space<<" repeated="<<s.repeated<<" covariance="<<s.covariance_residual<<'\n';for(const auto&a:s.arms){std::cout<<a.label<<" theta=("<<a.theta[0]<<','<<a.theta[1]<<','<<a.theta[2]<<','<<a.theta[3]<<") energy="<<a.refined_energy<<" gradient="<<infinity_norm(a.gradient)<<" min_eigen="<<*std::min_element(a.eigenvalues.begin(),a.eigenvalues.end())<<" impulse="<<a.max_impulse<<" state="<<a.max_state_distance<<" drift="<<a.max_energy_drift<<" recovery="<<a.recovery<<'\n';for(const auto&t:a.translations)std::cout<<"  axis="<<t.axis<<" gradient="<<t.gradient<<" curvature="<<t.curvature<<'\n';}return s.verdict==FTD_FULL_HALF_VERDICT_INVALID?1:0;}
