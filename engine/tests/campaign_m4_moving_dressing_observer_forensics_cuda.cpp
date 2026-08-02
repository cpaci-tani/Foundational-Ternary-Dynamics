/** FTD-0762: outcome-aware CUDA forensics for the FTD-0761 observer failure. */

#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/cuda_state_only_support_ladder.h"
#include "ftd/eft/support_invariant_matter_predicate.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace ftd;
using namespace ftd::eft;

constexpr char kProtocolSha256[] =
    "880293A2DC1F129637D1D1C28D8C0D9AE5FA3AC29D76042348CFE09ABB9E5B46";
constexpr int kRegisteredVolume = 321;
constexpr int kFormationTick = 160;
constexpr int kForensicTicks = 64;
constexpr double kBoost = 0.015;
constexpr double kGate = 1e-12;
constexpr std::array<int, 3> kSupports{{4, 6, 8}};

struct ForensicDirection {
  int x = 0;
  int y = 0;
  int z = 0;
  const char* label = "";
};

bool select_direction(const std::string& slug, ForensicDirection& direction) {
  if (slug == "face") { direction = {0, 0, 1, "0_0_1"}; return true; }
  if (slug == "edge") { direction = {0, 1, -1, "0_1_-1"}; return true; }
  if (slug == "body") { direction = {1, 1, 1, "1_1_1"}; return true; }
  return false;
}

Vec3 direction_unit(const ForensicDirection& direction) {
  const Vec3 value{static_cast<double>(direction.x),
                   static_cast<double>(direction.y),
                   static_cast<double>(direction.z)};
  return value * (1.0 / value.mag());
}

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double fractional_center_norm(const Vec3& center) {
  const Vec3 nearest{static_cast<double>(std::llround(center.x)),
                     static_cast<double>(std::llround(center.y)),
                     static_cast<double>(std::llround(center.z))};
  return (center - nearest).mag();
}

int forensic_wrap(int value, int L) {
  value %= L;
  return value < 0 ? value + L : value;
}

MatchedMatterPoint point_at(const Vec3& position, const Vec3& momentum, int L) {
  MatchedMatterPoint point;
  const long long ax = std::llround(position.x);
  const long long ay = std::llround(position.y);
  const long long az = std::llround(position.z);
  point.anchor = {forensic_wrap(static_cast<int>(ax), L),
                  forensic_wrap(static_cast<int>(ay), L),
                  forensic_wrap(static_cast<int>(az), L)};
  point.remainder = {position.x - static_cast<double>(ax),
                     position.y - static_cast<double>(ay),
                     position.z - static_cast<double>(az)};
  point.momentum = momentum;
  return point;
}

Vec3 forensic_position(const MatchedMatterPoint& point) {
  return {point.anchor.x + point.remainder.x,
          point.anchor.y + point.remainder.y,
          point.anchor.z + point.remainder.z};
}

ConnectedMooreBlockState make_parent_geometry(
    int L, const ForensicDirection& direction) {
  ConnectedMooreBlockState state(L);
  const Vec3 center{static_cast<double>(L / 2),
                    static_cast<double>(L / 2),
                    static_cast<double>(L / 2)};
  const Vec3 unit = direction_unit(direction);
  state.constituents.push_back(point_at(
      center - unit * 0.65, unit * 0.0120, L));
  state.constituents.push_back(point_at(
      center + unit * 0.65, unit * (-0.0120), L));
  state.charges = {+1, -1};
  return state;
}

ConnectedMooreBlockOptions forensic_options() {
  ConnectedMooreBlockOptions options;
  options.dt = 0.25;
  options.binding_law = ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth = 0.01;
  options.compact_pair_cutoff_distance_squared = 1.5;
  options.allow_shared_anchor_chart = true;
  options.gate_tolerance = 1e-10;
  options.solve_tolerance = 2e-14;
  options.max_iterations = 384;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;
  return options;
}

Vec3 object_center(const ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents)
    result += forensic_position(point);
  return result * (1.0 / static_cast<double>(state.constituents.size()));
}

