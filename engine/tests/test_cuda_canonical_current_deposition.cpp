// Collision-free deterministic CUDA deposition for canonical oriented faces.

#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/eft/quadratic_coat_face_current.h"

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

  std::vector<QuadraticCoatFaceCurrent> original{
      segment(L,{{{4,4,4},0,0.125},{{13,4,4},0,0.25},
                 {{2,3,5},1,-0.5}}),
      segment(L,{{{4,4,4},0,-0.0625},{{2,3,5},1,0.125},
                 {{6,1,7},2,0.375}})};
  auto permuted=original;
  std::reverse(permuted.begin(),permuted.end());
  for(auto& item:permuted)
    std::reverse(item.sparse_current.begin(),item.sparse_current.end());

  const auto canonical=aggregate_quadratic_coat_face_current(
      original,scale,0.0);
  const auto permuted_canonical=aggregate_quadratic_coat_face_current(
      permuted,scale,0.0);
  if(!canonical.valid||!permuted_canonical.valid
      ||canonical.raw_contributions!=6||canonical.entries.size()!=3)
    return 1;

  CudaMatchedFieldPipeline first(L),second(L);
  if(!first.valid()||!second.valid()
      ||!first.upload(electric,magnetic)||!second.upload(electric,magnetic)
      ||!first.prepare_forward(lambda)||!second.prepare_forward(lambda))
    return 1;
  MatchedFaceFlux pre;
  MatchedEdgeField magnetic_after;
  if(!first.download_prepared(magnetic_after,pre)
      ||!first.apply_canonical_sparse_current(original,scale)
      ||!second.apply_canonical_sparse_current(permuted,scale))
    return 1;
  MatchedFaceFlux after_first,after_second;
  MatchedEdgeField magnetic_first,magnetic_second;
  if(!first.download_after(after_first,magnetic_first)
      ||!second.download_after(after_second,magnetic_second)) return 1;

  auto expected=pre;
  for(const auto& entry:canonical.entries) {
    const auto i=static_cast<std::size_t>(original.front().index(
        entry.face.x,entry.face.y,entry.face.z));
    auto& component=entry.axis==0?expected.x:(entry.axis==1?expected.y:expected.z);
    component[i]-=entry.value;
  }
  const double repeat_difference=difference(after_first,after_second);
  const double expected_difference=difference(after_first,expected);
  const double magnetic_difference=difference(magnetic_first,magnetic_second);
  bool coefficient_identity=canonical.entries.size()==permuted_canonical.entries.size();
  for(std::size_t i=0;i<canonical.entries.size()&&coefficient_identity;++i) {
    const auto& a=canonical.entries[i];
    const auto& b=permuted_canonical.entries[i];
    coefficient_identity=a.axis==b.axis&&a.face.x==b.face.x
        &&a.face.y==b.face.y&&a.face.z==b.face.z&&a.value==b.value;
  }
  const bool pass=coefficient_identity&&repeat_difference==0.0
      &&expected_difference==0.0&&magnetic_difference==0.0;
  std::cout<<std::setprecision(17)
      <<"raw="<<canonical.raw_contributions
      <<" net="<<canonical.entries.size()
      <<" repeat_difference="<<repeat_difference
      <<" expected_difference="<<expected_difference
      <<" magnetic_difference="<<magnetic_difference<<'\n';
  return pass?0:1;
}

