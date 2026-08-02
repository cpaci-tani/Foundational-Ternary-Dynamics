// FTD-0628: symmetry-reduced static dressing refinement.

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
using Theta = std::array<double,4>;
using Matrix = std::array<std::array<double,4>,4>;
constexpr char protocol_sha256[] =
    "4B6CA4AD4ACF106124AAF9C791AF4F7B3374DC30DF3A5A9FDEDC784F66D640C6";
constexpr char parent_result_sha256[] =
    "E3451D9230A87610B68F8DF27D67C5D536C5582B24818ACF6CB93FDB7E62AE93";
constexpr char parent_protocol_sha256[] =
    "72B38166003A90DF92FFEFEF90F2F363A00A96CFEA4EEDDB8BBC57EE3CAF0A4A";
constexpr int L=17, width=2, tick_count=64;
constexpr double action_gate=1e-10, gradient_step=2e-5;
constexpr double hessian_step=2e-4;

struct EnergyEvaluation {
  bool valid=false;
  double energy=INFINITY, binding=INFINITY, field=INFINITY;
  double gauss=INFINITY;
  ftd::eft::ConnectedMooreBlockState state;
  EnergyEvaluation():state(L){}
};

struct OptimizationRecord {
  int iteration=0;
  Theta theta{{0,0,0,0}};
  double energy=INFINITY, gradient=INFINITY, minimum_eigenvalue=INFINITY;
  double accepted_scale=0.0, gauss=INFINITY;
};

struct EigenSystem {
  Matrix vectors{};
  Theta values{{0,0,0,0}};
};

struct TickRecord {
  int tick=0, multiplicity=1, shared_pairs=0;
  double separation=INFINITY, state_distance=INFINITY;
  double center_displacement=INFINITY, energy_drift=INFINITY;
  double common=INFINITY;
};

struct Arm {
  std::string label;
  int orientation=0;
  bool initialization=false, optimization=false, positive_hessian=false;
  bool one_step=false, full_stationarity=false, forward=false, reverse=false;
  bool repeated=false;
  int evaluations=0, iterations=0;
  Theta rigid{{1.5,0.5,0.5,0.5}}, theta{{0,0,0,0}};
  Theta gradient{{INFINITY,INFINITY,INFINITY,INFINITY}};
  Theta eigenvalues{{INFINITY,INFINITY,INFINITY,INFINITY}};
  Matrix hessian{};
  double rigid_energy=INFINITY, refined_energy=INFINITY;
  double max_impulse=INFINITY, first_displacement=INFINITY;
  double first_momentum=INFINITY, max_center_displacement=0.0;
  double max_state_distance=0.0, max_energy_drift=0.0;
  double max_common=0.0, recovery=INFINITY, min_separation=INFINITY;
  int max_multiplicity=1;
  ftd::eft::ConnectedMooreBlockState initial;
  std::vector<OptimizationRecord> records;
  std::vector<TickRecord> ticks;
  explicit Arm(int size=L):initial(size){}
};

struct Summary {
  bool parent=false, normalization=false, coverage=false, initialization=false;
  bool ansatz=false, full_space=false, repeated=false, covariance=false;
  double beta=0.0, covariance_residual=INFINITY;
  double rotated_state_residual=INFINITY;
  double worst_common=0.0, worst_drift=0.0, worst_recovery=0.0;
  std::string verdict;
  std::vector<Arm> arms;
};

double component(const Vec3& v,int axis) {
  return axis==0?v.x:(axis==1?v.y:v.z);
}

void set_component(Vec3& v,int axis,double value) {
  if(axis==0)v.x=value; else if(axis==1)v.y=value; else v.z=value;
}

Vec3 position(const ftd::eft::MatchedMatterPoint& point) {
  return {point.anchor.x+point.remainder.x,
          point.anchor.y+point.remainder.y,
          point.anchor.z+point.remainder.z};
}

Vec3 center(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 c{}; for(const auto& p:state.constituents)c+=position(p);
  return c*(1.0/static_cast<double>(state.constituents.size()));
}

Vec3 momentum(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 p{}; for(const auto& point:state.constituents)p+=point.momentum;
  return p;
}

