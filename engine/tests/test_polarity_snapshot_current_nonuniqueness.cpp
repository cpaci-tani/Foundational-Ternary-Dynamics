// FTD-0719: an unordered polarity snapshot fixes div(J), not the cycle current.

#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/eft/quadratic_coat_face_current.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

using ftd::Vec3;

constexpr char snapshot_protocol_sha256[] =
    "DE13969105F196E64C61FC106945B372EBE63DA0230DB30E32526A4BC83E7B77";
constexpr int L = 9;

struct Rotation {
  std::array<int,3> permutation{};
  std::array<int,3> sign{};
};

struct Path {
  Vec3 start{};
  Vec3 end{};
  int charge = 0;
};

struct History {
  bool valid = false;
  double continuity = INFINITY;
  double causal = INFINITY;
  ftd::eft::MatchedFaceFlux current;
  std::vector<double> rho_before;
  std::vector<double> rho_after;
  explicit History(int size=L)
      : current(size),rho_before(static_cast<std::size_t>(size)*size*size,0.0),
        rho_after(static_cast<std::size_t>(size)*size*size,0.0) {}
};

struct Metrics {
  bool valid = false;
  double density_before = INFINITY;
  double density_after = INFINITY;
  double continuity_direct = INFINITY;
  double continuity_crossed = INFINITY;
  double causal = INFINITY;
  double divergence = INFINITY;
  double current_l2 = INFINITY;
  double current_maximum = INFINITY;
  double curl_l2 = INFINITY;
  double curl_maximum = INFINITY;
  double moment = INFINITY;
  double connection_witness = INFINITY;
  ftd::eft::MatchedFaceFlux difference;
  explicit Metrics(int size=L):difference(size){}
};

struct Summary {
  bool construction = false;
  bool density = false;
  bool continuity = false;
  bool causal = false;
  bool solenoidal = false;
  bool nonzero = false;
  bool transverse = false;
  bool moment = false;
  bool witness = false;
  bool reversal = false;
  bool cubic = false;
  bool translation = false;
  int rotations = 0;
  double density_before = INFINITY;
  double density_after = INFINITY;
  double continuity_residual = INFINITY;
  double causal_excess = INFINITY;
  double divergence_residual = INFINITY;
  double current_l2 = INFINITY;
  double current_maximum = INFINITY;
  double curl_l2 = INFINITY;
  double curl_maximum = INFINITY;
  double moment_residual = INFINITY;
  double connection_witness = INFINITY;
  double reversal_residual = INFINITY;
  double cubic_covariance_residual = INFINITY;
  double translation_covariance_residual = INFINITY;
  std::vector<std::array<double,7>> covariance_rows;
  std::string verdict = "POLARITY_SNAPSHOT_CURRENT_WITNESS_EXECUTION_INVALID";
};

int parity(const std::array<int,3>& p) {
  int inversions=0;
  for(int i=0;i<3;++i)for(int j=i+1;j<3;++j)inversions+=p[i]>p[j];
  return inversions%2?-1:1;
}

std::vector<Rotation> rotations() {
  std::vector<Rotation> result;
  std::array<int,3> permutation{{0,1,2}};
  do {
    for(int sx:{-1,1})for(int sy:{-1,1})for(int sz:{-1,1}) {
      std::array<int,3> sign{{sx,sy,sz}};
      if(parity(permutation)*sx*sy*sz==1)result.push_back({permutation,sign});
    }
  } while(std::next_permutation(permutation.begin(),permutation.end()));
  return result;
}

Vec3 rotate(const Vec3& value,const Rotation& rotation) {
  const std::array<double,3> input{{value.x,value.y,value.z}};
  return {rotation.sign[0]*input[rotation.permutation[0]],
          rotation.sign[1]*input[rotation.permutation[1]],
          rotation.sign[2]*input[rotation.permutation[2]]};
}

