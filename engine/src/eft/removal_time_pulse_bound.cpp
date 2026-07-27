#include "ftd/eft/removal_time_pulse_bound.h"

#include "ftd/constants.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace ftd::eft {
namespace {

using Matrix3 = std::array<std::array<int, 3>, 3>;

constexpr double TOL = 1e-12;
constexpr int RUN_TICKS = 128;
constexpr std::array<int, 4> SPECTRAL_VOLUMES{{9, 17, 33, 65}};
constexpr std::array<int, 2> LIVE_VOLUMES{{9, 17}};

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double distance(const Vec3& lhs, const Vec3& rhs) {
  return (lhs - rhs).mag();
}

Coord add(Coord lhs, Coord rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Coord subtract(Coord lhs, Coord rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Vec3 rotate(const Matrix3& matrix, const Vec3& value) {
  const std::array<double, 3> input{{value.x, value.y, value.z}};
  std::array<double, 3> output{};
  for (int row = 0; row < 3; ++row)
    for (int column = 0; column < 3; ++column)
      output[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)]
                [static_cast<std::size_t>(column)]
          * input[static_cast<std::size_t>(column)];
  return {output[0], output[1], output[2]};
}

Coord rotate(const Matrix3& matrix, Coord value) {
  const std::array<int, 3> input{{value.x, value.y, value.z}};
  std::array<int, 3> output{};
  for (int row = 0; row < 3; ++row)
    for (int column = 0; column < 3; ++column)
      output[static_cast<std::size_t>(row)] +=
          matrix[static_cast<std::size_t>(row)]
                [static_cast<std::size_t>(column)]
          * input[static_cast<std::size_t>(column)];
  return {output[0], output[1], output[2]};
}

std::vector<Matrix3> proper_rotations() {
  std::vector<Matrix3> result;
  std::array<int, 3> permutation{{0, 1, 2}};
  do {
    int inversions = 0;
    for (int i = 0; i < 3; ++i)
      for (int j = i + 1; j < 3; ++j)
        if (permutation[static_cast<std::size_t>(i)]
            > permutation[static_cast<std::size_t>(j)]) ++inversions;
    const int parity = inversions % 2 == 0 ? 1 : -1;
    for (int sx : {-1, 1}) for (int sy : {-1, 1}) for (int sz : {-1, 1}) {
      if (parity * sx * sy * sz != 1) continue;
      Matrix3 matrix{};
      const std::array<int, 3> signs{{sx, sy, sz}};
      for (int row = 0; row < 3; ++row) {
        matrix[static_cast<std::size_t>(row)]
              [static_cast<std::size_t>(
                  permutation[static_cast<std::size_t>(row)])] =
            signs[static_cast<std::size_t>(row)];
      }
      result.push_back(matrix);
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));
  return result;
}

std::vector<Coord> source_sites(int L, int source_count, int variant) {
  const Coord center{L / 2, L / 2, L / 2};
  std::vector<Coord> sites;
  sites.reserve(static_cast<std::size_t>(source_count));
  if (source_count == 5) {
    const std::array<Coord, 4> positive{{
        {1, 1, 1}, {1, -1, -1}, {-1, 1, -1}, {-1, -1, 1}}};
    const std::array<Coord, 4> negative{{
        {1, 1, -1}, {1, -1, 1}, {-1, 1, 1}, {-1, -1, -1}}};
    const auto& orbit = variant == 0 ? positive : negative;
    for (Coord offset : orbit) sites.push_back(add(center, offset));
    sites.push_back(center);
    return sites;
  }

  const int radius = variant == 0 ? 1 : 2;
  for (Coord offset : std::array<Coord, 6>{{
           {radius, 0, 0}, {-radius, 0, 0},
           {0, radius, 0}, {0, -radius, 0},
           {0, 0, radius}, {0, 0, -radius}}}) {
    sites.push_back(add(center, offset));
  }
  return sites;
}

std::vector<int> removal_ticks(RemovalHistoryKind history,
                               int source_count) {
  std::vector<int> ticks(static_cast<std::size_t>(source_count), -1);
  if (history == RemovalHistoryKind::SynchronousPulse) {
    std::fill(ticks.begin(), ticks.end(), 16);
  } else if (history == RemovalHistoryKind::StaggeredPulse) {
    for (int j = 0; j < source_count; ++j)
      ticks[static_cast<std::size_t>(j)] = 4 * (j + 1);
  } else if (history == RemovalHistoryKind::PairedPulse) {
    const std::array<int, 6> paired{{8, 8, 16, 16, 24, 24}};
    for (int j = 0; j < source_count; ++j)
      ticks[static_cast<std::size_t>(j)] = paired[static_cast<std::size_t>(j)];
  }
  return ticks;
}

std::pair<double, int> maximum_history_bound(
    const RemovalTimePulseVolume& volume, int source_count) {
  double maximum = 0.0;
  int maximizing_removed = 0;
  for (int removed = 0; removed <= source_count; ++removed) {
    const double bound = volume.common_step_coefficient
            * std::sqrt(static_cast<double>(source_count - removed))
        + removed * volume.exact_one_source_pulse_bound;
    if (bound > maximum) {
      maximum = bound;
      maximizing_removed = removed;
    }
  }
  return {maximum, maximizing_removed};
}

RemovalTimePulseVolume spectral_bound(int L) {
  RemovalTimePulseVolume result;
  result.lattice_size = L;
  const long double pi = std::acos(-1.0L);
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  long double step_sum = 0.0L;
  long double pulse_sum = 0.0L;
  long double common_sum = 0.0L;
  long double maximum_a = 0.0L;
  for (int nx = 0; nx < L; ++nx) {
    const long double kx = 2.0L * pi * nx / L;
    const long double cx = std::cos(kx);
    const long double sx = std::sin(kx);
    for (int ny = 0; ny < L; ++ny) {
      const long double ky = 2.0L * pi * ny / L;
      const long double cy = std::cos(ky);
      const long double sy = std::sin(ky);
      for (int nz = 0; nz < L; ++nz) {
        const long double kz = 2.0L * pi * nz / L;
        const long double cz = std::cos(kz);
        const long double sz = std::sin(kz);
        const long double symbol = 4.0L
            - (2.0L / 3.0L) * (cx + cy + cz)
            - (2.0L / 3.0L) * (cx * cy + cx * cz + cy * cz);
        if (symbol <= 1e-24L) continue;
        const long double gradient =
            std::sqrt(sx * sx + sy * sy + sz * sz);
        const long double a = c2 * symbol;
        const long double secant =
            1.0L / std::sqrt(1.0L - a / 4.0L);
        const long double step_envelope = 1.0L + secant;
        maximum_a = std::max(maximum_a, a);
        step_sum += step_envelope * gradient / symbol;
        pulse_sum += 2.0L * secant * gradient / symbol;
        common_sum += step_envelope * step_envelope / symbol;
      }
    }
  }

  const long double volume = static_cast<long double>(L) * L * L;
  const long double coupling = static_cast<long double>(G_C);
  result.maximum_mode_eigenvalue = static_cast<double>(maximum_a);
  result.one_source_step_triangle_bound = static_cast<double>(
      coupling * step_sum / (c2 * volume));
  result.exact_one_source_pulse_bound = static_cast<double>(
      coupling * pulse_sum / (c2 * volume));
  result.common_step_coefficient = static_cast<double>(
      coupling / c2 * std::sqrt(common_sum / volume));

  int count = 1;
  while (maximum_history_bound(result, count).first + TOL < K_GENESIS)
    ++count;
  result.uniform_closed_source_count = count - 1;
  result.first_source_count_not_excluded = count;
  const auto closed = maximum_history_bound(result, count - 1);
  const auto open = maximum_history_bound(result, count);
  result.closed_count_history_bound = closed.first;
  result.maximizing_removed_at_closed_count = closed.second;
  result.closed_count_margin = K_GENESIS - closed.first;
  result.first_open_count_history_bound = open.first;
  result.maximizing_removed_at_first_open_count = open.second;
  result.first_open_count_margin = K_GENESIS - open.first;
  result.continuous_relaxation_at_closed_count =
      (count - 1) * result.exact_one_source_pulse_bound
      + result.common_step_coefficient * result.common_step_coefficient
          / (4.0 * result.exact_one_source_pulse_bound);
  return result;
}

long double step_response(long double theta, int n) {
  if (n < 0) return 0.0L;
  return 1.0L - std::cos(n * theta)
      + std::tan(theta / 2.0L) * std::sin(n * theta);
}

long double pulse_response(long double theta, int n, int removal_tick) {
  return step_response(theta, n)
      - step_response(theta, n - removal_tick);
}

long double factored_pulse_response(long double theta, int n,
                                    int removal_tick) {
  return 2.0L / std::cos(theta / 2.0L)
      * std::sin(removal_tick * theta / 2.0L)
      * std::sin((n - (removal_tick - 1.0L) / 2.0L) * theta);
}

Vec3 removal_kernel(int L, Coord displacement, int observation_tick,
                    int removal_tick) {
  const long double pi = std::acos(-1.0L);
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  long double x = 0.0L;
  long double y = 0.0L;
  long double z = 0.0L;
  for (int nx = 0; nx < L; ++nx) {
    const long double kx = 2.0L * pi * nx / L;
    const long double cx = std::cos(kx);
    const long double sx = std::sin(kx);
    for (int ny = 0; ny < L; ++ny) {
      const long double ky = 2.0L * pi * ny / L;
      const long double cy = std::cos(ky);
      const long double sy = std::sin(ky);
      for (int nz = 0; nz < L; ++nz) {
        const long double kz = 2.0L * pi * nz / L;
        const long double cz = std::cos(kz);
        const long double sz = std::sin(kz);
        const long double symbol = 4.0L
            - (2.0L / 3.0L) * (cx + cy + cz)
            - (2.0L / 3.0L) * (cx * cy + cx * cz + cy * cz);
        if (symbol <= 1e-24L) continue;
        const long double theta = std::acos(1.0L - c2 * symbol / 2.0L);
        const long double response = factored_pulse_response(
            theta, observation_tick, removal_tick);
        const long double phase = kx * displacement.x
            + ky * displacement.y + kz * displacement.z;
        const long double factor = std::sin(phase) * response / symbol;
        x += sx * factor;
        y += sy * factor;
        z += sz * factor;
      }
    }
  }
  const long double volume = static_cast<long double>(L) * L * L;
  const long double scale = static_cast<long double>(G_C) / (c2 * volume);
  return {static_cast<double>(scale * x),
          static_cast<double>(scale * y),
          static_cast<double>(scale * z)};
}

void verify_pulse_identity(RemovalTimePulseBoundResult& result) {
  const long double pi = std::acos(-1.0L);
  const int L = 9;
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  for (int nx = 0; nx < L; ++nx) {
    const long double kx = 2.0L * pi * nx / L;
    const long double cx = std::cos(kx);
    for (int ny = 0; ny < L; ++ny) {
      const long double ky = 2.0L * pi * ny / L;
      const long double cy = std::cos(ky);
      for (int nz = 0; nz < L; ++nz) {
        const long double kz = 2.0L * pi * nz / L;
        const long double cz = std::cos(kz);
        const long double symbol = 4.0L
            - (2.0L / 3.0L) * (cx + cy + cz)
            - (2.0L / 3.0L) * (cx * cy + cx * cz + cy * cz);
        if (symbol <= 1e-24L) continue;
        const long double theta = std::acos(1.0L - c2 * symbol / 2.0L);
        for (int removal_tick : {1, 4, 16, 24}) {
          for (int observation_tick : {removal_tick, removal_tick + 1,
                                       removal_tick + 17}) {
            const long double direct = pulse_response(
                theta, observation_tick, removal_tick);
            const long double factored = factored_pulse_response(
                theta, observation_tick, removal_tick);
            result.maximum_pulse_identity_residual = std::max(
                result.maximum_pulse_identity_residual,
                static_cast<double>(std::abs(direct - factored)));
            ++result.pulse_identity_checks;
          }
        }
      }
    }
  }
  result.exact_pulse_identity_derived =
      result.maximum_pulse_identity_residual <= TOL;
}

void verify_gram_and_covariance(RemovalTimePulseBoundResult& result) {
  for (int L : LIVE_VOLUMES) {
    for (int source_count : {5, 6}) {
      for (int variant : {0, 1}) {
        for (RemovalHistoryKind history : {
                 RemovalHistoryKind::SynchronousPulse,
                 RemovalHistoryKind::StaggeredPulse,
                 RemovalHistoryKind::PairedPulse}) {
          const auto sites = source_sites(L, source_count, variant);
          const auto ticks = removal_ticks(history, source_count);
          for (int translation : {0, 1}) {
            const Coord shift{translation, -translation, 2 * translation};
            const Coord observation{L / 2 + 2 + shift.x,
                                    L / 2 + 1 + shift.y,
                                    L / 2 - 1 + shift.z};
            std::vector<Vec3> kernels;
            Vec3 direct{};
            for (int j = 0; j < source_count; ++j) {
              const Coord shifted_site = add(
                  sites[static_cast<std::size_t>(j)], shift);
              const Vec3 kernel = removal_kernel(
                  L, subtract(observation, shifted_site), 64,
                  ticks[static_cast<std::size_t>(j)]);
              kernels.push_back(kernel);
              direct += kernel;
            }
            long double gram = 0.0L;
            for (const Vec3& lhs : kernels)
              for (const Vec3& rhs : kernels)
                gram += static_cast<long double>(lhs.dot(rhs));
            const long double norm2 = direct.dot(direct);
            result.maximum_gram_residual = std::max(
                result.maximum_gram_residual,
                static_cast<double>(std::abs(norm2 - gram)));
            ++result.gram_checks;

            const Coord unshifted_observation{L / 2 + 2, L / 2 + 1,
                                              L / 2 - 1};
            Vec3 untranslated{};
            for (int j = 0; j < source_count; ++j) {
              untranslated += removal_kernel(
                  L, subtract(unshifted_observation,
                              sites[static_cast<std::size_t>(j)]),
                  64, ticks[static_cast<std::size_t>(j)]);
            }
            result.maximum_translation_residual = std::max(
                result.maximum_translation_residual,
                distance(direct, untranslated));
          }
        }
      }
    }
  }

  const int L = 17;
  const Coord center{L / 2, L / 2, L / 2};
  const Coord observation_offset{2, 1, -1};
  const auto sites = source_sites(L, 5, 0);
  const auto ticks = removal_ticks(RemovalHistoryKind::StaggeredPulse, 5);
  Vec3 base{};
  for (int j = 0; j < 5; ++j) {
    base += removal_kernel(
        L, subtract(add(center, observation_offset),
                    sites[static_cast<std::size_t>(j)]),
        64, ticks[static_cast<std::size_t>(j)]);
  }
  const auto rotations = proper_rotations();
  for (const auto& rotation : rotations) {
    const Coord rotated_observation = add(center, rotate(rotation,
                                                         observation_offset));
    Vec3 transformed{};
    for (int j = 0; j < 5; ++j) {
      const Coord offset = subtract(sites[static_cast<std::size_t>(j)], center);
      const Coord rotated_site = add(center, rotate(rotation, offset));
      transformed += removal_kernel(
          L, subtract(rotated_observation, rotated_site), 64,
          ticks[static_cast<std::size_t>(j)]);
    }
    result.maximum_cubic_covariance_residual = std::max(
        result.maximum_cubic_covariance_residual,
        distance(transformed, rotate(rotation, base)));
    ++result.proper_cubic_rotation_arms;
  }

  result.gram_identity_verified = result.gram_checks == 48
      && result.maximum_gram_residual <= TOL;
  result.translation_covariant = result.maximum_translation_residual <= TOL;
  result.cubic_covariant = result.proper_cubic_rotation_arms == 24
      && result.maximum_cubic_covariance_residual <= TOL;
}

void configure_bridge(RenderBridge& bridge, std::uint32_t seed) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = true;
  bridge.toggles.coupling = true;
  bridge.toggles.genesis = true;
  bridge.toggles.dual_substrate = false;
  bridge.toggles.flux_boundary = FluxBoundaryMode::Periodic;
  bridge.toggles.langevin_seed = seed;
  bridge.seed_rng(seed);
}

int originals_remaining(const RenderBridge& bridge,
                        const std::vector<int>& original_indices) {
  int remaining = 0;
  for (int index : original_indices) {
    if (bridge.voxels()[static_cast<std::size_t>(index)].state != 0)
      ++remaining;
  }
  return remaining;
}

RemovalTimePulseArm run_arm(
    int L, int source_count, int polarity, int variant,
    RemovalHistoryKind history, std::uint32_t seed,
    const RemovalTimePulseVolume& volume) {
  RemovalTimePulseArm result;
  result.history = history;
  result.lattice_size = L;
  result.source_count = source_count;
  result.polarity = polarity;
  result.shape_variant = variant;
  result.seed = seed;
  result.analytic_bound = maximum_history_bound(volume, source_count).first;

  RenderBridge bridge(L);
  configure_bridge(bridge, seed);
  if (bridge.backend_kind() != Backend::Kind::Cpu
      || !bridge.toggles.validate()) return result;

  const auto sites = source_sites(L, source_count, variant);
  const auto schedule = removal_ticks(history, source_count);
  std::vector<int> original_indices;
  original_indices.reserve(sites.size());
  for (Coord site : sites) {
    bridge.inject_particle(site.x, site.y, site.z,
        static_cast<std::int8_t>(polarity), Vec3{});
    const int index = bridge.lattice().index(site.x, site.y, site.z);
    original_indices.push_back(index);
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    voxel.flux = {};
    voxel.wave_vel = {};
    voxel.velocity = {};
    voxel.remainder = {};
    voxel.locked = history != RemovalHistoryKind::NativeUnlocked;
  }
  if (!bridge.enable_history_journal(true)) return result;

  bool scope_respected = true;
  for (int tick = 0; tick < RUN_TICKS; ++tick) {
    if (history != RemovalHistoryKind::NativeUnlocked) {
      for (int j = 0; j < source_count; ++j) {
        if (schedule[static_cast<std::size_t>(j)] == tick) {
          const int index = original_indices[static_cast<std::size_t>(j)];
          bridge.set_state(index, 0);
          bridge.voxels()[static_cast<std::size_t>(index)].locked = false;
        }
      }
    }

    bridge.tick();
    ++result.ticks;
    for (const auto& event : bridge.history_events()) {
      if (event.kind == HistoryEventKind::Genesis)
        ++result.genesis_events;
      if (event.kind == HistoryEventKind::Evaporation)
        ++result.evaporation_events;
    }

    if (originals_remaining(bridge, original_indices) == 0
        && result.all_originals_removed_tick < 0) {
      result.all_originals_removed_tick = tick;
    }
    for (const auto& voxel : bridge.voxels()) {
      const double flux = voxel.flux.mag();
      result.maximum_flux = std::max(result.maximum_flux, flux);
      result.maximum_bound_excess = std::max(
          result.maximum_bound_excess, flux - result.analytic_bound);
      if (flux > result.analytic_bound + TOL) scope_respected = false;
      result.maximum_velocity = std::max(
          result.maximum_velocity, max_abs(voxel.velocity));
      result.maximum_remainder = std::max(
          result.maximum_remainder, max_abs(voxel.remainder));
    }
  }

  result.all_originals_removed = result.all_originals_removed_tick >= 0;
  result.analytic_scope_respected = scope_respected;
  result.valid = result.ticks == RUN_TICKS
      && result.genesis_events == 0
      && result.analytic_scope_respected
      && result.maximum_velocity == 0.0
      && result.maximum_remainder == 0.0;
  return result;
}

void hash_bytes(std::uint64_t& hash, const void* data, std::size_t size) {
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (std::size_t i = 0; i < size; ++i) {
    hash ^= static_cast<std::uint64_t>(bytes[i]);
    hash *= 1099511628211ull;
  }
}

template <typename T>
void hash_value(std::uint64_t& hash, const T& value) {
  hash_bytes(hash, &value, sizeof(value));
}

std::uint64_t selected_state_hash(const RenderBridge& bridge) {
  std::uint64_t hash = 1469598103934665603ull;
  hash_value(hash, bridge.current_tick());
  hash_value(hash, bridge.physical_time());
  for (const auto& voxel : bridge.voxels()) {
    hash_value(hash, voxel.state);
    for (const auto& value : {voxel.flux, voxel.wave_vel, voxel.flux_L,
                             voxel.flux_R, voxel.wave_vel_L,
                             voxel.wave_vel_R, voxel.velocity,
                             voxel.remainder}) {
      hash_value(hash, value.x);
      hash_value(hash, value.y);
      hash_value(hash, value.z);
    }
    hash_value(hash, voxel.locked);
    hash_value(hash, voxel.particle_id);
    hash_value(hash, voxel.spin);
    hash_value(hash, voxel.color);
  }
  return hash;
}

void configure_neutrality_bridge(RenderBridge& bridge, std::uint32_t seed) {
  configure_bridge(bridge, seed);
  for (Coord site : source_sites(9, 6, 0)) {
    bridge.inject_particle(site.x, site.y, site.z, 1, Vec3{});
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(
        bridge.lattice().index(site.x, site.y, site.z))];
    voxel.velocity = {};
    voxel.remainder = {};
    voxel.locked = false;
  }
}

