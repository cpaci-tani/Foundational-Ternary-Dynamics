/**
 * @file campaign_production_ternary_plaquette_recurrence_census.cpp
 * @brief FTD-0915 locked observation-only production plaquette census.
 *
 * Enumerates all elementary cardinal plaquettes after each complete
 * RenderBridge tick.  The observer never writes production state.
 */

#include "ftd/constants.h"
#include "ftd/eft/native_ternary_plaquette_quarter_turn.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

constexpr char kProtocolSha256[] =
    "C302319900BAC4920277FACCC3A9164F0AE64DCAC8FBD256A4F36B48E7CC970C";
constexpr int kTicks = 128;
constexpr int kCellSeedGate = 6;
constexpr double kInjectionAmplitude = 10.0;
constexpr double kLangevinTemperature = 0.005;
constexpr double kLangevinGamma = 0.02;
constexpr std::array<int, 2> kVolumes{{21, 27}};
constexpr std::array<std::uint32_t, 8> kSeeds{{
    0x09150001u, 0x09150002u, 0x09150003u, 0x09150004u,
    0x09150005u, 0x09150006u, 0x09150007u, 0x09150008u}};

using Word = std::array<int, 4>;
using IVec = std::array<int, 3>;

enum class Plane : int { XY = 0, YZ = 1, ZX = 2 };
enum class Family : int {
  AxialLive = 0,
  DiagonalLive = 1,
  AxialNoBath = 2,
  EmptyControl = 3,
};
enum class Relation : int {
  Forward = 0,
  Reverse = 1,
  Stationary = 2,
  HalfTurn = 3,
  AdjacentDefect = 4,
  SupportLoss = 5,
};

constexpr std::array<Family, 4> kFamilies{{
    Family::AxialLive, Family::DiagonalLive,
    Family::AxialNoBath, Family::EmptyControl}};

const char* plane_name(Plane plane) {
  switch (plane) {
    case Plane::XY: return "xy";
    case Plane::YZ: return "yz";
    case Plane::ZX: return "zx";
  }
  return "unknown";
}

const char* family_name(Family family) {
  switch (family) {
    case Family::AxialLive: return "axial_live";
    case Family::DiagonalLive: return "diagonal_live";
    case Family::AxialNoBath: return "axial_no_bath";
    case Family::EmptyControl: return "empty_control";
  }
  return "unknown";
}

const char* relation_name(Relation relation) {
  switch (relation) {
    case Relation::Forward: return "FORWARD";
    case Relation::Reverse: return "REVERSE";
    case Relation::Stationary: return "STATIONARY";
    case Relation::HalfTurn: return "HALF_TURN";
    case Relation::AdjacentDefect: return "ADJACENT_DEFECT";
    case Relation::SupportLoss: return "SUPPORT_LOSS";
  }
  return "UNKNOWN";
}

bool is_live_family(Family family) {
  return family != Family::EmptyControl;
}

struct SupportKey {
  Plane plane = Plane::XY;
  int x = 0;
  int y = 0;
  int z = 0;

  bool operator<(const SupportKey& other) const {
    return std::tie(plane, x, y, z)
        < std::tie(other.plane, other.x, other.y, other.z);
  }
};

struct ExposureKey {
  SupportKey support{};
  int positive_id = -1;
  int negative_id = -1;

  bool operator<(const ExposureKey& other) const {
    return std::tie(
        support.plane, support.x, support.y, support.z,
        positive_id, negative_id)
        < std::tie(
            other.support.plane, other.support.x, other.support.y,
            other.support.z, other.positive_id, other.negative_id);
  }
};

struct VertexSample {
  int site_index = -1;
  int state = 0;
  int particle_id = -1;
  std::array<double, 3> flux{};
  std::array<double, 3> wave_velocity{};
};

struct Exposure {
  ExposureKey key{};
  std::array<int, 4> indices{};
  Word word{};
  std::array<VertexSample, 4> vertices{};
  double local_energy = 0.0;
};

struct RunTrack {
  int direction = 0;
  int last_transition_tick = -2;
  int current_run = 0;
  int maximum_run = 0;
  Word start_word{};
  bool full_cycle = false;
  int full_cycle_count = 0;
};

