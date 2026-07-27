#include "ftd/eft/orientation_gauss_independence.h"

#include "ftd/eft/worldline_current_kernel.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

namespace ftd::eft {
namespace {

constexpr double pi = 3.141592653589793238462643383279502884;
constexpr double gate = 1e-12;

using Vertices = std::array<Vec3,6>;

const Vertices unit_octahedron{{
    {1.0,0.0,0.0},{-1.0,0.0,0.0},
    {0.0,1.0,0.0},{0.0,-1.0,0.0},
    {0.0,0.0,1.0},{0.0,0.0,-1.0}}};

Vec3 cyclic_rotate(const Vec3& value,int rotation) {
  if (rotation == 1) return {value.z,value.x,value.y};
  if (rotation == 2) return {value.y,value.z,value.x};
  return value;
}

std::array<int,3> face_indices(int sx,int sy,int sz) {
  const int ix = sx == 0 ? 0 : 1;
  const int iy = sy == 0 ? 2 : 3;
  const int iz = sz == 0 ? 4 : 5;
  return (sx+sy+sz)%2 == 1
      ? std::array<int,3>{{ix,iz,iy}}
      : std::array<int,3>{{ix,iy,iz}};
}

double solid_angle(const Vec3& a,const Vec3& b,const Vec3& c) {
  const double numerator = a.dot(Vec3::cross(b,c));
  const double denominator = 1.0+a.dot(b)+b.dot(c)+c.dot(a);
  return 2.0*std::atan2(numerator,denominator);
}

double orientation_degree(const Vertices& field,double& minimum_magnitude) {
  Vertices normalized{};
  minimum_magnitude = std::numeric_limits<double>::infinity();
  for (std::size_t index = 0; index < field.size(); ++index) {
    const double magnitude = field[index].mag();
    minimum_magnitude = std::min(minimum_magnitude,magnitude);
    if (!(magnitude > 0.0))
      return std::numeric_limits<double>::quiet_NaN();
    normalized[index] = field[index]*(1.0/magnitude);
  }
  double total = 0.0;
  for (int sx = 0; sx < 2; ++sx)
    for (int sy = 0; sy < 2; ++sy)
      for (int sz = 0; sz < 2; ++sz) {
        const auto face = face_indices(sx,sy,sz);
        total += solid_angle(normalized[face[0]],normalized[face[1]],
                             normalized[face[2]]);
      }
  return total/(4.0*pi);
}

double geometric_gauss_flux(const Vertices& positions,
                            const Vertices& field) {
  double total = 0.0;
  for (int sx = 0; sx < 2; ++sx)
    for (int sy = 0; sy < 2; ++sy)
      for (int sz = 0; sz < 2; ++sz) {
        const auto face = face_indices(sx,sy,sz);
        const Vec3& a = positions[face[0]];
        const Vec3& b = positions[face[1]];
        const Vec3& c = positions[face[2]];
        const Vec3 oriented_area = Vec3::cross(b-a,c-a)*0.5;
        const Vec3 mean_field =
            (field[face[0]]+field[face[1]]+field[face[2]])*(1.0/3.0);
        total += oriented_area.dot(mean_field);
      }
  return total;
}

OrientationGaussArm analyze_arm(const std::string& family,double amplitude,
                                int polarity,int rotation) {
  OrientationGaussArm result;
  result.family = family;
  result.amplitude = amplitude;
  result.polarity = polarity;
  result.cyclic_rotation = rotation;
  Vertices positions{};
  Vertices field{};
  const Vec3 offset = cyclic_rotate({0.0,0.0,2.0},rotation);
  for (std::size_t index = 0; index < unit_octahedron.size(); ++index) {
    positions[index] = cyclic_rotate(unit_octahedron[index],rotation);
    const Vec3 raw = family == "hedgehog"
        ? positions[index] : positions[index]+offset;
    field[index] = raw*(polarity*amplitude);
  }
  result.orientation_degree =
      orientation_degree(field,result.minimum_field_magnitude);
  result.gauss_flux = geometric_gauss_flux(positions,field);
  result.expected_degree = family == "hedgehog" ? polarity : 0.0;
  result.expected_flux = 4.0*polarity*amplitude;
  result.degree_residual =
      std::abs(result.orientation_degree-result.expected_degree);
  result.flux_residual = std::abs(result.gauss_flux-result.expected_flux);
  result.valid = std::isfinite(result.orientation_degree)
      && result.minimum_field_magnitude > 0.0
      && result.degree_residual <= gate && result.flux_residual <= gate;
  return result;
}

const OrientationGaussArm* find_arm(
    const std::vector<OrientationGaussArm>& arms,const std::string& family,
    double amplitude,int polarity,int rotation) {
  for (const auto& arm : arms)
    if (arm.family == family && arm.amplitude == amplitude
        && arm.polarity == polarity && arm.cyclic_rotation == rotation)
      return &arm;
  return nullptr;
}

std::vector<double> deterministic_zero_sum_source(int L) {
  const int volume = L*L*L;
  std::vector<double> source(static_cast<std::size_t>(volume),0.0);
  double sum = 0.0;
  for (int index = 1; index < volume; ++index) {
    source[static_cast<std::size_t>(index)] = (index%7)-3;
    sum += source[static_cast<std::size_t>(index)];
  }
  source[0] = -sum;
  return source;
}

}  // namespace

OrientationGaussIndependenceResult
analyze_orientation_gauss_independence() {
  OrientationGaussIndependenceResult result;
  const std::array<double,5> amplitudes{{1.0,0.5,0.25,0.125,0.0625}};
  for (const std::string family : {"hedgehog","translated_image"})
    for (double amplitude : amplitudes)
      for (int polarity : {1,-1})
        for (int rotation = 0; rotation < 3; ++rotation) {
          auto arm = analyze_arm(family,amplitude,polarity,rotation);
          result.maximum_degree_residual = std::max(
              result.maximum_degree_residual,arm.degree_residual);
          result.maximum_flux_residual = std::max(
              result.maximum_flux_residual,arm.flux_residual);
          result.arms.push_back(std::move(arm));
        }

  bool pair_identity = true;
  bool scale_identity = true;
  bool mirror_identity = true;
  bool rotation_identity = true;
  for (double amplitude : amplitudes)
    for (int polarity : {1,-1})
      for (int rotation = 0; rotation < 3; ++rotation) {
        const auto* hedgehog = find_arm(
            result.arms,"hedgehog",amplitude,polarity,rotation);
        const auto* translated = find_arm(
            result.arms,"translated_image",amplitude,polarity,rotation);
        if (!hedgehog || !translated) {
          pair_identity = false;
          continue;
        }
        const double residual =
            std::abs(hedgehog->gauss_flux-translated->gauss_flux);
        result.maximum_equal_flux_residual = std::max(
            result.maximum_equal_flux_residual,residual);
        pair_identity = pair_identity && residual <= gate
            && std::abs(hedgehog->orientation_degree
                        -translated->orientation_degree) > 0.5;
      }

  for (const std::string family : {"hedgehog","translated_image"})
    for (int polarity : {1,-1})
      for (int rotation = 0; rotation < 3; ++rotation) {
        const auto* reference = find_arm(
            result.arms,family,amplitudes.front(),polarity,rotation);
        if (!reference) { scale_identity = false; continue; }
        for (double amplitude : amplitudes) {
          const auto* arm = find_arm(
              result.arms,family,amplitude,polarity,rotation);
          if (!arm) { scale_identity = false; continue; }
          const double residual = std::max(
              std::abs(arm->orientation_degree-reference->orientation_degree),
              std::abs(arm->gauss_flux
                       -reference->gauss_flux*amplitude));
          result.maximum_scale_linearity_residual = std::max(
              result.maximum_scale_linearity_residual,residual);
          scale_identity = scale_identity && residual <= gate;
        }
      }

  for (const std::string family : {"hedgehog","translated_image"})
    for (double amplitude : amplitudes)
      for (int rotation = 0; rotation < 3; ++rotation) {
        const auto* plus = find_arm(
            result.arms,family,amplitude,1,rotation);
        const auto* minus = find_arm(
            result.arms,family,amplitude,-1,rotation);
        if (!plus || !minus) { mirror_identity = false; continue; }
        const double residual = std::max(
            std::abs(plus->orientation_degree+minus->orientation_degree),
            std::abs(plus->gauss_flux+minus->gauss_flux));
        result.maximum_polarity_mirror_residual = std::max(
            result.maximum_polarity_mirror_residual,residual);
        mirror_identity = mirror_identity && residual <= gate;
      }

  for (const std::string family : {"hedgehog","translated_image"})
    for (double amplitude : amplitudes)
      for (int polarity : {1,-1}) {
        const auto* reference = find_arm(
            result.arms,family,amplitude,polarity,0);
        if (!reference) { rotation_identity = false; continue; }
        for (int rotation = 1; rotation < 3; ++rotation) {
          const auto* arm = find_arm(
              result.arms,family,amplitude,polarity,rotation);
          if (!arm) { rotation_identity = false; continue; }
          const double residual = std::max(
              std::abs(arm->orientation_degree-reference->orientation_degree),
              std::abs(arm->gauss_flux-reference->gauss_flux));
          result.maximum_cyclic_covariance_residual = std::max(
              result.maximum_cyclic_covariance_residual,residual);
          rotation_identity = rotation_identity && residual <= gate;
        }
      }

  result.degree_does_not_determine_flux = true;
  for (std::size_t index = 1; index < amplitudes.size(); ++index) {
    const auto* first = find_arm(result.arms,"hedgehog",amplitudes[0],1,0);
    const auto* other = find_arm(result.arms,"hedgehog",amplitudes[index],1,0);
    result.degree_does_not_determine_flux =
        result.degree_does_not_determine_flux && first && other
        && std::abs(first->orientation_degree-other->orientation_degree)<=gate
        && std::abs(first->gauss_flux-other->gauss_flux)>gate;
  }
  result.flux_does_not_determine_degree = pair_identity;
  result.amplitude_rescaling_separates_observables = scale_identity;
  result.polarity_mirror_exact = mirror_identity;
  result.cubic_covariance_exact = rotation_identity;

  bool rank_valid = true;
  for (int L : {3,5}) {
    const auto dimensions = face_complex_kernel_dimension(L);
    const auto routed = route_zero_sum_source_on_tree(
        L,deterministic_zero_sum_source(L));
    rank_valid = rank_valid && dimensions.valid
        && dimensions.divergence_rank == dimensions.site_dimension-1
        && routed.valid;
    result.maximum_tree_routing_residual = std::max(
        result.maximum_tree_routing_residual,routed.routing_residual);
    if (dimensions.valid && routed.valid) ++result.rank_witnesses;
  }
  result.periodic_divergence_image_is_zero_sum = rank_valid
      && result.rank_witnesses == result.expected_rank_witnesses
      && result.maximum_tree_routing_residual <= gate;

  result.topology_alone_charge_magnitude_closed =
      result.degree_does_not_determine_flux
      && result.flux_does_not_determine_degree;
  result.topological_core_with_action_remains_open = true;
  result.valid = result.arms.size() == 60
      && std::all_of(result.arms.begin(),result.arms.end(),
                     [](const auto& arm) { return arm.valid; })
      && result.topology_alone_charge_magnitude_closed
      && result.amplitude_rescaling_separates_observables
      && result.polarity_mirror_exact && result.cubic_covariance_exact
      && result.periodic_divergence_image_is_zero_sum
      && result.topological_core_with_action_remains_open;
  return result;
}

}  // namespace ftd::eft
