#pragma once

#include <vector>

namespace ftd::native {

// A W×H scalar field slice (row-major, i + j*w) plus its value range, computed
// adapter-side from the dense voxel grid for the Flux-slice panel. `ok` is false
// until a slice has been produced.
struct FieldSliceResult {
    int w = 0;
    int h = 0;
    float mn = 0.0f;
    float mx = 0.0f;
    std::vector<float> data;   // size w*h, row-major
    bool ok = false;
};

// The Flux-slice panel shows three orthogonal centre-plane slices of one field.
enum FieldSlicePlane { SLICE_YZ = 0, SLICE_XZ = 1, SLICE_XY = 2, SLICE_PLANES = 3 };

}  // namespace ftd::native