struct TickObservation {
  bool finite = true;
  bool nonmutating = true;
  bool controls_pass = true;
  bool reconstructible = true;
  long long plaquettes_enumerated = 0;
  long long raw_exposures = 0;
  long long identity_exposures = 0;
  long long transition_attempts = 0;
  long long forward = 0;
  long long reverse = 0;
  long long stationary = 0;
  long long half_turn = 0;
  long long adjacent_defect = 0;
  long long support_loss = 0;
  int maximum_current_run = 0;
  int positive_count = 0;
  int negative_count = 0;
  long long genesis_events = 0;
  long long evaporation_events = 0;
  double total_native_energy = 0.0;
  double maximum_control_residual = 0.0;
  std::uint64_t voxel_hash_before = 0;
  std::uint64_t voxel_hash_after = 0;
  std::uint64_t rng_hash_before = 0;
  std::uint64_t rng_hash_after = 0;
};

struct ArmResult {
  int volume = 0;
  Family family = Family::AxialLive;
  std::uint32_t seed = 0;
  bool executed = false;
  bool finite = true;
  bool nonmutating = true;
  bool controls_pass = true;
  bool reconstructible = true;
  bool enumeration_complete = true;
  int ticks = 0;
  long long raw_exposures = 0;
  long long identity_exposures = 0;
  long long transition_attempts = 0;
  long long forward = 0;
  long long reverse = 0;
  long long stationary = 0;
  long long half_turn = 0;
  long long adjacent_defect = 0;
  long long support_loss = 0;
  int maximum_oriented_run = 0;
  int full_cycle_keys = 0;
  int full_cycle_count = 0;
  long long genesis_events = 0;
  long long evaporation_events = 0;
  double maximum_native_energy = 0.0;
  double maximum_control_residual = 0.0;
};

std::uint64_t hash_voxels(const std::vector<ftd::Voxel>& voxels) {
  constexpr std::uint64_t offset = 1469598103934665603ull;
  constexpr std::uint64_t prime = 1099511628211ull;
  std::uint64_t hash = offset;
  const auto* bytes = reinterpret_cast<const unsigned char*>(voxels.data());
  const std::size_t count = voxels.size() * sizeof(ftd::Voxel);
  for (std::size_t index = 0; index < count; ++index) {
    hash ^= static_cast<std::uint64_t>(bytes[index]);
    hash *= prime;
  }
  return hash;
}

Word shift_forward(const Word& word) {
  return {word[3], word[0], word[1], word[2]};
}

Word shift_reverse(const Word& word) {
  return {word[1], word[2], word[3], word[0]};
}

bool is_orbit_word(const Word& word) {
  int positive = -1;
  int negative = -1;
  for (int index = 0; index < 4; ++index) {
    const int value = word[static_cast<std::size_t>(index)];
    if (value == 1) {
      if (positive >= 0) return false;
      positive = index;
    } else if (value == -1) {
      if (negative >= 0) return false;
      negative = index;
    } else if (value != 0) {
      return false;
    }
  }
  return positive >= 0 && negative >= 0
      && (positive + 2) % 4 == negative;
}

std::array<int, 4> support_indices(
    const ftd::Lattice& lattice, const SupportKey& support) {
  const int x = support.x;
  const int y = support.y;
  const int z = support.z;
  switch (support.plane) {
    case Plane::XY:
      return {{
          lattice.index(x + 1, y, z),
          lattice.index(x + 1, y + 1, z),
          lattice.index(x, y + 1, z),
          lattice.index(x, y, z)}};
    case Plane::YZ:
      return {{
          lattice.index(x, y + 1, z),
          lattice.index(x, y + 1, z + 1),
          lattice.index(x, y, z + 1),
          lattice.index(x, y, z)}};
    case Plane::ZX:
      return {{
          lattice.index(x, y, z + 1),
          lattice.index(x + 1, y, z + 1),
          lattice.index(x + 1, y, z),
          lattice.index(x, y, z)}};
  }
  return {};
}

std::array<IVec, 4> doubled_offsets(Plane plane) {
  switch (plane) {
    case Plane::XY:
      return {{{{1, -1, 0}}, {{1, 1, 0}},
               {{-1, 1, 0}}, {{-1, -1, 0}}}};
    case Plane::YZ:
      return {{{{0, 1, -1}}, {{0, 1, 1}},
               {{0, -1, 1}}, {{0, -1, -1}}}};
    case Plane::ZX:
      return {{{{-1, 0, 1}}, {{1, 0, 1}},
               {{1, 0, -1}}, {{-1, 0, -1}}}};
  }
  return {};
}

IVec positive_normal(Plane plane) {
  switch (plane) {
    case Plane::XY: return {0, 0, 1};
    case Plane::YZ: return {1, 0, 0};
    case Plane::ZX: return {0, 1, 0};
  }
  return {};
}

