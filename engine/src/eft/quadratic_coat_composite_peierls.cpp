#include "ftd/eft/quadratic_coat_composite_peierls.h"

#include "ftd/eft/coupled_matched_face_transaction.h"
#include "ftd/eft/matched_face_energy_transaction.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <numeric>
#include <set>

namespace ftd::eft {
namespace {

constexpr double pi = 3.1415926535897932384626433832795;

int wrap(int value, int L) {
  const int remainder = value%L;
  return remainder < 0 ? remainder+L : remainder;
}

std::size_t index(int L, int x, int y, int z) {
  return (static_cast<std::size_t>(wrap(x, L))*L+wrap(y, L))*L+wrap(z, L);
}

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

Vec3 translated_position(const Coord& origin, const Coord& offset,
                         int axis, double fraction) {
  Vec3 result{static_cast<double>(origin.x+offset.x),
              static_cast<double>(origin.y+offset.y),
              static_cast<double>(origin.z+offset.z)};
  if (axis == 0) result.x += fraction;
  else if (axis == 1) result.y += fraction;
  else result.z += fraction;
  return result;
}

double max_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double max_abs(const std::vector<double>& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i])*rhs[i];
  return result;
}

void negative_laplacian(int L, const std::vector<double>& input,
                        std::vector<double>& output) {
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto i = index(L, x, y, z);
        output[i] = 6.0*input[i]
            -input[index(L, x+1, y, z)]-input[index(L, x-1, y, z)]
            -input[index(L, x, y+1, z)]-input[index(L, x, y-1, z)]
            -input[index(L, x, y, z+1)]-input[index(L, x, y, z-1)];
      }
}

struct Deposit {
  bool valid = false;
  std::vector<double> density;
  double neutrality_residual = INFINITY;
  double partition_residual = INFINITY;
  double first_moment_residual = INFINITY;
};

Deposit deposit_composite(
    int L, const std::vector<QuadraticCompositeConstituent>& constituents,
    const Coord& origin, int axis, double fraction) {
  Deposit result;
  result.density.assign(static_cast<std::size_t>(L)*L*L, 0.0);
  result.partition_residual = 0.0;
  result.first_moment_residual = 0.0;
  for (const auto& constituent : constituents) {
    const auto coat = make_quadratic_polarity_coat(
        translated_position(origin, constituent.offset, axis, fraction),
        constituent.polarity);
    if (!coat.valid) return result;
    result.partition_residual = std::max(
        result.partition_residual, std::abs(coat.partition_residual));
    result.first_moment_residual = std::max(
        result.first_moment_residual,
        max_component(coat.first_moment_residual));
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& entry = coat.weights[item];
      result.density[index(L, entry.site.x, entry.site.y, entry.site.z)]
          += entry.weight;
    }
  }
  result.neutrality_residual = std::abs(static_cast<double>(
      std::accumulate(result.density.begin(), result.density.end(), 0.0L)));
  result.valid = result.partition_residual <= 1e-12
      && result.first_moment_residual <= 1e-12
      && result.neutrality_residual <= 1e-12;
  return result;
}

struct LongitudinalField {
  bool valid = false;
  int iterations = 0;
  double poisson_residual = INFINITY;
  double gauss_residual = INFINITY;
  double curl_residual = INFINITY;
  double energy = INFINITY;
  std::vector<double> density;
  MatchedFaceFlux electric;
  explicit LongitudinalField(int L = 0) : electric(L) {}
};

LongitudinalField solve_longitudinal(
    int L, const Deposit& deposit, double beta,
    double tolerance, int max_iterations) {
  LongitudinalField result(L);
  if (!deposit.valid) return result;
  result.density = deposit.density;
  const std::size_t count = result.density.size();
  std::vector<double> source = result.density;
  const long double mean = std::accumulate(
      source.begin(), source.end(), 0.0L)/count;
  for (double& value : source) value -= static_cast<double>(mean);
  std::vector<double> potential(count, 0.0);
  std::vector<double> residual = source;
  std::vector<double> direction = residual;
  std::vector<double> image(count, 0.0);
  long double rr = dot(residual, residual);
  result.poisson_residual = max_abs(residual);
  bool converged = result.poisson_residual <= tolerance;
  for (int iteration = 1; !converged && iteration <= max_iterations;
       ++iteration) {
    negative_laplacian(L, direction, image);
    const long double denominator = dot(direction, image);
    if (!(denominator > 0.0L)
        || !std::isfinite(static_cast<double>(denominator))) break;
    const long double alpha = rr/denominator;
    for (std::size_t i = 0; i < count; ++i) {
      potential[i] += static_cast<double>(alpha*direction[i]);
      residual[i] -= static_cast<double>(alpha*image[i]);
    }
    result.iterations = iteration;
    result.poisson_residual = max_abs(residual);
    converged = result.poisson_residual <= tolerance;
    if (converged) break;
    const long double next = dot(residual, residual);
    if (!(next >= 0.0L) || !std::isfinite(static_cast<double>(next))) break;
    const long double ratio = next/rr;
    for (std::size_t i = 0; i < count; ++i)
      direction[i] = residual[i]+static_cast<double>(ratio*direction[i]);
    rr = next;
  }
  if (!converged) return result;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = result.electric.index(x, y, z);
        result.electric.x[i] = potential[i]-potential[index(L, x+1, y, z)];
        result.electric.y[i] = potential[i]-potential[index(L, x, y+1, z)];
        result.electric.z[i] = potential[i]-potential[index(L, x, y, z+1)];
      }
  result.gauss_residual = max_fractional_gauss_residual(
      result.electric, result.density);
  result.curl_residual = max_curl_adjoint(result.electric);
  result.energy = beta*quadratic_energy(result.electric);
  result.valid = result.poisson_residual <= tolerance
      && std::isfinite(result.energy)
      && result.gauss_residual <= 1e-12
      && result.curl_residual <= 1e-12;
  return result;
}

