/**
 * Test: Larmor radiation-reaction damping law (regression for the 2026-09-16
 * sign-of-physics fix).
 *
 * BUG BEING PINNED. phase_write and its CUDA mirror computed
 *
 *     larmor_mod  = min(1, LARMOR_FLOOR + K_LARMOR * a^2)
 *     eff_damping = 1 - DAMPING * larmor_mod
 *
 * i.e. the baseline loss MULTIPLIED by a factor capped at 1. Enabling the
 * toggle could therefore only ever REDUCE dissipation: 1% of the baseline loss
 * at a = 0, parity with the baseline at a = sqrt((1-FLOOR)/K_LARMOR) ~ 0.171,
 * and never more than the baseline at any acceleration. An accelerating charge
 * radiated LESS than a static one, and constants.h's claim that the N_EFF
 * scaling of K_LARMOR "ensures Larmor dominates" was unreachable for every
 * value of the constant.
 *
 * The law now lives in ftd/larmor_damping.h and both backends call it:
 *
 *     gain(a) = min(1 + K_LARMOR * a^2, LARMOR_MAX_GAIN)
 *     eff(a)  = damping_factor ^ gain(a)
 *
 * Checks:
 *   LDL-1: eff(0) is EXACTLY the baseline survival factor (toggle is a no-op
 *          at zero acceleration).
 *   LDL-2: eff is monotonically non-increasing in |a| over a wide sweep, so
 *          the damping applied (1 - eff) is monotonically non-decreasing.
 *   LDL-3: eff(a) < baseline strictly for every a > 0 — Larmor always
 *          dissipates MORE than baseline. This is the assertion the old
 *          formula could not satisfy for ANY a.
 *   LDL-4: eff stays in (0, baseline] for extreme / non-finite accelerations —
 *          no sign flip, no amplification, no NaN reaching the registers.
 *   LDL-5: the gain is the a^2 law below the clamp, and saturates at it.
 *   LDL-6: engine level — with the toggle ON at a genuinely accelerating site
 *          the field retains LESS energy than the same run with the toggle
 *          OFF. (The old formula produced the opposite ordering, which
 *          test_larmor.cpp LAM-1 used to assert.)
 *   LDL-7: the toggle remains golden-neutral: with larmor_radiation OFF the
 *          run is bit-identical to a run that never mentions it.
 */

#include "ftd/constants.h"
#include "ftd/larmor_damping.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <limits>

namespace {

// The baseline per-step survival factor phase_write computes for dt <= 1.
const double kBaseline = 1.0 - ftd::DAMPING;

// Energy left in the field after `ticks` ticks at a site whose acceleration
// magnitude is pinned to `accel`. Forces and movement are OFF, so nothing
// overwrites Voxel::accel_mag after we set it; the damping block is the only
// consumer of near_accel.
double retained_energy(bool larmor, double accel, int ticks) {
    ftd::RenderBridge rb(16);
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.damping = true;
    rb.toggles.selective_damping = true;
    rb.toggles.larmor_radiation = larmor;

    const int mid = 8;
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    ftd::Voxel& v = rb.voxel_at(mid, mid, mid);
    v.locked = true;
    v.accel_mag = accel;

    rb.run(ticks);
    return rb.energy_audit().field_energy;
}

}  // namespace

