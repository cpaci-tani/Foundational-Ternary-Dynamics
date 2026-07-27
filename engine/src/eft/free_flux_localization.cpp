#include "ftd/eft/free_flux_localization.h"

#include "ftd/eft/integer_bloch_transport.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

constexpr long double pi =
    3.141592653589793238462643383279502884L;

double coordinate_group_velocity(
    const Coord& direction,
    const std::array<double, 3>& momentum,
    double symbol,
    double c2) {
  const double phase = native_bloch_phase(symbol, c2);
  const double denominator = 2.0*std::sin(phase);
  if (!(denominator > 0.0))
    return std::numeric_limits<double>::quiet_NaN();
  const auto gradient = full_stencil_symbol_gradient(momentum);
  return c2*(direction.x*gradient[0]
      +direction.y*gradient[1]
      +direction.z*gradient[2])/denominator;
}

}  // namespace

FreeFluxLocalizationResult analyze_free_flux_localization(
    int L,
    int first_mode,
    int last_mode,
    double center_mode,
    double width_modes,
    const std::vector<Coord>& directions,
    double c2) {
  FreeFluxLocalizationResult result;
  result.L = L;
  result.c2 = c2;
  result.first_mode = first_mode;
  result.last_mode = last_mode;
  result.center_mode = center_mode;
  result.width_modes = width_modes;
  if (L < 3 || first_mode <= 0 || last_mode < first_mode
      || 2*last_mode >= L || directions.empty()
      || !std::isfinite(center_mode) || !std::isfinite(width_modes)
      || !(width_modes > 0.0) || !std::isfinite(c2) || !(c2 > 0.0)) {
    return result;
  }

  // The FULL-stencil symbol is a finite trigonometric polynomial, hence real
  // analytic.  Since M(0)=0 and M(k)>0 for a sufficiently small nonzero axis
  // momentum, tr U=2-c2*M is nonconstant.  For fixed lambda, the determinant
  // det(U-lambda I) is therefore a nonzero real-analytic function.  Its zero
  // set has measure zero, which cannot support a nonzero L2 Fourier function.
  result.finite_range_symbol_is_real_analytic = true;
  const double axis_symbol = full_stencil_symbol(
      {{2.0*static_cast<double>(pi)/L, 0.0, 0.0}});
  result.transfer_trace_is_nonconstant = axis_symbol > 0.0;

  double minimum_variance = std::numeric_limits<double>::infinity();
  bool every_band_nonflat = true;
  bool every_packet_broadens = true;
  for (const Coord& direction : directions) {
    if (direction.x == 0 && direction.y == 0 && direction.z == 0)
      return FreeFluxLocalizationResult{};
    FreeFluxPacketDiagnostics packet;
    packet.direction = direction;
    packet.spectral_mode_count = last_mode-first_mode+1;
    packet.minimum_symbol = std::numeric_limits<double>::infinity();
    packet.maximum_symbol = -std::numeric_limits<double>::infinity();
    packet.minimum_phase = std::numeric_limits<double>::infinity();
    packet.maximum_phase = -std::numeric_limits<double>::infinity();

    long double norm = 0.0L;
    long double velocity_sum = 0.0L;
    long double velocity_square_sum = 0.0L;
    for (int mode = first_mode; mode <= last_mode; ++mode) {
      const double q = 2.0*static_cast<double>(pi)*mode/L;
      const std::array<double, 3> momentum{{
          q*direction.x, q*direction.y, q*direction.z}};
      const double symbol = full_stencil_symbol(momentum);
      const double phase = native_bloch_phase(symbol, c2);
      const double velocity = coordinate_group_velocity(
          direction, momentum, symbol, c2);
      if (!std::isfinite(symbol) || !std::isfinite(phase)
          || !std::isfinite(velocity)) return FreeFluxLocalizationResult{};
      const double delta = (mode-center_mode)/width_modes;
      const long double weight = std::exp(-delta*delta);
      norm += weight;
      velocity_sum += weight*velocity;
      velocity_square_sum += weight*velocity*velocity;
      packet.minimum_symbol = std::min(packet.minimum_symbol, symbol);
      packet.maximum_symbol = std::max(packet.maximum_symbol, symbol);
      packet.minimum_phase = std::min(packet.minimum_phase, phase);
      packet.maximum_phase = std::max(packet.maximum_phase, phase);
    }
    if (!(norm > 0.0L)) return FreeFluxLocalizationResult{};
    packet.spectral_norm = static_cast<double>(norm);
    packet.mean_coordinate_velocity = static_cast<double>(velocity_sum/norm);
    packet.coordinate_velocity_variance = std::max(0.0,
        static_cast<double>(velocity_square_sum/norm)
        -packet.mean_coordinate_velocity*packet.mean_coordinate_velocity);
    packet.valid = packet.maximum_symbol > packet.minimum_symbol
        && packet.maximum_phase > packet.minimum_phase
        && packet.coordinate_velocity_variance > 0.0;
    every_band_nonflat = every_band_nonflat
        && packet.maximum_phase > packet.minimum_phase;
    every_packet_broadens = every_packet_broadens
        && packet.coordinate_velocity_variance > 0.0;
    minimum_variance = std::min(
        minimum_variance, packet.coordinate_velocity_variance);
    result.packets.push_back(packet);
  }

  result.native_band_is_not_flat = every_band_nonflat;
  result.no_nonzero_l2_point_spectrum =
      result.finite_range_symbol_is_real_analytic
      && result.transfer_trace_is_nonconstant;

  // If U(k)^T equalled a translation phase on a positive-measure set, the
  // corresponding real-analytic determinant would vanish identically.  At
  // k=0 this forces the phase to one.  Along a nonzero direction orthogonal
  // to any proposed integer displacement, the translation phase stays one
  // while the nonflat native eigenphase changes.  Thus the determinant is not
  // identically zero and its zero set again has measure zero.
  result.no_nonzero_finite_time_rigid_l2_translate =
      result.no_nonzero_l2_point_spectrum && result.native_band_is_not_flat;

  // For a single branch, X_i(t)=X_i(0)+t*d_i theta as an exact Fourier-space
  // operator identity.  Taking the centered square gives the covariance law
  // registered in the protocol.  A real nonnegative spectral envelope has
  // zero initial symmetrized X-v covariance.
  result.exact_branch_second_moment_identity = true;
  result.unchirped_localized_packet_must_broaden = every_packet_broadens;
  result.minimum_velocity_variance = minimum_variance;
  result.valid = result.finite_range_symbol_is_real_analytic
      && result.transfer_trace_is_nonconstant
      && result.native_band_is_not_flat
      && result.no_nonzero_l2_point_spectrum
      && result.no_nonzero_finite_time_rigid_l2_translate
      && result.exact_branch_second_moment_identity
      && result.unchirped_localized_packet_must_broaden
      && result.packets.size() == directions.size()
      && std::all_of(result.packets.begin(), result.packets.end(),
          [](const FreeFluxPacketDiagnostics& packet) {
            return packet.valid;
          });
  return result;
}

}  // namespace ftd::eft
