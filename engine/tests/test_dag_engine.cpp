#include <iostream>
#include <cmath>
#include "ftd/dag_engine.h"
#include "ftd/render_bridge.h"

using namespace ftd;

int asserts_passed = 0;
int asserts_total = 0;

void assert_test(bool condition, const std::string& name) {
    asserts_total++;
    if (condition) {
        asserts_passed++;
        std::cout << "[PASS] " << name << "\n";
    } else {
        std::cerr << "[FAIL] " << name << "\n";
    }
}

void test_dag_initialization() {
    std::cout << "--- test_dag_initialization ---\n";
    DagEngine eng(16);
    assert_test(eng.scale_level() == 0, "Scale level is 0");
    assert_test(std::string(eng.scale_name()) == "DagEngine", "Scale name is DagEngine");
    assert_test(eng.dag().size() == 16, "DAG size is 16");
    assert_test(eng.dag().depth() == 4, "DAG depth is 4");
    
    // Test the universally shared zero-leaf
    Voxel v = eng.dag().get_voxel(5, 5, 5);
    assert_test(v.state == 0, "Voxel state is 0");
    assert_test(v.flux.mag() == 0.0, "Voxel flux is zero");
}

void test_dag_set_and_get() {
    std::cout << "--- test_dag_set_and_get ---\n";
    DagEngine eng(16);
    
    // Inject flux
    eng.inject_flux(8, 8, 8, 1.0, 0.0, 0.0);
    
    Voxel v1 = eng.dag().get_voxel(8, 8, 8);
    Voxel v2 = eng.dag().get_voxel(0, 0, 0); // Unchanged zero void
    
    assert_test(v1.flux.x == 1.0, "Flux updated cleanly via copy-on-write path tracking");
    assert_test(v2.flux.x == 0.0, "Universal void voxel remains untouched");
}

void test_dag_tick_propagation() {
    std::cout << "--- test_dag_tick_propagation ---\n";
    DagEngine eng(16);
    
    // Inject flux at the center
    eng.inject_flux(8, 8, 8, 1.0, 0.0, 0.0);
    
    // Run one tick
    eng.tick();
    
    // Because laplacian distributes flux to neighbors, the central flux should decrease
    // and face neighbors should increase in wave_vel.
    Voxel center = eng.dag().get_voxel(8, 8, 8);
    Voxel right = eng.dag().get_voxel(9, 8, 8);
    
    // The laplacian at (8,8,8) is negative (curvature peak).
    // So wave_vel becomes negative, flux drops.
    assert_test(center.flux.x < 1.0, "Central flux dissipates correctly via DAG Laplacian");
    
    // The neighbors receive positive Laplacian
    assert_test(right.flux.x > 0.0, "Neighbor receives propagating wave via DAG Laplacian");
}

int main() {
    std::cout << "Running DagEngine Structural Parity Tests...\n";

    try {
        test_dag_initialization();
        test_dag_set_and_get();
        test_dag_tick_propagation();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    std::cout << "\nResults: " << asserts_passed << " / " << asserts_total << " passed.\n";
    return (asserts_passed == asserts_total) ? 0 : 1;
}