IVec add(const IVec& left, const IVec& right) {
  return {left[0] + right[0], left[1] + right[1], left[2] + right[2]};
}

IVec scale(const IVec& value, int factor) {
  return {factor * value[0], factor * value[1], factor * value[2]};
}

IVec cross(const IVec& left, const IVec& right) {
  return {
      left[1] * right[2] - left[2] * right[1],
      left[2] * right[0] - left[0] * right[2],
      left[0] * right[1] - left[1] * right[0]};
}

int dot(const IVec& left, const IVec& right) {
  return left[0] * right[0] + left[1] * right[1]
      + left[2] * right[2];
}

int max_abs(const IVec& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

IVec dipole(Plane plane, const Word& word) {
  IVec result{};
  const auto offsets = doubled_offsets(plane);
  for (int index = 0; index < 4; ++index) {
    result = add(result, scale(
        offsets[static_cast<std::size_t>(index)],
        word[static_cast<std::size_t>(index)]));
  }
  return result;
}

std::array<VertexSample, 4> sample_vertices(
    const std::vector<ftd::Voxel>& voxels,
    const std::array<int, 4>& indices) {
  std::array<VertexSample, 4> samples{};
  for (int vertex = 0; vertex < 4; ++vertex) {
    const int site_index = indices[static_cast<std::size_t>(vertex)];
    const auto& voxel = voxels[static_cast<std::size_t>(site_index)];
    samples[static_cast<std::size_t>(vertex)] = VertexSample{
        site_index,
        static_cast<int>(voxel.state),
        voxel.particle_id,
        {voxel.flux.x, voxel.flux.y, voxel.flux.z},
        {voxel.wave_vel.x, voxel.wave_vel.y, voxel.wave_vel.z}};
  }
  return samples;
}

Word word_from_vertices(const std::array<VertexSample, 4>& vertices) {
  Word word{};
  for (int vertex = 0; vertex < 4; ++vertex)
    word[static_cast<std::size_t>(vertex)] =
        vertices[static_cast<std::size_t>(vertex)].state;
  return word;
}

double local_energy(const std::array<VertexSample, 4>& vertices) {
  double energy = 0.0;
  for (const auto& vertex : vertices) {
    for (int component = 0; component < 3; ++component) {
      const double flux = vertex.flux[static_cast<std::size_t>(component)];
      const double wave =
          vertex.wave_velocity[static_cast<std::size_t>(component)];
      energy += 0.5 * (flux * flux + wave * wave);
    }
  }
  return energy;
}

Relation classify(const Word& before, const Word& after) {
  if (after == shift_forward(before)) return Relation::Forward;
  if (after == shift_reverse(before)) return Relation::Reverse;
  if (after == before) return Relation::Stationary;
  if (after == shift_forward(shift_forward(before)))
    return Relation::HalfTurn;
  return Relation::AdjacentDefect;
}

bool word_for_id_pair_on_support(
    const std::array<VertexSample, 4>& vertices,
    int positive_id, int negative_id, Word& word) {
  word = word_from_vertices(vertices);
  int positive_count = 0;
  int negative_count = 0;
  for (int vertex = 0; vertex < 4; ++vertex) {
    const auto& sample = vertices[static_cast<std::size_t>(vertex)];
    if (sample.state == 1 && sample.particle_id == positive_id)
      ++positive_count;
    if (sample.state == -1 && sample.particle_id == negative_id)
      ++negative_count;
  }
  int total_positive = 0;
  int total_negative = 0;
  int total_zero = 0;
  for (int value : word) {
    total_positive += value == 1 ? 1 : 0;
    total_negative += value == -1 ? 1 : 0;
    total_zero += value == 0 ? 1 : 0;
  }
  return positive_count == 1 && negative_count == 1
      && total_positive == 1 && total_negative == 1 && total_zero == 2;
}

bool directed_transition_control(
    Plane plane, const Word& before, const Word& after,
    Relation relation, IVec& before_d, IVec& after_d, IVec& bivector,
    double& residual) {
  before_d = dipole(plane, before);
  after_d = dipole(plane, after);
  bivector = cross(before_d, after_d);
  const int expected_sign = relation == Relation::Forward ? 1 : -1;
  const IVec expected_bivector = scale(positive_normal(plane), 8 * expected_sign);
  const IVec reconstructed_numerator = cross(bivector, before_d);
  const IVec expected_numerator = scale(after_d, dot(before_d, before_d));
  const IVec reverse_bivector = cross(after_d, before_d);
  residual = static_cast<double>(std::max({
      std::abs(dot(before_d, after_d)),
      std::abs(dot(before_d, before_d) - 8),
      std::abs(dot(after_d, after_d) - 8),
      max_abs(add(bivector, scale(expected_bivector, -1))),
      max_abs(add(reconstructed_numerator, scale(expected_numerator, -1))),
      max_abs(add(reverse_bivector, bivector))}));
  return residual == 0.0;
}

using ExposureMap = std::map<ExposureKey, Exposure>;
using TrackMap = std::map<ExposureKey, RunTrack>;

void write_word(std::ofstream& stream, const Word& word);

void write_vertices(
    std::ofstream& stream,
    const std::array<VertexSample, 4>& vertices) {
  for (const auto& vertex : vertices) {
    stream << ',' << vertex.site_index << ',' << vertex.state
        << ',' << vertex.particle_id
        << ',' << vertex.flux[0] << ',' << vertex.flux[1]
        << ',' << vertex.flux[2]
        << ',' << vertex.wave_velocity[0]
        << ',' << vertex.wave_velocity[1]
        << ',' << vertex.wave_velocity[2];
  }
}

void write_vertex_header(std::ofstream& stream, const char* prefix) {
  for (int vertex = 0; vertex < 4; ++vertex) {
    stream << ',' << prefix << "_v" << vertex << "_site"
        << ',' << prefix << "_v" << vertex << "_state"
        << ',' << prefix << "_v" << vertex << "_particle_id"
        << ',' << prefix << "_v" << vertex << "_jx"
        << ',' << prefix << "_v" << vertex << "_jy"
        << ',' << prefix << "_v" << vertex << "_jz"
        << ',' << prefix << "_v" << vertex << "_wx"
        << ',' << prefix << "_v" << vertex << "_wy"
        << ',' << prefix << "_v" << vertex << "_wz";
  }
}

ExposureMap enumerate_exposures(
    const ftd::RenderBridge& bridge, int volume, Family family,
    std::uint32_t seed, int tick, TickObservation& observation,
    std::ofstream& exposure_stream) {
  ExposureMap exposures;
  const auto& lattice = bridge.lattice();
  const auto& voxels = bridge.voxels();
  const int lattice_volume = lattice.size();
  observation.reconstructible = observation.reconstructible
      && volume == lattice_volume;
  for (int x = 0; x < lattice_volume; ++x) {
    for (int y = 0; y < lattice_volume; ++y) {
      for (int z = 0; z < lattice_volume; ++z) {
        for (Plane plane : {Plane::XY, Plane::YZ, Plane::ZX}) {
          ++observation.plaquettes_enumerated;
          const SupportKey support{plane, x, y, z};
          const auto indices = support_indices(lattice, support);
          const auto vertices = sample_vertices(voxels, indices);
          const Word word = word_from_vertices(vertices);
          int positive_vertex = -1;
          int negative_vertex = -1;
          for (int vertex = 0; vertex < 4; ++vertex) {
            const auto& sample = vertices[static_cast<std::size_t>(vertex)];
            if (sample.state == 1) positive_vertex = vertex;
            if (sample.state == -1) negative_vertex = vertex;
          }
          if (!is_orbit_word(word)) continue;
          ++observation.raw_exposures;
          const auto& positive =
              vertices[static_cast<std::size_t>(positive_vertex)];
          const auto& negative =
              vertices[static_cast<std::size_t>(negative_vertex)];
          if (positive.particle_id < 0 || negative.particle_id < 0)
            continue;
          ++observation.identity_exposures;
          const ExposureKey key{
              support, positive.particle_id, negative.particle_id};
          const Exposure exposure{
              key, indices, word, vertices, local_energy(vertices)};
          observation.reconstructible = observation.reconstructible
              && positive.state == 1 && negative.state == -1
              && positive.particle_id == key.positive_id
              && negative.particle_id == key.negative_id;
          exposures.emplace(key, exposure);
          exposure_stream << volume << ',' << family_name(family) << ','
              << seed << ',' << tick << ',' << plane_name(plane) << ','
              << x << ',' << y << ',' << z << ',' << key.positive_id
              << ',' << key.negative_id;
          write_word(exposure_stream, word);
          exposure_stream << ',' << exposure.local_energy;
          write_vertices(exposure_stream, vertices);
          exposure_stream << '\n';
        }
      }
    }
  }
  return exposures;
}

void write_word(std::ofstream& stream, const Word& word) {
  for (int value : word) stream << ',' << value;
}

TickObservation observe_tick(
    const ftd::RenderBridge& bridge, int volume, Family family,
    std::uint32_t seed, int tick, ExposureMap& previous,
    TrackMap& tracks, std::ofstream& exposures,
    std::ofstream& transitions) {
  TickObservation observation;
  observation.voxel_hash_before = hash_voxels(bridge.voxels());
  observation.rng_hash_before = bridge.rng_state_hash();
  observation.genesis_events = bridge.genesis_events_this_tick();
  observation.evaporation_events = bridge.evaporation_events_this_tick();
  for (const auto& voxel : bridge.voxels()) {
    observation.positive_count += voxel.state == 1 ? 1 : 0;
    observation.negative_count += voxel.state == -1 ? 1 : 0;
    observation.total_native_energy +=
        0.5 * (voxel.flux.mag2() + voxel.wave_vel.mag2());
  }

  ExposureMap current = enumerate_exposures(
      bridge, volume, family, seed, tick, observation, exposures);
  for (const auto& item : previous) {
    ++observation.transition_attempts;
    const auto& key = item.first;
    const auto& before = item.second;
    const auto after_vertices = sample_vertices(
        bridge.voxels(), before.indices);
    Word after_word = word_from_vertices(after_vertices);
    double after_energy = local_energy(after_vertices);
    Relation relation = Relation::SupportLoss;
    const auto current_item = current.find(key);
    if (current_item != current.end()) {
      after_word = current_item->second.word;
      after_energy = current_item->second.local_energy;
      relation = classify(before.word, after_word);
    } else if (word_for_id_pair_on_support(
                   after_vertices,
                   key.positive_id, key.negative_id, after_word)) {
      relation = Relation::AdjacentDefect;
    }

    IVec before_d{};
    IVec after_d{};
    IVec bivector{};
    double residual = 0.0;
    bool transition_control = true;
    bool closure = false;
    auto& track = tracks[key];
    if (relation == Relation::Forward || relation == Relation::Reverse) {
      const int direction = relation == Relation::Forward ? 1 : -1;
      transition_control = directed_transition_control(
          key.support.plane, before.word, after_word, relation,
          before_d, after_d, bivector, residual);
      if (track.last_transition_tick == tick - 1
          && track.direction == direction) {
        ++track.current_run;
      } else {
        track.direction = direction;
        track.current_run = 1;
        track.start_word = before.word;
      }
      track.last_transition_tick = tick;
      track.maximum_run = std::max(track.maximum_run, track.current_run);
      if (track.current_run % 4 == 0) {
        closure = after_word == track.start_word;
        transition_control = transition_control && closure;
        if (closure) {
          track.full_cycle = true;
          ++track.full_cycle_count;
        }
      }
      if (relation == Relation::Forward) ++observation.forward;
      else ++observation.reverse;
    } else {
      track.direction = 0;
      track.current_run = 0;
      track.last_transition_tick = tick;
      if (relation == Relation::Stationary) ++observation.stationary;
      if (relation == Relation::HalfTurn) ++observation.half_turn;
      if (relation == Relation::AdjacentDefect) ++observation.adjacent_defect;
      if (relation == Relation::SupportLoss) ++observation.support_loss;
    }
    observation.maximum_current_run = std::max(
        observation.maximum_current_run, track.current_run);
    observation.controls_pass = observation.controls_pass
        && transition_control;
    observation.maximum_control_residual = std::max(
        observation.maximum_control_residual, residual);

    transitions << volume << ',' << family_name(family) << ',' << seed
        << ',' << tick << ',' << plane_name(key.support.plane) << ','
        << key.support.x << ',' << key.support.y << ',' << key.support.z
        << ',' << key.positive_id << ',' << key.negative_id;
    write_word(transitions, before.word);
    write_word(transitions, after_word);
    transitions << ',' << relation_name(relation)
        << ',' << before_d[0] << ',' << before_d[1] << ',' << before_d[2]
        << ',' << after_d[0] << ',' << after_d[1] << ',' << after_d[2]
        << ',' << bivector[0] << ',' << bivector[1] << ',' << bivector[2]
        << ',' << before.local_energy << ',' << after_energy
        << ',' << track.current_run << ',' << closure
        << ',' << transition_control << ',' << residual;
    write_vertices(transitions, before.vertices);
    write_vertices(transitions, after_vertices);
    transitions << '\n';
  }

  previous = std::move(current);
  observation.finite = std::isfinite(observation.total_native_energy)
      && std::isfinite(observation.maximum_control_residual);
  observation.voxel_hash_after = hash_voxels(bridge.voxels());
  observation.rng_hash_after = bridge.rng_state_hash();
  observation.nonmutating =
      observation.voxel_hash_before == observation.voxel_hash_after
      && observation.rng_hash_before == observation.rng_hash_after;
  return observation;
}

void configure(ftd::RenderBridge& bridge, Family family, std::uint32_t seed) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
  bridge.toggles.wave_propagation = true;
  bridge.toggles.gauss_projection = true;
  bridge.toggles.genesis = true;
  bridge.toggles.coupling = true;
  bridge.toggles.dual_substrate = false;
  bridge.toggles.langevin = family != Family::AxialNoBath;
  if (bridge.toggles.langevin) {
    bridge.toggles.langevin_T = kLangevinTemperature;
    bridge.toggles.langevin_gamma = kLangevinGamma;
  }
  bridge.seed_rng(seed);
  const int center = bridge.lattice().size() / 2;
  if (family == Family::AxialLive || family == Family::AxialNoBath) {
    bridge.inject_flux(center, center, center,
                       {kInjectionAmplitude * ftd::K_GENESIS, 0.0, 0.0});
  } else if (family == Family::DiagonalLive) {
    const double component = kInjectionAmplitude * ftd::K_GENESIS
        / std::sqrt(3.0);
    bridge.inject_flux(center, center, center,
                       {component, component, component});
  }
}

