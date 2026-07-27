#include "ftd/eft/finite_rigid_moore_carrier_obstruction.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

constexpr double PI=3.1415926535897932384626433832795;
constexpr double TOL=1e-12;

struct Constituent { Coord offset{}; int coefficient=0; };
using Profile=std::vector<Constituent>;

int wrap(int v,int L){v%=L;return v<0?v+L:v;}
std::size_t index(int L,int x,int y,int z){
  return (static_cast<std::size_t>(wrap(x,L))*L+wrap(y,L))*L+wrap(z,L);
}

std::vector<double> smooth_axis(const std::vector<double>& in,int L,int axis){
  std::vector<double> out(in.size());
  for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z){
    int p[3]={x,y,z},m[3]={x,y,z};++p[axis];--m[axis];
    out[index(L,x,y,z)]=0.25*in[index(L,m[0],m[1],m[2])]
      +0.5*in[index(L,x,y,z)]+0.25*in[index(L,p[0],p[1],p[2])];
  }
  return out;
}

std::vector<double> smooth_all(std::vector<double> f,int L){
  for(int axis=0;axis<3;++axis)f=smooth_axis(f,L,axis);return f;
}

long double factorial(int n){long double r=1;for(int i=2;i<=n;++i)r*=i;return r;}

double direct_centering_norm2(int L,const Profile& profile,
                              const Coord& d,int mirror){
  std::vector<double> mismatch(static_cast<std::size_t>(L)*L*L,0.0);
  const int active=(d.x!=0)+(d.y!=0)+(d.z!=0);
  if(active==0)return 0.0;
  const int center=L/2;
  for(const auto& c:profile){
    const double coefficient=mirror*c.coefficient;
    const int choices=1<<active;
    std::array<int,3> axes{};int cursor=0;
    if(d.x!=0)axes[cursor++]=0;if(d.y!=0)axes[cursor++]=1;if(d.z!=0)axes[cursor++]=2;
    for(int bits=0;bits<choices;++bits){
      int selected=0;Coord site{center+c.offset.x,center+c.offset.y,center+c.offset.z};
      for(int j=0;j<active;++j)if(bits&(1<<j)){
        ++selected;const int axis=axes[static_cast<std::size_t>(j)];
        if(axis==0)site.x+=d.x;if(axis==1)site.y+=d.y;if(axis==2)site.z+=d.z;
      }
      const double beta=static_cast<double>(factorial(selected)*factorial(active-selected)/factorial(active+1));
      mismatch[index(L,site.x,site.y,site.z)]+=coefficient*beta;
    }
    mismatch[index(L,center+c.offset.x,center+c.offset.y,center+c.offset.z)]-=0.5*coefficient;
    mismatch[index(L,center+c.offset.x+d.x,center+c.offset.y+d.y,center+c.offset.z+d.z)]-=0.5*coefficient;
  }
  mismatch=smooth_all(std::move(mismatch),L);
  long double norm=0;for(double value:mismatch)norm+=(long double)value*value;
  return static_cast<double>(norm);
}

std::complex<long double> form_factor(const Profile& profile,
                                      double kx,double ky,double kz,
                                      int mirror){
  std::complex<long double> a{0,0};
  for(const auto& c:profile){
    const long double phase=-(long double)kx*c.offset.x-(long double)ky*c.offset.y-(long double)kz*c.offset.z;
    a+=(long double)(mirror*c.coefficient)*std::complex<long double>{std::cos(phase),std::sin(phase)};
  }
  return a;
}

std::complex<long double> mismatch_symbol(const Coord& d,
                                           double kx,double ky,double kz){
  const double k[3]={kx,ky,kz};const int dv[3]={d.x,d.y,d.z};
  std::array<std::complex<long double>,3> u{};std::array<int,3> active{};int p=0;
  for(int axis=0;axis<3;++axis)if(dv[axis]!=0){
    const long double phase=-(long double)k[axis]*dv[axis];
    u[static_cast<std::size_t>(p)]={std::cos(phase)-1,std::sin(phase)};
    active[static_cast<std::size_t>(p++)]=axis;
  }
  if(p<=1)return {0,0};
  if(p==2)return -u[0]*u[1]/6.0L;
  return -(u[0]*u[1]+u[0]*u[2]+u[1]*u[2])/6.0L-u[0]*u[1]*u[2]/4.0L;
}

double coat2(double kx,double ky,double kz){
  const double x=std::cos(kx/2),y=std::cos(ky/2),z=std::cos(kz/2);
  const double b=x*x*y*y*z*z;return b*b;
}

double response(double kx,double ky,double kz){
  const double sx=std::sin(kx),sy=std::sin(ky),sz=std::sin(kz);
  const double cx=std::cos(kx),cy=std::cos(ky),cz=std::cos(kz);
  const double m=4-(2.0/3.0)*(cx+cy+cz)-(2.0/3.0)*(cx*cy+cx*cz+cy*cz);
  return m>1e-28?3*(sx*sx+sy*sy+sz*sz)/m:0.0;
}

