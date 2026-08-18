// engine/tests/test_interop_particle_record_layout.cpp
#include "ftd/interop_particle_record.h"
#include "ftd/test_telemetry.h"

#include <cstddef>

int main() {
    ftd::test::init("test_interop_particle_record_layout");

    ftd::test::check("InteropParticleRecord is 32 bytes",
                      sizeof(ftd::InteropParticleRecord) == 32);
    ftd::test::check("x is at offset 0",
                      offsetof(ftd::InteropParticleRecord, x) == 0);
    ftd::test::check("y is at offset 4",
                      offsetof(ftd::InteropParticleRecord, y) == 4);
    ftd::test::check("z is at offset 8",
                      offsetof(ftd::InteropParticleRecord, z) == 8);
    ftd::test::check("size is at offset 12",
                      offsetof(ftd::InteropParticleRecord, size) == 12);
    ftd::test::check("r is at offset 16",
                      offsetof(ftd::InteropParticleRecord, r) == 16);
    ftd::test::check("g is at offset 20",
                      offsetof(ftd::InteropParticleRecord, g) == 20);
    ftd::test::check("b is at offset 24",
                      offsetof(ftd::InteropParticleRecord, b) == 24);
    ftd::test::check("reserved is at offset 28",
                      offsetof(ftd::InteropParticleRecord, reserved) == 28);
    ftd::test::check("InteropParticleHeader is 4 bytes",
                      sizeof(ftd::InteropParticleHeader) == 4);

    return ftd::test::finalize();
}
