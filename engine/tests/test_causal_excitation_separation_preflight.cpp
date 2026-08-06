// Diagnostic-only initialization probe following execution-invalid FTD-0684.
#define FTD_0678_EMBEDDED
#include "test_localized_basin_relaxation.cpp"
#undef FTD_0678_EMBEDDED

#include "ftd/eft/component_aware_radial_field_profile.h"

namespace {

constexpr int probe_volume = 129;

ftd::eft::ConnectedMooreBlockState probe_reference() {
  const auto base = load_refined_state(0);
  auto geometry = base;
  geometry.electric = ftd::eft::MatchedFaceFlux(probe_volume);
  geometry.magnetic_half = ftd::eft::MatchedEdgeField(probe_volume);
  const Vec3 base_center = center(base);
  const Vec3 target_center{64.0, 64.0, 64.0};
  for (int particle = 0; particle < count; ++particle) {
    const Vec3 x = target_center
        + (position(base.constituents[particle]) - base_center);
    geometry.constituents[particle] = point_at_volume(x, probe_volume);
    geometry.constituents[particle].momentum = {};
  }
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  return dressed.valid ? dressed.state
                       : ftd::eft::ConnectedMooreBlockState{};
}

}  // namespace

int main() {
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  const auto base = load_refined_state(0);
  const auto analytic = analytic_at(
      "causal_excitation_separation_preflight", 0, base,
      normalization.mapped_field_work_coefficient, options);
  const auto modes = full_modes(analytic.hessian);
  const auto control = probe_reference();
  double nominal = 0.0;
  auto excited = volume_excitation(control, modes, +1, nominal);
  const double before_scale = maximum_momentum(excited);
  const double scale = before_scale > 0.0 ? 1.25e-7 / before_scale : 0.0;
  for (auto& constituent : excited.constituents)
    constituent.momentum *= scale;
  const auto basis = donor_modes(modes);
  const auto reservoir = ftd::eft::evaluate_connected_reservoir_decomposition(
      control, excited, basis, {6, 7},
      normalization.mapped_field_work_coefficient, options, 1e-10);
  const double omega = 0.5 * (modes.modes[6].omega + modes.modes[7].omega);
  const Vec3 origin{64.0, 64.0, 64.0};
  const auto basin = ftd::eft::observe_localized_basin(
      control, control, origin, 8, 48, omega,
      normalization.mapped_field_work_coefficient, options.wave_speed,
      ftd::M_INERTIAL, 1e-12);
  const auto profile = ftd::eft::observe_component_aware_radial_field_profile(
      control.electric, control.magnetic_half,
      control.electric, control.magnetic_half,
      origin, normalization.mapped_field_work_coefficient,
      options.wave_speed, 1e-12);
  const Vec3 measured_center = center(control);
  std::cout << std::setprecision(17)
            << "normalization=" << normalization.valid
            << " analytic=" << analytic.valid
            << " modes=" << modes.valid
            << " count=" << control.constituents.size()
            << " L=" << control.electric.L << '\n'
            << "center=" << measured_center.x << ',' << measured_center.y
            << ',' << measured_center.z
            << " center_residual=" << (measured_center - origin).mag() << '\n'
            << "profile_valid=" << profile.valid
            << " zero=" << profile.zero_profile
            << " basin_valid=" << basin.valid << '\n'
            << "excited_count=" << excited.constituents.size()
            << " p_before=" << before_scale
            << " p_after=" << maximum_momentum(excited)
            << " nominal=" << nominal << '\n'
            << "reservoir_valid=" << reservoir.valid
            << " orthonormal=" << reservoir.mode_orthonormality_residual
            << " target=" << reservoir.target_mode_energy
            << " total_mode=" << reservoir.total_mode_energy
            << " matter_residual=" << reservoir.matter_decomposition_residual
            << " field_residual=" << reservoir.field_decomposition_residual
            << " complete_residual="
            << reservoir.complete_decomposition_residual << '\n';
  return 0;
}
