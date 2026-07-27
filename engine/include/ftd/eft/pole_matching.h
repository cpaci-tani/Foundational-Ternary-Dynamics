#pragma once
/**
 * @file ftd/eft/pole_matching.h
 * @brief Scheme-carrying records for physical pole and universal-cone matching.
 *
 * These records prevent an off-shell coefficient from being silently reused
 * as an on-shell observable. They contain no dynamics and perform no fit.
 */

#include <string>
#include <vector>

namespace ftd {
namespace eft {

enum class InfraredPrescription {
    QedL,
    PositiveMass,
    InfiniteVolume,
};

struct PoleScheme {
    int volume = 0;
    int mode = 0;
    double momentum = 0.0;
    double gauge_xi = 1.0;
    InfraredPrescription infrared = InfraredPrescription::QedL;
    std::vector<double> masses;
    int charged_species = 0;
    double fit_window_min = 0.0;
    double fit_window_max = 0.0;
    std::string renormalization_condition;
};

struct PoleMatchResult {
    PoleScheme scheme;
    double matter_speed = 0.0;
    double photon_speed = 0.0;
    double bare_mismatch = 0.0;
    double uncertainty = 0.0;
    bool on_shell = false;
    bool gauge_independent = false;
    bool valid = false;
};

struct CountertermPrediction {
    PoleMatchResult input;
    double eta = 0.0;
    double residual_mismatch = 0.0;
    bool within_tolerance = false;
};

class CountertermTrajectory {
  public:
    /// Calibrate once from a valid, gauge-independent on-shell result.
    /// Subsequent calibration attempts are rejected.
    bool calibrate_once(const PoleMatchResult& reference);
    bool calibrated() const { return calibrated_; }
    double eta() const { return eta_; }
    const PoleScheme& reference_scheme() const { return reference_; }

    CountertermPrediction predict(const PoleMatchResult& result,
                                  double tolerance) const;

  private:
    bool calibrated_ = false;
    double eta_ = 0.0;
    PoleScheme reference_{};
};

}  // namespace eft
}  // namespace ftd
