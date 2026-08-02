/** FTD-0766: aged, signed-pair wake/entrainment discriminator on CUDA. */

#pragma push_macro("main")
#undef main
#define main ftd0764_reference_main
#include "campaign_transported_chart_matter_morphology_cuda.cpp"
#undef main
#pragma pop_macro("main")

#include <map>

namespace {

using namespace ftd;
using namespace ftd::eft;

constexpr char kFtd0766ProtocolSha256[] =
    "B8FF05668DF306D05B6D3F7F4715C38B6C3A78C9205E9C747146C2F3A95AFA7F";
constexpr int kFtd0766Volume=321;
constexpr int kFtd0766Ticks=64;
constexpr double kFtd0766Gate=1e-12;
constexpr std::array<int,3> kFtd0766Ages{{0,64,128}};
constexpr std::array<int,5> kFtd0766Times{{0,16,32,48,64}};
constexpr std::array<double,7> kFtd0766Boosts{{
    0.0,-0.030,-0.015,-0.0075,0.0075,0.015,0.030}};

bool wake_time(int value) {
  return std::find(kFtd0766Times.begin(),kFtd0766Times.end(),value)
      !=kFtd0766Times.end();
}

struct WakeCheckpoint {
  int tau=0;
  bool valid=false;
  FractionalCheckpoint field;
  TransportedChartMorphologyObservation morphology;
  CudaTransportedChartMorphologyTelemetry telemetry;
  Vec3 matter_momentum{},local_momentum{},spline_momentum{};
  Vec3 core_center{},residual_centroid{};
  double local_defect=0.0;
  double spline_defect=0.0;
  double common_residual=0.0;
  double energy_residual=0.0;
  double energy_drift=0.0;
  double speed_excess=0.0;
  double sigma_min=INFINITY;
  double condition=0.0;
  double inverse_residual=0.0;
  bool inverse_valid=true;
};

ResidualLongitudinalPartition union_partition(
    const TransportedChartMorphologyObservation& value) {
  return {value.near_longitudinal.trailing
              +value.outer_longitudinal.trailing,
          value.near_longitudinal.neutral
              +value.outer_longitudinal.neutral,
          value.near_longitudinal.leading
              +value.outer_longitudinal.leading};
}

Vec3 residual_centroid(
    const TransportedChartMorphologyObservation& value) {
  const double near=value.near_residual_energy;
  const double outer=value.outer_residual_energy;
  if(!(near+outer>0.0)) return value.center;
  return value.center+(value.near_residual_first_moment*near
      +value.outer_residual_first_moment*outer)*(1.0/(near+outer));
}

WakeCheckpoint observe_wake_checkpoint(
    int tau,const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action,
    const TransportedChartMorphologyOptions& morphology) {
  WakeCheckpoint result;
  result.tau=tau;
  result.field=observe_fractional_checkpoint(state,action);
  result.morphology=observe_transported_chart_morphology_cuda(
      state,action,morphology,&result.telemetry);
  result.matter_momentum=matter_momentum(state);
  result.core_center=result.morphology.center;
  result.residual_centroid=residual_centroid(result.morphology);
  result.valid=result.field.valid&&result.morphology.valid
      &&result.morphology.longitudinal_partition_enabled
      &&result.morphology.longitudinal_partition_residual<=kFtd0766Gate
      &&result.telemetry.valid&&result.telemetry.complete_field_downloads==0;
  return result;
}

struct WakeArm {
  double boost=0.0;
  bool initialized=false;
  bool executed=false;
  bool valid=false;
  Vec3 aligned_direction{};
  Vec3 initial_local_total{},initial_spline_total{};
  double initial_energy=INFINITY;
  double maximum_common_residual=0.0;
  double maximum_energy_residual=0.0;
  double maximum_speed_excess=0.0;
  std::vector<WakeCheckpoint> checkpoints;
};

WakeArm run_wake_arm(const ConnectedMooreBlockState& aged,double boost,
                     const ForensicDirection& direction,
                     const ConnectedMooreBlockOptions& action,double beta) {
  WakeArm arm;
  arm.boost=boost;
  const Vec3 base_direction=direction_unit(direction);
  arm.aligned_direction=boost<0.0?base_direction*(-1.0):base_direction;
  auto initial=aged;
  if(boost!=0.0) for(auto& point:initial.constituents)
    point.momentum+=base_direction*boost;
  const auto core=observe_support_invariant_matter(initial,action);
  arm.initialized=core.valid&&core.member
      &&core.graph_margin>=1e-6&&core.energy_margin>=1e-6;
  if(!arm.initialized) return arm;
  auto morphology=morphology_options("face",action);
  morphology.longitudinal_direction=arm.aligned_direction;
  morphology.longitudinal_dead_band=0.5;
  arm.checkpoints.push_back(observe_wake_checkpoint(
      0,initial,action,morphology));
  MorphologyCudaStepper stepper(std::move(initial),action,beta);
  if(!stepper.valid()) return arm;
  bool execution=true;
  for(int tau=1;tau<=kFtd0766Ticks;++tau) {
    const bool checkpoint=wake_time(tau);
    const auto step=stepper.advance(checkpoint);
    arm.maximum_common_residual=std::max(
        arm.maximum_common_residual,step.common_residual);
    arm.maximum_energy_residual=std::max(
        arm.maximum_energy_residual,step.energy_residual);
    arm.maximum_speed_excess=std::max(
        arm.maximum_speed_excess,step.speed_excess);
    execution=execution&&step.valid&&step.common
        &&step.common_residual<=kFtd0766Gate
        &&step.energy_residual<=kFtd0766Gate
        &&step.speed_excess<=kFtd0766Gate;
    if(tau==1&&step.valid) {
      const auto initial_matter=arm.checkpoints.front().matter_momentum;
      arm.initial_local_total=initial_matter+step.local_before;
      arm.initial_spline_total=initial_matter+step.spline_before;
      arm.initial_energy=step.energy_before;
      arm.checkpoints.front().local_momentum=step.local_before;
      arm.checkpoints.front().spline_momentum=step.spline_before;
    }
    if(!step.valid) break;
    if(checkpoint) {
      auto value=observe_wake_checkpoint(
          tau,stepper.state(),action,morphology);
      value.local_momentum=step.local_after;
      value.spline_momentum=step.spline_after;
      value.local_defect=(value.matter_momentum+value.local_momentum
          -arm.initial_local_total).mag();
      value.spline_defect=(value.matter_momentum+value.spline_momentum
          -arm.initial_spline_total).mag();
      value.common_residual=step.common_residual;
      value.energy_residual=step.energy_residual;
      value.energy_drift=std::abs(step.energy_after-arm.initial_energy);
      value.speed_excess=step.speed_excess;
      value.sigma_min=step.sigma_min;
      value.condition=step.condition;
      value.inverse_valid=step.inverse_valid;
      value.inverse_residual=step.inverse_residual;
      value.valid=value.valid&&step.regularity_measured
          &&step.sigma_min>=1e-3&&step.condition<=1e4
          &&step.inverse_valid&&step.inverse_residual<=kFtd0766Gate
          &&step.common_residual<=kFtd0766Gate
          &&step.energy_residual<=kFtd0766Gate
          &&step.speed_excess<=kFtd0766Gate;
      arm.checkpoints.push_back(std::move(value));
    }
  }
  arm.executed=execution&&arm.checkpoints.size()==kFtd0766Times.size();
  arm.valid=arm.initialized&&arm.executed
      &&std::all_of(arm.checkpoints.begin(),arm.checkpoints.end(),
          [](const auto& value){return value.valid;});
  return arm;
}

struct WakePairMetrics {
  double magnitude=0.0;
  bool valid=false;
  double maximum_core_mirror_residual=INFINITY;
  double maximum_field_mirror_residual=INFINITY;
  double final_pair_asymmetry=INFINITY;
  double final_pair_entrainment=INFINITY;
};

double normalized_difference(double lhs,double rhs) {
  return std::abs(lhs-rhs)/std::max({1e-300,std::abs(lhs),std::abs(rhs)});
}

WakePairMetrics compare_wake_pair(const WakeArm& minus,const WakeArm& plus,
                                  const WakeArm& rest,double magnitude) {
  WakePairMetrics result;
  result.magnitude=magnitude;
  if(!minus.valid||!plus.valid||!rest.valid
      ||minus.checkpoints.size()!=plus.checkpoints.size()
      ||plus.checkpoints.size()!=rest.checkpoints.size()) return result;
  result.maximum_core_mirror_residual=0.0;
  result.maximum_field_mirror_residual=0.0;
  const Vec3 common_center=plus.checkpoints.front().core_center;
  for(std::size_t i=0;i<plus.checkpoints.size();++i) {
    const Vec3 plus_displacement=
        plus.checkpoints[i].core_center-common_center;
    const Vec3 minus_displacement=
        minus.checkpoints[i].core_center-common_center;
    result.maximum_core_mirror_residual=std::max(
        result.maximum_core_mirror_residual,
        (plus_displacement+minus_displacement).mag());
    const auto a=union_partition(plus.checkpoints[i].morphology);
    const auto b=union_partition(minus.checkpoints[i].morphology);
    const double plus_energy=a.total();
    const double minus_energy=b.total();
    result.maximum_field_mirror_residual=std::max({
        result.maximum_field_mirror_residual,
        normalized_difference(a.trailing,b.trailing),
        normalized_difference(a.leading,b.leading),
        normalized_difference(plus_energy,minus_energy)});
  }
  const auto& plus0=plus.checkpoints.front();
  const auto& minus0=minus.checkpoints.front();
  const auto& rest0=rest.checkpoints.front();
  const auto& plus1=plus.checkpoints.back();
  const auto& minus1=minus.checkpoints.back();
  const auto& rest1=rest.checkpoints.back();
  const double plus_core=(plus1.core_center-plus0.core_center)
      .dot(plus.aligned_direction);
  const double minus_core=(minus1.core_center-minus0.core_center)
      .dot(minus.aligned_direction);
  const Vec3 rest_residual_motion=
      rest1.residual_centroid-rest0.residual_centroid;
  const double plus_residual=(plus1.residual_centroid
      -plus0.residual_centroid-rest_residual_motion)
      .dot(plus.aligned_direction);
  const double minus_residual=(minus1.residual_centroid
      -minus0.residual_centroid-rest_residual_motion)
      .dot(minus.aligned_direction);
  const auto plus_partition=union_partition(plus1.morphology);
  const auto minus_partition=union_partition(minus1.morphology);
  result.final_pair_asymmetry=0.5*(plus_partition.asymmetry()
      +minus_partition.asymmetry());
  result.final_pair_entrainment=0.5*(plus_residual/plus_core
      +minus_residual/minus_core);
  result.valid=result.maximum_core_mirror_residual<=1e-10
      &&result.maximum_field_mirror_residual<=1e-10
      &&plus_core>0.0&&minus_core>0.0
      &&std::isfinite(result.final_pair_asymmetry)
      &&std::isfinite(result.final_pair_entrainment);
  return result;
}

struct WakeAgeResult {
  int age=0;
  bool aging_valid=false;
  bool valid=false;
  std::vector<WakeArm> arms;
  std::vector<WakePairMetrics> pairs;
};

const WakeArm* find_arm(const WakeAgeResult& value,double boost) {
  const auto found=std::find_if(value.arms.begin(),value.arms.end(),
      [boost](const auto& arm){return arm.boost==boost;});
  return found==value.arms.end()?nullptr:&*found;
}

WakeAgeResult run_wake_age(const ConnectedMooreBlockState& state,int age,
                           bool aging_valid,const ForensicDirection& direction,
                           const ConnectedMooreBlockOptions& action,double beta) {
  WakeAgeResult result;
  result.age=age;
  result.aging_valid=aging_valid;
  for(const double boost:kFtd0766Boosts) {
    std::cout<<"FTD-0766 age="<<age<<" boost="<<boost<<" start\n"
             <<std::flush;
    result.arms.push_back(run_wake_arm(
        state,boost,direction,action,beta));
    std::cout<<std::boolalpha<<"FTD-0766 age="<<age<<" boost="<<boost
             <<" valid="<<result.arms.back().valid<<"\n"<<std::flush;
  }
  const auto* rest=find_arm(result,0.0);
  for(const double magnitude:{0.0075,0.015,0.030}) {
    const auto* minus=find_arm(result,-magnitude);
    const auto* plus=find_arm(result,magnitude);
    result.pairs.push_back(minus&&plus&&rest
        ?compare_wake_pair(*minus,*plus,*rest,magnitude)
        :WakePairMetrics{});
  }
  bool rest_static=false;
  if(rest&&rest->valid)
    rest_static=(rest->checkpoints.back().core_center
        -rest->checkpoints.front().core_center).mag()<=1e-12;
  result.valid=result.aging_valid&&rest_static
      &&std::all_of(result.arms.begin(),result.arms.end(),
          [](const auto& arm){return arm.valid;})
      &&std::all_of(result.pairs.begin(),result.pairs.end(),
          [](const auto& pair){return pair.valid;});
  return result;
}

struct AgedWakeCampaign {
  bool parent_valid=false;
  bool execution_valid=false;
  std::vector<WakeAgeResult> ages;
  bool aligned_trailing_excess=false;
  bool amplitude_ordered=false;
  bool age_stable=false;
  std::string wake_verdict="AGED_WAKE_EXECUTION_INVALID";
  std::string entrainment_verdict="ENTRAINMENT_EXECUTION_INVALID";
};

const WakePairMetrics* find_pair(const WakeAgeResult& age,double magnitude) {
  const auto found=std::find_if(age.pairs.begin(),age.pairs.end(),
      [magnitude](const auto& pair){return pair.magnitude==magnitude;});
  return found==age.pairs.end()?nullptr:&*found;
}

AgedWakeCampaign run_aged_wake_campaign() {
  AgedWakeCampaign result;
  ForensicDirection direction;
  if(!select_direction("face",direction)) return result;
  const auto normalization=measure_face_flux_normalization();
  if(!normalization.valid) return result;
  auto action=forensic_options();
  auto parent=build_parent(kFtd0766Volume,direction,action,
      normalization.mapped_field_work_coefficient);
  result.parent_valid=parent.valid;
  if(!parent.valid) return result;
  auto aged_state=std::move(parent.state);
  int current_age=0;
  bool aging_valid=true;
  for(const int age:kFtd0766Ages) {
    if(age>current_age) {
      MorphologyCudaStepper aging(std::move(aged_state),action,
          normalization.mapped_field_work_coefficient);
      if(!aging.valid()) return result;
      for(int tick=current_age+1;tick<=age;++tick) {
        const auto step=aging.advance(false);
        aging_valid=aging_valid&&step.valid&&step.common
            &&step.common_residual<=kFtd0766Gate
            &&step.energy_residual<=kFtd0766Gate
            &&step.speed_excess<=kFtd0766Gate;
        if(!step.valid) break;
      }
      aged_state=aging.release_state();
      current_age=age;
    }
    result.ages.push_back(run_wake_age(aged_state,age,aging_valid,
        direction,action,normalization.mapped_field_work_coefficient));
  }
  result.execution_valid=result.parent_valid
      &&std::all_of(result.ages.begin(),result.ages.end(),
          [](const auto& age){return age.valid;});
  if(!result.execution_valid) return result;
  result.aligned_trailing_excess=true;
  result.amplitude_ordered=true;
  for(const auto& age:result.ages) {
    std::array<double,3> values{};
    for(std::size_t i=0;i<values.size();++i) {
      const auto* pair=find_pair(age,std::array<double,3>{{
          0.0075,0.015,0.030}}[i]);
      values[i]=pair?pair->final_pair_asymmetry:-INFINITY;
      result.aligned_trailing_excess=
          result.aligned_trailing_excess&&values[i]>=1e-5;
    }
    result.amplitude_ordered=result.amplitude_ordered
        &&0.0<values[0]&&values[0]<values[1]&&values[1]<values[2];
  }
  result.age_stable=true;
  for(const double magnitude:{0.0075,0.015,0.030}) {
    const auto* age64=find_pair(result.ages[1],magnitude);
    const auto* age128=find_pair(result.ages[2],magnitude);
    const double a=age64->final_pair_asymmetry;
    const double b=age128->final_pair_asymmetry;
    result.age_stable=result.age_stable
        &&std::abs(a-b)/std::max(std::abs(a),std::abs(b))<=0.25;
  }
  result.wake_verdict=result.aligned_trailing_excess
      &&result.amplitude_ordered&&result.age_stable
      ?"DYNAMICAL_WAKE_CANDIDATE":"WAKE_CREATION_NOT_ESTABLISHED";
  std::array<double,3> entrainment{};
  for(std::size_t i=0;i<result.ages.size();++i)
    entrainment[i]=find_pair(result.ages[i],0.015)
        ->final_pair_entrainment;
  if(entrainment[2]-entrainment[0]>=0.10&&entrainment[2]>0.50)
    result.entrainment_verdict="AGE_IMPROVING_ENTRAINMENT";
  else if(std::all_of(entrainment.begin(),entrainment.end(),
      [](double value){return value<0.20;}))
    result.entrainment_verdict="PERSISTENT_UNDER_ENTRAINMENT";
  else result.entrainment_verdict="MIXED_ENTRAINMENT";
  return result;
}

void write_partition(std::ostream& out,
                     const ResidualLongitudinalPartition& value) {
  out<<"{\"trailing\": "<<json_number(value.trailing)
     <<", \"neutral\": "<<json_number(value.neutral)
     <<", \"leading\": "<<json_number(value.leading)
     <<", \"asymmetry\": "<<json_number(value.asymmetry())<<'}';
}

void write_wake_checkpoint(std::ostream& out,const WakeCheckpoint& value) {
  out<<"{\"tau\": "<<value.tau<<", \"valid\": "<<value.valid
     <<", \"core_center\": "; write_vec(out,value.core_center);
  out<<", \"residual_centroid\": "; write_vec(out,value.residual_centroid);
  out<<", \"matter_momentum\": "; write_vec(out,value.matter_momentum);
  out<<", \"local_momentum\": "; write_vec(out,value.local_momentum);
  out<<", \"spline_momentum\": "; write_vec(out,value.spline_momentum);
  out<<", \"local_defect\": "<<json_number(value.local_defect)
     <<", \"spline_defect\": "<<json_number(value.spline_defect)
     <<", \"common_residual\": "<<json_number(value.common_residual)
     <<", \"energy_residual\": "<<json_number(value.energy_residual)
     <<", \"energy_drift\": "<<json_number(value.energy_drift)
     <<", \"speed_excess\": "<<json_number(value.speed_excess)
     <<", \"sigma_min\": "<<json_number(value.sigma_min)
     <<", \"condition\": "<<json_number(value.condition)
     <<", \"inverse_valid\": "<<value.inverse_valid
     <<", \"inverse_residual\": "<<json_number(value.inverse_residual)
     <<", \"fractional_observer_valid\": "<<value.field.valid
     <<", \"boundary_ledger_valid\": "
     <<value.field.boundary_ledger_valid
     <<", \"ladder_valid\": "<<value.field.ladder_valid
     <<", \"morphology_reconstruction_residual\": "
     <<json_number(std::max(std::abs(
          value.morphology.energy_reconstruction_residual),
          value.morphology.maximum_mode_reconstruction_residual))
     <<", \"partition_residual\": "
     <<json_number(value.morphology.longitudinal_partition_residual)
     <<", \"near\": "; write_partition(out,value.morphology.near_longitudinal);
  out<<", \"outer\": "; write_partition(out,value.morphology.outer_longitudinal);
  out<<", \"union\": "; write_partition(out,union_partition(value.morphology));
  out<<'}';
}

std::filesystem::path ftd0766_results_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results"/"ftd_0766";
}

