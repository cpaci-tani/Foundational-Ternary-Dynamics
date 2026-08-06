#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/local_polarity_regularity.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ftd::eft {
namespace {

struct DeviceVectorField {
  double* x;
  double* y;
  double* z;
};

struct DeviceCurrentEntry {
  std::size_t index;
  int axis;
  double value;
};

struct DeviceCurrentGroup {
  std::size_t index;
  std::size_t offset;
  std::size_t count;
  int axis;
};

constexpr int kDeterministicMaximumRadii=6;
constexpr int kDeterministicObserverQuantities=
    kDeterministicMaximumRadii+2;

struct DeviceSelectedRadii {
  int count=0;
  int threshold2[kDeterministicMaximumRadii]{};
};

struct DeviceDensityEntry {
  std::size_t index;
  double value;
};

struct DeviceStencil {
  int count=0;
  int shifts[7]{};
  double weights[7]{};
};

enum class SplineKernel { B1, B2 };

double spline_kernel_value(SplineKernel kernel,double value) {
  return evaluate_local_polarity_kernel(
      kernel==SplineKernel::B1?LocalPolarityKernel::Hat
                              :LocalPolarityKernel::QuadraticBSpline,value);
}

void append_spline_knots(std::vector<double>& knots,
                         SplineKernel kernel,double center) {
  if(kernel==SplineKernel::B1)
    knots.insert(knots.end(),{center-1.0,center,center+1.0});
  else
    knots.insert(knots.end(),{center-1.5,center-0.5,center+0.5,center+1.5});
}

double spline_overlap(SplineKernel left,SplineKernel right,
                      double relative_center) {
  constexpr std::array<long double,4> nodes{{
      -0.861136311594052575223946488893L,
      -0.339981043584856264802665759103L,
       0.339981043584856264802665759103L,
       0.861136311594052575223946488893L}};
  constexpr std::array<long double,4> weights{{
      0.347854845137453857373063949222L,
      0.652145154862546142626936050778L,
      0.652145154862546142626936050778L,
      0.347854845137453857373063949222L}};
  std::vector<double> knots;
  append_spline_knots(knots,left,0.0);
  append_spline_knots(knots,right,relative_center);
  std::sort(knots.begin(),knots.end());
  knots.erase(std::unique(knots.begin(),knots.end()),knots.end());
  long double integral=0.0L;
  for(std::size_t piece=1;piece<knots.size();++piece) {
    const long double lo=knots[piece-1],hi=knots[piece];
    const long double midpoint=0.5L*(lo+hi),half_width=0.5L*(hi-lo);
    for(std::size_t sample=0;sample<nodes.size();++sample) {
      const double x=static_cast<double>(midpoint+half_width*nodes[sample]);
      integral+=half_width*weights[sample]
          *static_cast<long double>(spline_kernel_value(left,x))
          *spline_kernel_value(right,x-relative_center);
    }
  }
  return static_cast<double>(integral);
}

DeviceStencil make_spline_stencil(int face_component,int edge_component,
                                  int axis) {
  const auto face_kernel=face_component==axis
      ?SplineKernel::B1:SplineKernel::B2;
  const auto edge_kernel=edge_component==axis
      ?SplineKernel::B2:SplineKernel::B1;
  const double face_shift=face_component==axis?0.5:0.0;
  const double edge_shift=edge_component==axis?0.0:0.5;
  DeviceStencil result;
  for(int shift=-3;shift<=3;++shift) {
    const double value=spline_overlap(face_kernel,edge_kernel,
        static_cast<double>(shift)+edge_shift-face_shift);
    if(std::abs(value)>1e-18) {
      result.shifts[result.count]=shift;
      result.weights[result.count]=value;
      ++result.count;
    }
  }
  return result;
}

__device__ __forceinline__ std::size_t device_index(
    int x, int y, int z, int L) {
  if (x < 0) x += L;
  if (x >= L) x -= L;
  if (y < 0) y += L;
  if (y >= L) y -= L;
  if (z < 0) z += L;
  if (z >= L) z -= L;
  return (static_cast<std::size_t>(x)*L+y)*L+z;
}

__device__ __forceinline__ void decode_index(
    std::size_t index, int L, int& x, int& y, int& z) {
  z = static_cast<int>(index%static_cast<std::size_t>(L));
  const std::size_t plane = index/static_cast<std::size_t>(L);
  y = static_cast<int>(plane%static_cast<std::size_t>(L));
  x = static_cast<int>(plane/static_cast<std::size_t>(L));
}

__device__ __forceinline__ void curl_edge(
    DeviceVectorField edge, int x, int y, int z, int L,
    double& cx, double& cy, double& cz) {
  const auto i = device_index(x,y,z,L);
  const auto xm = device_index(x-1,y,z,L);
  const auto ym = device_index(x,y-1,z,L);
  const auto zm = device_index(x,y,z-1,L);
  cx = edge.z[i]-edge.z[ym]-edge.y[i]+edge.y[zm];
  cy = edge.x[i]-edge.x[zm]-edge.z[i]+edge.z[xm];
  cz = edge.y[i]-edge.y[xm]-edge.x[i]+edge.x[ym];
}

__device__ __forceinline__ void curl_face_adjoint(
    DeviceVectorField face, int x, int y, int z, int L,
    double& cx, double& cy, double& cz) {
  const auto i = device_index(x,y,z,L);
  const auto xp = device_index(x+1,y,z,L);
  const auto yp = device_index(x,y+1,z,L);
  const auto zp = device_index(x,y,z+1,L);
  cx = face.z[yp]-face.z[i]-face.y[zp]+face.y[i];
  cy = face.x[zp]-face.x[i]-face.z[xp]+face.z[i];
  cz = face.y[xp]-face.y[i]-face.x[yp]+face.x[i];
}

__global__ void prepare_magnetic_kernel(
    DeviceVectorField electric, DeviceVectorField magnetic_before,
    DeviceVectorField magnetic_after, std::size_t count, int L,
    double lambda, int* finite_flag) {
  for (std::size_t i = blockIdx.x*blockDim.x+threadIdx.x;
       i < count; i += static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    int x,y,z;
    decode_index(i,L,x,y,z);
    double cx,cy,cz;
    curl_face_adjoint(electric,x,y,z,L,cx,cy,cz);
    const double bx = magnetic_before.x[i]-lambda*cx;
    const double by = magnetic_before.y[i]-lambda*cy;
    const double bz = magnetic_before.z[i]-lambda*cz;
    magnetic_after.x[i]=bx;
    magnetic_after.y[i]=by;
    magnetic_after.z[i]=bz;
    if (!isfinite(bx)||!isfinite(by)||!isfinite(bz)) atomicExch(finite_flag,0);
  }
}

__global__ void prepare_electric_kernel(
    DeviceVectorField electric_before, DeviceVectorField magnetic_after,
    DeviceVectorField electric_pre_current, std::size_t count, int L,
    double lambda, int* finite_flag) {
  for (std::size_t i = blockIdx.x*blockDim.x+threadIdx.x;
       i < count; i += static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    int x,y,z;
    decode_index(i,L,x,y,z);
    double cx,cy,cz;
    curl_edge(magnetic_after,x,y,z,L,cx,cy,cz);
    const double ex = electric_before.x[i]+lambda*cx;
    const double ey = electric_before.y[i]+lambda*cy;
    const double ez = electric_before.z[i]+lambda*cz;
    electric_pre_current.x[i]=ex;
    electric_pre_current.y[i]=ey;
    electric_pre_current.z[i]=ez;
    if (!isfinite(ex)||!isfinite(ey)||!isfinite(ez)) atomicExch(finite_flag,0);
  }
}

__global__ void apply_current_kernel(
    DeviceVectorField electric, const DeviceCurrentEntry* entries,
    std::size_t count, double scale, int* finite_flag) {
  for (std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;
       i<count;i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    const auto entry=entries[i];
    double* component=entry.axis==0?electric.x:(entry.axis==1?electric.y:electric.z);
    const double increment=scale*entry.value;
    atomicAdd(component+entry.index,increment);
    if (!isfinite(increment)) atomicExch(finite_flag,0);
  }
}

__global__ void apply_unique_current_kernel(
    DeviceVectorField electric, const DeviceCurrentEntry* entries,
    std::size_t count, double scale, int* finite_flag) {
  for (std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;
       i<count;i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    const auto entry=entries[i];
    double* component=entry.axis==0?electric.x:(entry.axis==1?electric.y:electric.z);
    const double increment=scale*entry.value;
    component[entry.index]+=increment;
    if (!isfinite(increment)) atomicExch(finite_flag,0);
  }
}

__global__ void apply_ordered_current_kernel(
    DeviceVectorField electric,const DeviceCurrentEntry* entries,
    const DeviceCurrentGroup* groups,std::size_t group_count,double scale,
    int* finite_flag) {
  for(std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;
      i<group_count;i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    const auto group=groups[i];
    double* component=group.axis==0?electric.x:
        (group.axis==1?electric.y:electric.z);
    double value=component[group.index];
    for(std::size_t item=0;item<group.count;++item) {
      const double increment=__dmul_rn(
          scale,entries[group.offset+item].value);
      value=__dadd_rn(value,increment);
      if(!isfinite(increment)||!isfinite(value)) atomicExch(finite_flag,0);
    }
    component[group.index]=value;
  }
}

__global__ void scatter_density_kernel(
    double* density,const DeviceDensityEntry* entries,std::size_t count) {
  for(std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;i<count;
      i+=static_cast<std::size_t>(blockDim.x)*gridDim.x)
    atomicAdd(density+entries[i].index,entries[i].value);
}

__global__ void gauss_residual_kernel(
    DeviceVectorField electric,const double* density,std::size_t count,
    int L,double* partial,int* finite_flag) {
  __shared__ double values[256];
  double local=0.0;
  for(std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;i<count;
      i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    int x,y,z;
    decode_index(i,L,x,y,z);
    const double divergence=electric.x[i]-electric.x[device_index(x-1,y,z,L)]
        +electric.y[i]-electric.y[device_index(x,y-1,z,L)]
        +electric.z[i]-electric.z[device_index(x,y,z-1,L)];
    const double residual=fabs(divergence-density[i]);
    local=max(local,residual);
    if(!isfinite(residual)) atomicExch(finite_flag,0);
  }
  values[threadIdx.x]=local;
  __syncthreads();
  for(int stride=blockDim.x/2;stride>0;stride>>=1) {
    if(threadIdx.x<stride) values[threadIdx.x]=max(
        values[threadIdx.x],values[threadIdx.x+stride]);
    __syncthreads();
  }
  if(threadIdx.x==0) partial[blockIdx.x]=values[0];
}

__global__ void local_translation_momentum_kernel(
    DeviceVectorField electric,DeviceVectorField magnetic,std::size_t count,
    int L,double* partial,int* finite_flag) {
  __shared__ double values[3][256];
  double local[3]={0.0,0.0,0.0};
  for(std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;i<count;
      i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    int x,y,z;
    decode_index(i,L,x,y,z);
    for(int axis=0;axis<3;++axis) {
      int px=x,py=y,pz=z,mx=x,my=y,mz=z;
      if(axis==0){++px;--mx;} else if(axis==1){++py;--my;} else{++pz;--mz;}
      double cpx,cpy,cpz,cmx,cmy,cmz;
      curl_edge(magnetic,px,py,pz,L,cpx,cpy,cpz);
      curl_edge(magnetic,mx,my,mz,L,cmx,cmy,cmz);
      const double value=0.5*(electric.x[i]*(cpx-cmx)
          +electric.y[i]*(cpy-cmy)+electric.z[i]*(cpz-cmz));
      local[axis]+=value;
      if(!isfinite(value)) atomicExch(finite_flag,0);
    }
  }
  for(int axis=0;axis<3;++axis)
    values[axis][threadIdx.x]=local[axis];
  __syncthreads();
  for(int stride=blockDim.x/2;stride>0;stride>>=1) {
    if(threadIdx.x<stride)
      for(int axis=0;axis<3;++axis)
        values[axis][threadIdx.x]+=values[axis][threadIdx.x+stride];
    __syncthreads();
  }
  if(threadIdx.x==0)
    for(int axis=0;axis<3;++axis)
      partial[axis*gridDim.x+blockIdx.x]=values[axis][0];
}

__global__ void integer_time_magnetic_kernel(
    DeviceVectorField electric,DeviceVectorField magnetic_half,
    DeviceVectorField magnetic_integer,std::size_t count,int L,
    double half_lambda,int* finite_flag) {
  for(std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;i<count;
      i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    int x,y,z;
    decode_index(i,L,x,y,z);
    double cx,cy,cz;
    curl_face_adjoint(electric,x,y,z,L,cx,cy,cz);
    const double bx=magnetic_half.x[i]-half_lambda*cx;
    const double by=magnetic_half.y[i]-half_lambda*cy;
    const double bz=magnetic_half.z[i]-half_lambda*cz;
    magnetic_integer.x[i]=bx;
    magnetic_integer.y[i]=by;
    magnetic_integer.z[i]=bz;
    if(!isfinite(bx)||!isfinite(by)||!isfinite(bz)) atomicExch(finite_flag,0);
  }
}

__global__ void convolve_axis_kernel(
    const double* input,double* output,std::size_t count,int L,int axis,
    DeviceStencil stencil) {
  for(std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;i<count;
      i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    int x,y,z;
    decode_index(i,L,x,y,z);
    double value=0.0;
    for(int item=0;item<stencil.count;++item) {
      int sx=x,sy=y,sz=z;
      if(axis==0)sx+=stencil.shifts[item];
      else if(axis==1)sy+=stencil.shifts[item];
      else sz+=stencil.shifts[item];
      value+=stencil.weights[item]*input[device_index(sx,sy,sz,L)];
    }
    output[i]=value;
  }
}

__global__ void dot_reduce_kernel(
    const double* left,const double* right,std::size_t count,double* partial,
    int* finite_flag) {
  __shared__ double values[256];
  double local=0.0;
  for(std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;i<count;
      i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    const double value=left[i]*right[i];
    local+=value;
    if(!isfinite(value)) atomicExch(finite_flag,0);
  }
  values[threadIdx.x]=local;
  __syncthreads();
  for(int stride=blockDim.x/2;stride>0;stride>>=1) {
    if(threadIdx.x<stride)values[threadIdx.x]+=values[threadIdx.x+stride];
    __syncthreads();
  }
  if(threadIdx.x==0)partial[blockIdx.x]=values[0];
}

__device__ __forceinline__ int periodic_abs_doubled_device(
    int coordinate2,int origin2,int L) {
  const int period=2*L;
  int delta=(coordinate2-origin2)%period;
  if(delta<0) delta+=period;
  return min(delta,period-delta);
}

__device__ __forceinline__ int component_radius2(
    int x2,int y2,int z2,int ox2,int oy2,int oz2,int L) {
  return max(periodic_abs_doubled_device(x2,ox2,L),
      max(periodic_abs_doubled_device(y2,oy2,L),
          periodic_abs_doubled_device(z2,oz2,L)));
}

__device__ __forceinline__ double field_energy_contributions(
    DeviceVectorField electric,DeviceVectorField magnetic,
    int x,int y,int z,int L,double lambda,double* values) {
  const auto i=device_index(x,y,z,L);
  double cbx,cby,cbz,cex,cey,cez;
  curl_edge(magnetic,x,y,z,L,cbx,cby,cbz);
  curl_face_adjoint(electric,x,y,z,L,cex,cey,cez);
  values[0]=0.5*electric.x[i]*electric.x[i]
      -0.25*lambda*electric.x[i]*cbx;
  values[1]=0.5*electric.y[i]*electric.y[i]
      -0.25*lambda*electric.y[i]*cby;
  values[2]=0.5*electric.z[i]*electric.z[i]
      -0.25*lambda*electric.z[i]*cbz;
  values[3]=0.5*magnetic.x[i]*magnetic.x[i]
      -0.25*lambda*magnetic.x[i]*cex;
  values[4]=0.5*magnetic.y[i]*magnetic.y[i]
      -0.25*lambda*magnetic.y[i]*cey;
  values[5]=0.5*magnetic.z[i]*magnetic.z[i]
      -0.25*lambda*magnetic.z[i]*cez;
  double total=0.0;
  #pragma unroll
  for(int c=0;c<6;++c) total+=values[c];
  return total;
}

__global__ void regional_profile_kernel(
    DeviceVectorField electric_before,DeviceVectorField magnetic_before,
    DeviceVectorField electric_pre,DeviceVectorField magnetic_after,
    DeviceVectorField electric_after,std::size_t count,int L,double lambda,
    int ox2,int oy2,int oz2,double* bins,double* partial,int* finite_flag) {
  extern __shared__ double shared[];
  const int bin_count=L+1;
  double* histogram=shared;
  double* reductions=histogram+3*bin_count;
  for(int i=threadIdx.x;i<3*bin_count;i+=blockDim.x) histogram[i]=0.0;
  __syncthreads();
  double local[3]={0.0,0.0,0.0};
  for(std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;
      i<count;i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    int x,y,z;
    decode_index(i,L,x,y,z);
    double values[6];
    DeviceVectorField electric[3]={electric_before,electric_pre,electric_after};
    DeviceVectorField magnetic[3]={magnetic_before,magnetic_after,magnetic_after};
    #pragma unroll
    for(int p=0;p<3;++p) {
      const double sum=field_energy_contributions(
          electric[p],magnetic[p],x,y,z,L,lambda,values);
      local[p]+=sum;
      if(!isfinite(sum)) atomicExch(finite_flag,0);
      const int radius[6]={
          component_radius2(2*x+1,2*y,2*z,ox2,oy2,oz2,L),
          component_radius2(2*x,2*y+1,2*z,ox2,oy2,oz2,L),
          component_radius2(2*x,2*y,2*z+1,ox2,oy2,oz2,L),
          component_radius2(2*x,2*y+1,2*z+1,ox2,oy2,oz2,L),
          component_radius2(2*x+1,2*y,2*z+1,ox2,oy2,oz2,L),
          component_radius2(2*x+1,2*y+1,2*z,ox2,oy2,oz2,L)};
      #pragma unroll
      for(int c=0;c<6;++c) atomicAdd(histogram+p*bin_count+radius[c],values[c]);
    }
  }
  #pragma unroll
  for(int p=0;p<3;++p) reductions[p*blockDim.x+threadIdx.x]=local[p];
  __syncthreads();
  for(int stride=blockDim.x/2;stride>0;stride>>=1) {
    if(threadIdx.x<stride) {
      #pragma unroll
      for(int p=0;p<3;++p)
        reductions[p*blockDim.x+threadIdx.x]
            +=reductions[p*blockDim.x+threadIdx.x+stride];
    }
    __syncthreads();
  }
  if(threadIdx.x==0) {
    #pragma unroll
    for(int p=0;p<3;++p) partial[p*gridDim.x+blockIdx.x]=reductions[p*blockDim.x];
  }
  __syncthreads();
  for(int i=threadIdx.x;i<3*bin_count;i+=blockDim.x) atomicAdd(bins+i,histogram[i]);
}

__global__ void deterministic_regional_profile_kernel(
    DeviceVectorField electric_before,DeviceVectorField magnetic_before,
    DeviceVectorField electric_pre,DeviceVectorField magnetic_after,
    DeviceVectorField electric_after,std::size_t count,int L,double lambda,
    int ox2,int oy2,int oz2,DeviceSelectedRadii selected,double* partial,
    int* finite_flag) {
  extern __shared__ double reductions[];
  double local[3][kDeterministicObserverQuantities]{};
  for(std::size_t i=blockIdx.x*blockDim.x+threadIdx.x;
      i<count;i+=static_cast<std::size_t>(blockDim.x)*gridDim.x) {
    int x,y,z;
    decode_index(i,L,x,y,z);
    const int radius2[6]={
        component_radius2(2*x+1,2*y,2*z,ox2,oy2,oz2,L),
        component_radius2(2*x,2*y+1,2*z,ox2,oy2,oz2,L),
        component_radius2(2*x,2*y,2*z+1,ox2,oy2,oz2,L),
        component_radius2(2*x,2*y+1,2*z+1,ox2,oy2,oz2,L),
        component_radius2(2*x+1,2*y,2*z+1,ox2,oy2,oz2,L),
        component_radius2(2*x+1,2*y+1,2*z,ox2,oy2,oz2,L)};
    double values[6];
    DeviceVectorField electric[3]={electric_before,electric_pre,electric_after};
    DeviceVectorField magnetic[3]={magnetic_before,magnetic_after,magnetic_after};
    #pragma unroll
    for(int p=0;p<3;++p) {
      const double sum=field_energy_contributions(
          electric[p],magnetic[p],x,y,z,L,lambda,values);
      local[p][0]+=sum;
      #pragma unroll
      for(int c=0;c<6;++c) {
        local[p][1]+=values[c];
        #pragma unroll
        for(int r=0;r<kDeterministicMaximumRadii;++r)
          if(r<selected.count&&radius2[c]<=selected.threshold2[r])
            local[p][2+r]+=values[c];
      }
      if(!isfinite(sum)) atomicExch(finite_flag,0);
    }
  }
  #pragma unroll
  for(int p=0;p<3;++p)
    #pragma unroll
    for(int q=0;q<kDeterministicObserverQuantities;++q)
      reductions[(p*kDeterministicObserverQuantities+q)*blockDim.x
          +threadIdx.x]=local[p][q];
  __syncthreads();
  for(int stride=blockDim.x/2;stride>0;stride>>=1) {
    if(threadIdx.x<stride) {
      #pragma unroll
      for(int p=0;p<3;++p)
        #pragma unroll
        for(int q=0;q<kDeterministicObserverQuantities;++q)
          reductions[(p*kDeterministicObserverQuantities+q)*blockDim.x
              +threadIdx.x]
              +=reductions[(p*kDeterministicObserverQuantities+q)*blockDim.x
                  +threadIdx.x+stride];
    }
    __syncthreads();
  }
  if(threadIdx.x==0) {
    #pragma unroll
    for(int p=0;p<3;++p)
      #pragma unroll
      for(int q=0;q<kDeterministicObserverQuantities;++q)
        partial[(p*kDeterministicObserverQuantities+q)*gridDim.x+blockIdx.x]
            =reductions[(p*kDeterministicObserverQuantities+q)*blockDim.x];
  }
}

double milliseconds_since(const std::chrono::steady_clock::time_point& start) {
  return std::chrono::duration<double,std::milli>(
      std::chrono::steady_clock::now()-start).count();
}

double* device_component(DeviceVectorField field,int axis) {
  return axis==0?field.x:(axis==1?field.y:field.z);
}

}  // namespace