struct ForensicStep {
  bool valid = false;
  bool common = false;
};

class ForensicCudaStepper {
 public:
  ForensicCudaStepper(ConnectedMooreBlockState initial,
                      ConnectedMooreBlockOptions options,
                      double interaction_scale)
      : state_(std::move(initial)), options_(std::move(options)),
        interaction_scale_(interaction_scale), pipeline_(state_.electric.L),
        prepared_magnetic_(state_.electric.L),
        prepared_electric_(state_.electric.L) {
    const double center = static_cast<double>(state_.electric.L / 2);
    fixed_center_ = {center, center, center};
    options_.defer_volume_diagnostics = true;
    valid_ = pipeline_.valid()
        && pipeline_.upload(state_.electric, state_.magnetic_half);
  }

  bool valid() const { return valid_; }
  ConnectedMooreBlockState release_state() { return std::move(state_); }

  ForensicStep advance() {
    ForensicStep result;
    if (!valid_) return result;
    const double lambda = options_.wave_speed * options_.dt;
    if (!pipeline_.prepare_forward(lambda)
        || !pipeline_.download_prepared(
            prepared_magnetic_, prepared_electric_)) {
      valid_ = false;
      return result;
    }
    auto step = solve_connected_moore_block_forward_prepared(
        state_, std::move(prepared_magnetic_), std::move(prepared_electric_),
        options_, &cache_);
    if (!step.volume_diagnostics_pending
        || !pipeline_.apply_ordered_sparse_current(
            step.segments, options_.polarity_scale)) {
      valid_ = false;
      return result;
    }
    const auto profile = pipeline_.observe_deterministic(
        lambda, fixed_center_, {8}, 1e-10);
    if (!profile.valid) {
      valid_ = false;
      return result;
    }
    const auto diagnostics = pipeline_.diagnose_common_action(
        step.segments, options_.polarity_scale, interaction_scale_,
        options_.wave_speed, options_.dt, 1e-10);
    step = complete_connected_moore_block_volume_diagnostics(
        std::move(step), diagnostics, options_);
    result.valid = step.valid;
    result.common = step.common_action_gates_pass;
    state_ = std::move(step.later);
    if (!pipeline_.advance()) valid_ = false;
    valid_ = valid_ && result.valid;
    return result;
  }

 private:
  ConnectedMooreBlockState state_;
  ConnectedMooreBlockOptions options_;
  double interaction_scale_ = 0.0;
  Vec3 fixed_center_{};
  CudaMatchedFieldPipeline pipeline_;
  MatchedEdgeField prepared_magnetic_;
  MatchedFaceFlux prepared_electric_;
  ConnectedMooreBlockSolveCache cache_;
  bool valid_ = false;
};

struct ParentState {
  bool valid = false;
  ConnectedMooreBlockState state;
  explicit ParentState(int L) : state(L) {}
};

ParentState build_parent(int L, const ForensicDirection& direction,
                         const ConnectedMooreBlockOptions& options,
                         double interaction_scale) {
  ParentState result(L);
  auto preparation = prepare_finite_support_derived_compact_pair(
      make_parent_geometry(L, direction), options, 4, 1e-13, 4096);
  if (!preparation.valid || !preparation.density_contained
      || !preparation.compact_support
      || !preparation.zero_boundary_crossing) return result;
  ForensicCudaStepper stepper(
      std::move(preparation.state), options, interaction_scale);
  if (!stepper.valid()) return result;
  for (int tick = 1; tick <= kFormationTick; ++tick) {
    const auto step = stepper.advance();
    if (!step.valid || !step.common) return result;
  }
  result.state = stepper.release_state();
  const auto core = observe_support_invariant_matter(result.state, options);
  result.valid = core.valid && core.member
      && core.graph_margin >= 1e-6 && core.energy_margin >= 1e-6;
  return result;
}