std::vector<Path> paths(const Vec3& origin,const Rotation& rotation,
                        bool crossed,bool reverse) {
  const Vec3 plus=origin+rotate({0.0,0.0,-0.75},rotation);
  const Vec3 minus=origin+rotate({0.0,0.0,+0.75},rotation);
  const Vec3 x=rotate({0.25,0.0,0.0},rotation);
  const Vec3 y=rotate({0.0,0.25,0.0},rotation);
  std::vector<Path> result{
      {plus+x,plus+(crossed?y*-1.0:y),+1},
      {plus-x,plus+(crossed?y:y*-1.0),+1},
      {minus+x,minus+y,-1},
      {minus-x,minus-y,-1}};
  if(reverse)for(auto& path:result)std::swap(path.start,path.end);
  return result;
}

History history(const std::vector<Path>& paths) {
  History result;
  result.continuity=0.0;result.causal=0.0;result.valid=paths.size()==4;
  for(const auto& path:paths) {
    const auto segment=ftd::eft::make_quadratic_coat_face_current(
        L,path.start,path.end,path.charge,true);
    result.valid=result.valid&&segment.valid;
    result.causal=std::max(result.causal,segment.causal_excess);
    if(!segment.valid)continue;
    for(std::size_t i=0;i<result.rho_before.size();++i) {
      result.rho_before[i]+=segment.rho_before[i];
      result.rho_after[i]+=segment.rho_after[i];
      result.current.x[i]+=segment.current_x[i];
      result.current.y[i]+=segment.current_y[i];
      result.current.z[i]+=segment.current_z[i];
    }
  }
  for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z) {
    const auto i=static_cast<std::size_t>(result.current.index(x,y,z));
    result.continuity=std::max(result.continuity,std::abs(
        result.rho_after[i]-result.rho_before[i]
        +ftd::eft::divergence_at(result.current,x,y,z)));
  }
  return result;
}

double maximum_difference(const std::vector<double>& a,
                          const std::vector<double>& b) {
  if(a.size()!=b.size())return INFINITY;
  double result=0.0;
  for(std::size_t i=0;i<a.size();++i)result=std::max(result,std::abs(a[i]-b[i]));
  return result;
}

template<typename Field>
double field_l2(const Field& field) {
  long double squared=0.0L;
  for(std::size_t i=0;i<field.x.size();++i)squared+=
      static_cast<long double>(field.x[i])*field.x[i]
      +static_cast<long double>(field.y[i])*field.y[i]
      +static_cast<long double>(field.z[i])*field.z[i];
  return std::sqrt(static_cast<double>(squared));
}

template<typename Field>
double field_maximum(const Field& field) {
  double result=0.0;
  for(std::size_t i=0;i<field.x.size();++i)result=std::max({result,
      std::abs(field.x[i]),std::abs(field.y[i]),std::abs(field.z[i])});
  return result;
}

template<typename Field>
double field_difference(const Field& a,const Field& b,double sign=1.0) {
  double result=0.0;
  for(std::size_t i=0;i<a.x.size();++i)result=std::max({result,
      std::abs(a.x[i]-sign*b.x[i]),std::abs(a.y[i]-sign*b.y[i]),
      std::abs(a.z[i]-sign*b.z[i])});
  return result;
}

Metrics evaluate(const Vec3& origin,const Rotation& rotation,bool reverse=false) {
  Metrics result;
  const auto direct=history(paths(origin,rotation,false,reverse));
  const auto crossed=history(paths(origin,rotation,true,reverse));
  result.valid=direct.valid&&crossed.valid;
  result.density_before=maximum_difference(direct.rho_before,crossed.rho_before);
  result.density_after=maximum_difference(direct.rho_after,crossed.rho_after);
  result.continuity_direct=direct.continuity;
  result.continuity_crossed=crossed.continuity;
  result.causal=std::max(direct.causal,crossed.causal);
  Vec3 moment{};
  long double witness=0.0L;
  for(std::size_t i=0;i<result.difference.x.size();++i) {
    result.difference.x[i]=direct.current.x[i]-crossed.current.x[i];
    result.difference.y[i]=direct.current.y[i]-crossed.current.y[i];
    result.difference.z[i]=direct.current.z[i]-crossed.current.z[i];
    moment.x+=result.difference.x[i];moment.y+=result.difference.y[i];
    moment.z+=result.difference.z[i];
    witness+=static_cast<long double>(result.difference.x[i])*result.difference.x[i]
        +static_cast<long double>(result.difference.y[i])*result.difference.y[i]
        +static_cast<long double>(result.difference.z[i])*result.difference.z[i];
  }
  result.divergence=ftd::eft::max_divergence(result.difference);
  result.current_l2=field_l2(result.difference);
  result.current_maximum=field_maximum(result.difference);
  const auto curl=ftd::eft::matched_curl_adjoint(result.difference);
  result.curl_l2=field_l2(curl);
  result.curl_maximum=field_maximum(curl);
  result.moment=moment.mag();
  result.connection_witness=static_cast<double>(witness);
  return result;
}