struct CudaMatchedFieldPipeline::Impl {
  int L=0;
  std::size_t count=0;
  bool ok=false;
  bool prepared=false;
  bool current_applied=false;
  double lambda=0.0;
  std::string last_error;
  CudaMatchedFieldTimings timing{};
  DeviceVectorField e0{},b0{},b1{},epre{},e1{};
  DeviceVectorField bint0{},bint1{};
  DeviceCurrentEntry* current_entries=nullptr;
  std::size_t current_capacity=0;
  DeviceCurrentGroup* current_groups=nullptr;
  std::size_t current_group_capacity=0;
  DeviceDensityEntry* density_entries=nullptr;
  std::size_t density_capacity=0;
  double* rho0=nullptr;
  double* rho1=nullptr;
  double* scratch0=nullptr;
  double* scratch1=nullptr;
  double* bins=nullptr;
  double* partial=nullptr;
  int partial_blocks=0;
  int* finite_flag=nullptr;
  bool observed=false;
  std::array<double,3> observed_energy{};

  bool check(cudaError_t status,const char* context) {
    if(status==cudaSuccess) return true;
    last_error=std::string(context)+": "+cudaGetErrorString(status);
    ok=false;
    return false;
  }

  bool allocate_field(DeviceVectorField& field) {
    const std::size_t bytes=count*sizeof(double);
    return check(cudaMalloc(&field.x,bytes),"cudaMalloc field.x")
        &&check(cudaMalloc(&field.y,bytes),"cudaMalloc field.y")
        &&check(cudaMalloc(&field.z,bytes),"cudaMalloc field.z");
  }

