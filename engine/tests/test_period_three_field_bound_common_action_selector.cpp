// FTD-0718: export the locked force seed and independently replay the
// algorithmically selected source-free co-moving field correction.

#define FTD_0712_EMBEDDED
#include "test_resonant_internal_gait_cancellation.cpp"
#undef FTD_0712_EMBEDDED

#include "ftd/eft/coupled_matched_face_transaction.h"
#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/matched_face_momentum_transaction.h"
#include "ftd/eft/quadratic_coat_orbit_gather.h"
#include "ftd/eft/spline_poynting_momentum.h"

#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr char selector3_protocol_sha256[] =
    "EAC3AF4476F6F7FF4223B2D2B9BA864E151D0625B17175BFD4F79555C6CCED10";
constexpr char selector3_matter_protocol_sha256[] =
    "668C2D55EBB59572CE6C1E01928E4AE9A94E0913C964F5E45A69CCC8B5C2B4F9";
constexpr char selector3_field_protocol_sha256[] =
    "5F74489C3BD5F7DCC28B99442DE13FBA36AC9110F9099065FF70C65F6041BE19";
constexpr char selector3_preflight_protocol_sha256[] =
    "BCAE18C3786A02266910F80875DD13FD0E3337A91635A01F83252170B5BD294B";
constexpr int selector3_ticks = 3;
constexpr int selector3_L = 33;

struct Selector3Field {
  ftd::eft::MatchedFaceFlux electric;
  ftd::eft::MatchedEdgeField magnetic;
  explicit Selector3Field(int L = 0) : electric(L), magnetic(L) {}
};

struct Selector3Matter {
  Vec3 p0{};
  Vec3 p1{};
  Vec3 velocity{};
};

struct Selector3Seed {
  int particle = 0;
  int tick = 0;
  int charge = 0;
  Vec3 start{};
  Vec3 end{};
  Vec3 velocity{};
  Vec3 target_impulse{};
  Vec3 base_impulse{};
  Vec3 residual{};
};

struct Selector3Summary {
  bool parents = false;
  bool reference = false;
  bool matter = false;
  bool field = false;
  bool currents = false;
  bool seed = false;
  bool correction_loaded = false;
  bool selector_solved = false;
  bool selector_insufficient = false;
  bool replay = false;
  int segments = 0;
  double beta = INFINITY;
  double continuity = INFINITY;
  double causal = INFINITY;
  double correction_divergence = INFINITY;
  double correction_return = INFINITY;
  double maximum_force_residual = INFINITY;
  double maximum_gauss = INFINITY;
  double maximum_electric_adjoint = INFINITY;
  double maximum_magnetic_work = INFINITY;
  double maximum_energy = INFINITY;
  double sourced_return = INFINITY;
  double maximum_local_momentum_defect = INFINITY;
  double maximum_spline_momentum_defect = INFINITY;
  double correction_l2 = INFINITY;
  double correction_maximum = INFINITY;
  std::vector<Selector3Seed> rows;
  std::string verdict = "PERIOD_THREE_FIELD_BOUND_SELECTOR_EXECUTION_INVALID";
};

double selector3_energy(const Vec3& p) {
  return std::sqrt(ftd::E_REST*ftd::E_REST
      +ftd::C_SPEED*ftd::C_SPEED*p.mag2());
}

bool selector3_fingerprint(const std::filesystem::path& path,
                           const char* protocol, const char* verdict) {
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find(protocol) != std::string::npos
      && bytes.find(verdict) != std::string::npos;
}

bool selector3_load_delta(std::array<Vec3, count>& delta) {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0713/ftd_0713_causal_bound_internal_gait_state_v1.csv";
  std::ifstream input(path); std::string line; std::getline(input, line);
  int loaded = 0;
  while (std::getline(input, line)) {
    std::stringstream row(line); std::array<std::string, 4> f;
    for (auto& value : f) std::getline(row, value, ',');
    const int particle = std::stoi(f[0]);
    if (particle < 0 || particle >= count) return false;
    delta[particle] = {std::stod(f[1]), std::stod(f[2]), std::stod(f[3])};
    ++loaded;
  }
  Vec3 sum{}; for (const auto& value : delta) sum += value;
  return loaded == count && sum.mag() <= 1e-14;
}

bool selector3_load_matter(
    std::array<std::array<Selector3Matter, count>, selector3_ticks>& matter) {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0715/ftd_0715_period_three_internal_momentum_lift_segments_v1.csv";
  std::ifstream input(path); std::string line; std::getline(input, line);
  std::array<std::array<bool, count>, selector3_ticks> seen{};
  int loaded = 0;
  while (std::getline(input, line)) {
    std::stringstream row(line); std::array<std::string, 20> f;
    for (auto& value : f) std::getline(row, value, ',');
    const int particle = std::stoi(f[0]);
    const int tick = std::stoi(f[1]);
    if (particle < 0 || particle >= count || tick < 0
        || tick >= selector3_ticks || seen[tick][particle]) return false;
    seen[tick][particle] = true;
    matter[tick][particle].velocity = {
        std::stod(f[5]), std::stod(f[6]), std::stod(f[7])};
    matter[tick][particle].p0 = {
        std::stod(f[8]), std::stod(f[9]), std::stod(f[10])};
    matter[tick][particle].p1 = {
        std::stod(f[11]), std::stod(f[12]), std::stod(f[13])};
    ++loaded;
  }
  return loaded == count*selector3_ticks;
}

bool selector3_load_field(Selector3Field& field,
                          const std::filesystem::path& path) {
  std::ifstream input(path); std::string line; std::getline(input, line);
  std::vector<bool> seen(field.electric.x.size(), false); int loaded = 0;
  while (std::getline(input, line)) {
    std::stringstream row(line); std::array<std::string, 9> f;
    for (auto& value : f) std::getline(row, value, ',');
    const int x = std::stoi(f[0]), y = std::stoi(f[1]), z = std::stoi(f[2]);
    const auto i = static_cast<std::size_t>(field.electric.index(x,y,z));
    if (i >= seen.size() || seen[i]) return false;
    seen[i] = true;
    field.electric.x[i] = std::stod(f[3]);
    field.electric.y[i] = std::stod(f[4]);
    field.electric.z[i] = std::stod(f[5]);
    field.magnetic.x[i] = std::stod(f[6]);
    field.magnetic.y[i] = std::stod(f[7]);
    field.magnetic.z[i] = std::stod(f[8]);
    ++loaded;
  }
  return loaded == static_cast<int>(seen.size())
      && std::all_of(seen.begin(), seen.end(), [](bool value){return value;});
}

template <typename Field>
void selector3_add(Field& target, const Field& source, double scale = 1.0) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale*source.x[i];
    target.y[i] += scale*source.y[i];
    target.z[i] += scale*source.z[i];
  }
}

void selector3_add_segment(ftd::eft::MatchedFaceFlux& target,
                           const ftd::eft::QuadraticCoatFaceCurrent& segment) {
  for (const auto& entry : segment.sparse_current) {
    const auto i = static_cast<std::size_t>(target.index(
        entry.face.x, entry.face.y, entry.face.z));
    auto& value = entry.axis == 0 ? target.x[i]
        : (entry.axis == 1 ? target.y[i] : target.z[i]);
    value += entry.value;
  }
}

template <typename Field>
Field selector3_translate(const Field& input, int dx) {
  Field output(input.L);
  for (int x=0;x<input.L;++x) for(int y=0;y<input.L;++y)
    for(int z=0;z<input.L;++z) {
      const auto from=static_cast<std::size_t>(input.index(x,y,z));
      const auto to=static_cast<std::size_t>(output.index(x+dx,y,z));
      output.x[to]=input.x[from]; output.y[to]=input.y[from];
      output.z[to]=input.z[from];
    }
  return output;
}

void selector3_advance(Selector3Field& state,
                       const ftd::eft::MatchedFaceFlux* current) {
  selector3_add(state.magnetic,
      ftd::eft::matched_curl_adjoint(state.electric), -ftd::C_SPEED);
  selector3_add(state.electric,
      ftd::eft::matched_curl(state.magnetic), +ftd::C_SPEED);
  if (current != nullptr) selector3_add(state.electric, *current, -1.0);
}

ftd::eft::MatchedFaceFlux selector3_midpoint(
    const ftd::eft::MatchedFaceFlux& a,
    const ftd::eft::MatchedFaceFlux& b) {
  auto result = a; selector3_add(result, b, 1.0);
  for (std::size_t i=0;i<result.x.size();++i) {
    result.x[i]*=0.5; result.y[i]*=0.5; result.z[i]*=0.5;
  }
  return result;
}