double max_component(const Vec3& v) {
  return std::max({std::abs(v.x),std::abs(v.y),std::abs(v.z)});
}

int wrap(int value) { const int r=value%L; return r<0?r+L:r; }

ftd::eft::MatchedMatterPoint point_at(const Vec3& x) {
  ftd::eft::MatchedMatterPoint point;
  const long long ax=std::llround(x.x),ay=std::llround(x.y),az=std::llround(x.z);
  point.anchor={wrap(static_cast<int>(ax)),wrap(static_cast<int>(ay)),
                wrap(static_cast<int>(az))};
  point.remainder={x.x-ax,x.y-ay,x.z-az};
  return point;
}

bool admissible(const Theta& t) {
  return t[0]>=1.25&&t[0]<=1.75&&t[1]>=0.25&&t[1]<=0.75
      &&t[0]-t[1]>=0.50&&t[2]>=0.25&&t[2]<=0.75
      &&t[3]>=0.25&&t[3]<=0.75;
}

ftd::eft::ConnectedMooreBlockState geometry_from(
    const ftd::eft::ConnectedMooreBlockState& base,const Theta& theta,
    int orientation) {
  auto result=base;
  const Vec3 c=center(base);
  for(std::size_t i=0;i<result.constituents.size();++i) {
    const Vec3 d0=position(base.constituents[i])-c;
    const bool outer=std::abs(component(d0,orientation))>1.0;
    Vec3 d{};
    set_component(d,orientation,std::copysign(outer?theta[0]:theta[1],
                                              component(d0,orientation)));
    const double transverse=outer?theta[2]:theta[3];
    for(int axis=0;axis<3;++axis)if(axis!=orientation)
      set_component(d,axis,std::copysign(transverse,component(d0,axis)));
    result.constituents[i]=point_at(c+d);
  }
  return result;
}

EnergyEvaluation evaluate_energy(
    const ftd::eft::ConnectedMooreBlockState& base,const Theta& theta,
    int orientation,double beta,const ftd::eft::ConnectedMooreBlockOptions& options,
    int& evaluations) {
  EnergyEvaluation result; ++evaluations;
  if(!admissible(theta))return result;
  const auto geometry=geometry_from(base,theta,orientation);
  const auto dressed=ftd::eft::redress_connected_moore_block(
      geometry,true,1e-13,4096);
  if(!dressed.valid)return result;
  result.valid=true; result.state=dressed.state; result.gauss=dressed.gauss_residual;
  result.binding=ftd::eft::connected_moore_block_binding_energy(
      dressed.state,options);
  result.field=beta*ftd::eft::matched_modified_energy(
      dressed.state.electric,dressed.state.magnetic_half,ftd::C_SPEED);
  result.energy=result.binding+result.field;
  return result;
}

bool gradient_at(const ftd::eft::ConnectedMooreBlockState& base,
                 const Theta& theta,int orientation,double beta,
                 const ftd::eft::ConnectedMooreBlockOptions& options,
                 int& evaluations,Theta& gradient) {
  for(int i=0;i<4;++i) {
    Theta plus=theta,minus=theta;plus[i]+=gradient_step;minus[i]-=gradient_step;
    const auto ep=evaluate_energy(base,plus,orientation,beta,options,evaluations);
    const auto em=evaluate_energy(base,minus,orientation,beta,options,evaluations);
    if(!ep.valid||!em.valid)return false;
    gradient[i]=(ep.energy-em.energy)/(2.0*gradient_step);
  }
  return true;
}