  void free_field(DeviceVectorField& field) {
    cudaFree(field.x); cudaFree(field.y); cudaFree(field.z);
    field={};
  }

  explicit Impl(int size):L(size) {
    if(L<5) { last_error="L must be at least five"; return; }
    int devices=0;
    if(!check(cudaGetDeviceCount(&devices),"cudaGetDeviceCount")||devices<1) {
      if(devices<1) last_error="no CUDA device";
      return;
    }
    count=static_cast<std::size_t>(L)*L*L;
    partial_blocks=std::min<int>(2048,
        static_cast<int>((count+255)/256));
    const std::size_t bytes=count*sizeof(double);
    ok=allocate_field(e0)&&allocate_field(b0)&&allocate_field(b1)
        &&allocate_field(epre)&&allocate_field(e1)
        &&allocate_field(bint0)&&allocate_field(bint1)
        &&check(cudaMalloc(&rho0,bytes),"cudaMalloc rho0")
        &&check(cudaMalloc(&rho1,bytes),"cudaMalloc rho1")
        &&check(cudaMalloc(&scratch0,bytes),"cudaMalloc scratch0")
        &&check(cudaMalloc(&scratch1,bytes),"cudaMalloc scratch1")
        &&check(cudaMalloc(&bins,3*static_cast<std::size_t>(L+1)*sizeof(double)),
                 "cudaMalloc bins")
        &&check(cudaMalloc(&partial,3*kDeterministicObserverQuantities
                 *static_cast<std::size_t>(partial_blocks)
                 *sizeof(double)),"cudaMalloc partial")
        &&check(cudaMalloc(&finite_flag,sizeof(int)),"cudaMalloc finite flag");
  }

