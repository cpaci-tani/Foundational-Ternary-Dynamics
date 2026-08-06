// FTD-0641: independent source-free face/edge modes on the dressed background.

#define FTD_0639_EMBEDDED
#include "test_connected_block_analytic_dynamical_rest.cpp"
#undef FTD_0639_EMBEDDED

#include <map>

namespace {

constexpr char field_protocol_sha256[] =
    "6EB4C1035C29187F22D2FC8BD7A152EF47F148165204B7614504A70979EDB9C8";
constexpr char field_parent_result_sha256[] =
    "AB43D342CFE48BEF452955E56B1EDC34F9EE51911F7D899932E7E542877E6B9A";
constexpr int field_ticks = 256;
constexpr double field_full = 1e-7,field_half = 5e-8;

struct FieldSpec {
  std::string label,kind,family;
  std::array<int,3> wave{};
  int permutation=0,polarization=0,sign=1;
  double target=0;
};
struct FieldTick { int tick=0;double q=0,divergence=0,full_drift=0,background_drift=0,recurrence=0; };
struct FieldArm {
  FieldSpec spec;bool initialized=false,complete=false,reversed=false,bounded=false;
  double sigma=0,predicted=INFINITY,phase=INFINITY,phase_error=INFINITY,
      initial_max=INFINITY,max_divergence=0,max_full_drift=0,max_background_drift=0,
      max_recurrence=INFINITY,recovery=INFINITY;
  std::vector<FieldTick> ticks;
};
struct FieldSummary {
  bool parent=false,coverage=false,execution=false,bounded=false,frequency=false,
      recurrence=false,amplitude=false,sign=false,polarization=false,cubic=false,
      monotonic=false;
  double amplitude_residual=INFINITY,sign_residual=INFINITY,
      polarization_residual=INFINITY,cubic_residual=INFINITY,worst_divergence=0,
      worst_energy=0,worst_recurrence=0,worst_recovery=0;
  std::string verdict="CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_EXECUTION_INVALID";
  std::vector<FieldArm> arms;
};

bool field_parent_fingerprint() {
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/
      "results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_v1.json";
  std::ifstream input(path,std::ios::binary);const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  return bytes.find("CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_CONSTRUCTIVE")!=std::string::npos
      && bytes.find("203168B41C4B611695A7DF0AA9D311EF2A23AED10CA69900B46C834BA1DD7BDC")!=std::string::npos;
}
void add_field(ftd::eft::MatchedFaceFlux& target,const ftd::eft::MatchedFaceFlux& delta,double scale=1) { for(std::size_t i=0;i<target.x.size();++i){target.x[i]+=scale*delta.x[i];target.y[i]+=scale*delta.y[i];target.z[i]+=scale*delta.z[i];} }
ftd::eft::MatchedFaceFlux difference(const ftd::eft::MatchedFaceFlux&a,const ftd::eft::MatchedFaceFlux&b){ftd::eft::MatchedFaceFlux d(a.L);for(std::size_t i=0;i<d.x.size();++i){d.x[i]=a.x[i]-b.x[i];d.y[i]=a.y[i]-b.y[i];d.z[i]=a.z[i]-b.z[i];}return d;}
void advance_field(ftd::eft::MatchedFaceFlux& e,ftd::eft::MatchedEdgeField& b){const auto ce=ftd::eft::matched_curl_adjoint(e);for(std::size_t i=0;i<b.x.size();++i){b.x[i]-=ftd::C_SPEED*ce.x[i];b.y[i]-=ftd::C_SPEED*ce.y[i];b.z[i]-=ftd::C_SPEED*ce.z[i];}const auto cb=ftd::eft::matched_curl(b);for(std::size_t i=0;i<e.x.size();++i){e.x[i]+=ftd::C_SPEED*cb.x[i];e.y[i]+=ftd::C_SPEED*cb.y[i];e.z[i]+=ftd::C_SPEED*cb.z[i];}}
void reverse_field(ftd::eft::MatchedFaceFlux& e,ftd::eft::MatchedEdgeField& b){const auto cb=ftd::eft::matched_curl(b);for(std::size_t i=0;i<e.x.size();++i){e.x[i]-=ftd::C_SPEED*cb.x[i];e.y[i]-=ftd::C_SPEED*cb.y[i];e.z[i]-=ftd::C_SPEED*cb.z[i];}const auto ce=ftd::eft::matched_curl_adjoint(e);for(std::size_t i=0;i<b.x.size();++i){b.x[i]+=ftd::C_SPEED*ce.x[i];b.y[i]+=ftd::C_SPEED*ce.y[i];b.z[i]+=ftd::C_SPEED*ce.z[i];}}
double face_dot(const ftd::eft::MatchedFaceFlux&a,const ftd::eft::MatchedFaceFlux&b){return static_cast<double>(ftd::eft::matched_face_dot(a,b));}
double field_max(const ftd::eft::MatchedFaceFlux&e){double r=0;for(std::size_t i=0;i<e.x.size();++i)r=std::max({r,std::abs(e.x[i]),std::abs(e.y[i]),std::abs(e.z[i])});return r;}

ftd::eft::MatchedFaceFlux mode_shape(const FieldSpec& spec) {
  ftd::eft::MatchedEdgeField potential(L);int accepted=-1;
  for(int axis=0,count_axis=0;axis<3;++axis){ftd::eft::MatchedEdgeField candidate(L);for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z){const int i=candidate.index(x,y,z);const double phase=2*ftd::PI*(spec.wave[0]*x+spec.wave[1]*y+spec.wave[2]*z)/L;const double value=std::cos(phase);if(axis==0)candidate.x[i]=value;if(axis==1)candidate.y[i]=value;if(axis==2)candidate.z[i]=value;}const auto curl=ftd::eft::matched_curl(candidate);if(field_max(curl)>1e-12){if(count_axis==spec.polarization){potential=std::move(candidate);accepted=axis;break;}++count_axis;}}
  auto result=ftd::eft::matched_curl(potential);const double maximum=field_max(result);if(accepted<0||maximum<=0)return ftd::eft::MatchedFaceFlux{};for(std::size_t i=0;i<result.x.size();++i){result.x[i]/=maximum;result.y[i]/=maximum;result.z[i]/=maximum;}return result;
}

