// FTD-0655: observer-only co-motion of matter and field-energy structure factors.

#include <complex>
#include <numeric>

#define main ftd_0650_reference_main
#include "test_cell_measure_long_horizon_transport.cpp"
#undef main

namespace {

constexpr char protocol_sha256_v5[] =
    "09523E64E273E7808FF21A446B26C012531931EF948F3F48F090D4F851C0F2A0";
constexpr double physical_horizon_v5 = 64.0;

struct StructureSample {
  int tick = 0;
  std::complex<double> matter{};
  std::complex<double> field{};
  Vec3 center{};
};

struct SeriesFit {
  bool valid = false;
  double mean_amplitude = NAN;
  double amplitude_cv = INFINITY;
  double slope = NAN;
  double phase_rms = INFINITY;
  double phase_velocity = NAN;
};

struct DressingArm {
  Arm dynamics;
  std::vector<StructureSample> samples;
  SeriesFit matter;
  SeriesFit field;
  bool matter_pass = false;
  bool field_pass = false;
  double relative_phase_rms = INFINITY;
  double center_velocity = NAN;
  double matter_center_mismatch = INFINITY;
  double field_matter_mismatch = INFINITY;
};

struct DressingSummary {
  std::vector<DressingArm> arms;
  bool coverage = false;
  bool execution = false;
  bool exact = false;
  bool coherence = false;
  bool matter = false;
  bool field = false;
  bool mirror = false;
  bool cubic = false;
  bool width_trend = false;
  double worst_action = 0.0;
  double worst_recovery = 0.0;
  double worst_strain = 0.0;
  double mirror_residual = 0.0;
  double cubic_residual = 0.0;
  std::map<int,double> max_velocity_mismatch;
  std::map<int,double> max_relative_phase_rms;
  std::map<int,double> max_field_cv;
  std::string verdict = "MOBILE_DRESSING_STRUCTURE_FACTOR_EXECUTION_INVALID";
};

std::array<int,3> wave_integer(const Spec& spec) {
  std::array<int,3> result{{0,0,0}};
  if (spec.family == "110") return {{1,1,0}};
  if (spec.family == "111") return {{1,1,1}};
  const std::array<double,3> direction{{
      spec.direction.x,spec.direction.y,spec.direction.z}};
  int axis = 0;
  for (int i = 1; i < 3; ++i)
    if (std::abs(direction[i]) > std::abs(direction[axis])) axis = i;
  result[axis] = direction[axis] < 0.0 ? -1 : 1;
  return result;
}

double phase_at(const std::array<int,3>& n, int L,
                double x, double y, double z) {
  return 2.0*ftd::PI*(n[0]*x+n[1]*y+n[2]*z)/L;
}

StructureSample observe_structure(const ConnectedMooreBlockState& state,
                                  const Spec& spec, int tick,
                                  double mass_scale,
                                  double field_scale) {
  StructureSample sample;
  sample.tick = tick;
  sample.center = center(state);
  const auto n = wave_integer(spec);
  for (const auto& point : state.constituents) {
    const Vec3 r = position(point);
    sample.matter += mass_scale*std::polar(
        1.0,-phase_at(n,state.electric.L,r.x,r.y,r.z));
  }
  const int L = state.electric.L;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const std::size_t i = index(L,x,y,z);
        const double ex = 0.5*field_scale*state.electric.x[i]*state.electric.x[i];
        const double ey = 0.5*field_scale*state.electric.y[i]*state.electric.y[i];
        const double ez = 0.5*field_scale*state.electric.z[i]*state.electric.z[i];
        const double bx = 0.5*field_scale*state.magnetic_half.x[i]*state.magnetic_half.x[i];
        const double by = 0.5*field_scale*state.magnetic_half.y[i]*state.magnetic_half.y[i];
        const double bz = 0.5*field_scale*state.magnetic_half.z[i]*state.magnetic_half.z[i];
        sample.field += ex*std::polar(1.0,-phase_at(n,L,x+0.5,y,z));
        sample.field += ey*std::polar(1.0,-phase_at(n,L,x,y+0.5,z));
        sample.field += ez*std::polar(1.0,-phase_at(n,L,x,y,z+0.5));
        sample.field += bx*std::polar(1.0,-phase_at(n,L,x,y+0.5,z+0.5));
        sample.field += by*std::polar(1.0,-phase_at(n,L,x+0.5,y,z+0.5));
        sample.field += bz*std::polar(1.0,-phase_at(n,L,x+0.5,y+0.5,z));
      }
  return sample;
}

