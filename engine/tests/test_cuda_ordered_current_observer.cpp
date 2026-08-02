// Ordered raw CUDA deposition and deterministic selected-radius observation.

#include "ftd/eft/cuda_matched_field_pipeline.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {

template <typename Field>
double difference(const Field& left,const Field& right) {
  double result=0.0;
  for(std::size_t i=0;i<left.x.size();++i)
    result=std::max({result,std::abs(left.x[i]-right.x[i]),
        std::abs(left.y[i]-right.y[i]),std::abs(left.z[i]-right.z[i])});
  return result;
}

ftd::eft::QuadraticCoatFaceCurrent segment(
    int L,std::vector<ftd::eft::QuadraticCoatSparseCurrentEntry> entries) {
  ftd::eft::QuadraticCoatFaceCurrent result;
  result.L=L;
  result.sparse_current=std::move(entries);
  result.dense_materialized=false;
  result.valid=true;
  return result;
}

void apply_cpu(ftd::eft::MatchedFaceFlux& field,
               const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
               double scale) {
  for(const auto& item:segments) for(const auto& entry:item.sparse_current) {
    const auto i=static_cast<std::size_t>(item.index(
        entry.face.x,entry.face.y,entry.face.z));
    auto& component=entry.axis==0?field.x:(entry.axis==1?field.y:field.z);
    component[i]+=-scale*entry.value;
  }
}

bool equal_profile(const ftd::eft::BatchedRegionalEnergyProfile& a,
                   const ftd::eft::BatchedRegionalEnergyProfile& b) {
  if(a.valid!=b.valid||a.energy_before!=b.energy_before
      ||a.energy_pre_current!=b.energy_pre_current
      ||a.energy_after!=b.energy_after
      ||a.maximum_scalar_equivalence_residual
          !=b.maximum_scalar_equivalence_residual
      ||a.regions.size()!=b.regions.size()) return false;
  for(std::size_t i=0;i<a.regions.size();++i) {
    const auto& x=a.regions[i]; const auto& y=b.regions[i];
    if(x.valid!=y.valid||x.chebyshev_radius!=y.chebyshev_radius
        ||x.energy_before!=y.energy_before
        ||x.energy_pre_current!=y.energy_pre_current
        ||x.energy_after!=y.energy_after
        ||x.boundary_transport_into!=y.boundary_transport_into
        ||x.source_exchange_into_field!=y.source_exchange_into_field
        ||x.energy_change!=y.energy_change
        ||x.global_source_free_residual!=y.global_source_free_residual
        ||x.partition_residual!=y.partition_residual
        ||x.regional_ledger_residual!=y.regional_ledger_residual) return false;
  }
  return true;
}

}  // namespace