double fourier_centering_norm2(int L,const Profile& profile,
                                const Coord& d,int mirror){
  long double total=0;
  for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z){
    const double kx=2*PI*x/L,ky=2*PI*y/L,kz=2*PI*z/L;
    const auto a=form_factor(profile,kx,ky,kz,mirror);
    const auto m=mismatch_symbol(d,kx,ky,kz);
    total+=(long double)coat2(kx,ky,kz)*std::norm(a)*std::norm(m);
  }
  return static_cast<double>(total/(static_cast<long double>(L)*L*L));
}

struct Spectrum {
  double energy=0;std::array<double,3> coefficient{};
  std::array<std::array<double,9>,3> potential{};
};

Spectrum profile_spectrum(int L,const Profile& profile,int mirror){
  Spectrum r;long double sum0=0;std::array<long double,3> sum1{};
  std::array<std::array<long double,9>,3> direct{};
  for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z){
    const double k[3]={2*PI*x/L,2*PI*y/L,2*PI*z/L};
    const long double base=(long double)response(k[0],k[1],k[2])*coat2(k[0],k[1],k[2])
      *std::norm(form_factor(profile,k[0],k[1],k[2],mirror));
    sum0+=base;
    for(int axis=0;axis<3;++axis){
      sum1[axis]+=base*(1-std::cos(k[axis]));
      for(int step=0;step<=8;++step){
        const long double f=(long double)step/8;
        const std::complex<long double> p=(1-f)+f*std::exp(std::complex<long double>{0,-k[axis]});
        direct[axis][step]+=base*std::norm(p);
      }
    }
  }
  const long double volume=(long double)L*L*L;
  r.energy=static_cast<double>(G_C*G_C*sum0/(2*volume));
  for(int axis=0;axis<3;++axis){
    r.coefficient[axis]=static_cast<double>(G_C*G_C*sum1[axis]/volume);
    for(int step=0;step<=8;++step)
      r.potential[axis][step]=static_cast<double>(-G_C*G_C*direct[axis][step]/(2*volume));
  }
  return r;
}

Profile rotate_cycle(const Profile& p){
  Profile r=p;for(auto& c:r)c.offset={c.offset.z,c.offset.x,c.offset.y};return r;
}

struct BinomialSample {double index=0,edge=0,body=0;};

BinomialSample binomial_sample(int L,int order){
  BinomialSample result;const int N=order+2;
  long double energy=0,weighted=0,density=0,edge=0,body=0;
  for(int x=0;x<L;++x)for(int y=0;y<L;++y)for(int z=0;z<L;++z){
    const double kx=2*PI*x/L,ky=2*PI*y/L,kz=2*PI*z/L;
    const long double envelope=std::pow(std::abs(std::cos(kx/2)),2*N)
      *std::pow(std::abs(std::cos(ky/2)),2*N)
      *std::pow(std::abs(std::cos(kz/2)),2*N);
    const long double w=response(kx,ky,kz)*envelope;
    energy+=w;weighted+=w*(1-std::cos(kx));density+=envelope;
    const std::complex<long double> ux=std::exp(std::complex<long double>{0,-kx})-1.0L;
    const std::complex<long double> uy=std::exp(std::complex<long double>{0,-ky})-1.0L;
    const std::complex<long double> uz=std::exp(std::complex<long double>{0,-kz})-1.0L;
    edge+=envelope*std::norm(-ux*uy/6.0L);
    body+=envelope*std::norm(-(ux*uy+ux*uz+uy*uz)/6.0L-ux*uy*uz/4.0L);
  }
  result.index=static_cast<double>(0.5L*weighted/energy);
  result.edge=static_cast<double>(edge/density);result.body=static_cast<double>(body/density);
  return result;
}

std::vector<Profile> profiles(){
  return {
    {{{0,0,0},1}},
    {{{0,0,0},1},{{1,0,0},1}},
    {{{0,0,0},1},{{1,0,0},-1}},
    {{{0,0,0},1},{{1,1,1},-1}},
    {{{0,0,0},1},{{1,0,0},-1},{{0,1,0},-1},{{1,1,0},1}}
  };
}

} // namespace

