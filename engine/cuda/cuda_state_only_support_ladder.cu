#include "ftd/eft/cuda_state_only_support_ladder.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <numeric>
#include <utility>

namespace ftd::eft {
namespace {

constexpr int kThreads = 256;
constexpr int kMaximumBlocks = 4096;
constexpr int kScaleQuantities = 4;
constexpr int kTransitionQuantities = 2;
constexpr int kMatterSumQuantities = 11;
constexpr int kMatterMaximumQuantities = 5;
constexpr int kShellQuantities = 8;

double milliseconds_since(const std::chrono::steady_clock::time_point& start) {
  return std::chrono::duration<double,std::milli>(
      std::chrono::steady_clock::now()-start).count();
}

double relative_scale(double a,double b,double c=0.0) {
  return std::max({1.0,std::abs(a),std::abs(b),std::abs(c)});
}

struct DeviceTriplet {
  double* x=nullptr;
  double* y=nullptr;
  double* z=nullptr;
};

void release(DeviceTriplet& field) {
  cudaFree(field.x); cudaFree(field.y); cudaFree(field.z);
  field={};
}

bool allocate(DeviceTriplet& field,std::size_t bytes) {
  return cudaMalloc(&field.x,bytes)==cudaSuccess
      &&cudaMalloc(&field.y,bytes)==cudaSuccess
      &&cudaMalloc(&field.z,bytes)==cudaSuccess;
}

template <typename Field>
bool upload(DeviceTriplet& target,const Field& source,std::size_t bytes) {
  return cudaMemcpy(target.x,source.x.data(),bytes,cudaMemcpyHostToDevice)
          ==cudaSuccess
      &&cudaMemcpy(target.y,source.y.data(),bytes,cudaMemcpyHostToDevice)
          ==cudaSuccess
      &&cudaMemcpy(target.z,source.z.data(),bytes,cudaMemcpyHostToDevice)
          ==cudaSuccess;
}

__global__ void sparse_triplet_scatter_kernel(
    DeviceTriplet target,const unsigned long long* indices,
    const int* axes,const double* values,std::size_t entries) {
  const std::size_t first=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  const std::size_t stride=static_cast<std::size_t>(gridDim.x)*blockDim.x;
  for(std::size_t item=first;item<entries;item+=stride) {
    const auto index=static_cast<std::size_t>(indices[item]);
    if(axes[item]==0) target.x[index]=values[item];
    else if(axes[item]==1) target.y[index]=values[item];
    else target.z[index]=values[item];
  }
}

template <typename Field>
bool upload_sparse(DeviceTriplet& target,const Field& source,
                   std::size_t bytes,
                   CudaStateOnlySupportLadderTelemetry& telemetry) {
  if(cudaMemset(target.x,0,bytes)!=cudaSuccess
      ||cudaMemset(target.y,0,bytes)!=cudaSuccess
      ||cudaMemset(target.z,0,bytes)!=cudaSuccess) return false;
  std::vector<unsigned long long> indices;
  std::vector<int> axes;
  std::vector<double> values;
  for(int axis=0;axis<3;++axis) {
    const auto& component=axis==0?source.x:(axis==1?source.y:source.z);
    for(std::size_t index=0;index<component.size();++index) {
      if(component[index]==0.0) continue;
      indices.push_back(static_cast<unsigned long long>(index));
      axes.push_back(axis);
      values.push_back(component[index]);
    }
  }
  if(values.empty()) return true;
  unsigned long long* device_indices=nullptr;
  int* device_axes=nullptr;
  double* device_values=nullptr;
  const std::size_t count=values.size();
  const std::size_t index_bytes=count*sizeof(unsigned long long);
  const std::size_t axis_bytes=count*sizeof(int);
  const std::size_t value_bytes=count*sizeof(double);
  const auto cleanup=[&]() {
    cudaFree(device_indices); cudaFree(device_axes); cudaFree(device_values);
  };
  if(cudaMalloc(&device_indices,index_bytes)!=cudaSuccess
      ||cudaMalloc(&device_axes,axis_bytes)!=cudaSuccess
      ||cudaMalloc(&device_values,value_bytes)!=cudaSuccess) {
    cleanup(); return false;
  }
  if(cudaMemcpy(device_indices,indices.data(),index_bytes,
          cudaMemcpyHostToDevice)!=cudaSuccess
      ||cudaMemcpy(device_axes,axes.data(),axis_bytes,
          cudaMemcpyHostToDevice)!=cudaSuccess
      ||cudaMemcpy(device_values,values.data(),value_bytes,
          cudaMemcpyHostToDevice)!=cudaSuccess) {
    cleanup(); return false;
  }
  const int blocks=std::min<int>(kMaximumBlocks,
      static_cast<int>((count+kThreads-1)/kThreads));
  sparse_triplet_scatter_kernel<<<blocks,kThreads>>>(
      target,device_indices,device_axes,device_values,count);
  const bool valid=cudaGetLastError()==cudaSuccess
      &&cudaDeviceSynchronize()==cudaSuccess;
  cleanup();
  telemetry.host_to_device_bytes+=index_bytes+axis_bytes+value_bytes;
  return valid;
}

int wrap_host(int value,int L) {
  value%=L;
  return value<0?value+L:value;
}

int shortest_delta_host(int coordinate,int center,int L) {
  int delta=coordinate-center;
  if(delta>L/2) delta-=L;
  if(delta<-L/2) delta+=L;
  return delta;
}

struct MappedCompactPreparation {
  FiniteSupportPairPreparation local{};
  Vec3 physical_center{};
  Vec3 target_center{};
  Vec3 fractional_center_offset{};
  int target_L=0;
  bool valid=false;
};

MappedCompactPreparation prepare_mapped_compact_pair(
    const ConnectedMooreBlockState& geometry,
    const ConnectedMooreBlockOptions& options,int support_half_width,
    double poisson_tolerance,int poisson_max_iterations,
    bool allow_fractional_center=false) {
  MappedCompactPreparation result;
  result.target_L=geometry.electric.L;
  if(result.target_L<5||geometry.constituents.size()!=2
      ||geometry.charges.size()!=2) return result;
  const auto position=[](const MatchedMatterPoint& point) {
    return Vec3{point.anchor.x+point.remainder.x,
                point.anchor.y+point.remainder.y,
                point.anchor.z+point.remainder.z};
  };
  const Vec3 centroid=(position(geometry.constituents[0])
      +position(geometry.constituents[1]))*0.5;
  result.physical_center=centroid;
  result.target_center={static_cast<double>(std::llround(centroid.x)),
                        static_cast<double>(std::llround(centroid.y)),
                        static_cast<double>(std::llround(centroid.z))};
  result.fractional_center_offset=centroid-result.target_center;
  if(!allow_fractional_center
      &&result.fractional_center_offset.mag()>1e-12) return result;
  const int local_L=2*support_half_width+7;
  const int local_center=local_L/2;
  ConnectedMooreBlockState local_geometry(local_L);
  local_geometry.charges=geometry.charges;
  local_geometry.edges=geometry.edges;
  local_geometry.width=geometry.width;
  local_geometry.orientation_axis=geometry.orientation_axis;
  local_geometry.constituents=geometry.constituents;
  const int target_center[3]={
      static_cast<int>(std::llround(result.target_center.x)),
      static_cast<int>(std::llround(result.target_center.y)),
      static_cast<int>(std::llround(result.target_center.z))};
  for(auto& point:local_geometry.constituents) {
    point.anchor={
        local_center+shortest_delta_host(
            point.anchor.x,target_center[0],result.target_L),
        local_center+shortest_delta_host(
            point.anchor.y,target_center[1],result.target_L),
        local_center+shortest_delta_host(
            point.anchor.z,target_center[2],result.target_L)};
  }
  result.local=prepare_finite_support_derived_compact_pair(
      local_geometry,options,support_half_width,poisson_tolerance,
      poisson_max_iterations,allow_fractional_center);
  result.valid=result.local.valid&&result.local.compact_support
      &&result.local.zero_boundary_crossing;
  return result;
}

template <typename Field>
bool upload_sparse_mapped(DeviceTriplet& target,const Field& source,
                          std::size_t target_bytes,int target_L,
                          const Vec3& target_center,
                          CudaStateOnlySupportLadderTelemetry& telemetry) {
  if(cudaMemset(target.x,0,target_bytes)!=cudaSuccess
      ||cudaMemset(target.y,0,target_bytes)!=cudaSuccess
      ||cudaMemset(target.z,0,target_bytes)!=cudaSuccess) return false;
  std::vector<unsigned long long> indices;
  std::vector<int> axes;
  std::vector<double> values;
  const int source_L=source.L;
  const int source_center=source_L/2;
  const int cx=static_cast<int>(std::llround(target_center.x));
  const int cy=static_cast<int>(std::llround(target_center.y));
  const int cz=static_cast<int>(std::llround(target_center.z));
  const std::size_t plane=static_cast<std::size_t>(source_L)*source_L;
  for(int axis=0;axis<3;++axis) {
    const auto& component=axis==0?source.x:(axis==1?source.y:source.z);
    for(std::size_t source_index=0;
        source_index<component.size();++source_index) {
      if(component[source_index]==0.0) continue;
      const int x=static_cast<int>(source_index/plane);
      const std::size_t remainder=source_index-static_cast<std::size_t>(x)*plane;
      const int y=static_cast<int>(remainder/source_L);
      const int z=static_cast<int>(remainder-static_cast<std::size_t>(y)*source_L);
      const int target_x=wrap_host(cx+x-source_center,target_L);
      const int target_y=wrap_host(cy+y-source_center,target_L);
      const int target_z=wrap_host(cz+z-source_center,target_L);
      const auto target_index=(static_cast<unsigned long long>(target_x)*target_L
          +target_y)*target_L+target_z;
      indices.push_back(target_index);
      axes.push_back(axis);
      values.push_back(component[source_index]);
    }
  }
  if(values.empty()) return true;
  unsigned long long* device_indices=nullptr;
  int* device_axes=nullptr;
  double* device_values=nullptr;
  const std::size_t count=values.size();
  const std::size_t index_bytes=count*sizeof(unsigned long long);
  const std::size_t axis_bytes=count*sizeof(int);
  const std::size_t value_bytes=count*sizeof(double);
  const auto cleanup=[&]() {
    cudaFree(device_indices); cudaFree(device_axes); cudaFree(device_values);
  };
  if(cudaMalloc(&device_indices,index_bytes)!=cudaSuccess
      ||cudaMalloc(&device_axes,axis_bytes)!=cudaSuccess
      ||cudaMalloc(&device_values,value_bytes)!=cudaSuccess) {
    cleanup(); return false;
  }
  if(cudaMemcpy(device_indices,indices.data(),index_bytes,
          cudaMemcpyHostToDevice)!=cudaSuccess
      ||cudaMemcpy(device_axes,axes.data(),axis_bytes,
          cudaMemcpyHostToDevice)!=cudaSuccess
      ||cudaMemcpy(device_values,values.data(),value_bytes,
          cudaMemcpyHostToDevice)!=cudaSuccess) {
    cleanup(); return false;
  }
  const int blocks=std::min<int>(kMaximumBlocks,
      static_cast<int>((count+kThreads-1)/kThreads));
  sparse_triplet_scatter_kernel<<<blocks,kThreads>>>(
      target,device_indices,device_axes,device_values,count);
  const bool valid=cudaGetLastError()==cudaSuccess
      &&cudaDeviceSynchronize()==cudaSuccess;
  cleanup();
  telemetry.host_to_device_bytes+=index_bytes+axis_bytes+value_bytes;
  return valid;
}

double mapped_face_component(
    const MappedCompactPreparation& preparation,int axis,
    int x,int y,int z) {
  const auto& field=preparation.local.state.electric;
  const int target_center[3]={
      static_cast<int>(std::llround(preparation.target_center.x)),
      static_cast<int>(std::llround(preparation.target_center.y)),
      static_cast<int>(std::llround(preparation.target_center.z))};
  const int local_center=field.L/2;
  const int lx=local_center+shortest_delta_host(
      wrap_host(x,preparation.target_L),target_center[0],preparation.target_L);
  const int ly=local_center+shortest_delta_host(
      wrap_host(y,preparation.target_L),target_center[1],preparation.target_L);
  const int lz=local_center+shortest_delta_host(
      wrap_host(z,preparation.target_L),target_center[2],preparation.target_L);
  if(lx<0||lx>=field.L||ly<0||ly>=field.L||lz<0||lz>=field.L)
    return 0.0;
  const auto item=static_cast<std::size_t>(field.index(lx,ly,lz));
  return axis==0?field.x[item]:(axis==1?field.y[item]:field.z[item]);
}

__device__ void reduce_shared(double* values,int quantities) {
  __syncthreads();
  for(int stride=blockDim.x/2;stride>0;stride>>=1) {
    if(threadIdx.x<stride)
      for(int q=0;q<quantities;++q)
        values[q*blockDim.x+threadIdx.x]
            +=values[q*blockDim.x+threadIdx.x+stride];
    __syncthreads();
  }
}

__device__ void reduce_shared_max(double* values,int quantities) {
  __syncthreads();
  for(int stride=blockDim.x/2;stride>0;stride>>=1) {
    if(threadIdx.x<stride)
      for(int q=0;q<quantities;++q)
        values[q*blockDim.x+threadIdx.x]=fmax(
            values[q*blockDim.x+threadIdx.x],
            values[q*blockDim.x+threadIdx.x+stride]);
    __syncthreads();
  }
}

struct DeviceVec3 {
  double x=0.0,y=0.0,z=0.0;
};

__device__ DeviceVec3 add(DeviceVec3 a,DeviceVec3 b) {
  return {a.x+b.x,a.y+b.y,a.z+b.z};
}
__device__ DeviceVec3 subtract(DeviceVec3 a,DeviceVec3 b) {
  return {a.x-b.x,a.y-b.y,a.z-b.z};
}
__device__ DeviceVec3 scale(DeviceVec3 a,double value) {
  return {a.x*value,a.y*value,a.z*value};
}
__device__ double dot(DeviceVec3 a,DeviceVec3 b) {
  return a.x*b.x+a.y*b.y+a.z*b.z;
}
__device__ double mag2(DeviceVec3 value) { return dot(value,value); }
__device__ DeviceVec3 cross(DeviceVec3 a,DeviceVec3 b) {
  return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
__device__ double maximum_component(DeviceVec3 value) {
  return fmax(fabs(value.x),fmax(fabs(value.y),fabs(value.z)));
}
__device__ double sample_energy(DeviceVec3 electric,DeviceVec3 magnetic) {
  return 0.5*(mag2(electric)+mag2(magnetic));
}
__device__ int wrap_coordinate(int value,int L) {
  value%=L;
  return value<0?value+L:value;
}
__device__ std::size_t lattice_index(int x,int y,int z,int L) {
  return (static_cast<std::size_t>(wrap_coordinate(x,L))*L
      +wrap_coordinate(y,L))*L+wrap_coordinate(z,L);
}
__device__ double component(
    const DeviceTriplet field,int axis,int x,int y,int z,int L) {
  const auto index=lattice_index(x,y,z,L);
  return axis==0?field.x[index]:(axis==1?field.y[index]:field.z[index]);
}
__device__ double curl_adjoint_component(
    const DeviceTriplet field,int axis,int x,int y,int z,int L) {
  const auto f=[&](int c,int xx,int yy,int zz) {
    return component(field,c,xx,yy,zz,L);
  };
  if(axis==0)
    return f(2,x,y+1,z)-f(2,x,y,z)-f(1,x,y,z+1)+f(1,x,y,z);
  if(axis==1)
    return f(0,x,y,z+1)-f(0,x,y,z)-f(2,x+1,y,z)+f(2,x,y,z);
  return f(1,x+1,y,z)-f(1,x,y,z)-f(0,x,y+1,z)+f(0,x,y,z);
}
__device__ double integer_edge_component(
    const DeviceTriplet electric,const DeviceTriplet magnetic,
    int axis,int x,int y,int z,int L,double half_step_scale) {
  return component(magnetic,axis,x,y,z,L)
      +half_step_scale*curl_adjoint_component(electric,axis,x,y,z,L);
}
__device__ DeviceVec3 centered_face(
    const DeviceTriplet field,int x,int y,int z,int L) {
  return {
      0.5*(component(field,0,x,y,z,L)+component(field,0,x-1,y,z,L)),
      0.5*(component(field,1,x,y,z,L)+component(field,1,x,y-1,z,L)),
      0.5*(component(field,2,x,y,z,L)+component(field,2,x,y,z-1,L))};
}
__device__ DeviceVec3 centered_integer_edge(
    const DeviceTriplet electric,const DeviceTriplet magnetic,
    int x,int y,int z,int L,double half_step_scale) {
  const auto b=[&](int axis,int xx,int yy,int zz) {
    return integer_edge_component(
        electric,magnetic,axis,xx,yy,zz,L,half_step_scale);
  };
  return {
      0.25*(b(0,x,y,z)+b(0,x,y-1,z)+b(0,x,y,z-1)+b(0,x,y-1,z-1)),
      0.25*(b(1,x,y,z)+b(1,x-1,y,z)+b(1,x,y,z-1)+b(1,x-1,y,z-1)),
      0.25*(b(2,x,y,z)+b(2,x-1,y,z)+b(2,x,y-1,z)+b(2,x-1,y-1,z))};
}
__device__ double divergence(
    const DeviceTriplet field,int x,int y,int z,int L) {
  return component(field,0,x,y,z,L)-component(field,0,x-1,y,z,L)
      +component(field,1,x,y,z,L)-component(field,1,x,y-1,z,L)
      +component(field,2,x,y,z,L)-component(field,2,x,y,z-1,L);
}
__device__ int shortest_delta_device(int coordinate,int center,int L) {
  int delta=coordinate-center;
  if(delta>L/2) delta-=L;
  if(delta<-L/2) delta+=L;
  return delta;
}

__device__ double shortest_delta_device(
    int coordinate,double center,int L) {
  double delta=static_cast<double>(coordinate)-center;
  const double half=0.5*static_cast<double>(L);
  if(delta>half) delta-=static_cast<double>(L);
  if(delta<-half) delta+=static_cast<double>(L);
  return delta;
}

struct DeviceCharacteristicSample {
  bool valid=false;
  double residual=0.0,outgoing=0.0,incoming=0.0,radial=0.0;
  double background=0.0,flux=0.0,reconstruction=0.0;
  double energy_partition=0.0,characteristic_flux=0.0;
};

__device__ DeviceCharacteristicSample decompose_sample(
    DeviceVec3 electric,DeviceVec3 magnetic,DeviceVec3 radial,
    double tolerance) {
  DeviceCharacteristicSample result;
  const double radial_norm=sqrt(mag2(radial));
  if(radial_norm<=tolerance) {
    result.residual=sample_energy(electric,magnetic);
    result.radial=result.residual;
    result.background=result.residual;
    result.valid=true;
    return result;
  }
  const DeviceVec3 n=scale(radial,1.0/radial_norm);
  const DeviceVec3 electric_radial=scale(n,dot(electric,n));
  const DeviceVec3 magnetic_radial=scale(n,dot(magnetic,n));
  const DeviceVec3 electric_tangent=subtract(electric,electric_radial);
  const DeviceVec3 magnetic_tangent=subtract(magnetic,magnetic_radial);
  const DeviceVec3 n_cross_b=cross(n,magnetic_tangent);
  const DeviceVec3 outgoing_electric=
      scale(subtract(electric_tangent,n_cross_b),0.5);
  const DeviceVec3 incoming_electric=
      scale(add(electric_tangent,n_cross_b),0.5);
  const DeviceVec3 outgoing_magnetic=cross(n,outgoing_electric);
  const DeviceVec3 incoming_magnetic=scale(cross(n,incoming_electric),-1.0);
  const DeviceVec3 background_electric=add(incoming_electric,electric_radial);
  const DeviceVec3 background_magnetic=add(incoming_magnetic,magnetic_radial);
  result.residual=sample_energy(electric,magnetic);
  result.outgoing=sample_energy(outgoing_electric,outgoing_magnetic);
  result.incoming=sample_energy(incoming_electric,incoming_magnetic);
  result.radial=sample_energy(electric_radial,magnetic_radial);
  result.background=result.incoming+result.radial;
  result.flux=dot(cross(electric,magnetic),n);
  result.reconstruction=fmax(
      maximum_component(subtract(subtract(electric,outgoing_electric),
                                 background_electric)),
      maximum_component(subtract(subtract(magnetic,outgoing_magnetic),
                                 background_magnetic)));
  result.energy_partition=result.residual-result.outgoing-result.background;
  result.characteristic_flux=result.flux-(result.outgoing-result.incoming);
  const double norm=fmax(1.0,
      fmax(fabs(result.residual),
           fmax(fabs(result.outgoing),fabs(result.background))));
  result.valid=result.reconstruction<=tolerance*norm
      &&fabs(result.energy_partition)<=tolerance*norm
      &&fabs(result.characteristic_flux)<=tolerance*norm;
  return result;
}

__global__ void matter_observer_global_kernel(
    DeviceTriplet actual_e,DeviceTriplet actual_b,
    DeviceTriplet bound_e,DeviceTriplet bound_b,
    std::size_t count,int L,int cx,int cy,int cz,
    double physical_cx,double physical_cy,double physical_cz,
    double half_step_scale,double tolerance,
    double* sum_partial,double* maximum_partial) {
  extern __shared__ double shared[];
  double sums[kMatterSumQuantities]{};
  double maxima[kMatterMaximumQuantities]{};
  const std::size_t first=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  const std::size_t stride=static_cast<std::size_t>(gridDim.x)*blockDim.x;
  const std::size_t plane=static_cast<std::size_t>(L)*L;
  for(std::size_t index=first;index<count;index+=stride) {
    const int x=static_cast<int>(index/plane);
    const std::size_t remainder=index-static_cast<std::size_t>(x)*plane;
    const int y=static_cast<int>(remainder/L);
    const int z=static_cast<int>(remainder-static_cast<std::size_t>(y)*L);
    const int dx=shortest_delta_device(x,cx,L);
    const int dy=shortest_delta_device(y,cy,L);
    const int dz=shortest_delta_device(z,cz,L);
    const double radial_x=shortest_delta_device(x,physical_cx,L);
    const double radial_y=shortest_delta_device(y,physical_cy,L);
    const double radial_z=shortest_delta_device(z,physical_cz,L);
    const DeviceVec3 actual_face=centered_face(actual_e,x,y,z,L);
    const DeviceVec3 bound_face=centered_face(bound_e,x,y,z,L);
    const DeviceVec3 residual_face=subtract(actual_face,bound_face);
    const DeviceVec3 actual_edge=centered_integer_edge(
        actual_e,actual_b,x,y,z,L,half_step_scale);
    const DeviceVec3 bound_edge=centered_integer_edge(
        bound_e,bound_b,x,y,z,L,half_step_scale);
    const DeviceVec3 residual_edge=subtract(actual_edge,bound_edge);
    const auto sample=decompose_sample(
        residual_face,residual_edge,
        {radial_x,radial_y,radial_z},tolerance);
    sums[0]+=sample_energy(actual_face,actual_edge);
    sums[1]+=sample_energy(bound_face,bound_edge);
    sums[2]+=sample.residual;
    sums[3]+=sample.outgoing;
    sums[4]+=sample.incoming;
    sums[5]+=sample.radial;
    sums[6]+=sample.background;
    sums[7]+=sample.flux;
    sums[9]+=dot(bound_face,residual_face);
    sums[10]+=dot(bound_edge,residual_edge);
    for(int axis=0;axis<3;++axis) {
      const double b=component(bound_e,axis,x,y,z,L);
      sums[8]+=b*(component(actual_e,axis,x,y,z,L)-b);
    }
    maxima[0]=fmax(maxima[0],sample.reconstruction);
    maxima[1]=fmax(maxima[1],fabs(divergence(actual_e,x,y,z,L)
        -divergence(bound_e,x,y,z,L)));
    maxima[2]=fmax(maxima[2],fabs(sample.characteristic_flux));
    maxima[3]=fmax(maxima[3],fabs(sample.energy_partition));
    maxima[4]=fmax(maxima[4],sample.valid?0.0:1.0);
  }
  for(int q=0;q<kMatterSumQuantities;++q)
    shared[q*blockDim.x+threadIdx.x]=sums[q];
  double* maximum_shared=shared+kMatterSumQuantities*blockDim.x;
  for(int q=0;q<kMatterMaximumQuantities;++q)
    maximum_shared[q*blockDim.x+threadIdx.x]=maxima[q];
  reduce_shared(shared,kMatterSumQuantities);
  reduce_shared_max(maximum_shared,kMatterMaximumQuantities);
  if(threadIdx.x==0) {
    for(int q=0;q<kMatterSumQuantities;++q)
      sum_partial[static_cast<std::size_t>(q)*gridDim.x+blockIdx.x]
          =shared[q*blockDim.x];
    for(int q=0;q<kMatterMaximumQuantities;++q)
      maximum_partial[static_cast<std::size_t>(q)*gridDim.x+blockIdx.x]
          =maximum_shared[q*blockDim.x];
  }
}

__global__ void matter_observer_shell_kernel(
    DeviceTriplet actual_e,DeviceTriplet actual_b,
    DeviceTriplet bound_e,DeviceTriplet bound_b,
    std::size_t count,int L,int cx,int cy,int cz,
    double physical_cx,double physical_cy,double physical_cz,int radius,
    double half_step_scale,double tolerance,double* partial) {
  extern __shared__ double shared[];
  double values[kShellQuantities]{};
  const std::size_t first=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  const std::size_t stride=static_cast<std::size_t>(gridDim.x)*blockDim.x;
  const std::size_t plane=static_cast<std::size_t>(L)*L;
  for(std::size_t index=first;index<count;index+=stride) {
    const int x=static_cast<int>(index/plane);
    const std::size_t remainder=index-static_cast<std::size_t>(x)*plane;
    const int y=static_cast<int>(remainder/L);
    const int z=static_cast<int>(remainder-static_cast<std::size_t>(y)*L);
    const int dx=shortest_delta_device(x,cx,L);
    const int dy=shortest_delta_device(y,cy,L);
    const int dz=shortest_delta_device(z,cz,L);
    if(max(abs(dx),max(abs(dy),abs(dz)))!=radius) continue;
    const double radial_x=shortest_delta_device(x,physical_cx,L);
    const double radial_y=shortest_delta_device(y,physical_cy,L);
    const double radial_z=shortest_delta_device(z,physical_cz,L);
    const DeviceVec3 residual_face=subtract(
        centered_face(actual_e,x,y,z,L),centered_face(bound_e,x,y,z,L));
    const DeviceVec3 residual_edge=subtract(
        centered_integer_edge(actual_e,actual_b,x,y,z,L,half_step_scale),
        centered_integer_edge(bound_e,bound_b,x,y,z,L,half_step_scale));
    const auto sample=decompose_sample(
        residual_face,residual_edge,
        {radial_x,radial_y,radial_z},tolerance);
    values[0]+=1.0;
    values[1]+=sample.residual;
    values[2]+=sample.outgoing;
    values[3]+=sample.incoming;
    values[4]+=sample.radial;
    values[5]+=sample.background;
    values[6]+=sample.flux;
    values[7]+=sample.valid?0.0:1.0;
  }
  for(int q=0;q<kShellQuantities;++q)
    shared[q*blockDim.x+threadIdx.x]=values[q];
  reduce_shared(shared,kShellQuantities);
  if(threadIdx.x==0)
    for(int q=0;q<kShellQuantities;++q)
      partial[static_cast<std::size_t>(q)*gridDim.x+blockIdx.x]
          =shared[q*blockDim.x];
}

__global__ void support_scale_kernel(
    const double* ax,const double* ay,const double* az,
    const double* bx,const double* by,const double* bz,
    std::size_t count,double* partial) {
  extern __shared__ double shared[];
  double actual_squared=0.0,bound_squared=0.0;
  double residual_squared=0.0,interference=0.0;
  const std::size_t first=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  const std::size_t stride=static_cast<std::size_t>(gridDim.x)*blockDim.x;
  for(std::size_t i=first;i<count;i+=stride) {
    const double av[3]={ax[i],ay[i],az[i]};
    const double bv[3]={bx[i],by[i],bz[i]};
    for(int component=0;component<3;++component) {
      const double r=av[component]-bv[component];
      actual_squared+=av[component]*av[component];
      bound_squared+=bv[component]*bv[component];
      residual_squared+=r*r;
      interference+=bv[component]*r;
    }
  }
  shared[threadIdx.x]=actual_squared;
  shared[blockDim.x+threadIdx.x]=bound_squared;
  shared[2*blockDim.x+threadIdx.x]=residual_squared;
  shared[3*blockDim.x+threadIdx.x]=interference;
  reduce_shared(shared,kScaleQuantities);
  if(threadIdx.x==0)
    for(int q=0;q<kScaleQuantities;++q)
      partial[static_cast<std::size_t>(q)*gridDim.x+blockIdx.x]
          =shared[q*blockDim.x];
}

__global__ void support_transition_kernel(
    const double* ix,const double* iy,const double* iz,
    const double* ox,const double* oy,const double* oz,
    std::size_t count,double* partial) {
  extern __shared__ double shared[];
  double difference_squared=0.0,projection=0.0;
  const std::size_t first=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  const std::size_t stride=static_cast<std::size_t>(gridDim.x)*blockDim.x;
  for(std::size_t i=first;i<count;i+=stride) {
    const double inner[3]={ix[i],iy[i],iz[i]};
    const double outer[3]={ox[i],oy[i],oz[i]};
    for(int component=0;component<3;++component) {
      const double difference=inner[component]-outer[component];
      difference_squared+=difference*difference;
      projection+=outer[component]*difference;
    }
  }
  shared[threadIdx.x]=difference_squared;
  shared[blockDim.x+threadIdx.x]=projection;
  reduce_shared(shared,kTransitionQuantities);
  if(threadIdx.x==0) {
    partial[blockIdx.x]=shared[0];
    partial[static_cast<std::size_t>(gridDim.x)+blockIdx.x]
        =shared[blockDim.x];
  }
}

std::vector<double> download_reduce(
    double* partial,int quantities,int blocks,
    CudaStateOnlySupportLadderTelemetry& telemetry) {
  const std::size_t values=static_cast<std::size_t>(quantities)*blocks;
  std::vector<double> host(values);
  const auto start=std::chrono::steady_clock::now();
  if(cudaMemcpy(host.data(),partial,values*sizeof(double),
                cudaMemcpyDeviceToHost)!=cudaSuccess) return {};
  telemetry.download_ms+=milliseconds_since(start);
  telemetry.device_to_host_bytes+=values*sizeof(double);
  std::vector<double> result(static_cast<std::size_t>(quantities),0.0);
  for(int q=0;q<quantities;++q) {
    long double sum=0.0L;
    const std::size_t offset=static_cast<std::size_t>(q)*blocks;
    for(int block=0;block<blocks;++block)
      sum+=host[offset+static_cast<std::size_t>(block)];
    result[static_cast<std::size_t>(q)]=static_cast<double>(sum);
  }
  return result;
}

std::vector<double> download_maximum(
    double* partial,int quantities,int blocks,
    CudaStateOnlySupportLadderTelemetry& telemetry) {
  const std::size_t values=static_cast<std::size_t>(quantities)*blocks;
  std::vector<double> host(values);
  const auto start=std::chrono::steady_clock::now();
  if(cudaMemcpy(host.data(),partial,values*sizeof(double),
                cudaMemcpyDeviceToHost)!=cudaSuccess) return {};
  telemetry.download_ms+=milliseconds_since(start);
  telemetry.device_to_host_bytes+=values*sizeof(double);
  std::vector<double> result(static_cast<std::size_t>(quantities),0.0);
  for(int q=0;q<quantities;++q) {
    const std::size_t offset=static_cast<std::size_t>(q)*blocks;
    for(int block=0;block<blocks;++block)
      result[static_cast<std::size_t>(q)]=std::max(
          result[static_cast<std::size_t>(q)],
          host[offset+static_cast<std::size_t>(block)]);
  }
  return result;
}

double host_face_component(const MatchedFaceFlux& field,int axis,
                           int x,int y,int z) {
  const auto index=static_cast<std::size_t>(field.index(x,y,z));
  return axis==0?field.x[index]:(axis==1?field.y[index]:field.z[index]);
}

void complete_boundary_ledger(
    StateOnlyMatterFieldObservation& result,
    const ConnectedMooreBlockState& actual,
    const ConnectedMooreBlockState& bound,
    int support,double tolerance,
    const CudaMatchedFieldDeviceView* resident_electric=nullptr,
    CudaStateOnlySupportLadderTelemetry* telemetry=nullptr,
    const MappedCompactPreparation* mapped_bound=nullptr) {
  const int cx=static_cast<int>(std::llround(result.support_center.x));
  const int cy=static_cast<int>(std::llround(result.support_center.y));
  const int cz=static_cast<int>(std::llround(result.support_center.z));
  const int side=2*support+1;
  const auto local_index=[=](int dx,int dy,int dz) {
    return static_cast<std::size_t>(dx+support)*side*side
        +static_cast<std::size_t>(dy+support)*side
        +static_cast<std::size_t>(dz+support);
  };
  std::vector<double> potential(
      static_cast<std::size_t>(side)*side*side,0.0);
  const auto bound_face=[&](int axis,int x,int y,int z) {
    return mapped_bound!=nullptr
        ?mapped_face_component(*mapped_bound,axis,x,y,z)
        :host_face_component(bound.electric,axis,x,y,z);
  };
  for(int dx=-support;dx<support;++dx)
    potential[local_index(dx+1,-support,-support)]=
        potential[local_index(dx,-support,-support)]
        -bound_face(0,cx+dx,cy-support,cz-support);
  for(int dx=-support;dx<=support;++dx)
    for(int dy=-support;dy<support;++dy)
      potential[local_index(dx,dy+1,-support)]=
          potential[local_index(dx,dy,-support)]
          -bound_face(1,cx+dx,cy+dy,cz-support);
  for(int dx=-support;dx<=support;++dx)
    for(int dy=-support;dy<=support;++dy)
      for(int dz=-support;dz<support;++dz)
        potential[local_index(dx,dy,dz+1)]=
            potential[local_index(dx,dy,dz)]
            -bound_face(2,cx+dx,cy+dy,cz+dz);
  bool device_copy_valid=true;
  const auto actual_face=[&](int axis,int x,int y,int z) {
    if(resident_electric==nullptr)
      return host_face_component(actual.electric,axis,x,y,z);
    const int L=resident_electric->L;
    const auto item=(static_cast<std::size_t>(wrap_host(x,L))*L
        +wrap_host(y,L))*L+wrap_host(z,L);
    const double* source=axis==0?resident_electric->x
        :(axis==1?resident_electric->y:resident_electric->z);
    double value=0.0;
    const auto start=std::chrono::steady_clock::now();
    if(cudaMemcpy(&value,source+item,sizeof(double),cudaMemcpyDeviceToHost)
        !=cudaSuccess) device_copy_valid=false;
    if(telemetry!=nullptr) {
      telemetry->download_ms+=milliseconds_since(start);
      telemetry->device_to_host_bytes+=sizeof(double);
    }
    return value;
  };
  const auto residual_face=[&](int axis,int x,int y,int z) {
    return actual_face(axis,x,y,z)
        -bound_face(axis,x,y,z);
  };
  long double induced=0.0L,flux=0.0L;
  for(int dx=-support;dx<=support;++dx)
    for(int dy=-support;dy<=support;++dy)
      for(int dz=-support;dz<=support;++dz) {
        double crossing=0.0;
        if(dx==support) crossing+=residual_face(0,cx+dx,cy+dy,cz+dz);
        if(dx==-support) crossing-=residual_face(0,cx+dx-1,cy+dy,cz+dz);
        if(dy==support) crossing+=residual_face(1,cx+dx,cy+dy,cz+dz);
        if(dy==-support) crossing-=residual_face(1,cx+dx,cy+dy-1,cz+dz);
        if(dz==support) crossing+=residual_face(2,cx+dx,cy+dy,cz+dz);
        if(dz==-support) crossing-=residual_face(2,cx+dx,cy+dy,cz+dz-1);
        induced-=static_cast<long double>(potential[local_index(dx,dy,dz)])
            *crossing;
        flux+=crossing;
      }
  result.induced_boundary_interference=static_cast<double>(induced);
  result.boundary_flux_sum=static_cast<double>(flux);
  result.primitive_boundary_identity_residual=
      result.primitive_face_interference-result.induced_boundary_interference;
  result.readout_interference_reconstruction_residual=
      result.bound_residual_interference
      -result.centered_electric_interference
      -result.centered_magnetic_interference;
  const double boundary_scale=relative_scale(
      result.primitive_face_interference,
      result.induced_boundary_interference,
      result.bound_residual_interference);
  result.boundary_energy_ledger_valid=
      device_copy_valid
      &&std::abs(result.boundary_flux_sum)<=tolerance*boundary_scale
      &&std::abs(result.primitive_boundary_identity_residual)
          <=tolerance*boundary_scale
      &&std::abs(result.readout_interference_reconstruction_residual)
          <=tolerance*boundary_scale;
}

void fail(CudaStateOnlySupportLadderTelemetry& telemetry,
          const std::string& message) {
  telemetry.error=message;
  telemetry.valid=false;
}

}  // namespace

StateOnlySupportLadderObservation observe_support_ladder_impl(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const CudaMatchedFieldDeviceView* resident_electric,
    const std::vector<int>& support_half_widths,
    double poisson_tolerance,int poisson_max_iterations,
    double gate_tolerance,
    CudaStateOnlySupportLadderTelemetry* telemetry_out,
    bool allow_fractional_center) {
  StateOnlySupportLadderObservation result;
  CudaStateOnlySupportLadderTelemetry telemetry;
  result.L=resident_electric?resident_electric->L:state.electric.L;
  const std::size_t count=result.L>0
      ?static_cast<std::size_t>(result.L)*result.L*result.L:0;
  const bool shapes=resident_electric
      ?resident_electric->valid()&&state.electric.L==result.L
          &&state.magnetic_half.L==result.L
      :result.L>0&&state.magnetic_half.L==result.L
          &&state.electric.x.size()==count&&state.electric.y.size()==count
          &&state.electric.z.size()==count;
  const bool ordered=!support_half_widths.empty()
      &&std::adjacent_find(support_half_widths.begin(),
          support_half_widths.end(),std::greater_equal<int>())
          ==support_half_widths.end();
  if(!shapes||!ordered||state.constituents.size()!=2
      ||state.charges.size()!=2||!state.edges.empty()
      ||action_options.binding_law!=ConnectedBindingLaw::DerivedCompactPair
      ||!(poisson_tolerance>0.0)||poisson_max_iterations<=0
      ||!(gate_tolerance>0.0)) {
    fail(telemetry,"invalid support-ladder input");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }

  ConnectedMooreBlockState geometry;
  geometry.electric.L=result.L;
  geometry.magnetic_half.L=result.L;
  geometry.constituents=state.constituents;
  geometry.charges=state.charges;
  geometry.edges=state.edges;
  geometry.width=state.width;
  geometry.orientation_axis=state.orientation_axis;

  std::vector<FiniteSupportPairPreparation> preparations;
  std::vector<MappedCompactPreparation> mapped_preparations;
  preparations.reserve(support_half_widths.size());
  mapped_preparations.reserve(support_half_widths.size());
  for(const int support:support_half_widths) {
    if(resident_electric!=nullptr) {
      auto preparation=prepare_mapped_compact_pair(
          geometry,action_options,support,poisson_tolerance,
          poisson_max_iterations,allow_fractional_center);
      if(!preparation.valid) {
        fail(telemetry,"mapped compact bound preparation failed");
        if(telemetry_out) *telemetry_out=telemetry;
        return result;
      }
      mapped_preparations.push_back(std::move(preparation));
    } else {
      auto preparation=prepare_finite_support_derived_compact_pair(
          geometry,action_options,support,poisson_tolerance,
          poisson_max_iterations,allow_fractional_center);
      if(!preparation.valid||!preparation.compact_support
          ||!preparation.zero_boundary_crossing) {
        fail(telemetry,"compact bound preparation failed");
        if(telemetry_out) *telemetry_out=telemetry;
        return result;
      }
      preparations.push_back(std::move(preparation));
    }
  }
  const bool mapped=resident_electric!=nullptr;
  const auto preparation_at=[&](std::size_t index)
      ->const FiniteSupportPairPreparation& {
    return mapped?mapped_preparations[index].local:preparations[index];
  };
  const auto center_at=[&](std::size_t index) {
    return mapped?mapped_preparations[index].physical_center
        :preparations[index].center;
  };
  const auto support_center_at=[&](std::size_t index) {
    return mapped?mapped_preparations[index].target_center
        :preparations[index].support_center;
  };
  const auto center_offset_at=[&](std::size_t index) {
    return mapped?mapped_preparations[index].fractional_center_offset
        :preparations[index].fractional_center_offset;
  };
  result.center=center_at(0);
  result.support_center=support_center_at(0);
  result.fractional_center_offset=center_offset_at(0);
  result.fractional_center_enabled=allow_fractional_center;

  const std::size_t bytes=count*sizeof(double);
  DeviceTriplet actual,bound_a,bound_b;
  const bool owns_actual=resident_electric==nullptr;
  if(!owns_actual) actual={const_cast<double*>(resident_electric->x),
      const_cast<double*>(resident_electric->y),
      const_cast<double*>(resident_electric->z)};
  double* partial=nullptr;
  const int blocks=std::min<int>(kMaximumBlocks,
      static_cast<int>((count+kThreads-1)/kThreads));
  const std::size_t partial_values=static_cast<std::size_t>(
      kScaleQuantities)*blocks;
  const auto allocation_start=std::chrono::steady_clock::now();
  bool allocated=(!owns_actual||allocate(actual,bytes))&&allocate(bound_a,bytes)
      &&allocate(bound_b,bytes)
      &&cudaMalloc(&partial,partial_values*sizeof(double))==cudaSuccess;
  telemetry.allocation_ms=milliseconds_since(allocation_start);
  const auto cleanup=[&]() {
    if(owns_actual) release(actual);
    release(bound_a); release(bound_b); cudaFree(partial);
  };
  if(!allocated) {
    cleanup();
    fail(telemetry,"CUDA allocation failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }

  const auto upload_start=std::chrono::steady_clock::now();
  if(owns_actual&&!upload(actual,state.electric,bytes)) {
    cleanup();
    fail(telemetry,"actual field upload failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  telemetry.upload_ms+=milliseconds_since(upload_start);
  if(owns_actual) telemetry.host_to_device_bytes+=3*bytes;

  bool all_valid=true,have_previous=false;
  double previous_energy=0.0;
  const std::size_t preparation_count=support_half_widths.size();
  result.scales.reserve(preparation_count);
  if(preparation_count>1)
    result.transitions.reserve(preparation_count-1);

  for(std::size_t scale_index=0;scale_index<preparation_count;++scale_index) {
    const auto& preparation=preparation_at(scale_index);
    auto& current=(scale_index%2==0)?bound_a:bound_b;
    auto& previous=(scale_index%2==0)?bound_b:bound_a;
    const auto bound_upload_start=std::chrono::steady_clock::now();
    const bool bound_uploaded=mapped
        ?upload_sparse_mapped(current,preparation.state.electric,bytes,
            result.L,mapped_preparations[scale_index].target_center,telemetry)
        :upload_sparse(current,preparation.state.electric,bytes,telemetry);
    if(!bound_uploaded) {
      all_valid=false; fail(telemetry,"bound field upload failed"); break;
    }
    telemetry.upload_ms+=milliseconds_since(bound_upload_start);

    const auto scale_start=std::chrono::steady_clock::now();
    support_scale_kernel<<<blocks,kThreads,
        kScaleQuantities*kThreads*sizeof(double)>>>(
        actual.x,actual.y,actual.z,current.x,current.y,current.z,
        count,partial);
    if(cudaGetLastError()!=cudaSuccess||cudaDeviceSynchronize()!=cudaSuccess) {
      all_valid=false; fail(telemetry,"support scale kernel failed"); break;
    }
    telemetry.kernel_ms+=milliseconds_since(scale_start);
    const auto values=download_reduce(
        partial,kScaleQuantities,blocks,telemetry);
    if(values.size()!=kScaleQuantities) {
      all_valid=false; fail(telemetry,"support scale reduction failed"); break;
    }

    StateOnlySupportScale scale;
    scale.support_half_width=support_half_widths[scale_index];
    scale.actual_face_energy=0.5*values[0];
    scale.bound_face_energy=preparation.electric_energy;
    scale.residual_face_energy=0.5*values[2];
    scale.primitive_interference=values[3];
    scale.poisson_residual=preparation.poisson_residual;
    scale.gauss_residual=preparation.gauss_residual;
    scale.energy_reconstruction_residual=scale.actual_face_energy
        -scale.bound_face_energy-scale.residual_face_energy
        -scale.primitive_interference;
    const double scale_norm=relative_scale(scale.actual_face_energy,
        scale.bound_face_energy,scale.residual_face_energy);
    const double bound_reduction_residual=
        0.5*values[1]-scale.bound_face_energy;
    scale.valid=std::abs(scale.energy_reconstruction_residual)
            <=gate_tolerance*scale_norm
        &&std::abs(bound_reduction_residual)<=gate_tolerance*scale_norm
        &&scale.gauss_residual<=gate_tolerance;
    result.maximum_energy_reconstruction_residual=std::max(
        result.maximum_energy_reconstruction_residual,
        std::abs(scale.energy_reconstruction_residual));
    all_valid=all_valid&&scale.valid
        &&(scale_index==0
            ||(result.center-center_at(scale_index)).mag()
                <=gate_tolerance);

    if(have_previous) {
      const auto transition_start=std::chrono::steady_clock::now();
      support_transition_kernel<<<blocks,kThreads,
          kTransitionQuantities*kThreads*sizeof(double)>>>(
          previous.x,previous.y,previous.z,current.x,current.y,current.z,
          count,partial);
      if(cudaGetLastError()!=cudaSuccess||cudaDeviceSynchronize()!=cudaSuccess) {
        all_valid=false; fail(telemetry,"support transition kernel failed");
        break;
      }
      telemetry.kernel_ms+=milliseconds_since(transition_start);
      const auto transition_values=download_reduce(
          partial,kTransitionQuantities,blocks,telemetry);
      if(transition_values.size()!=kTransitionQuantities) {
        all_valid=false; fail(telemetry,"support transition reduction failed");
        break;
      }
      StateOnlySupportTransition transition;
      transition.inner_half_width=
          support_half_widths[scale_index-1];
      transition.outer_half_width=support_half_widths[scale_index];
      transition.relaxation_energy=0.5*transition_values[0];
      transition.outer_difference_inner_product=transition_values[1];
      transition.monotonicity_margin=previous_energy-scale.bound_face_energy;
      transition.pythagorean_residual=transition.monotonicity_margin
          -transition.relaxation_energy;
      const double transition_norm=relative_scale(previous_energy,
          scale.bound_face_energy,transition.relaxation_energy);
      transition.valid=transition.monotonicity_margin
              >=-gate_tolerance*transition_norm
          &&std::abs(transition.outer_difference_inner_product)
              <=gate_tolerance*transition_norm
          &&std::abs(transition.pythagorean_residual)
              <=gate_tolerance*transition_norm;
      result.maximum_projection_residual=std::max({
          result.maximum_projection_residual,
          std::abs(transition.outer_difference_inner_product),
          std::abs(transition.pythagorean_residual)});
      all_valid=all_valid&&transition.valid;
      result.transitions.push_back(transition);
    }
    previous_energy=scale.bound_face_energy;
    have_previous=true;
    result.scales.push_back(scale);
  }

  cleanup();
  result.valid=all_valid&&result.scales.size()==support_half_widths.size()
      &&result.transitions.size()+1==result.scales.size();
  telemetry.valid=result.valid&&telemetry.error.empty();
  if(telemetry_out) *telemetry_out=telemetry;
  return result;
}

StateOnlySupportLadderObservation observe_state_only_support_ladder_cuda(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const std::vector<int>& support_half_widths,
    double poisson_tolerance,int poisson_max_iterations,
    double gate_tolerance,
    CudaStateOnlySupportLadderTelemetry* telemetry_out,
    bool allow_fractional_center) {
  return observe_support_ladder_impl(state,action_options,nullptr,
      support_half_widths,poisson_tolerance,poisson_max_iterations,
      gate_tolerance,telemetry_out,allow_fractional_center);
}

StateOnlySupportLadderObservation
observe_state_only_support_ladder_cuda_resident(
    const ConnectedMooreBlockState& matter_only,
    const ConnectedMooreBlockOptions& action_options,
    const CudaMatchedFieldDeviceView& actual_electric,
    const std::vector<int>& support_half_widths,
    double poisson_tolerance,int poisson_max_iterations,
    double gate_tolerance,
    CudaStateOnlySupportLadderTelemetry* telemetry_out,
    bool allow_fractional_center) {
  return observe_support_ladder_impl(matter_only,action_options,
      &actual_electric,support_half_widths,poisson_tolerance,
      poisson_max_iterations,gate_tolerance,telemetry_out,
      allow_fractional_center);
}

StateOnlyMatterFieldObservation observe_matter_field_impl(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const CudaMatchedFieldDeviceView* resident_electric,
    const CudaMatchedFieldDeviceView* resident_magnetic,
    const StateOnlyMatterFieldObserverOptions& observer_options,
    CudaStateOnlySupportLadderTelemetry* telemetry_out) {
  StateOnlyMatterFieldObservation result;
  CudaStateOnlySupportLadderTelemetry telemetry;
  result.L=resident_electric?resident_electric->L:state.electric.L;
  result.support_half_width=observer_options.support_half_width;
  const std::size_t count=result.L>0
      ?static_cast<std::size_t>(result.L)*result.L*result.L:0;
  const bool resident=resident_electric!=nullptr||resident_magnetic!=nullptr;
  const bool shapes=resident
      ?resident_electric!=nullptr&&resident_magnetic!=nullptr
          &&resident_electric->valid()&&resident_magnetic->valid()
          &&resident_magnetic->L==result.L
          &&state.electric.L==result.L&&state.magnetic_half.L==result.L
      :result.L>0&&state.magnetic_half.L==result.L
          &&state.electric.x.size()==count&&state.electric.y.size()==count
          &&state.electric.z.size()==count
          &&state.magnetic_half.x.size()==count
          &&state.magnetic_half.y.size()==count
          &&state.magnetic_half.z.size()==count;
  if(!shapes||result.L%2==0||state.constituents.size()!=2
      ||state.charges.size()!=2||!state.edges.empty()
      ||action_options.binding_law!=ConnectedBindingLaw::DerivedCompactPair
      ||!(observer_options.wave_speed>0.0)
      ||!std::isfinite(observer_options.dt)
      ||!(observer_options.gate_tolerance>0.0)) {
    fail(telemetry,"invalid state-only observer input");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }

  ConnectedMooreBlockState geometry;
  geometry.electric.L=result.L;
  geometry.magnetic_half.L=result.L;
  geometry.constituents=state.constituents;
  geometry.charges=state.charges;
  geometry.edges=state.edges;
  geometry.width=state.width;
  geometry.orientation_axis=state.orientation_axis;
  MappedCompactPreparation mapped_preparation;
  FiniteSupportPairPreparation full_preparation;
  if(resident) mapped_preparation=prepare_mapped_compact_pair(
      geometry,action_options,observer_options.support_half_width,
      observer_options.poisson_tolerance,
      observer_options.poisson_max_iterations,
      observer_options.allow_fractional_center);
  else full_preparation=prepare_finite_support_derived_compact_pair(
      geometry,action_options,observer_options.support_half_width,
      observer_options.poisson_tolerance,
      observer_options.poisson_max_iterations,
      observer_options.allow_fractional_center);
  const auto& preparation=resident
      ?mapped_preparation.local:full_preparation;
  result.bound_poisson_residual=preparation.poisson_residual;
  result.bound_gauss_residual=preparation.gauss_residual;
  result.bound_outside_maximum=preparation.outside_maximum;
  result.bound_boundary_crossing_maximum=
      preparation.boundary_crossing_maximum;
  if((resident&&!mapped_preparation.valid)
      ||!preparation.valid||!preparation.compact_support
      ||!preparation.zero_boundary_crossing) {
    fail(telemetry,"compact observer preparation failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  result.center=resident?mapped_preparation.physical_center:preparation.center;
  result.support_center=resident?mapped_preparation.target_center
      :preparation.support_center;
  result.fractional_center_offset=resident
      ?mapped_preparation.fractional_center_offset
      :preparation.fractional_center_offset;
  result.fractional_center_enabled=observer_options.allow_fractional_center;
  result.net_charge=std::accumulate(
      state.charges.begin(),state.charges.end(),0);
  const double rest=action_options.constituent_mass_scale*E_REST;
  for(const auto& point:state.constituents)
    result.constituent_kinetic_energy+=std::sqrt(
        rest*rest+C_SPEED*C_SPEED*point.momentum.mag2())-rest;
  result.pair_internal_energy=connected_moore_block_binding_energy(
      state,action_options);
  for(const int radius:observer_options.shell_radii) {
    if(radius<=0||radius>result.L/2) {
      fail(telemetry,"invalid characteristic shell radius");
      if(telemetry_out) *telemetry_out=telemetry;
      return result;
    }
    StateOnlyCharacteristicShell shell;
    shell.radius=radius;
    result.shells.push_back(shell);
  }

  const std::size_t bytes=count*sizeof(double);
  DeviceTriplet actual_e,actual_b,bound_e,bound_b;
  const bool owns_actual=!resident;
  if(resident) {
    actual_e={const_cast<double*>(resident_electric->x),
        const_cast<double*>(resident_electric->y),
        const_cast<double*>(resident_electric->z)};
    actual_b={const_cast<double*>(resident_magnetic->x),
        const_cast<double*>(resident_magnetic->y),
        const_cast<double*>(resident_magnetic->z)};
  }
  double* sum_partial=nullptr;
  double* maximum_partial=nullptr;
  const int blocks=std::min<int>(kMaximumBlocks,
      static_cast<int>((count+kThreads-1)/kThreads));
  const std::size_t sum_values=static_cast<std::size_t>(
      std::max(kMatterSumQuantities,kShellQuantities))*blocks;
  const std::size_t maximum_values=static_cast<std::size_t>(
      kMatterMaximumQuantities)*blocks;
  const auto allocation_start=std::chrono::steady_clock::now();
  const bool allocated=(!owns_actual||allocate(actual_e,bytes))
      &&(!owns_actual||allocate(actual_b,bytes))
      &&allocate(bound_e,bytes)&&allocate(bound_b,bytes)
      &&cudaMalloc(&sum_partial,sum_values*sizeof(double))==cudaSuccess
      &&cudaMalloc(&maximum_partial,maximum_values*sizeof(double))
          ==cudaSuccess;
  telemetry.allocation_ms=milliseconds_since(allocation_start);
  const auto cleanup=[&]() {
    if(owns_actual) { release(actual_e); release(actual_b); }
    release(bound_e); release(bound_b);
    cudaFree(sum_partial); cudaFree(maximum_partial);
  };
  if(!allocated) {
    cleanup(); fail(telemetry,"CUDA observer allocation failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  const auto upload_start=std::chrono::steady_clock::now();
  if((owns_actual&&(!upload(actual_e,state.electric,bytes)
      ||!upload(actual_b,state.magnetic_half,bytes)))
      ||(resident?!upload_sparse_mapped(
              bound_e,preparation.state.electric,bytes,result.L,
              mapped_preparation.target_center,telemetry)
          :!upload_sparse(bound_e,preparation.state.electric,bytes,telemetry))
      ||(resident?!upload_sparse_mapped(
              bound_b,preparation.state.magnetic_half,bytes,result.L,
              mapped_preparation.target_center,telemetry)
          :!upload_sparse(bound_b,preparation.state.magnetic_half,bytes,
              telemetry))) {
    cleanup(); fail(telemetry,"observer field upload failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  telemetry.upload_ms=milliseconds_since(upload_start);
  if(owns_actual) telemetry.host_to_device_bytes+=6*bytes;

  const int cx=static_cast<int>(std::llround(result.support_center.x));
  const int cy=static_cast<int>(std::llround(result.support_center.y));
  const int cz=static_cast<int>(std::llround(result.support_center.z));
  const double half_step_scale=
      -0.5*observer_options.wave_speed*observer_options.dt;
  const auto kernel_start=std::chrono::steady_clock::now();
  matter_observer_global_kernel<<<blocks,kThreads,
      (kMatterSumQuantities+kMatterMaximumQuantities)
          *kThreads*sizeof(double)>>>(
      actual_e,actual_b,bound_e,bound_b,count,result.L,cx,cy,cz,
      result.center.x,result.center.y,result.center.z,
      half_step_scale,observer_options.gate_tolerance,
      sum_partial,maximum_partial);
  if(cudaGetLastError()!=cudaSuccess||cudaDeviceSynchronize()!=cudaSuccess) {
    cleanup(); fail(telemetry,"state-only observer kernel failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  telemetry.kernel_ms+=milliseconds_since(kernel_start);
  const auto sums=download_reduce(
      sum_partial,kMatterSumQuantities,blocks,telemetry);
  const auto maxima=download_maximum(
      maximum_partial,kMatterMaximumQuantities,blocks,telemetry);
  if(sums.size()!=kMatterSumQuantities
      ||maxima.size()!=kMatterMaximumQuantities) {
    cleanup(); fail(telemetry,"state-only observer reduction failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }

  for(std::size_t shell_index=0;shell_index<result.shells.size();++shell_index) {
    const auto shell_start=std::chrono::steady_clock::now();
    matter_observer_shell_kernel<<<blocks,kThreads,
        kShellQuantities*kThreads*sizeof(double)>>>(
        actual_e,actual_b,bound_e,bound_b,count,result.L,cx,cy,cz,
        result.center.x,result.center.y,result.center.z,
        result.shells[shell_index].radius,half_step_scale,
        observer_options.gate_tolerance,sum_partial);
    if(cudaGetLastError()!=cudaSuccess||cudaDeviceSynchronize()!=cudaSuccess) {
      cleanup(); fail(telemetry,"characteristic shell kernel failed");
      if(telemetry_out) *telemetry_out=telemetry;
      return result;
    }
    telemetry.kernel_ms+=milliseconds_since(shell_start);
    const auto values=download_reduce(
        sum_partial,kShellQuantities,blocks,telemetry);
    if(values.size()!=kShellQuantities) {
      cleanup(); fail(telemetry,"characteristic shell reduction failed");
      if(telemetry_out) *telemetry_out=telemetry;
      return result;
    }
    auto& shell=result.shells[shell_index];
    shell.samples=static_cast<int>(std::llround(values[0]));
    shell.residual_energy=values[1];
    shell.outgoing_energy=values[2];
    shell.incoming_energy=values[3];
    shell.radial_energy=values[4];
    shell.background_energy=values[5];
    shell.signed_radial_poynting=values[6];
    shell.outward_characteristic_power=shell.outgoing_energy;
    shell.inward_characteristic_power=shell.incoming_energy;
  }
  cleanup();

  result.bound_energy=sums[1];
  result.residual_energy=sums[2];
  result.outgoing_energy=sums[3];
  result.incoming_energy=sums[4];
  result.radial_energy=sums[5];
  result.background_energy=sums[6];
  result.signed_radial_poynting=sums[7];
  result.outward_characteristic_power=result.outgoing_energy;
  result.inward_characteristic_power=result.incoming_energy;
  result.primitive_face_interference=sums[8];
  result.centered_electric_interference=sums[9];
  result.centered_magnetic_interference=sums[10];
  result.bound_residual_interference=sums[0]-sums[1]-sums[2];
  result.centering_metric_interference=
      result.centered_electric_interference
      -result.primitive_face_interference;
  result.maximum_reconstruction_residual=maxima[0];
  result.actual_gauss_compatibility_residual=maxima[1];
  result.characteristic_flux_residual=maxima[2];
  complete_boundary_ledger(result,state,preparation.state,
      observer_options.support_half_width,observer_options.gate_tolerance,
      resident_electric,&telemetry,
      resident?&mapped_preparation:nullptr);
  result.energy_partition_residual=result.residual_energy
      -result.outgoing_energy-result.background_energy;
  const double norm=relative_scale(result.residual_energy,
      result.outgoing_energy,result.background_energy);
  result.valid=maxima[4]==0.0
      &&result.maximum_reconstruction_residual
          <=observer_options.gate_tolerance*norm
      &&result.actual_gauss_compatibility_residual
          <=observer_options.gate_tolerance
      &&std::abs(result.energy_partition_residual)
          <=observer_options.gate_tolerance*norm
      &&result.characteristic_flux_residual
          <=observer_options.gate_tolerance*norm
      &&std::isfinite(result.pair_internal_energy);
  telemetry.valid=result.valid&&telemetry.error.empty();
  if(telemetry_out) *telemetry_out=telemetry;
  return result;
}

StateOnlyMatterFieldObservation observe_state_only_matter_field_cuda(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const StateOnlyMatterFieldObserverOptions& observer_options,
    CudaStateOnlySupportLadderTelemetry* telemetry_out) {
  return observe_matter_field_impl(state,action_options,nullptr,nullptr,
      observer_options,telemetry_out);
}

StateOnlyMatterFieldObservation observe_state_only_matter_field_cuda_resident(
    const ConnectedMooreBlockState& matter_only,
    const ConnectedMooreBlockOptions& action_options,
    const CudaMatchedFieldDeviceView& actual_electric,
    const CudaMatchedFieldDeviceView& actual_magnetic,
    const StateOnlyMatterFieldObserverOptions& observer_options,
    CudaStateOnlySupportLadderTelemetry* telemetry_out) {
  return observe_matter_field_impl(matter_only,action_options,
      &actual_electric,&actual_magnetic,observer_options,telemetry_out);
}

}  // namespace ftd::eft