ArmResult run_arm(
    int volume, Family family, std::uint32_t seed,
    std::ofstream& ticks, std::ofstream& exposures,
    std::ofstream& transitions) {
  ArmResult arm;
  arm.volume = volume;
  arm.family = family;
  arm.seed = seed;
  ftd::RenderBridge bridge(volume);
  configure(bridge, family, seed);
  ExposureMap previous;
  TrackMap tracks;
  const long long expected_plaquettes =
      3LL * volume * volume * volume;

  for (int tick = 0; tick < kTicks; ++tick) {
    bridge.run(1);
    const TickObservation observation = observe_tick(
        bridge, volume, family, seed, tick, previous, tracks,
        exposures, transitions);
    ++arm.ticks;
    arm.finite = arm.finite && observation.finite;
    arm.nonmutating = arm.nonmutating && observation.nonmutating;
    arm.controls_pass = arm.controls_pass && observation.controls_pass;
    arm.reconstructible = arm.reconstructible
        && observation.reconstructible;
    arm.enumeration_complete = arm.enumeration_complete
        && observation.plaquettes_enumerated == expected_plaquettes;
    arm.raw_exposures += observation.raw_exposures;
    arm.identity_exposures += observation.identity_exposures;
    arm.transition_attempts += observation.transition_attempts;
    arm.forward += observation.forward;
    arm.reverse += observation.reverse;
    arm.stationary += observation.stationary;
    arm.half_turn += observation.half_turn;
    arm.adjacent_defect += observation.adjacent_defect;
    arm.support_loss += observation.support_loss;
    arm.maximum_oriented_run = std::max(
        arm.maximum_oriented_run, observation.maximum_current_run);
    arm.genesis_events += observation.genesis_events;
    arm.evaporation_events += observation.evaporation_events;
    arm.maximum_native_energy = std::max(
        arm.maximum_native_energy, observation.total_native_energy);
    arm.maximum_control_residual = std::max(
        arm.maximum_control_residual,
        observation.maximum_control_residual);

    ticks << volume << ',' << family_name(family) << ',' << seed << ','
        << tick << ',' << observation.plaquettes_enumerated << ','
        << observation.raw_exposures << ','
        << observation.identity_exposures << ','
        << observation.transition_attempts << ','
        << observation.forward << ',' << observation.reverse << ','
        << observation.stationary << ',' << observation.half_turn << ','
        << observation.adjacent_defect << ',' << observation.support_loss
        << ',' << observation.maximum_current_run << ','
        << observation.positive_count << ',' << observation.negative_count
        << ',' << observation.genesis_events << ','
        << observation.evaporation_events << ','
        << observation.total_native_energy << ','
        << observation.nonmutating << ',' << observation.controls_pass
        << ',' << observation.reconstructible << ','
        << observation.maximum_control_residual << ','
        << observation.voxel_hash_before << ','
        << observation.voxel_hash_after << ','
        << observation.rng_hash_before << ','
        << observation.rng_hash_after << '\n';
  }

  for (const auto& item : tracks) {
    arm.maximum_oriented_run = std::max(
        arm.maximum_oriented_run, item.second.maximum_run);
    if (item.second.full_cycle) ++arm.full_cycle_keys;
    arm.full_cycle_count += item.second.full_cycle_count;
  }
  arm.executed = arm.ticks == kTicks;
  return arm;
}

