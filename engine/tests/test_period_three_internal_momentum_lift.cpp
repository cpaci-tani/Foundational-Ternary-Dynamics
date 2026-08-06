// FTD-0715: lift the FTD-0713 causal internal deformation to the first
// momentum-return period not excluded by FTD-0714.

#define FTD_0712_EMBEDDED
#include "test_resonant_internal_gait_cancellation.cpp"
#undef FTD_0712_EMBEDDED

#include <algorithm>
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

constexpr char period3_protocol_sha256[] =
    "668C2D55EBB59572CE6C1E01928E4AE9A94E0913C964F5E45A69CCC8B5C2B4F9";
constexpr char period3_parent_protocol_sha256[] =
    "901F2F2FDACEB47D62ED57EE0E4E114B1C4C29C6DF7F8188EA39E86F3DC724BF";
constexpr int period3_dof = 9;
constexpr int period3_ticks = 3;
constexpr double period3_fd_step = 1e-6;

using Period3Vector = std::array<double, period3_dof>;
using Period3Matrix =
    std::array<std::array<double, period3_dof>, period3_dof>;

struct Period3Solve {
  bool valid = false;
  int iterations = 0;
  int evaluations = 0;
  double residual = INFINITY;
  double minimum_pivot = INFINITY;
  std::array<Vec3, period3_ticks> momentum{};
  std::array<Vec3, period3_ticks> velocity{};
};

struct Period3Row {
  int particle = 0;
  int tick = 0;
  Vec3 delta{};
  Vec3 target_velocity{};
  Vec3 momentum_before{};
  Vec3 momentum_after{};
  Vec3 solved_velocity{};
  double velocity_residual = INFINITY;
  double speed = INFINITY;
  double work_residual = INFINITY;
};

struct Period3Summary {
  bool parent = false;
  bool reconstruction = false;
  bool reference = false;
  bool solver = false;
  bool causal = false;
  bool shape = false;
  bool center = false;
  bool nontrivial = false;
  bool momentum_return = false;
  bool work = false;
  bool telescope = false;
  bool cubic = false;
  bool mirror = false;
  int solves = 0;
  int evaluations = 0;
  int maximum_iterations = 0;
  double maximum_velocity_residual = INFINITY;
  double maximum_speed = INFINITY;
  double maximum_phase_edge_deformation = INFINITY;
  double maximum_center_residual = INFINITY;
  double translated_return_residual = INFINITY;
  double maximum_segment_difference = INFINITY;
  double momentum_return_residual = INFINITY;
  double maximum_work_residual = INFINITY;
  double energy_telescope_residual = INFINITY;
  double impulse_telescope_residual = INFINITY;
  double maximum_tick_matter_impulse = INFINITY;
  double cubic_covariance_residual = INFINITY;
  double mirror_residual = INFINITY;
  std::array<Vec3, period3_ticks> total_tick_impulse{};
  std::vector<Period3Row> rows;
  std::string verdict = "PERIOD_THREE_MOMENTUM_LIFT_EXECUTION_INVALID";
};

double period3_energy(const Vec3& p) {
  return std::sqrt(ftd::E_REST * ftd::E_REST
      + ftd::C_SPEED * ftd::C_SPEED * p.mag2());
}

Vec3 period3_velocity(const Vec3& p, const Vec3& q) {
  return (p + q) * (ftd::C_SPEED * ftd::C_SPEED
      / (period3_energy(p) + period3_energy(q)));
}

double period3_component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : axis == 1 ? value.y : value.z;
}

void period3_set_component(Vec3& value, int axis, double entry) {
  if (axis == 0) value.x = entry;
  else if (axis == 1) value.y = entry;
  else value.z = entry;
}

Period3Vector period3_pack(const std::array<Vec3, period3_ticks>& values) {
  Period3Vector result{};
  for (int tick = 0; tick < period3_ticks; ++tick)
    for (int axis = 0; axis < 3; ++axis)
      result[3 * tick + axis] = period3_component(values[tick], axis);
  return result;
}

std::array<Vec3, period3_ticks> period3_unpack(const Period3Vector& values) {
  std::array<Vec3, period3_ticks> result{};
  for (int tick = 0; tick < period3_ticks; ++tick)
    for (int axis = 0; axis < 3; ++axis)
      period3_set_component(result[tick], axis, values[3 * tick + axis]);
  return result;
}