ConnectedMooreBlockState geometry_only(
    const ConnectedMooreBlockState& state) {
  ConnectedMooreBlockState result;
  result.electric.L = state.electric.L;
  result.magnetic_half.L = state.magnetic_half.L;
  result.constituents = state.constituents;
  result.charges = state.charges;
  result.edges = state.edges;
  result.width = state.width;
  result.orientation_axis = state.orientation_axis;
  return result;
}

ConnectedMooreBlockState rigidly_recentered_geometry(
    const ConnectedMooreBlockState& state) {
  auto result = geometry_only(state);
  const Vec3 center = object_center(state);
  const Vec3 target{static_cast<double>(std::llround(center.x)),
                    static_cast<double>(std::llround(center.y)),
                    static_cast<double>(std::llround(center.z))};
  const Vec3 shift = target - center;
  for (auto& point : result.constituents) {
    const Vec3 position = forensic_position(point) + shift;
    const long long ax = std::llround(position.x);
    const long long ay = std::llround(position.y);
    const long long az = std::llround(position.z);
    point.anchor = {forensic_wrap(static_cast<int>(ax), state.electric.L),
                    forensic_wrap(static_cast<int>(ay), state.electric.L),
                    forensic_wrap(static_cast<int>(az), state.electric.L)};
    point.remainder = {position.x - static_cast<double>(ax),
                       position.y - static_cast<double>(ay),
                       position.z - static_cast<double>(az)};
  }
  return result;
}

struct ForensicResult {
  std::string slug;
  std::string direction;
  bool parent_valid = false;
  bool replay_executed = false;
  bool replay_common = false;
  Vec3 center{};
  double fractional_center = INFINITY;
  bool cpu_observer_valid = false;
  bool cpu_boundary_ledger_valid = false;
  bool cpu_ladder_valid = false;
  bool cuda_observer_valid = false;
  bool cuda_boundary_ledger_valid = false;
  bool cuda_ladder_valid = false;
  std::string cuda_observer_error;
  std::string cuda_ladder_error;
  bool same_geometry_preparation_valid = false;
  double same_geometry_poisson_residual = INFINITY;
  double same_geometry_gauss_residual = INFINITY;
  bool recentered_preparation_valid = false;
  bool recentered_cuda_observer_valid = false;
  bool recentered_cuda_boundary_ledger_valid = false;
  bool recentered_cuda_ladder_valid = false;
  std::string recentered_cuda_observer_error;
  std::string recentered_cuda_ladder_error;
  double recentered_fractional_center = INFINITY;
  double relative_geometry_residual = INFINITY;
  double momentum_preservation_residual = INFINITY;
  double recentered_maximum_reconstruction_residual = INFINITY;
  double recentered_actual_gauss_residual = INFINITY;
  double recentered_energy_partition_residual = INFINITY;
  double recentered_characteristic_flux_residual = INFINITY;
  double recentered_ladder_energy_residual = INFINITY;
  double recentered_ladder_projection_residual = INFINITY;
  std::size_t cuda_host_to_device_bytes = 0;
  std::size_t cuda_device_to_host_bytes = 0;
  double cuda_kernel_ms = 0.0;
  bool chart_obstruction = false;
  bool physical_dressing_mismatch = false;
  bool infrastructure_unresolved = true;
};

