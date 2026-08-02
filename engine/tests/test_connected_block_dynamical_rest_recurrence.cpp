// FTD-0627: long-horizon classification of fibre-enabled centre rest.

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
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace {

using ftd::Vec3;
constexpr char protocol_sha256[] =
    "72B38166003A90DF92FFEFEF90F2F363A00A96CFEA4EEDDB8BBC57EE3CAF0A4A";
constexpr char parent_sha256[] =
    "DEDFF2C31C510A7944CF5FD213E1165172342324B6C38432D599F4F212570308";
constexpr char parent_protocol_sha256[] =
    "67806EA9B3D8ED02B2BF04A839B21E1053FDE1199DE46FB2E064D6E061544C52";
constexpr int L = 17, width = 2, tick_count = 256;
constexpr double action_gate = 1e-10;

struct Spectrum {
  std::string observable;
  bool nonzero = false;
  double total_power = 0.0, concentration = 0.0;
  std::array<int,8> bins{};
  std::array<double,8> powers{};
};

struct Tick {
  int index = 0, multiplicity = 1, shared_pairs = 0;
  Vec3 displacement{}, momentum{};
  double state_distance = INFINITY, internal_distance = INFINITY;
  double shape = INFINITY, strain = INFINITY, separation = INFINITY;
  double kinetic = INFINITY, binding = INFINITY, field = INFINITY;
  double total = INFINITY, drift = INFINITY;
  std::array<double,3> q{{0,0,0}};
  double interface_coordinate = 0.0, common = INFINITY;
};

struct Arm {
  std::string label;
  int orientation = 0, phase_axis = 0;
  bool initialization = false, forward = false, reverse = false;
  bool exact = false, metadata = false, bounded = false;
  int failure_tick = 0, period = 0, max_multiplicity = 1;
  double min_separation = INFINITY, max_state_distance = 0.0;
  double max_internal_distance = 0.0, max_shape = 0.0;
  double max_strain = 0.0, max_displacement = 0.0;
  double max_momentum = 0.0, max_common = 0.0, max_drift = 0.0;
  double recovery = INFINITY;
  std::vector<Tick> ticks;
  std::vector<Spectrum> spectra;
};

struct Summary {
  bool parent = false, normalization = false, coverage = false;
  bool execution = false, bounded = false, recurrence = false;
  bool spectral = false, covariance = false;
  double beta = 0.0, covariance_residual = INFINITY;
  double worst_common = 0.0, worst_drift = 0.0, worst_recovery = 0.0;
  int common_period = 0;
  std::string verdict;
  std::vector<Arm> arms;
};

Vec3 position(const ftd::eft::MatchedMatterPoint& point) {
  return {point.anchor.x+point.remainder.x,
          point.anchor.y+point.remainder.y,
          point.anchor.z+point.remainder.z};
}

Vec3 center(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += position(point);
  return result*(1.0/static_cast<double>(state.constituents.size()));
}

Vec3 momentum(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += point.momentum;
  return result;
}

Vec3 cycle(const Vec3& value) { return {value.z,value.x,value.y}; }

double max_component(const Vec3& value) {
  return std::max({std::abs(value.x),std::abs(value.y),std::abs(value.z)});
}

double relative(double lhs, double rhs) {
  return std::abs(lhs-rhs)/std::max({1e-300,std::abs(lhs),std::abs(rhs)});
}

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

double kinetic(const ftd::eft::ConnectedMooreBlockState& state) {
  long double result = 0.0L;
  for (const auto& point : state.constituents)
    result += ftd::eft::production_flat_energy_from_momentum(point.momentum);
  return static_cast<double>(result);
}

double field(const ftd::eft::ConnectedMooreBlockState& state, double beta) {
  return beta*ftd::eft::matched_modified_energy(
      state.electric,state.magnetic_half,ftd::C_SPEED);
}

double shape(const ftd::eft::ConnectedMooreBlockState& initial,
             const ftd::eft::ConnectedMooreBlockState& state) {
  const Vec3 c0 = center(initial), c1 = center(state);
  long double sum = 0.0L;
  for (std::size_t i = 0; i < state.constituents.size(); ++i) {
    const Vec3 delta = (position(state.constituents[i])-c1)
        -(position(initial.constituents[i])-c0);
    sum += delta.dot(delta);
  }
  return std::sqrt(static_cast<double>(
      sum/static_cast<long double>(state.constituents.size())));
}

double internal_distance(const ftd::eft::ConnectedMooreBlockState& initial,
                         const ftd::eft::ConnectedMooreBlockState& state) {
  const Vec3 c0 = center(initial), c1 = center(state);
  double result = 0.0;
  for (std::size_t i = 0; i < state.constituents.size(); ++i)
    result = std::max({result,
        max_component((position(state.constituents[i])-c1)
                      -(position(initial.constituents[i])-c0)),
        max_component(state.constituents[i].momentum
                      -initial.constituents[i].momentum)});
  return result;
}

std::tuple<int,int,double> fibre_metrics(
    const ftd::eft::ConnectedMooreBlockState& state) {
  std::map<std::tuple<int,int,int>,std::vector<std::size_t>> grouped;
  for (std::size_t i=0;i<state.constituents.size();++i) {
    const auto& a=state.constituents[i].anchor;
    grouped[{a.x,a.y,a.z}].push_back(i);
  }
  int maximum=1,pairs=0;
  double minimum=INFINITY;
  for (const auto& group:grouped) {
    maximum=std::max(maximum,static_cast<int>(group.second.size()));
    for (std::size_t i=0;i<group.second.size();++i)
      for (std::size_t j=i+1;j<group.second.size();++j) {
        ++pairs;
        minimum=std::min(minimum,
            (position(state.constituents[group.second[i]])
             -position(state.constituents[group.second[j]])).mag());
      }
  }
  return {maximum,pairs,minimum};
}

std::array<double,3> bond_coordinates(
    const ftd::eft::ConnectedMooreBlockState& state) {
  std::array<long double,3> sums{{0,0,0}};
  std::array<int,3> counts{{0,0,0}};
  for (const auto& edge:state.edges) {
    const int shell=static_cast<int>(std::llround(edge.rest_length_squared))-1;
    if (shell<0 || shell>2) continue;
    const Vec3 delta=position(state.constituents[edge.first])
        -position(state.constituents[edge.second]);
    sums[shell]+=delta.dot(delta)-edge.rest_length_squared;
    ++counts[shell];
  }
  std::array<double,3> result{{0,0,0}};
  for (int shell=0;shell<3;++shell)
    if (counts[shell]>0) result[shell]=static_cast<double>(
        sums[shell]/static_cast<long double>(counts[shell]));
  return result;
}

double interface_coordinate(
    const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 positive{},negative{};
  int np=0,nn=0;
  for (std::size_t i=0;i<state.constituents.size();++i)
    if (state.charges[i]>0) { positive+=position(state.constituents[i]); ++np; }
    else { negative+=position(state.constituents[i]); ++nn; }
  positive*=1.0/np; negative*=1.0/nn;
  return component(positive-negative,state.orientation_axis);
}

bool metadata_equal(const ftd::eft::ConnectedMooreBlockState& a,
                    const ftd::eft::ConnectedMooreBlockState& b) {
  if (a.width!=b.width || a.orientation_axis!=b.orientation_axis
      || a.charges!=b.charges || a.constituents.size()!=b.constituents.size()
      || a.edges.size()!=b.edges.size()) return false;
  for (std::size_t i=0;i<a.edges.size();++i) {
    const auto& x=a.edges[i]; const auto& y=b.edges[i];
    if (x.first!=y.first || x.second!=y.second
        || x.reference_delta.x!=y.reference_delta.x
        || x.reference_delta.y!=y.reference_delta.y
        || x.reference_delta.z!=y.reference_delta.z
        || x.rest_length_squared!=y.rest_length_squared) return false;
  }
  return true;
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

Spectrum spectrum(const std::string& name,const std::vector<double>& values) {
  Spectrum result; result.observable=name;
  const double mean=std::accumulate(values.begin(),values.end(),0.0)
      /static_cast<double>(values.size());
  std::vector<std::pair<double,int>> powers;
  for (int k=1;k<=tick_count/2;++k) {
    long double real=0.0L,imaginary=0.0L;
    for (int n=0;n<tick_count;++n) {
      const long double angle=2.0L*std::acos(-1.0L)*k*n/tick_count;
      const long double value=values[n]-mean;
      real+=value*std::cos(angle); imaginary-=value*std::sin(angle);
    }
    const double power=static_cast<double>(real*real+imaginary*imaginary);
    powers.push_back({power,k}); result.total_power+=power;
  }
  result.nonzero=result.total_power>1e-28;
  std::sort(powers.begin(),powers.end(),[](const auto& a,const auto& b) {
    if (a.first!=b.first) return a.first>b.first;
    return a.second<b.second;
  });
  double selected=0.0;
  for (int i=0;i<8;++i) {
    result.powers[i]=powers[i].first; result.bins[i]=powers[i].second;
    selected+=powers[i].first;
  }
  if (result.nonzero) result.concentration=selected/result.total_power;
  return result;
}

bool parent_fingerprint() {
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0626/ftd_0626_connected_block_shared_anchor_fibre_v1.json";
  std::ifstream in(path,std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(in)),{});
  return bytes.find(parent_protocol_sha256)!=std::string::npos
      && bytes.find("CONNECTED_BLOCK_FIBRE_CLOSED_NEGATIVE")!=std::string::npos;
}

Arm run_arm(const std::string& label,int orientation,int phase_axis,
            const ftd::eft::ConnectedMooreBlockOptions& options,double beta) {
  Arm result; result.label=label; result.orientation=orientation;
  result.phase_axis=phase_axis;
  const auto init=ftd::eft::initialize_connected_moore_block(
      L,width,orientation,phase_axis,0.5);
  result.initialization=init.valid && init.state.constituents.size()==16
      && init.state.edges.size()==72 && init.gauss_residual<=1e-11;
  if (!result.initialization) return result;
  const auto initial=init.state;
  auto state=initial;
  const Vec3 c0=center(initial);
  const double kinetic0=kinetic(initial);
  const double binding0=ftd::eft::connected_moore_block_binding_energy(
      initial,options);
  const double field0=field(initial,beta);
  const double total0=kinetic0+binding0+field0;
  result.metadata=result.exact=true;
  for (int tick=1;tick<=tick_count;++tick) {
    const auto step=ftd::eft::solve_connected_moore_block_forward(state,options);
    const double common=common_residual(step);
    result.max_common=std::max(result.max_common,common);
    if (!step.common_action_gates_pass || common>action_gate
        || !step.graph_connected || !step.graph_local) {
      result.failure_tick=tick; result.exact=false; break;
    }
    result.metadata=result.metadata&&metadata_equal(initial,step.later);
    state=step.later;
    int multiplicity=1,pairs=0; double separation=INFINITY;
    std::tie(multiplicity,pairs,separation)=fibre_metrics(state);
    const Vec3 displacement=center(state)-c0,p=momentum(state);
    const double shape_value=shape(initial,state);
    const double internal=internal_distance(initial,state);
    const double k=kinetic(state);
    const double b=ftd::eft::connected_moore_block_binding_energy(state,options);
    const double f=field(state,beta),t=k+b+f,drift=std::abs(t-total0);
    const double distance=ftd::eft::connected_moore_block_state_max_difference(
        initial,state);
    result.max_multiplicity=std::max(result.max_multiplicity,multiplicity);
    if (std::isfinite(separation))
      result.min_separation=std::min(result.min_separation,separation);
    result.max_state_distance=std::max(result.max_state_distance,distance);
    result.max_internal_distance=std::max(result.max_internal_distance,internal);
    result.max_shape=std::max(result.max_shape,shape_value);
    result.max_strain=std::max(result.max_strain,step.maximum_edge_strain);
    result.max_displacement=std::max(result.max_displacement,displacement.mag());
    result.max_momentum=std::max(result.max_momentum,p.mag());
    result.max_drift=std::max(result.max_drift,drift);
    result.ticks.push_back({tick,multiplicity,pairs,displacement,p,distance,
        internal,shape_value,step.maximum_edge_strain,separation,k,b,f,t,drift,
        bond_coordinates(state),interface_coordinate(state),common});
  }
  result.forward=result.exact && result.ticks.size()==tick_count;
  if (result.forward) {
    for (int period=16;period<=128;++period)
      if (result.ticks[period-1].state_distance<=1e-6
          && result.ticks[2*period-1].state_distance<=1e-6) {
        result.period=period; break;
      }
    std::vector<double> q1,q2,q3,interface_values;
    for (const auto& tick:result.ticks) {
      q1.push_back(tick.q[0]); q2.push_back(tick.q[1]);
      q3.push_back(tick.q[2]);
      interface_values.push_back(tick.interface_coordinate);
    }
    result.spectra.push_back(spectrum("Q1",q1));
    result.spectra.push_back(spectrum("Q2",q2));
    result.spectra.push_back(spectrum("Q3",q3));
    result.spectra.push_back(spectrum("interface",interface_values));
  }
  result.reverse=result.forward;
  for (int tick=tick_count;result.reverse&&tick>=1;--tick) {
    const auto step=ftd::eft::solve_connected_moore_block_reverse(state,options);
    result.max_common=std::max(result.max_common,common_residual(step));
    int multiplicity=1,pairs=0; double separation=INFINITY;
    if (step.common_action_gates_pass)
      std::tie(multiplicity,pairs,separation)=fibre_metrics(step.earlier);
    if (!step.common_action_gates_pass || common_residual(step)>action_gate
        || !step.graph_connected || !step.graph_local || multiplicity>2
        || (pairs>0&&separation<0.90)
        || !metadata_equal(initial,step.earlier)) result.reverse=false;
    else state=step.earlier;
  }
  if (result.reverse)
    result.recovery=ftd::eft::connected_moore_block_state_max_difference(
        initial,state);
  result.reverse=result.reverse&&result.recovery<=1e-8;
  result.bounded=result.forward&&result.reverse&&result.metadata
      &&result.max_common<=action_gate&&result.max_drift<=1e-8
      &&result.max_displacement<=1e-8&&result.max_momentum<=1e-8
      &&result.max_shape<=1e-2&&result.max_strain<=3e-2
      &&result.max_multiplicity<=2&&result.min_separation>=0.90;
  return result;
}

double covariance(const Arm& a,const Arm& b) {
  if (a.ticks.size()!=b.ticks.size()) return INFINITY;
  double result=0.0;
  for (std::size_t i=0;i<a.ticks.size();++i) {
    const auto& x=a.ticks[i]; const auto& y=b.ticks[i];
    result=std::max({result,max_component(y.displacement-cycle(x.displacement)),
        max_component(y.momentum-cycle(x.momentum)),
        std::abs(y.state_distance-x.state_distance),
        std::abs(y.internal_distance-x.internal_distance),
        std::abs(y.shape-x.shape),std::abs(y.strain-x.strain),
        std::abs(y.separation-x.separation),relative(y.total,x.total),
        std::abs(y.q[0]-x.q[0]),std::abs(y.q[1]-x.q[1]),
        std::abs(y.q[2]-x.q[2]),
        std::abs(y.interface_coordinate-x.interface_coordinate)});
  }
  return result;
}

bool spectra_match(const Arm& a,const Arm& b) {
  if (a.spectra.size()!=4||b.spectra.size()!=4) return false;
  for (std::size_t i=0;i<4;++i) {
    if (a.spectra[i].observable!=b.spectra[i].observable
        ||a.spectra[i].nonzero!=b.spectra[i].nonzero) return false;
    std::set<int> x(a.spectra[i].bins.begin(),a.spectra[i].bins.end());
    std::set<int> y(b.spectra[i].bins.begin(),b.spectra[i].bins.end());
    if (x!=y) return false;
  }
  return true;
}

void evaluate(Summary& s) {
  s.coverage=s.arms.size()==2;
  s.execution=s.parent&&s.normalization&&s.coverage
      &&std::all_of(s.arms.begin(),s.arms.end(),[](const Arm& a) {
          return a.initialization&&a.forward&&a.reverse&&a.metadata;
        });
  for (const auto& a:s.arms) {
    s.worst_common=std::max(s.worst_common,a.max_common);
    s.worst_drift=std::max(s.worst_drift,a.max_drift);
    if (std::isfinite(a.recovery))s.worst_recovery=std::max(s.worst_recovery,a.recovery);
  }
  s.bounded=s.execution&&std::all_of(s.arms.begin(),s.arms.end(),
      [](const Arm& a){return a.bounded;});
  if (s.coverage) {
    s.covariance_residual=covariance(s.arms[0],s.arms[1]);
    s.covariance=s.covariance_residual<=1e-8;
    if (s.arms[0].period>0&&s.arms[0].period==s.arms[1].period) {
      s.recurrence=true;s.common_period=s.arms[0].period;
    }
    s.spectral=spectra_match(s.arms[0],s.arms[1]);
    for (const auto& a:s.arms)for(const auto& spectrum:a.spectra)
      if(spectrum.nonzero&&spectrum.concentration<0.90)s.spectral=false;
  }
  if(!s.parent||!s.normalization||!s.coverage||!s.covariance)
    s.verdict="CONNECTED_BLOCK_DYNAMICAL_REST_EXECUTION_INVALID";
  else if(!s.bounded)s.verdict="CONNECTED_BLOCK_DYNAMICAL_REST_CLOSED_NEGATIVE";
  else if(s.recurrence)s.verdict="CONNECTED_BLOCK_PERIODIC_DYNAMICAL_REST_CONSTRUCTIVE";
  else if(s.spectral)s.verdict="CONNECTED_BLOCK_BOUNDED_SPECTRAL_DYNAMICAL_REST_CONSTRUCTIVE";
  else s.verdict="CONNECTED_BLOCK_BOUNDED_IRREGULAR_REST_OPEN";
}

void number(std::ostream& out,double value){if(std::isfinite(value))out<<value;else out<<"null";}

void write_records(const Summary& s) {
  const auto dir=std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0627";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0627_connected_block_dynamical_rest_v1.json");
  json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0627\",\n"
      <<"  \"protocol_sha256\": \""<<protocol_sha256<<"\",\n"
      <<"  \"parent_result_sha256\": \""<<parent_sha256<<"\",\n"
      <<"  \"verdict\": \""<<s.verdict<<"\",\n"
      <<"  \"production_changed\": false,\n"
      <<"  \"coverage_pass\": "<<s.coverage<<",\n"
      <<"  \"execution_pass\": "<<s.execution<<",\n"
      <<"  \"bounded_pass\": "<<s.bounded<<",\n"
      <<"  \"recurrence_pass\": "<<s.recurrence<<",\n"
      <<"  \"spectral_concentration_pass\": "<<s.spectral<<",\n"
      <<"  \"covariance_pass\": "<<s.covariance<<",\n"
      <<"  \"common_period\": "<<s.common_period<<",\n"
      <<"  \"covariance_residual\": ";number(json,s.covariance_residual);
  json<<",\n  \"worst_common_residual\": "<<s.worst_common
      <<",\n  \"worst_energy_drift\": "<<s.worst_drift
      <<",\n  \"worst_recovery\": "<<s.worst_recovery<<"\n}\n";

  std::ofstream arms(dir/"ftd_0627_connected_block_dynamical_rest_arms_v1.csv");
  arms<<"ftd_id,label,orientation,phase_axis,init,forward,reverse,exact,metadata,bounded,failure_tick,period,max_multiplicity,min_separation,max_state_distance,max_internal_distance,max_shape,max_strain,max_displacement,max_momentum,max_common,max_drift,recovery\n";
  for(const auto& a:s.arms)arms<<std::setprecision(17)<<"FTD-0627,"<<a.label<<','
      <<a.orientation<<','<<a.phase_axis<<','<<a.initialization<<','<<a.forward
      <<','<<a.reverse<<','<<a.exact<<','<<a.metadata<<','<<a.bounded<<','
      <<a.failure_tick<<','<<a.period<<','<<a.max_multiplicity<<','
      <<a.min_separation<<','<<a.max_state_distance<<','<<a.max_internal_distance
      <<','<<a.max_shape<<','<<a.max_strain<<','<<a.max_displacement<<','
      <<a.max_momentum<<','<<a.max_common<<','<<a.max_drift<<','<<a.recovery<<'\n';

  std::ofstream ticks(dir/"ftd_0627_connected_block_dynamical_rest_ticks_v1.csv");
  ticks<<"ftd_id,label,tick,multiplicity,shared_pairs,separation,state_distance,internal_distance,dx,dy,dz,px,py,pz,shape,strain,kinetic,binding,field,total,drift,q1,q2,q3,interface,common\n";
  for(const auto& a:s.arms)for(const auto& t:a.ticks)
    ticks<<std::setprecision(17)<<"FTD-0627,"<<a.label<<','<<t.index<<','
        <<t.multiplicity<<','<<t.shared_pairs<<','<<t.separation<<','
        <<t.state_distance<<','<<t.internal_distance<<','<<t.displacement.x<<','
        <<t.displacement.y<<','<<t.displacement.z<<','<<t.momentum.x<<','
        <<t.momentum.y<<','<<t.momentum.z<<','<<t.shape<<','<<t.strain<<','
        <<t.kinetic<<','<<t.binding<<','<<t.field<<','<<t.total<<','<<t.drift<<','
        <<t.q[0]<<','<<t.q[1]<<','<<t.q[2]<<','<<t.interface_coordinate<<','
        <<t.common<<'\n';

  std::ofstream spectra(dir/"ftd_0627_connected_block_dynamical_rest_spectra_v1.csv");
  spectra<<"ftd_id,label,observable,nonzero,total_power,concentration,bins,powers\n";
  for(const auto& a:s.arms)for(const auto& p:a.spectra){
    spectra<<std::setprecision(17)<<"FTD-0627,"<<a.label<<','<<p.observable<<','
        <<p.nonzero<<','<<p.total_power<<','<<p.concentration<<',';
    for(int i=0;i<8;++i){if(i)spectra<<';';spectra<<p.bins[i];}
    spectra<<',';
    for(int i=0;i<8;++i){if(i)spectra<<';';spectra<<p.powers[i];}
    spectra<<'\n';
  }
}

}  // namespace

