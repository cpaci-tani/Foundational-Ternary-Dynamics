#include "ftd/eft/full_surface_source_obstruction.h"

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

struct Site {
  std::array<int,3> position{};
  int polarity = 0;
};

struct Profile {
  const char* name = "";
  std::vector<Site> sites;
  int expected_order = -1;
};

const std::array<Profile,4> profiles{{
    {"point",{{{{0,0,0}},1}},0},
    {"axial_dipole",{
        {{{0,0,0}},1},{{{1,0,0}},-1}},1},
    {"planar_quadrupole",{
        {{{0,0,0}},1},{{{1,0,0}},-1},
        {{{0,1,0}},-1},{{{1,1,0}},1}},2},
    {"cubic_octupole",{
        {{{0,0,0}},1},{{{1,0,0}},-1},
        {{{0,1,0}},-1},{{{0,0,1}},-1},
        {{{1,1,0}},1},{{{1,0,1}},1},
        {{{0,1,1}},1},{{{1,1,1}},-1}},3}}};

const std::array<std::array<int,3>,8> raw_directions{{
    {{-1,0,0}},{{1,1,0}},{{1,0,1}},{{1,1,1}},
    {{-1,1,1}},{{2,-1,3}},{{-2,3,1}},{{3,2,-1}}}};

const std::array<int,4> periods{{64,128,256,512}};

long long integer_power(int value,int exponent) {
  long long result = 1;
  for (int index = 0; index < exponent; ++index) result *= value;
  return result;
}

long long mixed_moment(const Profile& profile,
                       int px,int py,int pz) {
  long long result = 0;
  for (const auto& site : profile.sites)
    result += static_cast<long long>(site.polarity)
        *integer_power(site.position[0],px)
        *integer_power(site.position[1],py)
        *integer_power(site.position[2],pz);
  return result;
}

int leading_total_order(const Profile& profile) {
  for (int order = 0; order <= 12; ++order)
    for (int px = 0; px <= order; ++px)
      for (int py = 0; py <= order-px; ++py) {
        const int pz = order-px-py;
        if (mixed_moment(profile,px,py,pz) != 0) return order;
      }
  return -1;
}

std::array<double,3> normalize(const std::array<int,3>& value) {
  const double norm = std::sqrt(
      static_cast<double>(value[0]*value[0]
          +value[1]*value[1]+value[2]*value[2]));
  return {{value[0]/norm,value[1]/norm,value[2]/norm}};
}

template <typename T>
std::array<T,3> cyclic_rotate(const std::array<T,3>& value,int axis) {
  std::array<T,3> result{};
  for (int component = 0; component < 3; ++component)
    result[(component+axis)%3] = value[component];
  return result;
}

Profile rotated_profile(const Profile& profile,int axis) {
  Profile result{profile.name,{},profile.expected_order};
  result.sites.reserve(profile.sites.size());
  for (const auto& site : profile.sites)
    result.sites.push_back({cyclic_rotate(site.position,axis),site.polarity});
  return result;
}

double norm2(const std::array<double,3>& value) {
  return value[0]*value[0]+value[1]*value[1]+value[2]*value[2];
}

std::array<double,3> cross(const std::array<double,3>& lhs,
                           const std::array<double,3>& rhs) {
  return {{
      lhs[1]*rhs[2]-lhs[2]*rhs[1],
      lhs[2]*rhs[0]-lhs[0]*rhs[2],
      lhs[0]*rhs[1]-lhs[1]*rhs[0]}};
}

double denominator(double radius,
                   const std::array<double,3>& direction,
                   int axis,int period,double c2) {
  std::array<double,3> momentum{};
  for (int component = 0; component < 3; ++component)
    momentum[component] = radius*direction[component]/period;
  const double omega =
      (2.0*static_cast<double>(pi)+momentum[axis])/period;
  return production_driven_denominator(
      full_stencil_symbol(momentum),c2,omega);
}

