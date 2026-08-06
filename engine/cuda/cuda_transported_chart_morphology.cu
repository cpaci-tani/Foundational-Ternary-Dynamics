#include "ftd/eft/cuda_transported_chart_morphology.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <vector>

namespace ftd::eft {
namespace {

constexpr int kThreads=256;
constexpr int kMaximumBlocks=4096;
constexpr int kSummaryQuantities=20;
constexpr int kModeQuantities=10;

struct DeviceTriplet { double* x=nullptr; double* y=nullptr; double* z=nullptr; };

double milliseconds_since(const std::chrono::steady_clock::time_point& start) {
  return std::chrono::duration<double,std::milli>(
      std::chrono::steady_clock::now()-start).count();
}

bool allocate(DeviceTriplet& field,std::size_t bytes) {
  return cudaMalloc(&field.x,bytes)==cudaSuccess
      &&cudaMalloc(&field.y,bytes)==cudaSuccess
      &&cudaMalloc(&field.z,bytes)==cudaSuccess;
}

void release(DeviceTriplet& field) {
  cudaFree(field.x); cudaFree(field.y); cudaFree(field.z);
  field={};
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

__device__ double curl_adjoint_component(DeviceTriplet field,int axis,
                                         int x,int y,int z,int L) {
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
    DeviceTriplet electric,DeviceTriplet magnetic,int axis,
    int x,int y,int z,int L,double half_step_scale) {
  return component(magnetic,axis,x,y,z,L)
      +half_step_scale*curl_adjoint_component(electric,axis,x,y,z,L);
}

__device__ int shortest_delta(int coordinate,int center,int L) {
  int delta=coordinate-center;
  if(delta>L/2) delta-=L;
  if(delta<-L/2) delta+=L;
  return delta;
}

__device__ double shortest_delta(double coordinate,double center,int L) {
  double delta=coordinate-center;
  const double half=0.5*static_cast<double>(L);
  if(delta>half) delta-=static_cast<double>(L);
  if(delta<-half) delta+=static_cast<double>(L);
  return delta;
}

__device__ void reduce_sum(double* values,int quantities) {
  __syncthreads();
  for(int stride=blockDim.x/2;stride>0;stride>>=1) {
    if(threadIdx.x<stride)
      for(int q=0;q<quantities;++q)
        values[q*blockDim.x+threadIdx.x]
            +=values[q*blockDim.x+threadIdx.x+stride];
    __syncthreads();
  }
}

__global__ void summary_kernel(
    DeviceTriplet actual_e,DeviceTriplet actual_b,
    DeviceTriplet bound_e,DeviceTriplet bound_b,
    std::size_t count,int L,int cx,int cy,int cz,
    double physical_cx,double physical_cy,double physical_cz,
    int near_radius,int outer_radius,double half_step_scale,
    int longitudinal_enabled,double longitudinal_x,double longitudinal_y,
    double longitudinal_z,double longitudinal_dead_band,
    double* partial) {
  extern __shared__ double shared[];
  double values[kSummaryQuantities]{};
  const std::size_t first=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  const std::size_t stride=static_cast<std::size_t>(gridDim.x)*blockDim.x;
  const std::size_t plane=static_cast<std::size_t>(L)*L;
  for(std::size_t linear=first;linear<count;linear+=stride) {
    const int x=static_cast<int>(linear/plane);
    const std::size_t rem=linear-static_cast<std::size_t>(x)*plane;
    const int y=static_cast<int>(rem/L);
    const int z=static_cast<int>(rem-static_cast<std::size_t>(y)*L);
    const int chart_radius=max(abs(shortest_delta(x,cx,L)),
        max(abs(shortest_delta(y,cy,L)),abs(shortest_delta(z,cz,L))));
    const bool near=chart_radius<=near_radius;
    const bool outer=chart_radius>near_radius&&chart_radius<=outer_radius;
    for(int family=0;family<2;++family) for(int axis=0;axis<3;++axis) {
      const double ox=family==0?(axis==0?0.5:0.0):(axis==0?0.0:0.5);
      const double oy=family==0?(axis==1?0.5:0.0):(axis==1?0.0:0.5);
      const double oz=family==0?(axis==2?0.5:0.0):(axis==2?0.0:0.5);
      const double actual=family==0?component(actual_e,axis,x,y,z,L)
          :integer_edge_component(actual_e,actual_b,axis,x,y,z,L,
                                  half_step_scale);
      const double bound=family==0?component(bound_e,axis,x,y,z,L)
          :integer_edge_component(bound_e,bound_b,axis,x,y,z,L,
                                  half_step_scale);
      const double residual=actual-bound;
      const double ua=0.5*actual*actual;
      const double ub=0.5*bound*bound;
      const double ur=0.5*residual*residual;
      const double ui=bound*residual;
      values[0]+=ua; values[1]+=ub; values[2]+=ur; values[3]+=ui;
      const double dx=shortest_delta(x+ox,physical_cx,L);
      const double dy=shortest_delta(y+oy,physical_cy,L);
      const double dz=shortest_delta(z+oz,physical_cz,L);
      if(near) {
        values[4]+=ur; values[6]+=ur*dx; values[7]+=ur*dy;
        values[8]+=ur*dz; values[9]+=ur*(dx*dx+dy*dy+dz*dz);
        if(longitudinal_enabled) {
          const double longitudinal=dx*longitudinal_x
              +dy*longitudinal_y+dz*longitudinal_z;
          if(longitudinal<-longitudinal_dead_band) values[14]+=ur;
          else if(longitudinal>longitudinal_dead_band) values[16]+=ur;
          else values[15]+=ur;
        }
      } else if(outer) {
        values[5]+=ur; values[10]+=ur*dx; values[11]+=ur*dy;
        values[12]+=ur*dz; values[13]+=ur*(dx*dx+dy*dy+dz*dz);
        if(longitudinal_enabled) {
          const double longitudinal=dx*longitudinal_x
              +dy*longitudinal_y+dz*longitudinal_z;
          if(longitudinal<-longitudinal_dead_band) values[17]+=ur;
          else if(longitudinal>longitudinal_dead_band) values[19]+=ur;
          else values[18]+=ur;
        }
      }
    }
  }
  for(int q=0;q<kSummaryQuantities;++q)
    shared[q*blockDim.x+threadIdx.x]=values[q];
  reduce_sum(shared,kSummaryQuantities);
  if(threadIdx.x==0) for(int q=0;q<kSummaryQuantities;++q)
    partial[static_cast<std::size_t>(q)*gridDim.x+blockIdx.x]
        =shared[q*blockDim.x];
}

__global__ void mode_kernel(
    DeviceTriplet actual_e,DeviceTriplet actual_b,
    DeviceTriplet bound_e,DeviceTriplet bound_b,
    std::size_t count,int L,int cx,int cy,int cz,int near_radius,
    double half_step_scale,int nx,int ny,int nz,double* partial) {
  extern __shared__ double shared[];
  double values[kModeQuantities]{};
  const std::size_t first=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  const std::size_t stride=static_cast<std::size_t>(gridDim.x)*blockDim.x;
  const std::size_t plane=static_cast<std::size_t>(L)*L;
  const double phase_scale=-2.0*PI/static_cast<double>(L);
  for(std::size_t linear=first;linear<count;linear+=stride) {
    const int x=static_cast<int>(linear/plane);
    const std::size_t rem=linear-static_cast<std::size_t>(x)*plane;
    const int y=static_cast<int>(rem/L);
    const int z=static_cast<int>(rem-static_cast<std::size_t>(y)*L);
    const int chart_radius=max(abs(shortest_delta(x,cx,L)),
        max(abs(shortest_delta(y,cy,L)),abs(shortest_delta(z,cz,L))));
    const bool near=chart_radius<=near_radius;
    for(int family=0;family<2;++family) for(int axis=0;axis<3;++axis) {
      const double ox=family==0?(axis==0?0.5:0.0):(axis==0?0.0:0.5);
      const double oy=family==0?(axis==1?0.5:0.0):(axis==1?0.0:0.5);
      const double oz=family==0?(axis==2?0.5:0.0):(axis==2?0.0:0.5);
      const double actual=family==0?component(actual_e,axis,x,y,z,L)
          :integer_edge_component(actual_e,actual_b,axis,x,y,z,L,
                                  half_step_scale);
      const double bound=family==0?component(bound_e,axis,x,y,z,L)
          :integer_edge_component(bound_e,bound_b,axis,x,y,z,L,
                                  half_step_scale);
      const double residual=actual-bound;
      const double channels[5]={0.5*actual*actual,0.5*bound*bound,
          0.5*residual*residual,bound*residual,
          near?0.5*residual*residual:0.0};
      const double theta=phase_scale*(nx*(x+ox)+ny*(y+oy)+nz*(z+oz));
      double sine=0.0,cosine=0.0;
      sincos(theta,&sine,&cosine);
      for(int channel=0;channel<5;++channel) {
        values[2*channel]+=channels[channel]*cosine;
        values[2*channel+1]+=channels[channel]*sine;
      }
    }
  }
  for(int q=0;q<kModeQuantities;++q)
    shared[q*blockDim.x+threadIdx.x]=values[q];
  reduce_sum(shared,kModeQuantities);
  if(threadIdx.x==0) for(int q=0;q<kModeQuantities;++q)
    partial[static_cast<std::size_t>(q)*gridDim.x+blockIdx.x]
        =shared[q*blockDim.x];
}

std::vector<double> download_reduce(double* partial,int quantities,int blocks,
    CudaTransportedChartMorphologyTelemetry& telemetry) {
  std::vector<double> host(static_cast<std::size_t>(quantities)*blocks);
  const std::size_t bytes=host.size()*sizeof(double);
  if(cudaMemcpy(host.data(),partial,bytes,cudaMemcpyDeviceToHost)!=cudaSuccess)
    return {};
  telemetry.device_to_host_bytes+=bytes;
  std::vector<double> result(quantities,0.0);
  for(int q=0;q<quantities;++q) {
    long double sum=0.0L;
    for(int block=0;block<blocks;++block)
      sum+=host[static_cast<std::size_t>(q)*blocks+block];
    result[q]=static_cast<double>(sum);
  }
  return result;
}

void fail(CudaTransportedChartMorphologyTelemetry& telemetry,
          const std::string& error) {
  telemetry.error=error;
  telemetry.valid=false;
}

}  // namespace

TransportedChartMorphologyObservation
observe_transported_chart_morphology_cuda(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const TransportedChartMorphologyOptions& options,
    CudaTransportedChartMorphologyTelemetry* telemetry_out) {
  TransportedChartMorphologyObservation result;
  CudaTransportedChartMorphologyTelemetry telemetry;
  result.L=state.electric.L;
  const std::size_t count=result.L>0
      ?static_cast<std::size_t>(result.L)*result.L*result.L:0;
  bool valid=result.L>0&&state.magnetic_half.L==result.L
      &&state.electric.x.size()==count&&state.magnetic_half.x.size()==count
      &&state.constituents.size()==2&&state.charges.size()==2
      &&state.edges.empty()
      &&action_options.binding_law==ConnectedBindingLaw::DerivedCompactPair
      &&options.near_radius>0&&options.outer_radius>options.near_radius
      &&options.outer_radius<=result.L/2&&!options.modes.empty()
      &&options.wave_speed>0.0&&std::isfinite(options.dt)
      &&options.gate_tolerance>0.0
      &&std::isfinite(options.longitudinal_direction.x)
      &&std::isfinite(options.longitudinal_direction.y)
      &&std::isfinite(options.longitudinal_direction.z)
      &&std::isfinite(options.longitudinal_dead_band)
      &&options.longitudinal_dead_band>=0.0;
  for(const auto& mode:options.modes)
    valid=valid&&(mode.nx!=0||mode.ny!=0||mode.nz!=0)
        &&std::abs(mode.nx)<=result.L/2
        &&std::abs(mode.ny)<=result.L/2
        &&std::abs(mode.nz)<=result.L/2;
  if(!valid) {
    fail(telemetry,"invalid morphology input");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  const double direction_magnitude=options.longitudinal_direction.mag();
  result.longitudinal_partition_enabled=direction_magnitude>0.0;
  if(result.longitudinal_partition_enabled)
    result.longitudinal_direction=
        options.longitudinal_direction*(1.0/direction_magnitude);
  ConnectedMooreBlockState geometry;
  geometry.electric.L=result.L;
  geometry.magnetic_half.L=result.L;
  geometry.constituents=state.constituents;
  geometry.charges=state.charges;
  geometry.width=state.width;
  geometry.orientation_axis=state.orientation_axis;
  const auto preparation=prepare_finite_support_derived_compact_pair(
      geometry,action_options,options.support_half_width,
      options.poisson_tolerance,options.poisson_max_iterations,true);
  if(!preparation.valid||!preparation.compact_support
      ||!preparation.zero_boundary_crossing) {
    fail(telemetry,"fractional compact preparation failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  result.center=preparation.center;
  result.support_center=preparation.support_center;
  result.fractional_center_offset=preparation.fractional_center_offset;
  result.bound_gauss_residual=preparation.gauss_residual;
  const int cx=static_cast<int>(std::llround(result.support_center.x));
  const int cy=static_cast<int>(std::llround(result.support_center.y));
  const int cz=static_cast<int>(std::llround(result.support_center.z));

  DeviceTriplet actual_e,actual_b,bound_e,bound_b;
  double* partial=nullptr;
  const std::size_t bytes=count*sizeof(double);
  const int blocks=std::min<int>(kMaximumBlocks,
      static_cast<int>((count+kThreads-1)/kThreads));
  const auto allocation_start=std::chrono::steady_clock::now();
  const bool allocated=allocate(actual_e,bytes)&&allocate(actual_b,bytes)
      &&allocate(bound_e,bytes)&&allocate(bound_b,bytes)
      &&cudaMalloc(&partial,static_cast<std::size_t>(kSummaryQuantities)
          *blocks*sizeof(double))==cudaSuccess;
  telemetry.allocation_ms=milliseconds_since(allocation_start);
  const auto cleanup=[&]() {
    release(actual_e); release(actual_b); release(bound_e); release(bound_b);
    cudaFree(partial);
  };
  if(!allocated) {
    cleanup(); fail(telemetry,"CUDA morphology allocation failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  const auto upload_start=std::chrono::steady_clock::now();
  if(!upload(actual_e,state.electric,bytes)
      ||!upload(actual_b,state.magnetic_half,bytes)
      ||!upload(bound_e,preparation.state.electric,bytes)
      ||!upload(bound_b,preparation.state.magnetic_half,bytes)) {
    cleanup(); fail(telemetry,"CUDA morphology upload failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  telemetry.upload_ms=milliseconds_since(upload_start);
  telemetry.host_to_device_bytes=12*bytes;
  const double half_step_scale=-0.5*options.wave_speed*options.dt;
  const auto summary_start=std::chrono::steady_clock::now();
  summary_kernel<<<blocks,kThreads,kSummaryQuantities*kThreads*sizeof(double)>>>(
      actual_e,actual_b,bound_e,bound_b,count,result.L,cx,cy,cz,
      result.center.x,result.center.y,result.center.z,
      options.near_radius,options.outer_radius,half_step_scale,
      result.longitudinal_partition_enabled?1:0,
      result.longitudinal_direction.x,result.longitudinal_direction.y,
      result.longitudinal_direction.z,options.longitudinal_dead_band,partial);
  if(cudaGetLastError()!=cudaSuccess||cudaDeviceSynchronize()!=cudaSuccess) {
    cleanup(); fail(telemetry,"CUDA morphology summary failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  telemetry.kernel_ms+=milliseconds_since(summary_start);
  const auto summary=download_reduce(partial,kSummaryQuantities,blocks,telemetry);
  if(summary.size()!=kSummaryQuantities) {
    cleanup(); fail(telemetry,"CUDA morphology summary reduction failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return result;
  }
  result.actual_energy=summary[0];
  result.bound_energy=summary[1];
  result.residual_energy=summary[2];
  result.interference_energy=summary[3];
  result.near_residual_energy=summary[4];
  result.outer_residual_energy=summary[5];
  if(result.near_residual_energy>0.0) {
    result.near_residual_first_moment={summary[6],summary[7],summary[8]};
    result.near_residual_first_moment*=1.0/result.near_residual_energy;
    result.near_residual_second_moment=summary[9];
    result.near_residual_rms_radius=
        std::sqrt(summary[9]/result.near_residual_energy);
  }
  if(result.outer_residual_energy>0.0) {
    result.outer_residual_first_moment={summary[10],summary[11],summary[12]};
    result.outer_residual_first_moment*=1.0/result.outer_residual_energy;
    result.outer_residual_second_moment=summary[13];
    result.outer_residual_rms_radius=
        std::sqrt(summary[13]/result.outer_residual_energy);
  }
  if(result.longitudinal_partition_enabled) {
    result.near_longitudinal={summary[14],summary[15],summary[16]};
    result.outer_longitudinal={summary[17],summary[18],summary[19]};
    result.longitudinal_partition_residual=std::max(
        std::abs(result.near_residual_energy
            -result.near_longitudinal.total()),
        std::abs(result.outer_residual_energy
            -result.outer_longitudinal.total()));
  }
  result.coefficients.reserve(options.modes.size());
  for(const auto& mode:options.modes) {
    const auto mode_start=std::chrono::steady_clock::now();
    mode_kernel<<<blocks,kThreads,kModeQuantities*kThreads*sizeof(double)>>>(
        actual_e,actual_b,bound_e,bound_b,count,result.L,cx,cy,cz,
        options.near_radius,half_step_scale,mode.nx,mode.ny,mode.nz,partial);
    if(cudaGetLastError()!=cudaSuccess||cudaDeviceSynchronize()!=cudaSuccess) {
      cleanup(); fail(telemetry,"CUDA morphology mode failed");
      if(telemetry_out) *telemetry_out=telemetry;
      return result;
    }
    telemetry.kernel_ms+=milliseconds_since(mode_start);
    const auto values=download_reduce(partial,kModeQuantities,blocks,telemetry);
    if(values.size()!=kModeQuantities) {
      cleanup(); fail(telemetry,"CUDA morphology mode reduction failed");
      if(telemetry_out) *telemetry_out=telemetry;
      return result;
    }
    TransportedChartModeCoefficient coefficient;
    coefficient.mode=mode;
    coefficient.actual={values[0],values[1]};
    coefficient.bound={values[2],values[3]};
    coefficient.residual={values[4],values[5]};
    coefficient.interference={values[6],values[7]};
    coefficient.near_residual={values[8],values[9]};
    result.maximum_mode_reconstruction_residual=std::max(
        result.maximum_mode_reconstruction_residual,
        std::abs(coefficient.actual-coefficient.bound
            -coefficient.residual-coefficient.interference));
    result.coefficients.push_back(coefficient);
  }
  cleanup();
  result.energy_reconstruction_residual=result.actual_energy
      -result.bound_energy-result.residual_energy-result.interference_energy;
  const double scale=std::max({1.0,std::abs(result.actual_energy),
      std::abs(result.bound_energy),std::abs(result.residual_energy),
      std::abs(result.interference_energy)});
  result.valid=std::abs(result.energy_reconstruction_residual)
          <=options.gate_tolerance*scale
      &&result.maximum_mode_reconstruction_residual
          <=options.gate_tolerance*scale
      &&result.longitudinal_partition_residual
          <=options.gate_tolerance*scale
      &&result.bound_gauss_residual<=options.gate_tolerance;
  telemetry.valid=result.valid&&telemetry.error.empty();
  if(telemetry_out) *telemetry_out=telemetry;
  return result;
}

}  // namespace ftd::eft
