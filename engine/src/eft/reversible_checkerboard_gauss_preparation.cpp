#include "ftd/eft/reversible_checkerboard_gauss_preparation.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace ftd::eft {
namespace {

bool finite_values(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(), [](double value) {
    return std::isfinite(value);
  });
}

bool finite_face(const MatchedFaceFlux& field) {
  return finite_values(field.x)
      && finite_values(field.y)
      && finite_values(field.z);
}

int wrap(int value, int L) {
  const int result = value % L;
  return result < 0 ? result + L : result;
}

void coordinates(int L, int index, int& x, int& y, int& z) {
  z = index % L;
  const int xy = index / L;
  y = xy % L;
  x = xy / L;
}

int parity_of(int x, int y, int z) {
  return (x + y + z) & 1;
}

double active_residual(
    const MatchedFaceFlux& flux,
    const std::vector<double>& charge,
    int x,
    int y,
    int z) {
  const int index = flux.index(x, y, z);
  return divergence_at(flux, x, y, z)
      - charge[static_cast<std::size_t>(index)];
}

void add_incidence_row(
    MatchedFaceFlux& flux,
    int x,
    int y,
    int z,
    double scale) {
  const int center = flux.index(x, y, z);
  const int xm = flux.index(x - 1, y, z);
  const int ym = flux.index(x, y - 1, z);
  const int zm = flux.index(x, y, z - 1);
  flux.x[static_cast<std::size_t>(center)] += scale;
  flux.x[static_cast<std::size_t>(xm)] -= scale;
  flux.y[static_cast<std::size_t>(center)] += scale;
  flux.y[static_cast<std::size_t>(ym)] -= scale;
  flux.z[static_cast<std::size_t>(center)] += scale;
  flux.z[static_cast<std::size_t>(zm)] -= scale;
}

double environment_energy(
    const std::vector<double>& environment,
    int L,
    int parity) {
  long double result = 0.0L;
  for (int index = 0; index < L * L * L; ++index) {
    int x = 0, y = 0, z = 0;
    coordinates(L, index, x, y, z);
    if (parity_of(x, y, z) != parity) continue;
    const long double value = environment[static_cast<std::size_t>(index)];
    result += value * value / 12.0L;
  }
  return static_cast<double>(result);
}

ReversibleCheckerboardGaussStatus validate(
    const MatchedFaceFlux& flux,
    const std::vector<double>& charge,
    const std::vector<double>* environment,
    double tolerance) {
  if (flux.L < 4) return ReversibleCheckerboardGaussStatus::InvalidSize;
  if ((flux.L & 1) != 0) {
    return ReversibleCheckerboardGaussStatus::OddPeriodicSize;
  }
  const std::size_t count = static_cast<std::size_t>(flux.L * flux.L * flux.L);
  if (charge.size() != count
      || (environment != nullptr && environment->size() != count)) {
    return ReversibleCheckerboardGaussStatus::ShapeMismatch;
  }
  if (!std::isfinite(tolerance) || tolerance <= 0.0
      || !finite_face(flux) || !finite_values(charge)
      || (environment != nullptr && !finite_values(*environment))) {
    return ReversibleCheckerboardGaussStatus::NonFiniteInput;
  }
  const long double sum = std::accumulate(
      charge.begin(), charge.end(), 0.0L,
      [](long double total, double value) { return total + value; });
  double scale = 1.0;
  for (double value : charge) scale = std::max(scale, std::abs(value));
  if (std::abs(static_cast<double>(sum))
      > tolerance * scale * static_cast<double>(count)) {
    return ReversibleCheckerboardGaussStatus::IncompatibleCharge;
  }
  return ReversibleCheckerboardGaussStatus::Valid;
}

}  // namespace

