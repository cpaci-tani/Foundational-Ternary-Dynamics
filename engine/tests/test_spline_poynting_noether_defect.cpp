// FTD-0619: spline-Poynting versus fixed-lattice Noether defect.
#define FTD0618_NO_MAIN
#include "test_closed_symmetry_balanced_gait.cpp"
#include "ftd/eft/spline_poynting_momentum.h"

namespace {

constexpr char protocol_sha256_0619[] =
    "F2E97844E14B77C152E986CD2CA317337FEE04E2367F73AD4A73FD76FE61E107";
constexpr char parent_sha256_0619[] =
    "5F04E64DFD7CBFD10CE3AC779361C4124654C817320DFC81E6D5A482889F54D3";
constexpr int source_ticks_0619 = 256;
constexpr int source_mode_0619 = 2;
constexpr double source_amplitude_0619 = 0.02;

double max_component_0619(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y), std::abs(value.z)});
}

double vec_difference_0619(const Vec3& lhs, const Vec3& rhs) {
  return max_component_0619(lhs - rhs);
}

Vec3 direct_spline_integral_0619(
    const ftd::eft::MatchedFaceFlux& electric,
    const ftd::eft::MatchedEdgeField& magnetic) {
  constexpr std::array<long double, 3> nodes{{
      -0.774596669241483377035853079956L,
       0.0L,
       0.774596669241483377035853079956L}};
  constexpr std::array<long double, 3> weights{{
      0.555555555555555555555555555556L,
      0.888888888888888888888888888889L,
      0.555555555555555555555555555556L}};
  long double px = 0.0L, py = 0.0L, pz = 0.0L;
  const int Lq = electric.L;
  for (int x = 0; x < Lq; ++x) {
    for (int y = 0; y < Lq; ++y) {
      for (int z = 0; z < Lq; ++z) {
        for (int hx = 0; hx < 2; ++hx) {
          for (int hy = 0; hy < 2; ++hy) {
            for (int hz = 0; hz < 2; ++hz) {
              for (std::size_t ix = 0; ix < nodes.size(); ++ix) {
                for (std::size_t iy = 0; iy < nodes.size(); ++iy) {
                  for (std::size_t iz = 0; iz < nodes.size(); ++iz) {
                    const Vec3 position{
                        x + 0.25 * (2 * hx + 1)
                            + 0.25 * static_cast<double>(nodes[ix]),
                        y + 0.25 * (2 * hy + 1)
                            + 0.25 * static_cast<double>(nodes[iy]),
                        z + 0.25 * (2 * hz + 1)
                            + 0.25 * static_cast<double>(nodes[iz])};
                    const Vec3 e = ftd::eft::interpolate_quadratic_face_field(
                        electric, position);
                    const Vec3 b = ftd::eft::interpolate_quadratic_edge_field(
                        magnetic, position);
                    const Vec3 cross = Vec3::cross(e, b);
                    const long double weight = 0.015625L
                        * weights[ix] * weights[iy] * weights[iz];
                    px += weight * cross.x;
                    py += weight * cross.y;
                    pz += weight * cross.z;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return {static_cast<double>(px), static_cast<double>(py),
          static_cast<double>(pz)};
}

double validate_overlap_0619() {
  constexpr int Lq = 7;
  ftd::eft::MatchedFaceFlux electric(Lq);
  ftd::eft::MatchedEdgeField magnetic_half(Lq);
  for (int x = 0; x < Lq; ++x) {
    for (int y = 0; y < Lq; ++y) {
      for (int z = 0; z < Lq; ++z) {
        const int i = electric.index(x, y, z);
        const double phase = 2.0 * ftd::PI
            * static_cast<double>(x + 2 * y + 3 * z) / Lq;
        electric.x[static_cast<std::size_t>(i)] = 0.011 * std::sin(phase);
        electric.y[static_cast<std::size_t>(i)] = 0.013 * std::cos(phase);
        electric.z[static_cast<std::size_t>(i)] = 0.017 * std::sin(2.0 * phase);
        magnetic_half.x[static_cast<std::size_t>(i)] = 0.019 * std::cos(phase);
        magnetic_half.y[static_cast<std::size_t>(i)] = 0.023 * std::sin(phase);
        magnetic_half.z[static_cast<std::size_t>(i)] = 0.029 * std::cos(2.0 * phase);
      }
    }
  }
  const auto magnetic = ftd::eft::matched_integer_time_magnetic(
      electric, magnetic_half, ftd::C_SPEED, 1.0);
  const Vec3 overlap = ftd::eft::integrate_quadratic_spline_cross(
      electric, magnetic);
  const Vec3 direct = direct_spline_integral_0619(electric, magnetic);
  return vec_difference_0619(overlap, direct);
}

void seed_directed_mode_0619(ftd::eft::MatchedFaceFlux& electric,
                             ftd::eft::MatchedEdgeField& magnetic,
                             int axis, int direction) {
  const int Lq = electric.L;
  const double k = 2.0 * ftd::PI * source_mode_0619 / Lq;
  const int electric_axis = (axis + 1) % 3;
  const int magnetic_axis = (axis + 2) % 3;
  for (int x = 0; x < Lq; ++x) {
    for (int y = 0; y < Lq; ++y) {
      for (int z = 0; z < Lq; ++z) {
        const int coordinate = axis == 0 ? x : (axis == 1 ? y : z);
        const int i = electric.index(x, y, z);
        const double e = source_amplitude_0619 * std::cos(
            k * coordinate - 0.5 * k);
        const double b = direction * source_amplitude_0619
            * std::cos(k * coordinate);
        if (electric_axis == 0) electric.x[static_cast<std::size_t>(i)] = e;
        if (electric_axis == 1) electric.y[static_cast<std::size_t>(i)] = e;
        if (electric_axis == 2) electric.z[static_cast<std::size_t>(i)] = e;
        if (magnetic_axis == 0) magnetic.x[static_cast<std::size_t>(i)] = b;
        if (magnetic_axis == 1) magnetic.y[static_cast<std::size_t>(i)] = b;
        if (magnetic_axis == 2) magnetic.z[static_cast<std::size_t>(i)] = b;
      }
    }
  }
}

struct SourceArm0619 {
  int L = 0;
  int axis = 0;
  int direction = 0;
  bool valid = false;
  Vec3 selected_initial{};
  Vec3 spline_initial{};
  double selected_absolute_drift = 0.0;
  double selected_relative_drift = 0.0;
  double spline_absolute_drift = 0.0;
  double spline_relative_drift = 0.0;
  double spline_transverse = 0.0;
};

SourceArm0619 run_source_arm_0619(int Lq, int axis, int direction,
                                  double beta) {
  SourceArm0619 result;
  result.L = Lq;
  result.axis = axis;
  result.direction = direction;
  ftd::eft::MatchedFaceFlux electric(Lq);
  ftd::eft::MatchedEdgeField magnetic(Lq);
  seed_directed_mode_0619(electric, magnetic, axis, direction);
  result.selected_initial = ftd::eft::matched_local_translation_momentum(
      electric, magnetic) * beta;
  const auto spline0 = ftd::eft::measure_spline_poynting_momentum(
      electric, magnetic, ftd::C_SPEED, 1.0, beta);
  result.spline_initial = spline0.momentum;
  result.spline_transverse = std::hypot(
      axis == 0 ? result.spline_initial.y : result.spline_initial.x,
      axis == 2 ? result.spline_initial.y : result.spline_initial.z);
  for (int tick = 0; tick < source_ticks_0619; ++tick) {
    const auto curl_e = ftd::eft::matched_curl_adjoint(electric);
    for (std::size_t i = 0; i < magnetic.x.size(); ++i) {
      magnetic.x[i] -= ftd::C_SPEED * curl_e.x[i];
      magnetic.y[i] -= ftd::C_SPEED * curl_e.y[i];
      magnetic.z[i] -= ftd::C_SPEED * curl_e.z[i];
    }
    const auto curl_b = ftd::eft::matched_curl(magnetic);
    for (std::size_t i = 0; i < electric.x.size(); ++i) {
      electric.x[i] += ftd::C_SPEED * curl_b.x[i];
      electric.y[i] += ftd::C_SPEED * curl_b.y[i];
      electric.z[i] += ftd::C_SPEED * curl_b.z[i];
    }
    const Vec3 selected = ftd::eft::matched_local_translation_momentum(
        electric, magnetic) * beta;
    const auto spline = ftd::eft::measure_spline_poynting_momentum(
        electric, magnetic, ftd::C_SPEED, 1.0, beta);
    result.selected_absolute_drift = std::max(
        result.selected_absolute_drift,
        (selected - result.selected_initial).mag());
    result.spline_absolute_drift = std::max(
        result.spline_absolute_drift,
        (spline.momentum - result.spline_initial).mag());
    result.valid = spline.valid;
  }
  result.selected_relative_drift = result.selected_absolute_drift
      / std::max(1e-30, result.selected_initial.mag());
  result.spline_relative_drift = result.spline_absolute_drift
      / std::max(1e-30, result.spline_initial.mag());
  result.valid = result.valid && result.selected_initial.mag() > 1e-8
      && result.spline_initial.mag() > 1e-8
      && std::isfinite(result.selected_relative_drift)
      && std::isfinite(result.spline_relative_drift);
  return result;
}

struct ChannelTick0619 {
  int tick = 0;
  Vec3 matter_delta{};
  Vec3 electric_impulse{};
  Vec3 magnetic_impulse{};
  Vec3 binding_impulse{};
  Vec3 selected_delta{};
  Vec3 spline_delta{};
  Vec3 selected_defect{};
  Vec3 spline_defect{};
  double matter_equation_residual = 0.0;
  double binding_residual = 0.0;
  double cumulative_selected = 0.0;
  double cumulative_spline = 0.0;
};

struct ChannelArm0619 {
  int sign = 0;
  bool initialized = false;
  bool complete = false;
  bool algebraic_pass = false;
  double maximum_matter_equation_residual = INFINITY;
  double maximum_binding_residual = INFINITY;
  double maximum_selected_step_defect = INFINITY;
  double maximum_spline_step_defect = INFINITY;
  double maximum_cumulative_selected = INFINITY;
  double maximum_cumulative_spline = INFINITY;
  Vec3 cumulative_electric{};
  Vec3 cumulative_magnetic{};
  Vec3 cumulative_binding{};
  Vec3 cumulative_selected_field{};
  Vec3 cumulative_spline_field{};
  Vec3 final_matter_delta{};
  std::vector<ChannelTick0619> ticks;
};

ChannelArm0619 run_channel_arm_0619(
    int sign, const BalancedRestContext& rest,
    const ftd::eft::ClosedNeutralPairOptions& options) {
  ChannelArm0619 arm;
  arm.sign = sign;
  const auto fixture = make_pair_fixture_0618(sign, rest);
  arm.initialized = fixture.valid;
  if (!fixture.valid) return arm;
  auto current = fixture.state;
  const Vec3 matter0 = core_momentum_0618(current, 0)
      + core_momentum_0618(current, 3);
  const double beta = ftd::eft::measure_face_flux_normalization()
      .mapped_field_work_coefficient;
  const Vec3 selected0 = ftd::eft::matched_local_translation_momentum(
      current.electric, current.magnetic_half) * beta;
  const auto spline0 = ftd::eft::measure_spline_poynting_momentum(
      current.electric, current.magnetic_half,
      options.wave_speed, options.dt, beta);
  if (!spline0.valid) return arm;
  arm.maximum_matter_equation_residual = 0.0;
  arm.maximum_binding_residual = 0.0;
  arm.maximum_selected_step_defect = 0.0;
  arm.maximum_spline_step_defect = 0.0;
  arm.maximum_cumulative_selected = 0.0;
  arm.maximum_cumulative_spline = 0.0;
  arm.ticks.reserve(balanced_ticks);
  for (int tick = 0; tick < balanced_ticks; ++tick) {
    const auto step = ftd::eft::solve_closed_neutral_pair_forward(
        current, options);
    if (!step.valid) break;
    ChannelTick0619 sample;
    sample.tick = tick + 1;
    sample.matter_delta = step.matter_momentum_after
        - step.matter_momentum_before;
    for (std::size_t a = 0; a < step.electric_impulses.size(); ++a) {
      sample.electric_impulse += step.electric_impulses[a];
      sample.magnetic_impulse += step.magnetic_impulses[a];
      sample.binding_impulse += step.binding_impulses[a];
    }
    sample.matter_equation_residual = (
        sample.matter_delta - sample.electric_impulse
        - sample.magnetic_impulse - sample.binding_impulse).mag();
    sample.binding_residual = sample.binding_impulse.mag();
    sample.selected_delta = step.field_pseudomomentum_after
        - step.field_pseudomomentum_before;
    const auto spline_before = ftd::eft::measure_spline_poynting_momentum(
        step.earlier.electric, step.earlier.magnetic_half,
        options.wave_speed, options.dt, beta);
    const auto spline_after = ftd::eft::measure_spline_poynting_momentum(
        step.later.electric, step.later.magnetic_half,
        options.wave_speed, options.dt, beta);
    if (!spline_before.valid || !spline_after.valid) break;
    sample.spline_delta = spline_after.momentum - spline_before.momentum;
    sample.selected_defect = sample.matter_delta + sample.selected_delta;
    sample.spline_defect = sample.matter_delta + sample.spline_delta;
    arm.cumulative_electric += sample.electric_impulse;
    arm.cumulative_magnetic += sample.magnetic_impulse;
    arm.cumulative_binding += sample.binding_impulse;
    arm.cumulative_selected_field += sample.selected_delta;
    arm.cumulative_spline_field += sample.spline_delta;
    const Vec3 matter = step.matter_momentum_after - matter0;
    sample.cumulative_selected = (
        matter + step.field_pseudomomentum_after - selected0).mag();
    sample.cumulative_spline = (
        matter + spline_after.momentum - spline0.momentum).mag();
    arm.maximum_matter_equation_residual = std::max(
        arm.maximum_matter_equation_residual,
        sample.matter_equation_residual);
    arm.maximum_binding_residual = std::max(
        arm.maximum_binding_residual, sample.binding_residual);
    arm.maximum_selected_step_defect = std::max(
        arm.maximum_selected_step_defect, sample.selected_defect.mag());
    arm.maximum_spline_step_defect = std::max(
        arm.maximum_spline_step_defect, sample.spline_defect.mag());
    arm.maximum_cumulative_selected = std::max(
        arm.maximum_cumulative_selected, sample.cumulative_selected);
    arm.maximum_cumulative_spline = std::max(
        arm.maximum_cumulative_spline, sample.cumulative_spline);
    arm.ticks.push_back(sample);
    current = step.later;
  }
  arm.complete = arm.ticks.size() == balanced_ticks;
  arm.final_matter_delta = core_momentum_0618(current, 0)
      + core_momentum_0618(current, 3) - matter0;
  arm.algebraic_pass = arm.complete
      && arm.maximum_matter_equation_residual <= 1e-12
      && arm.maximum_binding_residual <= 1e-12;
  return arm;
}

struct Summary0619 {
  bool parent_fingerprint = false;
  bool overlap_pass = false;
  bool source_coverage = false;
  bool source_spline_pass = false;
  bool channel_coverage = false;
  bool selected_closes = false;
  bool spline_closes = false;
  double overlap_residual = INFINITY;
  double source_covariance_residual = INFINITY;
  double maximum_selected_source_relative_drift = INFINITY;
  double maximum_spline_source_relative_drift = INFINITY;
  double maximum_cumulative_selected = INFINITY;
  double maximum_cumulative_spline = INFINITY;
  double active_sign_mirror_residual = INFINITY;
  std::vector<SourceArm0619> sources;
  std::vector<ChannelArm0619> channels;
  std::string verdict;
};

void evaluate_summary_0619(Summary0619& summary) {
  summary.source_coverage = summary.sources.size() == 12
      && std::all_of(summary.sources.begin(), summary.sources.end(),
          [](const SourceArm0619& arm) { return arm.valid; });
  summary.maximum_selected_source_relative_drift = 0.0;
  summary.maximum_spline_source_relative_drift = 0.0;
  summary.source_covariance_residual = 0.0;
  for (const auto& arm : summary.sources) {
    summary.maximum_selected_source_relative_drift = std::max(
        summary.maximum_selected_source_relative_drift,
        arm.selected_relative_drift);
    summary.maximum_spline_source_relative_drift = std::max(
        summary.maximum_spline_source_relative_drift,
        arm.spline_relative_drift);
    summary.source_covariance_residual = std::max(
        summary.source_covariance_residual, arm.spline_transverse);
  }
  for (int Lq : {16, 17}) {
    std::vector<double> signed_axial;
    for (const auto& arm : summary.sources) {
      if (arm.L != Lq) continue;
      const double axial = arm.axis == 0 ? arm.spline_initial.x
          : (arm.axis == 1 ? arm.spline_initial.y : arm.spline_initial.z);
      signed_axial.push_back(arm.direction * axial);
    }
    if (signed_axial.size() == 6) {
      const auto [lo, hi] = std::minmax_element(
          signed_axial.begin(), signed_axial.end());
      summary.source_covariance_residual = std::max(
          summary.source_covariance_residual, *hi - *lo);
    } else {
      summary.source_covariance_residual = INFINITY;
    }
  }
  summary.source_spline_pass = summary.source_coverage
      && summary.maximum_spline_source_relative_drift <= 1e-10
      && std::all_of(summary.sources.begin(), summary.sources.end(),
          [](const SourceArm0619& arm) {
            return arm.spline_absolute_drift <= 1e-10;
          })
      && summary.source_covariance_residual <= 1e-12;
  summary.channel_coverage = summary.channels.size() == 3
      && std::all_of(summary.channels.begin(), summary.channels.end(),
          [](const ChannelArm0619& arm) { return arm.algebraic_pass; });
  summary.maximum_cumulative_selected = 0.0;
  summary.maximum_cumulative_spline = 0.0;
  for (const auto& arm : summary.channels) {
    summary.maximum_cumulative_selected = std::max(
        summary.maximum_cumulative_selected,
        arm.maximum_cumulative_selected);
    summary.maximum_cumulative_spline = std::max(
        summary.maximum_cumulative_spline,
        arm.maximum_cumulative_spline);
  }
  summary.selected_closes = summary.channel_coverage
      && summary.maximum_cumulative_selected <= 1e-10;
  summary.spline_closes = summary.source_spline_pass
      && summary.channel_coverage
      && summary.maximum_cumulative_spline <= 1e-10;
  const auto plus = std::find_if(summary.channels.begin(),
      summary.channels.end(), [](const ChannelArm0619& arm) {
        return arm.sign == +1;
      });
  const auto minus = std::find_if(summary.channels.begin(),
      summary.channels.end(), [](const ChannelArm0619& arm) {
        return arm.sign == -1;
      });
  summary.active_sign_mirror_residual = 0.0;
  if (plus == summary.channels.end() || minus == summary.channels.end()
      || plus->ticks.size() != minus->ticks.size()) {
    summary.active_sign_mirror_residual = INFINITY;
  } else {
    for (std::size_t i = 0; i < plus->ticks.size(); ++i)
      summary.active_sign_mirror_residual = std::max(
          summary.active_sign_mirror_residual,
          (plus->ticks[i].spline_defect
           + minus->ticks[i].spline_defect).mag());
  }
}

void write_record_0619(const Summary0619& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0619";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0619_spline_poynting_noether_defect_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0619\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256_0619 << "\",\n"
       << "  \"parent_result_sha256\": \"" << parent_sha256_0619 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_fingerprint_pass\": " << summary.parent_fingerprint << ",\n"
       << "  \"overlap_pass\": " << summary.overlap_pass << ",\n"
       << "  \"source_coverage\": " << summary.source_coverage << ",\n"
       << "  \"source_spline_pass\": " << summary.source_spline_pass << ",\n"
       << "  \"channel_coverage\": " << summary.channel_coverage << ",\n"
       << "  \"selected_closes\": " << summary.selected_closes << ",\n"
       << "  \"spline_closes\": " << summary.spline_closes << ",\n"
       << "  \"overlap_residual\": " << json_number(summary.overlap_residual) << ",\n"
       << "  \"source_covariance_residual\": "
       << json_number(summary.source_covariance_residual) << ",\n"
       << "  \"maximum_selected_source_relative_drift\": "
       << json_number(summary.maximum_selected_source_relative_drift) << ",\n"
       << "  \"maximum_spline_source_relative_drift\": "
       << json_number(summary.maximum_spline_source_relative_drift) << ",\n"
       << "  \"maximum_cumulative_selected\": "
       << json_number(summary.maximum_cumulative_selected) << ",\n"
       << "  \"maximum_cumulative_spline\": "
       << json_number(summary.maximum_cumulative_spline) << ",\n"
       << "  \"active_sign_mirror_residual\": "
       << json_number(summary.active_sign_mirror_residual) << ",\n"
       << "  \"sources\": [\n";
  for (std::size_t i = 0; i < summary.sources.size(); ++i) {
    const auto& arm = summary.sources[i];
    json << "    {\"L\": " << arm.L << ", \"axis\": " << arm.axis
         << ", \"direction\": " << arm.direction
         << ", \"valid\": " << arm.valid
         << ", \"selected_initial\": [" << arm.selected_initial.x << ','
         << arm.selected_initial.y << ',' << arm.selected_initial.z << ']'
         << ", \"spline_initial\": [" << arm.spline_initial.x << ','
         << arm.spline_initial.y << ',' << arm.spline_initial.z << ']'
         << ", \"selected_absolute_drift\": "
         << arm.selected_absolute_drift
         << ", \"selected_relative_drift\": "
         << arm.selected_relative_drift
         << ", \"spline_absolute_drift\": " << arm.spline_absolute_drift
         << ", \"spline_relative_drift\": " << arm.spline_relative_drift
         << ", \"spline_transverse\": " << arm.spline_transverse << '}'
         << (i + 1 == summary.sources.size() ? "\n" : ",\n");
  }
  json << "  ],\n  \"channels\": [\n";
  for (std::size_t i = 0; i < summary.channels.size(); ++i) {
    const auto& arm = summary.channels[i];
    json << "    {\"sign\": " << arm.sign
         << ", \"initialized\": " << arm.initialized
         << ", \"complete\": " << arm.complete
         << ", \"algebraic_pass\": " << arm.algebraic_pass
         << ", \"maximum_matter_equation_residual\": "
         << arm.maximum_matter_equation_residual
         << ", \"maximum_binding_residual\": "
         << arm.maximum_binding_residual
         << ", \"maximum_selected_step_defect\": "
         << arm.maximum_selected_step_defect
         << ", \"maximum_spline_step_defect\": "
         << arm.maximum_spline_step_defect
         << ", \"maximum_cumulative_selected\": "
         << arm.maximum_cumulative_selected
         << ", \"maximum_cumulative_spline\": "
         << arm.maximum_cumulative_spline
         << ", \"cumulative_electric\": [" << arm.cumulative_electric.x
         << ',' << arm.cumulative_electric.y << ','
         << arm.cumulative_electric.z << ']'
         << ", \"cumulative_magnetic\": [" << arm.cumulative_magnetic.x
         << ',' << arm.cumulative_magnetic.y << ','
         << arm.cumulative_magnetic.z << ']'
         << ", \"cumulative_binding\": [" << arm.cumulative_binding.x
         << ',' << arm.cumulative_binding.y << ','
         << arm.cumulative_binding.z << ']'
         << ", \"cumulative_selected_field\": ["
         << arm.cumulative_selected_field.x << ','
         << arm.cumulative_selected_field.y << ','
         << arm.cumulative_selected_field.z << ']'
         << ", \"cumulative_spline_field\": ["
         << arm.cumulative_spline_field.x << ','
         << arm.cumulative_spline_field.y << ','
         << arm.cumulative_spline_field.z << ']'
         << ", \"final_matter_delta\": [" << arm.final_matter_delta.x
         << ',' << arm.final_matter_delta.y << ','
         << arm.final_matter_delta.z << "]}"
         << (i + 1 == summary.channels.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream source_csv(dir / "ftd_0619_source_free_v1.csv");
  source_csv << "ftd_id,L,axis,direction,valid,selected_x,selected_y,selected_z,"
      "spline_x,spline_y,spline_z,selected_absolute_drift,"
      "selected_relative_drift,spline_absolute_drift,spline_relative_drift,"
      "spline_transverse\n";
  for (const auto& arm : summary.sources)
    source_csv << std::setprecision(17) << "FTD-0619," << arm.L << ','
        << arm.axis << ',' << arm.direction << ',' << arm.valid << ','
        << arm.selected_initial.x << ',' << arm.selected_initial.y << ','
        << arm.selected_initial.z << ',' << arm.spline_initial.x << ','
        << arm.spline_initial.y << ',' << arm.spline_initial.z << ','
        << arm.selected_absolute_drift << ',' << arm.selected_relative_drift
        << ',' << arm.spline_absolute_drift << ',' << arm.spline_relative_drift
        << ',' << arm.spline_transverse << '\n';

  std::ofstream ticks(dir / "ftd_0619_channels_v1.csv");
  ticks << "ftd_id,sign,tick,dpx,dpy,dpz,iex,iey,iez,ibx,iby,ibz,"
      "ibindx,ibindy,ibindz,dselx,dsely,dselz,dsplx,dsply,dsplz,"
      "rselx,rsely,rselz,rsplx,rsply,rsplz,matter_residual,"
      "binding_residual,cumulative_selected,cumulative_spline\n";
  for (const auto& arm : summary.channels) {
    for (const auto& tick : arm.ticks) {
      ticks << std::setprecision(17) << "FTD-0619," << arm.sign << ','
          << tick.tick << ',' << tick.matter_delta.x << ','
          << tick.matter_delta.y << ',' << tick.matter_delta.z << ','
          << tick.electric_impulse.x << ',' << tick.electric_impulse.y << ','
          << tick.electric_impulse.z << ',' << tick.magnetic_impulse.x << ','
          << tick.magnetic_impulse.y << ',' << tick.magnetic_impulse.z << ','
          << tick.binding_impulse.x << ',' << tick.binding_impulse.y << ','
          << tick.binding_impulse.z << ',' << tick.selected_delta.x << ','
          << tick.selected_delta.y << ',' << tick.selected_delta.z << ','
          << tick.spline_delta.x << ',' << tick.spline_delta.y << ','
          << tick.spline_delta.z << ',' << tick.selected_defect.x << ','
          << tick.selected_defect.y << ',' << tick.selected_defect.z << ','
          << tick.spline_defect.x << ',' << tick.spline_defect.y << ','
          << tick.spline_defect.z << ',' << tick.matter_equation_residual << ','
          << tick.binding_residual << ',' << tick.cumulative_selected << ','
          << tick.cumulative_spline << '\n';
    }
  }
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  Summary0619 summary;
  const auto parent_path = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0618"
      / "ftd_0618_closed_symmetry_balanced_gait_v1.json";
  std::ifstream parent(parent_path, std::ios::binary);
  std::string bytes((std::istreambuf_iterator<char>(parent)),
                    std::istreambuf_iterator<char>());
  summary.parent_fingerprint =
      bytes.find("\"ftd_id\": \"FTD-0618\"") != std::string::npos
      && bytes.find("SYMMETRY_BALANCED_GAIT_KINEMATIC_MOMENTUM_OPEN")
          != std::string::npos
      && bytes.find("\"momentum_pass\": false") != std::string::npos;
  summary.overlap_residual = validate_overlap_0619();
  summary.overlap_pass = summary.overlap_residual <= 1e-12;

  const auto normalization = ftd::eft::measure_face_flux_normalization();
  if (normalization.valid) {
    for (int Lq : {16, 17})
      for (int axis = 0; axis < 3; ++axis)
        for (int direction : {+1, -1})
          summary.sources.push_back(run_source_arm_0619(
              Lq, axis, direction,
              normalization.mapped_field_work_coefficient));
  }

  ftd::eft::ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  options.allow_shared_anchor_chart = true;
  const BalancedRestContext rest = summary.parent_fingerprint
      ? make_balanced_rest_context_0618(options) : BalancedRestContext{};
  if (rest.valid)
    for (int sign : {0, +1, -1})
      summary.channels.push_back(run_channel_arm_0619(sign, rest, options));
  evaluate_summary_0619(summary);

  const bool protocol = summary.parent_fingerprint && normalization.valid
      && summary.overlap_pass && summary.source_coverage
      && summary.channel_coverage && summary.active_sign_mirror_residual <= 1e-10;
  if (!protocol)
    summary.verdict = "MOMENTUM_CHANNEL_DISCRIMINATOR_NUMERICALLY_UNRESOLVED";
  else if (summary.spline_closes)
    summary.verdict = "SPLINE_POYNTING_CLOSES_BALANCED_GAIT";
  else if (summary.source_spline_pass)
    summary.verdict = "CONTINUOUS_TRANSLATION_DEFECT_MEASURED";
  else
    summary.verdict = "SPLINE_POYNTING_NOT_CONSERVED";
  write_record_0619(summary);

  std::cout << "protocol_sha256=" << protocol_sha256_0619 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "overlap=" << summary.overlap_residual
            << " source_selected_rel="
            << summary.maximum_selected_source_relative_drift
            << " source_spline_rel="
            << summary.maximum_spline_source_relative_drift
            << " source_covariance=" << summary.source_covariance_residual
            << " cumulative_selected="
            << summary.maximum_cumulative_selected
            << " cumulative_spline=" << summary.maximum_cumulative_spline
            << " mirror=" << summary.active_sign_mirror_residual << '\n';
  for (const auto& arm : summary.channels)
    std::cout << "sign=" << arm.sign
              << " electric=(" << arm.cumulative_electric.x << ','
              << arm.cumulative_electric.y << ','
              << arm.cumulative_electric.z << ')'
              << " magnetic=(" << arm.cumulative_magnetic.x << ','
              << arm.cumulative_magnetic.y << ','
              << arm.cumulative_magnetic.z << ')'
              << " matter=(" << arm.final_matter_delta.x << ','
              << arm.final_matter_delta.y << ','
              << arm.final_matter_delta.z << ')'
              << " selected=" << arm.maximum_cumulative_selected
              << " spline=" << arm.maximum_cumulative_spline << '\n';
  return summary.channels.size() == 3 ? 0 : 1;
}