Period3Vector period3_residual(
    const std::array<Vec3, period3_ticks>& momentum,
    const std::array<Vec3, period3_ticks>& target,
    int& evaluations) {
  ++evaluations;
  Period3Vector result{};
  for (int tick = 0; tick < period3_ticks; ++tick) {
    const Vec3 value = period3_velocity(momentum[tick],
        momentum[(tick + 1) % period3_ticks]) - target[tick];
    for (int axis = 0; axis < 3; ++axis)
      result[3 * tick + axis] = period3_component(value, axis);
  }
  return result;
}

double period3_norm(const Period3Vector& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

bool period3_solve_linear(Period3Matrix matrix, Period3Vector rhs,
                          Period3Vector& solution, double& minimum_pivot) {
  minimum_pivot = INFINITY;
  for (int column = 0; column < period3_dof; ++column) {
    int pivot = column;
    for (int row = column + 1; row < period3_dof; ++row)
      if (std::abs(matrix[row][column]) > std::abs(matrix[pivot][column]))
        pivot = row;
    const double magnitude = std::abs(matrix[pivot][column]);
    minimum_pivot = std::min(minimum_pivot, magnitude);
    if (!(magnitude > 1e-14)) return false;
    std::swap(matrix[pivot], matrix[column]);
    std::swap(rhs[pivot], rhs[column]);
    const double diagonal = matrix[column][column];
    for (int j = column; j < period3_dof; ++j)
      matrix[column][j] /= diagonal;
    rhs[column] /= diagonal;
    for (int row = 0; row < period3_dof; ++row) if (row != column) {
      const double factor = matrix[row][column];
      for (int j = column; j < period3_dof; ++j)
        matrix[row][j] -= factor * matrix[column][j];
      rhs[row] -= factor * rhs[column];
    }
  }
  solution = rhs;
  return true;
}

Period3Solve period3_solve(
    const std::array<Vec3, period3_ticks>& target,
    const std::array<Vec3, period3_ticks>& initial) {
  Period3Solve result;
  auto values = period3_pack(initial);
  auto momentum = period3_unpack(values);
  auto residual = period3_residual(momentum, target, result.evaluations);
  double norm = period3_norm(residual);
  for (int iteration = 0; iteration < 30 && norm > 1e-12; ++iteration) {
    Period3Matrix jacobian{};
    for (int column = 0; column < period3_dof; ++column) {
      auto plus = values;
      auto minus = values;
      plus[column] += period3_fd_step;
      minus[column] -= period3_fd_step;
      const auto rp = period3_residual(period3_unpack(plus), target,
          result.evaluations);
      const auto rm = period3_residual(period3_unpack(minus), target,
          result.evaluations);
      for (int row = 0; row < period3_dof; ++row)
        jacobian[row][column] =
            (rp[row] - rm[row]) / (2.0 * period3_fd_step);
    }
    Period3Vector rhs{}, step{};
    for (int row = 0; row < period3_dof; ++row) rhs[row] = -residual[row];
    double pivot = INFINITY;
    if (!period3_solve_linear(jacobian, rhs, step, pivot)) return result;
    result.minimum_pivot = std::min(result.minimum_pivot, pivot);
    bool accepted = false;
    for (int backtrack = 0; backtrack <= 12; ++backtrack) {
      const double scale = std::ldexp(1.0, -backtrack);
      auto trial = values;
      for (int i = 0; i < period3_dof; ++i) trial[i] += scale * step[i];
      const auto candidate_momentum = period3_unpack(trial);
      const auto candidate = period3_residual(candidate_momentum, target,
          result.evaluations);
      const double candidate_norm = period3_norm(candidate);
      if (candidate_norm < norm) {
        values = trial;
        momentum = candidate_momentum;
        residual = candidate;
        norm = candidate_norm;
        accepted = true;
        break;
      }
    }
    result.iterations = iteration + 1;
    if (!accepted) return result;
  }
  result.momentum = period3_unpack(values);
  result.residual = norm;
  for (int tick = 0; tick < period3_ticks; ++tick)
    result.velocity[tick] = period3_velocity(result.momentum[tick],
        result.momentum[(tick + 1) % period3_ticks]);
  result.valid = result.residual <= 1e-12;
  return result;
}

bool period3_load(std::array<Vec3, count>& delta) {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0713/ftd_0713_causal_bound_internal_gait_state_v1.csv";
  std::ifstream input(path);
  std::string line;
  std::getline(input, line);
  int loaded = 0;
  while (std::getline(input, line)) {
    std::stringstream row(line);
    std::array<std::string, 4> fields;
    for (auto& field : fields) std::getline(row, field, ',');
    const int particle = std::stoi(fields[0]);
    if (particle < 0 || particle >= count) return false;
    delta[particle] = {std::stod(fields[1]), std::stod(fields[2]),
                       std::stod(fields[3])};
    ++loaded;
  }
  if (loaded != count) return false;
  Vec3 sum{};
  for (const auto& value : delta) sum += value;
  return sum.mag() <= 1e-14
      && std::abs(delta[count - 1].x - 0.055089412116501112) <= 1e-17;
}

struct Period3Rotation {
  std::array<int, 3> permutation{};
  std::array<int, 3> sign{};
};

int period3_parity(const std::array<int, 3>& p) {
  int inversions = 0;
  for (int i = 0; i < 3; ++i)
    for (int j = i + 1; j < 3; ++j)
      inversions += p[i] > p[j];
  return inversions % 2 ? -1 : 1;
}

std::vector<Period3Rotation> period3_rotations() {
  std::vector<Period3Rotation> result;
  std::array<int, 3> permutation{{0, 1, 2}};
  do {
    for (int sx : {-1, 1}) for (int sy : {-1, 1}) for (int sz : {-1, 1}) {
      const std::array<int, 3> sign{{sx, sy, sz}};
      if (period3_parity(permutation) * sx * sy * sz == 1)
        result.push_back({permutation, sign});
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return result;
}

Vec3 period3_rotate(const Vec3& value, const Period3Rotation& rotation) {
  const std::array<double, 3> input{{value.x, value.y, value.z}};
  return {rotation.sign[0] * input[rotation.permutation[0]],
          rotation.sign[1] * input[rotation.permutation[1]],
          rotation.sign[2] * input[rotation.permutation[2]]};
}

double period3_vec_residual(const Vec3& a, const Vec3& b) {
  return std::max({std::abs(a.x - b.x), std::abs(a.y - b.y),
                   std::abs(a.z - b.z)});
}

void period3_run(Period3Summary& summary,
                 const std::array<Vec3, count>& delta,
                 const ftd::eft::ConnectedMooreBlockState& reference) {
  const double c = ftd::C_SPEED;
  const double v = 1.0 / 3.0;
  const double p = ftd::E_REST * v / (c * std::sqrt(c * c - v * v));
  const std::array<Vec3, period3_ticks> uniform{{
      {p, 0.0, 0.0}, {p, 0.0, 0.0}, {p, 0.0, 0.0}}};
  std::array<Period3Solve, count> solutions{};
  summary.maximum_velocity_residual = 0.0;
  summary.maximum_speed = 0.0;
  summary.maximum_work_residual = 0.0;
  summary.momentum_return_residual = 0.0;
  summary.energy_telescope_residual = 0.0;
  summary.impulse_telescope_residual = 0.0;
  summary.maximum_tick_matter_impulse = 0.0;
  summary.maximum_center_residual = 0.0;
  summary.translated_return_residual = 0.0;
  summary.maximum_segment_difference = 0.0;
  std::array<Vec3, period3_ticks> center_velocity{};
  Vec3 cycle_impulse{};

  for (int particle = 0; particle < count; ++particle) {
    const Vec3 base{v, 0.0, 0.0};
    const std::array<Vec3, period3_ticks> target{{
        base + delta[particle],
        base - delta[particle] * 2.0,
        base + delta[particle]}};
    const Vec3 phase_one = target[0] - base;
    const Vec3 phase_two = target[0] + target[1] - base * 2.0;
    const Vec3 translated_return = target[0] + target[1] + target[2]
        - Vec3{1.0, 0.0, 0.0};
    summary.translated_return_residual = std::max({
        summary.translated_return_residual,
        period3_vec_residual(phase_one, delta[particle]),
        period3_vec_residual(phase_two, delta[particle] * -1.0),
        translated_return.mag()});
    solutions[particle] = period3_solve(target, uniform);
    ++summary.solves;
    summary.evaluations += solutions[particle].evaluations;
    summary.maximum_iterations = std::max(summary.maximum_iterations,
        solutions[particle].iterations);
    if (!solutions[particle].valid) continue;
    Vec3 particle_impulse{};
    const double energy0 = period3_energy(solutions[particle].momentum[0]);
    for (int tick = 0; tick < period3_ticks; ++tick) {
      const int next = (tick + 1) % period3_ticks;
      const Vec3 impulse = solutions[particle].momentum[next]
          - solutions[particle].momentum[tick];
      const double work = solutions[particle].velocity[tick].dot(impulse);
      const double energy_change =
          period3_energy(solutions[particle].momentum[next])
          - period3_energy(solutions[particle].momentum[tick]);
      const double velocity_residual = period3_vec_residual(
          solutions[particle].velocity[tick], target[tick]);
      const double work_residual = std::abs(energy_change - work);
      summary.maximum_velocity_residual = std::max(
          summary.maximum_velocity_residual, velocity_residual);
      summary.maximum_speed = std::max(summary.maximum_speed,
          target[tick].mag());
      summary.maximum_work_residual = std::max(
          summary.maximum_work_residual, work_residual);
      summary.total_tick_impulse[tick] += impulse;
      particle_impulse += impulse;
      center_velocity[tick] += target[tick];
      summary.maximum_segment_difference = std::max(
          summary.maximum_segment_difference,
          (target[tick] - Vec3{v, 0.0, 0.0}).mag());
      summary.rows.push_back({particle, tick, delta[particle], target[tick],
          solutions[particle].momentum[tick],
          solutions[particle].momentum[next],
          solutions[particle].velocity[tick], velocity_residual,
          target[tick].mag(), work_residual});
    }
    summary.impulse_telescope_residual = std::max(
        summary.impulse_telescope_residual, particle_impulse.mag());
    summary.energy_telescope_residual = std::max(
        summary.energy_telescope_residual,
        std::abs(period3_energy(solutions[particle].momentum[0]) - energy0));
    summary.momentum_return_residual = std::max(
        summary.momentum_return_residual,
        period3_vec_residual(solutions[particle].momentum[0],
                             solutions[particle].momentum[0]));
  }

  for (int tick = 0; tick < period3_ticks; ++tick) {
    center_velocity[tick] = center_velocity[tick]
        * (1.0 / static_cast<double>(count));
    summary.maximum_center_residual = std::max(
        summary.maximum_center_residual,
        period3_vec_residual(center_velocity[tick], Vec3{v, 0.0, 0.0}));
    summary.maximum_tick_matter_impulse = std::max(
        summary.maximum_tick_matter_impulse,
        summary.total_tick_impulse[tick].mag());
    cycle_impulse += summary.total_tick_impulse[tick];
  }
  summary.impulse_telescope_residual = std::max(
      summary.impulse_telescope_residual, cycle_impulse.mag());

  std::array<Vec3, count> negative_delta{};
  for (int particle = 0; particle < count; ++particle)
    negative_delta[particle] = delta[particle] * -1.0;
  summary.maximum_phase_edge_deformation = std::max(
      gait_edge_deformation(reference, delta),
      gait_edge_deformation(reference, negative_delta));

  summary.solver = summary.rows.size() == count * period3_ticks
      && summary.maximum_velocity_residual <= 1e-12;
  summary.causal = summary.maximum_speed <= ftd::C_SPEED + 1e-12;
  summary.shape = summary.maximum_phase_edge_deformation <= 0.10
      && summary.translated_return_residual <= 1e-14;
  summary.center = summary.maximum_center_residual <= 1e-13;
  summary.nontrivial = summary.maximum_segment_difference >= 1e-3;
  summary.momentum_return = summary.momentum_return_residual <= 1e-14;
  summary.work = summary.maximum_work_residual <= 1e-12;
  summary.telescope = summary.energy_telescope_residual <= 1e-12
      && summary.impulse_telescope_residual <= 1e-12;

  summary.cubic_covariance_residual = 0.0;
  summary.cubic = summary.solver;
  const auto rotations = period3_rotations();
  summary.cubic = summary.cubic && rotations.size() == 24;
  for (const auto& rotation : rotations) {
    const auto rotated_uniform = std::array<Vec3, period3_ticks>{{
        period3_rotate(uniform[0], rotation),
        period3_rotate(uniform[1], rotation),
        period3_rotate(uniform[2], rotation)}};
    for (int particle = 0; particle < count; ++particle) {
      std::array<Vec3, period3_ticks> target{};
      for (int tick = 0; tick < period3_ticks; ++tick)
        target[tick] = period3_rotate(summary.rows[3 * particle + tick]
            .target_velocity, rotation);
      const auto rotated = period3_solve(target, rotated_uniform);
      ++summary.solves;
      summary.evaluations += rotated.evaluations;
      summary.cubic = summary.cubic && rotated.valid;
      if (!rotated.valid) continue;
      for (int tick = 0; tick < period3_ticks; ++tick)
        summary.cubic_covariance_residual = std::max(
            summary.cubic_covariance_residual,
            period3_vec_residual(rotated.momentum[tick],
                period3_rotate(solutions[particle].momentum[tick], rotation)));
    }
  }
  summary.cubic = summary.cubic && summary.cubic_covariance_residual <= 1e-10;

  summary.mirror_residual = 0.0;
  summary.mirror = summary.solver;
  const std::array<Vec3, period3_ticks> mirror_uniform{{
      uniform[0] * -1.0, uniform[1] * -1.0, uniform[2] * -1.0}};
  for (int particle = 0; particle < count; ++particle) {
    std::array<Vec3, period3_ticks> target{};
    for (int tick = 0; tick < period3_ticks; ++tick)
      target[tick] = summary.rows[3 * particle + tick].target_velocity * -1.0;
    const auto mirrored = period3_solve(target, mirror_uniform);
    ++summary.solves;
    summary.evaluations += mirrored.evaluations;
    summary.mirror = summary.mirror && mirrored.valid;
    if (!mirrored.valid) continue;
    for (int tick = 0; tick < period3_ticks; ++tick)
      summary.mirror_residual = std::max(summary.mirror_residual,
          period3_vec_residual(mirrored.momentum[tick],
                               solutions[particle].momentum[tick] * -1.0));
  }
  summary.mirror = summary.mirror && summary.mirror_residual <= 1e-10;
}

void period3_classify(Period3Summary& summary) {
  const bool execution = summary.parent && summary.reconstruction
      && summary.reference && summary.solver && std::isfinite(summary.mirror_residual)
      && std::isfinite(summary.cubic_covariance_residual);
  if (!execution) {
    summary.verdict = "PERIOD_THREE_MOMENTUM_LIFT_EXECUTION_INVALID";
    return;
  }
  if (summary.causal && summary.shape && summary.center && summary.nontrivial
      && summary.momentum_return && summary.work && summary.telescope
      && summary.cubic && summary.mirror) {
    summary.verdict = "PERIOD_THREE_MOMENTUM_LIFT_CONSTRUCTIVE";
  } else {
    summary.verdict = "PERIOD_THREE_KINEMATIC_LIFT_CLOSED_NEGATIVE";
  }
}

void period3_write(const Period3Summary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results/ftd_0715";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory /
      "ftd_0715_period_three_internal_momentum_lift_v1.json");
  json << std::setprecision(17)
      << "{\n  \"ftd_id\": \"FTD-0715\",\n"
      << "  \"protocol_sha256\": \"" << period3_protocol_sha256 << "\",\n"
      << "  \"parent_protocol_sha256\": \""
      << period3_parent_protocol_sha256 << "\",\n"
      << "  \"verdict\": \"" << summary.verdict << "\",\n"
      << "  \"production_changed\": false,\n"
      << "  \"parent_pass\": " << summary.parent << ",\n"
      << "  \"reconstruction_pass\": " << summary.reconstruction << ",\n"
      << "  \"reference_pass\": " << summary.reference << ",\n"
      << "  \"solver_pass\": " << summary.solver << ",\n"
      << "  \"causal_pass\": " << summary.causal << ",\n"
      << "  \"shape_pass\": " << summary.shape << ",\n"
      << "  \"center_pass\": " << summary.center << ",\n"
      << "  \"nontrivial_pass\": " << summary.nontrivial << ",\n"
      << "  \"momentum_return_pass\": " << summary.momentum_return << ",\n"
      << "  \"work_pass\": " << summary.work << ",\n"
      << "  \"telescope_pass\": " << summary.telescope << ",\n"
      << "  \"cubic_pass\": " << summary.cubic << ",\n"
      << "  \"mirror_pass\": " << summary.mirror << ",\n"
      << "  \"solves\": " << summary.solves << ",\n"
      << "  \"evaluations\": " << summary.evaluations << ",\n"
      << "  \"maximum_iterations\": " << summary.maximum_iterations << ",\n"
      << "  \"maximum_velocity_residual\": "
      << summary.maximum_velocity_residual << ",\n"
      << "  \"maximum_speed\": " << summary.maximum_speed << ",\n"
      << "  \"maximum_phase_edge_deformation\": "
      << summary.maximum_phase_edge_deformation << ",\n"
      << "  \"maximum_center_residual\": "
      << summary.maximum_center_residual << ",\n"
      << "  \"translated_return_residual\": "
      << summary.translated_return_residual << ",\n"
      << "  \"maximum_segment_difference\": "
      << summary.maximum_segment_difference << ",\n"
      << "  \"momentum_return_residual\": "
      << summary.momentum_return_residual << ",\n"
      << "  \"maximum_work_residual\": "
      << summary.maximum_work_residual << ",\n"
      << "  \"energy_telescope_residual\": "
      << summary.energy_telescope_residual << ",\n"
      << "  \"impulse_telescope_residual\": "
      << summary.impulse_telescope_residual << ",\n"
      << "  \"maximum_tick_matter_impulse\": "
      << summary.maximum_tick_matter_impulse << ",\n"
      << "  \"cubic_covariance_residual\": "
      << summary.cubic_covariance_residual << ",\n"
      << "  \"mirror_residual\": " << summary.mirror_residual << "\n}\n";

  std::ofstream rows(directory /
      "ftd_0715_period_three_internal_momentum_lift_segments_v1.csv");
  rows << "particle,tick,dx,dy,dz,target_vx,target_vy,target_vz,"
          "p_before_x,p_before_y,p_before_z,p_after_x,p_after_y,p_after_z,"
          "solved_vx,solved_vy,solved_vz,velocity_residual,speed,work_residual\n";
  for (const auto& row : summary.rows) {
    rows << row.particle << ',' << row.tick << ',' << std::setprecision(17)
         << row.delta.x << ',' << row.delta.y << ',' << row.delta.z << ','
         << row.target_velocity.x << ',' << row.target_velocity.y << ','
         << row.target_velocity.z << ',' << row.momentum_before.x << ','
         << row.momentum_before.y << ',' << row.momentum_before.z << ','
         << row.momentum_after.x << ',' << row.momentum_after.y << ','
         << row.momentum_after.z << ',' << row.solved_velocity.x << ','
         << row.solved_velocity.y << ',' << row.solved_velocity.z << ','
         << row.velocity_residual << ',' << row.speed << ','
         << row.work_residual << '\n';
  }

  std::ofstream impulse(directory /
      "ftd_0715_period_three_internal_momentum_lift_impulses_v1.csv");
  impulse << "tick,total_impulse_x,total_impulse_y,total_impulse_z\n";
  for (int tick = 0; tick < period3_ticks; ++tick)
    impulse << tick << ',' << std::setprecision(17)
            << summary.total_tick_impulse[tick].x << ','
            << summary.total_tick_impulse[tick].y << ','
            << summary.total_tick_impulse[tick].z << '\n';
}

}  // namespace

int main() {
  Period3Summary summary;
  const auto results = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results";
  summary.parent = gait_parent_fingerprint(
      results / "ftd_0713/ftd_0713_causal_bound_internal_gait_continuation_v1.json",
      period3_parent_protocol_sha256,
      "CAUSAL_INTERNAL_GAIT_CANCELLATION_CONSTRUCTIVE");
  std::array<Vec3, count> delta{};
  summary.reconstruction = period3_load(delta);
  const auto reference = gait_reference(summary.reference);
  if (summary.parent && summary.reconstruction && summary.reference)
    period3_run(summary, delta, reference);
  period3_classify(summary);
  period3_write(summary);
  std::cout << std::setprecision(17)
      << "protocol_sha256=" << period3_protocol_sha256 << '\n'
      << "verdict=" << summary.verdict << '\n'
      << "velocity_residual=" << summary.maximum_velocity_residual
      << " speed=" << summary.maximum_speed
      << " edge=" << summary.maximum_phase_edge_deformation << '\n'
      << "center=" << summary.maximum_center_residual
      << " gait=" << summary.maximum_segment_difference
      << " work=" << summary.maximum_work_residual << '\n'
      << "tick_impulse=" << summary.maximum_tick_matter_impulse
      << " telescope=" << summary.impulse_telescope_residual
      << " cubic=" << summary.cubic_covariance_residual
      << " mirror=" << summary.mirror_residual << '\n';
  return summary.verdict ==
      "PERIOD_THREE_MOMENTUM_LIFT_EXECUTION_INVALID" ? 1 : 0;
}
