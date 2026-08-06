/** FTD-0761: boosted transport discovery for the certified M3 family. */

#define FTD_0754_MAIN_NAME ftd_0761_global_observer_main
#include "campaign_state_only_observer_discovery_cuda.cpp"
#undef FTD_0754_MAIN_NAME

#include "ftd/eft/support_invariant_matter_predicate.h"

#include <array>
#include <bitset>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>

namespace ftd0761_embedded {

#include "campaign_m3_support_invariant_validation_cuda.cpp"

namespace {

using namespace ftd;
using namespace ftd::eft;

constexpr char kM4ProtocolSha256[] =
    "AD6368C6793374771703A1506FA60C06E1D11C0649227F315DD1A79A0F3BDA5C";
constexpr int kVolume = 321;
constexpr int kFormationTick = 160;
constexpr int kTransportTicks = 256;
constexpr double kBoost = 0.015;
constexpr double kCoreGate = 1e-6;
constexpr double kCommonGate = 1e-10;
constexpr double kEnergyGate = 1e-8;
constexpr double kSpeedGate = 1e-12;
constexpr double kSigmaGate = 1e-3;
constexpr double kConditionGate = 1e4;
constexpr double kReverseGate = 1e-10;
constexpr double kRestGate = 1e-9;
constexpr double kFinalDisplacementGate = 1.0;
constexpr double kTransverseGate = 0.10;
constexpr double kMirrorGate = 1e-7;
constexpr double kMomentumStepGate = 1e-9;
constexpr double kMomentumCumulativeGate = 1e-8;
constexpr std::array<int, 5> kCheckpoints{{160, 224, 288, 352, 416}};

bool checkpoint_tick(int tick) {
  return std::find(kCheckpoints.begin(), kCheckpoints.end(), tick)
      != kCheckpoints.end();
}

Vec3 object_center(const ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents)
    result += effective_position(point);
  return result * (1.0 / static_cast<double>(state.constituents.size()));
}

Vec3 direction_unit(const Direction& direction) {
  const Vec3 value{static_cast<double>(direction.x),
                   static_cast<double>(direction.y),
                   static_cast<double>(direction.z)};
  return value * (1.0 / value.mag());
}

Vec3 matter_momentum(const ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += point.momentum;
  return result;
}

double matter_energy(const ConnectedMooreBlockState& state,
                     const ConnectedMooreBlockOptions& options) {
  long double result = connected_moore_block_binding_energy(state, options);
  for (const auto& point : state.constituents)
    result += options.constituent_mass_scale
        * production_flat_energy_from_momentum(point.momentum);
  return static_cast<double>(result);
}

double total_energy(const ConnectedMooreBlockState& state,
                    const ConnectedMooreBlockOptions& options,
                    double interaction_scale) {
  return matter_energy(state, options)
      + interaction_scale * matched_modified_energy(
          state.electric, state.magnetic_half,
          options.wave_speed * options.dt);
}

double step_common_residual(const ConnectedMooreBlockStepResult& step) {
  return std::max({
      step.root_residual, step.continuity_residual,
      step.gauss_before_residual, step.gauss_after_residual,
      step.force_residual, step.kinematic_residual,
      step.kinetic_discrete_gradient_residual,
      step.electric_adjoint_residual, step.magnetic_work_residual,
      step.binding_work_residual, step.binding_impulse_sum_residual,
      step.matter_work_residual, step.field_work_residual,
      step.total_energy_residual});
}

double vec_max(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

struct M4Row {
  int tick = 0;
  bool member = false;
  bool checkpoint = false;
  bool observer_valid = false;
  bool ladder_valid = false;
  bool step_valid = true;
  bool common = true;
  bool regularity_measured = false;
  bool reverse_valid = true;
  bool site_projection_valid = true;
  int site_hops = 0;
  Vec3 center{};
  Vec3 relative{};
  Vec3 p0{}, p1{};
  Vec3 matter_p{}, field_p{}, total_p{};
  std::array<Coord, 2> anchors{};
  std::array<Vec3, 2> remainders{};
  double graph_margin = -INFINITY;
  double energy_margin = -INFINITY;
  double pair_energy = INFINITY;
  double common_residual = INFINITY;
  double energy_residual = INFINITY;
  double energy_drift = INFINITY;
  double speed_excess = INFINITY;
  double sigma_min = 0.0;
  double condition_number = INFINITY;
  double reverse_recovery = INFINITY;
  double momentum_step_defect = INFINITY;
  double momentum_cumulative_defect = INFINITY;
};

struct M4Step {
  bool valid = false;
  bool common = false;
  bool site_projection_valid = false;
  bool regularity_measured = false;
  int site_hops = 0;
  Vec3 field_momentum_after{};
  double common_residual = INFINITY;
  double energy_residual = INFINITY;
  double total_energy_after = INFINITY;
  double speed_excess = INFINITY;
  double sigma_min = 0.0;
  double condition_number = INFINITY;
  double momentum_step_defect = INFINITY;
  double reverse_recovery = INFINITY;
  bool reverse_valid = false;
};

class M4CudaStepper {
 public:
  M4CudaStepper(ConnectedMooreBlockState initial,
                ConnectedMooreBlockOptions options,
                double interaction_scale)
      : state_(std::move(initial)), options_(std::move(options)),
        interaction_scale_(interaction_scale), pipeline_(state_.electric.L),
        prepared_magnetic_(state_.electric.L),
        prepared_electric_(state_.electric.L) {
    const double c = static_cast<double>(state_.electric.L / 2);
    fixed_center_ = {c, c, c};
    options_.defer_volume_diagnostics = true;
    valid_ = pipeline_.valid()
        && pipeline_.upload(state_.electric, state_.magnetic_half);
  }

  bool valid() const { return valid_; }
  const ConnectedMooreBlockState& state() const { return state_; }
  ConnectedMooreBlockState release_state() { return std::move(state_); }

  M4Step advance(bool measure_regularity, bool reverse_check = true) {
    M4Step result;
    if (!valid_) return result;
    options_.measure_final_root_regularity = measure_regularity;
    const double lambda = options_.wave_speed * options_.dt;
    if (!pipeline_.prepare_forward(lambda)
        || !pipeline_.download_prepared(
            prepared_magnetic_, prepared_electric_)) {
      valid_ = false;
      return result;
    }
    auto step = solve_connected_moore_block_forward_prepared(
        state_, std::move(prepared_magnetic_), std::move(prepared_electric_),
        options_, &forward_cache_);
    if (!step.volume_diagnostics_pending
        || !pipeline_.apply_ordered_sparse_current(
            step.segments, options_.polarity_scale)) {
      valid_ = false;
      return result;
    }
    const auto profile = pipeline_.observe_deterministic(
        lambda, fixed_center_, {8}, kCommonGate);
    if (!profile.valid) {
      valid_ = false;
      return result;
    }
    const auto diagnostics = pipeline_.diagnose_common_action(
        step.segments, options_.polarity_scale, interaction_scale_,
        options_.wave_speed, options_.dt, kCommonGate);
    step = complete_connected_moore_block_volume_diagnostics(
        std::move(step), diagnostics, options_);
    if (reverse_check && measure_regularity
        && step.valid && step.common_action_gates_pass) {
      auto reverse_options = options_;
      reverse_options.defer_volume_diagnostics = false;
      reverse_options.measure_final_root_regularity = false;
      ConnectedMooreBlockSolveCache reverse_cache;
      const auto reverse = solve_connected_moore_block_reverse(
          step.later, reverse_options, &reverse_cache);
      result.reverse_valid = reverse.valid
          && reverse.common_action_gates_pass;
      if (result.reverse_valid)
        result.reverse_recovery = connected_moore_block_state_max_difference(
            state_, reverse.earlier);
      if (result.reverse_valid && !std::isfinite(result.reverse_recovery)) {
        std::cerr << "FTD-0761 reverse topology mismatch"
                  << " L=" << state_.electric.L << '/'
                  << reverse.earlier.electric.L
                  << " constituents=" << state_.constituents.size()
                  << '/' << reverse.earlier.constituents.size()
                  << " charges=" << (state_.charges
                                      == reverse.earlier.charges)
                  << " face_size=" << state_.electric.x.size() << '/'
                  << reverse.earlier.electric.x.size()
                  << " edge_size=" << state_.magnetic_half.x.size()
                  << '/' << reverse.earlier.magnetic_half.x.size()
                  << " edges=" << state_.edges.size() << '/'
                  << reverse.earlier.edges.size();
        if (state_.edges.size() == reverse.earlier.edges.size()) {
          for (std::size_t e = 0; e < state_.edges.size(); ++e) {
            const auto& a = state_.edges[e];
            const auto& b = reverse.earlier.edges[e];
            std::cerr << " edge" << e << '='
                      << a.first << ':' << a.second << ':'
                      << a.reference_delta.x << ':' << a.reference_delta.y
                      << ':' << a.reference_delta.z << ':'
                      << a.rest_length_squared << '/'
                      << b.first << ':' << b.second << ':'
                      << b.reference_delta.x << ':' << b.reference_delta.y
                      << ':' << b.reference_delta.z << ':'
                      << b.rest_length_squared;
          }
        }
        std::cerr << '\n';
      }
    } else {
      result.reverse_valid = true;
      result.reverse_recovery = 0.0;
    }
    result.valid = step.valid && step.common_action_gates_pass;
    result.common = step.common_action_gates_pass;
    result.site_projection_valid = step.site_projection_valid;
    result.regularity_measured =
        step.solve.final_root_regularity_measured;
    result.site_hops = step.site_hops;
    result.field_momentum_after = step.spline_field_momentum_after;
    result.common_residual = step_common_residual(step);
    result.energy_residual = std::abs(step.total_energy_residual);
    result.total_energy_after = step.kinetic_energy_after
        + step.binding_energy_after + step.field_energy_after;
    result.speed_excess = step.causal_speed_excess;
    result.sigma_min = step.solve.final_minimum_singular_value;
    result.condition_number = step.solve.final_condition_number;
    result.momentum_step_defect = step.spline_defect_norm;
    if (!pipeline_.advance()) valid_ = false;
    state_ = std::move(step.later);
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
  ConnectedMooreBlockSolveCache forward_cache_;
  bool valid_ = false;
};

M3ParentCheckpoint build_parent(
    int L, const Direction& direction,
    const ConnectedMooreBlockOptions& options,
    double interaction_scale, M4Step* formation_step = nullptr,
    int tick_limit = kFormationTick) {
  M3ParentCheckpoint result(L);
  result.direction = direction.label;
  auto preparation = prepare_finite_support_derived_compact_pair(
      make_geometry(L, direction, false, 1.30, 0.0120),
      options, 4, 1e-13, 4096);
  if (!preparation.valid || !preparation.density_contained
      || !preparation.compact_support
      || !preparation.zero_boundary_crossing)
    return result;
  M4CudaStepper stepper(
      std::move(preparation.state), options, interaction_scale);
  if (!stepper.valid()) return result;
  for (int tick = 1; tick <= tick_limit; ++tick) {
    const auto step = stepper.advance(tick == tick_limit, false);
    if (!step.valid || !step.common) return result;
    if (formation_step != nullptr && tick == tick_limit)
      *formation_step = step;
  }
  result.state = stepper.release_state();
  const auto core = observe_support_invariant_matter(result.state, options);
  result.valid = core.valid && core.member
      && core.graph_margin >= kCoreGate
      && core.energy_margin >= kCoreGate;
  return result;
}

struct M4Arm {
  std::string name;
  int sign = 0;
  bool initialized = false;
  bool executed = false;
  bool exact = false;
  bool stationary = false;
  bool transport = false;
  bool field_balanced = false;
  int total_hops = 0;
  double external_energy = INFINITY;
  double minimum_graph_margin = INFINITY;
  double minimum_energy_margin = INFINITY;
  double maximum_common = 0.0;
  double maximum_energy_residual = 0.0;
  double maximum_energy_drift = 0.0;
  double maximum_speed_excess = 0.0;
  double minimum_sigma = INFINITY;
  double maximum_condition = 0.0;
  double maximum_reverse_recovery = 0.0;
  double maximum_momentum_step_defect = 0.0;
  double maximum_momentum_cumulative_defect = 0.0;
  std::vector<M4Row> rows;
};

M4Row make_row(int tick, const ConnectedMooreBlockState& state,
               const ConnectedMooreBlockOptions& options,
               double interaction_scale, double initial_energy,
               const Vec3& initial_total_momentum,
               const M4Step* step = nullptr,
               bool forced_checkpoint = false) {
  M4Row row;
  row.tick = tick;
  row.checkpoint = checkpoint_tick(tick) || forced_checkpoint;
  const auto core = observe_support_invariant_matter(state, options);
  row.member = core.valid && core.member;
  row.graph_margin = core.graph_margin;
  row.energy_margin = core.energy_margin;
  row.pair_energy = core.pair_energy;
  row.center = object_center(state);
  row.relative = core.relative_position;
  if (state.constituents.size() == 2) {
    row.p0 = state.constituents[0].momentum;
    row.p1 = state.constituents[1].momentum;
    for (int a = 0; a < 2; ++a) {
      row.anchors[a] = state.constituents[a].anchor;
      row.remainders[a] = state.constituents[a].remainder;
    }
  }
  row.matter_p = matter_momentum(state);
  if (step) row.field_p = step->field_momentum_after;
  else {
    const auto spline = measure_spline_poynting_momentum(
        state.electric, state.magnetic_half, options.wave_speed, options.dt,
        interaction_scale);
    row.field_p = spline.momentum;
  }
  row.total_p = row.matter_p + row.field_p;
  row.momentum_cumulative_defect =
      (row.total_p - initial_total_momentum).mag();
  row.energy_drift = std::abs((step ? step->total_energy_after
      : total_energy(state, options, interaction_scale)) - initial_energy);
  if (row.checkpoint) {
    const auto checkpoint = m3_make_checkpoint_record(
        tick, state, options, 8);
    row.observer_valid = checkpoint.observer_valid;
    row.ladder_valid = checkpoint.ladder_valid;
  } else {
    row.observer_valid = true;
    row.ladder_valid = true;
  }
  if (step) {
    row.step_valid = step->valid;
    row.common = step->common;
    row.site_projection_valid = step->site_projection_valid;
    row.site_hops = step->site_hops;
    row.common_residual = step->common_residual;
    row.energy_residual = step->energy_residual;
    row.speed_excess = step->speed_excess;
    row.regularity_measured = step->regularity_measured;
    row.sigma_min = step->sigma_min;
    row.condition_number = step->condition_number;
    row.reverse_valid = step->reverse_valid;
    row.reverse_recovery = step->reverse_recovery;
    row.momentum_step_defect = step->momentum_step_defect;
  } else {
    row.common_residual = 0.0;
    row.energy_residual = 0.0;
    row.speed_excess = 0.0;
    row.momentum_step_defect = 0.0;
    row.reverse_recovery = 0.0;
  }
  return row;
}

M4Arm run_arm(const ConnectedMooreBlockState& parent,
              const Direction& direction, int sign,
              const ConnectedMooreBlockOptions& options,
              double interaction_scale,
              const M4Step* formation_step = nullptr,
              int ticks = kTransportTicks) {
  M4Arm arm;
  arm.sign = sign;
  arm.name = sign == 0 ? "rest" : (sign > 0 ? "plus" : "minus");
  auto initial = parent;
  const double energy_before = matter_energy(initial, options);
  if (sign != 0)
    for (auto& point : initial.constituents)
      point.momentum += direction_unit(direction) * (sign * kBoost);
  arm.external_energy = matter_energy(initial, options) - energy_before;
  const auto core = observe_support_invariant_matter(initial, options);
  arm.initialized = core.valid && core.member
      && core.graph_margin >= kCoreGate && core.energy_margin >= kCoreGate;
  if (!arm.initialized) return arm;
  const double initial_energy = total_energy(
      initial, options, interaction_scale);
  const auto initial_spline = measure_spline_poynting_momentum(
      initial.electric, initial.magnetic_half, options.wave_speed, options.dt,
      interaction_scale);
  const Vec3 initial_total_momentum =
      matter_momentum(initial) + initial_spline.momentum;
  auto initial_row = make_row(kFormationTick, initial, options,
      interaction_scale, initial_energy, initial_total_momentum);
  if (formation_step != nullptr) {
    initial_row.regularity_measured =
        formation_step->regularity_measured;
    initial_row.sigma_min = formation_step->sigma_min;
    initial_row.condition_number = formation_step->condition_number;
    arm.minimum_sigma = initial_row.sigma_min;
    arm.maximum_condition = initial_row.condition_number;
  }
  arm.rows.push_back(initial_row);
  M4CudaStepper stepper(std::move(initial), options, interaction_scale);
  if (!stepper.valid()) return arm;
  bool exact = initial_row.member
      && initial_row.graph_margin >= kCoreGate
      && initial_row.energy_margin >= kCoreGate
      && initial_row.observer_valid && initial_row.ladder_valid
      && initial_row.regularity_measured
      && initial_row.sigma_min >= kSigmaGate
      && initial_row.condition_number <= kConditionGate;
  for (int offset = 1; offset <= ticks; ++offset) {
    const int tick = kFormationTick + offset;
    const bool registered_checkpoint = checkpoint_tick(tick);
    const bool reverse_checkpoint = registered_checkpoint
        || (ticks != kTransportTicks && offset == ticks);
    const auto step = stepper.advance(reverse_checkpoint);
    auto row = make_row(tick, stepper.state(), options, interaction_scale,
        initial_energy, initial_total_momentum, &step,
        registered_checkpoint);
    arm.rows.push_back(row);
    arm.total_hops += row.site_hops;
    arm.minimum_graph_margin = std::min(
        arm.minimum_graph_margin, row.graph_margin);
    arm.minimum_energy_margin = std::min(
        arm.minimum_energy_margin, row.energy_margin);
    arm.maximum_common = std::max(arm.maximum_common, row.common_residual);
    arm.maximum_energy_residual = std::max(
        arm.maximum_energy_residual, row.energy_residual);
    arm.maximum_energy_drift = std::max(
        arm.maximum_energy_drift, row.energy_drift);
    arm.maximum_speed_excess = std::max(
        arm.maximum_speed_excess, row.speed_excess);
    arm.maximum_momentum_step_defect = std::max(
        arm.maximum_momentum_step_defect, row.momentum_step_defect);
    arm.maximum_momentum_cumulative_defect = std::max(
        arm.maximum_momentum_cumulative_defect,
        row.momentum_cumulative_defect);
    if (reverse_checkpoint) {
      arm.minimum_sigma = std::min(arm.minimum_sigma, row.sigma_min);
      arm.maximum_condition = std::max(
          arm.maximum_condition, row.condition_number);
      arm.maximum_reverse_recovery = std::max(
          arm.maximum_reverse_recovery, row.reverse_recovery);
    }
    exact = exact && row.member
        && row.graph_margin >= kCoreGate
        && row.energy_margin >= kCoreGate
        && row.step_valid && row.common
        && row.common_residual <= kCommonGate
        && row.energy_residual <= kEnergyGate
        && row.speed_excess <= kSpeedGate
        && row.observer_valid && row.ladder_valid;
    if (reverse_checkpoint)
      exact = exact && row.regularity_measured
          && row.sigma_min >= kSigmaGate
          && row.condition_number <= kConditionGate
          && row.reverse_valid
          && row.reverse_recovery <= kReverseGate;
    if (!step.valid) break;
  }
  arm.executed = arm.rows.size() == static_cast<std::size_t>(ticks + 1);
  arm.exact = arm.executed && exact;
  arm.field_balanced = arm.exact
      && arm.maximum_momentum_step_defect <= kMomentumStepGate
      && arm.maximum_momentum_cumulative_defect <= kMomentumCumulativeGate;
  return arm;
}

struct DirectionSummary {
  std::string slug;
  std::string direction;
  bool parent = false;
  bool infrastructure = false;
  bool baseline = false;
  bool coherence = false;
  bool transport = false;
  bool field_balanced = false;
  double plus_final = 0.0;
  double minus_final = 0.0;
  double maximum_transverse = 0.0;
  double mirror_residual = 0.0;
  std::array<double, 4> plus_increments{};
  std::array<double, 4> minus_increments{};
  std::array<M4Arm, 3> arms;
};

DirectionSummary evaluate_direction(const std::string& slug,
                                    const Direction& direction,
                                    bool parent_valid,
                                    std::array<M4Arm, 3> arms) {
  DirectionSummary result;
  result.slug = slug;
  result.direction = direction.label;
  result.parent = parent_valid;
  result.arms = std::move(arms);
  auto& rest = result.arms[0];
  auto& plus = result.arms[1];
  auto& minus = result.arms[2];
  result.infrastructure = parent_valid
      && std::all_of(result.arms.begin(), result.arms.end(),
          [](const M4Arm& arm) { return arm.initialized && arm.executed; });
  result.baseline = result.infrastructure && rest.exact
      && (rest.rows.back().center - rest.rows.front().center).mag()
          <= kRestGate;
  result.coherence = result.baseline && plus.exact && minus.exact;
  if (!result.coherence) return result;
  const Vec3 unit = direction_unit(direction);
  std::array<double, 5> plus_blocks{}, minus_blocks{};
  for (int tick_index = 0; tick_index <= kTransportTicks; ++tick_index) {
    const Vec3 dp = plus.rows[tick_index].center
        - rest.rows[tick_index].center;
    const Vec3 dm = minus.rows[tick_index].center
        - rest.rows[tick_index].center;
    const double sp = dp.dot(unit);
    const double sm = -dm.dot(unit);
    result.maximum_transverse = std::max({result.maximum_transverse,
        (dp - unit * sp).mag(),
        (dm + unit * sm).mag()});
    result.mirror_residual = std::max({result.mirror_residual,
        vec_max(dp + dm),
        std::abs(plus.rows[tick_index].graph_margin
                 - minus.rows[tick_index].graph_margin),
        std::abs(plus.rows[tick_index].energy_margin
                 - minus.rows[tick_index].energy_margin)});
    if (tick_index % 64 == 0) {
      const int block = tick_index / 64;
      plus_blocks[block] = sp;
      minus_blocks[block] = sm;
    }
  }
  for (int block = 0; block < 4; ++block) {
    result.plus_increments[block] = plus_blocks[block + 1]
        - plus_blocks[block];
    result.minus_increments[block] = minus_blocks[block + 1]
        - minus_blocks[block];
  }
  result.plus_final = plus_blocks[4];
  result.minus_final = minus_blocks[4];
  const bool increasing =
      std::all_of(result.plus_increments.begin(),
                  result.plus_increments.end(),
                  [](double value) { return value > 0.0; })
      && std::all_of(result.minus_increments.begin(),
                     result.minus_increments.end(),
                     [](double value) { return value > 0.0; });
  result.transport = result.plus_final >= kFinalDisplacementGate
      && result.minus_final >= kFinalDisplacementGate
      && increasing && result.maximum_transverse <= kTransverseGate
      && plus.total_hops >= 2 && minus.total_hops >= 2
      && result.mirror_residual <= kMirrorGate;
  plus.transport = minus.transport = result.transport;
  result.field_balanced = result.transport
      && plus.field_balanced && minus.field_balanced;
  return result;
}

std::filesystem::path results_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0761";
}

std::string json_number(double value) {
  if (!std::isfinite(value)) return "null";
  std::ostringstream stream;
  stream << std::setprecision(17) << value;
  return stream.str();
}

void write_direction(const DirectionSummary& summary) {
  const auto directory = results_directory();
  std::filesystem::create_directories(directory);
  const auto stem = "ftd_0761_m4_boosted_transport_v1_" + summary.slug;
  std::ofstream csv(directory / (stem + ".csv"));
  csv << "ftd_id,protocol_sha256,direction,arm,sign,tick,member,"
         "graph_margin,energy_margin,pair_energy,cx,cy,cz,rx,ry,rz,"
         "a0x,a0y,a0z,r0x,r0y,r0z,p0x,p0y,p0z,"
         "a1x,a1y,a1z,r1x,r1y,r1z,p1x,p1y,p1z,"
         "matter_px,matter_py,matter_pz,field_px,field_py,field_pz,"
         "total_px,total_py,total_pz,site_hops,site_projection_valid,"
         "step_valid,common,common_residual,energy_residual,energy_drift,"
         "speed_excess,checkpoint,observer_valid,ladder_valid,"
         "regularity_measured,sigma_min,condition_number,reverse_valid,"
         "reverse_recovery,momentum_step_defect,momentum_cumulative_defect\n";
  for (const auto& arm : summary.arms)
    for (const auto& row : arm.rows) {
      csv << std::setprecision(17)
          << "FTD-0761," << kM4ProtocolSha256 << ',' << summary.direction
          << ',' << arm.name << ',' << arm.sign << ',' << row.tick << ','
          << row.member << ',' << row.graph_margin << ',' << row.energy_margin
          << ',' << row.pair_energy << ',' << row.center.x << ','
          << row.center.y << ',' << row.center.z << ',' << row.relative.x
          << ',' << row.relative.y << ',' << row.relative.z << ','
          << row.anchors[0].x << ',' << row.anchors[0].y << ','
          << row.anchors[0].z << ',' << row.remainders[0].x << ','
          << row.remainders[0].y << ',' << row.remainders[0].z << ','
          << row.p0.x << ',' << row.p0.y << ',' << row.p0.z << ','
          << row.anchors[1].x << ',' << row.anchors[1].y << ','
          << row.anchors[1].z << ',' << row.remainders[1].x << ','
          << row.remainders[1].y << ',' << row.remainders[1].z << ','
          << row.p1.x << ',' << row.p1.y << ',' << row.p1.z << ','
          << row.matter_p.x << ',' << row.matter_p.y << ',' << row.matter_p.z
          << ',' << row.field_p.x << ',' << row.field_p.y << ','
          << row.field_p.z << ',' << row.total_p.x << ',' << row.total_p.y
          << ',' << row.total_p.z << ',' << row.site_hops << ','
          << row.site_projection_valid << ',' << row.step_valid << ','
          << row.common << ',' << row.common_residual << ','
          << row.energy_residual << ',' << row.energy_drift << ','
          << row.speed_excess << ',' << row.checkpoint << ','
          << row.observer_valid << ',' << row.ladder_valid << ','
          << row.regularity_measured << ',' << row.sigma_min << ','
          << row.condition_number << ',' << row.reverse_valid << ','
          << row.reverse_recovery << ',' << row.momentum_step_defect << ','
          << row.momentum_cumulative_defect << '\n';
    }
  std::ofstream json(directory / (stem + ".json"));
  json << std::boolalpha << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0761\",\n"
       << "  \"protocol_sha256\": \"" << kM4ProtocolSha256 << "\",\n"
       << "  \"direction\": \"" << summary.direction << "\",\n"
       << "  \"slug\": \"" << summary.slug << "\",\n"
       << "  \"volume\": " << kVolume << ",\n"
       << "  \"formation_tick\": " << kFormationTick << ",\n"
       << "  \"transport_ticks\": " << kTransportTicks << ",\n"
       << "  \"boost\": " << kBoost << ",\n"
       << "  \"parent_pass\": " << summary.parent << ",\n"
       << "  \"infrastructure_pass\": " << summary.infrastructure << ",\n"
       << "  \"baseline_pass\": " << summary.baseline << ",\n"
       << "  \"coherence_pass\": " << summary.coherence << ",\n"
       << "  \"transport_pass\": " << summary.transport << ",\n"
       << "  \"field_balanced\": " << summary.field_balanced << ",\n"
       << "  \"plus_final\": " << json_number(summary.plus_final) << ",\n"
       << "  \"minus_final\": " << json_number(summary.minus_final) << ",\n"
       << "  \"maximum_transverse\": "
       << json_number(summary.maximum_transverse) << ",\n"
       << "  \"mirror_residual\": "
       << json_number(summary.mirror_residual) << ",\n"
       << "  \"arms\": [\n";
  for (std::size_t index = 0; index < summary.arms.size(); ++index) {
    const auto& arm = summary.arms[index];
    json << "    {\"name\": \"" << arm.name << "\", \"sign\": "
         << arm.sign << ", \"initialized\": " << arm.initialized
         << ", \"executed\": " << arm.executed
         << ", \"exact\": " << arm.exact
         << ", \"transport\": " << arm.transport
         << ", \"field_balanced\": " << arm.field_balanced
         << ", \"total_hops\": " << arm.total_hops
         << ", \"external_energy\": " << json_number(arm.external_energy)
         << ", \"minimum_graph_margin\": "
         << json_number(arm.minimum_graph_margin)
         << ", \"minimum_energy_margin\": "
         << json_number(arm.minimum_energy_margin)
         << ", \"maximum_common\": " << json_number(arm.maximum_common)
         << ", \"maximum_energy_residual\": "
         << json_number(arm.maximum_energy_residual)
         << ", \"maximum_energy_drift\": "
         << json_number(arm.maximum_energy_drift)
         << ", \"maximum_speed_excess\": "
         << json_number(arm.maximum_speed_excess)
         << ", \"minimum_sigma\": " << json_number(arm.minimum_sigma)
         << ", \"maximum_condition\": "
         << json_number(arm.maximum_condition)
         << ", \"maximum_reverse_recovery\": "
         << json_number(arm.maximum_reverse_recovery)
         << ", \"maximum_momentum_step_defect\": "
         << json_number(arm.maximum_momentum_step_defect)
         << ", \"maximum_momentum_cumulative_defect\": "
         << json_number(arm.maximum_momentum_cumulative_defect) << "}"
         << (index + 1 == summary.arms.size() ? "\n" : ",\n");
  }
  json << "  ],\n  \"production_changed\": false,\n"
       << "  \"dynamics_changed\": false\n}\n";
}

bool json_bit(const std::filesystem::path& path, const std::string& key) {
  std::ifstream input(path);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find("\"" + key + "\": true") != std::string::npos;
}

void write_aggregate() {
  const auto directory = results_directory();
  const std::array<std::string, 3> slugs{{"face", "edge", "body"}};
  bool infrastructure = true, baseline = true, coherence = true;
  int transport_count = 0;
  bool all_passing_balanced = true;
  for (const auto& slug : slugs) {
    const auto path = directory
        / ("ftd_0761_m4_boosted_transport_v1_" + slug + ".json");
    infrastructure = infrastructure && std::filesystem::is_regular_file(path)
        && json_bit(path, "infrastructure_pass");
    baseline = baseline && json_bit(path, "baseline_pass");
    coherence = coherence && json_bit(path, "coherence_pass");
    const bool transport = json_bit(path, "transport_pass");
    transport_count += transport;
    if (transport) all_passing_balanced = all_passing_balanced
        && json_bit(path, "field_balanced");
  }
  std::string verdict;
  if (!infrastructure) verdict = "M4_BOOST_DISCOVERY_INFRASTRUCTURE_UNRESOLVED";
  else if (!baseline) verdict = "M4_BOOST_DISCOVERY_BASELINE_INVALID";
  else if (!coherence)
    verdict = "M4_BOOSTED_RELATIONAL_COHERENCE_CLOSED_AT_REGISTERED_SCALE";
  else if (transport_count == 0)
    verdict = "M4_BOOSTED_RELATIONAL_TRANSPORT_CLOSED_AT_REGISTERED_SCALE";
  else if (transport_count < 3)
    verdict = "M4_BOOSTED_RELATIONAL_TRANSPORT_ANISOTROPIC_WITNESS";
  else verdict = "M4_BOOSTED_RELATIONAL_TRANSPORT_WITNESS";
  if (transport_count > 0)
    verdict += all_passing_balanced
        ? "_FIELD_BALANCED" : "_SUBSTRATE_REACTION_UNRESOLVED";
  std::ofstream json(directory / "ftd_0761_m4_boosted_transport_v1.json");
  json << std::boolalpha
       << "{\n  \"ftd_id\": \"FTD-0761\",\n"
       << "  \"protocol_sha256\": \"" << kM4ProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"infrastructure_pass\": " << infrastructure << ",\n"
       << "  \"baseline_pass\": " << baseline << ",\n"
       << "  \"coherence_pass\": " << coherence << ",\n"
       << "  \"transport_direction_count\": " << transport_count << ",\n"
       << "  \"passing_directions_field_balanced\": "
       << all_passing_balanced << ",\n"
       << "  \"production_changed\": false,\n"
       << "  \"dynamics_changed\": false\n}\n";
}

int qualify() {
  Direction direction;
  if (!select_horizon_direction("face", direction)) return 1;
  const auto normalization = measure_face_flux_normalization();
  auto options = m3_options();
  M4Step formation_step;
  auto parent = build_parent(
      33, direction, options, normalization.mapped_field_work_coefficient,
      &formation_step);
  if (!parent.valid) return 1;
  auto arm = run_arm(parent.state, direction, +1, options,
      normalization.mapped_field_work_coefficient, &formation_step, 2);
  const bool pass = arm.initialized && arm.executed
      && arm.rows.size() == 3 && arm.rows.back().reverse_valid
      && arm.rows.back().reverse_recovery <= kReverseGate;
  std::cout << "FTD-0761 qualification pass=" << pass
            << " rows=" << arm.rows.size()
            << " initialized=" << arm.initialized
            << " executed=" << arm.executed
            << " exact=" << arm.exact;
  if (!arm.rows.empty()) {
    const auto& row = arm.rows.back();
    std::cout << " step=" << row.step_valid
              << " common=" << row.common
              << " member=" << row.member
              << " graph=" << row.graph_margin
              << " energy_margin=" << row.energy_margin
              << " residual=" << row.common_residual
              << " energy_residual=" << row.energy_residual
              << " observer=" << row.observer_valid
              << " ladder=" << row.ladder_valid
              << " regularity=" << row.regularity_measured
              << " sigma=" << row.sigma_min
              << " condition=" << row.condition_number
              << " reverse=" << row.reverse_valid
              << " reverse_recovery=" << row.reverse_recovery;
  }
  std::cout << '\n';
  return pass ? 0 : 1;
}

int run_registered(const std::string& slug) {
  if (std::string(kM4ProtocolSha256) == "UNLOCKED") return 3;
  Direction direction;
  if (!select_horizon_direction(slug, direction)) return 2;
  if (slug == "body") {
    for (const auto& prior : {"face", "edge"}) {
      const auto path = results_directory()
          / (std::string("ftd_0761_m4_boosted_transport_v1_")
             + prior + ".json");
      if (!std::filesystem::is_regular_file(path)) return 4;
    }
  }
  const auto normalization = measure_face_flux_normalization();
  if (!normalization.valid) return 1;
  auto options = m3_options();
  M4Step formation_step;
  auto parent = build_parent(
      kVolume, direction, options,
      normalization.mapped_field_work_coefficient, &formation_step);
  std::array<M4Arm, 3> arms;
  if (parent.valid) {
    arms[0] = run_arm(parent.state, direction, 0, options,
        normalization.mapped_field_work_coefficient, &formation_step);
    arms[1] = run_arm(parent.state, direction, +1, options,
        normalization.mapped_field_work_coefficient, &formation_step);
    arms[2] = run_arm(parent.state, direction, -1, options,
        normalization.mapped_field_work_coefficient, &formation_step);
  }
  auto summary = evaluate_direction(slug, direction, parent.valid,
                                    std::move(arms));
  write_direction(summary);
  if (slug == "body") write_aggregate();
  std::cout << "FTD-0761 direction=" << slug
            << " infrastructure=" << summary.infrastructure
            << " baseline=" << summary.baseline
            << " coherence=" << summary.coherence
            << " transport=" << summary.transport
            << " balanced=" << summary.field_balanced << '\n';
  return summary.infrastructure ? 0 : 1;
}

}  // namespace

int run(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "--qualify") return qualify();
  if (argc == 3 && std::string(argv[1]) == "--run")
    return run_registered(argv[2]);
  std::cout << "FTD-0761 runner: --qualify; --run face|edge|body\n";
  return argc == 1 ? 0 : 2;
}

}  // namespace ftd0761_embedded

int main(int argc, char** argv) {
  return ftd0761_embedded::run(argc, argv);
}
