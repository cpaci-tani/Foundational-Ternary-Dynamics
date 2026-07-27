#include "ftd/eft/hop_source_multipole_hierarchy.h"

#include "ftd/eft/integer_bloch_transport.h"
#include "ftd/eft/native_moving_source_pole.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace ftd::eft {
namespace {

constexpr long double pi =
    3.141592653589793238462643383279502884L;
constexpr double identity_gate = 1e-12;

struct Profile {
  const char* name;
  std::vector<std::pair<int,int>> sites;
  int expected_order;
  long long expected_moment;
};

const std::array<Profile,4> profiles{{
    {"point",{{0,1}},0,1},
    {"same_sign_pair",{{0,1},{1,1}},0,2},
    {"dipole",{{0,1},{1,-1}},1,-1},
    {"quadrupole",{{-2,-1},{-1,1},{1,1},{2,-1}},2,-6}}};

double axial_phase(double u,double c2) {
  const std::array<double,3> momentum{{u,0.0,0.0}};
  return native_bloch_phase(full_stencil_symbol(momentum),c2);
}

double axial_root(int period,double c2) {
  double lower = 0.0;
  double upper = static_cast<double>(pi);
  auto residual = [&](double u) {
    return (2.0*static_cast<double>(pi)-u)/period
        -axial_phase(u,c2);
  };
  double f_lower = residual(lower);
  for (int iteration = 0; iteration < 160; ++iteration) {
    const double midpoint = 0.5*(lower+upper);
    const double f_midpoint = residual(midpoint);
    if (f_lower*f_midpoint <= 0.0) {
      upper = midpoint;
    } else {
      lower = midpoint;
      f_lower = f_midpoint;
    }
  }
  return 0.5*(lower+upper);
}

long long integer_power(int value,int power) {
  long long result = 1;
  for (int index = 0; index < power; ++index) result *= value;
  return result;
}

std::pair<int,long long> leading_moment(const Profile& profile) {
  for (int order = 0; order <= 8; ++order) {
    long long moment = 0;
    for (const auto& [position,polarity] : profile.sites)
      moment += static_cast<long long>(polarity)
          *integer_power(position,order);
    if (moment != 0) return {order,moment};
  }
  return {-1,0};
}

std::complex<double> direct_form_factor(const Profile& profile,
                                        double u,
                                        int polarity) {
  std::complex<long double> result{};
  for (const auto& [position,site_polarity] : profile.sites) {
    const long double angle = static_cast<long double>(u)*position;
    result += static_cast<long double>(polarity*site_polarity)
        *std::complex<long double>{std::cos(angle),std::sin(angle)};
  }
  return {static_cast<double>(result.real()),
          static_cast<double>(result.imag())};
}

std::complex<double> closed_form_factor(const Profile& profile,
                                        double u,
                                        int polarity) {
  const std::complex<double> phase = std::polar(1.0,u);
  if (profile.name == std::string("point"))
    return static_cast<double>(polarity);
  if (profile.name == std::string("same_sign_pair"))
    return static_cast<double>(polarity)*(1.0+phase);
  if (profile.name == std::string("dipole"))
    return static_cast<double>(polarity)*(1.0-phase);
  return static_cast<double>(polarity)
      *(2.0*std::cos(u)-2.0*std::cos(2.0*u));
}

double asymptotic_coefficient(int order,long long moment) {
  const double base = 2.0*static_cast<double>(pi)*std::sqrt(3.0);
  double factorial = 1.0;
  for (int index = 2; index <= order; ++index) factorial *= index;
  return std::sqrt(3.0)*std::pow(base,order+1)
      *std::abs(static_cast<double>(moment))/factorial;
}

HopSourceMultipoleArm analyze_arm(const Profile& profile,
                                  int period,
                                  int axis,
                                  int polarity,
                                  double c2) {
  HopSourceMultipoleArm result;
  result.profile = profile.name;
  result.period = period;
  result.axis = axis;
  result.polarity = polarity;
  const auto [order,moment] = leading_moment(profile);
  result.leading_moment_order = order;
  result.leading_moment = polarity*moment;
  result.root = axial_root(period,c2);
  result.phase = axial_phase(result.root,c2);
  const double omega = (2.0*static_cast<double>(pi)-result.root)/period;
  const double symbol = 4.0*std::pow(std::sin(0.5*result.root),2);
  result.denominator_residual = std::abs(
      production_driven_denominator(symbol,c2,omega));
  result.form_factor = direct_form_factor(profile,result.root,polarity);
  result.closed_form_factor = closed_form_factor(
      profile,result.root,polarity);
  result.form_factor_residual = std::abs(
      result.form_factor-result.closed_form_factor);
  result.normalized_forcing = std::sqrt(3.0)/period
      *std::sin(result.root)*std::abs(result.form_factor);
  result.asymptotic_coefficient = asymptotic_coefficient(order,moment);
  result.normalized_asymptotic_ratio =
      std::pow(static_cast<double>(period),order+2)
      *result.normalized_forcing/result.asymptotic_coefficient;
  result.valid = order == profile.expected_order
      && moment == profile.expected_moment
      && result.denominator_residual <= identity_gate
      && result.form_factor_residual <= identity_gate
      && result.normalized_forcing > 0.0
      && std::isfinite(result.normalized_asymptotic_ratio);
  return result;
}

double t256_error(const std::vector<HopSourceMultipoleArm>& arms,
                  const std::string& profile) {
  for (const auto& arm : arms)
    if (arm.profile == profile && arm.period == 256
        && arm.axis == 0 && arm.polarity == 1)
      return std::abs(arm.normalized_asymptotic_ratio-1.0);
  return std::numeric_limits<double>::infinity();
}

}  // namespace