  ~Impl() {
    free_field(e0); free_field(b0); free_field(b1); free_field(epre); free_field(e1);
    free_field(bint0); free_field(bint1);
    cudaFree(current_entries); cudaFree(current_groups); cudaFree(density_entries);
    cudaFree(rho0); cudaFree(rho1); cudaFree(scratch0); cudaFree(scratch1);
    cudaFree(bins); cudaFree(partial); cudaFree(finite_flag);
  }

  bool copy_to_device(DeviceVectorField field,const MatchedFaceFlux& host) {
    const std::size_t bytes=count*sizeof(double);
    return check(cudaMemcpy(field.x,host.x.data(),bytes,cudaMemcpyHostToDevice),"upload x")
        &&check(cudaMemcpy(field.y,host.y.data(),bytes,cudaMemcpyHostToDevice),"upload y")
        &&check(cudaMemcpy(field.z,host.z.data(),bytes,cudaMemcpyHostToDevice),"upload z");
  }

  bool copy_to_device(DeviceVectorField field,const MatchedEdgeField& host) {
    const std::size_t bytes=count*sizeof(double);
    return check(cudaMemcpy(field.x,host.x.data(),bytes,cudaMemcpyHostToDevice),"upload x")
        &&check(cudaMemcpy(field.y,host.y.data(),bytes,cudaMemcpyHostToDevice),"upload y")
        &&check(cudaMemcpy(field.z,host.z.data(),bytes,cudaMemcpyHostToDevice),"upload z");
  }

  template<class HostField>
  bool copy_to_host(HostField& host,DeviceVectorField field) {
    if(host.L!=L||host.x.size()!=count||host.y.size()!=count
        ||host.z.size()!=count) host=HostField(L);
    const std::size_t bytes=count*sizeof(double);
    return check(cudaMemcpy(host.x.data(),field.x,bytes,cudaMemcpyDeviceToHost),"download x")
        &&check(cudaMemcpy(host.y.data(),field.y,bytes,cudaMemcpyDeviceToHost),"download y")
        &&check(cudaMemcpy(host.z.data(),field.z,bytes,cudaMemcpyDeviceToHost),"download z");
  }
};

CudaMatchedFieldPipeline::CudaMatchedFieldPipeline(int L)
    : impl_(std::make_unique<Impl>(L)) {}
CudaMatchedFieldPipeline::~CudaMatchedFieldPipeline()=default;
CudaMatchedFieldPipeline::CudaMatchedFieldPipeline(
    CudaMatchedFieldPipeline&&) noexcept=default;
CudaMatchedFieldPipeline& CudaMatchedFieldPipeline::operator=(
    CudaMatchedFieldPipeline&&) noexcept=default;

bool CudaMatchedFieldPipeline::valid() const { return impl_&&impl_->ok; }
int CudaMatchedFieldPipeline::size() const { return impl_?impl_->L:0; }
const char* CudaMatchedFieldPipeline::error() const {
  return impl_?impl_->last_error.c_str():"missing implementation";
}
const CudaMatchedFieldTimings& CudaMatchedFieldPipeline::timings() const {
  static const CudaMatchedFieldTimings empty{};
  return impl_?impl_->timing:empty;
}

CudaMatchedFieldResidentViews
CudaMatchedFieldPipeline::resident_views() const {
  CudaMatchedFieldResidentViews result;
  if(!valid()) return result;
  const auto view=[&](DeviceVectorField field) {
    return CudaMatchedFieldDeviceView{
        impl_->L,field.x,field.y,field.z};
  };
  result.electric_before=view(impl_->e0);
  result.magnetic_before=view(impl_->b0);
  result.magnetic_prepared=view(impl_->b1);
  result.electric_pre_current=view(impl_->epre);
  result.electric_after=view(impl_->e1);
  result.prepared=impl_->prepared;
  result.current_applied=impl_->current_applied;
  return result;
}

bool CudaMatchedFieldPipeline::upload(
    const MatchedFaceFlux& electric,const MatchedEdgeField& magnetic_half) {
  if(!valid()||electric.L!=impl_->L||magnetic_half.L!=impl_->L) return false;
  const auto start=std::chrono::steady_clock::now();
  const bool result=impl_->copy_to_device(impl_->e0,electric)
      &&impl_->copy_to_device(impl_->b0,magnetic_half);
  impl_->timing.upload_ms+=milliseconds_since(start);
  impl_->prepared=false; impl_->current_applied=false; impl_->observed=false;
  return result;
}

bool CudaMatchedFieldPipeline::prepare_forward(double lambda) {
  if(!valid()||!std::isfinite(lambda)||!(lambda>0.0)) return false;
  const auto start=std::chrono::steady_clock::now();
  const int one=1;
  if(!impl_->check(cudaMemcpy(impl_->finite_flag,&one,sizeof(int),cudaMemcpyHostToDevice),
                   "reset finite flag")) return false;
  const int threads=256;
  const int blocks=std::min<int>(4096,
      static_cast<int>((impl_->count+threads-1)/threads));
  prepare_magnetic_kernel<<<blocks,threads>>>(impl_->e0,impl_->b0,impl_->b1,
      impl_->count,impl_->L,lambda,impl_->finite_flag);
  if(!impl_->check(cudaGetLastError(),"prepare magnetic launch")) return false;
  prepare_electric_kernel<<<blocks,threads>>>(impl_->e0,impl_->b1,impl_->epre,
      impl_->count,impl_->L,lambda,impl_->finite_flag);
  if(!impl_->check(cudaGetLastError(),"prepare electric launch")) return false;
  const std::size_t bytes=impl_->count*sizeof(double);
  if(!impl_->check(cudaMemcpy(impl_->e1.x,impl_->epre.x,bytes,cudaMemcpyDeviceToDevice),
                   "seed e1.x")
      ||!impl_->check(cudaMemcpy(impl_->e1.y,impl_->epre.y,bytes,cudaMemcpyDeviceToDevice),
                   "seed e1.y")
      ||!impl_->check(cudaMemcpy(impl_->e1.z,impl_->epre.z,bytes,cudaMemcpyDeviceToDevice),
                   "seed e1.z")) return false;
  int finite=0;
  if(!impl_->check(cudaMemcpy(&finite,impl_->finite_flag,sizeof(int),cudaMemcpyDeviceToHost),
                   "read finite flag")) return false;
  impl_->timing.prepare_ms+=milliseconds_since(start);
  impl_->lambda=lambda; impl_->prepared=finite!=0; impl_->current_applied=false;
  impl_->observed=false;
  if(!impl_->prepared) impl_->last_error="non-finite CUDA field preparation";
  return impl_->prepared;
}

