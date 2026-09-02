/**
 * Scale-1 fine-structure claim boundary.
 *
 * This target deliberately preserves the historical filename while replacing
 * its invalid positive-physics assertion.  Scale 1 has an imposed spin-orbit
 * extension and a supported momentum-form Verlet integrator; it does not have
 * a closed fine-structure Hamiltonian or a native atomic bound state.  The
 * retired isotropic `relativistic` force rescale must fail closed.
 */

#include "ftd/particle_engine.h"
#include "ftd/scale1/domain.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {
int failures = 0;

void check(const char* label, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
}

void seed_pair(ftd::ParticleEngine& engine) {
    engine.set_dt(0.25);
    engine.set_softening(0.1);
    engine.toggles.minimal();
    engine.add_locked_particle(+1, {0.0, 0.0, 0.0}, 20.0 * ftd::K_B, +1);
    engine.add_particle(-1, {12.0, 0.0, 0.0}, {0.0, 0.01, 0.0},
                        ftd::K_B, 0.3, +1);
}
}

int main() {
    using namespace ftd;
    std::cout << "== Scale-1 fine-structure claim boundary ==\n";

    {
        ParticleEngine baseline;
        seed_pair(baseline);
        std::string reason;
        check("verified effective baseline is admissible",
              baseline.toggles.validate(&reason));
        const auto diagnostics = baseline.diagnostics();
        check("verified baseline energy ledger is complete",
              diagnostics.state_energy_complete
              && diagnostics.missing_mask == 0
              && diagnostics.drift_eligible);
    }

    {
        ParticleEngine imposed;
        seed_pair(imposed);
        imposed.toggles.spin_orbit = true;
        std::string reason;
        check("imposed spin-orbit extension remains executable",
              imposed.toggles.validate(&reason));
        const auto diagnostics = imposed.diagnostics();
        check("spin-orbit exposes missing potential coverage",
              !diagnostics.state_energy_complete
              && !diagnostics.drift_eligible
              && (diagnostics.missing_mask
                  & scale1_bit(Scale1Coverage::SpinOrbitPotential)) != 0);

        const auto snapshot = imposed.snapshot("s1-advanced-force-isolation", "cpu");
        bool found_unaccounted_spin_orbit = false;
        for (const auto& force : snapshot.forces) {
            if (force.term_id == "spin_orbit") {
                found_unaccounted_spin_orbit = !force.accounted
                    && force.status == Scale1EpistemicStatus::Imposed;
            }
        }
        check("snapshot labels spin-orbit imposed and unaccounted",
              found_unaccounted_spin_orbit);
    }

    {
        ParticleEngine retired;
        seed_pair(retired);
        retired.toggles.relativistic = true;
        std::string reason;
        check("retired isotropic relativistic rescale fails closed",
              !retired.toggles.validate(&reason)
              && reason.find("retired/unavailable") != std::string::npos);
    }

    {
        ParticleEngine momentum_form;
        seed_pair(momentum_form);
        momentum_form.toggles.relativistic_verlet = true;
        std::string reason;
        check("momentum-form Verlet profile is admissible",
              momentum_form.toggles.validate(&reason));
        momentum_form.run(256);
        const auto diagnostics = momentum_form.diagnostics();
        check("momentum-form integration remains finite",
              std::isfinite(diagnostics.total_energy)
              && std::isfinite(diagnostics.total_momentum.x)
              && std::isfinite(diagnostics.total_momentum.y)
              && std::isfinite(diagnostics.total_momentum.z));
    }

    const auto* scenario = find_scale1_scenario_spec("s1-advanced-force-isolation");
    check("advanced-force scenario is explicitly effective reference",
          scenario && scenario->scenario_class == Scale1ScenarioClass::EffectiveReference
          && scenario->status == Scale1EpistemicStatus::Imposed);

    std::cout << (failures == 0 ? "ALL PASS\n"
                                : "FAILURES: " + std::to_string(failures) + "\n");
    return failures == 0 ? 0 : 1;
}