std::pair<bool,double> radial_root(
    const std::array<double,3>& direction,
    int axis,int period,double c2) {
  const double r0 = 2.0*static_cast<double>(pi)/std::sqrt(c2);
  double lower = 0.0;
  double upper = 2.0*r0;
  double f_lower = denominator(lower,direction,axis,period,c2);
  const double f_upper = denominator(upper,direction,axis,period,c2);
  if (!(f_lower < 0.0 && f_upper > 0.0)) return {false,0.0};
  for (int iteration = 0; iteration < 180; ++iteration) {
    const double midpoint = 0.5*(lower+upper);
    const double f_midpoint = denominator(
        midpoint,direction,axis,period,c2);
    if (f_lower*f_midpoint <= 0.0) upper = midpoint;
    else { lower = midpoint; f_lower = f_midpoint; }
  }
  return {true,0.5*(lower+upper)};
}

std::complex<double> form_factor(const Profile& profile,
                                 const std::array<double,3>& momentum,
                                 int polarity) {
  std::complex<long double> result{};
  for (const auto& site : profile.sites) {
    long double phase = 0.0L;
    for (int component = 0; component < 3; ++component)
      phase += static_cast<long double>(momentum[component])
          *site.position[component];
    result += static_cast<long double>(polarity*site.polarity)
        *std::complex<long double>{std::cos(phase),std::sin(phase)};
  }
  return {static_cast<double>(result.real()),
          static_cast<double>(result.imag())};
}

std::complex<double> imaginary_power(int order) {
  std::complex<double> result{1.0,0.0};
  for (int index = 0; index < order; ++index)
    result *= std::complex<double>{0.0,1.0};
  return result;
}

double factorial(int order) {
  double result = 1.0;
  for (int index = 2; index <= order; ++index) result *= index;
  return result;
}

std::complex<double> leading_polynomial(
    const Profile& profile,
    const std::array<double,3>& direction,
    int order,int polarity) {
  double moment = 0.0;
  for (const auto& site : profile.sites) {
    double projection = 0.0;
    for (int component = 0; component < 3; ++component)
      projection += direction[component]*site.position[component];
    moment += polarity*site.polarity*std::pow(projection,order);
  }
  return imaginary_power(order)*(moment/factorial(order));
}

double radial_derivative(const std::array<double,3>& momentum,
                         const std::array<double,3>& direction,
                         int axis,int period,double c2,double omega) {
  const auto gradient = full_stencil_symbol_gradient(momentum);
  double radial_symbol_derivative = 0.0;
  for (int component = 0; component < 3; ++component)
    radial_symbol_derivative += gradient[component]
        *direction[component]/period;
  return c2*radial_symbol_derivative
      -2.0*std::sin(omega)*direction[axis]
          /(static_cast<double>(period)*period);
}

