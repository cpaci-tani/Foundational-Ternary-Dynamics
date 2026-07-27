#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <iostream>

namespace {
int failures = 0;
void check(const char* name, bool ok) {
    std::cout << (ok ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!ok) ++failures;
}

bool same_erased_site(const ftd::Voxel& a, const ftd::Voxel& b) {
    return a.state == b.state && a.state == 0 &&
           a.particle_id == b.particle_id && a.pair_id == b.pair_id &&
           a.spin == b.spin && a.color == b.color &&
           a.flux.x == b.flux.x && a.flux.y == b.flux.y && a.flux.z == b.flux.z &&
           a.velocity.x == b.velocity.x && a.velocity.y == b.velocity.y &&
           a.velocity.z == b.velocity.z;
}

void configure_evaporation(ftd::RenderBridge& rb, int sign) {
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.evaporation = true;
    rb.toggles.langevin_seed = 404;
    rb.inject_particle(3, 3, 3, static_cast<std::int8_t>(sign), {0, 0, 0});
}
}  // namespace

int main() {
    ftd::RenderBridge plus(8), minus(8);
    configure_evaporation(plus, +1);
    configure_evaporation(minus, -1);
    check("evaporation preimages are distinct",
          plus.state_at(3, 3, 3) == +1 && minus.state_at(3, 3, 3) == -1);
    int accepted_tick = -1;
    for (int tick = 0; tick < 128; ++tick) {
        plus.tick();
        minus.tick();
        if (plus.state_at(3, 3, 3) == 0 && minus.state_at(3, 3, 3) == 0) {
            accepted_tick = tick;
            break;
        }
    }
    check("two distinct signed preimages reach one evaporation image",
          accepted_tick >= 0 && same_erased_site(
              plus.voxel_at(3, 3, 3), minus.voxel_at(3, 3, 3)));

    ftd::RenderBridge spin_a(8), spin_b(8);
    for (auto* rb : {&spin_a, &spin_b}) {
        rb->force_cpu();
        rb->toggles.disable_all();
        rb->toggles.movement = true;
        rb->inject_particle(3, 1, 1, +1, {0, 0, ftd::K_B});
        rb->inject_particle(4, 1, 1, -1, {0, 0, -ftd::K_B});
        rb->voxel_at(3, 1, 1).velocity = {1, 0, 0};
    }
    spin_a.voxel_at(3, 1, 1).spin = +1;
    spin_a.voxel_at(3, 1, 1).color = 1;
    spin_b.voxel_at(3, 1, 1).spin = -1;
    spin_b.voxel_at(3, 1, 1).color = 3;
    for (int tick = 0; tick < 4 && spin_a.state_at(3, 1, 1) != 0; ++tick) {
        spin_a.tick();
        spin_b.tick();
    }
    check("annihilation erases distinct spin/color preimages",
          same_erased_site(spin_a.voxel_at(3, 1, 1), spin_b.voxel_at(3, 1, 1)) &&
          same_erased_site(spin_a.voxel_at(4, 1, 1), spin_b.voxel_at(4, 1, 1)));
    return failures;
}
