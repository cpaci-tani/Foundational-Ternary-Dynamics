/** Regression for the cross-platform FTD-0745 baseline loader. */

#define FTD_CAUSAL_HORIZON_MAIN ftd_0746_embedded_main
#include "test_causal_horizon_environmental_persistence.cpp"

#include <iostream>

int main() {
  const auto fields=horizon_split_csv("alpha,omega\r");
  if(fields.size()!=2||fields[0]!="alpha"||fields[1]!="omega") {
    std::cerr<<"CRLF field normalization failed\n";
    return 1;
  }

  for(const auto* direction:{"0_0_1","0_1_-1","1_1_1"}) {
    bool valid=false;
    const auto baseline=load_horizon_baseline(direction,valid);
    if(!valid||baseline.size()!=static_cast<std::size_t>(kHorizonPrefixTicks+1)) {
      std::cerr<<"failed to load FTD-0745 prefix for "<<direction<<'\n';
      return 1;
    }
  }
  std::cout<<"causal-horizon CRLF baseline loader: PASS\n";
  return 0;
}