std::vector<double> unwrap(const std::vector<double>& raw) {
  if (raw.empty()) return {};
  std::vector<double> result(raw.size());
  result[0] = raw[0];
  for (std::size_t i = 1; i < raw.size(); ++i) {
    double delta = raw[i]-raw[i-1];
    while (delta > ftd::PI) delta -= 2.0*ftd::PI;
    while (delta < -ftd::PI) delta += 2.0*ftd::PI;
    result[i] = result[i-1]+delta;
  }
  return result;
}

SeriesFit fit_series(const std::vector<StructureSample>& samples,
                     bool matter, const Spec& spec, int L) {
  SeriesFit fit;
  if (samples.size() < 3) return fit;
  std::vector<double> amplitude,phase;
  amplitude.reserve(samples.size());
  phase.reserve(samples.size());
  for (const auto& sample : samples) {
    const auto value = matter ? sample.matter : sample.field;
    amplitude.push_back(std::abs(value));
    phase.push_back(std::arg(value));
  }
  fit.mean_amplitude = std::accumulate(
      amplitude.begin(),amplitude.end(),0.0)/amplitude.size();
  if (!(fit.mean_amplitude > 0.0)) return fit;
  long double variance = 0.0;
  for (double value : amplitude)
    variance += (value-fit.mean_amplitude)*(value-fit.mean_amplitude);
  fit.amplitude_cv = std::sqrt(static_cast<double>(variance/amplitude.size()))
      /fit.mean_amplitude;
  const auto theta = unwrap(phase);
  const double mean_t = 0.5*static_cast<double>(theta.size()-1);
  const double mean_theta = std::accumulate(
      theta.begin(),theta.end(),0.0)/theta.size();
  long double numerator = 0.0,denominator = 0.0;
  for (std::size_t i = 0; i < theta.size(); ++i) {
    const double dt = static_cast<double>(i)-mean_t;
    numerator += dt*(theta[i]-mean_theta);
    denominator += dt*dt;
  }
  if (!(denominator > 0.0)) return fit;
  fit.slope = static_cast<double>(numerator/denominator);
  long double residual2 = 0.0;
  for (std::size_t i = 0; i < theta.size(); ++i) {
    const double predicted = mean_theta
        +fit.slope*(static_cast<double>(i)-mean_t);
    residual2 += (theta[i]-predicted)*(theta[i]-predicted);
  }
  fit.phase_rms = std::sqrt(static_cast<double>(residual2/theta.size()));
  const auto n = wave_integer(spec);
  const double k = 2.0*ftd::PI*std::sqrt(
      static_cast<double>(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]))/L;
  fit.phase_velocity = -fit.slope/k;
  fit.valid = std::isfinite(fit.phase_velocity)
      && std::isfinite(fit.phase_rms) && std::isfinite(fit.amplitude_cv);
  return fit;
}

double relative_phase_rms(const std::vector<StructureSample>& samples) {
  std::vector<double> phase;
  phase.reserve(samples.size());
  for (const auto& sample : samples) {
    if (!(std::abs(sample.matter) > 0.0) || !(std::abs(sample.field) > 0.0))
      return INFINITY;
    phase.push_back(std::arg(sample.field/sample.matter));
  }
  const auto theta = unwrap(phase);
  const double mean = std::accumulate(theta.begin(),theta.end(),0.0)/theta.size();
  long double residual2 = 0.0;
  for (double value : theta) residual2 += (value-mean)*(value-mean);
  return std::sqrt(static_cast<double>(residual2/theta.size()));
}

std::vector<Spec> specs_v5() {
  const double inv_sqrt2 = 1.0/std::sqrt(2.0);
  const double inv_sqrt3 = 1.0/std::sqrt(3.0);
  const std::array<std::pair<std::string,Vec3>,3> directions{{
      {"100",{1,0,0}},
      {"110",{inv_sqrt2,inv_sqrt2,0}},
      {"111",{inv_sqrt3,inv_sqrt3,inv_sqrt3}}}};
  std::vector<Spec> result;
  for (int width : {2,3,4}) {
    for (const auto& family : directions)
      result.push_back({"p_w"+std::to_string(width)+"_v03_"+family.first,
          "primary",family.first,width,0,0,family.second,0.03});
    result.push_back({"m_w"+std::to_string(width)+"_v03_100",
        "mirror","100",width,0,0,{1,0,0},-0.03});
    result.push_back({"c_w"+std::to_string(width)+"_o1",
        "cubic","100",width,1,1,{0,1,0},0.03});
    result.push_back({"c_w"+std::to_string(width)+"_o2",
        "cubic","100",width,2,2,{0,0,1},0.03});
  }
  return result;
}

