/**
 * Test: CSV Export Utility
 *
 * Verifies that the csv_export.h functions produce valid CSV files
 * with correct headers, dimensions, and data content.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include "ftd/render_bridge.h"
#include "ftd/csv_export.h"

int g_failures = 0;
int g_passes = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
        ++g_passes;
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++g_failures;
    }
}

// Count lines in a file (excluding empty trailing line)
int count_lines(const std::string& filename) {
    std::ifstream in(filename);
    int count = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) ++count;
    }
    return count;
}

// Count commas in first data line (to infer column count)
int count_columns(const std::string& filename) {
    std::ifstream in(filename);
    std::string header;
    std::getline(in, header);  // read header
    int commas = 0;
    for (char c : header) {
        if (c == ',') ++commas;
    }
    return commas + 1;  // columns = commas + 1
}

// Read first line (header) of a file
std::string read_header(const std::string& filename) {
    std::ifstream in(filename);
    std::string header;
    std::getline(in, header);
    return header;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: CSV Export Utility\n";
    std::cout << "================================================================\n\n";

    const int L = 8;  // Small lattice for fast tests
    ftd::RenderBridge bridge(L);

    // Inject some flux to have non-trivial data
    bridge.inject_flux(4, 4, 4, {1.0, 0.5, -0.3});
    bridge.inject_flux(2, 2, 2, {-0.2, 0.8, 0.1});
    bridge.tick();  // Let it propagate once

    // Temporary file names
    const std::string f_field  = "test_export_field.csv";
    const std::string f_slice  = "test_export_slice.csv";
    const std::string f_diag   = "test_export_diag.csv";
    const std::string f_line   = "test_export_line.csv";

    // ---- Test 1: Full field export ----
    std::cout << "  --- Full Field Export ---\n";
    {
        bool ok = ftd::csv::export_flux_field(bridge, f_field);
        check("export_flux_field returns true", ok);

        // Should have header + L^3 data rows
        int expected_lines = 1 + L * L * L;  // header + data
        int actual_lines = count_lines(f_field);
        check("Correct number of rows (header + L^3)",
              actual_lines == expected_lines);

        // Header should have 10 columns
        std::string header = read_header(f_field);
        check("Header contains 'x,y,z,Jx,Jy,Jz'",
              header.find("x,y,z,Jx,Jy,Jz") != std::string::npos);
        check("Header has 10 columns", count_columns(f_field) == 10);

        // Verify some data exists (non-zero flux)
        std::ifstream in(f_field);
        std::string line;
        std::getline(in, line);  // skip header
        bool found_nonzero = false;
        while (std::getline(in, line)) {
            // Check if any flux component is non-zero
            if (line.find("0.00000000e+00,0.00000000e+00,0.00000000e+00,0.00000000e+00") == std::string::npos) {
                // This line has some non-zero data
                found_nonzero = true;
                break;
            }
        }
        check("File contains non-zero flux data", found_nonzero);
    }

    // ---- Test 2: Density slice export ----
    std::cout << "\n  --- Density Slice Export ---\n";
    {
        // Slice at z = 4
        bool ok = ftd::csv::export_density_slice(bridge, f_slice, 'z', 4);
        check("export_density_slice returns true", ok);

        // Should have header + L^2 rows (x,y grid for fixed z)
        int expected_lines = 1 + L * L;
        int actual_lines = count_lines(f_slice);
        check("Correct number of rows (header + L^2)",
              actual_lines == expected_lines);

        // Header should have 7 columns
        std::string header = read_header(f_slice);
        check("Z-slice header starts with 'x,y'",
              header.substr(0, 3) == "x,y");
        check("Header has 7 columns", count_columns(f_slice) == 7);

        // Test other axes
        bool ok_x = ftd::csv::export_density_slice(bridge, f_slice, 'x', 4);
        check("X-slice export succeeds", ok_x);
        std::string hdr_x = read_header(f_slice);
        check("X-slice header starts with 'y,z'",
              hdr_x.substr(0, 3) == "y,z");

        bool ok_y = ftd::csv::export_density_slice(bridge, f_slice, 'y', 4);
        check("Y-slice export succeeds", ok_y);
        std::string hdr_y = read_header(f_slice);
        check("Y-slice header starts with 'x,z'",
              hdr_y.substr(0, 3) == "x,z");

        // Invalid axis should return false
        bool bad = ftd::csv::export_density_slice(bridge, f_slice, 'q', 4);
        check("Invalid axis returns false", !bad);

        // Out-of-range index should return false
        bool oor = ftd::csv::export_density_slice(bridge, f_slice, 'z', 999);
        check("Out-of-range index returns false", !oor);
    }

    // ---- Test 3: Diagnostics timeseries ----
    std::cout << "\n  --- Diagnostics Timeseries ---\n";
    {
        // Remove if exists
        std::remove(f_diag.c_str());

        // First call should create header + 1 row
        bool ok1 = ftd::csv::export_diagnostics_row(bridge, f_diag);
        check("First diagnostics_row returns true", ok1);
        check("First call creates 2 lines (header + row)",
              count_lines(f_diag) == 2);

        // Header check
        std::string header = read_header(f_diag);
        check("Diagnostics header contains 'tick'",
              header.find("tick") != std::string::npos);
        check("Diagnostics header contains 'manifested'",
              header.find("manifested") != std::string::npos);
        check("Diagnostics has 14 columns", count_columns(f_diag) == 14);

        // Run a tick and append another row
        bridge.tick();
        bool ok2 = ftd::csv::export_diagnostics_row(bridge, f_diag);
        check("Second diagnostics_row returns true", ok2);
        check("Second call appends (now 3 lines)",
              count_lines(f_diag) == 3);

        // Run more ticks and append
        for (int i = 0; i < 5; ++i) {
            bridge.tick();
            ftd::csv::export_diagnostics_row(bridge, f_diag);
        }
        check("After 7 total calls: 8 lines (header + 7 rows)",
              count_lines(f_diag) == 8);
    }

    // ---- Test 4: Line profile export ----
    std::cout << "\n  --- Line Profile Export ---\n";
    {
        bool ok = ftd::csv::export_line_profile(bridge, f_line, 'z', 4, 4);
        check("export_line_profile returns true", ok);

        // Should have header + L rows
        int expected_lines = 1 + L;
        int actual_lines = count_lines(f_line);
        check("Correct number of rows (header + L)",
              actual_lines == expected_lines);

        std::string header = read_header(f_line);
        check("Line profile header starts with 'position'",
              header.find("position") == 0);
        check("Line profile has 6 columns", count_columns(f_line) == 6);

        // Test other axes
        bool ok_x = ftd::csv::export_line_profile(bridge, f_line, 'x', 4, 4);
        check("X-axis line profile succeeds", ok_x);
        bool ok_y = ftd::csv::export_line_profile(bridge, f_line, 'y', 4, 4);
        check("Y-axis line profile succeeds", ok_y);
    }

    // ---- Cleanup ----
    std::remove(f_field.c_str());
    std::remove(f_slice.c_str());
    std::remove(f_diag.c_str());
    std::remove(f_line.c_str());

    // ---- Summary ----
    std::cout << "\n================================================================\n";
    std::cout << "  CSV Export Tests: " << g_passes << " passed, "
              << g_failures << " failed\n";
    std::cout << "================================================================\n";

    return g_failures;
}