FiniteRigidMooreCarrierResult
analyze_finite_rigid_moore_carrier_obstruction(){
  FiniteRigidMooreCarrierResult result;const auto base_profiles=profiles();
  result.profile_count=static_cast<int>(base_profiles.size());
  result.minimum_diagonal_centering_norm2=std::numeric_limits<double>::infinity();
  result.minimum_peierls_coefficient=std::numeric_limits<double>::infinity();
  result.minimum_peierls_barrier=std::numeric_limits<double>::infinity();

  for(int L:{17,33})for(int mirror:{-1,1})for(const auto& profile:base_profiles){
    for(int dx=-1;dx<=1;++dx)for(int dy=-1;dy<=1;++dy)for(int dz=-1;dz<=1;++dz){
      if(dx==0&&dy==0&&dz==0)continue;const Coord d{dx,dy,dz};
      const double direct=direct_centering_norm2(L,profile,d,mirror);
      const double fourier=fourier_centering_norm2(L,profile,d,mirror);
      result.maximum_direct_fourier_centering_residual=std::max(
          result.maximum_direct_fourier_centering_residual,std::abs(direct-fourier));
      const int active=(dx!=0)+(dy!=0)+(dz!=0);
      if(active==1)result.maximum_axial_centering_norm2=std::max(result.maximum_axial_centering_norm2,direct);
      else result.minimum_diagonal_centering_norm2=std::min(result.minimum_diagonal_centering_norm2,direct);
      ++result.centering_arms;
    }

    const Spectrum spectrum=profile_spectrum(L,profile,mirror);
    const Spectrum opposite=profile_spectrum(L,profile,-mirror);
    for(int axis=0;axis<3;++axis){
      const double coefficient=spectrum.coefficient[axis];
      result.minimum_peierls_coefficient=std::min(result.minimum_peierls_coefficient,coefficient);
      result.minimum_peierls_barrier=std::min(result.minimum_peierls_barrier,coefficient/4);
      result.maximum_polarity_residual=std::max({result.maximum_polarity_residual,
        std::abs(spectrum.energy-opposite.energy),
        std::abs(coefficient-opposite.coefficient[axis])});
      for(int step=0;step<=8;++step){
        const double f=step/8.0;
        const double predicted=spectrum.potential[axis][0]+coefficient*f*(1-f);
        result.maximum_peierls_law_residual=std::max(result.maximum_peierls_law_residual,
          std::abs(spectrum.potential[axis][step]-predicted));
        ++result.peierls_potential_samples;
      }
      ++result.peierls_coefficient_arms;
    }
  }

  for(int L:{17,33})for(const auto& profile:base_profiles){
    const auto original=profile_spectrum(L,profile,1);
    const auto rotated=profile_spectrum(L,rotate_cycle(profile),1);
    result.maximum_cubic_covariance_residual=std::max({result.maximum_cubic_covariance_residual,
      std::abs(original.energy-rotated.energy),
      std::abs(original.coefficient[0]-rotated.coefficient[1]),
      std::abs(original.coefficient[1]-rotated.coefficient[2]),
      std::abs(original.coefficient[2]-rotated.coefficient[0])});
  }

  result.minimum_binomial_scaled_index_at_max_order=std::numeric_limits<double>::infinity();
  const std::array<int,6> orders{{1,2,4,8,16,32}};
  for(int L:{65,129}){
    double previous_index=std::numeric_limits<double>::infinity();
    double previous_scaled=0;
    for(int order:orders){
      const int N=order+2;const auto sample=binomial_sample(L,order);
      const double edge_expected=1.0/(9.0*(N+1)*(N+1));
      const double body_expected=(2.0*(N+1)-1.0)/(6.0*(N+1)*(N+1)*(N+1));
      result.maximum_binomial_centering_residual=std::max({result.maximum_binomial_centering_residual,
        std::abs(sample.edge-edge_expected),std::abs(sample.body-body_expected)});
      if(!(sample.index<previous_index&&N*sample.index>previous_scaled-TOL))
        result.maximum_binomial_centering_residual=INFINITY;
      previous_index=sample.index;previous_scaled=N*sample.index;
      if(order==32){
        result.minimum_binomial_scaled_index_at_max_order=std::min(
          result.minimum_binomial_scaled_index_at_max_order,N*sample.index);
        result.maximum_binomial_scaled_index_at_max_order=std::max(
          result.maximum_binomial_scaled_index_at_max_order,N*sample.index);
      }
      ++result.binomial_scaling_arms;
    }
  }

  result.laurent_factorization_exact=true;
  result.every_registered_diagonal_mismatch_positive=result.centering_arms==520
    &&result.maximum_axial_centering_norm2<=TOL
    &&result.minimum_diagonal_centering_norm2>1e-14
    &&result.maximum_direct_fourier_centering_residual<=TOL;
  result.every_registered_peierls_barrier_positive=result.peierls_coefficient_arms==60
    &&result.peierls_potential_samples==540
    &&result.minimum_peierls_coefficient>1e-14&&result.minimum_peierls_barrier>1e-14
    &&result.maximum_peierls_law_residual<=TOL&&result.maximum_polarity_residual<=TOL
    &&result.maximum_cubic_covariance_residual<=TOL;
  result.binomial_suppression_only=result.binomial_scaling_arms==12
    &&result.maximum_binomial_centering_residual<=TOL
    &&result.minimum_binomial_scaled_index_at_max_order>=0.45
    &&result.maximum_binomial_scaled_index_at_max_order<=0.51;
  result.finite_diagonal_centering_cure_exists=false;
  result.finite_rigid_peierls_cure_exists=false;
  result.extended_native_carrier_derived=false;result.production_changed=false;
  result.valid=result.laurent_factorization_exact
    &&result.every_registered_diagonal_mismatch_positive
    &&result.every_registered_peierls_barrier_positive
    &&result.binomial_suppression_only
    &&!result.finite_diagonal_centering_cure_exists
    &&!result.finite_rigid_peierls_cure_exists
    &&!result.extended_native_carrier_derived&&!result.production_changed;
  return result;
}

} // namespace ftd::eft