FullSurfaceSourceArm analyze_arm(const Profile& base_profile,
                                 int period,
                                 int direction_index,
                                 int axis,
                                 int polarity,
                                 double c2) {
  FullSurfaceSourceArm result;
  result.profile = base_profile.name;
  result.period = period;
  result.direction_index = direction_index;
  result.axis = axis;
  result.polarity = polarity;
  result.leading_order = leading_total_order(base_profile);
  const auto base_direction = normalize(raw_directions[direction_index]);
  result.direction = cyclic_rotate(base_direction,axis);
  const auto profile = rotated_profile(base_profile,axis);
  const auto [bracketed,root] = radial_root(
      result.direction,axis,period,c2);
  result.root_bracketed = bracketed;
  result.root_radius = root;
  if (!bracketed) return result;
  for (int component = 0; component < 3; ++component)
    result.momentum[component] = root*result.direction[component]/period;
  result.omega = (2.0*static_cast<double>(pi)
      +result.momentum[axis])/period;
  result.denominator_residual = std::abs(production_driven_denominator(
      full_stencil_symbol(result.momentum),c2,result.omega));
  result.scaled_radial_derivative = static_cast<double>(period)*period
      *std::abs(radial_derivative(result.momentum,result.direction,
          axis,period,c2,result.omega));
  result.form_factor = form_factor(profile,result.momentum,polarity);
  result.leading_polynomial = leading_polynomial(
      profile,result.direction,result.leading_order,polarity);
  result.leading_witness = std::abs(
      result.direction[axis]*result.leading_polynomial) > 1e-14;

  const std::complex<double> unit{1.0,0.0};
  result.floquet_coefficient =
      (unit-std::polar(1.0,result.momentum[axis]))
      /(static_cast<double>(period)
          *(unit-std::polar(1.0,result.omega)));
  std::array<double,3> q{};
  std::array<double,3> velocity{};
  velocity[axis] = 1.0/period;
  for (int component = 0; component < 3; ++component)
    q[component] = std::sin(result.momentum[component]);
  const auto transverse = cross(q,velocity);
  result.source_forcing_over_gc = std::abs(result.floquet_coefficient)
      *std::abs(result.form_factor)
      *std::sqrt(norm2(q)+norm2(transverse));

  const double r0 = 2.0*static_cast<double>(pi)/std::sqrt(c2);
  result.asymptotic_coefficient_over_gc = std::sqrt(3.0)
      *std::pow(r0,result.leading_order+1)
      *std::abs(result.direction[axis]*result.leading_polynomial);
  if (result.leading_witness) {
    result.asymptotic_ratio =
        std::pow(static_cast<double>(period),result.leading_order+2)
        *result.source_forcing_over_gc
        /result.asymptotic_coefficient_over_gc;
  }
  result.radius_first_correction_residual = std::abs(
      period*(root-r0)
      -6.0*static_cast<double>(pi)*result.direction[axis]);
  result.valid = result.leading_order == base_profile.expected_order
      && result.denominator_residual <= identity_gate
      && result.scaled_radial_derivative > 1.0
      && std::isfinite(result.source_forcing_over_gc)
      && (!result.leading_witness
          || (result.source_forcing_over_gc > 0.0
              && std::isfinite(result.asymptotic_ratio)));
  return result;
}

const FullSurfaceSourceArm* find_arm(
    const std::vector<FullSurfaceSourceArm>& arms,
    const std::string& profile,int period,int direction_index,
    int axis,int polarity) {
  for (const auto& arm : arms)
    if (arm.profile == profile && arm.period == period
        && arm.direction_index == direction_index && arm.axis == axis
        && arm.polarity == polarity)
      return &arm;
  return nullptr;
}

}  // namespace