bool hessian_at(const ftd::eft::ConnectedMooreBlockState& base,
                const Theta& theta,int orientation,double beta,
                const ftd::eft::ConnectedMooreBlockOptions& options,
                int& evaluations,Matrix& hessian) {
  const auto e0=evaluate_energy(base,theta,orientation,beta,options,evaluations);
  if(!e0.valid)return false;
  for(int i=0;i<4;++i) {
    Theta plus=theta,minus=theta;plus[i]+=hessian_step;minus[i]-=hessian_step;
    const auto ep=evaluate_energy(base,plus,orientation,beta,options,evaluations);
    const auto em=evaluate_energy(base,minus,orientation,beta,options,evaluations);
    if(!ep.valid||!em.valid)return false;
    hessian[i][i]=(ep.energy-2.0*e0.energy+em.energy)
        /(hessian_step*hessian_step);
    for(int j=i+1;j<4;++j) {
      Theta pp=theta,pm=theta,mp=theta,mm=theta;
      pp[i]+=hessian_step;pp[j]+=hessian_step;
      pm[i]+=hessian_step;pm[j]-=hessian_step;
      mp[i]-=hessian_step;mp[j]+=hessian_step;
      mm[i]-=hessian_step;mm[j]-=hessian_step;
      const auto epp=evaluate_energy(base,pp,orientation,beta,options,evaluations);
      const auto epm=evaluate_energy(base,pm,orientation,beta,options,evaluations);
      const auto emp=evaluate_energy(base,mp,orientation,beta,options,evaluations);
      const auto emm=evaluate_energy(base,mm,orientation,beta,options,evaluations);
      if(!epp.valid||!epm.valid||!emp.valid||!emm.valid)return false;
      hessian[i][j]=hessian[j][i]=(epp.energy-epm.energy-emp.energy+emm.energy)
          /(4.0*hessian_step*hessian_step);
    }
  }
  return true;
}

EigenSystem diagonalize(Matrix matrix) {
  EigenSystem result;
  for(int i=0;i<4;++i)result.vectors[i][i]=1.0;
  for(int iteration=0;iteration<96;++iteration) {
    int p=0,q=1;double largest=std::abs(matrix[p][q]);
    for(int i=0;i<4;++i)for(int j=i+1;j<4;++j)
      if(std::abs(matrix[i][j])>largest){largest=std::abs(matrix[i][j]);p=i;q=j;}
    if(largest<1e-13)break;
    const double angle=0.5*std::atan2(2.0*matrix[p][q],matrix[q][q]-matrix[p][p]);
    const double c=std::cos(angle),s=std::sin(angle);
    for(int k=0;k<4;++k)if(k!=p&&k!=q) {
      const double mkp=matrix[k][p],mkq=matrix[k][q];
      matrix[k][p]=matrix[p][k]=c*mkp-s*mkq;
      matrix[k][q]=matrix[q][k]=s*mkp+c*mkq;
    }
    const double app=matrix[p][p],aqq=matrix[q][q],apq=matrix[p][q];
    matrix[p][p]=c*c*app-2*c*s*apq+s*s*aqq;
    matrix[q][q]=s*s*app+2*c*s*apq+c*c*aqq;
    matrix[p][q]=matrix[q][p]=0.0;
    for(int k=0;k<4;++k) {
      const double vkp=result.vectors[k][p],vkq=result.vectors[k][q];
      result.vectors[k][p]=c*vkp-s*vkq;
      result.vectors[k][q]=s*vkp+c*vkq;
    }
  }
  for(int i=0;i<4;++i)result.values[i]=matrix[i][i];
  return result;
}

double infinity_norm(const Theta& v) {
  return std::max({std::abs(v[0]),std::abs(v[1]),std::abs(v[2]),std::abs(v[3])});
}

Theta regularized_newton_step(const Theta& gradient,const EigenSystem& eig) {
  Theta projected{{0,0,0,0}},step{{0,0,0,0}};
  for(int mode=0;mode<4;++mode)for(int i=0;i<4;++i)
    projected[mode]+=eig.vectors[i][mode]*gradient[i];
  for(int mode=0;mode<4;++mode)
    projected[mode]/=std::max(1e-6,eig.values[mode]);
  for(int i=0;i<4;++i)for(int mode=0;mode<4;++mode)
    step[i]-=eig.vectors[i][mode]*projected[mode];
  return step;
}

bool parent_fingerprint() {
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0627/ftd_0627_connected_block_dynamical_rest_v1.json";
  std::ifstream in(path,std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(in)),{});
  return bytes.find(parent_protocol_sha256)!=std::string::npos
      &&bytes.find("CONNECTED_BLOCK_BOUNDED_IRREGULAR_REST_OPEN")!=std::string::npos;
}