DressingArm run_arm_v5(const Spec& spec) {
  DressingArm result;
  Arm& arm = result.dynamics;
  arm.spec = spec;
  arm.a = 2.0/spec.width;
  arm.ticks = 32*spec.width;
  arm.mass_scale = arm.a*arm.a*arm.a;
  arm.polarity_scale = arm.mass_scale;
  arm.binding_scale = arm.mass_scale;
  arm.field_scale = 1.0/arm.a;
  const int L = 8*spec.width+1;
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      L,spec.width,spec.orientation,0,0.0,1e-13,16384);
  if (!initialized.valid) return result;
  auto initial = initialized.state;
  scale_field(initial,arm.polarity_scale);
  const Vec3 launch = ftd::eft::production_flat_momentum(
      spec.direction*spec.speed)*arm.mass_scale;
  for (auto& point : initial.constituents) point.momentum = launch;
  arm.constituent_count = static_cast<int>(initial.constituents.size());
  arm.initialized = arm.constituent_count
      == 2*spec.width*spec.width*spec.width;
  if (!arm.initialized) return result;

  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.constituent_mass_scale = arm.mass_scale;
  options.polarity_scale = arm.polarity_scale;
  options.binding_stiffness = arm.binding_scale;
  options.field_energy_scale = arm.field_scale;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache,reverse_cache;

  auto state = initial;
  arm.initial_hash = state_hash(initial);
  arm.initial_center = center(initial);
  result.samples.push_back(observe_structure(
      state,spec,0,arm.mass_scale,arm.field_scale));
  double initial_energy = NAN;
  arm.forward = true;
  for (int tick = 1; tick <= arm.ticks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state,options,&forward_cache);
    if (tick == 1) initial_energy = total_energy(step,false);
    const TickRecord row = tick_record(
        "forward",tick,step,step.later,true,initial_energy);
    arm.records.push_back(row);
    arm.maximum_action = std::max(arm.maximum_action,row.action);
    arm.maximum_causal = std::max(arm.maximum_causal,row.causal);
    arm.maximum_relative_edge_strain = std::max(
        arm.maximum_relative_edge_strain,row.relative_edge_strain);
    arm.maximum_energy_drift = std::max(
        arm.maximum_energy_drift,row.energy_drift);
    arm.maximum_anchor_multiplicity = std::max(
        arm.maximum_anchor_multiplicity,row.anchor_multiplicity);
    arm.total_hops += row.site_hops;
    const bool pass = row.valid && step.common_action_gates_pass
        && row.graph_connected && row.graph_local
        && row.constituent_count == arm.constituent_count
        && row.anchor_multiplicity <= fibre_limit
        && row.action <= 1e-9 && row.causal <= 1e-12;
    if (!pass) { arm.forward = false; break; }
    state = step.later;
    result.samples.push_back(observe_structure(
        state,spec,tick,arm.mass_scale,arm.field_scale));
  }
  arm.forward = arm.forward
      && result.samples.size() == static_cast<std::size_t>(arm.ticks+1);
  if (!arm.forward) return result;

  arm.final_state = state;
  arm.final_hash = state_hash(state);
  arm.final_center = center(state);
  const Vec3 displacement = arm.final_center-arm.initial_center;
  result.center_velocity = arm.a*displacement.dot(spec.direction)
      /physical_horizon_v5;
  result.matter = fit_series(result.samples,true,spec,L);
  result.field = fit_series(result.samples,false,spec,L);
  result.relative_phase_rms = relative_phase_rms(result.samples);
  result.matter_center_mismatch = std::abs(
      result.matter.phase_velocity-result.center_velocity)/0.03;
  result.field_matter_mismatch = std::abs(
      result.field.phase_velocity-result.matter.phase_velocity)/0.03;
  result.matter_pass = result.matter.valid
      && result.matter.mean_amplitude > 1e-8
      && result.matter.phase_rms < 0.10
      && result.matter.amplitude_cv < 0.10
      && result.matter_center_mismatch < 0.10;
  result.field_pass = result.field.valid
      && result.field.mean_amplitude > 1e-12
      && result.field.phase_rms < 0.20
      && result.field.amplitude_cv < 0.20
      && result.relative_phase_rms < 0.20
      && result.field_matter_mismatch < 0.10;

  arm.reverse = true;
  for (int tick = arm.ticks; tick >= 1; --tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state,options,&reverse_cache);
    const TickRecord row = tick_record(
        "reverse",tick,step,step.earlier,false,initial_energy);
    arm.records.push_back(row);
    arm.maximum_action = std::max(arm.maximum_action,row.action);
    arm.maximum_causal = std::max(arm.maximum_causal,row.causal);
    arm.maximum_relative_edge_strain = std::max(
        arm.maximum_relative_edge_strain,row.relative_edge_strain);
    arm.maximum_energy_drift = std::max(
        arm.maximum_energy_drift,row.energy_drift);
    arm.maximum_anchor_multiplicity = std::max(
        arm.maximum_anchor_multiplicity,row.anchor_multiplicity);
    const bool pass = row.valid && step.common_action_gates_pass
        && row.graph_connected && row.graph_local
        && row.constituent_count == arm.constituent_count
        && row.anchor_multiplicity <= fibre_limit
        && row.action <= 1e-9 && row.causal <= 1e-12;
    if (!pass) { arm.reverse = false; break; }
    state = step.earlier;
  }
  arm.reverse = arm.reverse
      && arm.records.size() == static_cast<std::size_t>(2*arm.ticks);
  if (arm.reverse) {
    arm.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initial,state);
    arm.recovered_hash = state_hash(state);
  }
  arm.exact = arm.forward && arm.reverse
      && arm.maximum_action <= 1e-9 && arm.maximum_causal <= 1e-12
      && arm.recovery <= 1e-7;
  arm.coherent = arm.exact
      && arm.maximum_relative_edge_strain <= 0.10
      && arm.maximum_anchor_multiplicity <= fibre_limit;
  return result;
}