int main(){
  std::cout<<std::setprecision(17);
  Summary s;s.parent=parent_fingerprint();
  const auto normalization=ftd::eft::measure_face_flux_normalization();
  s.normalization=normalization.valid;s.beta=normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.gate_tolerance=action_gate;options.solve_tolerance=2e-11;
  options.max_iterations=48;options.allow_shared_anchor_chart=true;
  if(s.parent&&s.normalization){
    auto x=std::async(std::launch::async,[&](){return run_arm("rest_x",0,0,options,s.beta);});
    auto y=std::async(std::launch::async,[&](){return run_arm("rest_y",1,1,options,s.beta);});
    s.arms.push_back(x.get());std::cout<<"completed rest_x"<<std::endl;
    s.arms.push_back(y.get());std::cout<<"completed rest_y"<<std::endl;
  }
  evaluate(s);write_records(s);
  std::cout<<"protocol_sha256="<<protocol_sha256<<'\n'<<"verdict="<<s.verdict
      <<'\n'<<"execution="<<s.execution<<" bounded="<<s.bounded
      <<" recurrence="<<s.recurrence<<" spectral="<<s.spectral
      <<" period="<<s.common_period<<" covariance="<<s.covariance_residual<<'\n';
  for(const auto& a:s.arms){
    std::cout<<a.label<<" forward="<<a.forward<<" reverse="<<a.reverse
        <<" bounded="<<a.bounded<<" period="<<a.period
        <<" multiplicity="<<a.max_multiplicity<<" min_sep="<<a.min_separation
        <<" state="<<a.max_state_distance<<" internal="<<a.max_internal_distance
        <<" shape="<<a.max_shape<<" strain="<<a.max_strain
        <<" displacement="<<a.max_displacement<<" momentum="<<a.max_momentum
        <<" common="<<a.max_common<<" drift="<<a.max_drift
        <<" recovery="<<a.recovery<<'\n';
    for(const auto& p:a.spectra)
      std::cout<<"  "<<p.observable<<" C8="<<p.concentration
          <<" k1="<<p.bins[0]<<" k2="<<p.bins[1]<<'\n';
  }
  return s.verdict=="CONNECTED_BLOCK_DYNAMICAL_REST_EXECUTION_INVALID"?1:0;
}
