#pragma once
/**
 * @file engine_state.h
 * @brief Cache-friendly simulation storage anchored by authoritative ternary state.
 *
 * The first migrated authority is the manifestation layer: s(x) in {-1,0,+1}.
 * Continuous fields remain double precision and are still bridged through the
 * legacy Voxel storage while the phase kernels move toward SoA access.
 */

#include "ftd/voxel.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ftd {

class TernaryField {
public:
  TernaryField() = default;
  explicit TernaryField(std::size_t n) { resize(n); }

  void resize(std::size_t n) {
    state_.assign(n, 0);
    const std::size_t words = (n + 63u) / 64u;
    pos_bits_.assign(words, 0);
    neg_bits_.assign(words, 0);
    occupied_bits_.assign(words, 0);
    active_indices_.clear();
    ordered_active_indices_.clear();
    active_pos_.assign(n, -1);
    ordered_active_dirty_ = false;
    positive_count_ = 0;
    negative_count_ = 0;
    charge_sum_ = 0;
  }

  std::size_t size() const { return state_.size(); }
  bool empty() const { return state_.empty(); }

  int8_t state_at(std::size_t idx) const { return state_[idx]; }
  bool is_manifested(std::size_t idx) const { return state_[idx] != 0; }
  int positive_count() const { return positive_count_; }
  int negative_count() const { return negative_count_; }
  int manifested_count() const {
    return positive_count_ + negative_count_;
  }
  long long charge_sum() const { return charge_sum_; }

  const std::vector<int>& active_indices() const { return active_indices_; }
  const std::vector<int>& ordered_active_indices() const {
    if (ordered_active_dirty_) {
      ordered_active_indices_ = active_indices_;
      std::sort(ordered_active_indices_.begin(), ordered_active_indices_.end());
      ordered_active_dirty_ = false;
    }
    return ordered_active_indices_;
  }
  const std::vector<int8_t>& states() const { return state_; }
  const std::vector<std::uint64_t>& pos_bits() const { return pos_bits_; }
  const std::vector<std::uint64_t>& neg_bits() const { return neg_bits_; }
  const std::vector<std::uint64_t>& occupied_bits() const {
    return occupied_bits_;
  }

  void clear() {
    std::fill(state_.begin(), state_.end(), int8_t{0});
    std::fill(pos_bits_.begin(), pos_bits_.end(), std::uint64_t{0});
    std::fill(neg_bits_.begin(), neg_bits_.end(), std::uint64_t{0});
    std::fill(occupied_bits_.begin(), occupied_bits_.end(), std::uint64_t{0});
    active_indices_.clear();
    ordered_active_indices_.clear();
    std::fill(active_pos_.begin(), active_pos_.end(), -1);
    ordered_active_dirty_ = false;
    positive_count_ = 0;
    negative_count_ = 0;
    charge_sum_ = 0;
  }

  int8_t set_state(std::size_t idx, int8_t raw_state) {
    const int8_t s = normalize(raw_state);
    const int8_t old = state_[idx];
    if (old == s) return s;

    if (old > 0) --positive_count_;
    else if (old < 0) --negative_count_;
    charge_sum_ -= static_cast<long long>(old);

    if (old == 0 && s != 0) {
      active_pos_[idx] = static_cast<int>(active_indices_.size());
      active_indices_.push_back(static_cast<int>(idx));
      ordered_active_dirty_ = true;
    } else if (old != 0 && s == 0) {
      remove_active(idx);
    }

    state_[idx] = s;
    if (s > 0) ++positive_count_;
    else if (s < 0) ++negative_count_;
    charge_sum_ += static_cast<long long>(s);
    set_bits(idx, s);
    return s;
  }

  void rebuild_from_voxels(const std::vector<Voxel>& voxels) {
    resize(voxels.size());
    active_indices_.reserve(voxels.size() / 32u + 8u);
    ordered_active_indices_.reserve(voxels.size() / 32u + 8u);
    for (std::size_t i = 0; i < voxels.size(); ++i) {
      set_state(i, voxels[i].state);
    }
  }

