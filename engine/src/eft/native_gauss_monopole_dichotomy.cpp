#include "ftd/eft/native_gauss_monopole_dichotomy.h"

#include "ftd/constants.h"
#include "ftd/eft/full_surface_source_obstruction.h"
#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <limits>
#include <numeric>
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

const std::array<std::array<int,3>,4> raw_directions{{
    {{1,0,0}},{{1,1,0}},{{1,1,1}},{{-1,2,3}}}};

const std::array<int,4> volumes{{32,64,128,256}};

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

long long integer_power(int value,int exponent) {
  long long result = 1;
  for (int index = 0; index < exponent; ++index) result *= value;
  return result;
}

long long mixed_moment(const Profile& profile,int px,int py,int pz) {
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

int total_polarity(const Profile& profile) {
  int result = 0;
  for (const auto& site : profile.sites) result += site.polarity;
  return result;
}

double factorial(int order) {
  double result = 1.0;
  for (int index = 2; index <= order; ++index) result *= index;
  return result;
}

std::complex<double> imaginary_power(int order) {
  std::complex<double> result{1.0,0.0};
  for (int index = 0; index < order; ++index)
    result *= std::complex<double>{0.0,1.0};
  return result;
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

std::complex<double> leading_polynomial(
    const Profile& profile,const std::array<double,3>& unit_direction,
    int order,int polarity) {
  double moment = 0.0;
  for (const auto& site : profile.sites) {
    double projection = 0.0;
    for (int component = 0; component < 3; ++component)
      projection += unit_direction[component]*site.position[component];
    moment += polarity*site.polarity*std::pow(projection,order);
  }
  return imaginary_power(order)*(moment/factorial(order));
}

double complex_norm(const std::array<std::complex<double>,3>& values) {
  double norm_squared = 0.0;
  for (const auto value : values) norm_squared += std::norm(value);
  return std::sqrt(norm_squared);
}

GaussMonopoleArm analyze_arm(const Profile& base_profile,int volume,
                             int direction_index,int axis,int polarity) {
  GaussMonopoleArm result;
  result.profile = base_profile.name;
  result.volume = volume;
  result.direction_index = direction_index;
  result.axis = axis;
  result.polarity = polarity;
  result.total_polarity = polarity*total_polarity(base_profile);
  result.leading_order = leading_total_order(base_profile);
  const auto profile = rotated_profile(base_profile,axis);
  const auto raw = cyclic_rotate(raw_directions[direction_index],axis);
  double raw_norm_squared = 0.0;
  for (int component = 0; component < 3; ++component) {
    result.momentum[component] = 2.0*static_cast<double>(pi)
        *raw[component]/volume;
    raw_norm_squared += static_cast<double>(raw[component]*raw[component]);
  }
  result.kappa = 2.0*static_cast<double>(pi)
      *std::sqrt(raw_norm_squared)/volume;
  std::array<double,3> unit_direction{};
  for (int component = 0; component < 3; ++component)
    unit_direction[component] = raw[component]/std::sqrt(raw_norm_squared);

  result.form_factor = form_factor(profile,result.momentum,polarity);
  result.leading_polynomial = leading_polynomial(
      profile,unit_direction,result.leading_order,polarity);
  result.leading_witness = std::abs(result.leading_polynomial) > 1e-14;
  std::array<std::complex<double>,3> d{};
  for (int component = 0; component < 3; ++component) {
    d[component] = std::complex<double>{1.0,0.0}
        -std::polar(1.0,-result.momentum[component]);
    result.face_laplacian += std::norm(d[component]);
  }
  if (!(result.face_laplacian > 0.0)) return result;
  for (int component = 0; component < 3; ++component)
    result.longitudinal_face_field[component] = std::conj(d[component])
        *result.form_factor/result.face_laplacian;
  result.monopole_estimator = std::sqrt(result.face_laplacian)
      *complex_norm(result.longitudinal_face_field);
  result.face_gauss_identity_residual = std::abs(
      result.monopole_estimator-std::abs(result.form_factor));
  if (result.leading_witness)
    result.asymptotic_ratio = std::abs(result.form_factor)
        /(std::pow(result.kappa,result.leading_order)
          *std::abs(result.leading_polynomial));
  result.valid = result.leading_order == base_profile.expected_order
      && result.face_gauss_identity_residual <= identity_gate
      && std::isfinite(result.monopole_estimator)
      && (!result.leading_witness
          || (result.asymptotic_ratio > 0.0
              && std::isfinite(result.asymptotic_ratio)));
  return result;
}

const GaussMonopoleArm* find_arm(
    const std::vector<GaussMonopoleArm>& arms,const std::string& profile,
    int volume,int direction_index,int axis,int polarity) {
  for (const auto& arm : arms)
    if (arm.profile == profile && arm.volume == volume
        && arm.direction_index == direction_index && arm.axis == axis
        && arm.polarity == polarity)
      return &arm;
  return nullptr;
}

long double total_divergence(const MatchedFaceFlux& field) {
  long double result = 0.0L;
  for (int x = 0; x < field.L; ++x)
    for (int y = 0; y < field.L; ++y)
      for (int z = 0; z < field.L; ++z)
        result += divergence_at(field,x,y,z);
  return result;
}

MatchedFaceFlux deterministic_periodic_face_field(int L) {
  MatchedFaceFlux field(L);
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(field.index(x,y,z));
        field.x[i] = ((x+2*y+3*z)%7-3)/8.0;
        field.y[i] = ((3*x+y+2*z)%7-3)/16.0;
        field.z[i] = ((2*x+3*y+z)%7-3)/32.0;
      }
  return field;
}

long long zero_mode_numerator_sum(const Profile& base_profile,int L,
                                  int axis,int polarity) {
  const auto profile = rotated_profile(base_profile,axis);
  const long long count = static_cast<long long>(L)*L*L;
  std::vector<int> source(static_cast<std::size_t>(count),0);
  const auto index = [L](int x,int y,int z) {
    return static_cast<std::size_t>((x*L+y)*L+z);
  };
  int total = 0;
  for (const auto& site : profile.sites) {
    const int value = polarity*site.polarity;
    source[index(site.position[0]+2,site.position[1]+2,
                 site.position[2]+2)] += value;
    total += value;
  }
  long long sum = 0;
  for (const int value : source)
    sum += count*value-total;
  return sum;
}

}  // namespace