std::filesystem::path result_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0915" / "v3";
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const bool reference_valid =
      ftd::eft::analyze_native_ternary_plaquette_quarter_turn().valid;
  const auto directory = result_directory();
  std::filesystem::create_directories(directory);
  std::ofstream ticks(directory / "ftd_0915_tick_census_v3.csv");
  std::ofstream exposures(
      directory / "ftd_0915_exposure_census_v3.csv");
  std::ofstream transitions(
      directory / "ftd_0915_transition_census_v3.csv");
  ticks << std::setprecision(17);
  exposures << std::setprecision(17);
  transitions << std::setprecision(17);
  ticks << "volume,family,seed,tick,plaquettes_enumerated,raw_exposures,"
      "identity_exposures,transition_attempts,forward,reverse,stationary,"
      "half_turn,adjacent_defect,support_loss,maximum_current_run,"
      "positive_count,negative_count,genesis_events,evaporation_events,"
      "total_native_energy,nonmutating,controls_pass,reconstructible,"
      "maximum_control_residual,voxel_hash_before,voxel_hash_after,"
      "rng_hash_before,rng_hash_after\n";
  exposures << "volume,family,seed,tick,plane,x,y,z,positive_id,"
      "negative_id,word_0,word_1,word_2,word_3,local_energy";
  write_vertex_header(exposures, "sample");
  exposures << '\n';
  transitions << "volume,family,seed,tick,plane,x,y,z,positive_id,"
      "negative_id,before_0,before_1,before_2,before_3,after_0,after_1,"
      "after_2,after_3,relation,dx0,dy0,dz0,dx1,dy1,dz1,lx,ly,lz,"
      "energy_before,energy_after,current_run,closure,control_pass,"
      "control_residual";
  write_vertex_header(transitions, "before");
  write_vertex_header(transitions, "after");
  transitions << '\n';

  std::vector<ArmResult> arms;
  arms.reserve(kVolumes.size() * kSeeds.size() * kFamilies.size());
  for (int volume : kVolumes) {
    for (std::uint32_t seed : kSeeds) {
      for (Family family : kFamilies) {
        std::cout << "RUN volume=" << volume
                  << " family=" << family_name(family)
                  << " seed=" << seed << '\n';
        arms.push_back(run_arm(
            volume, family, seed, ticks, exposures, transitions));
        const auto& arm = arms.back();
        std::cout << "ARM identity_exposures=" << arm.identity_exposures
                  << " directed=" << arm.forward + arm.reverse
                  << " max_run=" << arm.maximum_oriented_run
                  << " full_cycle_keys=" << arm.full_cycle_keys
                  << " nonmutating=" << arm.nonmutating
                  << " controls=" << arm.controls_pass << '\n';
      }
    }
  }

  const bool matrix_complete = arms.size() == 64
      && std::all_of(arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.executed && arm.ticks == kTicks;
      });
  const bool finite = std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.finite;
      });
  const bool nonmutating = std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.nonmutating;
      });
  const bool controls = std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.controls_pass && arm.reconstructible
            && arm.enumeration_complete;
      });
  const bool protocol_valid = reference_valid && matrix_complete && finite
      && nonmutating && controls;

  std::map<std::pair<int, Family>, int> cell_cycle_seeds;
  bool any_live_cycle = false;
  bool any_live_directed = false;
  bool any_live_exposure = false;
  for (const auto& arm : arms) {
    if (!is_live_family(arm.family)) continue;
    any_live_cycle = any_live_cycle || arm.full_cycle_keys > 0;
    any_live_directed = any_live_directed
        || arm.forward + arm.reverse > 0;
    any_live_exposure = any_live_exposure || arm.identity_exposures > 0;
    if (arm.full_cycle_keys > 0)
      ++cell_cycle_seeds[{arm.volume, arm.family}];
  }
  bool outcome_a = protocol_valid;
  for (int volume : kVolumes) {
    for (Family family : {
             Family::AxialLive, Family::DiagonalLive,
             Family::AxialNoBath}) {
      outcome_a = outcome_a
          && cell_cycle_seeds[{volume, family}] >= kCellSeedGate;
    }
  }

  std::string verdict;
  if (!protocol_valid) {
    verdict = "PROTOCOL_INVALID_NO_RECURRENCE_VERDICT";
  } else if (outcome_a) {
    verdict = "A_CROSS_VOLUME_PRODUCTION_RECURRENCE";
  } else if (any_live_cycle) {
    verdict = "B_ISOLATED_EXACT_PRODUCTION_RECURRENCE";
  } else if (any_live_directed) {
    verdict = "C_DIRECTED_FORMATION_WITHOUT_FULL_RECURRENCE";
  } else if (any_live_exposure) {
    verdict = "D_EXPOSURE_WITHOUT_DIRECTED_TRANSPORT";
  } else {
    verdict = "E_NO_IDENTITY_BEARING_PRODUCTION_EXPOSURE";
  }

  std::ofstream summary(directory / "ftd_0915_summary_v3.json");
  summary << std::setprecision(17)
      << "{\n"
      << "  \"identifier\": \"FTD-0915\",\n"
      << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
      << "  \"arm_count\": " << arms.size() << ",\n"
      << "  \"ticks_per_arm\": " << kTicks << ",\n"
      << "  \"cell_seed_gate\": " << kCellSeedGate << ",\n"
      << "  \"reference_valid\": "
      << (reference_valid ? "true" : "false") << ",\n"
      << "  \"matrix_complete\": "
      << (matrix_complete ? "true" : "false") << ",\n"
      << "  \"finite\": " << (finite ? "true" : "false") << ",\n"
      << "  \"nonmutating\": "
      << (nonmutating ? "true" : "false") << ",\n"
      << "  \"controls_pass\": "
      << (controls ? "true" : "false") << ",\n"
      << "  \"protocol_valid\": "
      << (protocol_valid ? "true" : "false") << ",\n"
      << "  \"any_live_cycle\": "
      << (any_live_cycle ? "true" : "false") << ",\n"
      << "  \"any_live_directed\": "
      << (any_live_directed ? "true" : "false") << ",\n"
      << "  \"any_live_exposure\": "
      << (any_live_exposure ? "true" : "false") << ",\n"
      << "  \"verdict\": \"" << verdict << "\",\n"
      << "  \"arms\": [\n";
  for (std::size_t index = 0; index < arms.size(); ++index) {
    const auto& arm = arms[index];
    summary << "    {\"volume\": " << arm.volume
        << ", \"family\": \"" << family_name(arm.family) << "\""
        << ", \"seed\": " << arm.seed
        << ", \"raw_exposures\": " << arm.raw_exposures
        << ", \"identity_exposures\": " << arm.identity_exposures
        << ", \"transition_attempts\": " << arm.transition_attempts
        << ", \"forward\": " << arm.forward
        << ", \"reverse\": " << arm.reverse
        << ", \"stationary\": " << arm.stationary
        << ", \"half_turn\": " << arm.half_turn
        << ", \"adjacent_defect\": " << arm.adjacent_defect
        << ", \"support_loss\": " << arm.support_loss
        << ", \"maximum_oriented_run\": " << arm.maximum_oriented_run
        << ", \"full_cycle_keys\": " << arm.full_cycle_keys
        << ", \"full_cycle_count\": " << arm.full_cycle_count
        << ", \"genesis_events\": " << arm.genesis_events
        << ", \"evaporation_events\": " << arm.evaporation_events
        << ", \"maximum_native_energy\": " << arm.maximum_native_energy
        << ", \"maximum_control_residual\": "
        << arm.maximum_control_residual
        << ", \"finite\": " << (arm.finite ? "true" : "false")
        << ", \"nonmutating\": "
        << (arm.nonmutating ? "true" : "false")
        << ", \"controls_pass\": "
        << (arm.controls_pass ? "true" : "false")
        << ", \"reconstructible\": "
        << (arm.reconstructible ? "true" : "false")
        << ", \"enumeration_complete\": "
        << (arm.enumeration_complete ? "true" : "false") << "}"
        << (index + 1 == arms.size() ? "\n" : ",\n");
  }
  summary << "  ]\n}\n";

  std::cout << "FTD-0915 protocol=" << kProtocolSha256
            << " arms=" << arms.size()
            << " reference_valid=" << reference_valid
            << " matrix_complete=" << matrix_complete
            << " finite=" << finite
            << " nonmutating=" << nonmutating
            << " controls=" << controls
            << " verdict=" << verdict << '\n';
  std::cout << "PRODUCTION_TICK_MODIFIED=FALSE\n";
  std::cout << "GSTAR_READ=FALSE\n";
  std::cout << "BORN_BELL_TARGET_READ=FALSE\n";
  std::cout << "PROTECTION_DERIVED=FALSE\n";
  return protocol_valid ? 0 : 1;
}