struct Spectrum {
  double energy_zero = 0.0;
  double coefficient = 0.0;
  std::size_t positive_terms = 0;
};

Spectrum evaluate_spectrum(
    int L, const std::vector<QuadraticCompositeConstituent>& constituents,
    int axis, double beta) {
  Spectrum result;
  long double energy_sum = 0.0L;
  long double coefficient_sum = 0.0L;
  for (int mx = 0; mx < L; ++mx) {
    const double kx = 2.0*pi*mx/L;
    const double cx = std::cos(kx);
    const double bx = (3.0+cx)/4.0;
    for (int my = 0; my < L; ++my) {
      const double ky = 2.0*pi*my/L;
      const double cy = std::cos(ky);
      const double by = (3.0+cy)/4.0;
      for (int mz = 0; mz < L; ++mz) {
        if (mx == 0 && my == 0 && mz == 0) continue;
        const double kz = 2.0*pi*mz/L;
        const double cz = std::cos(kz);
        const double bz = (3.0+cz)/4.0;
        const double lambda = 2.0*(3.0-cx-cy-cz);
        std::complex<long double> structure{0.0L, 0.0L};
        for (const auto& constituent : constituents) {
          const long double phase = -(static_cast<long double>(kx)
              *constituent.offset.x+static_cast<long double>(ky)
              *constituent.offset.y+static_cast<long double>(kz)
              *constituent.offset.z);
          structure += static_cast<long double>(constituent.polarity)
              *std::complex<long double>{std::cos(phase), std::sin(phase)};
        }
        const long double structure2 = std::norm(structure);
        const std::array<double, 3> cosine{{cx, cy, cz}};
        const std::array<double, 3> centered{{bx, by, bz}};
        const long double centered_product = static_cast<long double>(
            bx*bx*by*by*bz*bz);
        energy_sum += structure2*centered_product/lambda;
        long double transverse = 1.0L;
        for (int direction = 0; direction < 3; ++direction)
          if (direction != axis)
            transverse *= static_cast<long double>(
                centered[direction]*centered[direction]);
        const long double axis_factor =
            (1.0L-cosine[axis])*(1.0L-cosine[axis]);
        const long double term = structure2*axis_factor*transverse/lambda;
        coefficient_sum += term;
        if (term > 0.0L) ++result.positive_terms;
      }
    }
  }
  const long double scale = static_cast<long double>(beta)
      /(2.0L*L*L*L);
  result.energy_zero = static_cast<double>(scale*energy_sum);
  result.coefficient = static_cast<double>(scale*coefficient_sum);
  return result;
}

bool exact_axis_invariant(
    int L, const std::vector<QuadraticCompositeConstituent>& constituents,
    int axis) {
  std::vector<int> source(static_cast<std::size_t>(L)*L*L, 0);
  for (const auto& constituent : constituents)
    source[index(L, constituent.offset.x, constituent.offset.y,
                 constituent.offset.z)] += constituent.polarity;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        int px = x;
        int py = y;
        int pz = z;
        if (axis == 0) --px;
        else if (axis == 1) --py;
        else --pz;
        if (source[index(L, x, y, z)] != source[index(L, px, py, pz)])
          return false;
      }
  return true;
}

double quartic_shape(double fraction) {
  const double square = fraction*fraction;
  return square*square-0.5*square;
}

MatchedFaceFlux midpoint(const MatchedFaceFlux& lhs,
                         const MatchedFaceFlux& rhs) {
  MatchedFaceFlux result(lhs.L);
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    result.x[i] = 0.5*(lhs.x[i]+rhs.x[i]);
    result.y[i] = 0.5*(lhs.y[i]+rhs.y[i]);
    result.z[i] = 0.5*(lhs.z[i]+rhs.z[i]);
  }
  return result;
}