std::vector<double> selector3_density(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const std::array<Vec3,count>& delta, int phase) {
  std::vector<double> density(static_cast<std::size_t>(selector3_L)
      *selector3_L*selector3_L,0.0);
  for (int particle=0;particle<count;++particle) {
    Vec3 x=position(reference.constituents[particle]);
    if(phase==1)x+=Vec3{1.0/3.0,0,0}+delta[particle];
    if(phase==2)x+=Vec3{2.0/3.0,0,0}-delta[particle];
    if(phase==3)x+=Vec3{1.0,0,0};
    const auto coat=ftd::eft::make_quadratic_polarity_coat(
        x,reference.charges[particle]);
    if(!coat.valid)return {};
    for(std::size_t item=0;item<coat.weight_count;++item) {
      const auto& entry=coat.weights[item];
      density[static_cast<std::size_t>(reference.electric.index(
          entry.site.x,entry.site.y,entry.site.z))]+=entry.weight;
    }
  }
  return density;
}

bool selector3_make_segments(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const std::array<Vec3,count>& delta,
    std::array<std::array<ftd::eft::QuadraticCoatFaceCurrent,count>,
               selector3_ticks>& segments,
    std::array<ftd::eft::MatchedFaceFlux,selector3_ticks>& currents,
    Selector3Summary& summary) {
  summary.continuity=0.0; summary.causal=0.0;
  for(int particle=0;particle<count;++particle) {
    const Vec3 x0=position(reference.constituents[particle]);
    const std::array<Vec3,4>x{{x0,x0+Vec3{1.0/3.0,0,0}+delta[particle],
        x0+Vec3{2.0/3.0,0,0}-delta[particle],x0+Vec3{1.0,0,0}}};
    for(int tick=0;tick<selector3_ticks;++tick) {
      auto& segment=segments[tick][particle];
      segment=ftd::eft::make_quadratic_coat_face_current(
          selector3_L,x[tick],x[tick+1],reference.charges[particle],false);
      ++summary.segments;
      if(!segment.valid)return false;
      summary.continuity=std::max(summary.continuity,segment.continuity_residual);
      summary.causal=std::max(summary.causal,segment.causal_excess);
      selector3_add_segment(currents[tick],segment);
    }
  }
  return summary.segments==count*selector3_ticks
      &&summary.continuity<=1e-12&&summary.causal<=1e-12;
}

double selector3_field_norm(const Selector3Field& field, double& maximum) {
  long double squared=0.0L; maximum=0.0;
  for(std::size_t i=0;i<field.electric.x.size();++i) {
    const std::array<double,6> values{{field.electric.x[i],field.electric.y[i],
        field.electric.z[i],field.magnetic.x[i],field.magnetic.y[i],
        field.magnetic.z[i]}};
    for(double value:values){squared+=static_cast<long double>(value)*value;
      maximum=std::max(maximum,std::abs(value));}
  }
  return std::sqrt(static_cast<double>(squared));
}

