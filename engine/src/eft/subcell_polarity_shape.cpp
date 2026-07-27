#include "ftd/eft/subcell_polarity_shape.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

struct AxisWeights {
  std::array<int, 2> site{};
  std::array<double, 2> weight{};
  std::size_t count = 0;
};

AxisWeights make_axis_weights(int anchor, double remainder) {
  AxisWeights result;
  const double fraction = std::abs(remainder);
  if (fraction == 0.0) {
    result.site[0] = anchor;
    result.weight[0] = 1.0;
    result.count = 1;
    return result;
  }

  const int direction = remainder < 0.0 ? -1 : 1;
  if (fraction == 1.0) {
    result.site[0] = anchor + direction;
    result.weight[0] = 1.0;
    result.count = 1;
    return result;
  }

  result.site[0] = anchor;
  result.site[1] = anchor + direction;
  result.weight[0] = 1.0 - fraction;
  result.weight[1] = fraction;
  result.count = 2;
  return result;
}

bool valid_remainder(double value) {
  return std::isfinite(value) && value >= -1.0 && value <= 1.0;
}

}  // namespace

SubcellPolarityShape make_subcell_polarity_shape(
    Coord anchor, const Vec3& remainder, int polarity) {
  SubcellPolarityShape result;
  result.anchor = anchor;
  result.remainder = remainder;
  result.polarity = polarity;
  result.effective_position = {
      static_cast<double>(anchor.x) + remainder.x,
      static_cast<double>(anchor.y) + remainder.y,
      static_cast<double>(anchor.z) + remainder.z};

  if ((polarity != -1 && polarity != 1)
      || !valid_remainder(remainder.x)
      || !valid_remainder(remainder.y)
      || !valid_remainder(remainder.z)) {
    return result;
  }

  const AxisWeights wx = make_axis_weights(anchor.x, remainder.x);
  const AxisWeights wy = make_axis_weights(anchor.y, remainder.y);
  const AxisWeights wz = make_axis_weights(anchor.z, remainder.z);

  long double partition = 0.0L;
  long double moment_x = 0.0L;
  long double moment_y = 0.0L;
  long double moment_z = 0.0L;
  for (std::size_t ix = 0; ix < wx.count; ++ix) {
    for (std::size_t iy = 0; iy < wy.count; ++iy) {
      for (std::size_t iz = 0; iz < wz.count; ++iz) {
        const double weight = static_cast<double>(polarity)
            * wx.weight[ix] * wy.weight[iy] * wz.weight[iz];
        result.weights[result.weight_count++] = {
            {wx.site[ix], wy.site[iy], wz.site[iz]}, weight};
        partition += static_cast<long double>(weight);
        moment_x += static_cast<long double>(weight) * wx.site[ix];
        moment_y += static_cast<long double>(weight) * wy.site[iy];
        moment_z += static_cast<long double>(weight) * wz.site[iz];
      }
    }
  }

  result.partition_residual = static_cast<double>(
      partition - static_cast<long double>(polarity));
  result.first_moment_residual = {
      static_cast<double>(moment_x - static_cast<long double>(polarity)
          * result.effective_position.x),
      static_cast<double>(moment_y - static_cast<long double>(polarity)
          * result.effective_position.y),
      static_cast<double>(moment_z - static_cast<long double>(polarity)
          * result.effective_position.z)};
  result.valid = result.weight_count >= 1 && result.weight_count <= 8
      && std::isfinite(result.partition_residual)
      && std::isfinite(result.first_moment_residual.x)
      && std::isfinite(result.first_moment_residual.y)
      && std::isfinite(result.first_moment_residual.z);
  return result;
}

double max_first_moment_residual(const SubcellPolarityShape& shape) {
  return std::max({std::abs(shape.first_moment_residual.x),
                   std::abs(shape.first_moment_residual.y),
                   std::abs(shape.first_moment_residual.z)});
}

}  // namespace ftd::eft