double estimate_field_phase(const FieldArm& arm){long double numerator=0,denominator=0;for(int n=1;n<field_ticks-1;++n){const double q=arm.ticks[n].q;numerator+=q*(arm.ticks[n+1].q+arm.ticks[n-1].q);denominator+=2*q*q;}return denominator>0?std::acos(std::clamp(static_cast<double>(numerator/denominator),-1.0,1.0)):INFINITY;}

FieldArm run_field(const FieldSpec& spec,const ftd::eft::ConnectedMooreBlockState& reference) {
  FieldArm arm;arm.spec=spec;const auto unit=mode_shape(spec);if(unit.L!=L)return arm;auto background_e=reference.electric;auto background_b=reference.magnetic_half;auto electric=background_e;auto magnetic=background_b;add_field(electric,unit,spec.sign*spec.target);const auto initial_e=electric;const auto initial_b=magnetic;arm.initial_max=field_max(difference(electric,background_e));arm.sigma=2*std::sqrt(std::pow(std::sin(ftd::PI*spec.wave[0]/L),2)+std::pow(std::sin(ftd::PI*spec.wave[1]/L),2)+std::pow(std::sin(ftd::PI*spec.wave[2]/L),2));arm.predicted=2*std::asin(ftd::C_SPEED*arm.sigma/2);arm.initialized=std::abs(arm.initial_max-spec.target)<=1e-14&&ftd::eft::max_divergence(difference(electric,background_e))<=1e-12;const double energy0=ftd::eft::matched_modified_energy(electric,magnetic,ftd::C_SPEED),background0=ftd::eft::matched_modified_energy(background_e,background_b,ftd::C_SPEED),denominator=face_dot(unit,unit);if(!arm.initialized||!(denominator>0))return arm;
  for(int tick=1;tick<=field_ticks;++tick){advance_field(electric,magnetic);advance_field(background_e,background_b);const auto delta=difference(electric,background_e);FieldTick row;row.tick=tick;row.q=face_dot(delta,unit)/denominator;row.divergence=ftd::eft::max_divergence(delta);row.full_drift=std::abs(ftd::eft::matched_modified_energy(electric,magnetic,ftd::C_SPEED)-energy0);row.background_drift=std::abs(ftd::eft::matched_modified_energy(background_e,background_b,ftd::C_SPEED)-background0);arm.max_divergence=std::max(arm.max_divergence,row.divergence);arm.max_full_drift=std::max(arm.max_full_drift,row.full_drift);arm.max_background_drift=std::max(arm.max_background_drift,row.background_drift);arm.ticks.push_back(row);}arm.complete=arm.ticks.size()==field_ticks;arm.phase=estimate_field_phase(arm);arm.phase_error=std::abs(arm.phase-arm.predicted)/arm.predicted;arm.max_recurrence=0;const double cos_phase=std::cos(arm.predicted),scale=std::max(1e-300,spec.target);for(int n=1;n<field_ticks-1;++n){arm.ticks[n].recurrence=std::abs(arm.ticks[n+1].q+arm.ticks[n-1].q-2*cos_phase*arm.ticks[n].q)/scale;arm.max_recurrence=std::max(arm.max_recurrence,arm.ticks[n].recurrence);}for(int tick=0;tick<field_ticks;++tick)reverse_field(electric,magnetic);arm.recovery=std::max(ftd::eft::matched_face_max_difference(electric,initial_e),ftd::eft::matched_edge_max_difference(magnetic,initial_b));arm.reversed=arm.recovery<=1e-11;arm.bounded=arm.complete&&arm.reversed&&arm.max_divergence<=1e-12&&arm.max_full_drift<=1e-12&&arm.max_background_drift<=1e-12;return arm;
}

