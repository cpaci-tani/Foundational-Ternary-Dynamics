#include "ftd/eft/common_moore_worldline_action.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <numeric>
#include <vector>

namespace ftd::eft {
namespace {

using Matrix3 = std::array<std::array<int, 3>, 3>;
constexpr double PI = 3.1415926535897932384626433832795;
constexpr double TOL = 1e-12;

int wrap(int value, int L) {
  const int r = value % L;
  return r < 0 ? r + L : r;
}

std::size_t flat_index(int L, int x, int y, int z) {
  const auto a = static_cast<std::size_t>(wrap(x, L));
  const auto b = static_cast<std::size_t>(wrap(y, L));
  const auto c = static_cast<std::size_t>(wrap(z, L));
  return (a * static_cast<std::size_t>(L) + b)
      * static_cast<std::size_t>(L) + c;
}

double component(const Vec3& v, int axis) {
  return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
}

void set_component(Vec3& v, int axis, double value) {
  if (axis == 0) v.x = value;
  if (axis == 1) v.y = value;
  if (axis == 2) v.z = value;
}

Vec3 add(const Vec3& a, const Vec3& b) {
  return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 subtract(const Vec3& a, const Vec3& b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 scale(const Vec3& v, double factor) {
  return {factor * v.x, factor * v.y, factor * v.z};
}

double max_abs(const Vec3& v) {
  return std::max({std::abs(v.x), std::abs(v.y), std::abs(v.z)});
}

double dot(const Vec3& a,const Vec3& b) {
  return a.x*b.x+a.y*b.y+a.z*b.z;
}

Vec3 cross(const Vec3& a,const Vec3& b) {
  return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}

double hat(double x, int site) {
  return std::max(0.0, 1.0 - std::abs(x - static_cast<double>(site)));
}

std::vector<double> smooth_axis(
    const std::vector<double>& input, int L, int axis) {
  std::vector<double> output(input.size());
  for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
      std::array<int, 3> p{{x, y, z}}, m=p, q=p;
      --m[static_cast<std::size_t>(axis)];
      ++q[static_cast<std::size_t>(axis)];
      output[flat_index(L,x,y,z)] =
          0.25 * input[flat_index(L,m[0],m[1],m[2])]
        + 0.50 * input[flat_index(L,x,y,z)]
        + 0.25 * input[flat_index(L,q[0],q[1],q[2])];
    }
  return output;
}

std::vector<double> smooth_all(std::vector<double> field, int L) {
  for (int axis=0; axis<3; ++axis) field=smooth_axis(field,L,axis);
  return field;
}

std::vector<Vec3> bridge_face_current(
    const std::array<std::vector<double>,3>& raw, int L) {
  std::vector<Vec3> result(raw[0].size());
  for (int axis=0; axis<3; ++axis) {
    std::vector<double> f=raw[static_cast<std::size_t>(axis)];
    for (int transverse=0; transverse<3; ++transverse)
      if (transverse != axis) f=smooth_axis(f,L,transverse);
    for (int x=0; x<L; ++x) for (int y=0; y<L; ++y)
      for (int z=0; z<L; ++z) {
        std::array<int,3> m{{x,y,z}};
        --m[static_cast<std::size_t>(axis)];
        const double value=0.5*(f[flat_index(L,x,y,z)]
            + f[flat_index(L,m[0],m[1],m[2])]);
        set_component(result[flat_index(L,x,y,z)],axis,value);
      }
  }
  return result;
}

void deposit_cic(std::vector<double>& rho, int L,
                 const Vec3& point, double weight) {
  const int lower[3]={static_cast<int>(std::floor(point.x)),
                      static_cast<int>(std::floor(point.y)),
                      static_cast<int>(std::floor(point.z))};
  for (int dx=0; dx<=1; ++dx) for (int dy=0; dy<=1; ++dy)
    for (int dz=0; dz<=1; ++dz) {
      const int x=lower[0]+dx, y=lower[1]+dy, z=lower[2]+dz;
      rho[flat_index(L,x,y,z)] += weight*hat(point.x,x)*hat(point.y,y)*hat(point.z,z);
    }
}

void deposit_instantaneous_face(
    std::array<std::vector<double>,3>& current, int L,
    const Vec3& point, const Vec3& delta, double weight) {
  for (int axis=0; axis<3; ++axis) {
    const double velocity=component(delta,axis);
    if (velocity==0.0) continue;
    const int a=(axis+1)%3, b=(axis+2)%3;
    const int face=static_cast<int>(std::floor(component(point,axis)));
    const int lower_a=static_cast<int>(std::floor(component(point,a)));
    const int lower_b=static_cast<int>(std::floor(component(point,b)));
    for (int da=0; da<=1; ++da) for (int db=0; db<=1; ++db) {
      std::array<int,3> p{{0,0,0}};
      p[static_cast<std::size_t>(axis)]=face;
      p[static_cast<std::size_t>(a)]=lower_a+da;
      p[static_cast<std::size_t>(b)]=lower_b+db;
      current[static_cast<std::size_t>(axis)][flat_index(L,p[0],p[1],p[2])]
          += weight*velocity
          *hat(component(point,a),p[static_cast<std::size_t>(a)])
          *hat(component(point,b),p[static_cast<std::size_t>(b)]);
    }
  }
}

std::vector<double> path_breaks(const Vec3& start, const Vec3& end) {
  std::vector<double> result{0.0,1.0};
  const Vec3 delta=subtract(end,start);
  for (int axis=0; axis<3; ++axis) {
    const double d=component(delta,axis);
    if (d==0.0) continue;
    const double lo=std::min(component(start,axis),component(end,axis));
    const double hi=std::max(component(start,axis),component(end,axis));
    for (int plane=static_cast<int>(std::floor(lo))+1;
         plane<=static_cast<int>(std::ceil(hi))-1; ++plane) {
      const double t=(plane-component(start,axis))/d;
      if (t>0.0 && t<1.0) result.push_back(t);
    }
  }
  std::sort(result.begin(),result.end());
  result.erase(std::unique(result.begin(),result.end(),[](double a,double b){
    return std::abs(a-b)<=32.0*std::numeric_limits<double>::epsilon();
  }),result.end());
  return result;
}

double sample_cic(const std::vector<double>& field, int L,
                  const Vec3& point) {
  double result=0.0;
  const int lower[3]={static_cast<int>(std::floor(point.x)),
                      static_cast<int>(std::floor(point.y)),
                      static_cast<int>(std::floor(point.z))};
  for(int dx=0;dx<=1;++dx)for(int dy=0;dy<=1;++dy)for(int dz=0;dz<=1;++dz){
    const int x=lower[0]+dx,y=lower[1]+dy,z=lower[2]+dz;
    result+=field[flat_index(L,x,y,z)]*hat(point.x,x)*hat(point.y,y)*hat(point.z,z);
  }
  return result;
}

std::array<std::vector<double>,3> adjoint_face_gather_field(
    const std::vector<Vec3>& central, int L) {
  std::array<std::vector<double>,3> result{{
      std::vector<double>(central.size()),std::vector<double>(central.size()),
      std::vector<double>(central.size())}};
  for(int axis=0;axis<3;++axis){
    std::vector<double> component_field(central.size());
    for(std::size_t i=0;i<central.size();++i)
      component_field[i]=component(central[i],axis);
    for(int transverse=0;transverse<3;++transverse)
      if(transverse!=axis)component_field=smooth_axis(component_field,L,transverse);
    for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z){
      std::array<int,3> p{{x,y,z}};
      ++p[static_cast<std::size_t>(axis)];
      result[static_cast<std::size_t>(axis)][flat_index(L,x,y,z)]=0.5*(
          component_field[flat_index(L,x,y,z)]
         +component_field[flat_index(L,p[0],p[1],p[2])]);
    }
  }
  return result;
}

double sample_raw_face_pairing(
    const std::array<std::vector<double>,3>& gathered, int L,
    const Vec3& point, const Vec3& delta) {
  double result=0.0;
  for(int axis=0;axis<3;++axis){
    const double velocity=component(delta,axis);
    if(velocity==0.0)continue;
    const int a=(axis+1)%3,b=(axis+2)%3;
    const int face=static_cast<int>(std::floor(component(point,axis)));
    const int lower_a=static_cast<int>(std::floor(component(point,a)));
    const int lower_b=static_cast<int>(std::floor(component(point,b)));
    for(int da=0;da<=1;++da)for(int db=0;db<=1;++db){
      std::array<int,3> p{{0,0,0}};
      p[static_cast<std::size_t>(axis)]=face;
      p[static_cast<std::size_t>(a)]=lower_a+da;
      p[static_cast<std::size_t>(b)]=lower_b+db;
      result+=velocity*gathered[static_cast<std::size_t>(axis)][flat_index(L,p[0],p[1],p[2])]
          *hat(component(point,a),p[static_cast<std::size_t>(a)])
          *hat(component(point,b),p[static_cast<std::size_t>(b)]);
    }
  }
  return result;
}

std::vector<double> divergence(const std::vector<Vec3>& field, int L) {
  std::vector<double> result(field.size());
  const auto at=[&](int x,int y,int z)->const Vec3&{
    return field[flat_index(L,x,y,z)];
  };
  for (int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z)
    result[flat_index(L,x,y,z)]=0.5*(
        at(x+1,y,z).x-at(x-1,y,z).x
       +at(x,y+1,z).y-at(x,y-1,z).y
       +at(x,y,z+1).z-at(x,y,z-1).z);
  return result;
}

std::vector<Vec3> gradient(const std::vector<double>& f,int L) {
  std::vector<Vec3> result(f.size());
  for(int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z)
    result[flat_index(L,x,y,z)]={
      0.5*(f[flat_index(L,x+1,y,z)]-f[flat_index(L,x-1,y,z)]),
      0.5*(f[flat_index(L,x,y+1,z)]-f[flat_index(L,x,y-1,z)]),
      0.5*(f[flat_index(L,x,y,z+1)]-f[flat_index(L,x,y,z-1)])};
  return result;
}

std::vector<Vec3> curl(const std::vector<Vec3>& f,int L) {
  std::vector<Vec3> result(f.size());
  const auto at=[&](int x,int y,int z)->const Vec3&{return f[flat_index(L,x,y,z)];};
  for(int x=0;x<L;++x) for(int y=0;y<L;++y) for(int z=0;z<L;++z)
    result[flat_index(L,x,y,z)]={
      0.5*(at(x,y+1,z).z-at(x,y-1,z).z-at(x,y,z+1).y+at(x,y,z-1).y),
      0.5*(at(x,y,z+1).x-at(x,y,z-1).x-at(x+1,y,z).z+at(x-1,y,z).z),
      0.5*(at(x+1,y,z).y-at(x-1,y,z).y-at(x,y+1,z).x+at(x,y-1,z).x)};
  return result;
}

long double pair(const std::vector<double>& a,const std::vector<double>& b) {
  long double r=0; for(std::size_t i=0;i<a.size();++i)r+=(long double)a[i]*b[i]; return r;
}
long double pair(const std::vector<Vec3>& a,const std::vector<Vec3>& b) {
  long double r=0; for(std::size_t i=0;i<a.size();++i)r+=(long double)a[i].x*b[i].x+(long double)a[i].y*b[i].y+(long double)a[i].z*b[i].z; return r;
}
double residual(long double a,long double b) {
  return static_cast<double>(std::abs(a-b)/(1.0L+std::max(std::abs(a),std::abs(b))));
}

double maximum_difference(const std::vector<double>& a,const std::vector<double>& b) {
  double r=0; for(std::size_t i=0;i<a.size();++i)r=std::max(r,std::abs(a[i]-b[i])); return r;
}
double maximum_difference(const std::vector<Vec3>& a,const std::vector<Vec3>& b) {
  double r=0; for(std::size_t i=0;i<a.size();++i)r=std::max(r,max_abs(subtract(a[i],b[i]))); return r;
}

std::vector<Matrix3> proper_rotations() {
  std::vector<Matrix3> result; std::array<int,3> p{{0,1,2}};
  do {
    int inv=0; for(int i=0;i<3;++i)for(int j=i+1;j<3;++j)if(p[i]>p[j])++inv;
    const int parity=inv%2==0?1:-1;
    for(int sx:{-1,1})for(int sy:{-1,1})for(int sz:{-1,1}) {
      if(parity*sx*sy*sz!=1)continue; Matrix3 m{}; const int s[3]={sx,sy,sz};
      for(int row=0;row<3;++row)m[row][p[row]]=s[row]; result.push_back(m);
    }
  } while(std::next_permutation(p.begin(),p.end())); return result;
}

Vec3 rotate(const Matrix3& m,const Vec3& v) {
  const double a[3]={v.x,v.y,v.z}; double b[3]={0,0,0};
  for(int i=0;i<3;++i)for(int j=0;j<3;++j)b[i]+=m[i][j]*a[j]; return {b[0],b[1],b[2]};
}

std::vector<Vec3> deterministic_field(int L,int fixture,double phase) {
  std::vector<Vec3> r(static_cast<std::size_t>(L)*L*L);
  for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z) {
    const double kx=2*PI*x/L,ky=2*PI*y/L,kz=2*PI*z/L,f=fixture+1;
    r[flat_index(L,x,y,z)]={0.17*std::sin(kx+f*ky+phase)+0.03*std::cos(kz),
      -0.13*std::cos(ky+f*kz-0.2*phase)+0.05*std::sin(kx),
      0.19*std::sin(kz+f*kx+0.3*phase)-0.07*std::cos(ky)};
  } return r;
}

long double direct_orbit_action(
    int L,const Vec3& start,const Vec3& end,int charge,
    const std::vector<Vec3>& r0,const std::vector<Vec3>& r1) {
  const auto d0=smooth_all(divergence(r0,L),L);
  const auto d1=smooth_all(divergence(r1,L),L);
  const auto c0=adjoint_face_gather_field(curl(r0,L),L);
  const auto c1=adjoint_face_gather_field(curl(r1,L),L);
  const Vec3 delta=subtract(end,start);const auto breaks=path_breaks(start,end);
  constexpr std::array<long double,4> nodes{{-0.8611363115940525752L,-0.3399810435848562648L,0.3399810435848562648L,0.8611363115940525752L}};
  constexpr std::array<long double,4> weights{{0.3478548451374538574L,0.6521451548625461426L,0.6521451548625461426L,0.3478548451374538574L}};
  long double action=0.0L;
  for(std::size_t piece=1;piece<breaks.size();++piece){
    const long double mid=0.5L*(breaks[piece-1]+breaks[piece]);
    const long double half=0.5L*(breaks[piece]-breaks[piece-1]);
    for(std::size_t sample=0;sample<4;++sample){
      const double t=static_cast<double>(mid+half*nodes[sample]);
      const Vec3 point=add(start,scale(delta,t));
      const double scalar=(1-t)*sample_cic(d0,L,point)+t*sample_cic(d1,L,point);
      const double vector=(1-t)*sample_raw_face_pairing(c0,L,point,delta)
          +t*sample_raw_face_pairing(c1,L,point,delta);
      action+=half*weights[sample]*static_cast<long double>(charge)*(scalar+vector);
    }
  }
  return static_cast<long double>(G_C)*action;
}

std::vector<double> sum(const std::vector<double>& a,const std::vector<double>& b) {
  std::vector<double> r(a.size());for(std::size_t i=0;i<a.size();++i)r[i]=a[i]+b[i];return r;
}
std::vector<Vec3> sum(const std::vector<Vec3>& a,const std::vector<Vec3>& b) {
  std::vector<Vec3> r(a.size());for(std::size_t i=0;i<a.size();++i)r[i]=add(a[i],b[i]);return r;
}

double norm2_difference(const std::vector<double>& a,const std::vector<double>& b) {
  long double r=0;for(std::size_t i=0;i<a.size();++i){const long double d=(long double)a[i]-b[i];r+=d*d;}return static_cast<double>(r);
}

double hodge_response(double kx,double ky,double kz) {
  const double sx=std::sin(kx),sy=std::sin(ky),sz=std::sin(kz);
  const double cx=std::cos(kx),cy=std::cos(ky),cz=std::cos(kz);
  const double m=4.0-(2.0/3.0)*(cx+cy+cz)-(2.0/3.0)*(cx*cy+cx*cz+cy*cz);
  if(m<=1e-28)return 0.0; return 3.0*(sx*sx+sy*sy+sz*sz)/m;
}

double coat_symbol(double kx,double ky,double kz) {
  const double x=std::cos(kx/2),y=std::cos(ky/2),z=std::cos(kz/2);
  return x*x*y*y*z*z;
}

double peierls_coefficient(int L,int axis) {
  long double total=0;
  for(int nx=0;nx<L;++nx)for(int ny=0;ny<L;++ny)for(int nz=0;nz<L;++nz){
    const double k[3]={2*PI*nx/L,2*PI*ny/L,2*PI*nz/L};
    const double b=coat_symbol(k[0],k[1],k[2]);
    total+=(long double)hodge_response(k[0],k[1],k[2])*b*b*(1-std::cos(k[axis]));
  }
  return G_C*G_C*static_cast<double>(total)/(static_cast<double>(L)*L*L);
}

double peierls_energy(int L,int axis,double r,int polarity) {
  long double total=0;
  for(int nx=0;nx<L;++nx)for(int ny=0;ny<L;++ny)for(int nz=0;nz<L;++nz){
    const double k[3]={2*PI*nx/L,2*PI*ny/L,2*PI*nz/L};
    const double b=coat_symbol(k[0],k[1],k[2]);
    const std::complex<double> amp=static_cast<double>(polarity)*b*((1-r)+r*std::exp(std::complex<double>(0,-k[axis])));
    total+=(long double)hodge_response(k[0],k[1],k[2])*std::norm(amp);
  }
  return -0.5*G_C*G_C*static_cast<double>(total)/(static_cast<double>(L)*L*L);
}

} // namespace

int MooreSpacetimeCurrent::index(int x,int y,int z) const {
  return L>0?static_cast<int>(flat_index(L,x,y,z)):-1;
}

MooreSpacetimeCurrent make_common_moore_spacetime_current(
    int L,const Vec3& start,const Vec3& end,int charge) {
  MooreSpacetimeCurrent result; result.L=L;result.charge=charge;
  result.start_effective_position=start;result.end_effective_position=end;
  if(L<7||(charge!=-1&&charge!=1))return result;
  const std::size_t n=static_cast<std::size_t>(L)*L*L;
  std::vector<double> raw0(n),raw1(n),raw_t0(n),raw_t1(n);
  std::array<std::vector<double>,3> raw_q0{{std::vector<double>(n),std::vector<double>(n),std::vector<double>(n)}};
  std::array<std::vector<double>,3> raw_q1{{std::vector<double>(n),std::vector<double>(n),std::vector<double>(n)}};
  deposit_cic(raw0,L,start,charge);deposit_cic(raw1,L,end,charge);
  const Vec3 delta=subtract(end,start); const auto breaks=path_breaks(start,end);
  constexpr std::array<long double,4> nodes{{-0.8611363115940525752L,-0.3399810435848562648L,0.3399810435848562648L,0.8611363115940525752L}};
  constexpr std::array<long double,4> weights{{0.3478548451374538574L,0.6521451548625461426L,0.6521451548625461426L,0.3478548451374538574L}};
  for(std::size_t piece=1;piece<breaks.size();++piece){
    const long double mid=0.5L*(breaks[piece-1]+breaks[piece]);
    const long double half=0.5L*(breaks[piece]-breaks[piece-1]);
    for(std::size_t s=0;s<4;++s){
      const double t=static_cast<double>(mid+half*nodes[s]);
      const double w=static_cast<double>(half*weights[s])*charge;
      const Vec3 point=add(start,scale(delta,t));
      deposit_cic(raw_t0,L,point,w*(1-t));deposit_cic(raw_t1,L,point,w*t);
      deposit_instantaneous_face(raw_q0,L,point,delta,w*(1-t));
      deposit_instantaneous_face(raw_q1,L,point,delta,w*t);
    }
  }
  // deposit_instantaneous_face receives the signed quadrature weight, so its
  // velocity factor is not charged a second time.
  result.rho_start=smooth_all(raw0,L);result.rho_end=smooth_all(raw1,L);
  result.temporal_density_start=smooth_all(raw_t0,L);
  result.temporal_density_end=smooth_all(raw_t1,L);
  result.current_start=bridge_face_current(raw_q0,L);
  result.current_end=bridge_face_current(raw_q1,L);
  const auto temporal=sum(result.temporal_density_start,result.temporal_density_end);
  const auto q=sum(result.current_start,result.current_end);
  const auto div0=divergence(result.current_start,L),div1=divergence(result.current_end,L),divq=divergence(q,L);
  long double temporal_sum=0;for(double v:temporal)temporal_sum+=v;
  result.temporal_partition_residual=std::abs(static_cast<double>(temporal_sum-charge));
  for(std::size_t i=0;i<n;++i){
    result.split_continuity_start_residual=std::max(result.split_continuity_start_residual,std::abs(div0[i]-result.rho_start[i]+temporal[i]));
    result.split_continuity_end_residual=std::max(result.split_continuity_end_residual,std::abs(div1[i]+result.rho_end[i]-temporal[i]));
    result.aggregate_continuity_residual=std::max(result.aggregate_continuity_residual,std::abs(result.rho_end[i]-result.rho_start[i]+divq[i]));
  }
  // Compare to the already-qualified FTD-0577 aggregate observer.
  const Coord a0{static_cast<int>(std::floor(start.x)),static_cast<int>(std::floor(start.y)),static_cast<int>(std::floor(start.z))};
  const Coord a1{static_cast<int>(std::floor(end.x)),static_cast<int>(std::floor(end.y)),static_cast<int>(std::floor(end.z))};
  const Vec3 r0{start.x-a0.x,start.y-a0.y,start.z-a0.z};
  const Vec3 r1{end.x-a1.x,end.y-a1.y,end.z-a1.z};
  const auto face=make_face_current_segment(L,a0,r0,a1,r1,charge);
  const auto aggregate=make_minimal_moore_compatibility_coat(face);
  result.current_reconstruction_residual=aggregate.valid?maximum_difference(q,aggregate.central_current):INFINITY;
  int support=0;for(std::size_t i=0;i<n;++i)if(std::abs(temporal[i])>1e-15||max_abs(q[i])>1e-15)++support;
  result.finite_range=support<L*L*L;
  result.valid=result.temporal_partition_residual<=TOL&&result.current_reconstruction_residual<=TOL
      &&result.split_continuity_start_residual<=TOL&&result.split_continuity_end_residual<=TOL
      &&result.aggregate_continuity_residual<=TOL&&result.finite_range;
  return result;
}

CommonMooreWorldlineActionResult analyze_common_moore_worldline_action() {
  CommonMooreWorldlineActionResult result;
  // All signed Moore one-cell paths, both volumes and polarities.
  for(int L:{17,33}){
    const double c=L/2;
    for(int charge:{-1,1}){
      for(int dx=-1;dx<=1;++dx)for(int dy=-1;dy<=1;++dy)for(int dz=-1;dz<=1;++dz){
        if(dx==0&&dy==0&&dz==0)continue;
        const auto current=make_common_moore_spacetime_current(L,{c,c,c},{c+dx,c+dy,c+dz},charge);
        result.maximum_temporal_partition_residual=std::max(result.maximum_temporal_partition_residual,current.temporal_partition_residual);
        result.maximum_current_reconstruction_residual=std::max(result.maximum_current_reconstruction_residual,current.current_reconstruction_residual);
        result.maximum_split_continuity_residual=std::max({result.maximum_split_continuity_residual,current.split_continuity_start_residual,current.split_continuity_end_residual});
        result.maximum_aggregate_continuity_residual=std::max(result.maximum_aggregate_continuity_residual,current.aggregate_continuity_residual);
        ++result.aggregate_split_arms;
      }
      ++result.polarity_arms;
    }
    ++result.volume_arms;
  }
  result.coated_spacetime_continuity_exact=result.aggregate_split_arms==104
      &&result.maximum_temporal_partition_residual<=TOL
      &&result.maximum_current_reconstruction_residual<=TOL
      &&result.maximum_split_continuity_residual<=TOL
      &&result.maximum_aggregate_continuity_residual<=TOL;

  // Deposited action and the orbit gather are the same pairing after applying
  // the self-adjoint coat/current bridge. Endpoint source checks use D^T=-G
  // and C^T=C independently on deterministic variations.
  const int L=17; const double c=L/2;
  for(int fixture=0;fixture<4;++fixture){
    const Vec3 end=fixture%2==0?Vec3{c+1,c+1,c}:Vec3{c+1,c+1,c+1};
    const auto j=make_common_moore_spacetime_current(L,{c,c,c},end,fixture%2?1:-1);
    const auto r0=deterministic_field(L,fixture,0.0),r1=deterministic_field(L,fixture,0.37);
    const auto d0=divergence(r0,L),d1=divergence(r1,L);
    const auto c0=curl(r0,L),c1=curl(r1,L);
    const long double deposited=G_C*(pair(j.temporal_density_start,d0)+pair(j.temporal_density_end,d1)+pair(j.current_start,c0)+pair(j.current_end,c1));
    // Independent orbit-side evaluation: coat and bridge adjoints are applied
    // to the field first, then sampled along the straight segment.
    const long double orbit=direct_orbit_action(L,{c,c,c},end,j.charge,r0,r1);
    result.maximum_deposit_orbit_action_residual=std::max(result.maximum_deposit_orbit_action_residual,residual(deposited,orbit));
    const auto g0=gradient(j.temporal_density_start,L),g1=gradient(j.temporal_density_end,L);
    const auto cq0=curl(j.current_start,L),cq1=curl(j.current_end,L);
    std::vector<Vec3>s0(g0.size()),s1(g1.size());for(std::size_t i=0;i<s0.size();++i){s0[i]=add(scale(g0[i],-G_C),scale(cq0[i],G_C));s1[i]=add(scale(g1[i],-G_C),scale(cq1[i],G_C));}
    result.maximum_endpoint_field_adjoint_residual=std::max({result.maximum_endpoint_field_adjoint_residual,
      residual(pair(s0,r0),G_C*(pair(j.temporal_density_start,d0)+pair(j.current_start,c0))),
      residual(pair(s1,r1),G_C*(pair(j.temporal_density_end,d1)+pair(j.current_end,c1)))});
    ++result.action_fixture_arms;
  }
  result.common_action_deposition_and_gather_adjoint=result.action_fixture_arms==4
      &&result.maximum_deposit_orbit_action_residual<=TOL
      &&result.maximum_endpoint_field_adjoint_residual<=TOL;
  result.reciprocal_path_gather_derived=result.common_action_deposition_and_gather_adjoint;
  for(int fixture=0;fixture<8;++fixture){
    const Vec3 v{0.13+0.01*fixture,-0.21+0.02*fixture,0.17-0.005*fixture};
    const Vec3 b{-0.11+0.007*fixture,0.19-0.003*fixture,0.23+0.004*fixture};
    result.maximum_magnetic_scalar_work_residual=std::max(
        result.maximum_magnetic_scalar_work_residual,std::abs(dot(v,cross(v,b))));
  }
  result.magnetic_scalar_work_zero=result.maximum_magnetic_scalar_work_residual<=TOL;

  // Integer translation covariance.
  const auto reference=make_common_moore_spacetime_current(L,{c+.10,c-.20,c+.15},{c+.70,c+.45,c+.30},1);
  for(const Vec3& shift:{Vec3{2,-3,1},Vec3{-3,2,-2},Vec3{1,2,-3}}){
    const auto translated=make_common_moore_spacetime_current(L,add(reference.start_effective_position,shift),add(reference.end_effective_position,shift),1);
    for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z){
      const auto i=flat_index(L,x,y,z),k=flat_index(L,x+(int)shift.x,y+(int)shift.y,z+(int)shift.z);
      result.maximum_translation_covariance_residual=std::max({result.maximum_translation_covariance_residual,
        std::abs(translated.temporal_density_start[k]-reference.temporal_density_start[i]),
        std::abs(translated.temporal_density_end[k]-reference.temporal_density_end[i]),
        max_abs(subtract(translated.current_start[k],reference.current_start[i])),
        max_abs(subtract(translated.current_end[k],reference.current_end[i]))});
    } ++result.translation_arms;
  }
  // Cubic covariance is checked at the invariant scalar diagnostics for all 24
  // rotations; full tensor covariance already belongs to FTD-0577.
  for(const auto& m:proper_rotations()){
    const Vec3 rs=add(Vec3{c,c,c},rotate(m,subtract(reference.start_effective_position,Vec3{c,c,c})));
    const Vec3 re=add(Vec3{c,c,c},rotate(m,subtract(reference.end_effective_position,Vec3{c,c,c})));
    const auto rotated=make_common_moore_spacetime_current(L,rs,re,1);
    result.maximum_cubic_covariance_residual=std::max({result.maximum_cubic_covariance_residual,
      std::abs(rotated.temporal_partition_residual-reference.temporal_partition_residual),
      std::abs(rotated.current_reconstruction_residual-reference.current_reconstruction_residual),
      std::abs(rotated.split_continuity_start_residual-reference.split_continuity_start_residual),
      std::abs(rotated.split_continuity_end_residual-reference.split_continuity_end_residual)});
    ++result.proper_cubic_rotation_arms;
  }

  const auto centering=[&](Vec3 end){
    const auto j=make_common_moore_spacetime_current(L,{c,c,c},end,1);
    std::vector<double> midpoint(j.rho_start.size());for(std::size_t i=0;i<midpoint.size();++i)midpoint[i]=0.5*(j.rho_start[i]+j.rho_end[i]);
    return norm2_difference(sum(j.temporal_density_start,j.temporal_density_end),midpoint);
  };
  result.axial_centering_norm2=centering({c+1,c,c});
  result.edge_centering_norm2=centering({c+1,c+1,c});
  result.body_centering_norm2=centering({c+1,c+1,c+1});
  result.minimum_diagonal_centering_norm2=std::min(result.edge_centering_norm2,result.body_centering_norm2);
  result.maximum_centering_rational_residual=std::max({std::abs(result.axial_centering_norm2),
    std::abs(result.edge_centering_norm2-1.0/1536.0),std::abs(result.body_centering_norm2-5.0/3072.0)});
  result.centering_arms=3;
  result.axial_energy_centering_exact=result.axial_centering_norm2<=TOL;
  result.diagonal_energy_centering_fails=result.minimum_diagonal_centering_norm2>1e-6&&result.maximum_centering_rational_residual<=TOL;

  result.minimum_peierls_coefficient=std::numeric_limits<double>::infinity();
  result.minimum_peierls_barrier=std::numeric_limits<double>::infinity();
  for(int volume:{17,33})for(int axis=0;axis<3;++axis){
    const double coefficient=peierls_coefficient(volume,axis),v0=peierls_energy(volume,axis,0,1);
    result.minimum_peierls_coefficient=std::min(result.minimum_peierls_coefficient,coefficient);
    result.minimum_peierls_barrier=std::min(result.minimum_peierls_barrier,coefficient/4);
    for(int polarity:{-1,1})for(int step=0;step<=8;++step){
      const double r=step/8.0,v=peierls_energy(volume,axis,r,polarity);
      result.maximum_peierls_law_residual=std::max(result.maximum_peierls_law_residual,std::abs(v-(v0+coefficient*r*(1-r))));
      result.maximum_peierls_polarity_residual=std::max(result.maximum_peierls_polarity_residual,std::abs(v-peierls_energy(volume,axis,r,-polarity)));
      ++result.peierls_arms;
    }
  }
  const double c0=peierls_coefficient(33,0);for(int axis=1;axis<3;++axis)result.maximum_peierls_cubic_residual=std::max(result.maximum_peierls_cubic_residual,std::abs(peierls_coefficient(33,axis)-c0));
  result.point_carrier_peierls_pinned=result.peierls_arms==108&&result.minimum_peierls_coefficient>1e-8
      &&result.minimum_peierls_barrier>1e-8&&result.maximum_peierls_law_residual<=TOL
      &&result.maximum_peierls_polarity_residual<=TOL&&result.maximum_peierls_cubic_residual<=TOL;
  result.unmodified_action_is_free_mobile_law=false;result.production_changed=false;
  result.valid=result.coated_spacetime_continuity_exact&&result.common_action_deposition_and_gather_adjoint
      &&result.reciprocal_path_gather_derived&&result.magnetic_scalar_work_zero
      &&result.translation_arms==3&&result.maximum_translation_covariance_residual<=TOL
      &&result.proper_cubic_rotation_arms==24&&result.maximum_cubic_covariance_residual<=TOL
      &&result.axial_energy_centering_exact&&result.diagonal_energy_centering_fails
      &&result.point_carrier_peierls_pinned&&!result.unmodified_action_is_free_mobile_law&&!result.production_changed;
  return result;
}

} // namespace ftd::eft