ForensicResult run_forensic(const std::string& slug, int L,
                            int ticks) {
  ForensicResult result;
  result.slug = slug;
  ForensicDirection direction;
  if (!select_direction(slug, direction)) return result;
  result.direction = direction.label;
  const auto normalization = measure_face_flux_normalization();
  if (!normalization.valid) return result;
  auto options = forensic_options();
  auto parent = build_parent(
      L, direction, options, normalization.mapped_field_work_coefficient);
  result.parent_valid = parent.valid;
  if (!parent.valid) return result;

  auto boosted = parent.state;
  for (auto& point : boosted.constituents)
    point.momentum += direction_unit(direction) * kBoost;
  ForensicCudaStepper stepper(
      std::move(boosted), options,
      normalization.mapped_field_work_coefficient);
  if (!stepper.valid()) return result;
  bool common = true;
  int completed = 0;
  for (int tick = 1; tick <= ticks; ++tick) {
    const auto step = stepper.advance();
    common = common && step.valid && step.common;
    if (!step.valid) break;
    ++completed;
  }
  result.replay_executed = completed == ticks;
  result.replay_common = result.replay_executed && common;
  auto evolved = stepper.release_state();
  result.center = object_center(evolved);
  result.fractional_center = fractional_center_norm(result.center);

  StateOnlyMatterFieldObserverOptions observer;
  observer.support_half_width = 4;
  observer.shell_radii = L >= 97
      ? std::vector<int>{8, 12, 16, 24, 32, 48}
      : std::vector<int>{4, 8, 12};
  observer.wave_speed = options.wave_speed;
  observer.dt = options.dt;
  const std::vector<int> supports(kSupports.begin(), kSupports.end());

  const auto cpu_observer = observe_state_only_matter_field(
      evolved, options, observer);
  const auto cpu_ladder = observe_state_only_support_ladder(
      evolved, options, supports, 1e-13, 4096, kGate);
  result.cpu_observer_valid = cpu_observer.valid;
  result.cpu_boundary_ledger_valid =
      cpu_observer.boundary_energy_ledger_valid;
  result.cpu_ladder_valid = cpu_ladder.valid;

  CudaStateOnlySupportLadderTelemetry cuda_observer_telemetry;
  CudaStateOnlySupportLadderTelemetry cuda_ladder_telemetry;
  const auto cuda_observer = observe_state_only_matter_field_cuda(
      evolved, options, observer, &cuda_observer_telemetry);
  const auto cuda_ladder = observe_state_only_support_ladder_cuda(
      evolved, options, supports, 1e-13, 4096, kGate,
      &cuda_ladder_telemetry);
  result.cuda_observer_valid = cuda_observer.valid;
  result.cuda_boundary_ledger_valid =
      cuda_observer.boundary_energy_ledger_valid;
  result.cuda_ladder_valid = cuda_ladder.valid;
  result.cuda_observer_error = cuda_observer_telemetry.error;
  result.cuda_ladder_error = cuda_ladder_telemetry.error;

  const auto same_geometry = prepare_finite_support_derived_compact_pair(
      geometry_only(evolved), options, 4, 1e-13, 4096);
  result.same_geometry_preparation_valid = same_geometry.valid;
  result.same_geometry_poisson_residual = same_geometry.poisson_residual;
  result.same_geometry_gauss_residual = same_geometry.gauss_residual;

  auto recentered_geometry = rigidly_recentered_geometry(evolved);
  const Vec3 relative_before = forensic_position(evolved.constituents[1])
      - forensic_position(evolved.constituents[0]);
  const Vec3 relative_after =
      forensic_position(recentered_geometry.constituents[1])
      - forensic_position(recentered_geometry.constituents[0]);
  result.relative_geometry_residual = maximum_component(
      relative_after - relative_before);
  result.momentum_preservation_residual = 0.0;
  for (std::size_t i = 0; i < evolved.constituents.size(); ++i)
    result.momentum_preservation_residual = std::max(
        result.momentum_preservation_residual,
        maximum_component(recentered_geometry.constituents[i].momentum
                          - evolved.constituents[i].momentum));
  result.recentered_fractional_center = fractional_center_norm(
      object_center(recentered_geometry));

  auto recentered = prepare_finite_support_derived_compact_pair(
      recentered_geometry, options, 4, 1e-13, 4096);
  result.recentered_preparation_valid = recentered.valid
      && recentered.compact_support && recentered.zero_boundary_crossing;
  if (result.recentered_preparation_valid) {
    CudaStateOnlySupportLadderTelemetry recentered_observer_telemetry;
    CudaStateOnlySupportLadderTelemetry recentered_ladder_telemetry;
    const auto recentered_observer = observe_state_only_matter_field_cuda(
        recentered.state, options, observer,
        &recentered_observer_telemetry);
    const auto recentered_ladder = observe_state_only_support_ladder_cuda(
        recentered.state, options, supports, 1e-13, 4096, kGate,
        &recentered_ladder_telemetry);
    result.recentered_cuda_observer_valid = recentered_observer.valid;
    result.recentered_cuda_boundary_ledger_valid =
        recentered_observer.boundary_energy_ledger_valid;
    result.recentered_cuda_ladder_valid = recentered_ladder.valid;
    result.recentered_cuda_observer_error =
        recentered_observer_telemetry.error;
    result.recentered_cuda_ladder_error = recentered_ladder_telemetry.error;
    result.recentered_maximum_reconstruction_residual =
        recentered_observer.maximum_reconstruction_residual;
    result.recentered_actual_gauss_residual =
        recentered_observer.actual_gauss_compatibility_residual;
    result.recentered_energy_partition_residual = std::abs(
        recentered_observer.energy_partition_residual);
    result.recentered_characteristic_flux_residual =
        recentered_observer.characteristic_flux_residual;
    result.recentered_ladder_energy_residual =
        recentered_ladder.maximum_energy_reconstruction_residual;
    result.recentered_ladder_projection_residual =
        recentered_ladder.maximum_projection_residual;
    result.cuda_host_to_device_bytes =
        recentered_observer_telemetry.host_to_device_bytes
        + recentered_ladder_telemetry.host_to_device_bytes;
    result.cuda_device_to_host_bytes =
        recentered_observer_telemetry.device_to_host_bytes
        + recentered_ladder_telemetry.device_to_host_bytes;
    result.cuda_kernel_ms = recentered_observer_telemetry.kernel_ms
        + recentered_ladder_telemetry.kernel_ms;
  }

  result.chart_obstruction = result.parent_valid
      && result.replay_common && result.fractional_center > kGate
      && !result.cpu_observer_valid && !result.cpu_ladder_valid
      && !result.cuda_observer_valid && !result.cuda_ladder_valid
      && !result.same_geometry_preparation_valid
      && result.relative_geometry_residual <= kGate
      && result.momentum_preservation_residual <= kGate
      && result.recentered_fractional_center <= kGate
      && result.recentered_preparation_valid
      && result.recentered_cuda_observer_valid
      && result.recentered_cuda_boundary_ledger_valid
      && result.recentered_cuda_ladder_valid;
  result.physical_dressing_mismatch = result.parent_valid
      && result.replay_common && result.same_geometry_preparation_valid
      && !result.cuda_observer_valid;
  result.infrastructure_unresolved = !result.chart_obstruction
      && !result.physical_dressing_mismatch;
  return result;
}

