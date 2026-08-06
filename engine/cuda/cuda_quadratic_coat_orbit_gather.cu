#include "ftd/eft/cuda_quadratic_coat_orbit_gather.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <utility>

namespace ftd::eft {
namespace {

struct DeviceVec3 {
  double x,y,z;
};

struct DeviceCurrentEntry {
  std::size_t index;
  int axis;
  double value;
};

struct DeviceOrbitSegment {
  DeviceVec3 start;
  DeviceVec3 end;
  DeviceVec3 velocity;
  std::size_t entry_offset;
  std::size_t entry_count;
  std::size_t break_offset;
  std::size_t break_count;
  int charge;
};

struct DeviceOrbitOutput {
  DeviceVec3 electric_average;
  DeviceVec3 magnetic_average;
  double current_work;
  int quadrature_pieces;
};

__device__ __constant__ double kGaussNodes[8]={
    -0.960289856497536231683560868569,
    -0.796666477413626739591553936476,
    -0.525532409916328985817739049189,
    -0.183434642495649804939476142360,
     0.183434642495649804939476142360,
     0.525532409916328985817739049189,
     0.796666477413626739591553936476,
     0.960289856497536231683560868569};

__device__ __constant__ double kGaussWeights[8]={
    0.101228536290376259152531354310,
    0.222381034453374470544355994426,
    0.313706645877887287337962201987,
    0.362683783378361982965150449277,
    0.362683783378361982965150449277,
    0.313706645877887287337962201987,
    0.222381034453374470544355994426,
    0.101228536290376259152531354310};

double milliseconds_since(const std::chrono::steady_clock::time_point& start) {
  return std::chrono::duration<double,std::milli>(
      std::chrono::steady_clock::now()-start).count();
}

double host_component(const Vec3& value,int axis) {
  return axis==0?value.x:(axis==1?value.y:value.z);
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x)&&std::isfinite(value.y)
      &&std::isfinite(value.z);
}

std::vector<double> half_integer_breaks(const Vec3& start,const Vec3& end) {
  std::vector<double> breaks{0.0,1.0};
  for(int axis=0;axis<3;++axis) {
    const double p0=host_component(start,axis);
    const double delta=host_component(end,axis)-p0;
    if(delta==0.0) continue;
    const double lower=std::min(p0,p0+delta);
    const double upper=std::max(p0,p0+delta);
    const int first=static_cast<int>(std::floor(lower))-2;
    const int last=static_cast<int>(std::ceil(upper))+2;
    for(int knot=first;knot<=last;++knot) {
      const double tau=(static_cast<double>(knot)+0.5-p0)/delta;
      if(tau>0.0&&tau<1.0) breaks.push_back(tau);
    }
  }
  std::sort(breaks.begin(),breaks.end());
  breaks.erase(std::unique(breaks.begin(),breaks.end(),
      [](double a,double b) {
        return std::abs(a-b)<=32.0*std::numeric_limits<double>::epsilon();
      }),breaks.end());
  return breaks;
}

__device__ double b1(double u) {
  const double radius=fabs(u);
  return radius<1.0?1.0-radius:0.0;
}

__device__ double b2(double u) {
  const double radius=fabs(u);
  if(radius<=0.5) return 0.75-radius*radius;
  if(radius<1.5) {
    const double tail=1.5-radius;
    return 0.5*tail*tail;
  }
  return 0.0;
}

__device__ int wrap(int value,int L) {
  value%=L;
  return value<0?value+L:value;
}

__device__ std::size_t index(int x,int y,int z,int L) {
  return (static_cast<std::size_t>(wrap(x,L))*L+wrap(y,L))*L+wrap(z,L);
}

__device__ double field_coefficient(
    CudaMatchedFieldDeviceView field,int axis,std::size_t item) {
  return axis==0?field.x[item]:(axis==1?field.y[item]:field.z[item]);
}

__device__ double face_component_at(
    CudaMatchedFieldDeviceView field,int axis,DeviceVec3 position) {
  const int lower_x=static_cast<int>(floor(position.x))-2;
  const int lower_y=static_cast<int>(floor(position.y))-2;
  const int lower_z=static_cast<int>(floor(position.z))-2;
  double result=0.0;
  for(int x=lower_x;x<=lower_x+4;++x) {
    const double wx=axis==0?b1(position.x-x-0.5):b2(position.x-x);
    if(wx==0.0) continue;
    for(int y=lower_y;y<=lower_y+4;++y) {
      const double wy=axis==1?b1(position.y-y-0.5):b2(position.y-y);
      if(wy==0.0) continue;
      for(int z=lower_z;z<=lower_z+4;++z) {
        const double wz=axis==2?b1(position.z-z-0.5):b2(position.z-z);
        if(wz==0.0) continue;
        result+=wx*wy*wz*field_coefficient(
            field,axis,index(x,y,z,field.L));
      }
    }
  }
  return result;
}

__device__ double edge_component_at(
    CudaMatchedFieldDeviceView field,int axis,DeviceVec3 position) {
  const int lower_x=static_cast<int>(floor(position.x))-2;
  const int lower_y=static_cast<int>(floor(position.y))-2;
  const int lower_z=static_cast<int>(floor(position.z))-2;
  double result=0.0;
  for(int x=lower_x;x<=lower_x+4;++x) {
    const double wx=axis==0?b2(position.x-x):b1(position.x-x-0.5);
    if(wx==0.0) continue;
    for(int y=lower_y;y<=lower_y+4;++y) {
      const double wy=axis==1?b2(position.y-y):b1(position.y-y-0.5);
      if(wy==0.0) continue;
      for(int z=lower_z;z<=lower_z+4;++z) {
        const double wz=axis==2?b2(position.z-z):b1(position.z-z-0.5);
        if(wz==0.0) continue;
        result+=wx*wy*wz*field_coefficient(
            field,axis,index(x,y,z,field.L));
      }
    }
  }
  return result;
}

__device__ double sparse_coefficient(
    const DeviceCurrentEntry* entries,std::size_t entry_count,
    int axis,std::size_t item) {
  double result=0.0;
  for(std::size_t entry=0;entry<entry_count;++entry)
    if(entries[entry].axis==axis&&entries[entry].index==item)
      result+=entries[entry].value;
  return result;
}

__device__ double sparse_component_at(
    const DeviceCurrentEntry* entries,std::size_t entry_count,
    int L,int axis,DeviceVec3 position) {
  const int lower_x=static_cast<int>(floor(position.x))-2;
  const int lower_y=static_cast<int>(floor(position.y))-2;
  const int lower_z=static_cast<int>(floor(position.z))-2;
  double result=0.0;
  for(int x=lower_x;x<=lower_x+4;++x) {
    const double wx=axis==0?b1(position.x-x-0.5):b2(position.x-x);
    if(wx==0.0) continue;
    for(int y=lower_y;y<=lower_y+4;++y) {
      const double wy=axis==1?b1(position.y-y-0.5):b2(position.y-y);
      if(wy==0.0) continue;
      for(int z=lower_z;z<=lower_z+4;++z) {
        const double wz=axis==2?b1(position.z-z-0.5):b2(position.z-z);
        if(wz==0.0) continue;
        result+=wx*wy*wz*sparse_coefficient(
            entries,entry_count,axis,index(x,y,z,L));
      }
    }
  }
  return result;
}

__global__ void orbit_gather_kernel(
    const DeviceOrbitSegment* segments,std::size_t segment_count,
    const DeviceCurrentEntry* entries,std::size_t entry_count,
    const double* breaks,
    CudaMatchedFieldDeviceView fixed_electric,
    CudaMatchedFieldDeviceView electric_pre_current,
    double current_scale,CudaMatchedFieldDeviceView magnetic,
    DeviceOrbitOutput* output) {
  const std::size_t item=static_cast<std::size_t>(blockIdx.x)*blockDim.x
      +threadIdx.x;
  if(item>=segment_count) return;
  const DeviceOrbitSegment segment=segments[item];
  const DeviceVec3 displacement{
      segment.end.x-segment.start.x,
      segment.end.y-segment.start.y,
      segment.end.z-segment.start.z};
  DeviceVec3 electric_average{0.0,0.0,0.0};
  DeviceVec3 magnetic_average{0.0,0.0,0.0};
  for(std::size_t piece=1;piece<segment.break_count;++piece) {
    const double lower=breaks[segment.break_offset+piece-1];
    const double upper=breaks[segment.break_offset+piece];
    const double midpoint=0.5*(lower+upper);
    const double half_width=0.5*(upper-lower);
    for(int sample=0;sample<8;++sample) {
      const double tau=midpoint+half_width*kGaussNodes[sample];
      const DeviceVec3 position{
          segment.start.x+displacement.x*tau,
          segment.start.y+displacement.y*tau,
          segment.start.z+displacement.z*tau};
      const double weight=half_width*kGaussWeights[sample];
      for(int axis=0;axis<3;++axis) {
        const double electric=0.5*(
            face_component_at(fixed_electric,axis,position)
            +face_component_at(electric_pre_current,axis,position)
            +current_scale*sparse_component_at(
                entries,entry_count,fixed_electric.L,axis,position));
        const double magnetic_value=edge_component_at(magnetic,axis,position);
        if(axis==0) {
          electric_average.x+=weight*electric;
          magnetic_average.x+=weight*magnetic_value;
        } else if(axis==1) {
          electric_average.y+=weight*electric;
          magnetic_average.y+=weight*magnetic_value;
        } else {
          electric_average.z+=weight*electric;
          magnetic_average.z+=weight*magnetic_value;
        }
      }
    }
  }
  double current_work=0.0;
  for(std::size_t local=0;local<segment.entry_count;++local) {
    const DeviceCurrentEntry entry=entries[segment.entry_offset+local];
    const double midpoint=0.5*(
        field_coefficient(fixed_electric,entry.axis,entry.index)
        +field_coefficient(electric_pre_current,entry.axis,entry.index)
        +current_scale*sparse_coefficient(
            entries,entry_count,entry.axis,entry.index));
    current_work+=entry.value*midpoint;
  }
  output[item]={electric_average,magnetic_average,current_work,
                static_cast<int>(segment.break_count)-1};
}

void fail(CudaQuadraticCoatOrbitGatherTelemetry& telemetry,
          const std::string& message) {
  telemetry.valid=false;
  telemetry.error=message;
}

}  // namespace

