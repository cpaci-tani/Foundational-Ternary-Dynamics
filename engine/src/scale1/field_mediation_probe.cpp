#include "ftd/scale1/field_mediation_probe.h"

#include <algorithm>
#include <cmath>

namespace ftd::scale1 {
namespace {

bool finite_vec3(const Vec3& value) {
    return std::isfinite(value.x) && std::isfinite(value.y)
        && std::isfinite(value.z);
}

int moore_distance(const Vec3& from, const Vec3& to) {
    const double distance = std::max({
        std::abs(to.x - from.x),
        std::abs(to.y - from.y),
        std::abs(to.z - from.z),
    });
    return std::max(1, static_cast<int>(std::ceil(distance)));
}

}  // namespace

bool FieldMediationProbe::dispatch(
        const FieldMediationDispatch& input, std::string* error) {
    const auto fail = [error](const char* message) {
        if (error) *error = message;
        return false;
    };
    if (input.emission_tick < current_tick_) {
        return fail("field carrier emission cannot precede the observer tick");
    }
    if (input.source_id < 0 || input.target_id < 0
        || input.source_id == input.target_id) {
        return fail("field carrier requires distinct nonnegative source and target ids");
    }
    if (!finite_vec3(input.source_position)
        || !finite_vec3(input.target_position)
        || !std::isfinite(input.signed_amplitude)
        || input.signed_amplitude == 0.0) {
        return fail("field carrier input must be finite and nonzero");
    }

    FieldMediationArrival carrier;
    carrier.sequence = next_sequence_++;
    carrier.emission_tick = input.emission_tick;
    carrier.moore_distance = moore_distance(
        input.source_position, input.target_position);
    carrier.arrival_tick = input.emission_tick + carrier.moore_distance;
    carrier.source_id = input.source_id;
    carrier.target_id = input.target_id;
    carrier.signed_amplitude = input.signed_amplitude;
    carrier.booked_energy = 0.5 * input.signed_amplitude
        * input.signed_amplitude;
    dispatched_energy_ += carrier.booked_energy;
    field_energy_ += carrier.booked_energy;

    const auto position = std::upper_bound(
        pending_.begin(), pending_.end(), carrier,
        [](const auto& lhs, const auto& rhs) {
            if (lhs.arrival_tick != rhs.arrival_tick) {
                return lhs.arrival_tick < rhs.arrival_tick;
            }
            return lhs.sequence < rhs.sequence;
        });
    pending_.insert(position, carrier);
    if (error) error->clear();
    return true;
}

std::vector<FieldMediationArrival> FieldMediationProbe::advance_to(
        std::int64_t tick) {
    if (tick < current_tick_) return {};
    current_tick_ = tick;
    std::vector<FieldMediationArrival> arrivals;
    while (!pending_.empty() && pending_.front().arrival_tick <= tick) {
        auto carrier = pending_.front();
        pending_.pop_front();
        field_energy_ -= carrier.booked_energy;
        delivered_energy_ += carrier.booked_energy;
        ++delivered_count_;
        arrivals.push_back(carrier);
    }
    if (std::abs(field_energy_) < 1e-15) field_energy_ = 0.0;
    return arrivals;
}

void FieldMediationProbe::clear() {
    pending_.clear();
    current_tick_ = 0;
    next_sequence_ = 0;
    delivered_count_ = 0;
    dispatched_energy_ = 0.0;
    field_energy_ = 0.0;
    delivered_energy_ = 0.0;
}

}  // namespace ftd::scale1