std::filesystem::path results_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0762";
}

std::string json_number(double value) {
  if (!std::isfinite(value)) return "null";
  std::ostringstream stream;
  stream << std::setprecision(17) << value;
  return stream.str();
}

void write_result(const ForensicResult& value) {
  const auto directory = results_directory();
  std::filesystem::create_directories(directory);
  const auto path = directory
      / ("ftd_0762_moving_dressing_observer_forensics_v1_"
         + value.slug + ".json");
  std::ofstream json(path);
  json << std::boolalpha << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0762\",\n"
       << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
       << "  \"direction\": \"" << value.direction << "\",\n"
       << "  \"slug\": \"" << value.slug << "\",\n"
       << "  \"volume\": " << kRegisteredVolume << ",\n"
       << "  \"formation_tick\": " << kFormationTick << ",\n"
       << "  \"forensic_ticks\": " << kForensicTicks << ",\n"
       << "  \"boost\": " << kBoost << ",\n"
       << "  \"parent_valid\": " << value.parent_valid << ",\n"
       << "  \"replay_executed\": " << value.replay_executed << ",\n"
       << "  \"replay_common\": " << value.replay_common << ",\n"
       << "  \"center\": [" << json_number(value.center.x) << ", "
       << json_number(value.center.y) << ", "
       << json_number(value.center.z) << "],\n"
       << "  \"fractional_center_norm\": "
       << json_number(value.fractional_center) << ",\n"
       << "  \"cpu_observer_valid\": " << value.cpu_observer_valid << ",\n"
       << "  \"cpu_boundary_ledger_valid\": "
       << value.cpu_boundary_ledger_valid << ",\n"
       << "  \"cpu_ladder_valid\": " << value.cpu_ladder_valid << ",\n"
       << "  \"cuda_observer_valid\": " << value.cuda_observer_valid << ",\n"
       << "  \"cuda_boundary_ledger_valid\": "
       << value.cuda_boundary_ledger_valid << ",\n"
       << "  \"cuda_ladder_valid\": " << value.cuda_ladder_valid << ",\n"
       << "  \"cuda_observer_error\": \""
       << value.cuda_observer_error << "\",\n"
       << "  \"cuda_ladder_error\": \""
       << value.cuda_ladder_error << "\",\n"
       << "  \"same_geometry_preparation_valid\": "
       << value.same_geometry_preparation_valid << ",\n"
       << "  \"same_geometry_poisson_residual\": "
       << json_number(value.same_geometry_poisson_residual) << ",\n"
       << "  \"same_geometry_gauss_residual\": "
       << json_number(value.same_geometry_gauss_residual) << ",\n"
       << "  \"recentered_preparation_valid\": "
       << value.recentered_preparation_valid << ",\n"
       << "  \"recentered_cuda_observer_valid\": "
       << value.recentered_cuda_observer_valid << ",\n"
       << "  \"recentered_cuda_boundary_ledger_valid\": "
       << value.recentered_cuda_boundary_ledger_valid << ",\n"
       << "  \"recentered_cuda_ladder_valid\": "
       << value.recentered_cuda_ladder_valid << ",\n"
       << "  \"recentered_cuda_observer_error\": \""
       << value.recentered_cuda_observer_error << "\",\n"
       << "  \"recentered_cuda_ladder_error\": \""
       << value.recentered_cuda_ladder_error << "\",\n"
       << "  \"recentered_fractional_center_norm\": "
       << json_number(value.recentered_fractional_center) << ",\n"
       << "  \"relative_geometry_residual\": "
       << json_number(value.relative_geometry_residual) << ",\n"
       << "  \"momentum_preservation_residual\": "
       << json_number(value.momentum_preservation_residual) << ",\n"
       << "  \"recentered_maximum_reconstruction_residual\": "
       << json_number(value.recentered_maximum_reconstruction_residual)
       << ",\n"
       << "  \"recentered_actual_gauss_residual\": "
       << json_number(value.recentered_actual_gauss_residual) << ",\n"
       << "  \"recentered_energy_partition_residual\": "
       << json_number(value.recentered_energy_partition_residual) << ",\n"
       << "  \"recentered_characteristic_flux_residual\": "
       << json_number(value.recentered_characteristic_flux_residual) << ",\n"
       << "  \"recentered_ladder_energy_residual\": "
       << json_number(value.recentered_ladder_energy_residual) << ",\n"
       << "  \"recentered_ladder_projection_residual\": "
       << json_number(value.recentered_ladder_projection_residual) << ",\n"
       << "  \"cuda_host_to_device_bytes\": "
       << value.cuda_host_to_device_bytes << ",\n"
       << "  \"cuda_device_to_host_bytes\": "
       << value.cuda_device_to_host_bytes << ",\n"
       << "  \"cuda_kernel_ms\": " << json_number(value.cuda_kernel_ms)
       << ",\n"
       << "  \"chart_obstruction\": " << value.chart_obstruction << ",\n"
       << "  \"physical_dressing_mismatch\": "
       << value.physical_dressing_mismatch << ",\n"
       << "  \"infrastructure_unresolved\": "
       << value.infrastructure_unresolved << ",\n"
       << "  \"production_changed\": false,\n"
       << "  \"dynamics_changed\": false\n}\n";
}