void write_aged_wake_result(const AgedWakeCampaign& value) {
  const auto directory=ftd0766_results_directory();
  std::filesystem::create_directories(directory);
  std::ofstream out(directory/"ftd_0766_aged_wake_entrainment_v1.json");
  out<<std::boolalpha<<std::setprecision(17)
     <<"{\n  \"ftd_id\": \"FTD-0766\",\n"
     <<"  \"protocol_sha256\": \""<<kFtd0766ProtocolSha256<<"\",\n"
     <<"  \"volume\": "<<kFtd0766Volume<<",\n"
     <<"  \"direction\": \"face (0,0,1)\",\n"
     <<"  \"parent_valid\": "<<value.parent_valid<<",\n"
     <<"  \"execution_valid\": "<<value.execution_valid<<",\n"
     <<"  \"aligned_trailing_excess\": "
     <<value.aligned_trailing_excess<<",\n"
     <<"  \"amplitude_ordered\": "<<value.amplitude_ordered<<",\n"
     <<"  \"age_stable\": "<<value.age_stable<<",\n"
     <<"  \"wake_verdict\": \""<<value.wake_verdict<<"\",\n"
     <<"  \"entrainment_verdict\": \""
     <<value.entrainment_verdict<<"\",\n  \"ages\": [";
  for(std::size_t ai=0;ai<value.ages.size();++ai) {
    if(ai) out<<',';
    const auto& age=value.ages[ai];
    out<<"\n    {\"age\": "<<age.age
       <<", \"aging_valid\": "<<age.aging_valid
       <<", \"valid\": "<<age.valid<<", \"pairs\": [";
    for(std::size_t pi=0;pi<age.pairs.size();++pi) {
      if(pi) out<<',';
      const auto& pair=age.pairs[pi];
      out<<"{\"magnitude\": "<<json_number(pair.magnitude)
         <<", \"valid\": "<<pair.valid
         <<", \"maximum_core_mirror_residual\": "
         <<json_number(pair.maximum_core_mirror_residual)
         <<", \"maximum_field_mirror_residual\": "
         <<json_number(pair.maximum_field_mirror_residual)
         <<", \"final_pair_asymmetry\": "
         <<json_number(pair.final_pair_asymmetry)
         <<", \"final_pair_entrainment\": "
         <<json_number(pair.final_pair_entrainment)<<'}';
    }
    out<<"], \"arms\": [";
    for(std::size_t armi=0;armi<age.arms.size();++armi) {
      if(armi) out<<',';
      const auto& arm=age.arms[armi];
      out<<"{\"boost\": "<<json_number(arm.boost)
         <<", \"initialized\": "<<arm.initialized
         <<", \"executed\": "<<arm.executed
         <<", \"valid\": "<<arm.valid
         <<", \"aligned_direction\": "; write_vec(out,arm.aligned_direction);
      out<<", \"maximum_common_residual\": "
         <<json_number(arm.maximum_common_residual)
         <<", \"maximum_energy_residual\": "
         <<json_number(arm.maximum_energy_residual)
         <<", \"maximum_speed_excess\": "
         <<json_number(arm.maximum_speed_excess)
         <<", \"checkpoints\": [";
      for(std::size_t ci=0;ci<arm.checkpoints.size();++ci) {
        if(ci) out<<',';
        write_wake_checkpoint(out,arm.checkpoints[ci]);
      }
      out<<"]}";
    }
    out<<"]}";
  }
  out<<"\n  ],\n  \"production_changed\": false,\n"
     <<"  \"dynamics_changed\": false,\n"
     <<"  \"new_primitive_added\": false\n}\n";
}

}  // namespace

int main(int argc,char** argv) {
  if(argc!=2||std::string(argv[1])!="--run") {
    std::cout<<"FTD-0766 runner: --run\n";
    return argc==1?0:2;
  }
  if(std::string(kFtd0766ProtocolSha256)=="UNLOCKED") return 3;
  const auto result=run_aged_wake_campaign();
  write_aged_wake_result(result);
  std::cout<<std::boolalpha
           <<"FTD-0766 execution="<<result.execution_valid
           <<" wake="<<result.wake_verdict
           <<" entrainment="<<result.entrainment_verdict<<'\n';
  return result.execution_valid?0:1;
}