const FieldArm* find_field(const FieldSummary&s,const std::string&family,int n,int permutation,int polarization,const std::string&kind){for(const auto&a:s.arms)if(a.spec.family==family&&*std::max_element(a.spec.wave.begin(),a.spec.wave.end())==n&&a.spec.permutation==permutation&&a.spec.polarization==polarization&&a.spec.kind==kind)return &a;return nullptr;}
double relative_field(double a,double b){return std::abs(a-b)/std::max({1e-300,std::abs(a),std::abs(b)});}
void evaluate_field(FieldSummary&s){s.coverage=s.arms.size()==54;s.execution=s.coverage&&std::all_of(s.arms.begin(),s.arms.end(),[](const FieldArm&a){return a.initialized&&a.complete&&a.reversed;});s.bounded=s.execution&&std::all_of(s.arms.begin(),s.arms.end(),[](const FieldArm&a){return a.bounded;});s.frequency=s.execution;s.recurrence=s.execution;for(const auto&a:s.arms){if(a.spec.kind=="primary")s.frequency=s.frequency&&a.phase_error<=1e-8;s.recurrence=s.recurrence&&a.max_recurrence<=1e-8;s.worst_divergence=std::max(s.worst_divergence,a.max_divergence);s.worst_energy=std::max({s.worst_energy,a.max_full_drift,a.max_background_drift});s.worst_recurrence=std::max(s.worst_recurrence,a.max_recurrence);s.worst_recovery=std::max(s.worst_recovery,a.recovery);}s.amplitude=s.sign=s.polarization=s.cubic=s.monotonic=s.execution;s.amplitude_residual=s.sign_residual=s.polarization_residual=s.cubic_residual=0;
  for(const std::string family:{"100","110","111"})for(int pol=0;pol<2;++pol){const int canonical=family=="110"?2:0;const auto*p=find_field(s,family,1,canonical,pol,"primary"),*h=find_field(s,family,1,canonical,pol,"half"),*n=find_field(s,family,1,canonical,pol,"negative");if(!p||!h||!n){s.amplitude=s.sign=false;continue;}const double amp=relative_field(p->phase,h->phase),sgn=relative_field(p->phase,n->phase);s.amplitude_residual=std::max(s.amplitude_residual,amp);s.sign_residual=std::max(s.sign_residual,sgn);double trajectory=0;for(int t=0;t<field_ticks;++t)trajectory=std::max(trajectory,std::abs(p->ticks[t].q+n->ticks[t].q)/field_full);s.sign_residual=std::max(s.sign_residual,trajectory);s.amplitude=s.amplitude&&amp<=1e-8;s.sign=s.sign&&sgn<=1e-8&&trajectory<=1e-8;}
  for(int n=1;n<=3;++n)for(const std::string family:{"100","110","111"}){const int permutations=family=="111"?1:3;for(int p=0;p<permutations;++p){const auto*a=find_field(s,family,n,p,0,"primary"),*b=find_field(s,family,n,p,1,"primary");if(!a||!b){s.polarization=false;continue;}const double r=relative_field(a->phase,b->phase);s.polarization_residual=std::max(s.polarization_residual,r);s.polarization=s.polarization&&r<=1e-10;}for(int pol=0;pol<2;++pol){const int canonical=family=="110"?2:0;const auto*base=find_field(s,family,n,canonical,pol,"primary");if(!base){s.cubic=false;continue;}for(int p=0;p<permutations;++p){const auto*a=find_field(s,family,n,p,pol,"primary");if(!a){s.cubic=false;continue;}const double r=relative_field(base->phase,a->phase);s.cubic_residual=std::max(s.cubic_residual,r);s.cubic=s.cubic&&r<=1e-10;}}}
  for(const std::string family:{"100","110","111"})for(int pol=0;pol<2;++pol){const int p=family=="110"?2:0;double prior=0;for(int n=1;n<=3;++n){const auto*a=find_field(s,family,n,p,pol,"primary");if(!a||!(a->phase>prior))s.monotonic=false;else prior=a->phase;}}
  if(!s.parent||!s.coverage)s.verdict="CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_EXECUTION_INVALID";else if(!s.bounded)s.verdict="CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_CLOSED_NEGATIVE";else if(s.frequency&&s.recurrence&&s.amplitude&&s.sign&&s.polarization&&s.cubic&&s.monotonic)s.verdict="CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_CONSTRUCTIVE";else s.verdict="CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_MIXED";
}

