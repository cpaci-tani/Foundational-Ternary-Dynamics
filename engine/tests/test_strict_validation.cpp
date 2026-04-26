/**
 * @file test_strict_validation.cpp
 * @brief Verify ARCH-3 toggle-validator strictness contract.
 *
 * Tests three behaviors:
 *   SV1. Default permissive mode: invalid combination prints to stderr but
 *        tick() returns; the warning is emitted ONCE per unique error string
 *        (not on every tick).
 *   SV2. strict_validation=true: tick() throws std::logic_error with the
 *        error message embedded.
 *   SV3. Recovery: if the toggles are corrected mid-run, the dedup memo
 *        resets so a new violation later is reported again.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "ftd/render_bridge.h"

namespace {

struct StderrCapture {
    std::stringstream sink;
    std::streambuf*   old;
    StderrCapture() : old(std::cerr.rdbuf(sink.rdbuf())) {}
    ~StderrCapture() { std::cerr.rdbuf(old); }
    std::string str() const { return sink.str(); }
};

int count_substring(const std::string& haystack, const std::string& needle) {
    int count = 0;
    size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  ARCH-3 Strict-Validation Contract Test\n");
    std::printf("================================================================\n\n");

    int failures = 0;

    // ── SV1: permissive mode dedups warnings to ONE per unique string ──
    {
        std::printf("  SV1 permissive mode (dedup):\n");
        ftd::RenderBridge rb(8);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        // Inject an invalid combination: lorentz_force without forces
        rb.toggles.lorentz_force = true;
        rb.toggles.forces        = false;
        rb.force_cpu();

        StderrCapture cap;
        for (int i = 0; i < 5; ++i) rb.tick();
        const std::string out = cap.str();
        const int n = count_substring(out, "lorentz_force requires forces");
        std::printf("    5 ticks with invalid combination: warning printed %d times (expect 1)\n", n);
        if (n != 1) { std::printf("    FAIL\n"); ++failures; }
        else        { std::printf("    PASS\n"); }
    }

    // ── SV2: strict mode throws std::logic_error ──
    {
        std::printf("\n  SV2 strict mode (throws):\n");
        ftd::RenderBridge rb(8);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.lorentz_force = true;   // invalid (forces=false)
        rb.toggles.forces        = false;
        rb.toggles.strict_validation = true;
        rb.force_cpu();

        // Suppress stderr that strict mode would also produce upstream of throw
        StderrCapture cap;
        bool threw = false;
        std::string what;
        try {
            rb.tick();
        } catch (const std::logic_error& e) {
            threw = true;
            what = e.what();
        }
        std::printf("    threw std::logic_error: %s\n", threw ? "YES" : "NO");
        std::printf("    what(): %s\n", what.c_str());
        const bool has_msg = what.find("lorentz_force requires forces") != std::string::npos;
        if (!threw || !has_msg) { std::printf("    FAIL\n"); ++failures; }
        else                    { std::printf("    PASS\n"); }
    }

    // ── SV3: recovery — fix toggles mid-run, then violate again ──
    {
        std::printf("\n  SV3 recovery (memo resets when toggles fixed):\n");
        ftd::RenderBridge rb(8);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.lorentz_force = true;
        rb.toggles.forces        = false;
        rb.force_cpu();

        StderrCapture cap;
        rb.tick();                          // emits warning #1
        rb.tick();                          // dedup'd
        rb.toggles.forces = true;           // fix
        rb.tick();                          // valid → memo cleared
        rb.toggles.forces = false;          // re-violate
        rb.tick();                          // emits warning #2
        const std::string out = cap.str();
        const int n = count_substring(out, "lorentz_force requires forces");
        std::printf("    after fix-and-re-violate: %d warnings (expect 2)\n", n);
        if (n != 2) { std::printf("    FAIL\n"); ++failures; }
        else        { std::printf("    PASS\n"); }
    }

    std::printf("\n================================================================\n");
    std::printf("  RESULT: %d failures\n", failures);
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