const DressingArm* find_v5(const DressingSummary& summary, int width,
                           const std::string& kind,
                           const std::string& family, int maps = 0) {
  for (const auto& arm : summary.arms)
    if (arm.dynamics.spec.width == width
        && arm.dynamics.spec.kind == kind
        && arm.dynamics.spec.family == family
        && arm.dynamics.spec.rotation_maps == maps) return &arm;
  return nullptr;
}

double metric_difference(const DressingArm& lhs, const DressingArm& rhs) {
  return std::max({
      std::abs(lhs.matter.phase_velocity-rhs.matter.phase_velocity),
      std::abs(lhs.field.phase_velocity-rhs.field.phase_velocity),
      std::abs(lhs.matter.phase_rms-rhs.matter.phase_rms),
      std::abs(lhs.field.phase_rms-rhs.field.phase_rms),
      std::abs(lhs.matter.amplitude_cv-rhs.matter.amplitude_cv),
      std::abs(lhs.field.amplitude_cv-rhs.field.amplitude_cv),
      std::abs(lhs.relative_phase_rms-rhs.relative_phase_rms)});
}

void evaluate_v5(DressingSummary& summary) {
  summary.coverage = summary.arms.size() == 18;
  summary.execution = summary.exact = summary.coherence = summary.coverage;
  summary.matter = summary.field = summary.coverage;
  for (const auto& arm : summary.arms) {
    summary.execution = summary.execution && arm.dynamics.initialized
        && arm.dynamics.forward && arm.dynamics.reverse
        && arm.samples.size() == static_cast<std::size_t>(arm.dynamics.ticks+1);
    summary.exact = summary.exact && arm.dynamics.exact;
    summary.coherence = summary.coherence && arm.dynamics.coherent;
    summary.matter = summary.matter && arm.matter_pass;
    summary.field = summary.field && arm.field_pass;
    summary.worst_action = std::max(
        summary.worst_action,arm.dynamics.maximum_action);
    summary.worst_strain = std::max(
        summary.worst_strain,arm.dynamics.maximum_relative_edge_strain);
    if (std::isfinite(arm.dynamics.recovery))
      summary.worst_recovery = std::max(
          summary.worst_recovery,arm.dynamics.recovery);
    const int width = arm.dynamics.spec.width;
    summary.max_velocity_mismatch[width] = std::max(
        summary.max_velocity_mismatch[width],arm.field_matter_mismatch);
    summary.max_relative_phase_rms[width] = std::max(
        summary.max_relative_phase_rms[width],arm.relative_phase_rms);
    summary.max_field_cv[width] = std::max(
        summary.max_field_cv[width],arm.field.amplitude_cv);
  }

  summary.mirror = summary.cubic = summary.execution;
  for (int width : {2,3,4}) {
    const auto* primary = find_v5(summary,width,"primary","100");
    const auto* mirror = find_v5(summary,width,"mirror","100");
    if (!primary || !mirror) { summary.mirror = false; continue; }
    const double mirror_residual = std::max({
        std::abs(primary->matter.phase_velocity+mirror->matter.phase_velocity),
        std::abs(primary->field.phase_velocity+mirror->field.phase_velocity),
        std::abs(primary->center_velocity+mirror->center_velocity)});
    summary.mirror_residual = std::max(
        summary.mirror_residual,mirror_residual);
    summary.mirror = summary.mirror && mirror_residual < 1e-8;
    for (int maps : {1,2}) {
      const auto* cubic = find_v5(summary,width,"cubic","100",maps);
      if (!cubic) { summary.cubic = false; continue; }
      const double residual = metric_difference(*primary,*cubic);
      summary.cubic_residual = std::max(summary.cubic_residual,residual);
      summary.cubic = summary.cubic && residual < 1e-8;
    }
  }
  summary.width_trend = summary.execution
      && summary.max_velocity_mismatch[4] < summary.max_velocity_mismatch[3]
      && summary.max_velocity_mismatch[3] < summary.max_velocity_mismatch[2]
      && summary.max_relative_phase_rms[4] < summary.max_relative_phase_rms[3]
      && summary.max_relative_phase_rms[3] < summary.max_relative_phase_rms[2]
      && summary.max_field_cv[4] < summary.max_field_cv[3]
      && summary.max_field_cv[3] < summary.max_field_cv[2];

  if (!summary.coverage || !summary.execution || !summary.exact
      || !summary.coherence)
    summary.verdict = "MOBILE_DRESSING_STRUCTURE_FACTOR_EXECUTION_INVALID";
  else if (!summary.matter)
    summary.verdict = "MOBILE_MATTER_STRUCTURE_FACTOR_CLOSED_NEGATIVE";
  else if (summary.field && summary.mirror && summary.cubic
           && summary.width_trend)
    summary.verdict = "MOBILE_DRESSED_STRUCTURE_FACTOR_CONSTRUCTIVE";
  else
    summary.verdict = "MOBILE_CORE_FIELD_DRESSING_MIXED";
}