std::vector<QuadraticCoatOrbitGatherResult>
evaluate_quadratic_coat_orbit_gather_sparse_midpoint_batch_cuda_resident(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    const CudaMatchedFieldDeviceView& fixed_electric,
    const CudaMatchedFieldDeviceView& electric_pre_current,
    double current_scale,
    const CudaMatchedFieldDeviceView& magnetic,
    const std::vector<Vec3>& discrete_gradient_velocities,
    double temporal_scale,double beta,double polarity_scale,
    CudaQuadraticCoatOrbitGatherTelemetry* telemetry_out) {
  CudaQuadraticCoatOrbitGatherTelemetry telemetry;
  std::vector<QuadraticCoatOrbitGatherResult> results;
  const int L=fixed_electric.L;
  if(segments.empty()||segments.size()!=discrete_gradient_velocities.size()
      ||!fixed_electric.valid()||!electric_pre_current.valid()
      ||!magnetic.valid()||electric_pre_current.L!=L||magnetic.L!=L
      ||!std::isfinite(current_scale)||!(temporal_scale>0.0)
      ||!std::isfinite(temporal_scale)||!std::isfinite(beta)
      ||!(polarity_scale>0.0)||!std::isfinite(polarity_scale)) {
    fail(telemetry,"invalid resident orbit-gather input");
    if(telemetry_out) *telemetry_out=telemetry;
    return results;
  }

  std::vector<DeviceOrbitSegment> packed_segments;
  std::vector<DeviceCurrentEntry> packed_entries;
  std::vector<double> packed_breaks;
  packed_segments.reserve(segments.size());
  for(std::size_t item=0;item<segments.size();++item) {
    const auto& segment=segments[item];
    const auto& velocity=discrete_gradient_velocities[item];
    if(!segment.valid||segment.L!=L||segment.dense_materialized
        ||(segment.charge!=-1&&segment.charge!=1)||!finite(velocity)) {
      fail(telemetry,"invalid sparse segment");
      if(telemetry_out) *telemetry_out=telemetry;
      return results;
    }
    const std::size_t entry_offset=packed_entries.size();
    for(const auto& entry:segment.sparse_current) {
      if(entry.axis<0||entry.axis>2||!std::isfinite(entry.value)) {
        fail(telemetry,"invalid sparse current entry");
        if(telemetry_out) *telemetry_out=telemetry;
        return results;
      }
      packed_entries.push_back({static_cast<std::size_t>(segment.index(
          entry.face.x,entry.face.y,entry.face.z)),entry.axis,entry.value});
    }
    const auto segment_breaks=half_integer_breaks(
        segment.start_effective_position,segment.end_effective_position);
    const std::size_t break_offset=packed_breaks.size();
    packed_breaks.insert(
        packed_breaks.end(),segment_breaks.begin(),segment_breaks.end());
    packed_segments.push_back({
        {segment.start_effective_position.x,
         segment.start_effective_position.y,
         segment.start_effective_position.z},
        {segment.end_effective_position.x,
         segment.end_effective_position.y,
         segment.end_effective_position.z},
        {velocity.x,velocity.y,velocity.z},
        entry_offset,segment.sparse_current.size(),
        break_offset,segment_breaks.size(),segment.charge});
  }

  DeviceOrbitSegment* device_segments=nullptr;
  DeviceCurrentEntry* device_entries=nullptr;
  double* device_breaks=nullptr;
  DeviceOrbitOutput* device_output=nullptr;
  const auto cleanup=[&]() {
    cudaFree(device_segments); cudaFree(device_entries);
    cudaFree(device_breaks); cudaFree(device_output);
  };
  const auto allocate=[](auto** pointer,std::size_t bytes) {
    return bytes==0||cudaMalloc(pointer,bytes)==cudaSuccess;
  };
  if(!allocate(&device_segments,packed_segments.size()*sizeof(DeviceOrbitSegment))
      ||!allocate(&device_entries,packed_entries.size()*sizeof(DeviceCurrentEntry))
      ||!allocate(&device_breaks,packed_breaks.size()*sizeof(double))
      ||!allocate(&device_output,packed_segments.size()*sizeof(DeviceOrbitOutput))) {
    cleanup(); fail(telemetry,"resident orbit-gather allocation failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return results;
  }

  const auto upload_start=std::chrono::steady_clock::now();
  const bool upload_ok=
      cudaMemcpy(device_segments,packed_segments.data(),
          packed_segments.size()*sizeof(DeviceOrbitSegment),
          cudaMemcpyHostToDevice)==cudaSuccess
      &&(packed_entries.empty()||cudaMemcpy(device_entries,packed_entries.data(),
          packed_entries.size()*sizeof(DeviceCurrentEntry),
          cudaMemcpyHostToDevice)==cudaSuccess)
      &&cudaMemcpy(device_breaks,packed_breaks.data(),
          packed_breaks.size()*sizeof(double),
          cudaMemcpyHostToDevice)==cudaSuccess;
  telemetry.upload_ms=milliseconds_since(upload_start);
  telemetry.host_to_device_bytes=packed_segments.size()*sizeof(DeviceOrbitSegment)
      +packed_entries.size()*sizeof(DeviceCurrentEntry)
      +packed_breaks.size()*sizeof(double);
  if(!upload_ok) {
    cleanup(); fail(telemetry,"resident orbit-gather upload failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return results;
  }

  const auto kernel_start=std::chrono::steady_clock::now();
  constexpr int threads=32;
  const int blocks=static_cast<int>((segments.size()+threads-1)/threads);
  orbit_gather_kernel<<<blocks,threads>>>(
      device_segments,segments.size(),device_entries,packed_entries.size(),
      device_breaks,fixed_electric,electric_pre_current,current_scale,
      magnetic,device_output);
  if(cudaGetLastError()!=cudaSuccess||cudaDeviceSynchronize()!=cudaSuccess) {
    cleanup(); fail(telemetry,"resident orbit-gather kernel failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return results;
  }
  telemetry.kernel_ms=milliseconds_since(kernel_start);

  std::vector<DeviceOrbitOutput> host_output(segments.size());
  const auto download_start=std::chrono::steady_clock::now();
  const bool download_ok=cudaMemcpy(host_output.data(),device_output,
      host_output.size()*sizeof(DeviceOrbitOutput),cudaMemcpyDeviceToHost)
      ==cudaSuccess;
  telemetry.download_ms=milliseconds_since(download_start);
  telemetry.device_to_host_bytes=host_output.size()*sizeof(DeviceOrbitOutput);
  cleanup();
  if(!download_ok) {
    fail(telemetry,"resident orbit-gather download failed");
    if(telemetry_out) *telemetry_out=telemetry;
    return results;
  }

  results.reserve(segments.size());
  for(std::size_t item=0;item<segments.size();++item) {
    const auto& segment=segments[item];
    const auto& raw=host_output[item];
    QuadraticCoatOrbitGatherResult result;
    result.L=L;
    result.charge=segment.charge;
    result.quadrature_pieces=raw.quadrature_pieces;
    result.start_effective_position=segment.start_effective_position;
    result.end_effective_position=segment.end_effective_position;
    result.displacement=result.end_effective_position
        -result.start_effective_position;
    result.discrete_gradient_velocity=discrete_gradient_velocities[item];
    result.temporal_scale=temporal_scale;
    result.beta=beta;
    const Vec3 electric_average{
        raw.electric_average.x,raw.electric_average.y,raw.electric_average.z};
    result.magnetic_average={
        raw.magnetic_average.x,raw.magnetic_average.y,raw.magnetic_average.z};
    const double effective_charge=polarity_scale*segment.charge;
    result.electric_force=electric_average*effective_charge;
    result.current_work=polarity_scale*raw.current_work;
    result.electric_work=result.displacement.dot(result.electric_force);
    result.electric_adjoint_residual=std::abs(
        result.current_work-result.electric_work);
    result.magnetic_impulse=Vec3::cross(
        result.discrete_gradient_velocity,result.magnetic_average)
        *(temporal_scale*beta*effective_charge);
    result.magnetic_work_residual=std::abs(
        result.discrete_gradient_velocity.dot(result.magnetic_impulse));
    const Vec3 kinematic=result.displacement
        -result.discrete_gradient_velocity*temporal_scale;
    result.kinematic_residual=std::max({std::abs(kinematic.x),
        std::abs(kinematic.y),std::abs(kinematic.z)});
    result.causal_excess=std::max(0.0,
        result.discrete_gradient_velocity.mag()-C_SPEED);
    result.valid=finite(result.electric_force)
        &&finite(result.magnetic_average)&&finite(result.magnetic_impulse)
        &&std::isfinite(result.current_work)
        &&result.electric_adjoint_residual<=5e-13
        &&result.magnetic_work_residual<=5e-13
        &&result.kinematic_residual<=5e-13
        &&result.causal_excess<=5e-13;
    results.push_back(result);
  }
  telemetry.valid=results.size()==segments.size()
      &&std::all_of(results.begin(),results.end(),
          [](const auto& result) { return result.valid; });
  if(!telemetry.valid&&telemetry.error.empty())
    telemetry.error="resident orbit-gather result invalid";
  if(telemetry_out) *telemetry_out=telemetry;
  return results;
}

}  // namespace ftd::eft