int main() {
    ftd::test::init("test_larmor_damping_law");

    // ------------------------------------------------------------------
    ftd::test::section("LDL-1: zero acceleration is exactly the baseline");
    {
        const double eff0 = ftd::larmor_effective_damping(kBaseline, 0.0);
        ftd::test::check("LDL-1: eff(0) == baseline bit-exactly",
                         eff0 == kBaseline);
        ftd::test::check("LDL-1: gain(0) == 1",
                         ftd::larmor_damping_gain(0.0) == 1.0);
    }

    // ------------------------------------------------------------------
    ftd::test::section("LDL-2: damping is monotonically non-decreasing in |a|");
    {
        bool monotone = true;
        double prev_eff = ftd::larmor_effective_damping(kBaseline, 0.0);
        double prev_a = 0.0;
        // Sweep across the whole interesting range: the old formula's parity
        // point (~0.171), the clamp knee (a = sqrt(255/K_LARMOR) ~ 2.74), and
        // well beyond it.
        for (int k = 1; k <= 2000; ++k) {
            const double a = k * 0.005;   // 0.005 .. 10.0
            const double eff = ftd::larmor_effective_damping(kBaseline, a);
            if (eff > prev_eff) {
                std::printf("    non-monotone at a=%.4f: eff=%.17g > prev(a=%.4f)=%.17g\n",
                            a, eff, prev_a, prev_eff);
                monotone = false;
                break;
            }
            prev_eff = eff;
            prev_a = a;
        }
        ftd::test::check("LDL-2: eff(|a|) non-increasing over a in [0, 10]",
                         monotone);
    }

    // ------------------------------------------------------------------
    ftd::test::section("LDL-3: every a > 0 dissipates MORE than baseline");
    {
        // Sample points chosen to straddle the old formula's behaviour: below
        // its parity point it damped 100x LESS than baseline, at 0.171 it
        // reached parity, and above it stayed pinned AT baseline.
        const double probes[] = {1e-3, 1e-2, 0.05, 0.1, 0.171, 0.5, 1.0, 5.0};
        bool all_above = true;
        for (const double a : probes) {
            const double eff = ftd::larmor_effective_damping(kBaseline, a);
            const double loss = 1.0 - eff;
            const double base_loss = 1.0 - kBaseline;
            std::printf("    a=%-7.4g  eff=%.12f  loss/baseline_loss=%.6f\n",
                        a, eff, loss / base_loss);
            ftd::test::metric("ldl3.loss_ratio", loss / base_loss, 0);
            if (!(eff < kBaseline) || !(loss > base_loss)) all_above = false;
        }
        ftd::test::check("LDL-3: eff(a) < baseline and loss(a) > baseline loss "
                         "for every a > 0",
                         all_above);
    }

    // ------------------------------------------------------------------
    ftd::test::section("LDL-4: bounded and stable at extreme accelerations");
    {
        const double huge = 1e12;
        const double inf = std::numeric_limits<double>::infinity();
        const double nan = std::numeric_limits<double>::quiet_NaN();

        const double e_huge = ftd::larmor_effective_damping(kBaseline, huge);
        const double e_inf = ftd::larmor_effective_damping(kBaseline, inf);
        const double e_nan = ftd::larmor_effective_damping(kBaseline, nan);
        std::printf("    eff(1e12)=%.17g  eff(inf)=%.17g  eff(nan)=%.17g\n",
                    e_huge, e_inf, e_nan);

        ftd::test::check("LDL-4: eff stays in (0, baseline] at a = 1e12",
                         e_huge > 0.0 && e_huge <= kBaseline);
        ftd::test::check("LDL-4: eff stays in (0, baseline] at a = inf",
                         e_inf > 0.0 && e_inf <= kBaseline);
        ftd::test::check("LDL-4: a = NaN does not propagate NaN into the factor",
                         std::isfinite(e_nan) && e_nan > 0.0
                             && e_nan <= kBaseline);
        // The clamp bound, stated explicitly so a change to LARMOR_MAX_GAIN
        // has to come here.
        ftd::test::check_close("LDL-4: worst-case factor == baseline^MAX_GAIN",
                               e_huge,
                               std::pow(kBaseline, ftd::LARMOR_MAX_GAIN),
                               1e-15);
    }

    // ------------------------------------------------------------------
    ftd::test::section("LDL-5: gain follows the a^2 law, then saturates");
    {
        const double a1 = 0.05, a2 = 0.10;
        const double g1 = ftd::larmor_damping_gain(a1) - 1.0;
        const double g2 = ftd::larmor_damping_gain(a2) - 1.0;
        ftd::test::check_close("LDL-5: gain excess scales as a^2 (ratio 4)",
                               g2 / g1, 4.0, 1e-9);
        ftd::test::check_close("LDL-5: gain excess == K_LARMOR * a^2",
                               g1, ftd::K_LARMOR * a1 * a1, 1e-12);
        ftd::test::check("LDL-5: gain saturates at LARMOR_MAX_GAIN",
                         ftd::larmor_damping_gain(1e6) == ftd::LARMOR_MAX_GAIN);
    }

    // ------------------------------------------------------------------
    ftd::test::section("LDL-6: engine level — accelerating site loses more");
    {
        const int kTicks = 400;
        const double accel = 0.4;   // gain ~ 6.4 => clearly above baseline
        const double e_on = retained_energy(true, accel, kTicks);
        const double e_off = retained_energy(false, accel, kTicks);
        std::printf("    larmor ON : field_energy = %.9g\n", e_on);
        std::printf("    larmor OFF: field_energy = %.9g\n", e_off);
        ftd::test::metric("ldl6.energy_on", e_on, kTicks);
        ftd::test::metric("ldl6.energy_off", e_off, kTicks);
        ftd::test::check("LDL-6: Larmor ON retains LESS field energy than the "
                         "baseline at an accelerating site",
                         std::isfinite(e_on) && std::isfinite(e_off)
                             && e_on < e_off);
    }

    // ------------------------------------------------------------------
    ftd::test::section("LDL-7: toggle OFF is exactly the baseline run");
    {
        const double e_a = retained_energy(false, 0.4, 200);
        const double e_b = retained_energy(false, 0.0, 200);
        ftd::test::check("LDL-7: with the toggle OFF the acceleration value is "
                         "ignored bit-exactly (golden-neutral)",
                         e_a == e_b);
    }

    return ftd::test::finalize();
}