int main() {
  using namespace ftd::eft;
  constexpr int L=9;
  constexpr double lambda=0.25*ftd::C_SPEED;
  constexpr double scale=0.75;
  MatchedFaceFlux electric(L);
  MatchedEdgeField magnetic(L);
  for(int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z) {
    const auto i=static_cast<std::size_t>(electric.index(x,y,z));
    electric.x[i]=1e-3*std::sin(0.17*x-0.23*y+0.31*z);
    electric.y[i]=1e-3*std::cos(0.29*x+0.11*y-0.19*z);
    electric.z[i]=1e-3*std::sin(0.13*x+0.37*y+0.07*z);
    magnetic.x[i]=7e-4*std::cos(0.21*x-0.09*y+0.27*z);
    magnetic.y[i]=7e-4*std::sin(0.33*x+0.15*y-0.05*z);
    magnetic.z[i]=7e-4*std::cos(0.25*x+0.03*y+0.35*z);
  }
  const std::vector<QuadraticCoatFaceCurrent> current{
      segment(L,{{{4,4,4},0,0.125},{{13,4,4},0,0.25},
                 {{2,3,5},1,-0.5}}),
      segment(L,{{{4,4,4},0,-0.0625},{{2,3,5},1,0.125},
                 {{6,1,7},2,0.375}})};
  CudaMatchedFieldPipeline first(L),second(L);
  if(!first.valid()||!second.valid()
      ||!first.upload(electric,magnetic)||!second.upload(electric,magnetic)
      ||!first.prepare_forward(lambda)||!second.prepare_forward(lambda))
    return 1;
  MatchedFaceFlux pre;
  MatchedEdgeField prepared_magnetic;
  if(!first.download_prepared(prepared_magnetic,pre)
      ||!first.apply_ordered_sparse_current(current,scale)
      ||!second.apply_ordered_sparse_current(current,scale)) return 1;
  MatchedFaceFlux expected=pre,after_first,after_second;
  apply_cpu(expected,current,scale);
  MatchedEdgeField magnetic_first,magnetic_second;
  if(!first.download_after(after_first,magnetic_first)
      ||!second.download_after(after_second,magnetic_second)) return 1;
  const auto profile_first=first.observe_deterministic(
      lambda,{4.0,4.0,4.0},{1,2,3},1e-10);
  const auto profile_second=second.observe_deterministic(
      lambda,{4.0,4.0,4.0},{1,2,3},1e-10);
  const auto profile_repeat=first.observe_deterministic(
      lambda,{4.0,4.0,4.0},{1,2,3},1e-10);

  const std::vector<QuadraticCoatFaceCurrent> ordered_a{
      segment(L,{{{1,1,1},0,1e16},{{1,1,1},0,-1e16},
                 {{1,1,1},0,1.0}})};
  const std::vector<QuadraticCoatFaceCurrent> ordered_b{
      segment(L,{{{1,1,1},0,1.0},{{1,1,1},0,1e16},
                 {{1,1,1},0,-1e16}})};
  CudaMatchedFieldPipeline order_a(L),order_b(L);
  MatchedFaceFlux zero(L),pre_a,pre_b,after_a,after_b,cpu_a,cpu_b;
  MatchedEdgeField zero_b(L),prepared_a,prepared_b,unused_a,unused_b;
  if(!order_a.valid()||!order_b.valid()
      ||!order_a.upload(zero,zero_b)||!order_b.upload(zero,zero_b)
      ||!order_a.prepare_forward(lambda)||!order_b.prepare_forward(lambda)
      ||!order_a.download_prepared(prepared_a,pre_a)
      ||!order_b.download_prepared(prepared_b,pre_b)
      ||!order_a.apply_ordered_sparse_current(ordered_a,1.0)
      ||!order_b.apply_ordered_sparse_current(ordered_b,1.0)
      ||!order_a.download_after(after_a,unused_a)
      ||!order_b.download_after(after_b,unused_b)) return 1;
  cpu_a=pre_a; cpu_b=pre_b;
  apply_cpu(cpu_a,ordered_a,1.0); apply_cpu(cpu_b,ordered_b,1.0);

  const double cpu_difference=difference(after_first,expected);
  const double repeat_difference=difference(after_first,after_second);
  const double order_a_difference=difference(after_a,cpu_a);
  const double order_b_difference=difference(after_b,cpu_b);
  const double order_sensitivity=difference(after_a,after_b);
  const bool observer_repeat=equal_profile(profile_first,profile_second)
      &&equal_profile(profile_first,profile_repeat);
  const bool pass=cpu_difference==0.0&&repeat_difference==0.0
      &&order_a_difference==0.0&&order_b_difference==0.0
      &&order_sensitivity>0.0&&profile_first.valid&&observer_repeat;
  std::cout<<std::setprecision(17)
      <<"cpu_difference="<<cpu_difference
      <<" repeat_difference="<<repeat_difference
      <<" order_a_difference="<<order_a_difference
      <<" order_b_difference="<<order_b_difference
      <<" order_sensitivity="<<order_sensitivity
      <<" observer_repeat="<<observer_repeat
      <<" partition="<<profile_first.maximum_scalar_equivalence_residual
      <<'\n';
  return pass?0:1;
}
