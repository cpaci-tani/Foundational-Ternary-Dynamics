#pragma once
/**
 * @file exact_travelling_mode.h
 * @brief Exact one-axis travelling eigenmode of the production wave tick.
 */

#include "ftd/constants.h"
#include "ftd/voxel.h"

#include <cmath>

namespace ftd::eft {

struct ExactTravellingModeSample {
  double flux = 0.0;
  double wave_vel = 0.0;
};

inline double exact_axis_mode_omega(int length, int mode_number) {
  const double wave_number = 2.0 * PI * static_cast<double>(mode_number)
      / static_cast<double>(length);
  return 2.0 * std::asin(
      C_WAVE * std::abs(std::sin(0.5 * wave_number)));
}

inline ExactTravellingModeSample exact_axis_travelling_mode(
    int coordinate, int length, int mode_number, double phase,
    int propagation_sign, double amplitude) {
  const double wave_number = 2.0 * PI * static_cast<double>(mode_number)
      / static_cast<double>(length);
  const double theta = wave_number * static_cast<double>(coordinate) + phase;
  const double omega = exact_axis_mode_omega(length, mode_number);
  const double sine = std::sin(theta);
  const double cosine = std::cos(theta);
  return {
      amplitude * sine,
      amplitude * ((1.0 - std::cos(omega)) * sine
                   - static_cast<double>(propagation_sign)
                       * std::sin(omega) * cosine)};
}

}  // namespace ftd::eft