bool read_bit(const std::filesystem::path& path, const std::string& key) {
  std::ifstream input(path);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find("\"" + key + "\": true") != std::string::npos;
}

void write_aggregate() {
  const auto directory = results_directory();
  const std::array<std::string, 3> slugs{{"face", "edge", "body"}};
  bool chart = true;
  bool physical = true;
  bool complete = true;
  for (const auto& slug : slugs) {
    const auto path = directory
        / ("ftd_0762_moving_dressing_observer_forensics_v1_"
           + slug + ".json");
    complete = complete && std::filesystem::is_regular_file(path);
    chart = chart && read_bit(path, "chart_obstruction");
    physical = physical && read_bit(path, "physical_dressing_mismatch");
  }
  std::string verdict = "INFRASTRUCTURE_UNRESOLVED";
  if (complete && chart)
    verdict = "OBSERVER_INTEGER_CENTER_CHART_OBSTRUCTION";
  else if (complete && physical)
    verdict = "PHYSICAL_DRESSING_MISMATCH_EXPOSED";
  std::ofstream json(directory
      / "ftd_0762_moving_dressing_observer_forensics_v1.json");
  json << std::boolalpha
       << "{\n  \"ftd_id\": \"FTD-0762\",\n"
       << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"all_artifacts_present\": " << complete << ",\n"
       << "  \"all_rays_chart_obstruction\": " << chart << ",\n"
       << "  \"all_rays_physical_dressing_mismatch\": " << physical
       << ",\n  \"production_changed\": false,\n"
       << "  \"dynamics_changed\": false\n}\n";
}

int qualify(const std::string& slug) {
  const auto result = run_forensic(slug, 33, 2);
  const bool pass = result.parent_valid && result.replay_common
      && result.fractional_center > kGate
      && !result.same_geometry_preparation_valid
      && result.recentered_preparation_valid
      && result.recentered_cuda_observer_valid
      && result.recentered_cuda_boundary_ledger_valid
      && result.recentered_cuda_ladder_valid;
  std::cout << std::boolalpha << std::setprecision(17)
            << "FTD-0762 qualification direction=" << slug
            << " pass=" << pass
            << " fractional_center=" << result.fractional_center
            << " same_geometry=" << result.same_geometry_preparation_valid
            << " recentered_preparation="
            << result.recentered_preparation_valid
            << " recentered_observer="
            << result.recentered_cuda_observer_valid
            << " recentered_ladder="
            << result.recentered_cuda_ladder_valid << '\n';
  return pass ? 0 : 1;
}

int run_registered(const std::string& slug) {
  if (std::string(kProtocolSha256) == "UNLOCKED") return 3;
  if (slug == "body") {
    for (const auto& prior : {"face", "edge"}) {
      const auto path = results_directory()
          / (std::string("ftd_0762_moving_dressing_observer_forensics_v1_")
             + prior + ".json");
      if (!std::filesystem::is_regular_file(path)) return 4;
    }
  }
  const auto result = run_forensic(
      slug, kRegisteredVolume, kForensicTicks);
  write_result(result);
  if (slug == "body") write_aggregate();
  std::cout << std::boolalpha << std::setprecision(17)
            << "FTD-0762 direction=" << slug
            << " replay=" << result.replay_common
            << " fractional_center=" << result.fractional_center
            << " same_geometry=" << result.same_geometry_preparation_valid
            << " recentered=" << result.recentered_preparation_valid
            << " recentered_observer="
            << result.recentered_cuda_observer_valid
            << " recentered_ladder="
            << result.recentered_cuda_ladder_valid
            << " chart_obstruction=" << result.chart_obstruction
            << " physical_mismatch=" << result.physical_dressing_mismatch
            << '\n';
  return result.infrastructure_unresolved ? 1 : 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc == 3 && std::string(argv[1]) == "--qualify")
    return qualify(argv[2]);
  if (argc == 3 && std::string(argv[1]) == "--run")
    return run_registered(argv[2]);
  std::cout << "FTD-0762 runner: --qualify face|edge|body; "
               "--run face|edge|body\n";
  return argc == 1 ? 0 : 2;
}