void write_field(const FieldSummary&s){const auto dir=std::filesystem::path(__FILE__).parent_path().parent_path()/"results/ftd_0641";std::filesystem::create_directories(dir);std::ofstream json(dir/"ftd_0641_connected_block_independent_field_modes_v1.json");json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0641\",\n  \"protocol_sha256\": \""<<field_protocol_sha256<<"\",\n  \"parent_result_sha256\": \""<<field_parent_result_sha256<<"\",\n  \"verdict\": \""<<s.verdict<<"\",\n  \"production_changed\": false,\n  \"arm_count\": "<<s.arms.size()<<",\n  \"ticks_each_direction\": "<<field_ticks<<",\n  \"coverage_pass\": "<<s.coverage<<",\n  \"execution_pass\": "<<s.execution<<",\n  \"bounded_pass\": "<<s.bounded<<",\n  \"frequency_pass\": "<<s.frequency<<",\n  \"recurrence_pass\": "<<s.recurrence<<",\n  \"amplitude_pass\": "<<s.amplitude<<",\n  \"sign_pass\": "<<s.sign<<",\n  \"polarization_pass\": "<<s.polarization<<",\n  \"cubic_pass\": "<<s.cubic<<",\n  \"monotonic_pass\": "<<s.monotonic<<",\n  \"amplitude_residual\": "<<s.amplitude_residual<<",\n  \"sign_residual\": "<<s.sign_residual<<",\n  \"polarization_residual\": "<<s.polarization_residual<<",\n  \"cubic_residual\": "<<s.cubic_residual<<",\n  \"worst_divergence\": "<<s.worst_divergence<<",\n  \"worst_energy_drift\": "<<s.worst_energy<<",\n  \"worst_recurrence\": "<<s.worst_recurrence<<",\n  \"worst_recovery\": "<<s.worst_recovery<<"\n}\n";
  std::ofstream arms(dir/"ftd_0641_connected_block_independent_field_modes_arms_v1.csv");arms<<"ftd_id,label,kind,family,nx,ny,nz,permutation,polarization,sign,target,initialized,complete,reversed,bounded,sigma,predicted_phase,phase,phase_error,initial_max,max_divergence,max_full_drift,max_background_drift,max_recurrence,recovery\n";for(const auto&a:s.arms)arms<<std::setprecision(17)<<"FTD-0641,"<<a.spec.label<<','<<a.spec.kind<<','<<a.spec.family<<','<<a.spec.wave[0]<<','<<a.spec.wave[1]<<','<<a.spec.wave[2]<<','<<a.spec.permutation<<','<<a.spec.polarization<<','<<a.spec.sign<<','<<a.spec.target<<','<<a.initialized<<','<<a.complete<<','<<a.reversed<<','<<a.bounded<<','<<a.sigma<<','<<a.predicted<<','<<a.phase<<','<<a.phase_error<<','<<a.initial_max<<','<<a.max_divergence<<','<<a.max_full_drift<<','<<a.max_background_drift<<','<<a.max_recurrence<<','<<a.recovery<<'\n';
  std::ofstream ticks(dir/"ftd_0641_connected_block_independent_field_modes_ticks_v1.csv");ticks<<"ftd_id,label,tick,q,divergence,full_drift,background_drift,recurrence\n";for(const auto&a:s.arms)for(const auto&t:a.ticks)ticks<<std::setprecision(17)<<"FTD-0641,"<<a.spec.label<<','<<t.tick<<','<<t.q<<','<<t.divergence<<','<<t.full_drift<<','<<t.background_drift<<','<<t.recurrence<<'\n';}

