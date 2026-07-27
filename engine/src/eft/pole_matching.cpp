#include "ftd/eft/pole_matching.h"

#include <cmath>

namespace ftd {
namespace eft {

bool CountertermTrajectory::calibrate_once(const PoleMatchResult& reference) {
    if (calibrated_ || !reference.valid || !reference.on_shell ||
        !reference.gauge_independent || reference.scheme.volume <= 0 ||
        reference.scheme.mode <= 0 || reference.scheme.momentum <= 0.0 ||
        reference.scheme.renormalization_condition.empty()) {
        return false;
    }
    eta_ = -reference.bare_mismatch;
    reference_ = reference.scheme;
    calibrated_ = true;
    return true;
}

CountertermPrediction CountertermTrajectory::predict(
    const PoleMatchResult& result, double tolerance) const {
    CountertermPrediction out;
    out.input = result;
    out.eta = calibrated_ ? eta_ : 0.0;
    out.residual_mismatch = result.bare_mismatch + out.eta;
    out.within_tolerance = calibrated_ && result.valid && result.on_shell &&
        result.gauge_independent && tolerance >= 0.0 &&
        std::abs(out.residual_mismatch) <= tolerance + result.uncertainty;
    return out;
}

}  // namespace eft
}  // namespace ftd