ReversibleCheckerboardGaussLayer
apply_reversible_checkerboard_gauss_layer(
    MatchedFaceFlux& flux,
    const std::vector<double>& compatible_charge,
    int parity,
    const std::vector<double>& incoming_environment,
    double tolerance) {
  ReversibleCheckerboardGaussLayer result;
  result.L = flux.L;
  result.parity = parity & 1;
  result.incoming_environment = incoming_environment;
  result.pseudoinverse_used = false;
  result.born_target_used = false;
  result.production_coupling_used = false;
  result.new_selected_type_added = false;
  result.six_face_local = true;
  result.disjoint_checkerboard_support = (flux.L & 1) == 0;

  result.status = validate(
      flux, compatible_charge, &incoming_environment, tolerance);
  if (!result.valid()) return result;

  const std::size_t count = compatible_charge.size();
  result.outgoing_environment.assign(count, 0.0);
  result.field_energy_before = quadratic_energy(flux);
  result.incoming_environment_energy = environment_energy(
      incoming_environment, flux.L, result.parity);
  result.fresh_environment = result.incoming_environment_energy == 0.0;

  for (int x = 0; x < flux.L; ++x) {
    for (int y = 0; y < flux.L; ++y) {
      for (int z = 0; z < flux.L; ++z) {
        if (parity_of(x, y, z) != result.parity) continue;
        const int index = flux.index(x, y, z);
        const std::size_t offset = static_cast<std::size_t>(index);
        const double old_residual = active_residual(
            flux, compatible_charge, x, y, z);
        const double incoming = incoming_environment[offset];
        result.maximum_active_residual_before = std::max(
            result.maximum_active_residual_before, std::abs(old_residual));
        add_incidence_row(
            flux, x, y, z, (incoming - old_residual) / 6.0);
        result.outgoing_environment[offset] = -old_residual;
        result.source_work += compatible_charge[offset]
            * (incoming - old_residual) / 6.0;
        ++result.active_cells;
      }
    }
  }

  for (int x = 0; x < flux.L; ++x) {
    for (int y = 0; y < flux.L; ++y) {
      for (int z = 0; z < flux.L; ++z) {
        if (parity_of(x, y, z) != result.parity) continue;
        result.maximum_active_residual_after = std::max(
            result.maximum_active_residual_after,
            std::abs(active_residual(
                flux, compatible_charge, x, y, z)));
      }
    }
  }

  result.field_energy_after = quadratic_energy(flux);
  result.outgoing_environment_energy = environment_energy(
      result.outgoing_environment, flux.L, result.parity);
  result.energy_ledger_residual =
      result.field_energy_after + result.outgoing_environment_energy
      - result.field_energy_before - result.incoming_environment_energy
      - result.source_work;
  const double energy_scale = std::max({
      1.0,
      std::abs(result.field_energy_before),
      std::abs(result.field_energy_after),
      std::abs(result.incoming_environment_energy),
      std::abs(result.outgoing_environment_energy),
      std::abs(result.source_work),
  });
  result.active_affine_projection_exact = result.fresh_environment
      && result.maximum_active_residual_after <= tolerance
          * std::max(1.0, result.maximum_active_residual_before);
  result.exact_inverse_formula =
      std::abs(result.energy_ledger_residual) <= 20.0 * tolerance * energy_scale;
  result.status = ReversibleCheckerboardGaussStatus::Valid;
  return result;
}

ReversibleCheckerboardGaussStatus
reverse_reversible_checkerboard_gauss_layer(
    MatchedFaceFlux& flux,
    const std::vector<double>& compatible_charge,
    const ReversibleCheckerboardGaussLayer& layer,
    std::vector<double>* recovered_incoming_environment,
    double tolerance) {
  if (!layer.valid() || layer.L != flux.L) {
    return ReversibleCheckerboardGaussStatus::ShapeMismatch;
  }
  const auto status = validate(
      flux, compatible_charge, &layer.outgoing_environment, tolerance);
  if (status != ReversibleCheckerboardGaussStatus::Valid) return status;

  std::vector<double> recovered(compatible_charge.size(), 0.0);
  for (int x = 0; x < flux.L; ++x) {
    for (int y = 0; y < flux.L; ++y) {
      for (int z = 0; z < flux.L; ++z) {
        if (parity_of(x, y, z) != layer.parity) continue;
        const int index = flux.index(x, y, z);
        const std::size_t offset = static_cast<std::size_t>(index);
        const double incoming = active_residual(
            flux, compatible_charge, x, y, z);
        const double old_residual = -layer.outgoing_environment[offset];
        add_incidence_row(
            flux, x, y, z, (old_residual - incoming) / 6.0);
        recovered[offset] = incoming;
      }
    }
  }
  if (recovered_incoming_environment != nullptr) {
    *recovered_incoming_environment = std::move(recovered);
  }
  return ReversibleCheckerboardGaussStatus::Valid;
}

