/** FTD-0560: periodic point-hop co-moving dressing obstruction. */

#include "ftd/constants.h"
#include "ftd/eft/native_hop_dressing_obstruction.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

int main() {
  int failures = 0;
  const auto check = [&](const std::string& label,bool condition) {
    if (!condition) {
      std::cerr << "FAIL: " << label << '\n';
      ++failures;
    }
  };

  const auto result = ftd::eft::analyze_native_hop_dressing_obstruction(
      ftd::C_WAVE*ftd::C_WAVE,ftd::G_C);
  check("observer verdict",result.valid);
  check("source orthogonality",
        result.native_source_components_are_orthogonal);
  check("all finite registered periods resonate",
        result.every_finite_registered_period_has_resonance);
  check("resonant native source nonzero",
        result.resonant_native_source_is_nonzero);
  check("Floquet coefficient identity",
        result.axial_floquet_coefficient_identity);
  check("square-summability obstruction",
        result.point_hop_dressing_not_square_summable);
  check("slow-hop forcing asymptotic",
        result.slow_hop_forcing_is_asymptotically_quadratic);
  check("locked arm count",result.arms.size()==96);
  check("root gate",result.maximum_root_residual<=1e-12);
  check("regular roots",result.minimum_regularity_derivative>1e-3);
  check("orthogonality gate",
        result.maximum_source_orthogonality_residual<=1e-12);
  check("coefficient gate",
        result.maximum_coefficient_identity_residual<=1e-12);
  check("nonzero forcing gate",
        result.minimum_normalized_effective_forcing>0.05);
  check("polarity mirror gate",
        result.maximum_polarity_mirror_residual<=1e-12);
  check("cubic covariance gate",
        result.maximum_cubic_covariance_residual<=1e-12);
  check("resonant response gate",
        result.maximum_resonant_coefficient_excess<=1e-12);

  std::cout << std::setprecision(17)
            << "arms=" << result.arms.size() << '\n'
            << "maximum_root_residual="
            << result.maximum_root_residual << '\n'
            << "minimum_regularity_derivative="
            << result.minimum_regularity_derivative << '\n'
            << "maximum_source_orthogonality_residual="
            << result.maximum_source_orthogonality_residual << '\n'
            << "maximum_coefficient_identity_residual="
            << result.maximum_coefficient_identity_residual << '\n'
            << "minimum_normalized_effective_forcing="
            << result.minimum_normalized_effective_forcing << '\n'
            << "maximum_polarity_mirror_residual="
            << result.maximum_polarity_mirror_residual << '\n'
            << "maximum_cubic_covariance_residual="
            << result.maximum_cubic_covariance_residual << '\n'
            << "maximum_resonant_coefficient_excess="
            << result.maximum_resonant_coefficient_excess << '\n'
            << "verdict="
            << (result.valid
                ? "POINT_HOP_DRESSING_OBSTRUCTED"
                : "POINT_HOP_DRESSING_OBSTRUCTION_FAILED") << '\n'
            << "failures=" << failures << '\n';
  return failures==0 ? 0 : 1;
}
