#include "ftd/eft/support_invariant_matter_predicate.h"

#include <cmath>
#include <numeric>

namespace ftd::eft {
namespace {

bool finite(const Vec3& value) {
  return std::isfinite(value.x)&&std::isfinite(value.y)
      &&std::isfinite(value.z);
}

Vec3 effective_position(const MatchedMatterPoint& point) {
  return {point.anchor.x+point.remainder.x,
          point.anchor.y+point.remainder.y,
          point.anchor.z+point.remainder.z};
}

}  // namespace

SupportInvariantMatterPredicate observe_support_invariant_matter(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options) {
  SupportInvariantMatterPredicate result;
  result.constituent_count=static_cast<int>(state.constituents.size());
  result.net_charge=std::accumulate(
      state.charges.begin(),state.charges.end(),0);
  const bool shapes=state.electric.L>0
      &&state.magnetic_half.L==state.electric.L;
  const bool charges=state.charges.size()==2
      &&state.charges[0]==-state.charges[1]
      &&(state.charges[0]==-1||state.charges[0]==+1);
  result.sector_valid=shapes&&state.constituents.size()==2&&charges
      &&state.edges.empty()
      &&options.binding_law==ConnectedBindingLaw::DerivedCompactPair
      &&options.compact_pair_cutoff_distance_squared>0.0
      &&options.compact_pair_well_depth>0.0;
  if(!result.sector_valid) {
    result.valid=true;
    return result;
  }

  const Vec3 x0=effective_position(state.constituents[0]);
  const Vec3 x1=effective_position(state.constituents[1]);
  result.center=(x0+x1)*0.5;
  result.relative_position=x1-x0;
  result.separation_squared=result.relative_position.mag2();
  result.total_momentum=state.constituents[0].momentum
      +state.constituents[1].momentum;
  const double rest=options.constituent_mass_scale*E_REST;
  for(const auto& point:state.constituents) {
    if(!finite(effective_position(point))||!finite(point.momentum)) return result;
    result.constituent_kinetic_energy+=std::sqrt(
        rest*rest+C_SPEED*C_SPEED*point.momentum.mag2())-rest;
  }
  result.binding_energy=connected_moore_block_binding_energy(state,options);
  result.pair_energy=result.constituent_kinetic_energy+result.binding_energy;
  result.graph_margin=options.compact_pair_cutoff_distance_squared
      -result.separation_squared;
  result.energy_margin=-result.pair_energy;
  result.valid=finite(result.center)&&finite(result.relative_position)
      &&finite(result.total_momentum)
      &&std::isfinite(result.separation_squared)
      &&std::isfinite(result.graph_margin)
      &&std::isfinite(result.constituent_kinetic_energy)
      &&std::isfinite(result.binding_energy)
      &&std::isfinite(result.pair_energy);
  result.member=result.valid&&result.graph_margin>0.0
      &&result.energy_margin>0.0;
  return result;
}

}  // namespace ftd::eft
