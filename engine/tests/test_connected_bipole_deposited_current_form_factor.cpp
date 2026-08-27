#include "ftd/eft/matched_face_current_spectrum.h"
#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double speed = 0.5;
constexpr char protocol_sha256[] =
    "D68433E89A6DC20FF8649E72782F00D6FF6A96EC1992CAD5807FC10B2E4B196D";
constexpr char state_sha256[] =
    "8A717BC9DFE3A43FB21A6B46EF723BD2649D5F1F5BC2174BBA6027D25550214F";

struct Point { ftd::Vec3 position{}; int charge=0; int particle=0; };
struct Row {
  int axis=0, sign=0, scale=0;
  double delta=0, k_fraction=0, k_parallel=0, k_perp=0, phase_residual=0;
  double total_power=0, transverse_power=0, longitudinal_power=0;
  double transverse_fraction=0, projection=0;
  ftd::eft::FaceCurrentComplexVector coefficient{};
};

std::vector<std::string> split(const std::string& line) {
  std::vector<std::string> result;
  std::stringstream stream(line);
  std::string item;
  while (std::getline(stream,item,',')) result.push_back(item);
  return result;
}

std::vector<Point> load_points() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_states_v1.csv";
  std::ifstream input(path);
  std::string line;
  std::getline(input,line);
  std::vector<Point> result;
  while (std::getline(input,line)) {
    const auto fields=split(line);
    if (fields.size()!=10 || fields[1]!="0") continue;
    result.push_back({{std::stod(fields[7]),std::stod(fields[8]),
                       std::stod(fields[9])},
                      std::stoi(fields[3]),std::stoi(fields[2])});
  }
  std::sort(result.begin(),result.end(),[](const Point&a,const Point&b){
    return a.particle<b.particle;
  });
  return result;
}

ftd::Vec3 cycle(ftd::Vec3 value) { return {value.z,value.x,value.y}; }
std::array<double,3> cycle(std::array<double,3> value) {
  return {{value[2],value[0],value[1]}};
}

std::vector<Point> rotate(std::vector<Point> points,int axis) {
  for (int turn=0;turn<axis;++turn)
    for (auto& point:points) point.position=cycle(point.position);
  return points;
}

double omega(const std::array<double,3>& k) {
  double s=0;
  for (double component:k) s+=std::sin(component/2)*std::sin(component/2);
  return 2*std::asin(std::sqrt(s/3));
}

double complex_residual(const ftd::eft::FaceCurrentComplexVector&a,
                        const ftd::eft::FaceCurrentComplexVector&b,
                        bool opposite=false) {
  double result=0;
  for(int component=0;component<3;++component)
    result=std::max(result,std::abs(a[component]+(opposite?1.0:-1.0)*b[component]));
  return result;
}

const Row* find(const std::vector<Row>&rows,int axis,int sign,int scale,double f){
  for(const auto&row:rows)if(row.axis==axis&&row.sign==sign&&row.scale==scale
      &&std::abs(row.k_fraction-f)<1e-12)return &row;
  return nullptr;
}

}  // namespace

