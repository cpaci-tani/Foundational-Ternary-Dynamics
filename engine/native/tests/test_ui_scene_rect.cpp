#include "native/scene_rect.h"
#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_ui_scene_rect");

    ftd::test::section("full-backbuffer fallback");
    const auto full = ftd::native::scene_rect_clamped_to({}, 1480, 860);
    ftd::test::check("empty rect becomes the backbuffer",
                     full.x == 0 && full.y == 0 && full.width == 1480 && full.height == 860);
    ftd::test::check("empty rect is not a scene hit target",
                     !ftd::native::scene_contains_client({}, 400, 10));

    ftd::test::section("panel strip is outside the scene");
    ftd::native::SceneRect scene{332, 0, 1148, 860};
    ftd::test::check("point in the Win32 strip is rejected",
                     !ftd::native::scene_contains_client(scene, 10, 40));
    ftd::test::check("point in the lattice view is accepted",
                     ftd::native::scene_contains_client(scene, 400, 40));
    int sx = 0, sy = 0;
    ftd::native::client_to_scene(scene, 400, 40, &sx, &sy);
    ftd::test::check("client-to-scene origin is the scene top-left",
                     sx == 68 && sy == 40);
    ftd::test::check("aspect uses the scene extent, not the window",
                     ftd::native::scene_aspect(scene) > 1.0f
                     && ftd::native::scene_aspect(scene)
                            < ftd::native::scene_aspect({0, 0, 1480, 860}) + 0.01f);

    ftd::test::section("input arbitration");
    ftd::test::check("orbit rejected in the Win32 strip",
                     !ftd::native::scene_accepts_pointer(scene, 10, 40, false));
    ftd::test::check("orbit accepted in the lattice view",
                     ftd::native::scene_accepts_pointer(scene, 400, 40, false));
    ftd::test::check("orbit rejected when the overlay wants the mouse",
                     !ftd::native::scene_accepts_pointer(scene, 400, 40, true));
    ftd::test::check("keys accepted with neither overlay nor edit focus",
                     ftd::native::scene_accepts_keyboard(false, false));
    ftd::test::check("keys rejected while typing in a Win32 edit",
                     !ftd::native::scene_accepts_keyboard(false, true));
    ftd::test::check("keys rejected when the overlay wants the keyboard",
                     !ftd::native::scene_accepts_keyboard(true, false));

    ftd::test::section("clamp rejects overflow");
    const auto clamped =
        ftd::native::scene_rect_clamped_to({100, 100, 5000, 5000}, 200, 200);
    ftd::test::check("overflowing scene is clipped to the framebuffer",
                     clamped.x == 100 && clamped.y == 100
                     && clamped.width == 100 && clamped.height == 100);

    return ftd::test::finalize();
}
