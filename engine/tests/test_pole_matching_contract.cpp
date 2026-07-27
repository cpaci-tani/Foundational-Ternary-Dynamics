#include "ftd/eft/pole_matching.h"

#include <cmath>
#include <iostream>

namespace {
int failures = 0;
void check(const char* name, bool ok) {
    std::cout << (ok ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!ok) ++failures;
}

ftd::eft::PoleMatchResult result(double mismatch) {
    ftd::eft::PoleMatchResult out;
    out.scheme.volume = 128;
    out.scheme.mode = 1;
    out.scheme.momentum = 2.0 * 3.14159265358979323846 / 128.0;
    out.scheme.gauge_xi = 1.0;
    out.scheme.infrared = ftd::eft::InfraredPrescription::PositiveMass;
    out.scheme.masses = {0.01};
    out.scheme.charged_species = 1;
    out.scheme.fit_window_min = 0.02;
    out.scheme.fit_window_max = 0.08;
    out.scheme.renormalization_condition = "photon/lightest-Dirac pole equality";
    out.bare_mismatch = mismatch;
    out.uncertainty = 1e-9;
    out.on_shell = true;
    out.gauge_independent = true;
    out.valid = true;
    return out;
}
}  // namespace

int main() {
    ftd::eft::CountertermTrajectory trajectory;
    auto off_shell = result(-0.3);
    off_shell.on_shell = false;
    check("off-shell input cannot calibrate the physical trajectory",
          !trajectory.calibrate_once(off_shell));

    const auto reference = result(-0.25);
    check("one physical reference calibrates", trajectory.calibrate_once(reference));
    check("eta cancels the reference mismatch", std::abs(trajectory.eta() - 0.25) < 1e-15);
    check("second calibration is rejected", !trajectory.calibrate_once(result(-0.1)));

    const auto same = trajectory.predict(reference, 1e-8);
    check("reference prediction closes", same.within_tolerance);
    const auto threshold = trajectory.predict(result(-0.27), 1e-8);
    check("new threshold is predicted without retuning",
          !threshold.within_tolerance && std::abs(threshold.residual_mismatch + 0.02) < 1e-15);
    return failures;
}