double common_residual(const ftd::eft::ConnectedMooreBlockStepResult& s) {
  return std::max({s.root_residual,s.continuity_residual,
      s.gauss_before_residual,s.gauss_after_residual,s.force_residual,
      s.kinematic_residual,s.kinetic_discrete_gradient_residual,
      s.electric_adjoint_residual,s.magnetic_work_residual,
      s.binding_work_residual,s.binding_impulse_sum_residual,
      s.matter_work_residual,s.field_work_residual,s.total_energy_residual,
      s.causal_speed_excess});
}

double kinetic(const ftd::eft::ConnectedMooreBlockState& state) {
  long double result=0.0L;
  for(const auto& p:state.constituents)
    result+=ftd::eft::production_flat_energy_from_momentum(p.momentum);
  return static_cast<double>(result);
}

double total_energy(const ftd::eft::ConnectedMooreBlockState& state,double beta,
                    const ftd::eft::ConnectedMooreBlockOptions& options) {
  return kinetic(state)+ftd::eft::connected_moore_block_binding_energy(state,options)
      +beta*ftd::eft::matched_modified_energy(
          state.electric,state.magnetic_half,ftd::C_SPEED);
}

std::tuple<int,int,double> fibre_metrics(
    const ftd::eft::ConnectedMooreBlockState& state) {
  std::map<std::tuple<int,int,int>,std::vector<std::size_t>> groups;
  for(std::size_t i=0;i<state.constituents.size();++i) {
    const auto& a=state.constituents[i].anchor;
    groups[{a.x,a.y,a.z}].push_back(i);
  }
  int maximum=1,pairs=0;double separation=INFINITY;
  for(const auto& group:groups) {
    maximum=std::max(maximum,static_cast<int>(group.second.size()));
    for(std::size_t i=0;i<group.second.size();++i)
      for(std::size_t j=i+1;j<group.second.size();++j) {
        ++pairs;separation=std::min(separation,
            (position(state.constituents[group.second[i]])
             -position(state.constituents[group.second[j]])).mag());
      }
  }
  return {maximum,pairs,separation};
}