bool observer_neutrality() {
  constexpr std::uint32_t seed = 0x0589AA55u;
  RenderBridge control(9);
  RenderBridge observed(9);
  configure_neutrality_bridge(control, seed);
  configure_neutrality_bridge(observed, seed);
  if (!observed.enable_history_journal(true)) return false;
  for (int tick = 0; tick < 32; ++tick) {
    control.tick();
    observed.tick();
  }
  return selected_state_hash(control) == selected_state_hash(observed)
      && control.rng_state_hash() == observed.rng_state_hash();
}

}  // namespace

const char* removal_history_name(RemovalHistoryKind kind) {
  switch (kind) {
    case RemovalHistoryKind::PermanentStep: return "permanent_step";
    case RemovalHistoryKind::SynchronousPulse: return "synchronous_pulse";
    case RemovalHistoryKind::StaggeredPulse: return "staggered_pulse";
    case RemovalHistoryKind::PairedPulse: return "paired_pulse";
    case RemovalHistoryKind::NativeUnlocked: return "native_unlocked";
  }
  return "unknown";
}

RemovalTimePulseBoundResult analyze_removal_time_pulse_bound() {
  RemovalTimePulseBoundResult result;
  result.volumes.reserve(SPECTRAL_VOLUMES.size());
  result.arms.reserve(96);

  bool spectral_valid = true;
  result.uniform_closed_source_count = std::numeric_limits<int>::max();
  result.first_source_count_not_excluded = 0;
  for (int L : SPECTRAL_VOLUMES) {
    auto volume = spectral_bound(L);
    spectral_valid = spectral_valid
        && volume.maximum_mode_eigenvalue > 0.0
        && volume.maximum_mode_eigenvalue < 4.0
        && volume.uniform_closed_source_count == 6
        && volume.first_source_count_not_excluded == 7
        && volume.closed_count_margin > 0.0
        && volume.first_open_count_margin < 0.0;
    result.uniform_closed_source_count = std::min(
        result.uniform_closed_source_count,
        volume.uniform_closed_source_count);
    result.first_source_count_not_excluded = std::max(
        result.first_source_count_not_excluded,
        volume.first_source_count_not_excluded);
    result.volumes.push_back(volume);
    ++result.spectral_volume_count;
  }
  result.arbitrary_removal_n_le_six_closed = spectral_valid
      && result.uniform_closed_source_count == 6;
  result.seven_source_bound_inconclusive = spectral_valid
      && result.first_source_count_not_excluded == 7;

  bool relaxation_valid = true;
  for (const auto& volume : result.volumes) {
    for (int source_count = 1; source_count <= 7; ++source_count) {
      const double relaxed = source_count
              * volume.exact_one_source_pulse_bound
          + volume.common_step_coefficient * volume.common_step_coefficient
              / (4.0 * volume.exact_one_source_pulse_bound);
      for (int removed = 0; removed <= source_count; ++removed) {
        const double exact = volume.common_step_coefficient
                * std::sqrt(static_cast<double>(source_count - removed))
            + removed * volume.exact_one_source_pulse_bound;
        relaxation_valid = relaxation_valid && exact <= relaxed + 1e-15;
      }
    }
  }
  result.continuous_relaxation_derived = relaxation_valid;
  verify_pulse_identity(result);
  verify_gram_and_covariance(result);

  std::uint32_t prescribed_seed = 0x0589C000u;
  for (int L : LIVE_VOLUMES) {
    const auto volume_it = std::find_if(
        result.volumes.begin(), result.volumes.end(),
        [L](const auto& volume) { return volume.lattice_size == L; });
    if (volume_it == result.volumes.end()) continue;
    for (int source_count : {5, 6}) {
      for (int polarity : {-1, 1}) {
        for (int variant : {0, 1}) {
          for (RemovalHistoryKind history : {
                   RemovalHistoryKind::PermanentStep,
                   RemovalHistoryKind::SynchronousPulse,
                   RemovalHistoryKind::StaggeredPulse,
                   RemovalHistoryKind::PairedPulse}) {
            result.arms.push_back(run_arm(
                L, source_count, polarity, variant, history,
                prescribed_seed++, *volume_it));
          }
        }
      }
    }
  }

  std::uint32_t unlocked_seed = 0x05890000u;
  for (int L : LIVE_VOLUMES) {
    const auto volume_it = std::find_if(
        result.volumes.begin(), result.volumes.end(),
        [L](const auto& volume) { return volume.lattice_size == L; });
    if (volume_it == result.volumes.end()) continue;
    for (int source_count : {5, 6}) {
      for (int polarity : {-1, 1}) {
        for (int variant : {0, 1}) {
          for (int seed_index = 0; seed_index < 2; ++seed_index) {
            result.arms.push_back(run_arm(
                L, source_count, polarity, variant,
                RemovalHistoryKind::NativeUnlocked,
                unlocked_seed++, *volume_it));
          }
        }
      }
    }
  }

  bool arms_valid = true;
  std::array<std::array<bool, 2>, 2> complete_cells{};
  for (const auto& arm : result.arms) {
    arms_valid = arms_valid && arm.valid;
    ++result.total_arms;
    result.total_ticks += arm.ticks;
    result.genesis_events += arm.genesis_events;
    result.evaporation_events += arm.evaporation_events;
    result.maximum_observed_flux = std::max(
        result.maximum_observed_flux, arm.maximum_flux);
    result.maximum_bound_excess = std::max(
        result.maximum_bound_excess, arm.maximum_bound_excess);
    result.maximum_velocity = std::max(
        result.maximum_velocity, arm.maximum_velocity);
    result.maximum_remainder = std::max(
        result.maximum_remainder, arm.maximum_remainder);
    if (arm.genesis_events > 0)
      result.analytic_contradiction_events += arm.genesis_events;
    if (arm.history == RemovalHistoryKind::NativeUnlocked) {
      ++result.native_unlocked_arms;
      if (arm.all_originals_removed) {
        const int li = arm.lattice_size == 9 ? 0 : 1;
        const int ni = arm.source_count == 5 ? 0 : 1;
        complete_cells[static_cast<std::size_t>(li)]
                      [static_cast<std::size_t>(ni)] = true;
      }
    } else {
      ++result.prescribed_history_arms;
    }
  }
  for (const auto& row : complete_cells)
    for (bool complete : row)
      if (complete) ++result.unlocked_cells_with_complete_removal;
  result.residual_branch_exercised =
      result.unlocked_cells_with_complete_removal == 4;
  result.observer_neutral = observer_neutrality();
  result.production_changed = false;

  result.valid = result.spectral_volume_count == 4
      && result.exact_pulse_identity_derived
      && result.continuous_relaxation_derived
      && result.arbitrary_removal_n_le_six_closed
      && result.seven_source_bound_inconclusive
      && result.gram_identity_verified
      && result.translation_covariant
      && result.cubic_covariant
      && result.prescribed_history_arms == 64
      && result.native_unlocked_arms == 32
      && result.total_arms == 96
      && result.total_ticks == 96 * RUN_TICKS
      && result.genesis_events == 0
      && result.analytic_contradiction_events == 0
      && result.residual_branch_exercised
      && result.maximum_bound_excess <= TOL
      && result.maximum_velocity == 0.0
      && result.maximum_remainder == 0.0
      && arms_valid
      && result.observer_neutral
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft
