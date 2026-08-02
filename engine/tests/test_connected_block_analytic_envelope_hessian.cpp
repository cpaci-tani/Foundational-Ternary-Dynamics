// FTD-0637: analytic envelope gradient/Hessian of the frozen dressed block.

#include "ftd/eft/connected_moore_block_action.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace {
using ftd::Vec3;
using Theta = std::array<double,4>;
using Matrix = std::vector<std::vector<double>>;

constexpr char protocol_sha256[] =
    "1BF4C901DA20669248C2790F49F9EF9D488C8D2C67C6EA6B4E89ADC9493178F1";
constexpr char parent_protocol_sha256[] =
    "97BEAB915084AC72D0F1D45992320750328C89DBA1EBFBD13D9A0A977631C24A";
constexpr char parent_result_sha256[] =
    "9A009218E5DA24E5C461A7737E4BAFD307CDA29D8FB5530FC9C2768008A7F176";
constexpr char parent_matrix_sha256[] =
    "EF446FE2724A513F96CDEBA6515FFADCACA97FE82C6AB926B042E21B3A6ECECB";
constexpr int L = 17;
constexpr int count = 16;
constexpr int N = 3*count;
constexpr std::size_t V = static_cast<std::size_t>(L)*L*L;
constexpr double gradient_h = 4e-6;
constexpr Theta theta_x{{1.4992742199186664,0.49947120868980366,
                         0.50009465475929205,0.50018755308199814}};
constexpr Theta theta_y{{1.4992742199191138,0.49947120868992617,
                         0.50009465475922343,0.5001875530819222}};

struct Basis { double value=0.0, first=0.0, second=0.0; };
struct Poisson { bool valid=false; int iterations=0; double residual=INFINITY; std::vector<double> potential; };
struct Deposit {
  bool valid=false;
  double charge_residual=INFINITY;
  double derivative_charge_residual=INFINITY;
  double derivative_moment_residual=INFINITY;
  std::vector<double> rho;
  std::vector<std::vector<double>> first;
};
struct Spectrum { bool valid=false; double residual=INFINITY,orthogonality=INFINITY; std::vector<double> values; };
struct Arm {
  std::string label;
  int orientation=0;
  bool valid=false,stationary=false,positive=false;
  int maximum_poisson_iterations=0;
  double minimum_knot_clearance=INFINITY;
  double maximum_poisson_residual=0.0;
  double charge_residual=INFINITY;
  double derivative_charge_residual=INFINITY;
  double derivative_moment_residual=INFINITY;
  double energy_identity_residual=INFINITY;
  double gradient_inf=INFINITY;
  double gradient_comparison=INFINITY;
  double hessian_comparison=INFINITY;
  double antisymmetry=INFINITY;
  double translation_identity=INFINITY;
  double min_eigen=INFINITY,max_eigen=INFINITY;
  double eigen_residual=INFINITY,orthogonality=INFINITY;
  std::vector<double> gradient,eigenvalues;
  Matrix hessian;
};
struct Summary {
  bool parent=false,normalization=false,covariance=false;
  double beta=0.0,covariance_residual=INFINITY;
  std::string verdict="CONNECTED_BLOCK_ANALYTIC_HESSIAN_EXECUTION_INVALID";
  std::vector<Arm> arms;
};

double component(const Vec3& v,int a) { return a==0?v.x:(a==1?v.y:v.z); }
void set_component(Vec3& v,int a,double x) { if(a==0)v.x=x;else if(a==1)v.y=x;else v.z=x; }
int wrap(int x) { const int r=x%L; return r<0?r+L:r; }
std::size_t index(int x,int y,int z) { return static_cast<std::size_t>((wrap(x)*L+wrap(y))*L+wrap(z)); }
Vec3 position(const ftd::eft::MatchedMatterPoint& p) { return {p.anchor.x+p.remainder.x,p.anchor.y+p.remainder.y,p.anchor.z+p.remainder.z}; }
Vec3 center(const ftd::eft::ConnectedMooreBlockState& s) { Vec3 c{};for(const auto&p:s.constituents)c+=position(p);return c*(1.0/s.constituents.size()); }
ftd::eft::MatchedMatterPoint point_at(const Vec3& x) {
  ftd::eft::MatchedMatterPoint p;
  const long long ax=std::llround(x.x),ay=std::llround(x.y),az=std::llround(x.z);
  p.anchor={wrap(static_cast<int>(ax)),wrap(static_cast<int>(ay)),wrap(static_cast<int>(az))};
  p.remainder={x.x-ax,x.y-ay,x.z-az};
  return p;
}
ftd::eft::ConnectedMooreBlockState geometry_from(
    const ftd::eft::ConnectedMooreBlockState& base,const Theta& t,int orientation) {
  auto result=base;const Vec3 c=center(base);Vec3 shift{};
  for(int axis=0;axis<3;++axis)if(axis!=orientation)set_component(shift,axis,.5);
  for(std::size_t k=0;k<result.constituents.size();++k) {
    const Vec3 d0=position(base.constituents[k])-c;
    const bool outer=std::abs(component(d0,orientation))>1;
    Vec3 d{};
    set_component(d,orientation,std::copysign(outer?t[0]:t[1],component(d0,orientation)));
    const double q=outer?t[2]:t[3];
    for(int axis=0;axis<3;++axis)if(axis!=orientation)set_component(d,axis,std::copysign(q,component(d0,axis)));
    result.constituents[k]=point_at(c+shift+d);
  }
  return result;
}
ftd::eft::ConnectedMooreBlockState displace(
    const ftd::eft::ConnectedMooreBlockState& state,int coordinate,double amount) {
  auto result=state;const int particle=coordinate/3,axis=coordinate%3;
  Vec3 x=position(result.constituents[particle]);
  set_component(x,axis,component(x,axis)+amount);
  result.constituents[particle]=point_at(x);
  return result;
}

Basis basis(double u) {
  const double r=std::abs(u);
  if(r<.5) return {.75-u*u,-2*u,-2};
  if(r<1.5) {
    const double tail=1.5-r,sign=u<0?-1.0:1.0;
    return {.5*tail*tail,-tail*sign,1};
  }
  return {};
}
double dot(const std::vector<double>& a,const std::vector<double>& b) {
  long double result=0;for(std::size_t i=0;i<a.size();++i)result+=static_cast<long double>(a[i])*b[i];return static_cast<double>(result);
}
void negative_laplacian(const std::vector<double>& input,std::vector<double>& output) {
  for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z) {
    const auto i=index(x,y,z);
    output[i]=6*input[i]-input[index(x+1,y,z)]-input[index(x-1,y,z)]
        -input[index(x,y+1,z)]-input[index(x,y-1,z)]
        -input[index(x,y,z+1)]-input[index(x,y,z-1)];
  }
}
Poisson solve_poisson(const std::vector<double>& source) {
  Poisson result;result.potential.assign(V,0.0);
  std::vector<double> rhs=source,residual(V),direction(V),image(V);
  const long double mean=std::accumulate(rhs.begin(),rhs.end(),0.0L)/V;
  for(double& value:rhs)value-=static_cast<double>(mean);
  residual=direction=rhs;long double rr=dot(residual,residual);
  for(int iteration=1;iteration<=4096;++iteration) {
    result.residual=0;for(double value:residual)result.residual=std::max(result.residual,std::abs(value));
    if(result.residual<=1e-13){result.iterations=iteration-1;break;}
    negative_laplacian(direction,image);const long double denominator=dot(direction,image);
    if(!(denominator>0))break;const long double alpha=rr/denominator;
    for(std::size_t i=0;i<V;++i){result.potential[i]+=static_cast<double>(alpha*direction[i]);residual[i]-=static_cast<double>(alpha*image[i]);}
    const long double next=dot(residual,residual),ratio=next/rr;
    for(std::size_t i=0;i<V;++i)direction[i]=residual[i]+static_cast<double>(ratio*direction[i]);
    rr=next;result.iterations=iteration;
  }
  std::vector<double> image_final(V);negative_laplacian(result.potential,image_final);
  result.residual=0;for(std::size_t i=0;i<V;++i)result.residual=std::max(result.residual,std::abs(image_final[i]-rhs[i]));
  result.valid=result.residual<=1e-13;return result;
}

Deposit deposit(const ftd::eft::ConnectedMooreBlockState& state) {
  Deposit result;result.rho.assign(V,0.0);result.first.assign(N,std::vector<double>(V,0.0));
  for(int p=0;p<count;++p) {
    const Vec3 xp=position(state.constituents[p]);const int q=state.charges[p];
    const int lo[3]={static_cast<int>(std::floor(xp.x))-2,static_cast<int>(std::floor(xp.y))-2,static_cast<int>(std::floor(xp.z))-2};
    for(int x=lo[0];x<=lo[0]+4;++x)for(int y=lo[1];y<=lo[1]+4;++y)for(int z=lo[2];z<=lo[2]+4;++z) {
      const int site[3]={x,y,z};Basis b[3];for(int a=0;a<3;++a)b[a]=basis(component(xp,a)-site[a]);
      const auto k=index(x,y,z);result.rho[k]+=q*b[0].value*b[1].value*b[2].value;
      for(int a=0;a<3;++a){double v=q*b[a].first;for(int c=0;c<3;++c)if(c!=a)v*=b[c].value;result.first[3*p+a][k]+=v;}
    }
  }
  result.charge_residual=std::abs(std::accumulate(result.rho.begin(),result.rho.end(),0.0));
  result.derivative_charge_residual=0;result.derivative_moment_residual=0;
  for(int c=0;c<N;++c) {
    result.derivative_charge_residual=std::max(result.derivative_charge_residual,std::abs(std::accumulate(result.first[c].begin(),result.first[c].end(),0.0)));
    const int particle=c/3,axis=c%3;
    for(int moment_axis=0;moment_axis<3;++moment_axis){long double moment=0;for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z){const int site[3]={x,y,z};moment+=static_cast<long double>(site[moment_axis])*result.first[c][index(x,y,z)];}const double target=moment_axis==axis?state.charges[particle]:0;result.derivative_moment_residual=std::max(result.derivative_moment_residual,std::abs(static_cast<double>(moment)-target));}
  }
  result.valid=result.charge_residual<=1e-12&&result.derivative_charge_residual<=1e-12&&result.derivative_moment_residual<=1e-12;return result;
}

double field_curvature_term(const ftd::eft::ConnectedMooreBlockState& state,
                            const std::vector<double>& phi,int ci,int cj) {
  if(ci/3!=cj/3)return 0;const int p=ci/3,a=ci%3,baxis=cj%3;
  const Vec3 xp=position(state.constituents[p]);const int q=state.charges[p];
  const int lo[3]={static_cast<int>(std::floor(xp.x))-2,static_cast<int>(std::floor(xp.y))-2,static_cast<int>(std::floor(xp.z))-2};
  long double result=0;
  for(int x=lo[0];x<=lo[0]+4;++x)for(int y=lo[1];y<=lo[1]+4;++y)for(int z=lo[2];z<=lo[2]+4;++z){const int site[3]={x,y,z};Basis bs[3];for(int k=0;k<3;++k)bs[k]=basis(component(xp,k)-site[k]);double value=q;if(a==baxis){value*=bs[a].second;for(int k=0;k<3;++k)if(k!=a)value*=bs[k].value;}else{value*=bs[a].first*bs[baxis].first;for(int k=0;k<3;++k)if(k!=a&&k!=baxis)value*=bs[k].value;}result+=static_cast<long double>(phi[index(x,y,z)])*value;}
  return static_cast<double>(result);
}
void add_binding(const ftd::eft::ConnectedMooreBlockState& state,
                 std::vector<double>& gradient,Matrix& hessian) {
  for(const auto& edge:state.edges){const Vec3 d=position(state.constituents[edge.first])-position(state.constituents[edge.second]);const double u=d.dot(d)-edge.rest_length_squared;for(int a=0;a<3;++a){const double ga=u*component(d,a);gradient[3*edge.first+a]+=ga;gradient[3*edge.second+a]-=ga;for(int b=0;b<3;++b){const double block=2*component(d,a)*component(d,b)+(a==b?u:0);const int ia=3*static_cast<int>(edge.first)+a,ja=3*static_cast<int>(edge.second)+a,ib=3*static_cast<int>(edge.first)+b,jb=3*static_cast<int>(edge.second)+b;hessian[ia][ib]+=block;hessian[ja][jb]+=block;hessian[ia][jb]-=block;hessian[ja][ib]-=block;}}}
}
double energy(const ftd::eft::ConnectedMooreBlockState& geometry,double beta,
              const ftd::eft::ConnectedMooreBlockOptions& options,bool* valid=nullptr) {
  const auto dressed=ftd::eft::redress_connected_moore_block_with_fibre_limit(geometry,8,1e-13,4096);
  if(valid)*valid=dressed.valid;if(!dressed.valid)return INFINITY;
  return ftd::eft::connected_moore_block_binding_energy(dressed.state,options)
      +beta*ftd::eft::matched_modified_energy(dressed.state.electric,dressed.state.magnetic_half,ftd::C_SPEED);
}
Spectrum diagonalize(const Matrix& input) {
  Spectrum r;Matrix a=input,v(N,std::vector<double>(N));for(int i=0;i<N;++i)v[i][i]=1;
  for(int iteration=0;iteration<200000;++iteration){int p=0,q=1;double largest=std::abs(a[p][q]);for(int i=0;i<N;++i)for(int j=i+1;j<N;++j)if(std::abs(a[i][j])>largest){largest=std::abs(a[i][j]);p=i;q=j;}if(largest<1e-11)break;const double angle=.5*std::atan2(2*a[p][q],a[q][q]-a[p][p]),c=std::cos(angle),s=std::sin(angle);for(int k=0;k<N;++k)if(k!=p&&k!=q){const double kp=a[k][p],kq=a[k][q];a[k][p]=a[p][k]=c*kp-s*kq;a[k][q]=a[q][k]=s*kp+c*kq;}const double pp=a[p][p],qq=a[q][q],pq=a[p][q];a[p][p]=c*c*pp-2*c*s*pq+s*s*qq;a[q][q]=s*s*pp+2*c*s*pq+c*c*qq;a[p][q]=a[q][p]=0;for(int k=0;k<N;++k){const double kp=v[k][p],kq=v[k][q];v[k][p]=c*kp-s*kq;v[k][q]=s*kp+c*kq;}}
  std::vector<int> order(N);std::iota(order.begin(),order.end(),0);std::sort(order.begin(),order.end(),[&](int x,int y){return a[x][x]<a[y][y];});r.values.resize(N);r.residual=0;r.orthogonality=0;for(int k=0;k<N;++k){const int source=order[k];r.values[k]=a[source][source];for(int i=0;i<N;++i){long double hv=0;for(int j=0;j<N;++j)hv+=static_cast<long double>(input[i][j])*v[j][source];r.residual=std::max(r.residual,std::abs(static_cast<double>(hv)-r.values[k]*v[i][source]));}for(int l=0;l<N;++l){long double product=0;for(int i=0;i<N;++i)product+=static_cast<long double>(v[i][source])*v[i][order[l]];r.orthogonality=std::max(r.orthogonality,std::abs(static_cast<double>(product)-(k==l?1.0:0.0)));}}
  r.valid=r.residual<=1e-7&&r.orthogonality<=1e-10;return r;
}
Matrix read_parent_hessian(const std::string& label) {
  const auto path=std::filesystem::path(__FILE__).parent_path().parent_path()/"results/ftd_0636/ftd_0636_connected_block_knot_local_hessian_v1.csv";
  std::ifstream input(path);std::string line;std::getline(input,line);Matrix result(N,std::vector<double>(N,NAN));
  while(std::getline(input,line)){std::stringstream row(line);std::string id,name,r,c,value;std::getline(row,id,',');std::getline(row,name,',');std::getline(row,r,',');std::getline(row,c,',');std::getline(row,value,',');if(name==label)result[std::stoi(r)][std::stoi(c)]=std::stod(value);}
  return result;
}
bool parent_fingerprint() {
  const auto root=std::filesystem::path(__FILE__).parent_path().parent_path();
  std::ifstream input(root/"results/ftd_0636/ftd_0636_connected_block_knot_local_hessian_v1.json",std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  return bytes.find(parent_protocol_sha256)!=std::string::npos
      && bytes.find("CONNECTED_BLOCK_KNOT_LOCAL_HESSIAN_EXECUTION_INVALID")!=std::string::npos;
}

Arm run_arm(const std::string& label,int orientation,const Theta& theta,double beta,
            const ftd::eft::ConnectedMooreBlockOptions& options) {
  Arm arm;arm.label=label;arm.orientation=orientation;
  const auto initial=ftd::eft::initialize_connected_moore_block(L,2,orientation,orientation,.5,1e-13,4096);
  if(!initial.valid)return arm;const auto state=geometry_from(initial.state,theta,orientation);
  for(const auto& p:state.constituents){const Vec3 x=position(p);for(int a=0;a<3;++a){const double f=component(x,a)-std::floor(component(x,a));arm.minimum_knot_clearance=std::min(arm.minimum_knot_clearance,std::abs(f-.5));}}
  const auto d=deposit(state);arm.charge_residual=d.charge_residual;arm.derivative_charge_residual=d.derivative_charge_residual;arm.derivative_moment_residual=d.derivative_moment_residual;if(!d.valid)return arm;
  const auto phi=solve_poisson(d.rho);arm.maximum_poisson_iterations=phi.iterations;arm.maximum_poisson_residual=phi.residual;if(!phi.valid)return arm;
  std::vector<std::vector<double>> response(N);
  for(int j=0;j<N;++j){auto p=solve_poisson(d.first[j]);arm.maximum_poisson_iterations=std::max(arm.maximum_poisson_iterations,p.iterations);arm.maximum_poisson_residual=std::max(arm.maximum_poisson_residual,p.residual);if(!p.valid)return arm;response[j]=std::move(p.potential);}
  arm.gradient.assign(N,0);arm.hessian.assign(N,std::vector<double>(N,0));
  for(int i=0;i<N;++i){arm.gradient[i]=beta*dot(d.first[i],phi.potential);for(int j=0;j<N;++j)arm.hessian[i][j]=beta*(dot(d.first[i],response[j])+field_curvature_term(state,phi.potential,i,j));}
  add_binding(state,arm.gradient,arm.hessian);
  arm.gradient_inf=0;arm.antisymmetry=0;for(int i=0;i<N;++i){arm.gradient_inf=std::max(arm.gradient_inf,std::abs(arm.gradient[i]));for(int j=0;j<N;++j)arm.antisymmetry=std::max(arm.antisymmetry,std::abs(arm.hessian[i][j]-arm.hessian[j][i]));}
  bool base_valid=false;const double measured_energy=energy(state,beta,options,&base_valid);const double analytic_energy=ftd::eft::connected_moore_block_binding_energy(state,options)+.5*beta*dot(d.rho,phi.potential);arm.energy_identity_residual=std::abs(measured_energy-analytic_energy);
  arm.gradient_comparison=0;for(int i=0;i<N;++i){bool vp=false,vm=false;const double ep=energy(displace(state,i,gradient_h),beta,options,&vp),em=energy(displace(state,i,-gradient_h),beta,options,&vm);if(!vp||!vm)return arm;const double fd=(ep-em)/(2*gradient_h);arm.gradient_comparison=std::max(arm.gradient_comparison,std::abs(fd-arm.gradient[i]));}
  const Matrix parent=read_parent_hessian(label);arm.hessian_comparison=0;for(int i=0;i<N;++i)for(int j=0;j<N;++j){if(!std::isfinite(parent[i][j]))return arm;arm.hessian_comparison=std::max(arm.hessian_comparison,std::abs(parent[i][j]-arm.hessian[i][j]));}
  arm.translation_identity=0;for(int axis=0;axis<3;++axis){std::vector<double> aggregate(V,0),v(N,0);for(int p=0;p<count;++p){v[3*p+axis]=.25;for(std::size_t k=0;k<V;++k)aggregate[k]+=d.first[3*p+axis][k];}const auto response_t=solve_poisson(aggregate);if(!response_t.valid)return arm;double direct=beta*dot(aggregate,response_t.potential);for(int p=0;p<count;++p)direct+=beta*field_curvature_term(state,phi.potential,3*p+axis,3*p+axis);long double rayleigh=0;for(int i=0;i<N;++i)for(int j=0;j<N;++j)rayleigh+=static_cast<long double>(v[i])*arm.hessian[i][j]*v[j];arm.translation_identity=std::max(arm.translation_identity,std::abs(static_cast<double>(rayleigh)-direct/16));}
  const auto spectrum=diagonalize(arm.hessian);arm.eigenvalues=spectrum.values;arm.eigen_residual=spectrum.residual;arm.orthogonality=spectrum.orthogonality;if(!spectrum.values.empty()){arm.min_eigen=spectrum.values.front();arm.max_eigen=spectrum.values.back();}
  arm.stationary=arm.gradient_inf<=1e-8;arm.positive=arm.min_eigen>1e-5;
  arm.valid=base_valid&&arm.minimum_knot_clearance>0&&arm.maximum_poisson_residual<=1e-13&&arm.energy_identity_residual<=1e-11&&arm.gradient_comparison<=5e-8&&arm.hessian_comparison<=5e-4&&arm.antisymmetry<=1e-12&&arm.translation_identity<=1e-12&&spectrum.valid;
  return arm;
}
void write(const Summary& s) {
  const auto dir=std::filesystem::path(__FILE__).parent_path().parent_path()/"results/ftd_0637";std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0637_connected_block_analytic_envelope_hessian_v1.json");json<<std::setprecision(17)<<"{\n  \"ftd_id\": \"FTD-0637\",\n  \"protocol_sha256\": \""<<protocol_sha256<<"\",\n  \"parent_result_sha256\": \""<<parent_result_sha256<<"\",\n  \"parent_matrix_sha256\": \""<<parent_matrix_sha256<<"\",\n  \"verdict\": \""<<s.verdict<<"\",\n  \"production_changed\": false,\n  \"covariance_residual\": "<<s.covariance_residual<<"\n}\n";
  std::ofstream arms(dir/"ftd_0637_connected_block_analytic_envelope_hessian_arms_v1.csv");arms<<"ftd_id,label,valid,stationary,positive,knot_clearance,poisson_iterations,poisson_residual,charge_residual,derivative_charge_residual,derivative_moment_residual,energy_identity_residual,gradient_inf,gradient_comparison,hessian_comparison,antisymmetry,translation_identity,min_eigen,max_eigen,eigen_residual,orthogonality\n";for(const auto&a:s.arms)arms<<std::setprecision(17)<<"FTD-0637,"<<a.label<<','<<a.valid<<','<<a.stationary<<','<<a.positive<<','<<a.minimum_knot_clearance<<','<<a.maximum_poisson_iterations<<','<<a.maximum_poisson_residual<<','<<a.charge_residual<<','<<a.derivative_charge_residual<<','<<a.derivative_moment_residual<<','<<a.energy_identity_residual<<','<<a.gradient_inf<<','<<a.gradient_comparison<<','<<a.hessian_comparison<<','<<a.antisymmetry<<','<<a.translation_identity<<','<<a.min_eigen<<','<<a.max_eigen<<','<<a.eigen_residual<<','<<a.orthogonality<<'\n';
  std::ofstream gradient(dir/"ftd_0637_connected_block_analytic_envelope_gradient_v1.csv");gradient<<"ftd_id,label,coordinate,value\n";for(const auto&a:s.arms)for(int i=0;i<N;++i)gradient<<std::setprecision(17)<<"FTD-0637,"<<a.label<<','<<i<<','<<a.gradient[i]<<'\n';
  std::ofstream hessian(dir/"ftd_0637_connected_block_analytic_envelope_hessian_v1.csv");hessian<<"ftd_id,label,row,col,value\n";for(const auto&a:s.arms)for(int i=0;i<N;++i)for(int j=0;j<N;++j)hessian<<std::setprecision(17)<<"FTD-0637,"<<a.label<<','<<i<<','<<j<<','<<a.hessian[i][j]<<'\n';
  std::ofstream eigen(dir/"ftd_0637_connected_block_analytic_envelope_eigenvalues_v1.csv");eigen<<"ftd_id,label,index,eigenvalue\n";for(const auto&a:s.arms)for(int i=0;i<N;++i)eigen<<std::setprecision(17)<<"FTD-0637,"<<a.label<<','<<i<<','<<a.eigenvalues[i]<<'\n';
}
}

int main() {
  Summary summary;summary.parent=parent_fingerprint();const auto normalization=ftd::eft::measure_face_flux_normalization();summary.normalization=normalization.valid;summary.beta=normalization.mapped_field_work_coefficient;ftd::eft::ConnectedMooreBlockOptions options;options.allow_shared_anchor_chart=true;
  if(summary.parent&&summary.normalization){summary.arms.push_back(run_arm("hessian_x",0,theta_x,summary.beta,options));std::cout<<"completed analytic_x\n";summary.arms.push_back(run_arm("hessian_y",1,theta_y,summary.beta,options));std::cout<<"completed analytic_y\n";}
  const bool coverage=summary.arms.size()==2&&std::all_of(summary.arms.begin(),summary.arms.end(),[](const Arm&a){return a.valid;});if(summary.arms.size()==2){summary.covariance_residual=0;for(int i=0;i<N;++i)summary.covariance_residual=std::max(summary.covariance_residual,std::abs(summary.arms[0].eigenvalues[i]-summary.arms[1].eigenvalues[i])/std::max({1.0,std::abs(summary.arms[0].eigenvalues[i]),std::abs(summary.arms[1].eigenvalues[i])}));summary.covariance=summary.covariance_residual<=1e-6;}
  if(coverage&&summary.covariance){const bool stationary=std::all_of(summary.arms.begin(),summary.arms.end(),[](const Arm&a){return a.stationary;});const bool positive=std::all_of(summary.arms.begin(),summary.arms.end(),[](const Arm&a){return a.positive;});const bool negative=std::any_of(summary.arms.begin(),summary.arms.end(),[](const Arm&a){return a.min_eigen<-1e-5;});if(!stationary)summary.verdict="CONNECTED_BLOCK_ANALYTIC_NONSTATIONARY";else if(positive)summary.verdict="CONNECTED_BLOCK_ANALYTIC_48D_BASIN_CONSTRUCTIVE";else if(negative)summary.verdict="CONNECTED_BLOCK_ANALYTIC_FALSE_MINIMUM";else summary.verdict="CONNECTED_BLOCK_ANALYTIC_HESSIAN_MARGINAL";}
  write(summary);std::cout<<std::setprecision(17)<<"protocol_sha256="<<protocol_sha256<<'\n'<<"verdict="<<summary.verdict<<'\n'<<"covariance="<<summary.covariance_residual<<'\n';for(const auto&a:summary.arms)std::cout<<a.label<<" valid="<<a.valid<<" gradient="<<a.gradient_inf<<" gradient_compare="<<a.gradient_comparison<<" hessian_compare="<<a.hessian_comparison<<" spectrum=("<<a.min_eigen<<','<<a.max_eigen<<") poisson="<<a.maximum_poisson_residual<<'\n';
  return summary.verdict=="CONNECTED_BLOCK_ANALYTIC_HESSIAN_EXECUTION_INVALID"?1:0;
}
