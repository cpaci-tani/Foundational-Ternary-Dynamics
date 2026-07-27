#include "ftd/eft/subcell_representation_quotient.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

struct AxisChart {
  int anchor = 0;
  double remainder = 0.0;
};

struct AxisCharts {
  std::array<AxisChart, 2> value{};
  int count = 0;
};

AxisCharts enumerate_axis(double position) {
  AxisCharts result;
  if (!std::isfinite(position)) return result;
  const double nearest = std::round(position);
  const double tolerance = 64.0
      * std::numeric_limits<double>::epsilon()
      * std::max(1.0, std::abs(position));
  if (std::abs(position - nearest) <= tolerance) {
    result.value[0] = {static_cast<int>(nearest), 0.0};
    result.count = 1;
    return result;
  }

  const double lower_value = std::floor(position);
  const int lower = static_cast<int>(lower_value);
  const double fraction = position - lower_value;
  result.value[0] = {lower, fraction};
  result.value[1] = {lower + 1, fraction - 1.0};
  result.count = 2;
  return result;
}

bool interior(double value) {
  return std::isfinite(value) && value > -1.0 && value < 1.0;
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

void translate_axis(int& anchor, double& remainder, double displacement) {
  remainder += displacement;
  while (remainder >= 1.0) {
    ++anchor;
    remainder -= 1.0;
  }
  while (remainder <= -1.0) {
    --anchor;
    remainder += 1.0;
  }
}

}  // namespace

std::vector<SubcellChart> enumerate_subcell_charts(
    const Vec3& effective_position) {
  std::vector<SubcellChart> result;
  const AxisCharts x = enumerate_axis(effective_position.x);
  const AxisCharts y = enumerate_axis(effective_position.y);
  const AxisCharts z = enumerate_axis(effective_position.z);
  if (x.count == 0 || y.count == 0 || z.count == 0) return result;
  result.reserve(static_cast<std::size_t>(x.count * y.count * z.count));
  for (int ix = 0; ix < x.count; ++ix) {
    for (int iy = 0; iy < y.count; ++iy) {
      for (int iz = 0; iz < z.count; ++iz) {
        SubcellChart chart;
        chart.anchor = {x.value[ix].anchor,
                        y.value[iy].anchor,
                        z.value[iz].anchor};
        chart.remainder = {x.value[ix].remainder,
                           y.value[iy].remainder,
                           z.value[iz].remainder};
        chart.valid = interior(chart.remainder.x)
            && interior(chart.remainder.y)
            && interior(chart.remainder.z);
        if (chart.valid) result.push_back(chart);
      }
    }
  }
  return result;
}

Vec3 subcell_chart_position(const SubcellChart& chart) {
  if (!chart.valid) return {NAN, NAN, NAN};
  return {static_cast<double>(chart.anchor.x) + chart.remainder.x,
          static_cast<double>(chart.anchor.y) + chart.remainder.y,
          static_cast<double>(chart.anchor.z) + chart.remainder.z};
}

bool equivalent_subcell_charts(const SubcellChart& lhs,
                               const SubcellChart& rhs,
                               double tolerance) {
  if (!lhs.valid || !rhs.valid || !std::isfinite(tolerance)
      || tolerance < 0.0) {
    return false;
  }
  return max_abs(subcell_chart_position(lhs)
                 - subcell_chart_position(rhs)) <= tolerance;
}

SubcellChart translate_subcell_chart(
    const SubcellChart& start,
    const Vec3& displacement) {
  SubcellChart result = start;
  if (!start.valid || !finite(displacement)) {
    result.valid = false;
    return result;
  }
  translate_axis(result.anchor.x, result.remainder.x, displacement.x);
  translate_axis(result.anchor.y, result.remainder.y, displacement.y);
  translate_axis(result.anchor.z, result.remainder.z, displacement.z);
  result.valid = interior(result.remainder.x)
      && interior(result.remainder.y)
      && interior(result.remainder.z);
  return result;
}

}  // namespace ftd::eft