Arm run_arm(const std::string& label,int orientation,double beta,
            const ftd::eft::ConnectedMooreBlockOptions& options) {
  Arm result;result.label=label;result.orientation=orientation;
  const auto parent=ftd::eft::initialize_connected_moore_block(
      L,width,orientation,orientation,0.5,1e-13,4096);
  result.initialization=parent.valid&&parent.state.constituents.size()==16
      &&parent.state.edges.size()==72;
  if(!result.initialization)return result;
  const auto base=parent.state;
  auto rigid=evaluate_energy(base,result.rigid,orientation,beta,options,result.evaluations);
  if(!rigid.valid)return result;
  result.rigid_energy=rigid.energy;
  Theta theta=result.rigid;
  for(int iteration=0;iteration<16;++iteration) {
    Theta gradient{};Matrix hessian{};
    auto current=evaluate_energy(base,theta,orientation,beta,options,result.evaluations);
    if(!current.valid||!gradient_at(base,theta,orientation,beta,options,
                                   result.evaluations,gradient)
        ||!hessian_at(base,theta,orientation,beta,options,
                      result.evaluations,hessian))break;
    const auto eig=diagonalize(hessian);
    result.records.push_back({iteration,theta,current.energy,
        infinity_norm(gradient),*std::min_element(eig.values.begin(),eig.values.end()),
        0.0,current.gauss});
    result.iterations=iteration+1;
    if(infinity_norm(gradient)<=1e-9)break;
    const Theta step=regularized_newton_step(gradient,eig);
    bool accepted=false;
    for(int backtrack=0;backtrack<=10;++backtrack) {
      const double scale=std::ldexp(1.0,-backtrack);
      Theta trial=theta;for(int i=0;i<4;++i)trial[i]+=scale*step[i];
      const auto evaluated=evaluate_energy(
          base,trial,orientation,beta,options,result.evaluations);
      if(admissible(trial)&&evaluated.valid&&evaluated.energy<current.energy) {
        theta=trial;result.records.back().accepted_scale=scale;accepted=true;break;
      }
    }
    if(!accepted)break;
  }
  result.theta=theta;
  auto refined=evaluate_energy(base,theta,orientation,beta,options,result.evaluations);
  if(!refined.valid||!gradient_at(base,theta,orientation,beta,options,
                                 result.evaluations,result.gradient)
      ||!hessian_at(base,theta,orientation,beta,options,
                    result.evaluations,result.hessian))return result;
  const auto eig=diagonalize(result.hessian);result.eigenvalues=eig.values;
  result.refined_energy=refined.energy;result.initial=refined.state;
  result.optimization=infinity_norm(result.gradient)<=1e-9
      &&result.refined_energy<result.rigid_energy;
  result.positive_hessian=*std::min_element(
      result.eigenvalues.begin(),result.eigenvalues.end())>1e-6;

  const auto first=ftd::eft::solve_connected_moore_block_forward(
      result.initial,options);
  result.one_step=first.common_action_gates_pass&&common_residual(first)<=action_gate;
  result.max_impulse=0.0;
  if(result.one_step)for(const auto& impulse:first.total_impulses)
    result.max_impulse=std::max(result.max_impulse,max_component(impulse));
  result.first_displacement=result.one_step
      ?ftd::eft::connected_moore_block_state_max_difference(
          result.initial,first.later):INFINITY;
  result.first_momentum=result.one_step?max_component(momentum(first.later)):INFINITY;
  result.full_stationarity=result.one_step&&result.max_impulse<=1e-9
      &&result.first_displacement<=1e-9&&result.first_momentum<=1e-9;

  auto state=result.initial;const Vec3 c0=center(state);
  const double energy0=total_energy(state,beta,options);
  result.forward=result.optimization&&result.positive_hessian&&result.one_step;
  for(int tick=1;result.forward&&tick<=tick_count;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_forward(state,options);
    const double common=common_residual(step);
    if(!step.common_action_gates_pass||common>action_gate){result.forward=false;break;}
    state=step.later;
    int multiplicity=1,pairs=0;double separation=INFINITY;
    std::tie(multiplicity,pairs,separation)=fibre_metrics(state);
    const double distance=ftd::eft::connected_moore_block_state_max_difference(
        result.initial,state);
    const double displacement=(center(state)-c0).mag();
    const double drift=std::abs(total_energy(state,beta,options)-energy0);
    result.max_multiplicity=std::max(result.max_multiplicity,multiplicity);
    if(std::isfinite(separation))result.min_separation=std::min(result.min_separation,separation);
    result.max_state_distance=std::max(result.max_state_distance,distance);
    result.max_center_displacement=std::max(result.max_center_displacement,displacement);
    result.max_energy_drift=std::max(result.max_energy_drift,drift);
    result.max_common=std::max(result.max_common,common);
    result.ticks.push_back({tick,multiplicity,pairs,separation,distance,
                            displacement,drift,common});
  }
  result.forward=result.forward&&result.ticks.size()==tick_count;
  result.reverse=result.forward;
  for(int tick=tick_count;result.reverse&&tick>=1;--tick) {
    const auto step=ftd::eft::solve_connected_moore_block_reverse(state,options);
    const double common=common_residual(step);
    result.max_common=std::max(result.max_common,common);
    if(!step.common_action_gates_pass||common>action_gate){result.reverse=false;break;}
    state=step.earlier;
  }
  if(result.reverse)result.recovery=ftd::eft::connected_moore_block_state_max_difference(
      result.initial,state);
  result.repeated=result.forward&&result.reverse&&result.full_stationarity
      &&result.max_center_displacement<=1e-10&&result.max_state_distance<=1e-8
      &&result.max_energy_drift<=1e-12&&result.recovery<=1e-10
      &&result.max_multiplicity<=2
      &&(!std::isfinite(result.min_separation)||result.min_separation>=0.9);
  return result;
}

double relative(double a,double b) {
  return std::abs(a-b)/std::max({1.0,std::abs(a),std::abs(b)});
}