void selector3_replay(
    Selector3Summary& summary, const Selector3Field& initial,
    const ftd::eft::ConnectedMooreBlockState& reference,
    const std::array<Vec3,count>& delta,
    const std::array<std::array<Selector3Matter,count>,selector3_ticks>& matter,
    const std::array<std::array<ftd::eft::QuadraticCoatFaceCurrent,count>,
                     selector3_ticks>& segments,
    const std::array<ftd::eft::MatchedFaceFlux,selector3_ticks>& currents,
    bool write_seed) {
  Selector3Field state=initial;
  summary.maximum_force_residual=0.0; summary.maximum_gauss=0.0;
  summary.maximum_electric_adjoint=0.0; summary.maximum_magnetic_work=0.0;
  summary.maximum_energy=0.0; summary.maximum_local_momentum_defect=0.0;
  summary.maximum_spline_momentum_defect=0.0;
  if(write_seed)summary.rows.clear();
  for(int tick=0;tick<selector3_ticks;++tick) {
    const auto density0=selector3_density(reference,delta,tick);
    const auto density1=selector3_density(reference,delta,tick+1);
    summary.maximum_gauss=std::max(summary.maximum_gauss,
        ftd::eft::max_fractional_gauss_residual(state.electric,density0));
    const double field0=summary.beta*ftd::eft::matched_modified_energy(
        state.electric,state.magnetic,ftd::C_SPEED);
    const Vec3 local0=ftd::eft::matched_local_translation_momentum(
        state.electric,state.magnetic)*summary.beta;
    const auto spline0=ftd::eft::measure_spline_poynting_momentum(
        state.electric,state.magnetic,ftd::C_SPEED,1.0,summary.beta);
    const Selector3Field before=state;
    selector3_advance(state,&currents[tick]);
    const auto electric_mid=selector3_midpoint(before.electric,state.electric);
    summary.maximum_gauss=std::max(summary.maximum_gauss,
        ftd::eft::max_fractional_gauss_residual(state.electric,density1));
    Vec3 matter_impulse{};
    long double matter_energy0=0.0L,matter_energy1=0.0L;
    for(int particle=0;particle<count;++particle) {
      const auto gather=ftd::eft::evaluate_quadratic_coat_orbit_gather_prevalidated_fields(
          segments[tick][particle],electric_mid,state.magnetic,
          matter[tick][particle].velocity,1.0,summary.beta,1.0);
      if(!gather.valid)return;
      const Vec3 impulse=(gather.electric_force+Vec3::cross(
          matter[tick][particle].velocity,gather.magnetic_average))*summary.beta;
      const Vec3 target=matter[tick][particle].p1-matter[tick][particle].p0;
      const Vec3 residual=target-impulse;
      summary.maximum_force_residual=std::max(
          summary.maximum_force_residual,residual.mag());
      summary.maximum_electric_adjoint=std::max(
          summary.maximum_electric_adjoint,gather.electric_adjoint_residual);
      summary.maximum_magnetic_work=std::max(
          summary.maximum_magnetic_work,gather.magnetic_work_residual);
      matter_impulse+=target;
      matter_energy0+=selector3_energy(matter[tick][particle].p0);
      matter_energy1+=selector3_energy(matter[tick][particle].p1);
      if(write_seed)summary.rows.push_back({particle,tick,
          reference.charges[particle],segments[tick][particle].start_effective_position,
          segments[tick][particle].end_effective_position,
          matter[tick][particle].velocity,target,impulse,residual});
    }
    const double field1=summary.beta*ftd::eft::matched_modified_energy(
        state.electric,state.magnetic,ftd::C_SPEED);
    summary.maximum_energy=std::max(summary.maximum_energy,std::abs(
        static_cast<double>(matter_energy1-matter_energy0)+field1-field0));
    const Vec3 local1=ftd::eft::matched_local_translation_momentum(
        state.electric,state.magnetic)*summary.beta;
    const auto spline1=ftd::eft::measure_spline_poynting_momentum(
        state.electric,state.magnetic,ftd::C_SPEED,1.0,summary.beta);
    if(!spline0.valid||!spline1.valid)return;
    summary.maximum_local_momentum_defect=std::max(
        summary.maximum_local_momentum_defect,(matter_impulse+local1-local0).mag());
    summary.maximum_spline_momentum_defect=std::max(
        summary.maximum_spline_momentum_defect,
        (matter_impulse+spline1.momentum-spline0.momentum).mag());
  }
  const auto expected_e=selector3_translate(initial.electric,1);
  const auto expected_b=selector3_translate(initial.magnetic,1);
  summary.sourced_return=std::max(
      ftd::eft::matched_face_max_difference(state.electric,expected_e),
      ftd::eft::matched_edge_max_difference(state.magnetic,expected_b));
  summary.replay=std::isfinite(summary.maximum_force_residual)
      &&std::isfinite(summary.maximum_energy)&&std::isfinite(summary.sourced_return);
}

