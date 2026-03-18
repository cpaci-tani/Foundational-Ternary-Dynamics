#include "ftd/lattice.h"

namespace ftd {

Lattice::Lattice(int size)
    : size_(size), total_(static_cast<int64_t>(size) * size * size) {}

}  // namespace ftd
