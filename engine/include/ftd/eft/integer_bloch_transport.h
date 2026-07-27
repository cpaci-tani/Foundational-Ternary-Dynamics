#pragma once
/**
 * @file integer_bloch_transport.h
 * @brief Observer-only Bloch analysis of the isolated production wave map
 *        (FTD-0556).
 */

#include "ftd/lattice.h"

#include <array>
#include <complex>
#include <vector>

namespace ftd::eft {

struct NativeBlochModeDiagnostics {
  bool valid = false;
  int L = 0;
  int mode_number = 0;
  Coord direction{};
  std::array<double, 3> momentum{};
  std::array<double, 3> group_velocity{};
  double symbol = 0.0;
  double kick = 0.0;
  double phase = 0.0;
  double invariant_determinant = 0.0;
  double determinant_residual = 0.0;
  double characteristic_residual = 0.0;
  double eigenvalue_modulus_residual = 0.0;
  double eigenvector_residual = 0.0;
  double invariant_residual = 0.0;
  double infrared_sixth_order_residual = 0.0;
  double maximum_identity_residual = 0.0;
};

struct IntegerBlochTransportResult {
  bool valid = false;
  bool scalar_finite_laurent_unitary_is_monomial = false;
  bool scalar_dispersive_band_requires_type_escape = false;
  bool native_pair_is_symplectic = false;
  int L = 0;
  double c2 = 0.0;
  double maximum_identity_residual = 0.0;
  double maximum_group_speed = 0.0;
  double maximum_ir_sixth_order_residual = 0.0;
  std::vector<NativeBlochModeDiagnostics> modes;
};

double full_stencil_symbol(const std::array<double, 3>& momentum);

std::array<double, 3> full_stencil_symbol_gradient(
    const std::array<double, 3>& momentum);

double native_bloch_phase(double symbol, double c2);

std::array<std::complex<double>, 2> native_bloch_step(
    const std::array<std::complex<double>, 2>& state,
    double kick);

double native_bloch_invariant(
    const std::array<std::complex<double>, 2>& state,
    double kick);

IntegerBlochTransportResult analyze_integer_bloch_transport(
    int L,
    const std::vector<int>& mode_numbers,
    const std::vector<Coord>& directions,
    double c2);

}  // namespace ftd::eft