bool CudaMatchedFieldPipeline::download_prepared(
    MatchedEdgeField& magnetic_after,MatchedFaceFlux& electric_pre_current) {
  if(!valid()||!impl_->prepared) return false;
  const auto start=std::chrono::steady_clock::now();
  const bool result=impl_->copy_to_host(magnetic_after,impl_->b1)
      &&impl_->copy_to_host(electric_pre_current,impl_->epre);
  impl_->timing.download_ms+=milliseconds_since(start);
  return result;
}

bool CudaMatchedFieldPipeline::apply_sparse_current(
    const std::vector<QuadraticCoatFaceCurrent>& segments,double polarity_scale) {
  if(!valid()||!impl_->prepared||!std::isfinite(polarity_scale)) return false;
  std::vector<DeviceCurrentEntry> entries;
  for(const auto& segment:segments) {
    if(!segment.valid||segment.L!=impl_->L||segment.dense_materialized) {
      impl_->last_error="CUDA pipeline requires valid sparse current segments";
      return false;
    }
    for(const auto& entry:segment.sparse_current) {
      if(entry.axis<0||entry.axis>2||!std::isfinite(entry.value)) return false;
      entries.push_back({static_cast<std::size_t>(segment.index(
          entry.face.x,entry.face.y,entry.face.z)),entry.axis,entry.value});
    }
  }
  const auto start=std::chrono::steady_clock::now();
  if(entries.size()>impl_->current_capacity) {
    cudaFree(impl_->current_entries); impl_->current_entries=nullptr;
    impl_->current_capacity=std::max<std::size_t>(64,entries.size());
    if(!impl_->check(cudaMalloc(&impl_->current_entries,
        impl_->current_capacity*sizeof(DeviceCurrentEntry)),"allocate current entries"))
      return false;
  }
  if(!entries.empty()) {
    if(!impl_->check(cudaMemcpy(impl_->current_entries,entries.data(),
        entries.size()*sizeof(DeviceCurrentEntry),cudaMemcpyHostToDevice),
        "upload current entries")) return false;
    const int one=1;
    if(!impl_->check(cudaMemcpy(impl_->finite_flag,&one,sizeof(int),cudaMemcpyHostToDevice),
                     "reset current finite flag")) return false;
    const int threads=256;
    const int blocks=static_cast<int>((entries.size()+threads-1)/threads);
    apply_current_kernel<<<blocks,threads>>>(impl_->e1,impl_->current_entries,
        entries.size(),-polarity_scale,impl_->finite_flag);
    if(!impl_->check(cudaGetLastError(),"apply current launch")) return false;
    int finite=0;
    if(!impl_->check(cudaMemcpy(&finite,impl_->finite_flag,sizeof(int),cudaMemcpyDeviceToHost),
                     "read current finite flag")||!finite) return false;
  }
  impl_->timing.current_ms+=milliseconds_since(start);
  impl_->current_applied=true;
  impl_->observed=false;
  return true;
}

bool CudaMatchedFieldPipeline::apply_canonical_sparse_current(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    double polarity_scale) {
  if(!valid()||!impl_->prepared||!std::isfinite(polarity_scale)
      ||segments.empty()) return false;
  const auto current=aggregate_quadratic_coat_face_current(
      segments,polarity_scale,0.0);
  if(!current.valid||current.L!=impl_->L) {
    impl_->last_error="canonical current aggregation failed";
    return false;
  }
  std::vector<DeviceCurrentEntry> entries;
  entries.reserve(current.entries.size());
  for(const auto& entry:current.entries) {
    if(entry.axis<0||entry.axis>2||!std::isfinite(entry.value)) return false;
    entries.push_back({static_cast<std::size_t>(segments.front().index(
        entry.face.x,entry.face.y,entry.face.z)),entry.axis,entry.value});
  }
  const auto start=std::chrono::steady_clock::now();
  if(entries.size()>impl_->current_capacity) {
    cudaFree(impl_->current_entries); impl_->current_entries=nullptr;
    impl_->current_capacity=std::max<std::size_t>(64,entries.size());
    if(!impl_->check(cudaMalloc(&impl_->current_entries,
        impl_->current_capacity*sizeof(DeviceCurrentEntry)),
        "allocate canonical current entries")) return false;
  }
  if(!entries.empty()) {
    if(!impl_->check(cudaMemcpy(impl_->current_entries,entries.data(),
        entries.size()*sizeof(DeviceCurrentEntry),cudaMemcpyHostToDevice),
        "upload canonical current entries")) return false;
    const int one=1;
    if(!impl_->check(cudaMemcpy(impl_->finite_flag,&one,sizeof(int),
        cudaMemcpyHostToDevice),"reset canonical current finite flag"))
      return false;
    const int threads=256;
    const int blocks=static_cast<int>((entries.size()+threads-1)/threads);
    apply_unique_current_kernel<<<blocks,threads>>>(
        impl_->e1,impl_->current_entries,entries.size(),-1.0,
        impl_->finite_flag);
    if(!impl_->check(cudaGetLastError(),"apply canonical current launch"))
      return false;
    int finite=0;
    if(!impl_->check(cudaMemcpy(&finite,impl_->finite_flag,sizeof(int),
        cudaMemcpyDeviceToHost),"read canonical current finite flag")
        ||!finite) return false;
  }
  impl_->timing.current_ms+=milliseconds_since(start);
  impl_->current_applied=true;
  impl_->observed=false;
  return true;
}

bool CudaMatchedFieldPipeline::apply_ordered_sparse_current(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    double polarity_scale) {
  if(!valid()||!impl_->prepared||!std::isfinite(polarity_scale)
      ||segments.empty()) return false;
  using Key=std::pair<int,std::size_t>;
  std::map<Key,std::vector<double>> grouped;
  for(const auto& segment:segments) {
    if(!segment.valid||segment.L!=impl_->L||segment.dense_materialized) {
      impl_->last_error="ordered CUDA current requires valid sparse segments";
      return false;
    }
    for(const auto& entry:segment.sparse_current) {
      if(entry.axis<0||entry.axis>2||!std::isfinite(entry.value)) return false;
      const auto index=static_cast<std::size_t>(segment.index(
          entry.face.x,entry.face.y,entry.face.z));
      grouped[{entry.axis,index}].push_back(entry.value);
    }
  }
  std::vector<DeviceCurrentEntry> entries;
  std::vector<DeviceCurrentGroup> groups;
  for(const auto& [key,values]:grouped) {
    const std::size_t offset=entries.size();
    for(const double value:values)
      entries.push_back({key.second,key.first,value});
    groups.push_back({key.second,offset,values.size(),key.first});
  }
  const auto start=std::chrono::steady_clock::now();
  if(entries.size()>impl_->current_capacity) {
    cudaFree(impl_->current_entries); impl_->current_entries=nullptr;
    impl_->current_capacity=std::max<std::size_t>(64,entries.size());
    if(!impl_->check(cudaMalloc(&impl_->current_entries,
        impl_->current_capacity*sizeof(DeviceCurrentEntry)),
        "allocate ordered current entries")) return false;
  }
  if(groups.size()>impl_->current_group_capacity) {
    cudaFree(impl_->current_groups); impl_->current_groups=nullptr;
    impl_->current_group_capacity=std::max<std::size_t>(64,groups.size());
    if(!impl_->check(cudaMalloc(&impl_->current_groups,
        impl_->current_group_capacity*sizeof(DeviceCurrentGroup)),
        "allocate ordered current groups")) return false;
  }
  if(!entries.empty()) {
    if(!impl_->check(cudaMemcpy(impl_->current_entries,entries.data(),
        entries.size()*sizeof(DeviceCurrentEntry),cudaMemcpyHostToDevice),
        "upload ordered current entries")
        ||!impl_->check(cudaMemcpy(impl_->current_groups,groups.data(),
        groups.size()*sizeof(DeviceCurrentGroup),cudaMemcpyHostToDevice),
        "upload ordered current groups")) return false;
    const int one=1;
    if(!impl_->check(cudaMemcpy(impl_->finite_flag,&one,sizeof(int),
        cudaMemcpyHostToDevice),"reset ordered current finite flag"))
      return false;
    const int threads=256;
    const int blocks=static_cast<int>((groups.size()+threads-1)/threads);
    apply_ordered_current_kernel<<<blocks,threads>>>(impl_->e1,
        impl_->current_entries,impl_->current_groups,groups.size(),
        -polarity_scale,impl_->finite_flag);
    if(!impl_->check(cudaGetLastError(),"apply ordered current launch"))
      return false;
    int finite=0;
    if(!impl_->check(cudaMemcpy(&finite,impl_->finite_flag,sizeof(int),
        cudaMemcpyDeviceToHost),"read ordered current finite flag")
        ||!finite) return false;
  }
  impl_->timing.current_ms+=milliseconds_since(start);
  impl_->current_applied=true;
  impl_->observed=false;
  return true;
}

