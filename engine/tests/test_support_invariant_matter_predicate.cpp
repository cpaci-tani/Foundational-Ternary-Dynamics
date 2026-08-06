/** FTD-0755: state-only support-independent relational-core predicate. */

#include "ftd/eft/support_invariant_matter_predicate.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures=0;

void check(const std::string& label,bool condition) {
  if(condition) return;
  ++failures;
  std::cerr<<"FAIL: "<<label<<'\n';
}

int wrap(int value,int L) {
  const int r=value%L;
  return r<0?r+L:r;
}

ftd::eft::MatchedMatterPoint point_at(
    const ftd::Vec3& position,const ftd::Vec3& momentum,int L) {
  ftd::eft::MatchedMatterPoint result;
  const long long ax=std::llround(position.x);
  const long long ay=std::llround(position.y);
  const long long az=std::llround(position.z);
  result.anchor={wrap(static_cast<int>(ax),L),
                 wrap(static_cast<int>(ay),L),
                 wrap(static_cast<int>(az),L)};
  result.remainder={position.x-ax,position.y-ay,position.z-az};
  result.momentum=momentum;
  return result;
}

ftd::eft::ConnectedMooreBlockState make_pair(
    int L,double separation,const ftd::Vec3& momentum) {
  ftd::eft::ConnectedMooreBlockState result(L);
  const ftd::Vec3 center{static_cast<double>(L/2),
                         static_cast<double>(L/2),
                         static_cast<double>(L/2)};
  result.constituents.push_back(point_at(
      center-ftd::Vec3{0.0,0.0,0.5*separation},momentum,L));
  result.constituents.push_back(point_at(
      center+ftd::Vec3{0.0,0.0,0.5*separation},momentum*(-1.0),L));
  result.charges={+1,-1};
  return result;
}

bool close(double a,double b,double tolerance=1e-13) {
  return std::abs(a-b)<=tolerance*std::max({1.0,std::abs(a),std::abs(b)});
}

}  // namespace

int main() {
  constexpr int L=17;
  ftd::eft::ConnectedMooreBlockOptions selected;
  selected.binding_law=ftd::eft::ConnectedBindingLaw::DerivedCompactPair;
  selected.compact_pair_well_depth=0.01;
  selected.compact_pair_cutoff_distance_squared=1.5;

  const auto quiet=make_pair(L,1.0,{0.0,0.0,0.0});
  const auto quiet_value=ftd::eft::observe_support_invariant_matter(
      quiet,selected);
  check("quiet reciprocal pair valid",quiet_value.valid);
  check("quiet reciprocal pair sector",quiet_value.sector_valid);
  check("quiet reciprocal pair member",quiet_value.member);
  check("quiet reciprocal pair strict margins",
      quiet_value.graph_margin>0.0&&quiet_value.energy_margin>0.0);
  check("predicate scope flags",quiet_value.state_only
      &&quiet_value.support_independent);

  const auto unbound=ftd::eft::observe_support_invariant_matter(
      make_pair(L,1.30,{0.0,0.0,0.012}),selected);
  check("unbound pair rejected",unbound.valid&&unbound.sector_valid
      &&!unbound.member&&unbound.graph_margin<0.0);

  const auto positive_contact=ftd::eft::observe_support_invariant_matter(
      make_pair(L,1.22,{0.0,0.0,0.10}),selected);
  check("positive-energy contact rejected",positive_contact.valid
      &&positive_contact.sector_valid&&!positive_contact.member
      &&positive_contact.graph_margin>0.0
      &&positive_contact.energy_margin<0.0);

  ftd::eft::ConnectedMooreBlockState empty(L);
  const auto empty_value=ftd::eft::observe_support_invariant_matter(
      empty,selected);
  check("empty field rejected by sector",empty_value.valid
      &&!empty_value.sector_valid&&!empty_value.member);

  auto fixed_options=selected;
  fixed_options.binding_law=ftd::eft::ConnectedBindingLaw::FixedEdgeQuartic;
  const auto fixed=ftd::eft::observe_support_invariant_matter(
      quiet,fixed_options);
  check("imposed wrong-action source rejected",fixed.valid
      &&!fixed.sector_valid&&!fixed.member);

  auto four=quiet;
  four.constituents.push_back(four.constituents[0]);
  four.constituents.push_back(four.constituents[1]);
  four.charges={+1,-1,+1,-1};
  const auto four_value=ftd::eft::observe_support_invariant_matter(
      four,selected);
  check("two pairs rejected by one-object sector",four_value.valid
      &&!four_value.sector_valid&&!four_value.member);

  auto conjugate=quiet;
  for(auto& charge:conjugate.charges) charge=-charge;
  const auto conjugate_value=ftd::eft::observe_support_invariant_matter(
      conjugate,selected);
  check("polarity conjugation preserves membership",conjugate_value.member);
  check("polarity conjugation preserves margins",
      close(conjugate_value.graph_margin,quiet_value.graph_margin)
      &&close(conjugate_value.energy_margin,quiet_value.energy_margin));

  auto field_changed=quiet;
  field_changed.electric.x[0]=0.7;
  field_changed.electric.y[1]=-0.4;
  field_changed.magnetic_half.z[2]=0.9;
  const auto field_value=ftd::eft::observe_support_invariant_matter(
      field_changed,selected);
  check("environmental field does not define core",field_value.member);
  check("environmental field preserves core margins",
      close(field_value.graph_margin,quiet_value.graph_margin)
      &&close(field_value.energy_margin,quiet_value.energy_margin));

  std::cout.precision(17);
  std::cout<<"quiet_graph_margin="<<quiet_value.graph_margin<<'\n'
           <<"quiet_energy_margin="<<quiet_value.energy_margin<<'\n'
           <<"positive_contact_energy_margin="
           <<positive_contact.energy_margin<<'\n'
           <<"support_invariant_matter_predicate failures="<<failures<<'\n';
  return failures==0?0:1;
}