  void write_to_voxels(std::vector<Voxel>& voxels) const {
    const std::size_t n = std::min(voxels.size(), state_.size());
    for (std::size_t i = 0; i < n; ++i) {
      voxels[i].state = state_[i];
    }
  }

  bool check_invariants(std::string* error = nullptr) const {
    if (state_.size() != active_pos_.size()) {
      if (error) *error = "state and active_pos sizes differ";
      return false;
    }
    if (pos_bits_.size() != neg_bits_.size() ||
        pos_bits_.size() != occupied_bits_.size()) {
      if (error) *error = "bit-plane sizes differ";
      return false;
    }

    int pos = 0, neg = 0;
    long long charge = 0;
    std::vector<int> seen(state_.size(), 0);
    for (std::size_t i = 0; i < state_.size(); ++i) {
      const int8_t s = state_[i];
      if (s < -1 || s > 1) {
        if (error) *error = "state outside ternary alphabet";
        return false;
      }
      if (s > 0) ++pos;
      if (s < 0) ++neg;
      charge += s;

      const bool p = bit_at(pos_bits_, i);
      const bool m = bit_at(neg_bits_, i);
      const bool o = bit_at(occupied_bits_, i);
      if (p && m) {
        if (error) *error = "positive and negative bit overlap";
        return false;
      }
      if (p != (s > 0) || m != (s < 0) || o != (s != 0)) {
        if (error) *error = "bit plane does not match state";
        return false;
      }
      if ((s == 0 && active_pos_[i] != -1) ||
          (s != 0 && active_pos_[i] < 0)) {
        if (error) *error = "active position does not match state";
        return false;
      }
    }

    for (std::size_t k = 0; k < active_indices_.size(); ++k) {
      const int idx = active_indices_[k];
      if (idx < 0 || static_cast<std::size_t>(idx) >= state_.size()) {
        if (error) *error = "active index out of range";
        return false;
      }
      if (state_[idx] == 0) {
        if (error) *error = "active index points at void state";
        return false;
      }
      if (active_pos_[idx] != static_cast<int>(k)) {
        if (error) *error = "active position back-reference mismatch";
        return false;
      }
      if (seen[idx]++) {
        if (error) *error = "duplicate active index";
        return false;
      }
    }

    if (!ordered_active_dirty_) {
      if (ordered_active_indices_.size() != active_indices_.size()) {
        if (error) *error = "ordered active cache size mismatch";
        return false;
      }
      if (!std::is_sorted(ordered_active_indices_.begin(),
                          ordered_active_indices_.end())) {
        if (error) *error = "ordered active cache is not sorted";
        return false;
      }
      std::vector<int> sorted_active = active_indices_;
      std::sort(sorted_active.begin(), sorted_active.end());
      if (ordered_active_indices_ != sorted_active) {
        if (error) *error = "ordered active cache differs from active set";
        return false;
      }
      for (int idx : ordered_active_indices_) {
        if (idx < 0 || static_cast<std::size_t>(idx) >= state_.size() ||
            state_[idx] == 0) {
          if (error) *error = "ordered active cache contains invalid index";
          return false;
        }
        if (seen[idx] == 0) {
          if (error) *error = "ordered active cache missing active index";
          return false;
        }
      }
    }

    if (pos != positive_count_ || neg != negative_count_ ||
        charge != charge_sum_ ||
        static_cast<int>(active_indices_.size()) != pos + neg) {
      if (error) *error = "cached counts do not match state";
      return false;
    }
    return true;
  }

