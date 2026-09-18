#ifndef FTD_CUDA_CURL_INDEX_HELPERS_CUH
#define FTD_CUDA_CURL_INDEX_HELPERS_CUH

// Shared device-side periodic-index and staggered-lattice curl helpers for the
// EFT CUDA observer translation units.
//
// Consolidation (2026-09-18): cuda_paired_field_response.cu and
// cuda_state_only_support_ladder.cu each carried a verbatim copy of
// wrap_coordinate / lattice_index / component / curl_adjoint_component and of
// the integer-time edge reconstruction (spelled integer_magnetic_component in
// the first, integer_edge_component in the second); the index pair was a third
// verbatim copy in cuda_quadratic_coat_orbit_gather.cu (spelled wrap / index).
// Every formula below is lifted unchanged from those copies so the observers
// cannot drift apart.
//
// Pattern mirrors engine/cuda/cuda_index.cuh (ADR-0007): every helper is
// __device__ __forceinline__, so it inlines at the call site and is safe to
// include from any .cu translation unit under CUDA_SEPARABLE_COMPILATION.
//
// The field argument is templated because the device triplet differs per TU
// (a const double* view in the paired-response TU, an owning double* handle in
// the support-ladder TU). Instantiation reproduces the per-TU copies exactly.
//
// NOT covered here, deliberately:
//   - cuda_matched_field_pipeline.cu keeps its own device_index (a single-step
//     conditional wrap, not a modulo) and its fused three-component curl_edge /
//     curl_face_adjoint. Those are a different implementation, not a copy.
//   - cuda_momentum_transport_current.cu and cuda_transported_chart_morphology.cu
//     still carry local copies; adopting this header there is follow-up work.

#include <cstddef>

namespace ftd::eft::device_field {

// Periodic wrap of a single coordinate onto [0, L).
__device__ __forceinline__ int wrap_coordinate(int value,int L) {
  value%=L;
  return value<0?value+L:value;
}

// X-major flat index on the periodic L^3 lattice: i = ((x*L)+y)*L+z.
__device__ __forceinline__ std::size_t lattice_index(int x,int y,int z,int L) {
  return (static_cast<std::size_t>(wrap_coordinate(x,L))*L
      +wrap_coordinate(y,L))*L+wrap_coordinate(z,L);
}

// Axis component of a device triplet at a periodic lattice site.
template <typename Triplet>
__device__ __forceinline__ double component(
    Triplet field,int axis,int x,int y,int z,int L) {
  const auto index=lattice_index(x,y,z,L);
  return axis==0?field.x[index]:(axis==1?field.y[index]:field.z[index]);
}

// Backward-difference curl of an edge field (edges -> faces).
template <typename Triplet>
__device__ __forceinline__ double curl_component(
    Triplet edge,int axis,int x,int y,int z,int L) {
  const auto f=[&](int c,int xx,int yy,int zz) {
    return component(edge,c,xx,yy,zz,L);
  };
  if(axis==0)
    return f(2,x,y,z)-f(2,x,y-1,z)-f(1,x,y,z)+f(1,x,y,z-1);
  if(axis==1)
    return f(0,x,y,z)-f(0,x,y,z-1)-f(2,x,y,z)+f(2,x-1,y,z);
  return f(1,x,y,z)-f(1,x-1,y,z)-f(0,x,y,z)+f(0,x,y-1,z);
}

// Adjoint (forward-difference) curl of a face field (faces -> edges).
template <typename Triplet>
__device__ __forceinline__ double curl_adjoint_component(
    Triplet face,int axis,int x,int y,int z,int L) {
  const auto f=[&](int c,int xx,int yy,int zz) {
    return component(face,c,xx,yy,zz,L);
  };
  if(axis==0)
    return f(2,x,y+1,z)-f(2,x,y,z)-f(1,x,y,z+1)+f(1,x,y,z);
  if(axis==1)
    return f(0,x,y,z+1)-f(0,x,y,z)-f(2,x+1,y,z)+f(2,x,y,z);
  return f(1,x+1,y,z)-f(1,x,y,z)-f(0,x,y+1,z)+f(0,x,y,z);
}

// Half-step edge field advanced to integer time by the adjoint curl of the
// face field. `half_step_scale` carries the caller's sign convention.
template <typename ElectricTriplet,typename MagneticTriplet>
__device__ __forceinline__ double integer_edge_component(
    ElectricTriplet electric,MagneticTriplet magnetic,int axis,
    int x,int y,int z,int L,double half_step_scale) {
  return component(magnetic,axis,x,y,z,L)
      +half_step_scale*curl_adjoint_component(electric,axis,x,y,z,L);
}

}  // namespace ftd::eft::device_field

#endif  // FTD_CUDA_CURL_INDEX_HELPERS_CUH