BatchedRegionalEnergyProfile CudaMatchedFieldPipeline::observe(
    double lambda,const Vec3& center,const std::vector<int>& radii,double tolerance) {
  BatchedRegionalEnergyProfile result;
  result.L=size(); result.center=center; result.lambda=lambda;
  const bool ordered=std::adjacent_find(radii.begin(),radii.end(),
      [](int a,int b){return a>=b;})==radii.end();
  if(!valid()||!impl_->prepared||!impl_->current_applied
      ||lambda!=impl_->lambda||radii.empty()||!ordered||radii.front()<0
      ||!std::isfinite(tolerance)||!(tolerance>0.0)
      ||center.x!=std::round(center.x)||center.y!=std::round(center.y)
      ||center.z!=std::round(center.z)) return result;
  const auto start=std::chrono::steady_clock::now();
  const std::size_t bin_values=3*static_cast<std::size_t>(impl_->L+1);
  if(!impl_->check(cudaMemset(impl_->bins,0,bin_values*sizeof(double)),"clear bins"))
    return result;
  const int one=1;
  if(!impl_->check(cudaMemcpy(impl_->finite_flag,&one,sizeof(int),cudaMemcpyHostToDevice),
                   "reset profile finite flag")) return result;
  const int threads=256;
  const std::size_t shared=(3*static_cast<std::size_t>(impl_->L+1)
      +3*threads)*sizeof(double);
  regional_profile_kernel<<<impl_->partial_blocks,threads,shared>>>(
      impl_->e0,impl_->b0,impl_->epre,impl_->b1,impl_->e1,impl_->count,
      impl_->L,lambda,static_cast<int>(2*center.x),static_cast<int>(2*center.y),
      static_cast<int>(2*center.z),impl_->bins,impl_->partial,impl_->finite_flag);
  if(!impl_->check(cudaGetLastError(),"regional profile launch")) return result;
  std::vector<double> host_bins(bin_values);
  std::vector<double> host_partial(
      3*static_cast<std::size_t>(impl_->partial_blocks));
  int finite=0;
  if(!impl_->check(cudaMemcpy(host_bins.data(),impl_->bins,
      host_bins.size()*sizeof(double),cudaMemcpyDeviceToHost),"download bins")
      ||!impl_->check(cudaMemcpy(host_partial.data(),impl_->partial,
      host_partial.size()*sizeof(double),cudaMemcpyDeviceToHost),"download partials")
      ||!impl_->check(cudaMemcpy(&finite,impl_->finite_flag,sizeof(int),
      cudaMemcpyDeviceToHost),"download profile finite flag")||!finite) return result;

  std::array<double,3> total{};
  std::array<double,3> bin_total{};
  std::array<std::vector<double>,3> cumulative;
  double partition=0.0;
  for(int p=0;p<3;++p) {
    long double exact=0.0L,from_bins=0.0L,running=0.0L;
    for(int block=0;block<impl_->partial_blocks;++block)
      exact+=host_partial[static_cast<std::size_t>(p)*impl_->partial_blocks+block];
    cumulative[p].resize(static_cast<std::size_t>(impl_->L+1));
    for(int bin=0;bin<=impl_->L;++bin) {
      const double value=host_bins[static_cast<std::size_t>(p)*(impl_->L+1)+bin];
      from_bins+=value; running+=value;
      cumulative[p][static_cast<std::size_t>(bin)]=static_cast<double>(running);
    }
    total[p]=static_cast<double>(exact);
    bin_total[p]=static_cast<double>(from_bins);
    partition=std::max(partition,std::abs(total[p]-bin_total[p]));
  }
  const double global=std::abs(total[1]-total[0]);
  result.energy_before=total[0];
  result.energy_pre_current=total[1];
  result.energy_after=total[2];
  impl_->observed_energy=total;
  impl_->observed=true;
  result.maximum_scalar_equivalence_residual=partition;
  result.regions.resize(radii.size()); result.valid=true;
  for(std::size_t r=0;r<radii.size();++r) {
    auto& record=result.regions[r];
    const int bin=std::min(2*radii[r],impl_->L);
    record.L=impl_->L; record.center=center;
    record.chebyshev_radius=radii[r]; record.lambda=lambda;
    record.energy_before=cumulative[0][static_cast<std::size_t>(bin)];
    record.energy_pre_current=cumulative[1][static_cast<std::size_t>(bin)];
    record.energy_after=cumulative[2][static_cast<std::size_t>(bin)];
    record.boundary_transport_into=record.energy_pre_current-record.energy_before;
    record.source_exchange_into_field=record.energy_after-record.energy_pre_current;
    record.energy_change=record.energy_after-record.energy_before;
    record.magnetic_update_residual=0.0;
    record.electric_pre_update_residual=0.0;
    record.global_source_free_residual=global;
    record.partition_residual=partition;
    record.regional_ledger_residual=std::abs(record.energy_change
        -record.boundary_transport_into-record.source_exchange_into_field);
    record.valid=global<=tolerance&&partition<=tolerance
        &&record.regional_ledger_residual<=tolerance
        &&std::isfinite(record.energy_before)
        &&std::isfinite(record.energy_pre_current)
        &&std::isfinite(record.energy_after);
    result.valid=result.valid&&record.valid;
  }
  impl_->observed=result.valid;
  impl_->timing.observe_ms+=milliseconds_since(start);
  return result;
}