std::vector<FieldSpec> field_specs(){std::vector<FieldSpec>s;for(int n=1;n<=3;++n){std::vector<std::tuple<std::string,std::array<int,3>,int>> waves{{"100",{n,0,0},0},{"100",{0,n,0},1},{"100",{0,0,n},2},{"110",{0,n,n},0},{"110",{n,0,n},1},{"110",{n,n,0},2},{"111",{n,n,n},0}};for(const auto&[family,wave,p]:waves)for(int pol=0;pol<2;++pol)s.push_back({family+"_n"+std::to_string(n)+"_p"+std::to_string(p)+"_e"+std::to_string(pol)+"_full","primary",family,wave,p,pol,+1,field_full});}for(const std::string family:{"100","110","111"})for(int pol=0;pol<2;++pol){const std::array<int,3>wave=family=="100"?std::array<int,3>{1,0,0}:(family=="110"?std::array<int,3>{1,1,0}:std::array<int,3>{1,1,1});const int p=family=="110"?2:0;s.push_back({family+"_n1_p"+std::to_string(p)+"_e"+std::to_string(pol)+"_half","half",family,wave,p,pol,+1,field_half});s.push_back({family+"_n1_p"+std::to_string(p)+"_e"+std::to_string(pol)+"_negative","negative",family,wave,p,pol,-1,field_full});}return s;}
}

int main(){FieldSummary summary;summary.parent=field_parent_fingerprint();const auto reference=load_refined_state(0);if(summary.parent&&reference.electric.L==L)for(const auto&spec:field_specs()){summary.arms.push_back(run_field(spec,reference));std::cout<<"completed "<<spec.label<<std::endl;}evaluate_field(summary);write_field(summary);std::cout<<std::setprecision(17)<<"protocol_sha256="<<field_protocol_sha256<<'\n'<<"verdict="<<summary.verdict<<'\n'<<"coverage="<<summary.coverage<<" bounded="<<summary.bounded<<" frequency="<<summary.frequency<<" recurrence="<<summary.recurrence<<" amplitude="<<summary.amplitude<<" sign="<<summary.sign<<" polarization="<<summary.polarization<<" cubic="<<summary.cubic<<" monotonic="<<summary.monotonic<<'\n'<<"worst_divergence="<<summary.worst_divergence<<" energy="<<summary.worst_energy<<" recurrence="<<summary.worst_recurrence<<" recovery="<<summary.worst_recovery<<'\n';for(const auto&a:summary.arms)if(!a.bounded||a.phase_error>1e-8||a.max_recurrence>1e-8)std::cout<<a.spec.label<<" bounded="<<a.bounded<<" phase_error="<<a.phase_error<<" recurrence="<<a.max_recurrence<<" divergence="<<a.max_divergence<<" recovery="<<a.recovery<<'\n';return summary.verdict=="CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_EXECUTION_INVALID"?1:0;}