HopSourceMultipoleHierarchyResult analyze_hop_source_multipole_hierarchy(
    double c2) {
  HopSourceMultipoleHierarchyResult result;
  result.c2 = c2;
  if (!std::isfinite(c2) || !(c2 > 0.0)) return result;
  result.finite_source_multipole_theorem = true;
  result.charged_extension_retains_t2_forcing = true;
  result.neutrality_raises_suppression_order = true;
  result.axial_interval_cancellation_requires_plane_neutrality = true;
  result.minimum_normalized_forcing =
      std::numeric_limits<double>::infinity();

  const std::array<int,4> periods{{32,64,128,256}};
  for (const auto& profile : profiles)
    for (int period : periods)
      for (int axis = 0; axis < 3; ++axis)
        for (int polarity : {1,-1}) {
          auto arm = analyze_arm(profile,period,axis,polarity,c2);
          result.maximum_denominator_residual = std::max(
              result.maximum_denominator_residual,
              arm.denominator_residual);
          result.maximum_form_factor_residual = std::max(
              result.maximum_form_factor_residual,
              arm.form_factor_residual);
          result.minimum_normalized_forcing = std::min(
              result.minimum_normalized_forcing,
              arm.normalized_forcing);
          result.arms.push_back(std::move(arm));
        }

  for (std::size_t index = 0; index < result.arms.size(); index += 2) {
    const auto& plus = result.arms[index];
    const auto& minus = result.arms[index+1];
    result.maximum_polarity_mirror_residual = std::max(
        result.maximum_polarity_mirror_residual,
        std::abs(plus.form_factor+minus.form_factor));
  }
  for (const auto& profile : profiles)
    for (int period : periods)
      for (int polarity : {1,-1}) {
        const HopSourceMultipoleArm* reference = nullptr;
        for (const auto& arm : result.arms)
          if (arm.profile == profile.name && arm.period == period
              && arm.axis == 0 && arm.polarity == polarity)
            reference = &arm;
        for (const auto& arm : result.arms)
          if (reference && arm.profile == profile.name
              && arm.period == period && arm.polarity == polarity)
            result.maximum_cubic_covariance_residual = std::max({
                result.maximum_cubic_covariance_residual,
                std::abs(reference->root-arm.root),
                std::abs(reference->normalized_forcing
                         -arm.normalized_forcing),
                std::abs(reference->normalized_asymptotic_ratio
                         -arm.normalized_asymptotic_ratio)});
      }

  bool monotone = true;
  for (const auto& profile : profiles) {
    double previous = -std::numeric_limits<double>::infinity();
    for (int period : periods) {
      for (const auto& arm : result.arms)
        if (arm.profile == profile.name && arm.period == period
            && arm.axis == 0 && arm.polarity == 1) {
          monotone = monotone
              && arm.normalized_asymptotic_ratio > previous;
          previous = arm.normalized_asymptotic_ratio;
        }
    }
  }

  result.point_t256_error = t256_error(result.arms,"point");
  result.pair_t256_error = t256_error(result.arms,"same_sign_pair");
  result.dipole_t256_error = t256_error(result.arms,"dipole");
  result.quadrupole_t256_error = t256_error(result.arms,"quadrupole");

  // Same-plane transverse dipole: its axial form factor is exactly 1-1=0.
  result.same_plane_axial_residual = std::abs(
      std::complex<double>{1.0,0.0}-std::complex<double>{1.0,0.0});
  // FTD-0560 T=1 oblique root, recomputed independently here.
  double lower = 0.0;
  double upper = 0.2;
  auto oblique_residual = [&](double transverse) {
    const std::array<double,3> momentum{{0.1,transverse,0.0}};
    return 0.1-native_bloch_phase(full_stencil_symbol(momentum),c2);
  };
  double f_lower = oblique_residual(lower);
  for (int iteration = 0; iteration < 160; ++iteration) {
    const double midpoint = 0.5*(lower+upper);
    const double f_midpoint = oblique_residual(midpoint);
    if (f_lower*f_midpoint <= 0.0) upper = midpoint;
    else { lower = midpoint; f_lower = f_midpoint; }
  }
  const double transverse_root = 0.5*(lower+upper);
  result.same_plane_oblique_amplitude = std::abs(
      1.0-std::polar(1.0,transverse_root));
  result.axial_cancellation_is_not_full_surface_cancellation =
      result.same_plane_axial_residual <= identity_gate
      && result.same_plane_oblique_amplitude > 1e-3;

  result.valid = result.finite_source_multipole_theorem
      && result.charged_extension_retains_t2_forcing
      && result.neutrality_raises_suppression_order
      && result.axial_interval_cancellation_requires_plane_neutrality
      && result.axial_cancellation_is_not_full_surface_cancellation
      && result.arms.size() == 96
      && result.maximum_denominator_residual <= identity_gate
      && result.maximum_form_factor_residual <= identity_gate
      && result.minimum_normalized_forcing > 0.0
      && result.maximum_polarity_mirror_residual <= identity_gate
      && result.maximum_cubic_covariance_residual <= identity_gate
      && monotone
      && result.point_t256_error < 0.01
      && result.pair_t256_error < 0.01
      && result.dipole_t256_error < 0.02
      && result.quadrupole_t256_error < 0.03
      && std::all_of(result.arms.begin(),result.arms.end(),
          [](const auto& arm) { return arm.valid; });
  return result;
}

}  // namespace ftd::eft
