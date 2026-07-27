/** FTD-0579: finite rigid Moore-carrier obstruction audit. */

#include "ftd/eft/finite_rigid_moore_carrier_obstruction.h"

#include <iostream>
#include <string>

namespace {
int failures=0;
void check(const std::string& label,bool pass){
  std::cout<<(pass?"  PASS  ":"  FAIL  ")<<label<<'\n';if(!pass)++failures;
}
}

int main(){
  const auto r=ftd::eft::analyze_finite_rigid_moore_carrier_obstruction();
  check("finite carrier mismatch factorizes in the Laurent domain",
        r.laurent_factorization_exact);
  check("520 direct and Fourier centering arms agree",
        r.centering_arms==520
        &&r.maximum_direct_fourier_centering_residual<=1e-12);
  check("axial centering closes but every registered diagonal remains nonzero",
        r.every_registered_diagonal_mismatch_positive
        &&r.maximum_axial_centering_norm2<=1e-12
        &&r.minimum_diagonal_centering_norm2>1e-14
        &&!r.finite_diagonal_centering_cure_exists);
  check("all 60 rigid-carrier Peierls coefficients are strictly positive",
        r.every_registered_peierls_barrier_positive
        &&r.peierls_coefficient_arms==60
        &&r.peierls_potential_samples==540
        &&r.minimum_peierls_coefficient>1e-14
        &&r.minimum_peierls_barrier>1e-14
        &&r.maximum_peierls_law_residual<=1e-12
        &&r.maximum_polarity_residual<=1e-12
        &&r.maximum_cubic_covariance_residual<=1e-12
        &&!r.finite_rigid_peierls_cure_exists);
  check("12 smooth-binomial controls obey exact centering ratios and suppress only",
        r.binomial_suppression_only
        &&r.binomial_scaling_arms==12
        &&r.maximum_binomial_centering_residual<=1e-12
        &&r.minimum_binomial_scaled_index_at_max_order>=0.45
        &&r.maximum_binomial_scaled_index_at_max_order<=0.51);
  check("no native extended carrier or production behavior is promoted",
        !r.extended_native_carrier_derived&&!r.production_changed);
  check("registered FTD-0579 verdict closes",r.valid);

  std::cout.precision(17);
  std::cout<<"profile_count="<<r.profile_count<<'\n'
    <<"centering_arms="<<r.centering_arms<<'\n'
    <<"peierls_coefficient_arms="<<r.peierls_coefficient_arms<<'\n'
    <<"peierls_potential_samples="<<r.peierls_potential_samples<<'\n'
    <<"binomial_scaling_arms="<<r.binomial_scaling_arms<<'\n'
    <<"maximum_direct_fourier_centering_residual="<<r.maximum_direct_fourier_centering_residual<<'\n'
    <<"maximum_axial_centering_norm2="<<r.maximum_axial_centering_norm2<<'\n'
    <<"minimum_diagonal_centering_norm2="<<r.minimum_diagonal_centering_norm2<<'\n'
    <<"minimum_peierls_coefficient="<<r.minimum_peierls_coefficient<<'\n'
    <<"minimum_peierls_barrier="<<r.minimum_peierls_barrier<<'\n'
    <<"maximum_peierls_law_residual="<<r.maximum_peierls_law_residual<<'\n'
    <<"maximum_polarity_residual="<<r.maximum_polarity_residual<<'\n'
    <<"maximum_cubic_covariance_residual="<<r.maximum_cubic_covariance_residual<<'\n'
    <<"maximum_binomial_centering_residual="<<r.maximum_binomial_centering_residual<<'\n'
    <<"minimum_binomial_scaled_index_at_max_order="<<r.minimum_binomial_scaled_index_at_max_order<<'\n'
    <<"maximum_binomial_scaled_index_at_max_order="<<r.maximum_binomial_scaled_index_at_max_order<<'\n'
    <<"finite_rigid_moore_carrier_obstruction failures="<<failures<<'\n'
    <<"verdict=FINITE_RIGID_MOORE_CARRIER_CANNOT_REMOVE_CENTERING_OR_PEIERLS_EXTENSION_SUPPRESSES_ONLY\n";
  return failures==0?0:1;
}