void add_segment(MatchedFaceFlux& current,
                 const QuadraticCoatFaceCurrent& segment) {
  for (std::size_t i = 0; i < current.x.size(); ++i) {
    current.x[i] += segment.current_x[i];
    current.y[i] += segment.current_y[i];
    current.z[i] += segment.current_z[i];
  }
}

double face_divergence(const MatchedFaceFlux& field,
                       int x, int y, int z) {
  const auto at = [&field](const std::vector<double>& values,
                           int sx, int sy, int sz) {
    return values[index(field.L, sx, sy, sz)];
  };
  return at(field.x, x, y, z)-at(field.x, x-1, y, z)
      +at(field.y, x, y, z)-at(field.y, x, y-1, z)
      +at(field.z, x, y, z)-at(field.z, x, y, z-1);
}

double density_difference(const std::vector<double>& lhs,
                          const std::vector<double>& rhs) {
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

}  // namespace

QuadraticCompositePeierlsResult
evaluate_quadratic_composite_peierls(
    int L,
    const std::vector<QuadraticCompositeConstituent>& constituents,
    const Coord& origin,
    int axis,
    double beta,
    const std::vector<double>& fractions,
    const std::vector<std::pair<double, double>>& work_intervals,
    double poisson_tolerance,
    int poisson_max_iterations) {
  QuadraticCompositePeierlsResult result;
  result.L = L;
  result.axis = axis;
  result.origin = origin;
  result.beta = beta;
  result.constituents = constituents;
  if (L < 5 || axis < 0 || axis > 2 || constituents.empty()
      || !std::isfinite(beta) || !(beta > 0.0)
      || fractions.empty() || !(poisson_tolerance > 0.0)
      || poisson_max_iterations <= 0) return result;

  int total_polarity = 0;
  std::set<std::size_t> primitive_sites;
  bool constituents_valid = true;
  for (const auto& constituent : constituents) {
    total_polarity += constituent.polarity;
    constituents_valid = constituents_valid
        && (constituent.polarity == -1 || constituent.polarity == 1);
    primitive_sites.insert(index(L, origin.x+constituent.offset.x,
        origin.y+constituent.offset.y, origin.z+constituent.offset.z));
  }
  result.neutral = total_polarity == 0;
  result.distinct_primitive_sites =
      primitive_sites.size() == constituents.size();
  if (!constituents_valid || !result.neutral
      || !result.distinct_primitive_sites) return result;

  result.axis_invariant = exact_axis_invariant(L, constituents, axis);
  const Spectrum spectrum = evaluate_spectrum(L, constituents, axis, beta);
  result.spectral_energy_zero = spectrum.energy_zero;
  result.peierls_coefficient = spectrum.coefficient;
  result.barrier = spectrum.coefficient/16.0;
  result.positive_spectral_terms = spectrum.positive_terms;

  std::vector<double> required_fractions = fractions;
  for (const auto& interval : work_intervals) {
    required_fractions.push_back(interval.first);
    required_fractions.push_back(interval.second);
  }
  std::sort(required_fractions.begin(), required_fractions.end());
  required_fractions.erase(std::unique(required_fractions.begin(),
      required_fractions.end()), required_fractions.end());
  struct CachedPoint {
    double fraction = 0.0;
    Deposit deposit;
    LongitudinalField field;
    CachedPoint(double value, int size)
        : fraction(value), field(size) {}
  };
  std::vector<CachedPoint> cache;
  for (double fraction : required_fractions) {
    if (!std::isfinite(fraction) || fraction < -0.5 || fraction > 0.5)
      return result;
    CachedPoint point(fraction, L);
    point.deposit = deposit_composite(
        L, constituents, origin, axis, fraction);
    point.field = solve_longitudinal(
        L, point.deposit, beta, poisson_tolerance,
        poisson_max_iterations);
    result.maximum_poisson_iterations = std::max(
        result.maximum_poisson_iterations, point.field.iterations);
    if (!point.deposit.valid || !point.field.valid) return result;
    cache.push_back(std::move(point));
  }
  const auto point_at = [&cache](double fraction) -> const CachedPoint* {
    const auto found = std::find_if(cache.begin(), cache.end(),
        [fraction](const CachedPoint& point) {
          return point.fraction == fraction;
        });
    return found == cache.end() ? nullptr : &*found;
  };

  for (double fraction : fractions) {
    const CachedPoint* point = point_at(fraction);
    if (point == nullptr) return result;
    QuadraticCompositePeierlsSample sample;
    sample.fraction = fraction;
    sample.spectral_energy = result.spectral_energy_zero
        +result.peierls_coefficient*quartic_shape(fraction);
    sample.predicted_energy = sample.spectral_energy;
    sample.poisson_energy = point->field.energy;
    sample.poisson_residual = point->field.poisson_residual;
    sample.gauss_residual = point->field.gauss_residual;
    sample.curl_residual = point->field.curl_residual;
    sample.neutrality_residual = point->deposit.neutrality_residual;
    sample.partition_residual = point->deposit.partition_residual;
    sample.first_moment_residual = point->deposit.first_moment_residual;
    sample.spectral_poisson_residual = std::abs(
        sample.spectral_energy-sample.poisson_energy);
    sample.quartic_residual = sample.spectral_poisson_residual;
    result.maximum_identity_residual = std::max({
        result.maximum_identity_residual, sample.poisson_residual,
        sample.gauss_residual, sample.curl_residual,
        sample.neutrality_residual, sample.partition_residual,
        sample.first_moment_residual, sample.spectral_poisson_residual,
        sample.quartic_residual});
    result.samples.push_back(sample);
  }

  for (const auto& interval : work_intervals) {
    const CachedPoint* before = point_at(interval.first);
    const CachedPoint* after = point_at(interval.second);
    if (before == nullptr || after == nullptr
        || interval.first == interval.second) return result;
    QuadraticCompositePeierlsWork work;
    work.fraction_before = interval.first;
    work.fraction_after = interval.second;
    const double displacement = interval.second-interval.first;
    const MatchedFaceFlux electric_midpoint = midpoint(
        before->field.electric, after->field.electric);
    MatchedFaceFlux aggregate_current(L);
    std::vector<double> aggregate_before(
        static_cast<std::size_t>(L)*L*L, 0.0);
    std::vector<double> aggregate_after = aggregate_before;
    Vec3 net_force{};
    MatchedEdgeField zero_magnetic(L);
    for (const auto& constituent : constituents) {
      const Vec3 start = translated_position(
          origin, constituent.offset, axis, interval.first);
      const Vec3 end = translated_position(
          origin, constituent.offset, axis, interval.second);
      const auto segment = make_quadratic_coat_face_current(
          L, start, end, constituent.polarity);
      if (!segment.valid) return result;
      add_segment(aggregate_current, segment);
      for (std::size_t i = 0; i < aggregate_before.size(); ++i) {
        aggregate_before[i] += segment.rho_before[i];
        aggregate_after[i] += segment.rho_after[i];
      }
      Vec3 velocity{};
      if (axis == 0) velocity.x = displacement;
      else if (axis == 1) velocity.y = displacement;
      else velocity.z = displacement;
      const auto gather = evaluate_quadratic_coat_orbit_gather(
          segment, electric_midpoint, zero_magnetic, velocity, 1.0, beta);
      if (!gather.valid) return result;
      net_force += gather.electric_force*beta;
      work.gather_adjoint_residual = std::max(
          work.gather_adjoint_residual,
          beta*gather.electric_adjoint_residual);
    }
    for (int x = 0; x < L; ++x)
      for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
          const auto i = index(L, x, y, z);
          work.continuity_residual = std::max(
              work.continuity_residual,
              std::abs(aggregate_after[i]-aggregate_before[i]
                       +face_divergence(aggregate_current, x, y, z)));
        }
    work.endpoint_density_residual = std::max(
        density_difference(aggregate_before, before->field.density),
        density_difference(aggregate_after, after->field.density));
    work.current_work = beta*static_cast<double>(
        matched_face_dot(electric_midpoint, aggregate_current));
    work.field_energy_change = after->field.energy-before->field.energy;
    work.spectral_energy_change = result.peierls_coefficient
        *(quartic_shape(interval.second)-quartic_shape(interval.first));
    work.net_force_component = component(net_force, axis);
    work.field_work_residual = std::abs(
        work.field_energy_change+work.current_work);
    work.spectral_work_residual = std::abs(
        work.spectral_energy_change+work.current_work);
    work.gather_adjoint_residual = std::max(
        work.gather_adjoint_residual,
        std::abs(displacement*work.net_force_component-work.current_work));
    result.maximum_identity_residual = std::max({
        result.maximum_identity_residual, work.continuity_residual,
        work.endpoint_density_residual, work.gather_adjoint_residual,
        work.field_work_residual, work.spectral_work_residual});
    result.work_samples.push_back(work);
  }

  result.valid = std::isfinite(result.spectral_energy_zero)
      && std::isfinite(result.peierls_coefficient)
      && std::isfinite(result.barrier)
      && result.samples.size() == fractions.size()
      && result.work_samples.size() == work_intervals.size();
  return result;
}

}  // namespace ftd::eft