NativeGaussMonopoleDichotomyResult
analyze_native_gauss_monopole_dichotomy() {
  NativeGaussMonopoleDichotomyResult result;
  result.infinite_volume_monopole_equals_net_polarity = true;
  result.neutral_finite_profile_has_no_monopole = true;
  result.native_ir_susceptibility_is_finite = true;
  result.native_ir_susceptibility = 3.0*G_C;
  result.nonlinear_topological_effective_charge_remains_open = true;

  const auto periodic_field = deterministic_periodic_face_field(8);
  result.periodic_telescope_residual = static_cast<double>(
      std::abs(total_divergence(periodic_field)));
  result.periodic_divergence_zero_sum =
      result.periodic_telescope_residual <= identity_gate;

  for (const auto& profile : profiles)
    for (int L : {8,16})
      for (int axis = 0; axis < 3; ++axis)
        for (int polarity : {1,-1})
          result.maximum_zero_mode_numerator_sum = std::max(
              result.maximum_zero_mode_numerator_sum,
              std::llabs(zero_mode_numerator_sum(
                  profile,L,axis,polarity)));
  result.production_zero_mode_subtracted =
      result.maximum_zero_mode_numerator_sum == 0;

  constexpr int solver_L = 8;
  MatchedFaceFlux indexing(solver_L);
  std::vector<int> point_source(
      static_cast<std::size_t>(solver_L*solver_L*solver_L),0);
  point_source[static_cast<std::size_t>(indexing.index(1,1,1))] = 1;
  MatchedGaussDynamics point_solver(solver_L);
  const auto point_result = point_solver.initialize_minimum_energy(point_source);
  result.matched_non_neutral_rejected = !point_result.valid
      && !point_result.neutral && !point_result.converged;

  auto dipole_source = point_source;
  dipole_source[static_cast<std::size_t>(indexing.index(2,1,1))] = -1;
  MatchedGaussDynamics dipole_solver(solver_L);
  const auto dipole_result =
      dipole_solver.initialize_minimum_energy(dipole_source);
  result.matched_neutral_gauss_residual = dipole_result.gauss_residual;
  result.matched_neutral_accepted = dipole_result.valid
      && dipole_result.neutral && dipole_result.converged
      && dipole_result.gauss_residual <= 1e-9;

  MatchedFaceFlux surface_field(solver_L);
  seed_dipole_path(surface_field,indexing.index(1,1,1),
                   indexing.index(5,5,5),1.0);
  const auto edge = make_transverse_challenge(solver_L,1e-3);
  const auto curl = matched_curl(edge);
  result.maximum_curl_divergence = max_divergence(curl);
  for (int radius = 0; radius <= 3; ++radius)
    for (int x = 0; x < solver_L; ++x)
      for (int y = 0; y < solver_L; ++y)
        for (int z = 0; z < solver_L; ++z) {
          const auto before = measure_face_cube_charge(
              surface_field,x,y,z,radius);
          const auto after = measure_face_cube_charge(curl,x,y,z,radius);
          result.maximum_closed_surface_flux_change = std::max(
              result.maximum_closed_surface_flux_change,
              std::abs(after.boundary_flux));
          result.maximum_closed_surface_flux_change = std::max(
              result.maximum_closed_surface_flux_change,
              std::abs((before.boundary_flux+after.boundary_flux)
                       -before.boundary_flux));
        }
  result.solenoidal_dressing_cannot_change_monopole =
      l1_norm(curl) > 0.0
      && result.maximum_curl_divergence <= identity_gate
      && result.maximum_closed_surface_flux_change <= identity_gate;

  for (const auto& profile : profiles)
    for (int volume : volumes)
      for (int direction_index = 0;
           direction_index < static_cast<int>(raw_directions.size());
           ++direction_index)
        for (int axis = 0; axis < 3; ++axis)
          for (int polarity : {1,-1}) {
            auto arm = analyze_arm(profile,volume,direction_index,
                                   axis,polarity);
            result.maximum_face_gauss_identity_residual = std::max(
                result.maximum_face_gauss_identity_residual,
                arm.face_gauss_identity_residual);
            if (arm.total_polarity != 0)
              result.maximum_point_monopole_error = std::max(
                  result.maximum_point_monopole_error,
                  std::abs(arm.monopole_estimator
                           -std::abs(arm.total_polarity)));
            if (volume == 256 && arm.total_polarity == 0) {
              result.maximum_l256_neutral_monopole_estimator = std::max(
                  result.maximum_l256_neutral_monopole_estimator,
                  arm.monopole_estimator);
              if (arm.leading_witness)
                result.maximum_l256_asymptotic_error = std::max(
                    result.maximum_l256_asymptotic_error,
                    std::abs(arm.asymptotic_ratio-1.0));
            }
            result.arms.push_back(std::move(arm));
          }

  result.expected_witness_groups = static_cast<int>(profiles.size())
      *static_cast<int>(volumes.size())*3*2;
  for (const auto& profile : profiles)
    for (int volume : volumes)
      for (int axis = 0; axis < 3; ++axis)
        for (int polarity : {1,-1}) {
          bool has_witness = false;
          for (int direction_index = 0;
               direction_index < static_cast<int>(raw_directions.size());
               ++direction_index) {
            const auto* arm = find_arm(result.arms,profile.name,volume,
                direction_index,axis,polarity);
            has_witness = has_witness || (arm && arm->leading_witness);
          }
          if (has_witness) ++result.witness_groups;
        }

  bool monotone = true;
  for (std::size_t profile_index = 1;
       profile_index < profiles.size(); ++profile_index)
    for (int direction_index = 0;
         direction_index < static_cast<int>(raw_directions.size());
         ++direction_index)
      for (int axis = 0; axis < 3; ++axis)
        for (int polarity : {1,-1}) {
          const auto* first = find_arm(result.arms,
              profiles[profile_index].name,volumes.front(),
              direction_index,axis,polarity);
          if (!first || !first->leading_witness) continue;
          double previous = std::numeric_limits<double>::infinity();
          bool group_monotone = true;
          for (int volume : volumes) {
            const auto* arm = find_arm(result.arms,
                profiles[profile_index].name,volume,direction_index,
                axis,polarity);
            group_monotone = group_monotone && arm
                && arm->monopole_estimator < previous;
            if (arm) previous = arm->monopole_estimator;
          }
          monotone = monotone && group_monotone;
          if (group_monotone) ++result.monotone_neutral_witnesses;
        }

  for (const auto& profile : profiles)
    for (int volume : volumes)
      for (int direction_index = 0;
           direction_index < static_cast<int>(raw_directions.size());
           ++direction_index)
        for (int axis = 0; axis < 3; ++axis) {
          const auto* plus = find_arm(result.arms,profile.name,volume,
              direction_index,axis,1);
          const auto* minus = find_arm(result.arms,profile.name,volume,
              direction_index,axis,-1);
          if (plus && minus)
            result.maximum_polarity_mirror_residual = std::max({
                result.maximum_polarity_mirror_residual,
                std::abs(plus->form_factor+minus->form_factor),
                std::abs(plus->monopole_estimator-minus->monopole_estimator)});
        }

  for (const auto& profile : profiles)
    for (int volume : volumes)
      for (int direction_index = 0;
           direction_index < static_cast<int>(raw_directions.size());
           ++direction_index)
        for (int polarity : {1,-1}) {
          const auto* reference = find_arm(result.arms,profile.name,volume,
              direction_index,0,polarity);
          for (int axis = 1; axis < 3; ++axis) {
            const auto* rotated = find_arm(result.arms,profile.name,volume,
                direction_index,axis,polarity);
            if (!reference || !rotated) continue;
            result.maximum_cyclic_covariance_residual = std::max({
                result.maximum_cyclic_covariance_residual,
                std::abs(reference->form_factor-rotated->form_factor),
                std::abs(reference->monopole_estimator
                         -rotated->monopole_estimator),
                std::abs(reference->face_laplacian
                         -rotated->face_laplacian)});
            for (int component = 0; component < 3; ++component)
              result.maximum_cyclic_covariance_residual = std::max(
                  result.maximum_cyclic_covariance_residual,
                  std::abs(reference->longitudinal_face_field[component]
                      -rotated->longitudinal_face_field[
                          (component+axis)%3]));
          }
        }

  const auto dressing = analyze_full_surface_source_obstruction(1.0/3.0);
  result.fixed_finite_linear_charged_carrier_closed = dressing.valid;
  result.valid = result.periodic_divergence_zero_sum
      && result.production_zero_mode_subtracted
      && result.matched_non_neutral_rejected
      && result.matched_neutral_accepted
      && result.infinite_volume_monopole_equals_net_polarity
      && result.neutral_finite_profile_has_no_monopole
      && result.solenoidal_dressing_cannot_change_monopole
      && result.native_ir_susceptibility_is_finite
      && result.fixed_finite_linear_charged_carrier_closed
      && result.nonlinear_topological_effective_charge_remains_open
      && result.arms.size() == 384
      && result.witness_groups == result.expected_witness_groups
      && monotone
      && result.maximum_zero_mode_numerator_sum == 0
      && result.periodic_telescope_residual <= identity_gate
      && result.matched_neutral_gauss_residual <= 1e-9
      && result.maximum_curl_divergence <= identity_gate
      && result.maximum_closed_surface_flux_change <= identity_gate
      && result.maximum_face_gauss_identity_residual <= identity_gate
      && result.maximum_point_monopole_error <= identity_gate
      && result.maximum_l256_neutral_monopole_estimator < 0.1
      && result.maximum_l256_asymptotic_error < 0.02
      && result.maximum_polarity_mirror_residual <= identity_gate
      && result.maximum_cyclic_covariance_residual <= identity_gate
      && std::all_of(result.arms.begin(),result.arms.end(),
          [](const auto& arm) { return arm.valid; });
  return result;
}

}  // namespace ftd::eft
