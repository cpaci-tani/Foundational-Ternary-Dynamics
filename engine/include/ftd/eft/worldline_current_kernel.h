#pragma once
/**
 * @file worldline_current_kernel.h
 * @brief Exact divergence-kernel dimension and constructive spanning-tree
 *        routing for periodic oriented face currents (FTD-0502).
 */

#include <cstdint>
#include <vector>

namespace ftd::eft {

struct FaceComplexKernelDimension {
  int L = 0;
  std::uint64_t site_dimension = 0;
  std::uint64_t face_current_dimension = 0;
  std::uint64_t divergence_rank = 0;
  std::uint64_t divergence_kernel_dimension = 0;
  bool valid = false;
};

/// Analytic dimensions for the connected periodic cubic cell complex:
/// rank(div)=V-1 and nullity(div)=2V+1.
FaceComplexKernelDimension face_complex_kernel_dimension(int L);

struct TreeRoutedFaceCurrent {
  int L = 0;
  std::vector<double> source;
  std::vector<double> current_x;
  std::vector<double> current_y;
  std::vector<double> current_z;
  double source_sum = 0.0;
  double routing_residual = 0.0;
  bool valid = false;

  int index(int x, int y, int z) const;
};

/// Construct J with div(J)=source for any zero-sum site source. The fixed tree
/// uses +z edges inside rows, +y edges inside x slabs, then +x edges to root.
TreeRoutedFaceCurrent route_zero_sum_source_on_tree(
    int L,
    const std::vector<double>& source,
    double tolerance = 1e-12);

}  // namespace ftd::eft
