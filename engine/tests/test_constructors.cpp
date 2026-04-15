/**
 * test_constructors — unit tests for ftd::ctor::*
 * Spec: docs/superpowers/specs/2026-04-15-ftd-constructors-design.md
 */

#include <algorithm>
#include <array>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "ftd/constructors.h"
#include "ftd/render_bridge.h"
#include "ftd/lattice.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_constructors");
    return ftd::test::finalize();
}
