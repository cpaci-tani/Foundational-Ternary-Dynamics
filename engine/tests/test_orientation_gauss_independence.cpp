/** FTD-0564: orientation degree and electric Gauss flux are independent. */

#include "ftd/eft/orientation_gauss_independence.h"

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
  const auto result = ftd::eft::analyze_orientation_gauss_independence();
  check("observer verdict",result.valid);
  check("locked arms",result.arms.size()==60);
  check("degree does not determine flux",
        result.degree_does_not_determine_flux);
  check("flux does not determine degree",
        result.flux_does_not_determine_degree);
  check("amplitude rescaling",
        result.amplitude_rescaling_separates_observables);
  check("polarity mirror",result.polarity_mirror_exact);
  check("cubic covariance",result.cubic_covariance_exact);
  check("periodic divergence image",
        result.periodic_divergence_image_is_zero_sum);
  check("rank witnesses",result.rank_witnesses==2);
  check("topology-only magnitude route closed",
        result.topology_alone_charge_magnitude_closed);
  check("topological core plus action remains open",
        result.topological_core_with_action_remains_open);
  check("degree residual",result.maximum_degree_residual<=1e-12);
  check("flux residual",result.maximum_flux_residual<=1e-12);
  check("equal-flux residual",result.maximum_equal_flux_residual<=1e-12);
  check("scale residual",result.maximum_scale_linearity_residual<=1e-12);
  check("mirror residual",result.maximum_polarity_mirror_residual<=1e-12);
  check("rotation residual",result.maximum_cyclic_covariance_residual<=1e-12);
  check("tree routing residual",result.maximum_tree_routing_residual<=1e-12);

  std::cout << std::setprecision(17)
            << "arms=" << result.arms.size() << '\n'
            << "rank_witnesses=" << result.rank_witnesses << '\n'
            << "maximum_degree_residual="
            << result.maximum_degree_residual << '\n'
            << "maximum_flux_residual="
            << result.maximum_flux_residual << '\n'
            << "maximum_equal_flux_residual="
            << result.maximum_equal_flux_residual << '\n'
            << "maximum_scale_linearity_residual="
            << result.maximum_scale_linearity_residual << '\n'
            << "maximum_polarity_mirror_residual="
            << result.maximum_polarity_mirror_residual << '\n'
            << "maximum_cyclic_covariance_residual="
            << result.maximum_cyclic_covariance_residual << '\n'
            << "maximum_tree_routing_residual="
            << result.maximum_tree_routing_residual << '\n'
            << "verdict="
            << (result.valid ? "ORIENTATION_GAUSS_INDEPENDENT"
                             : "ORIENTATION_GAUSS_TEST_FAILED") << '\n'
            << "failures=" << failures << '\n';
  return failures==0 ? 0 : 1;
}