double checkerboard_gauss_residual_l2_squared(
    const MatchedFaceFlux& flux,
    const std::vector<double>& compatible_charge,
    int parity) {
  if (validate(flux, compatible_charge, nullptr, 1e-12)
      != ReversibleCheckerboardGaussStatus::Valid) {
    return std::numeric_limits<double>::infinity();
  }
  long double result = 0.0L;
  for (int x = 0; x < flux.L; ++x) {
    for (int y = 0; y < flux.L; ++y) {
      for (int z = 0; z < flux.L; ++z) {
        if (parity >= 0 && parity_of(x, y, z) != (parity & 1)) continue;
        const long double value = active_residual(
            flux, compatible_charge, x, y, z);
        result += value * value;
      }
    }
  }
  return static_cast<double>(result);
}

double matched_face_difference_energy(
    const MatchedFaceFlux& first,
    const MatchedFaceFlux& second) {
  if (first.L != second.L || !finite_face(first) || !finite_face(second)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  long double result = 0.0L;
  for (std::size_t index = 0; index < first.x.size(); ++index) {
    const long double dx = first.x[index] - second.x[index];
    const long double dy = first.y[index] - second.y[index];
    const long double dz = first.z[index] - second.z[index];
    result += dx * dx + dy * dy + dz * dz;
  }
  return static_cast<double>(0.5L * result);
}

ReversibleCheckerboardGaussPreparation::
ReversibleCheckerboardGaussPreparation(
    int size,
    const std::vector<double>& compatible_charge,
    double tolerance) {
  reset(size, compatible_charge, tolerance);
}

ReversibleCheckerboardGaussStatus
ReversibleCheckerboardGaussPreparation::reset(
    int size,
    const std::vector<double>& compatible_charge,
    double tolerance) {
  flux_ = MatchedFaceFlux(size);
  charge_ = compatible_charge;
  history_.clear();
  tolerance_ = tolerance;
  history_energy_ = 0.0;
  source_work_ = 0.0;
  status_ = validate(flux_, charge_, nullptr, tolerance_);
  return status_;
}

ReversibleCheckerboardGaussLayer
ReversibleCheckerboardGaussPreparation::step_fresh_layer() {
  ReversibleCheckerboardGaussLayer result;
  if (!valid()) {
    result.status = status_;
    return result;
  }
  std::vector<double> fresh(charge_.size(), 0.0);
  result = apply_reversible_checkerboard_gauss_layer(
      flux_, charge_, static_cast<int>(history_.size() & 1U), fresh,
      tolerance_);
  if (!result.valid()) {
    status_ = result.status;
    return result;
  }
  history_energy_ += result.outgoing_environment_energy;
  source_work_ += result.source_work;
  history_.push_back(result);
  return result;
}

bool ReversibleCheckerboardGaussPreparation::reverse_last_layer(
    double tolerance) {
  if (!valid() || history_.empty()) return false;
  const auto layer = history_.back();
  std::vector<double> recovered;
  const auto reverse_status = reverse_reversible_checkerboard_gauss_layer(
      flux_, charge_, layer, &recovered, tolerance);
  if (reverse_status != ReversibleCheckerboardGaussStatus::Valid) return false;
  const double maximum_recovered = recovered.empty()
      ? 0.0
      : std::abs(*std::max_element(
          recovered.begin(), recovered.end(),
          [](double first, double second) {
            return std::abs(first) < std::abs(second);
          }));
  if (layer.fresh_environment && maximum_recovered > 20.0 * tolerance) {
    return false;
  }
  history_energy_ -= layer.outgoing_environment_energy;
  source_work_ -= layer.source_work;
  if (std::abs(history_energy_) <= 20.0 * tolerance) history_energy_ = 0.0;
  if (std::abs(source_work_) <= 20.0 * tolerance) source_work_ = 0.0;
  history_.pop_back();
  return true;
}

double ReversibleCheckerboardGaussPreparation::maximum_gauss_residual() const {
  if (!valid()) return std::numeric_limits<double>::infinity();
  double result = 0.0;
  for (int x = 0; x < flux_.L; ++x) {
    for (int y = 0; y < flux_.L; ++y) {
      for (int z = 0; z < flux_.L; ++z) {
        result = std::max(
            result,
            std::abs(active_residual(flux_, charge_, x, y, z)));
      }
    }
  }
  return result;
}

}  // namespace ftd::eft
