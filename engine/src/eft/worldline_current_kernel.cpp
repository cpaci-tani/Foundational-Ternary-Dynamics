#include "ftd/eft/worldline_current_kernel.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

int wrap(int value, int L) {
  value %= L;
  return value < 0 ? value + L : value;
}

int flat_index(int L, int x, int y, int z) {
  return (wrap(x, L) * L + wrap(y, L)) * L + wrap(z, L);
}

}  // namespace

FaceComplexKernelDimension face_complex_kernel_dimension(int L) {
  FaceComplexKernelDimension result;
  result.L = L;
  if (L < 2) return result;
  const std::uint64_t side = static_cast<std::uint64_t>(L);
  if (side > std::numeric_limits<std::uint64_t>::max() / side / side) {
    return result;
  }
  result.site_dimension = side * side * side;
  if (result.site_dimension
      > std::numeric_limits<std::uint64_t>::max() / 3) {
    return result;
  }
  result.face_current_dimension = 3 * result.site_dimension;
  result.divergence_rank = result.site_dimension - 1;
  result.divergence_kernel_dimension =
      result.face_current_dimension - result.divergence_rank;
  result.valid = result.divergence_kernel_dimension
      == 2 * result.site_dimension + 1;
  return result;
}

int TreeRoutedFaceCurrent::index(int x, int y, int z) const {
  return flat_index(L, x, y, z);
}

TreeRoutedFaceCurrent route_zero_sum_source_on_tree(
    int L,
    const std::vector<double>& source,
    double tolerance) {
  TreeRoutedFaceCurrent result;
  result.L = L;
  result.source = source;
  if (L < 2 || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  const std::size_t side = static_cast<std::size_t>(L);
  if (side > static_cast<std::size_t>(-1) / side / side) return result;
  const std::size_t volume = side * side * side;
  if (source.size() != volume) return result;
  for (double value : source) {
    if (!std::isfinite(value)) return result;
    result.source_sum += value;
  }
  if (std::abs(result.source_sum) > tolerance) return result;

  result.current_x.assign(volume, 0.0);
  result.current_y.assign(volume, 0.0);
  result.current_z.assign(volume, 0.0);
  std::vector<double> subtree = source;

  // Parent indices are strictly smaller under the x/y/z flat order. Routing
  // reverse order therefore accumulates each complete subtree before its edge
  // to the parent is assigned.
  for (std::size_t raw = volume; raw-- > 1;) {
    const int x = static_cast<int>(raw / (side * side));
    const int yz = static_cast<int>(raw % (side * side));
    const int y = yz / L;
    const int z = yz % L;
    int parent = 0;
    if (z > 0) {
      parent = flat_index(L, x, y, z - 1);
      result.current_z[static_cast<std::size_t>(parent)] = -subtree[raw];
    } else if (y > 0) {
      parent = flat_index(L, x, y - 1, 0);
      result.current_y[static_cast<std::size_t>(parent)] = -subtree[raw];
    } else {
      parent = flat_index(L, x - 1, 0, 0);
      result.current_x[static_cast<std::size_t>(parent)] = -subtree[raw];
    }
    subtree[static_cast<std::size_t>(parent)] += subtree[raw];
  }

  double residual = 0.0;
  const auto at = [&result](const std::vector<double>& field,
                            int x, int y, int z) {
    return field[static_cast<std::size_t>(result.index(x, y, z))];
  };
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const double divergence =
            at(result.current_x, x, y, z)
            - at(result.current_x, x - 1, y, z)
            + at(result.current_y, x, y, z)
            - at(result.current_y, x, y - 1, z)
            + at(result.current_z, x, y, z)
            - at(result.current_z, x, y, z - 1);
        const int index = result.index(x, y, z);
        residual = std::max(
            residual,
            std::abs(divergence
                     - source[static_cast<std::size_t>(index)]));
      }
    }
  }
  result.routing_residual = residual;
  result.valid = std::abs(subtree.front()) <= tolerance
      && residual <= tolerance;
  return result;
}

}  // namespace ftd::eft