BatchedRegionalEnergyProfile CudaMatchedFieldPipeline::observe_deterministic(
    double lambda,const Vec3& center,const std::vector<int>& radii,
    double tolerance) {
  BatchedRegionalEnergyProfile result;
  result.L=size(); result.center=center; result.lambda=lambda;
  const bool ordered=std::adjacent_find(radii.begin(),radii.end(),
      [](int a,int b){return a>=b;})==radii.end();
  if(!valid()||!impl_->prepared||!impl_->current_applied
      ||lambda!=impl_->lambda||radii.empty()
      ||radii.size()>kDeterministicMaximumRadii||!ordered||radii.front()<0
      ||!std::isfinite(tolerance)||!(tolerance>0.0)
      ||center.x!=std::round(center.x)||center.y!=std::round(center.y)
      ||center.z!=std::round(center.z)) return result;
  DeviceSelectedRadii selected;
  selected.count=static_cast<int>(radii.size());
  for(std::size_t r=0;r<radii.size();++r) {
    const long long doubled=2LL*static_cast<long long>(radii[r]);
    selected.threshold2[r]=static_cast<int>(std::min<long long>(impl_->L,doubled));
  }
  const auto start=std::chrono::steady_clock::now();
  const int one=1;
  if(!impl_->check(cudaMemcpy(impl_->finite_flag,&one,sizeof(int),
      cudaMemcpyHostToDevice),"reset deterministic profile finite flag"))
    return result;
  constexpr int threads=128;
  const std::size_t shared=3*kDeterministicObserverQuantities
      *static_cast<std::size_t>(threads)*sizeof(double);
  deterministic_regional_profile_kernel<<<impl_->partial_blocks,threads,shared>>>(
      impl_->e0,impl_->b0,impl_->epre,impl_->b1,impl_->e1,impl_->count,
      impl_->L,lambda,static_cast<int>(2*center.x),static_cast<int>(2*center.y),
      static_cast<int>(2*center.z),selected,impl_->partial,impl_->finite_flag);
  if(!impl_->check(cudaGetLastError(),"deterministic regional profile launch"))
    return result;
  const std::size_t partial_count=3*kDeterministicObserverQuantities
      *static_cast<std::size_t>(impl_->partial_blocks);
  std::vector<double> host_partial(partial_count);
  int finite=0;
  if(!impl_->check(cudaMemcpy(host_partial.data(),impl_->partial,
      host_partial.size()*sizeof(double),cudaMemcpyDeviceToHost),
      "download deterministic profile partials")
      ||!impl_->check(cudaMemcpy(&finite,impl_->finite_flag,sizeof(int),
      cudaMemcpyDeviceToHost),"download deterministic profile finite flag")
      ||!finite) return result;

  std::array<double,3> total{},component_total{};
  std::array<std::array<double,kDeterministicMaximumRadii>,3> inside{};
  double partition=0.0;
  const auto reduce=[&](int p,int q) {
    long double value=0.0L;
    const std::size_t base=static_cast<std::size_t>(
        p*kDeterministicObserverQuantities+q)*impl_->partial_blocks;
    for(int block=0;block<impl_->partial_blocks;++block)
      value+=host_partial[base+static_cast<std::size_t>(block)];
    return static_cast<double>(value);
  };
  for(int p=0;p<3;++p) {
    total[p]=reduce(p,0);
    component_total[p]=reduce(p,1);
    partition=std::max(partition,std::abs(total[p]-component_total[p]));
    for(std::size_t r=0;r<radii.size();++r)
      inside[p][r]=reduce(p,2+static_cast<int>(r));
  }
  const double global=std::abs(total[1]-total[0]);
  result.energy_before=total[0];
  result.energy_pre_current=total[1];
  result.energy_after=total[2];
  impl_->observed_energy=total;
  impl_->observed=true;
  result.maximum_scalar_equivalence_residual=partition;
  result.regions.resize(radii.size()); result.valid=true;
  for(std::size_t r=0;r<radii.size();++r) {
    auto& record=result.regions[r];
    record.L=impl_->L; record.center=center;
    record.chebyshev_radius=radii[r]; record.lambda=lambda;
    record.energy_before=inside[0][r];
    record.energy_pre_current=inside[1][r];
    record.energy_after=inside[2][r];
    record.boundary_transport_into=record.energy_pre_current-record.energy_before;
    record.source_exchange_into_field=record.energy_after-record.energy_pre_current;
    record.energy_change=record.energy_after-record.energy_before;
    record.magnetic_update_residual=0.0;
    record.electric_pre_update_residual=0.0;
    record.global_source_free_residual=global;
    record.partition_residual=partition;
    record.regional_ledger_residual=std::abs(record.energy_change
        -record.boundary_transport_into-record.source_exchange_into_field);
    record.valid=global<=tolerance&&partition<=tolerance
        &&record.regional_ledger_residual<=tolerance
        &&std::isfinite(record.energy_before)
        &&std::isfinite(record.energy_pre_current)
        &&std::isfinite(record.energy_after);
    result.valid=result.valid&&record.valid;
  }
  impl_->observed=result.valid;
  impl_->timing.observe_ms+=milliseconds_since(start);
  return result;
}