  static int8_t normalize(int8_t s) {
    if (s > 0) return 1;
    if (s < 0) return -1;
    return 0;
  }

private:
  void remove_active(std::size_t idx) {
    const int pos = active_pos_[idx];
    if (pos < 0) return;
    const int last = active_indices_.back();
    active_indices_[pos] = last;
    active_pos_[last] = pos;
    active_indices_.pop_back();
    active_pos_[idx] = -1;
    ordered_active_dirty_ = true;
  }

  void set_bits(std::size_t idx, int8_t s) {
    const std::size_t word = idx >> 6u;
    const std::uint64_t mask = std::uint64_t{1} << (idx & 63u);
    pos_bits_[word] &= ~mask;
    neg_bits_[word] &= ~mask;
    occupied_bits_[word] &= ~mask;
    if (s > 0) pos_bits_[word] |= mask;
    if (s < 0) neg_bits_[word] |= mask;
    if (s != 0) occupied_bits_[word] |= mask;
  }

  static bool bit_at(const std::vector<std::uint64_t>& bits,
                     std::size_t idx) {
    if (bits.empty()) return false;
    return (bits[idx >> 6u] & (std::uint64_t{1} << (idx & 63u))) != 0;
  }

  std::vector<int8_t> state_;
  std::vector<std::uint64_t> pos_bits_;
  std::vector<std::uint64_t> neg_bits_;
  std::vector<std::uint64_t> occupied_bits_;
  std::vector<int> active_indices_;
  mutable std::vector<int> ordered_active_indices_;
  std::vector<int> active_pos_;
  mutable bool ordered_active_dirty_ = false;
  int positive_count_ = 0;
  int negative_count_ = 0;
  long long charge_sum_ = 0;
};

struct FieldSoA {
  std::vector<double> flux_x, flux_y, flux_z;
  std::vector<double> wave_vel_x, wave_vel_y, wave_vel_z;
  std::vector<double> flux_L_x, flux_L_y, flux_L_z;
  std::vector<double> flux_R_x, flux_R_y, flux_R_z;
  std::vector<double> wave_vel_L_x, wave_vel_L_y, wave_vel_L_z;
  std::vector<double> wave_vel_R_x, wave_vel_R_y, wave_vel_R_z;
  std::vector<double> flux_strong_x, flux_strong_y, flux_strong_z;
  std::vector<double> wave_vel_strong_x, wave_vel_strong_y, wave_vel_strong_z;
  std::vector<double> flux_weak_x, flux_weak_y, flux_weak_z;
  std::vector<double> wave_vel_weak_x, wave_vel_weak_y, wave_vel_weak_z;

  void resize_primary(std::size_t n) {
    flux_x.assign(n, 0.0);
    flux_y.assign(n, 0.0);
    flux_z.assign(n, 0.0);
    wave_vel_x.assign(n, 0.0);
    wave_vel_y.assign(n, 0.0);
    wave_vel_z.assign(n, 0.0);
    clear_optional();
  }

  std::size_t size() const { return flux_x.size(); }
  bool primary_sized() const {
    return flux_y.size() == flux_x.size() &&
           flux_z.size() == flux_x.size() &&
           wave_vel_x.size() == flux_x.size() &&
           wave_vel_y.size() == flux_x.size() &&
           wave_vel_z.size() == flux_x.size();
  }
  bool optional_empty() const {
    return flux_L_x.empty() && flux_L_y.empty() && flux_L_z.empty() &&
           flux_R_x.empty() && flux_R_y.empty() && flux_R_z.empty() &&
           wave_vel_L_x.empty() && wave_vel_L_y.empty() && wave_vel_L_z.empty() &&
           wave_vel_R_x.empty() && wave_vel_R_y.empty() && wave_vel_R_z.empty() &&
           flux_strong_x.empty() && flux_strong_y.empty() && flux_strong_z.empty() &&
           wave_vel_strong_x.empty() && wave_vel_strong_y.empty() &&
           wave_vel_strong_z.empty() &&
           flux_weak_x.empty() && flux_weak_y.empty() && flux_weak_z.empty() &&
           wave_vel_weak_x.empty() && wave_vel_weak_y.empty() &&
           wave_vel_weak_z.empty();
  }