std::size_t index(int x,int y,int z) {
  return static_cast<std::size_t>((wrap(x)*L+wrap(y))*L+wrap(z));
}

double rotated_state_covariance(
    const ftd::eft::ConnectedMooreBlockState& x,
    const ftd::eft::ConnectedMooreBlockState& y) {
  double result=0.0;
  std::vector<bool> used(y.constituents.size(),false);
  for(std::size_t i=0;i<x.constituents.size();++i) {
    const Vec3 p=position(x.constituents[i]);
    const Vec3 rotated{p.z,p.x,p.y};
    const Vec3 rotated_momentum{x.constituents[i].momentum.z,
                                x.constituents[i].momentum.x,
                                x.constituents[i].momentum.y};
    double best=INFINITY;std::size_t selected=y.constituents.size();
    for(std::size_t j=0;j<y.constituents.size();++j)if(!used[j]
        &&x.charges[i]==y.charges[j]) {
      const double distance=(rotated-position(y.constituents[j])).mag();
      if(distance<best){best=distance;selected=j;}
    }
    if(selected==y.constituents.size())return INFINITY;
    used[selected]=true;
    result=std::max({result,best,
        max_component(rotated_momentum-y.constituents[selected].momentum)});
  }
  for(int ix=0;ix<L;++ix)for(int iy=0;iy<L;++iy)for(int iz=0;iz<L;++iz) {
    const std::size_t old=index(ix,iy,iz),rotated=index(iz,ix,iy);
    result=std::max({result,
      std::abs(x.electric.z[old]-y.electric.x[rotated]),
      std::abs(x.electric.x[old]-y.electric.y[rotated]),
      std::abs(x.electric.y[old]-y.electric.z[rotated]),
      std::abs(x.magnetic_half.z[old]-y.magnetic_half.x[rotated]),
      std::abs(x.magnetic_half.x[old]-y.magnetic_half.y[rotated]),
      std::abs(x.magnetic_half.y[old]-y.magnetic_half.z[rotated])});
  }
  return result;
}

double covariance(const Arm& a,const Arm& b) {
  double result=0.0;
  for(int i=0;i<4;++i)result=std::max({result,std::abs(a.theta[i]-b.theta[i]),
      relative(a.gradient[i],b.gradient[i]),relative(a.eigenvalues[i],b.eigenvalues[i])});
  result=std::max({result,relative(a.rigid_energy,b.rigid_energy),
      relative(a.refined_energy,b.refined_energy),relative(a.max_impulse,b.max_impulse),
      relative(a.max_state_distance,b.max_state_distance),
      relative(a.max_energy_drift,b.max_energy_drift),relative(a.recovery,b.recovery)});
  return result;
}

void evaluate(Summary& s) {
  s.coverage=s.arms.size()==2&&s.arms[0].orientation==0&&s.arms[1].orientation==1;
  s.initialization=s.coverage&&std::all_of(s.arms.begin(),s.arms.end(),
      [](const Arm& a){return a.initialization;});
  s.ansatz=s.initialization&&std::all_of(s.arms.begin(),s.arms.end(),
      [](const Arm& a){return a.optimization&&a.positive_hessian;});
  s.full_space=s.ansatz&&std::all_of(s.arms.begin(),s.arms.end(),
      [](const Arm& a){return a.full_stationarity;});
  s.repeated=s.full_space&&std::all_of(s.arms.begin(),s.arms.end(),
      [](const Arm& a){return a.repeated;});
  if(s.coverage){s.rotated_state_residual=rotated_state_covariance(
                     s.arms[0].initial,s.arms[1].initial);
                 s.covariance_residual=std::max(
                     covariance(s.arms[0],s.arms[1]),s.rotated_state_residual);
                 s.covariance=s.covariance_residual<=1e-9;}
  for(const auto& a:s.arms){s.worst_common=std::max(s.worst_common,a.max_common);
    s.worst_drift=std::max(s.worst_drift,a.max_energy_drift);
    if(std::isfinite(a.recovery))s.worst_recovery=std::max(s.worst_recovery,a.recovery);}
  if(!s.parent||!s.normalization||!s.coverage||!s.initialization||!s.covariance)
    s.verdict="CONNECTED_BLOCK_STATIC_REFINEMENT_EXECUTION_INVALID";
  else if(s.repeated)s.verdict="CONNECTED_BLOCK_STATIC_DRESSED_FIXED_POINT_CONSTRUCTIVE";
  else if(s.ansatz)s.verdict="CONNECTED_BLOCK_SYMMETRY_STATIONARY_ONLY";
  else s.verdict="CONNECTED_BLOCK_STATIC_REFINEMENT_CLOSED_NEGATIVE";
}

