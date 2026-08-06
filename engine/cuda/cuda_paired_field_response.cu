#include "ftd/eft/cuda_paired_field_response.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <vector>

namespace ftd::eft {
namespace {

constexpr int kThreads=256;
constexpr int kMaximumBlocks=4096;
constexpr int kPairedQuantities=14;
constexpr int kTransportQuantities=6;

struct DeviceTriplet {
  const double* x=nullptr;
  const double* y=nullptr;
  const double* z=nullptr;
};

struct OwnedTriplet {
  double* x=nullptr;
  double* y=nullptr;
  double* z=nullptr;
  DeviceTriplet view() const { return {x,y,z}; }
};

double milliseconds_since(const std::chrono::steady_clock::time_point& start) {
  return std::chrono::duration<double,std::milli>(
      std::chrono::steady_clock::now()-start).count();
}

bool allocate(OwnedTriplet& field,std::size_t bytes) {
  return cudaMalloc(&field.x,bytes)==cudaSuccess
      &&cudaMalloc(&field.y,bytes)==cudaSuccess
      &&cudaMalloc(&field.z,bytes)==cudaSuccess;
}

void release(OwnedTriplet& field) {
  cudaFree(field.x); cudaFree(field.y); cudaFree(field.z); field={};
}

template <typename Field>
bool upload(OwnedTriplet& target,const Field& source,std::size_t bytes) {
  return cudaMemcpy(target.x,source.x.data(),bytes,cudaMemcpyHostToDevice)
          ==cudaSuccess
      &&cudaMemcpy(target.y,source.y.data(),bytes,cudaMemcpyHostToDevice)
          ==cudaSuccess
      &&cudaMemcpy(target.z,source.z.data(),bytes,cudaMemcpyHostToDevice)
          ==cudaSuccess;
}

DeviceTriplet view(const CudaMatchedFieldDeviceView& field) {
  return {field.x,field.y,field.z};
}

__device__ int wrap_coordinate(int value,int L) {
  value%=L;
  return value<0?value+L:value;
}

__device__ std::size_t index_at(int x,int y,int z,int L) {
  return (static_cast<std::size_t>(wrap_coordinate(x,L))*L
      +wrap_coordinate(y,L))*L+wrap_coordinate(z,L);
}

__device__ double component(DeviceTriplet field,int axis,
                            int x,int y,int z,int L) {
  const auto index=index_at(x,y,z,L);
  return axis==0?field.x[index]:(axis==1?field.y[index]:field.z[index]);
}

__device__ double curl_component(DeviceTriplet edge,int axis,
                                 int x,int y,int z,int L) {
  const auto f=[&](int c,int xx,int yy,int zz) {
    return component(edge,c,xx,yy,zz,L);
  };
  if(axis==0)
    return f(2,x,y,z)-f(2,x,y-1,z)-f(1,x,y,z)+f(1,x,y,z-1);
  if(axis==1)
    return f(0,x,y,z)-f(0,x,y,z-1)-f(2,x,y,z)+f(2,x-1,y,z);
  return f(1,x,y,z)-f(1,x-1,y,z)-f(0,x,y,z)+f(0,x,y-1,z);
}

__device__ double curl_adjoint_component(DeviceTriplet face,int axis,
                                         int x,int y,int z,int L) {
  const auto f=[&](int c,int xx,int yy,int zz) {
    return component(face,c,xx,yy,zz,L);
  };
  if(axis==0)
    return f(2,x,y+1,z)-f(2,x,y,z)-f(1,x,y,z+1)+f(1,x,y,z);
  if(axis==1)
    return f(0,x,y,z+1)-f(0,x,y,z)-f(2,x+1,y,z)+f(2,x,y,z);
  return f(1,x+1,y,z)-f(1,x,y,z)-f(0,x,y+1,z)+f(0,x,y,z);
}

__device__ double integer_magnetic_component(
    DeviceTriplet electric,DeviceTriplet magnetic,int axis,
    int x,int y,int z,int L,double half_step_scale) {
  return component(magnetic,axis,x,y,z,L)
      +half_step_scale*curl_adjoint_component(electric,axis,x,y,z,L);
}

__device__ double periodic_delta(double coordinate,double center,int L) {
  double result=coordinate-center;
  const double half=0.5*static_cast<double>(L);
  if(result>half) result-=static_cast<double>(L);
  if(result<-half) result+=static_cast<double>(L);
  return result;
}

__device__ bool region_contains(
    int kind,double cx,double cy,double cz,
    double lx,double ly,double lz,
    double ux,double uy,double uz,double vx,double vy,double vz,
    double longitudinal_half_width,double transverse_half_width,
    double cube_radius,double px,double py,double pz,int L) {
  const double dx=periodic_delta(px,cx,L);
  const double dy=periodic_delta(py,cy,L);
  const double dz=periodic_delta(pz,cz,L);
  if(kind==static_cast<int>(FieldResponseRegionKind::ChebyshevCube))
    return max(abs(dx),max(abs(dy),abs(dz)))<=cube_radius;
  return abs(dx*lx+dy*ly+dz*lz)<=longitudinal_half_width
      &&abs(dx*ux+dy*uy+dz*uz)<=transverse_half_width
      &&abs(dx*vx+dy*vy+dz*vz)<=transverse_half_width;
}

__device__ double region_longitudinal(
    double cx,double cy,double cz,double lx,double ly,double lz,
    double px,double py,double pz,int L) {
  return periodic_delta(px,cx,L)*lx+periodic_delta(py,cy,L)*ly
      +periodic_delta(pz,cz,L)*lz;
}

__device__ void component_position(int family,int axis,int x,int y,int z,
                                   double& px,double& py,double& pz) {
  px=static_cast<double>(x); py=static_cast<double>(y);
  pz=static_cast<double>(z);
  if(family==0) {
    if(axis==0) px+=0.5;
    else if(axis==1) py+=0.5;
    else pz+=0.5;
  } else {
    if(axis==0) { py+=0.5; pz+=0.5; }
    else if(axis==1) { px+=0.5; pz+=0.5; }
    else { px+=0.5; py+=0.5; }
  }
}

__device__ void reduce_sum(double* values,int quantities) {
  __syncthreads();
  for(int stride=blockDim.x/2;stride>0;stride>>=1) {
    if(threadIdx.x<stride) for(int q=0;q<quantities;++q)
      values[q*blockDim.x+threadIdx.x]
          +=values[q*blockDim.x+threadIdx.x+stride];
    __syncthreads();
  }
}

__global__ void paired_region_kernel(
    DeviceTriplet moving_e,DeviceTriplet moving_b,
    DeviceTriplet rest_e,DeviceTriplet rest_b,
    DeviceTriplet moving_bound_e,DeviceTriplet moving_bound_b,
    DeviceTriplet rest_bound_e,DeviceTriplet rest_bound_b,
    std::size_t count,int L,double half_step_scale,
    int kind,double cx,double cy,double cz,
    double lx,double ly,double lz,
    double ux,double uy,double uz,double vx,double vy,double vz,
    double longitudinal_half_width,double transverse_half_width,
    double cube_radius,double* partial) {
  extern __shared__ double shared[];
  double values[kPairedQuantities]{};
  const std::size_t first=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  const std::size_t stride=static_cast<std::size_t>(gridDim.x)*blockDim.x;
  const std::size_t plane=static_cast<std::size_t>(L)*L;
  for(std::size_t linear=first;linear<count;linear+=stride) {
    const int x=static_cast<int>(linear/plane);
    const auto rem=linear-static_cast<std::size_t>(x)*plane;
    const int y=static_cast<int>(rem/L);
    const int z=static_cast<int>(rem-static_cast<std::size_t>(y)*L);
    for(int family=0;family<2;++family) for(int axis=0;axis<3;++axis) {
      double px=0.0,py=0.0,pz=0.0;
      component_position(family,axis,x,y,z,px,py,pz);
      if(!region_contains(kind,cx,cy,cz,lx,ly,lz,ux,uy,uz,vx,vy,vz,
          longitudinal_half_width,transverse_half_width,cube_radius,
          px,py,pz,L)) continue;
      const double longitudinal=region_longitudinal(
          cx,cy,cz,lx,ly,lz,px,py,pz,L);
      const double ma=family==0?component(moving_e,axis,x,y,z,L)
          :integer_magnetic_component(moving_e,moving_b,axis,x,y,z,L,
                                      half_step_scale);
      const double ra=family==0?component(rest_e,axis,x,y,z,L)
          :integer_magnetic_component(rest_e,rest_b,axis,x,y,z,L,
                                      half_step_scale);
      const double mb=family==0?component(moving_bound_e,axis,x,y,z,L)
          :integer_magnetic_component(moving_bound_e,moving_bound_b,
                                      axis,x,y,z,L,half_step_scale);
      const double rb=family==0?component(rest_bound_e,axis,x,y,z,L)
          :integer_magnetic_component(rest_bound_e,rest_bound_b,
                                      axis,x,y,z,L,half_step_scale);
      const double channel_values[4]={ma,ra,ma-mb,ra-rb};
      for(int channel=0;channel<2;++channel) {
        const double moving=channel_values[2*channel];
        const double rest=channel_values[2*channel+1];
        const double difference=moving-rest;
        const double moving_energy=0.5*moving*moving;
        const double rest_energy=0.5*rest*rest;
        const double difference_energy=0.5*difference*difference;
        const double cross=rest*difference;
        const double energy_difference=moving_energy-rest_energy;
        const int base=7*channel;
        values[base]+=moving_energy;
        values[base+1]+=rest_energy;
        values[base+2]+=difference_energy;
        values[base+3]+=cross;
        values[base+4]+=energy_difference*longitudinal;
        values[base+5]+=difference_energy*longitudinal;
        values[base+6]+=cross*longitudinal;
      }
    }
  }
  for(int q=0;q<kPairedQuantities;++q)
    shared[q*blockDim.x+threadIdx.x]=values[q];
  reduce_sum(shared,kPairedQuantities);
  if(threadIdx.x==0) for(int q=0;q<kPairedQuantities;++q)
    partial[static_cast<std::size_t>(q)*gridDim.x+blockIdx.x]
        =shared[q*blockDim.x];
}

__device__ double modified_component_energy(
    int family,int axis,DeviceTriplet electric,DeviceTriplet magnetic,
    int x,int y,int z,int L,double lambda) {
  if(family==0) {
    const double e=component(electric,axis,x,y,z,L);
    return 0.5*e*e-0.25*lambda*e
        *curl_component(magnetic,axis,x,y,z,L);
  }
  const double b=component(magnetic,axis,x,y,z,L);
  return 0.5*b*b-0.25*lambda*b
      *curl_adjoint_component(electric,axis,x,y,z,L);
}

__global__ void regional_transport_kernel(
    DeviceTriplet electric_before,DeviceTriplet magnetic_before,
    DeviceTriplet electric_pre,DeviceTriplet magnetic_after,
    DeviceTriplet electric_after,std::size_t count,int L,double lambda,
    int kind,double cx,double cy,double cz,
    double lx,double ly,double lz,
    double ux,double uy,double uz,double vx,double vy,double vz,
    double longitudinal_half_width,double transverse_half_width,
    double cube_radius,double* partial) {
  extern __shared__ double shared[];
  double values[kTransportQuantities]{};
  const std::size_t first=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  const std::size_t stride=static_cast<std::size_t>(gridDim.x)*blockDim.x;
  const std::size_t plane=static_cast<std::size_t>(L)*L;
  for(std::size_t linear=first;linear<count;linear+=stride) {
    const int x=static_cast<int>(linear/plane);
    const auto rem=linear-static_cast<std::size_t>(x)*plane;
    const int y=static_cast<int>(rem/L);
    const int z=static_cast<int>(rem-static_cast<std::size_t>(y)*L);
    for(int family=0;family<2;++family) for(int axis=0;axis<3;++axis) {
      double px=0.0,py=0.0,pz=0.0;
      component_position(family,axis,x,y,z,px,py,pz);
      const bool inside=region_contains(kind,cx,cy,cz,lx,ly,lz,
          ux,uy,uz,vx,vy,vz,longitudinal_half_width,
          transverse_half_width,cube_radius,px,py,pz,L);
      const int base=inside?0:3;
      values[base]+=modified_component_energy(family,axis,electric_before,
          magnetic_before,x,y,z,L,lambda);
      values[base+1]+=modified_component_energy(family,axis,electric_pre,
          magnetic_after,x,y,z,L,lambda);
      values[base+2]+=modified_component_energy(family,axis,electric_after,
          magnetic_after,x,y,z,L,lambda);
    }
  }
  for(int q=0;q<kTransportQuantities;++q)
    shared[q*blockDim.x+threadIdx.x]=values[q];
  reduce_sum(shared,kTransportQuantities);
  if(threadIdx.x==0) for(int q=0;q<kTransportQuantities;++q)
    partial[static_cast<std::size_t>(q)*gridDim.x+blockIdx.x]
        =shared[q*blockDim.x];
}

std::vector<double> download_reduce(double* partial,int quantities,int blocks,
    CudaPairedFieldResponseTelemetry& telemetry) {
  std::vector<double> host(static_cast<std::size_t>(quantities)*blocks);
  const std::size_t bytes=host.size()*sizeof(double);
  if(cudaMemcpy(host.data(),partial,bytes,cudaMemcpyDeviceToHost)!=cudaSuccess)
    return {};
  telemetry.device_to_host_bytes+=bytes;
  std::vector<double> result(quantities,0.0);
  for(int q=0;q<quantities;++q) for(int block=0;block<blocks;++block)
    result[q]+=host[static_cast<std::size_t>(q)*blocks+block];
  return result;
}

ConnectedMooreBlockState geometry_only(const ConnectedMooreBlockState& state) {
  ConnectedMooreBlockState result;
  result.electric.L=state.electric.L;
  result.magnetic_half.L=state.magnetic_half.L;
  result.constituents=state.constituents;
  result.charges=state.charges;
  result.edges=state.edges;
  result.width=state.width;
  result.orientation_axis=state.orientation_axis;
  return result;
}

void assign_channel(QuadraticFieldDifferenceChannel& result,
                    const std::vector<double>& values,int base) {
  result.moving_energy=values[base];
  result.rest_energy=values[base+1];
  result.energy_difference=result.moving_energy-result.rest_energy;
  result.difference_field_energy=values[base+2];
  result.cross_energy=values[base+3];
  result.energy_identity_residual=result.energy_difference
      -result.cross_energy-result.difference_field_energy;
  result.energy_difference_first_moment=values[base+4];
  result.difference_field_first_moment=values[base+5];
  result.cross_first_moment=values[base+6];
}

double maximum_scale(const PairedFieldResponseObservation& result) {
  double scale=1.0;
  for(const auto& region:result.regions) for(const auto* channel:
      {&region.actual,&region.residual})
    scale=std::max({scale,std::abs(channel->moving_energy),
        std::abs(channel->rest_energy),
        std::abs(channel->difference_field_energy),
        std::abs(channel->cross_energy)});
  return scale;
}

void fail(CudaPairedFieldResponseTelemetry& telemetry,const char* error) {
  telemetry.valid=false; telemetry.error=error;
}

}  // namespace

PairedFieldResponseObservation observe_paired_field_response_cuda(
    const ConnectedMooreBlockState& moving,
    const ConnectedMooreBlockState& rest,
    const ConnectedMooreBlockOptions& action_options,
    const PairedFieldResponseOptions& options,
    CudaPairedFieldResponseTelemetry* telemetry_out) {
  PairedFieldResponseObservation result;
  CudaPairedFieldResponseTelemetry telemetry;
  result.L=moving.electric.L;
  const int L=result.L;
  const std::size_t count=L>0?static_cast<std::size_t>(L)*L*L:0;
  const bool valid=L>0&&rest.electric.L==L&&moving.magnetic_half.L==L
      &&rest.magnetic_half.L==L&&moving.electric.x.size()==count
      &&rest.electric.x.size()==count&&moving.magnetic_half.x.size()==count
      &&rest.magnetic_half.x.size()==count&&moving.constituents.size()==2
      &&rest.constituents.size()==2&&moving.charges.size()==2
      &&rest.charges.size()==2&&options.outer_radius<=L/2
      &&options.wave_speed>0.0&&options.dt>0.0&&options.gate_tolerance>0.0;
  if(!valid) {
    fail(telemetry,"invalid paired-response input");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  const auto moving_bound=prepare_finite_support_derived_compact_pair(
      geometry_only(moving),action_options,options.support_half_width,
      options.poisson_tolerance,options.poisson_max_iterations,true);
  const auto rest_bound=prepare_finite_support_derived_compact_pair(
      geometry_only(rest),action_options,options.support_half_width,
      options.poisson_tolerance,options.poisson_max_iterations,true);
  if(!moving_bound.valid||!rest_bound.valid||!moving_bound.compact_support
      ||!rest_bound.compact_support||!moving_bound.zero_boundary_crossing
      ||!rest_bound.zero_boundary_crossing) {
    fail(telemetry,"paired compact preparation failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  result.moving_bound_center=moving_bound.center;
  result.rest_bound_center=rest_bound.center;
  result.moving_bound_gauss_residual=moving_bound.gauss_residual;
  result.rest_bound_gauss_residual=rest_bound.gauss_residual;
  const auto regions=make_ftd0768_response_regions(options);
  for(std::size_t i=0;i<regions.size();++i) result.regions[i].spec=regions[i];

  OwnedTriplet me,mb,re,rb,mbe,mbb,rbe,rbb;
  double* partial=nullptr;
  const std::size_t bytes=count*sizeof(double);
  const int blocks=std::min<int>(kMaximumBlocks,
      static_cast<int>((count+kThreads-1)/kThreads));
  const auto allocation_start=std::chrono::steady_clock::now();
  const bool allocated=allocate(me,bytes)&&allocate(mb,bytes)
      &&allocate(re,bytes)&&allocate(rb,bytes)&&allocate(mbe,bytes)
      &&allocate(mbb,bytes)&&allocate(rbe,bytes)&&allocate(rbb,bytes)
      &&cudaMalloc(&partial,static_cast<std::size_t>(kPairedQuantities)
          *blocks*sizeof(double))==cudaSuccess;
  telemetry.allocation_ms=milliseconds_since(allocation_start);
  const auto cleanup=[&]() {
    release(me); release(mb); release(re); release(rb); release(mbe);
    release(mbb); release(rbe); release(rbb); cudaFree(partial);
  };
  if(!allocated) {
    cleanup(); fail(telemetry,"paired-response allocation failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  const auto upload_start=std::chrono::steady_clock::now();
  const bool uploaded=upload(me,moving.electric,bytes)
      &&upload(mb,moving.magnetic_half,bytes)&&upload(re,rest.electric,bytes)
      &&upload(rb,rest.magnetic_half,bytes)
      &&upload(mbe,moving_bound.state.electric,bytes)
      &&upload(mbb,moving_bound.state.magnetic_half,bytes)
      &&upload(rbe,rest_bound.state.electric,bytes)
      &&upload(rbb,rest_bound.state.magnetic_half,bytes);
  telemetry.upload_ms=milliseconds_since(upload_start);
  telemetry.host_to_device_bytes=24*bytes;
  if(!uploaded) {
    cleanup(); fail(telemetry,"paired-response upload failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  const double half_step_scale=-0.5*options.wave_speed*options.dt;
  for(std::size_t index=0;index<regions.size();++index) {
    const auto& region=regions[index];
    const auto kernel_start=std::chrono::steady_clock::now();
    paired_region_kernel<<<blocks,kThreads,
        kPairedQuantities*kThreads*sizeof(double)>>>(
        me.view(),mb.view(),re.view(),rb.view(),mbe.view(),mbb.view(),
        rbe.view(),rbb.view(),count,L,half_step_scale,
        static_cast<int>(region.kind),region.center.x,region.center.y,
        region.center.z,region.longitudinal.x,region.longitudinal.y,
        region.longitudinal.z,region.transverse_u.x,region.transverse_u.y,
        region.transverse_u.z,region.transverse_v.x,region.transverse_v.y,
        region.transverse_v.z,region.longitudinal_half_width,
        region.transverse_half_width,region.chebyshev_radius,partial);
    if(cudaGetLastError()!=cudaSuccess||cudaDeviceSynchronize()!=cudaSuccess) {
      cleanup(); fail(telemetry,"paired-response kernel failed");
      if(telemetry_out) *telemetry_out=telemetry;
      return result;
    }
    telemetry.kernel_ms+=milliseconds_since(kernel_start);
    const auto values=download_reduce(
        partial,kPairedQuantities,blocks,telemetry);
    if(values.size()!=kPairedQuantities) {
      cleanup(); fail(telemetry,"paired-response reduction failed");
      if(telemetry_out) *telemetry_out=telemetry;
      return result;
    }
    assign_channel(result.regions[index].actual,values,0);
    assign_channel(result.regions[index].residual,values,7);
  }
  cleanup();
  result.maximum_energy_identity_residual=0.0;
  for(const auto& region:result.regions)
    result.maximum_energy_identity_residual=std::max({
        result.maximum_energy_identity_residual,
        std::abs(region.actual.energy_identity_residual),
        std::abs(region.residual.energy_identity_residual)});
  result.valid=result.maximum_energy_identity_residual
          <=options.gate_tolerance*maximum_scale(result)
      &&result.moving_bound_gauss_residual<=options.gate_tolerance
      &&result.rest_bound_gauss_residual<=options.gate_tolerance;
  telemetry.valid=result.valid&&telemetry.error.empty();
  if(telemetry_out) *telemetry_out=telemetry;
  return result;
}

RegionalModifiedEnergyTransportObservation
observe_regional_modified_energy_transport_cuda(
    const CudaMatchedFieldResidentViews& views,double lambda,
    const FieldResponseRegionSpec& region,double tolerance,
    CudaPairedFieldResponseTelemetry* telemetry_out) {
  RegionalModifiedEnergyTransportObservation result;
  CudaPairedFieldResponseTelemetry telemetry;
  result.spec=region;
  const int L=views.electric_before.L;
  const bool valid=views.prepared&&views.current_applied&&lambda>0.0
      &&tolerance>0.0&&views.electric_before.valid()
      &&views.magnetic_before.valid()&&views.magnetic_prepared.valid()
      &&views.electric_pre_current.valid()&&views.electric_after.valid()
      &&views.magnetic_before.L==L&&views.magnetic_prepared.L==L
      &&views.electric_pre_current.L==L&&views.electric_after.L==L;
  if(!valid) {
    fail(telemetry,"invalid regional-transport input");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  const std::size_t count=static_cast<std::size_t>(L)*L*L;
  const int blocks=std::min<int>(kMaximumBlocks,
      static_cast<int>((count+kThreads-1)/kThreads));
  double* partial=nullptr;
  const auto allocation_start=std::chrono::steady_clock::now();
  const bool allocated=cudaMalloc(&partial,
      static_cast<std::size_t>(kTransportQuantities)*blocks*sizeof(double))
      ==cudaSuccess;
  telemetry.allocation_ms=milliseconds_since(allocation_start);
  if(!allocated) {
    cudaFree(partial); fail(telemetry,"regional-transport allocation failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  const auto kernel_start=std::chrono::steady_clock::now();
  regional_transport_kernel<<<blocks,kThreads,
      kTransportQuantities*kThreads*sizeof(double)>>>(
      view(views.electric_before),view(views.magnetic_before),
      view(views.electric_pre_current),view(views.magnetic_prepared),
      view(views.electric_after),count,L,lambda,static_cast<int>(region.kind),
      region.center.x,region.center.y,region.center.z,region.longitudinal.x,
      region.longitudinal.y,region.longitudinal.z,region.transverse_u.x,
      region.transverse_u.y,region.transverse_u.z,region.transverse_v.x,
      region.transverse_v.y,region.transverse_v.z,
      region.longitudinal_half_width,region.transverse_half_width,
      region.chebyshev_radius,partial);
  if(cudaGetLastError()!=cudaSuccess||cudaDeviceSynchronize()!=cudaSuccess) {
    cudaFree(partial); fail(telemetry,"regional-transport kernel failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  telemetry.kernel_ms=milliseconds_since(kernel_start);
  const auto values=download_reduce(
      partial,kTransportQuantities,blocks,telemetry);
  cudaFree(partial);
  if(values.size()!=kTransportQuantities) {
    fail(telemetry,"regional-transport reduction failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  result.energy_before=values[0];
  result.energy_pre_current=values[1];
  result.energy_after=values[2];
  result.outside_energy_before=values[3];
  result.outside_energy_pre_current=values[4];
  result.outside_energy_after=values[5];
  result.boundary_transport_into=result.energy_pre_current
      -result.energy_before;
  result.boundary_transport_into_complement=
      result.outside_energy_pre_current-result.outside_energy_before;
  result.source_exchange_into_field=result.energy_after
      -result.energy_pre_current;
  result.energy_change=result.energy_after-result.energy_before;
  result.global_source_free_residual=
      (result.energy_pre_current+result.outside_energy_pre_current)
      -(result.energy_before+result.outside_energy_before);
  result.boundary_quadrature_residual=result.boundary_transport_into
      +result.boundary_transport_into_complement;
  result.ledger_residual=result.energy_change-result.boundary_transport_into
      -result.source_exchange_into_field;
  const double scale=std::max({1.0,std::abs(result.energy_before),
      std::abs(result.energy_pre_current),std::abs(result.energy_after),
      std::abs(result.outside_energy_before),
      std::abs(result.outside_energy_pre_current),
      std::abs(result.outside_energy_after)});
  result.valid=std::abs(result.ledger_residual)<=tolerance*scale
      &&std::abs(result.global_source_free_residual)<=tolerance*scale
      &&std::abs(result.boundary_quadrature_residual)<=tolerance*scale;
  telemetry.valid=result.valid&&telemetry.error.empty();
  if(telemetry_out) *telemetry_out=telemetry;
  return result;
}

}  // namespace ftd::eft