  Vec3 flux_at(std::size_t idx) const {
    return {flux_x[idx], flux_y[idx], flux_z[idx]};
  }
  Vec3 wave_vel_at(std::size_t idx) const {
    return {wave_vel_x[idx], wave_vel_y[idx], wave_vel_z[idx]};
  }
  double density_at(std::size_t idx) const {
    const double x = flux_x[idx];
    const double y = flux_y[idx];
    const double z = flux_z[idx];
    return std::sqrt(x * x + y * y + z * z);
  }

  void rebuild_primary_from_voxels(const std::vector<Voxel>& voxels) {
    if (flux_x.size() != voxels.size()) {
      resize_primary(voxels.size());
    }
    for (std::size_t i = 0; i < voxels.size(); ++i) {
      flux_x[i] = voxels[i].flux.x;
      flux_y[i] = voxels[i].flux.y;
      flux_z[i] = voxels[i].flux.z;
      wave_vel_x[i] = voxels[i].wave_vel.x;
      wave_vel_y[i] = voxels[i].wave_vel.y;
      wave_vel_z[i] = voxels[i].wave_vel.z;
    }
  }

  void clear() {
    flux_x.clear(); flux_y.clear(); flux_z.clear();
    wave_vel_x.clear(); wave_vel_y.clear(); wave_vel_z.clear();
    clear_optional();
  }

  void clear_optional() {
    flux_L_x.clear(); flux_L_y.clear(); flux_L_z.clear();
    flux_R_x.clear(); flux_R_y.clear(); flux_R_z.clear();
    wave_vel_L_x.clear(); wave_vel_L_y.clear(); wave_vel_L_z.clear();
    wave_vel_R_x.clear(); wave_vel_R_y.clear(); wave_vel_R_z.clear();
    flux_strong_x.clear(); flux_strong_y.clear(); flux_strong_z.clear();
    wave_vel_strong_x.clear(); wave_vel_strong_y.clear(); wave_vel_strong_z.clear();
    flux_weak_x.clear(); flux_weak_y.clear(); flux_weak_z.clear();
    wave_vel_weak_x.clear(); wave_vel_weak_y.clear(); wave_vel_weak_z.clear();
  }
};

struct ParticleMetaSoA {
  std::vector<double> velocity_x, velocity_y, velocity_z;
  std::vector<double> remainder_x, remainder_y, remainder_z;
  std::vector<double> latency, tau, accel_mag;
  std::vector<std::uint8_t> locked;
  std::vector<std::int32_t> particle_id;
  std::vector<int> pair_id;
  std::vector<int8_t> spin, color, flavor;

  void clear() {
    velocity_x.clear(); velocity_y.clear(); velocity_z.clear();
    remainder_x.clear(); remainder_y.clear(); remainder_z.clear();
    latency.clear(); tau.clear(); accel_mag.clear();
    locked.clear(); particle_id.clear(); pair_id.clear();
    spin.clear(); color.clear(); flavor.clear();
  }
};

struct ScratchSoA {
  std::vector<double> scalar_a;
  std::vector<double> scalar_b;
  std::vector<std::uint8_t> mask_a;

  void clear() {
    scalar_a.clear();
    scalar_b.clear();
    mask_a.clear();
  }
};

struct EngineState {
  EngineState() = default;
  explicit EngineState(std::size_t n) { resize(n); }

  void resize(std::size_t n) {
    total_sites = n;
    ternary.resize(n);
    fields.resize_primary(n);
    particle_meta.clear();
    scratch.clear();
  }

  std::size_t total_sites = 0;
  TernaryField ternary;
  FieldSoA fields;
  ParticleMetaSoA particle_meta;
  ScratchSoA scratch;
};

}  // namespace ftd