int main(){
  const auto source=load_points();
  bool execution=source.size()==16;
  int net_charge=0;
  for(const auto&p:source)net_charge+=p.charge;
  execution=execution&&net_charge==0;
  const std::array<double,8> fractions{{2.0/3.0,.70,.75,.80,.85,.90,.95,1.0}};
  const std::array<double,2> deltas{{5e-7,1e-6}};
  std::vector<Row> rows;
  double worst_continuity=0,worst_moment=0,worst_phase=0,worst_projection=0;
  for(int axis=0;axis<3;++axis){
    const auto points=rotate(source,axis);
    ftd::Vec3 direction{1,0,0};for(int turn=0;turn<axis;++turn)direction=cycle(direction);
    for(int sign:{-1,1})for(int scale=0;scale<2;++scale){
      const double delta=deltas[scale];
      std::vector<ftd::eft::QuadraticCoatSparseCurrentEntry> entries;
      for(const auto&p:points){
        const auto segment=ftd::eft::make_quadratic_coat_face_current(
            L,p.position,p.position+direction*(sign*delta),p.charge,false);
        execution=execution&&segment.valid;
        worst_continuity=std::max(worst_continuity,segment.continuity_residual);
        worst_moment=std::max(worst_moment,segment.current_moment_residual);
        entries.insert(entries.end(),segment.sparse_current.begin(),segment.sparse_current.end());
      }
      for(double fraction:fractions){
        const double kp=fraction*ftd::PI;
        double sy=3*std::sin(kp/4)*std::sin(kp/4)
            -std::sin(kp/2)*std::sin(kp/2);
        sy=std::max(0.0,std::min(1.0,sy));
        const double kt=2*std::asin(std::sqrt(sy));
        std::array<double,3> k{{kp,kt,0}};
        for(int turn=0;turn<axis;++turn)k=cycle(k);
        const auto observed=ftd::eft::observe_sparse_face_current_spectrum(
            L,entries,k,16*delta);
        Row row;row.axis=axis;row.sign=sign;row.scale=scale;row.delta=delta;
        row.k_fraction=fraction;row.k_parallel=kp;row.k_perp=kt;
        row.phase_residual=std::abs(omega(k)-speed*kp);
        row.total_power=observed.total_power;
        row.transverse_power=observed.transverse_power;
        row.longitudinal_power=observed.longitudinal_power;
        row.transverse_fraction=observed.transverse_fraction;
        row.projection=observed.projection_residual;
        row.coefficient=observed.current;
        worst_phase=std::max(worst_phase,row.phase_residual);
        worst_projection=std::max(worst_projection,row.projection);
        execution=execution&&observed.valid;
        rows.push_back(row);
      }
    }
  }
  execution=execution&&rows.size()==96&&worst_continuity<=1e-12
      &&worst_moment<=1e-12&&worst_phase<=2e-15&&worst_projection<=1e-14;
  double worst_mirror=0,worst_scale=0,worst_cubic=0;
  for(int axis=0;axis<3;++axis)for(int scale=0;scale<2;++scale)
    for(double fraction:fractions){
      const auto*p=find(rows,axis,1,scale,fraction),*n=find(rows,axis,-1,scale,fraction);
      if(!p||!n){execution=false;continue;}
      worst_mirror=std::max(worst_mirror,complex_residual(p->coefficient,n->coefficient,true));
    }
  for(int axis=0;axis<3;++axis)for(int sign:{-1,1})for(double fraction:fractions){
    const auto*a=find(rows,axis,sign,0,fraction),*b=find(rows,axis,sign,1,fraction);
    if(!a||!b){execution=false;continue;}
    worst_scale=std::max(worst_scale,complex_residual(a->coefficient,b->coefficient));
  }
  for(int axis=1;axis<3;++axis)for(int sign:{-1,1})for(int scale=0;scale<2;++scale)
    for(double fraction:fractions){
      const auto*base=find(rows,0,sign,scale,fraction),*rot=find(rows,axis,sign,scale,fraction);
      if(!base||!rot){execution=false;continue;}
      auto expected=base->coefficient;
      for(int turn=0;turn<axis;++turn){
        const auto prior=expected;expected={{prior[2],prior[0],prior[1]}};
      }
      worst_cubic=std::max(worst_cubic,complex_residual(rot->coefficient,expected));
      worst_cubic=std::max({worst_cubic,std::abs(rot->total_power-base->total_power),
          std::abs(rot->transverse_power-base->transverse_power)});
    }
  execution=execution&&worst_mirror<=5e-6&&worst_scale<=5e-6&&worst_cubic<=2e-12;
  double edge_power=0,offedge_power=INFINITY,interior_max=0;
  double collinear_fraction=0,edge_fraction_residual=0;
  for(const auto&row:rows)if(row.sign==1&&row.scale==1){
    if(std::abs(row.k_fraction-2.0/3.0)<1e-12)
      collinear_fraction=std::max(collinear_fraction,row.transverse_fraction);
    if(std::abs(row.k_fraction-1.0)<1e-12){
      edge_power=std::max(edge_power,row.total_power);
      edge_fraction_residual=std::max(edge_fraction_residual,
          std::abs(row.transverse_fraction-1.0/3.0));
    } else interior_max=std::max(interior_max,row.transverse_power);
    if(std::abs(row.k_fraction-.9)<1e-12)
      offedge_power=std::min(offedge_power,row.transverse_power);
  }
  const bool collinear=collinear_fraction<=1e-24;
  const bool edge=edge_power<=1e-7&&edge_fraction_residual<=1e-12;
  const bool offedge=offedge_power>=1e-5;
  const bool contrast=edge_power>0?interior_max/edge_power>=100:interior_max>0;
  std::string verdict="DEPOSITED_CURRENT_FORM_FACTOR_EXECUTION_INVALID";
  if(execution){
    if(edge&&offedge&&contrast&&collinear)verdict="DEPOSITED_CURRENT_EDGE_SCREENING_PARTIAL";
    else if(edge&&interior_max<1e-5)verdict="DEPOSITED_CURRENT_COMPLETE_SCREENING_CANDIDATE";
    else verdict="DEPOSITED_CURRENT_EDGE_SCREENING_CLOSED";
  }
  const auto outdir=std::filesystem::path(__FILE__).parent_path().parent_path()/"results/ftd_0703";
  std::filesystem::create_directories(outdir);
  std::ofstream csv(outdir/"ftd_0703_connected_bipole_deposited_current_form_factor_v1.csv");
  csv<<"ftd_id,axis,sign,scale,delta,k_fraction,k_parallel,k_perp,phase_residual,total_power,transverse_power,longitudinal_power,transverse_fraction,projection,jx_re,jx_im,jy_re,jy_im,jz_re,jz_im\n";
  for(const auto&r:rows)csv<<std::setprecision(17)<<"FTD-0703,"<<r.axis<<','<<r.sign<<','<<r.scale<<','<<r.delta<<','<<r.k_fraction<<','<<r.k_parallel<<','<<r.k_perp<<','<<r.phase_residual<<','<<r.total_power<<','<<r.transverse_power<<','<<r.longitudinal_power<<','<<r.transverse_fraction<<','<<r.projection<<','<<r.coefficient[0].real()<<','<<r.coefficient[0].imag()<<','<<r.coefficient[1].real()<<','<<r.coefficient[1].imag()<<','<<r.coefficient[2].real()<<','<<r.coefficient[2].imag()<<'\n';
  std::ofstream json(outdir/"ftd_0703_connected_bipole_deposited_current_form_factor_v1.json");
  json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0703\",\n  \"protocol_sha256\": \""<<protocol_sha256<<"\",\n  \"state_sha256\": \""<<state_sha256<<"\",\n  \"verdict\": \""<<verdict<<"\",\n  \"production_changed\": false,\n  \"row_count\": "<<rows.size()<<",\n  \"worst_continuity\": "<<worst_continuity<<",\n  \"worst_current_moment\": "<<worst_moment<<",\n  \"worst_phase_residual\": "<<worst_phase<<",\n  \"worst_projection\": "<<worst_projection<<",\n  \"worst_mirror\": "<<worst_mirror<<",\n  \"worst_scale\": "<<worst_scale<<",\n  \"worst_cubic\": "<<worst_cubic<<",\n  \"collinear_transverse_fraction\": "<<collinear_fraction<<",\n  \"edge_total_power\": "<<edge_power<<",\n  \"edge_fraction_residual\": "<<edge_fraction_residual<<",\n  \"offedge_transverse_power\": "<<offedge_power<<",\n  \"maximum_interior_transverse_power\": "<<interior_max<<",\n  \"interior_edge_contrast\": "<<(edge_power>0?interior_max/edge_power:INFINITY)<<"\n}\n";
  std::cout<<std::setprecision(17)<<"protocol_sha256="<<protocol_sha256<<'\n'<<"verdict="<<verdict<<'\n'<<"rows="<<rows.size()<<" continuity="<<worst_continuity<<" moment="<<worst_moment<<" phase="<<worst_phase<<" projection="<<worst_projection<<'\n'<<"mirror="<<worst_mirror<<" scale="<<worst_scale<<" cubic="<<worst_cubic<<'\n'<<"collinear="<<collinear_fraction<<" edge="<<edge_power<<" offedge="<<offedge_power<<" interior="<<interior_max<<" contrast="<<(edge_power>0?interior_max/edge_power:INFINITY)<<'\n';
  return verdict=="DEPOSITED_CURRENT_FORM_FACTOR_EXECUTION_INVALID"?1:0;
}