void write_v5(const DressingSummary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0655";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0655_mobile_dressing_structure_factor_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0655\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256_v5 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"exact_pass\": " << summary.exact << ",\n"
       << "  \"coherence_pass\": " << summary.coherence << ",\n"
       << "  \"matter_pass\": " << summary.matter << ",\n"
       << "  \"field_pass\": " << summary.field << ",\n"
       << "  \"mirror_pass\": " << summary.mirror << ",\n"
       << "  \"cubic_pass\": " << summary.cubic << ",\n"
       << "  \"width_trend_pass\": " << summary.width_trend << ",\n"
       << "  \"worst_action\": " << summary.worst_action << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
       << "  \"worst_strain\": " << summary.worst_strain << ",\n"
       << "  \"mirror_residual\": " << summary.mirror_residual << ",\n"
       << "  \"cubic_residual\": " << summary.cubic_residual << ",\n"
       << "  \"width2_velocity_mismatch\": " << summary.max_velocity_mismatch.at(2) << ",\n"
       << "  \"width3_velocity_mismatch\": " << summary.max_velocity_mismatch.at(3) << ",\n"
       << "  \"width4_velocity_mismatch\": " << summary.max_velocity_mismatch.at(4) << ",\n"
       << "  \"width2_relative_phase_rms\": " << summary.max_relative_phase_rms.at(2) << ",\n"
       << "  \"width3_relative_phase_rms\": " << summary.max_relative_phase_rms.at(3) << ",\n"
       << "  \"width4_relative_phase_rms\": " << summary.max_relative_phase_rms.at(4) << ",\n"
       << "  \"width2_field_cv\": " << summary.max_field_cv.at(2) << ",\n"
       << "  \"width3_field_cv\": " << summary.max_field_cv.at(3) << ",\n"
       << "  \"width4_field_cv\": " << summary.max_field_cv.at(4) << "\n}\n";

  std::ofstream arms(dir/"ftd_0655_mobile_dressing_structure_factor_arms_v1.csv");
  arms << "ftd_id,label,kind,family,width,speed,initialized,forward,reverse,exact,coherent,matter_pass,field_pass,center_velocity,matter_velocity,field_velocity,matter_mean,field_mean,matter_phase_rms,field_phase_rms,matter_cv,field_cv,relative_phase_rms,matter_center_mismatch,field_matter_mismatch,max_action,max_strain,recovery\n";
  for (const auto& result : summary.arms) {
    const auto& arm = result.dynamics;
    arms << std::setprecision(17) << "FTD-0655," << arm.spec.label << ','
         << arm.spec.kind << ',' << arm.spec.family << ',' << arm.spec.width
         << ',' << arm.spec.speed << ',' << arm.initialized << ',' << arm.forward
         << ',' << arm.reverse << ',' << arm.exact << ',' << arm.coherent << ','
         << result.matter_pass << ',' << result.field_pass << ','
         << result.center_velocity << ',' << result.matter.phase_velocity << ','
         << result.field.phase_velocity << ',' << result.matter.mean_amplitude
         << ',' << result.field.mean_amplitude << ',' << result.matter.phase_rms
         << ',' << result.field.phase_rms << ',' << result.matter.amplitude_cv
         << ',' << result.field.amplitude_cv << ',' << result.relative_phase_rms
         << ',' << result.matter_center_mismatch << ','
         << result.field_matter_mismatch << ',' << arm.maximum_action << ','
         << arm.maximum_relative_edge_strain << ',' << arm.recovery << '\n';
  }

  std::ofstream series(dir/"ftd_0655_mobile_dressing_structure_factor_series_v1.csv");
  series << "ftd_id,label,tick,matter_real,matter_imag,field_real,field_imag,center_x,center_y,center_z\n";
  for (const auto& result : summary.arms)
    for (const auto& sample : result.samples)
      series << std::setprecision(17) << "FTD-0655,"
             << result.dynamics.spec.label << ',' << sample.tick << ','
             << sample.matter.real() << ',' << sample.matter.imag() << ','
             << sample.field.real() << ',' << sample.field.imag() << ','
             << sample.center.x << ',' << sample.center.y << ','
             << sample.center.z << '\n';
}

}  // namespace

