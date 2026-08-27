#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string_view>

#include "term_toggles.h"

namespace ftd {

// Named profiles shared by production scenario setup and profile audits. Keep
// the term list separate from presentation metadata: this is an executable
// engine contract, not a claim that the prepared state is an atom.
inline constexpr std::array<std::string_view, 6>
    PREPARED_COULOMB_CANDIDATE_TERMS = {
        "wave_propagation",
        "coupling",
        "damping",
        "gauss_projection",
        "forces",
        "movement",
    };

template <std::size_t N>
inline void apply_isolated_toggle_profile(
    TermToggles& toggles,
    const std::array<std::string_view, N>& enabled_terms) {
    for (const auto& spec : TOGGLE_SPECS) toggles.*(spec.field) = false;

    for (const std::string_view name : enabled_terms) {
        const auto it = std::find_if(
            std::begin(TOGGLE_SPECS), std::end(TOGGLE_SPECS),
            [name](const ToggleSpec& spec) { return name == spec.name; });
        if (it == std::end(TOGGLE_SPECS)) {
            throw std::logic_error("unknown term in isolated scenario profile");
        }
        toggles.*(it->field) = true;
    }
}

}  // namespace ftd