void number(std::ostream& out,double value){if(std::isfinite(value))out<<value;else out<<"null";}

void write_records(const Summary& s) {
  const auto dir=std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0628";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0628_connected_block_static_dressing_refinement_v1.json");
  json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0628\",\n"
      <<"  \"protocol_sha256\": \""<<protocol_sha256<<"\",\n"
      <<"  \"parent_result_sha256\": \""<<parent_result_sha256<<"\",\n"
      <<"  \"verdict\": \""<<s.verdict<<"\",\n"
      <<"  \"production_changed\": false,\n"
      <<"  \"coverage_pass\": "<<s.coverage<<",\n"
      <<"  \"initialization_pass\": "<<s.initialization<<",\n"
      <<"  \"ansatz_stationarity_pass\": "<<s.ansatz<<",\n"
      <<"  \"full_space_stationarity_pass\": "<<s.full_space<<",\n"
      <<"  \"repeated_fixed_point_pass\": "<<s.repeated<<",\n"
      <<"  \"covariance_pass\": "<<s.covariance<<",\n"
      <<"  \"rotated_state_residual\": ";number(json,s.rotated_state_residual);
  json<<",\n"
      <<"  \"covariance_residual\": ";number(json,s.covariance_residual);
  json<<",\n  \"worst_common_residual\": "<<s.worst_common
      <<",\n  \"worst_energy_drift\": "<<s.worst_drift
      <<",\n  \"worst_recovery\": "<<s.worst_recovery<<"\n}\n";

  std::ofstream opt(dir/"ftd_0628_connected_block_static_dressing_optimization_v1.csv");
  opt<<"ftd_id,label,iteration,a,b,t_outer,t_inner,energy,gradient_inf,min_eigenvalue,accepted_scale,gauss\n";
  for(const auto& a:s.arms)for(const auto& r:a.records)
    opt<<std::setprecision(17)<<"FTD-0628,"<<a.label<<','<<r.iteration<<','
       <<r.theta[0]<<','<<r.theta[1]<<','<<r.theta[2]<<','<<r.theta[3]<<','
       <<r.energy<<','<<r.gradient<<','<<r.minimum_eigenvalue<<','
       <<r.accepted_scale<<','<<r.gauss<<'\n';

  std::ofstream arms(dir/"ftd_0628_connected_block_static_dressing_arms_v1.csv");
  arms<<"ftd_id,label,orientation,init,optimization,positive_hessian,one_step,full_stationarity,forward,reverse,repeated,evaluations,iterations,rigid_energy,refined_energy,a,b,t_outer,t_inner,g0,g1,g2,g3,e0,e1,e2,e3,h00,h01,h02,h03,h10,h11,h12,h13,h20,h21,h22,h23,h30,h31,h32,h33,max_impulse,first_displacement,first_momentum,max_center_displacement,max_state_distance,max_energy_drift,max_common,recovery,max_multiplicity,min_separation\n";
  for(const auto& a:s.arms)arms<<std::setprecision(17)<<"FTD-0628,"<<a.label<<','
      <<a.orientation<<','<<a.initialization<<','<<a.optimization<<','
      <<a.positive_hessian<<','<<a.one_step<<','<<a.full_stationarity<<','
      <<a.forward<<','<<a.reverse<<','<<a.repeated<<','<<a.evaluations<<','
      <<a.iterations<<','<<a.rigid_energy<<','<<a.refined_energy<<','
      <<a.theta[0]<<','<<a.theta[1]<<','<<a.theta[2]<<','<<a.theta[3]<<','
      <<a.gradient[0]<<','<<a.gradient[1]<<','<<a.gradient[2]<<','<<a.gradient[3]<<','
      <<a.eigenvalues[0]<<','<<a.eigenvalues[1]<<','<<a.eigenvalues[2]<<','
      <<a.eigenvalues[3]<<','
      <<a.hessian[0][0]<<','<<a.hessian[0][1]<<','<<a.hessian[0][2]<<','<<a.hessian[0][3]<<','
      <<a.hessian[1][0]<<','<<a.hessian[1][1]<<','<<a.hessian[1][2]<<','<<a.hessian[1][3]<<','
      <<a.hessian[2][0]<<','<<a.hessian[2][1]<<','<<a.hessian[2][2]<<','<<a.hessian[2][3]<<','
      <<a.hessian[3][0]<<','<<a.hessian[3][1]<<','<<a.hessian[3][2]<<','<<a.hessian[3][3]<<','
      <<a.max_impulse<<','<<a.first_displacement<<','
      <<a.first_momentum<<','<<a.max_center_displacement<<','<<a.max_state_distance
      <<','<<a.max_energy_drift<<','<<a.max_common<<','<<a.recovery<<','
      <<a.max_multiplicity<<','<<a.min_separation<<'\n';

  std::ofstream ticks(dir/"ftd_0628_connected_block_static_dressing_ticks_v1.csv");
  ticks<<"ftd_id,label,tick,multiplicity,shared_pairs,separation,state_distance,center_displacement,energy_drift,common\n";
  for(const auto& a:s.arms)for(const auto& t:a.ticks)
    ticks<<std::setprecision(17)<<"FTD-0628,"<<a.label<<','<<t.tick<<','
      <<t.multiplicity<<','<<t.shared_pairs<<','<<t.separation<<','
      <<t.state_distance<<','<<t.center_displacement<<','<<t.energy_drift<<','
      <<t.common<<'\n';
}

}  // namespace