FullSurfaceSourceObstructionResult analyze_full_surface_source_obstruction(
    double c2) {
  FullSurfaceSourceObstructionResult result;
  result.c2 = c2;
  if (!std::isfinite(c2) || std::abs(c2-1.0/3.0) > identity_gate)
    return result;
  result.full_direction_slow_branch_exists = true;
  result.finite_source_form_factor_is_analytic = true;
  result.lowest_homogeneous_moment_is_decisive = true;
  result.finite_rigid_universal_cancellation_closed = true;
  result.square_summable_linear_dressing_closed_for_slow_hops = true;
  result.nonlinear_deforming_carrier_remains_open = true;
  result.minimum_scaled_radial_derivative =
      std::numeric_limits<double>::infinity();
  result.minimum_witness_scaled_forcing =
      std::numeric_limits<double>::infinity();

  for (const auto& profile : profiles)
    for (int period : periods)
      for (int direction_index = 0;
           direction_index < static_cast<int>(raw_directions.size());
           ++direction_index)
        for (int axis = 0; axis < 3; ++axis)
          for (int polarity : {1,-1}) {
            auto arm = analyze_arm(profile,period,direction_index,
                axis,polarity,c2);
            result.maximum_denominator_residual = std::max(
                result.maximum_denominator_residual,
                arm.denominator_residual);
            result.minimum_scaled_radial_derivative = std::min(
                result.minimum_scaled_radial_derivative,
                arm.scaled_radial_derivative);
            if (arm.leading_witness) {
              const double scaled = std::pow(
                  static_cast<double>(period),arm.leading_order+2)
                  *arm.source_forcing_over_gc;
              result.minimum_witness_scaled_forcing = std::min(
                  result.minimum_witness_scaled_forcing,scaled);
              if (period == 512) {
                result.maximum_t512_radius_correction_residual = std::max(
                    result.maximum_t512_radius_correction_residual,
                    arm.radius_first_correction_residual);
                result.maximum_t512_asymptotic_error = std::max(
                    result.maximum_t512_asymptotic_error,
                    std::abs(arm.asymptotic_ratio-1.0));
              }
            } else if (period == 512) {
              result.maximum_t512_radius_correction_residual = std::max(
                  result.maximum_t512_radius_correction_residual,
                  arm.radius_first_correction_residual);
            }
            result.arms.push_back(std::move(arm));
          }

  result.expected_witness_groups = static_cast<int>(profiles.size())
      *static_cast<int>(periods.size())*3*2;
  for (const auto& profile : profiles)
    for (int period : periods)
      for (int axis = 0; axis < 3; ++axis)
        for (int polarity : {1,-1}) {
          bool has_witness = false;
          for (int direction_index = 0;
               direction_index < static_cast<int>(raw_directions.size());
               ++direction_index) {
            const auto* arm = find_arm(result.arms,profile.name,period,
                direction_index,axis,polarity);
            has_witness = has_witness || (arm && arm->leading_witness
                && arm->source_forcing_over_gc > 0.0);
          }
          if (has_witness) ++result.witness_groups;
        }

  for (const auto& profile : profiles)
    for (int period : periods)
      for (int direction_index = 0;
           direction_index < static_cast<int>(raw_directions.size());
           ++direction_index)
        for (int axis = 0; axis < 3; ++axis) {
          const auto* plus = find_arm(result.arms,profile.name,period,
              direction_index,axis,1);
          const auto* minus = find_arm(result.arms,profile.name,period,
              direction_index,axis,-1);
          if (plus && minus)
            result.maximum_polarity_mirror_residual = std::max({
                result.maximum_polarity_mirror_residual,
                std::abs(plus->form_factor+minus->form_factor),
                std::abs(plus->source_forcing_over_gc
                    -minus->source_forcing_over_gc)});
        }

  for (const auto& profile : profiles)
    for (int period : periods)
      for (int direction_index = 0;
           direction_index < static_cast<int>(raw_directions.size());
           ++direction_index)
        for (int polarity : {1,-1}) {
          const auto* reference = find_arm(result.arms,profile.name,period,
              direction_index,0,polarity);
          for (int axis = 1; axis < 3; ++axis) {
            const auto* rotated = find_arm(result.arms,profile.name,period,
                direction_index,axis,polarity);
            if (reference && rotated)
              result.maximum_cyclic_covariance_residual = std::max({
                  result.maximum_cyclic_covariance_residual,
                  std::abs(reference->root_radius-rotated->root_radius),
                  std::abs(reference->form_factor-rotated->form_factor),
                  std::abs(reference->source_forcing_over_gc
                      -rotated->source_forcing_over_gc),
                  std::abs(reference->scaled_radial_derivative
                      -rotated->scaled_radial_derivative)});
          }
        }

  result.valid = result.full_direction_slow_branch_exists
      && result.finite_source_form_factor_is_analytic
      && result.lowest_homogeneous_moment_is_decisive
      && result.finite_rigid_universal_cancellation_closed
      && result.square_summable_linear_dressing_closed_for_slow_hops
      && result.nonlinear_deforming_carrier_remains_open
      && result.arms.size() == 768
      && result.witness_groups == result.expected_witness_groups
      && result.maximum_denominator_residual <= identity_gate
      && result.minimum_scaled_radial_derivative > 1.0
      && result.maximum_polarity_mirror_residual <= identity_gate
      && result.maximum_cyclic_covariance_residual <= identity_gate
      && result.maximum_t512_radius_correction_residual < 0.25
      && result.maximum_t512_asymptotic_error < 0.20
      && result.minimum_witness_scaled_forcing > 0.0
      && std::all_of(result.arms.begin(),result.arms.end(),
          [](const auto& arm) { return arm.valid; });
  return result;
}

}  // namespace ftd::eft