double metric_difference(const Metrics& a,const Metrics& b) {
  return std::max({std::abs(a.current_l2-b.current_l2),
      std::abs(a.current_maximum-b.current_maximum),
      std::abs(a.curl_l2-b.curl_l2),
      std::abs(a.curl_maximum-b.curl_maximum),
      std::abs(a.connection_witness-b.connection_witness)});
}

void classify(Summary& s) {
  const bool execution=s.construction&&s.density&&s.continuity&&s.causal
      &&s.reversal&&s.cubic&&s.translation;
  if(!execution) {
    s.verdict="POLARITY_SNAPSHOT_CURRENT_WITNESS_EXECUTION_INVALID";
  } else if(s.solenoidal&&s.nonzero&&s.transverse&&s.moment&&s.witness) {
    s.verdict="POLARITY_SNAPSHOT_CURRENT_NONUNIQUENESS_THEOREM_WITNESSED";
  } else {
    s.verdict="POLARITY_SNAPSHOT_DETERMINES_REGISTERED_CURRENT";
  }
}

void write(const Summary& s) {
  const auto directory=std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0719";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory/"ftd_0719_polarity_snapshot_current_nonuniqueness_v1.json");
  json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0719\",\n"
      <<"  \"protocol_sha256\": \""<<snapshot_protocol_sha256<<"\",\n"
      <<"  \"verdict\": \""<<s.verdict<<"\",\n"
      <<"  \"production_changed\": false,\n"
      <<"  \"construction_pass\": "<<s.construction<<",\n"
      <<"  \"density_pass\": "<<s.density<<",\n"
      <<"  \"continuity_pass\": "<<s.continuity<<",\n"
      <<"  \"causal_pass\": "<<s.causal<<",\n"
      <<"  \"solenoidal_pass\": "<<s.solenoidal<<",\n"
      <<"  \"nonzero_pass\": "<<s.nonzero<<",\n"
      <<"  \"transverse_pass\": "<<s.transverse<<",\n"
      <<"  \"moment_pass\": "<<s.moment<<",\n"
      <<"  \"witness_pass\": "<<s.witness<<",\n"
      <<"  \"reversal_pass\": "<<s.reversal<<",\n"
      <<"  \"cubic_pass\": "<<s.cubic<<",\n"
      <<"  \"translation_pass\": "<<s.translation<<",\n"
      <<"  \"rotations\": "<<s.rotations<<",\n"
      <<"  \"density_before_residual\": "<<s.density_before<<",\n"
      <<"  \"density_after_residual\": "<<s.density_after<<",\n"
      <<"  \"continuity_residual\": "<<s.continuity_residual<<",\n"
      <<"  \"causal_excess\": "<<s.causal_excess<<",\n"
      <<"  \"difference_divergence_residual\": "<<s.divergence_residual<<",\n"
      <<"  \"current_difference_l2\": "<<s.current_l2<<",\n"
      <<"  \"current_difference_maximum\": "<<s.current_maximum<<",\n"
      <<"  \"curl_difference_l2\": "<<s.curl_l2<<",\n"
      <<"  \"curl_difference_maximum\": "<<s.curl_maximum<<",\n"
      <<"  \"current_moment_residual\": "<<s.moment_residual<<",\n"
      <<"  \"connection_witness\": "<<s.connection_witness<<",\n"
      <<"  \"reversal_residual\": "<<s.reversal_residual<<",\n"
      <<"  \"cubic_covariance_residual\": "<<s.cubic_covariance_residual<<",\n"
      <<"  \"translation_covariance_residual\": "
      <<s.translation_covariance_residual<<"\n}\n";
  std::ofstream csv(directory/"ftd_0719_polarity_snapshot_current_covariance_v1.csv");
  csv<<"rotation,current_l2,current_maximum,curl_l2,curl_maximum,witness,residual\n"
     <<std::setprecision(17);
  for(const auto& row:s.covariance_rows)csv<<static_cast<int>(row[0])<<','
      <<row[1]<<','<<row[2]<<','<<row[3]<<','<<row[4]<<','<<row[5]<<','<<row[6]<<'\n';
}

} // namespace