#ifndef FTD_0655_EMBEDDED
int main() {
  DressingSummary summary;
  const auto specs = specs_v5();
  constexpr std::size_t batch = 6;
  for (std::size_t start = 0; start < specs.size(); start += batch) {
    std::vector<std::future<DressingArm>> futures;
    const std::size_t end = std::min(specs.size(),start+batch);
    for (std::size_t i = start; i < end; ++i)
      futures.push_back(std::async(std::launch::async,
          [spec=specs[i]]() { return run_arm_v5(spec); }));
    for (std::size_t i = start; i < end; ++i) {
      summary.arms.push_back(futures[i-start].get());
      std::cout << "completed " << specs[i].label << std::endl;
    }
  }
  evaluate_v5(summary);
  write_v5(summary);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << protocol_sha256_v5 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "coverage=" << summary.coverage
            << " execution=" << summary.execution
            << " exact=" << summary.exact
            << " coherence=" << summary.coherence
            << " matter=" << summary.matter
            << " field=" << summary.field
            << " mirror=" << summary.mirror
            << " cubic=" << summary.cubic
            << " trend=" << summary.width_trend << '\n';
  for (const auto& arm : summary.arms)
    std::cout << arm.dynamics.spec.label
              << " vc=" << arm.center_velocity
              << " vm=" << arm.matter.phase_velocity
              << " vf=" << arm.field.phase_velocity
              << " mrms=" << arm.matter.phase_rms
              << " frms=" << arm.field.phase_rms
              << " mcv=" << arm.matter.amplitude_cv
              << " fcv=" << arm.field.amplitude_cv
              << " relative=" << arm.relative_phase_rms << '\n';
  return summary.verdict ==
      "MOBILE_DRESSING_STRUCTURE_FACTOR_EXECUTION_INVALID" ? 1 : 0;
}
#endif