ConnectedMooreBlockVolumeDiagnostics
CudaMatchedFieldPipeline::diagnose_common_action(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    double polarity_scale,double interaction_scale,double wave_speed,
    double dt,double tolerance) {
  ConnectedMooreBlockVolumeDiagnostics result;
  if(!valid()||!impl_->prepared||!impl_->current_applied||!impl_->observed
      ||segments.empty()||!(polarity_scale>0.0)
      ||!(interaction_scale>0.0)||!(wave_speed>0.0)||!(dt>0.0)
      ||!(tolerance>0.0)||!std::isfinite(polarity_scale)
      ||!std::isfinite(interaction_scale)||!std::isfinite(wave_speed)
      ||!std::isfinite(dt)||!std::isfinite(tolerance)
      ||std::abs(wave_speed*dt-impl_->lambda)>1e-15) return result;
  for(const auto& segment:segments)
    if(!segment.valid||segment.L!=impl_->L||segment.dense_materialized)
      return result;

  std::vector<DeviceDensityEntry> before,after;
  for(const auto& segment:segments) {
    for(std::size_t item=0;item<segment.start_coat.weight_count;++item) {
      const auto& entry=segment.start_coat.weights[item];
      before.push_back({static_cast<std::size_t>(segment.index(
          entry.site.x,entry.site.y,entry.site.z)),
          polarity_scale*entry.weight});
    }
    for(std::size_t item=0;item<segment.end_coat.weight_count;++item) {
      const auto& entry=segment.end_coat.weights[item];
      after.push_back({static_cast<std::size_t>(segment.index(
          entry.site.x,entry.site.y,entry.site.z)),
          polarity_scale*entry.weight});
    }
  }
  const std::size_t density_count=std::max(before.size(),after.size());
  if(density_count>impl_->density_capacity) {
    cudaFree(impl_->density_entries); impl_->density_entries=nullptr;
    impl_->density_capacity=std::max<std::size_t>(64,density_count);
    if(!impl_->check(cudaMalloc(&impl_->density_entries,
        impl_->density_capacity*sizeof(DeviceDensityEntry)),
        "allocate density entries")) return result;
  }
  const std::size_t bytes=impl_->count*sizeof(double);
  if(!impl_->check(cudaMemset(impl_->rho0,0,bytes),"clear rho0")
      ||!impl_->check(cudaMemset(impl_->rho1,0,bytes),"clear rho1"))
    return result;
  const int threads=256;
  const int blocks=impl_->partial_blocks;
  const auto scatter=[&](const std::vector<DeviceDensityEntry>& entries,
                         double* density) {
    if(entries.empty()) return true;
    if(!impl_->check(cudaMemcpy(impl_->density_entries,entries.data(),
        entries.size()*sizeof(DeviceDensityEntry),cudaMemcpyHostToDevice),
        "upload density entries")) return false;
    const int scatter_blocks=static_cast<int>((entries.size()+threads-1)/threads);
    scatter_density_kernel<<<scatter_blocks,threads>>>(
        density,impl_->density_entries,entries.size());
    return impl_->check(cudaGetLastError(),"scatter density launch");
  };
  if(!scatter(before,impl_->rho0)||!scatter(after,impl_->rho1)) return result;

  const int one=1;
  if(!impl_->check(cudaMemcpy(impl_->finite_flag,&one,sizeof(int),
      cudaMemcpyHostToDevice),"reset diagnostic finite flag")) return result;
  std::vector<double> host_partial(
      3*static_cast<std::size_t>(impl_->partial_blocks));
  const auto gauss=[&](DeviceVectorField electric,const double* density,
                       double& output) {
    gauss_residual_kernel<<<blocks,threads>>>(electric,density,impl_->count,
        impl_->L,impl_->partial,impl_->finite_flag);
    if(!impl_->check(cudaGetLastError(),"gauss residual launch")
        ||!impl_->check(cudaMemcpy(host_partial.data(),impl_->partial,
            static_cast<std::size_t>(blocks)*sizeof(double),
            cudaMemcpyDeviceToHost),"download gauss partials")) return false;
    output=0.0;
    for(int block=0;block<blocks;++block)
      output=std::max(output,host_partial[static_cast<std::size_t>(block)]);
    return true;
  };
  if(!gauss(impl_->e0,impl_->rho0,result.gauss_before_residual)
      ||!gauss(impl_->e1,impl_->rho1,result.gauss_after_residual))
    return result;

  const auto local_momentum=[&](DeviceVectorField electric,
                                DeviceVectorField magnetic,Vec3& output) {
    local_translation_momentum_kernel<<<blocks,threads>>>(electric,magnetic,
        impl_->count,impl_->L,impl_->partial,impl_->finite_flag);
    if(!impl_->check(cudaGetLastError(),"local momentum launch")
        ||!impl_->check(cudaMemcpy(host_partial.data(),impl_->partial,
            host_partial.size()*sizeof(double),cudaMemcpyDeviceToHost),
            "download local momentum partials")) return false;
    std::array<long double,3> sum{};
    for(int axis=0;axis<3;++axis)
      for(int block=0;block<blocks;++block)
        sum[axis]+=host_partial[static_cast<std::size_t>(axis)*blocks+block];
    output={interaction_scale*static_cast<double>(sum[0]),
            interaction_scale*static_cast<double>(sum[1]),
            interaction_scale*static_cast<double>(sum[2])};
    return true;
  };
  if(!local_momentum(impl_->e0,impl_->b0,
          result.local_field_momentum_before)
      ||!local_momentum(impl_->e1,impl_->b1,
          result.local_field_momentum_after)) return result;

  integer_time_magnetic_kernel<<<blocks,threads>>>(impl_->e0,impl_->b0,
      impl_->bint0,impl_->count,impl_->L,0.5*wave_speed*dt,
      impl_->finite_flag);
  integer_time_magnetic_kernel<<<blocks,threads>>>(impl_->e1,impl_->b1,
      impl_->bint1,impl_->count,impl_->L,0.5*wave_speed*dt,
      impl_->finite_flag);
  if(!impl_->check(cudaGetLastError(),"integer magnetic launch")) return result;

  const auto integrated_pair=[&](DeviceVectorField electric,
                                 DeviceVectorField magnetic_integer,
                                 int face_axis,int edge_axis,double& output) {
    const auto sx=make_spline_stencil(face_axis,edge_axis,0);
    const auto sy=make_spline_stencil(face_axis,edge_axis,1);
    const auto sz=make_spline_stencil(face_axis,edge_axis,2);
    convolve_axis_kernel<<<blocks,threads>>>(device_component(
        magnetic_integer,edge_axis),impl_->scratch0,impl_->count,impl_->L,0,sx);
    convolve_axis_kernel<<<blocks,threads>>>(impl_->scratch0,impl_->scratch1,
        impl_->count,impl_->L,1,sy);
    convolve_axis_kernel<<<blocks,threads>>>(impl_->scratch1,impl_->scratch0,
        impl_->count,impl_->L,2,sz);
    dot_reduce_kernel<<<blocks,threads>>>(device_component(electric,face_axis),
        impl_->scratch0,impl_->count,impl_->partial,impl_->finite_flag);
    if(!impl_->check(cudaGetLastError(),"spline pair launch")
        ||!impl_->check(cudaMemcpy(host_partial.data(),impl_->partial,
            static_cast<std::size_t>(blocks)*sizeof(double),
            cudaMemcpyDeviceToHost),"download spline partials")) return false;
    long double sum=0.0L;
    for(int block=0;block<blocks;++block)
      sum+=host_partial[static_cast<std::size_t>(block)];
    output=static_cast<double>(sum);
    return true;
  };
  const auto spline_momentum=[&](DeviceVectorField electric,
                                 DeviceVectorField magnetic_integer,
                                 Vec3& output) {
    double yz=0.0,zy=0.0,zx=0.0,xz=0.0,xy=0.0,yx=0.0;
    if(!integrated_pair(electric,magnetic_integer,1,2,yz)
        ||!integrated_pair(electric,magnetic_integer,2,1,zy)
        ||!integrated_pair(electric,magnetic_integer,2,0,zx)
        ||!integrated_pair(electric,magnetic_integer,0,2,xz)
        ||!integrated_pair(electric,magnetic_integer,0,1,xy)
        ||!integrated_pair(electric,magnetic_integer,1,0,yx)) return false;
    const double scale=interaction_scale/wave_speed;
    output={(yz-zy)*scale,(zx-xz)*scale,(xy-yx)*scale};
    return true;
  };
  if(!spline_momentum(impl_->e0,impl_->bint0,
          result.spline_field_momentum_before)
      ||!spline_momentum(impl_->e1,impl_->bint1,
          result.spline_field_momentum_after)) return result;
  int finite=0;
  if(!impl_->check(cudaMemcpy(&finite,impl_->finite_flag,sizeof(int),
      cudaMemcpyDeviceToHost),"read diagnostic finite flag")) return result;
  result.field_energy_before=interaction_scale*impl_->observed_energy[0];
  result.field_energy_after=interaction_scale*impl_->observed_energy[2];
  result.valid=finite!=0
      &&std::isfinite(result.gauss_before_residual)
      &&std::isfinite(result.gauss_after_residual)
      &&std::isfinite(result.field_energy_before)
      &&std::isfinite(result.field_energy_after)
      &&std::isfinite(result.local_field_momentum_before.x)
      &&std::isfinite(result.local_field_momentum_before.y)
      &&std::isfinite(result.local_field_momentum_before.z)
      &&std::isfinite(result.local_field_momentum_after.x)
      &&std::isfinite(result.local_field_momentum_after.y)
      &&std::isfinite(result.local_field_momentum_after.z)
      &&std::isfinite(result.spline_field_momentum_before.x)
      &&std::isfinite(result.spline_field_momentum_before.y)
      &&std::isfinite(result.spline_field_momentum_before.z)
      &&std::isfinite(result.spline_field_momentum_after.x)
      &&std::isfinite(result.spline_field_momentum_after.y)
      &&std::isfinite(result.spline_field_momentum_after.z);
  return result;
}

bool CudaMatchedFieldPipeline::download_after(
    MatchedFaceFlux& electric_after,MatchedEdgeField& magnetic_after) {
  if(!valid()||!impl_->prepared||!impl_->current_applied) return false;
  const auto start=std::chrono::steady_clock::now();
  const bool result=impl_->copy_to_host(electric_after,impl_->e1)
      &&impl_->copy_to_host(magnetic_after,impl_->b1);
  impl_->timing.download_ms+=milliseconds_since(start);
  return result;
}

bool CudaMatchedFieldPipeline::advance() {
  if(!valid()||!impl_->prepared||!impl_->current_applied) return false;
  const std::size_t bytes=impl_->count*sizeof(double);
  const bool result=impl_->check(cudaMemcpy(impl_->e0.x,impl_->e1.x,bytes,
      cudaMemcpyDeviceToDevice),"advance e.x")
      &&impl_->check(cudaMemcpy(impl_->e0.y,impl_->e1.y,bytes,
      cudaMemcpyDeviceToDevice),"advance e.y")
      &&impl_->check(cudaMemcpy(impl_->e0.z,impl_->e1.z,bytes,
      cudaMemcpyDeviceToDevice),"advance e.z")
      &&impl_->check(cudaMemcpy(impl_->b0.x,impl_->b1.x,bytes,
      cudaMemcpyDeviceToDevice),"advance b.x")
      &&impl_->check(cudaMemcpy(impl_->b0.y,impl_->b1.y,bytes,
      cudaMemcpyDeviceToDevice),"advance b.y")
      &&impl_->check(cudaMemcpy(impl_->b0.z,impl_->b1.z,bytes,
      cudaMemcpyDeviceToDevice),"advance b.z");
  impl_->prepared=false; impl_->current_applied=false; impl_->observed=false;
  return result;
}

bool cuda_matched_field_pipeline_available() {
  int devices=0;
  return cudaGetDeviceCount(&devices)==cudaSuccess&&devices>0;
}

}  // namespace ftd::eft