void selector3_write(const Selector3Summary& summary,
                     const std::filesystem::path& directory) {
  std::filesystem::create_directories(directory);
  std::ofstream seed(directory/"ftd_0718_period_three_force_seed_v1.csv");
  seed<<"particle,tick,charge,start_x,start_y,start_z,end_x,end_y,end_z,"
        "velocity_x,velocity_y,velocity_z,target_x,target_y,target_z,"
        "base_x,base_y,base_z,residual_x,residual_y,residual_z\n";
  seed<<std::setprecision(17);
  for(const auto& r:summary.rows)seed<<r.particle<<','<<r.tick<<','<<r.charge<<','
      <<r.start.x<<','<<r.start.y<<','<<r.start.z<<','<<r.end.x<<','<<r.end.y<<','
      <<r.end.z<<','<<r.velocity.x<<','<<r.velocity.y<<','<<r.velocity.z<<','
      <<r.target_impulse.x<<','<<r.target_impulse.y<<','<<r.target_impulse.z<<','
      <<r.base_impulse.x<<','<<r.base_impulse.y<<','<<r.base_impulse.z<<','
      <<r.residual.x<<','<<r.residual.y<<','<<r.residual.z<<'\n';
  std::ofstream json(directory/"ftd_0718_period_three_field_bound_selector_replay_v1.json");
  json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0718\",\n"
      <<"  \"protocol_sha256\": \""<<selector3_protocol_sha256<<"\",\n"
      <<"  \"verdict\": \""<<summary.verdict<<"\",\n"
      <<"  \"production_changed\": false,\n"
      <<"  \"parent_pass\": "<<summary.parents<<",\n"
      <<"  \"reference_pass\": "<<summary.reference<<",\n"
      <<"  \"matter_pass\": "<<summary.matter<<",\n"
      <<"  \"field_pass\": "<<summary.field<<",\n"
      <<"  \"current_pass\": "<<summary.currents<<",\n"
      <<"  \"seed_pass\": "<<summary.seed<<",\n"
      <<"  \"correction_loaded\": "<<summary.correction_loaded<<",\n"
      <<"  \"selector_solved\": "<<summary.selector_solved<<",\n"
      <<"  \"selector_insufficient\": "<<summary.selector_insufficient<<",\n"
      <<"  \"replay_pass\": "<<summary.replay<<",\n"
      <<"  \"segments\": "<<summary.segments<<",\n"
      <<"  \"beta\": "<<summary.beta<<",\n"
      <<"  \"continuity_residual\": "<<summary.continuity<<",\n"
      <<"  \"causal_excess\": "<<summary.causal<<",\n"
      <<"  \"correction_divergence\": "<<summary.correction_divergence<<",\n"
      <<"  \"correction_return_residual\": "<<summary.correction_return<<",\n"
      <<"  \"maximum_force_residual\": "<<summary.maximum_force_residual<<",\n"
      <<"  \"maximum_gauss_residual\": "<<summary.maximum_gauss<<",\n"
      <<"  \"maximum_electric_adjoint_residual\": "<<summary.maximum_electric_adjoint<<",\n"
      <<"  \"maximum_magnetic_work_residual\": "<<summary.maximum_magnetic_work<<",\n"
      <<"  \"maximum_energy_residual\": "<<summary.maximum_energy<<",\n"
      <<"  \"sourced_return_residual\": "<<summary.sourced_return<<",\n"
      <<"  \"maximum_local_momentum_defect\": "<<summary.maximum_local_momentum_defect<<",\n"
      <<"  \"maximum_spline_momentum_defect\": "<<summary.maximum_spline_momentum_defect<<",\n"
      <<"  \"correction_l2\": "<<summary.correction_l2<<",\n"
      <<"  \"correction_maximum\": "<<summary.correction_maximum<<"\n}\n";
}

} // namespace