int main() {
  Summary s;
  const Vec3 origin{4.0,4.0,4.0};
  const auto group=rotations();
  const auto base=evaluate(origin,group.front());
  s.construction=base.valid&&group.size()==24;
  s.density_before=base.density_before;s.density_after=base.density_after;
  s.continuity_residual=std::max(base.continuity_direct,base.continuity_crossed);
  s.causal_excess=base.causal;s.divergence_residual=base.divergence;
  s.current_l2=base.current_l2;s.current_maximum=base.current_maximum;
  s.curl_l2=base.curl_l2;s.curl_maximum=base.curl_maximum;
  s.moment_residual=base.moment;s.connection_witness=base.connection_witness;
  s.density=s.density_before<=1e-12&&s.density_after<=1e-12;
  s.continuity=s.continuity_residual<=1e-12;s.causal=s.causal_excess<=1e-12;
  s.solenoidal=s.divergence_residual<=1e-12;s.nonzero=s.current_l2>1e-6;
  s.transverse=s.curl_l2>1e-6;s.moment=s.moment_residual<=1e-12;
  s.witness=s.connection_witness>1e-6;
  const auto reversed=evaluate(origin,group.front(),true);
  s.reversal_residual=field_difference(reversed.difference,base.difference,-1.0);
  s.reversal=reversed.valid&&s.reversal_residual<=1e-12;
  s.cubic_covariance_residual=0.0;
  for(std::size_t i=0;i<group.size();++i) {
    const auto rotated=evaluate(origin,group[i]);
    const double residual=metric_difference(rotated,base);
    s.cubic_covariance_residual=std::max(s.cubic_covariance_residual,residual);
    s.covariance_rows.push_back({static_cast<double>(i),rotated.current_l2,
        rotated.current_maximum,rotated.curl_l2,rotated.curl_maximum,
        rotated.connection_witness,residual});
    s.construction=s.construction&&rotated.valid;
  }
  s.rotations=static_cast<int>(s.covariance_rows.size());
  s.cubic=s.rotations==24&&s.cubic_covariance_residual<=1e-12;
  const auto translated=evaluate(origin+Vec3{1.0,-1.0,2.0},group.front());
  s.translation_covariance_residual=metric_difference(translated,base);
  s.translation=translated.valid&&s.translation_covariance_residual<=1e-12;
  classify(s);write(s);
  std::cout<<std::setprecision(17)<<"protocol_sha256="<<snapshot_protocol_sha256
      <<'\n'<<"verdict="<<s.verdict<<'\n'
      <<"density="<<std::max(s.density_before,s.density_after)
      <<" continuity="<<s.continuity_residual
      <<" divergence="<<s.divergence_residual<<'\n'
      <<"current_l2="<<s.current_l2<<" curl_l2="<<s.curl_l2
      <<" moment="<<s.moment_residual<<" witness="<<s.connection_witness<<'\n'
      <<"reverse="<<s.reversal_residual<<" cubic="<<s.cubic_covariance_residual
      <<" translation="<<s.translation_covariance_residual<<'\n';
  return s.verdict=="POLARITY_SNAPSHOT_CURRENT_WITNESS_EXECUTION_INVALID"?1:0;
}