int main() {
  std::cout<<std::setprecision(17);
  Summary s;s.parent=parent_fingerprint();
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  s.normalization=normalization.valid;s.beta=normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.gate_tolerance=action_gate;options.solve_tolerance=2e-11;
  options.max_iterations=48;options.allow_shared_anchor_chart=true;
  if(s.parent&&s.normalization) {
    auto x=std::async(std::launch::async,[&](){return run_arm("static_x",0,s.beta,options);});
    auto y=std::async(std::launch::async,[&](){return run_arm("static_y",1,s.beta,options);});
    s.arms.push_back(x.get());std::cout<<"completed static_x"<<std::endl;
    s.arms.push_back(y.get());std::cout<<"completed static_y"<<std::endl;
  }
  evaluate(s);write_records(s);
  std::cout<<"protocol_sha256="<<protocol_sha256<<'\n'<<"verdict="<<s.verdict
      <<'\n'<<"initialization="<<s.initialization<<" ansatz="<<s.ansatz
      <<" full_space="<<s.full_space<<" repeated="<<s.repeated
      <<" covariance="<<s.covariance_residual<<'\n';
  for(const auto& a:s.arms)
    std::cout<<a.label<<" optimization="<<a.optimization
      <<" positive_hessian="<<a.positive_hessian
      <<" full_stationarity="<<a.full_stationarity<<" repeated="<<a.repeated
      <<" theta=("<<a.theta[0]<<','<<a.theta[1]<<','<<a.theta[2]<<','<<a.theta[3]
      <<") gradient="<<infinity_norm(a.gradient)
      <<" min_eigen="<<*std::min_element(a.eigenvalues.begin(),a.eigenvalues.end())
      <<" impulse="<<a.max_impulse<<" first="<<a.first_displacement
      <<" state="<<a.max_state_distance<<" drift="<<a.max_energy_drift
      <<" common="<<a.max_common<<" recovery="<<a.recovery<<'\n';
  return s.verdict=="CONNECTED_BLOCK_STATIC_REFINEMENT_EXECUTION_INVALID"?1:0;
}