int main() {
  Selector3Summary summary;
  const auto root=std::filesystem::path(__FILE__).parent_path().parent_path();
  const auto results=root/"results";
  const auto directory=results/"ftd_0718";
  summary.parents=selector3_fingerprint(results/
      "ftd_0715/ftd_0715_period_three_internal_momentum_lift_v1.json",
      selector3_matter_protocol_sha256,"PERIOD_THREE_MOMENTUM_LIFT_CONSTRUCTIVE")
      &&selector3_fingerprint(results/
      "ftd_0716/ftd_0716_period_three_comoving_field_solvability_v1.json",
      selector3_field_protocol_sha256,"PERIOD_THREE_COMOVING_FIELD_SOLUTION_REGULAR")
      &&selector3_fingerprint(results/
      "ftd_0717/ftd_0717_period_three_common_action_preflight_v1.json",
      selector3_preflight_protocol_sha256,
      "PERIOD_THREE_MINIMUM_NORM_FIELD_REQUIRES_COUPLED_SELECTION");
  std::array<Vec3,count> delta{}; summary.reference=selector3_load_delta(delta);
  auto reference=gait_reference(summary.reference);
  std::array<std::array<Selector3Matter,count>,selector3_ticks> matter{};
  summary.matter=selector3_load_matter(matter);
  Selector3Field base(selector3_L);
  summary.field=selector3_load_field(base,results/
      "ftd_0716/ftd_0716_period_three_comoving_field_correction_v1.csv");
  std::array<std::array<ftd::eft::QuadraticCoatFaceCurrent,count>,selector3_ticks>
      segments{};
  std::array<ftd::eft::MatchedFaceFlux,selector3_ticks> currents{{
      ftd::eft::MatchedFaceFlux(selector3_L),ftd::eft::MatchedFaceFlux(selector3_L),
      ftd::eft::MatchedFaceFlux(selector3_L)}};
  if(summary.reference)summary.currents=selector3_make_segments(
      reference,delta,segments,currents,summary);
  summary.beta=ftd::eft::measure_face_flux_normalization()
      .mapped_field_work_coefficient;
  if(summary.parents&&summary.reference&&summary.matter&&summary.field
      &&summary.currents&&std::isfinite(summary.beta)) {
    selector3_replay(summary,base,reference,delta,matter,segments,currents,true);
    summary.seed=summary.replay&&summary.rows.size()==count*selector3_ticks;
  }
  const auto correction_path=directory/
      "ftd_0718_period_three_field_bound_correction_v1.csv";
  Selector3Field correction(selector3_L);
  summary.correction_loaded=std::filesystem::exists(correction_path)
      &&selector3_load_field(correction,correction_path);
  const auto solve_path=directory/
      "ftd_0718_period_three_field_bound_selector_solve_v1.json";
  summary.selector_solved=selector3_fingerprint(solve_path,
      selector3_protocol_sha256,"PERIOD_THREE_HOMOGENEOUS_FORCE_SELECTOR_SOLVED");
  summary.selector_insufficient=selector3_fingerprint(solve_path,
      selector3_protocol_sha256,
      "PERIOD_THREE_HOMOGENEOUS_FIELD_FORCE_SPACE_INSUFFICIENT");
  if(summary.correction_loaded) {
    summary.correction_l2=selector3_field_norm(correction,
        summary.correction_maximum);
    summary.correction_divergence=ftd::eft::max_divergence(correction.electric);
    Selector3Field advanced=correction;
    for(int tick=0;tick<selector3_ticks;++tick)selector3_advance(advanced,nullptr);
    summary.correction_return=std::max(
        ftd::eft::matched_face_max_difference(advanced.electric,
            selector3_translate(correction.electric,1)),
        ftd::eft::matched_edge_max_difference(advanced.magnetic,
            selector3_translate(correction.magnetic,1)));
    selector3_add(base.electric,correction.electric);
    selector3_add(base.magnetic,correction.magnetic);
    selector3_replay(summary,base,reference,delta,matter,segments,currents,false);
  }
  const bool execution=summary.parents&&summary.reference&&summary.matter
      &&summary.field&&summary.currents&&summary.seed;
  const bool gates=summary.correction_loaded&&summary.replay
      &&summary.continuity<=1e-10&&summary.causal<=1e-10
      &&summary.correction_divergence<=1e-10&&summary.correction_return<=1e-10
      &&summary.maximum_force_residual<=1e-10&&summary.maximum_gauss<=1e-10
      &&summary.maximum_electric_adjoint<=1e-10
      &&summary.maximum_magnetic_work<=1e-10&&summary.maximum_energy<=1e-10
      &&summary.sourced_return<=1e-10;
  if(!execution)summary.verdict=
      "PERIOD_THREE_FIELD_BOUND_SELECTOR_EXECUTION_INVALID";
  else if(!summary.correction_loaded)summary.verdict=
      "PERIOD_THREE_FIELD_BOUND_SELECTOR_AWAITING_CORRECTION";
  else if(summary.selector_insufficient)summary.verdict=
      "PERIOD_THREE_HOMOGENEOUS_FIELD_FORCE_SPACE_INSUFFICIENT";
  else if(summary.selector_solved&&gates)summary.verdict=
      "PERIOD_THREE_FIELD_BOUND_COMMON_ACTION_CONSTRUCTIVE_MOMENTUM_OPEN";
  else summary.verdict="PERIOD_THREE_FIELD_BOUND_SELECTOR_REPLAY_NEGATIVE";
  selector3_write(summary,directory);
  std::cout<<std::setprecision(17)<<"protocol_sha256="
      <<selector3_protocol_sha256<<'\n'<<"verdict="<<summary.verdict<<'\n'
      <<"force="<<summary.maximum_force_residual
      <<" energy="<<summary.maximum_energy<<" gauss="<<summary.maximum_gauss
      <<" return="<<summary.sourced_return<<'\n'
      <<"correction_divergence="<<summary.correction_divergence
      <<" correction_return="<<summary.correction_return<<'\n';
  return summary.verdict=="PERIOD_THREE_FIELD_BOUND_SELECTOR_EXECUTION_INVALID"?1:0;
}
