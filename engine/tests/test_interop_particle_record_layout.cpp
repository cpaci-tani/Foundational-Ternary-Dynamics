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
    ftd::test::check("size is at offset 12",
                      offsetof(ftd::InteropParticleRecord, size) == 12);
    ftd::test::check("r is at offset 16",
                      offsetof(ftd::InteropParticleRecord, r) == 16);
    ftd::test::check("InteropParticleHeader is 4 bytes",
                      sizeof(ftd::InteropParticleHeader) == 4);

    return ftd::test::finalize();
}
