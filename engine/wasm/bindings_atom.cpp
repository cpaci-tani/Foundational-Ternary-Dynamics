/**
 * @file bindings_atom.cpp
 * @brief Embind bindings for AtomEngine (Scale 2).
 *
 * Extracted from ftd_wasm.cpp as part of W1-W3. Contains:
 *   - ae_toggle_map (pointer-to-member AtomToggles lookup)
 *   - cpk_color (CPK element color table helper)
 *   - Data extraction (get_ae_atom_data, get_ae_diagnostics)
 *   - Injection helpers (ae_add_atom, ae_add_locked_atom, ae_create_bond)
 *   - Controls (dt, softening, damping, bonding, clear)
 *   - Force diagnostics
 *   - The AtomEngine class_<> binding
 */

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <cmath>
#include <string>
#include <unordered_map>
#include "ftd/atom_engine.h"
#include "ftd/constants.h"

using namespace emscripten;

// CPK element colors: H=white, He=cyan, C=dark gray, N=blue, O=red, etc.
static void cpk_color(int Z, float& r, float& g, float& b) {
    switch (Z) {
        case  1: r=1.00f; g=1.00f; b=1.00f; break;  // H  — white
        case  2: r=0.85f; g=1.00f; b=1.00f; break;  // He — cyan
        case  3: r=0.80f; g=0.50f; b=1.00f; break;  // Li — violet
        case  4: r=0.76f; g=1.00f; b=0.00f; break;  // Be — dark yellow-green
        case  5: r=1.00f; g=0.71f; b=0.71f; break;  // B  — salmon
        case  6: r=0.56f; g=0.56f; b=0.56f; break;  // C  — dark gray
        case  7: r=0.19f; g=0.31f; b=0.97f; break;  // N  — blue
        case  8: r=1.00f; g=0.05f; b=0.05f; break;  // O  — red
        case  9: r=0.56f; g=0.88f; b=0.31f; break;  // F  — green
        case 10: r=0.70f; g=0.89f; b=0.96f; break;  // Ne — light cyan
        case 11: r=0.67f; g=0.36f; b=0.95f; break;  // Na — purple
        case 12: r=0.54f; g=1.00f; b=0.00f; break;  // Mg — green
        case 13: r=0.75f; g=0.65f; b=0.65f; break;  // Al — gray
        case 14: r=0.94f; g=0.78f; b=0.63f; break;  // Si — tan
        case 15: r=1.00f; g=0.50f; b=0.00f; break;  // P  — orange
        case 16: r=1.00f; g=1.00f; b=0.19f; break;  // S  — yellow
        case 17: r=0.12f; g=0.94f; b=0.12f; break;  // Cl — green
        case 18: r=0.50f; g=0.82f; b=0.89f; break;  // Ar — light blue
        default: r=0.70f; g=0.70f; b=0.70f; break;  // Unknown — light gray
    }
}

// ── AE Atom Data Extraction ─────────────────────────────────────────
static val get_ae_atom_data(ftd::AtomEngine& ae) {
    const auto& atoms = ae.atoms();
    int count = static_cast<int>(atoms.size());

    val positions  = val::global("Float32Array").new_(count * 3);
    val colors     = val::global("Float32Array").new_(count * 3);
    val sizes      = val::global("Float32Array").new_(count);
    val atomicNums = val::global("Int32Array").new_(count);
    val charges    = val::global("Int32Array").new_(count);
    val ids        = val::global("Int32Array").new_(count);

    // Count bonds for bond index array
    int total_bonds = 0;
    for (const auto& a : atoms) {
        for (const auto& b : a.bonds) {
            if (b.partner_id > a.id) total_bonds++;  // avoid double-counting
        }
    }
    val bonds = val::global("Int32Array").new_(total_bonds * 2);

    for (int i = 0; i < count; ++i) {
        const auto& a = atoms[i];

        positions.set(i * 3,     static_cast<float>(a.position.x));
        positions.set(i * 3 + 1, static_cast<float>(a.position.y));
        positions.set(i * 3 + 2, static_cast<float>(a.position.z));

        float cr, cg, cb;
        cpk_color(a.Z, cr, cg, cb);
        colors.set(i * 3,     cr);
        colors.set(i * 3 + 1, cg);
        colors.set(i * 3 + 2, cb);

        // Size proportional to atomic radius (log scale for visibility)
        float s = 4.0f + 3.0f * static_cast<float>(std::log10(a.radius + 1.0));
        if (s > 15.0f) s = 15.0f;
        sizes.set(i, s);

        atomicNums.set(i, a.Z);
        charges.set(i, a.charge);
        ids.set(i, a.id);
    }

    // Fill bond index pairs
    int bi = 0;
    for (const auto& a : atoms) {
        for (const auto& b : a.bonds) {
            if (b.partner_id > a.id) {
                bonds.set(bi * 2,     a.id);
                bonds.set(bi * 2 + 1, b.partner_id);
                bi++;
            }
        }
    }

    val result = val::object();
    result.set("positions",  positions);
    result.set("colors",     colors);
    result.set("sizes",      sizes);
    result.set("atomicNums", atomicNums);
    result.set("charges",    charges);
    result.set("ids",        ids);
    result.set("bonds",      bonds);
    result.set("bondCount",  total_bonds);
    result.set("count",      count);
    return result;
}

// ── AE Diagnostics ─────────────────────────────────────────────────
static val get_ae_diagnostics(ftd::AtomEngine& ae) {
    auto d = ae.diagnostics();
    val result = val::object();
    result.set("tick",          d.tick);
    result.set("atomCount",     d.atom_count);
    result.set("bondCount",     d.bond_count);
    result.set("totalKE",       d.total_ke);
    result.set("totalPEIonic",  d.total_pe_ionic);
    result.set("totalPEVdw",    d.total_pe_vdw);
    result.set("totalPEBond",   d.total_pe_bond);
    result.set("totalEnergy",   d.total_energy);
    result.set("momentumX",     d.total_momentum.x);
    result.set("momentumY",     d.total_momentum.y);
    result.set("momentumZ",     d.total_momentum.z);
    result.set("temperature",   d.temperature);
    return result;
}

// ── AE Atom injection ──────────────────────────────────────────────
static int ae_add_atom(ftd::AtomEngine& ae, int Z,
                       double x, double y, double z,
                       double vx, double vy, double vz,
                       int charge, int N) {
    return ae.add_atom(Z, ftd::Vec3(x, y, z), ftd::Vec3(vx, vy, vz), charge, N);
}

static int ae_add_locked_atom(ftd::AtomEngine& ae, int Z,
                               double x, double y, double z,
                               int charge, int N) {
    return ae.add_locked_atom(Z, ftd::Vec3(x, y, z), charge, N);
}

static int ae_create_bond(ftd::AtomEngine& ae, int id_a, int id_b, int order) {
    ae.create_bond(id_a, id_b, order);
    return 0;
}

// ── AE Controls ────────────────────────────────────────────────────
static void ae_set_dt(ftd::AtomEngine& ae, double dt) { ae.set_dt(dt); }
static double ae_get_dt(ftd::AtomEngine& ae) { return ae.dt(); }
static void ae_set_softening(ftd::AtomEngine& ae, double s) { ae.set_softening(s); }
static void ae_set_damping(ftd::AtomEngine& ae, bool e) { ae.set_damping_enabled(e); }
static void ae_set_bonding(ftd::AtomEngine& ae, bool e) { ae.set_bonding_enabled(e); }
static int ae_atom_count(ftd::AtomEngine& ae) { return static_cast<int>(ae.atoms().size()); }

static void ae_clear(ftd::AtomEngine& ae) {
    ae.atoms().clear();
}

// ── AE Toggle getter/setter (generic, by name) ────────────────────
// Pointer-to-member map for AtomToggles.
using AeBoolPTM = bool ftd::AtomToggles::*;
static const std::unordered_map<std::string, AeBoolPTM>& ae_toggle_map() {
    static const std::unordered_map<std::string, AeBoolPTM> kMap = {
        {"ionic",               &ftd::AtomToggles::ionic},
        {"van_der_waals",       &ftd::AtomToggles::van_der_waals},
        {"covalent_bonds",      &ftd::AtomToggles::covalent_bonds},
        {"auto_bonding",        &ftd::AtomToggles::auto_bonding},
        {"damping",             &ftd::AtomToggles::damping},
        {"h_bonds",             &ftd::AtomToggles::h_bonds},
        {"dipole_dipole",       &ftd::AtomToggles::dipole_dipole},
        {"angle_strain",        &ftd::AtomToggles::angle_strain},
        {"torsional",           &ftd::AtomToggles::torsional},
        {"improper_torsional",  &ftd::AtomToggles::improper_torsional},
        {"thermostat",          &ftd::AtomToggles::thermostat},
        {"electronegativity",   &ftd::AtomToggles::electronegativity},
    };
    return kMap;
}

static void ae_set_toggle(ftd::AtomEngine& ae, const std::string& name, bool val) {
    auto it = ae_toggle_map().find(name);
    if (it != ae_toggle_map().end()) ae.toggles.*(it->second) = val;
}

static bool ae_get_toggle(ftd::AtomEngine& ae, const std::string& name) {
    auto it = ae_toggle_map().find(name);
    if (it != ae_toggle_map().end()) return ae.toggles.*(it->second);
    return false;
}

// ── AE Force Diagnostic ───────────────────────────────────────────
static val get_ae_force_diag(ftd::AtomEngine& ae, int idx) {
    val result = val::object();
    const auto& fd = ae.force_diag();
    if (idx < 0 || idx >= static_cast<int>(fd.size())) return result;
    const auto& d = fd[idx];
    result.set("ionic_x", d.f_ionic.x); result.set("ionic_y", d.f_ionic.y); result.set("ionic_z", d.f_ionic.z);
    result.set("vdw_x", d.f_vdw.x); result.set("vdw_y", d.f_vdw.y); result.set("vdw_z", d.f_vdw.z);
    result.set("bond_x", d.f_bond.x); result.set("bond_y", d.f_bond.y); result.set("bond_z", d.f_bond.z);
    result.set("hbond_x", d.f_hbond.x); result.set("hbond_y", d.f_hbond.y); result.set("hbond_z", d.f_hbond.z);
    result.set("dipole_x", d.f_dipole.x); result.set("dipole_y", d.f_dipole.y); result.set("dipole_z", d.f_dipole.z);
    auto tot = d.total();
    result.set("total_x", tot.x); result.set("total_y", tot.y); result.set("total_z", tot.z);
    return result;
}

// ── Embind Registration ──────────────────────────────────────────────
EMSCRIPTEN_BINDINGS(ftd_module_atom) {
    class_<ftd::AtomEngine>("AtomEngine")
        .constructor<>()
        .function("tick", &ftd::AtomEngine::tick)
        .function("run",  &ftd::AtomEngine::run)
        .function("currentTick", &ftd::AtomEngine::current_tick)
        ;

    function("getAEAtomData",      &get_ae_atom_data);
    function("getAEDiagnostics",   &get_ae_diagnostics);
    function("aeAddAtom",          &ae_add_atom);
    function("aeAddLockedAtom",    &ae_add_locked_atom);
    function("aeCreateBond",       &ae_create_bond);
    function("aeSetDt",            &ae_set_dt);
    function("aeGetDt",            &ae_get_dt);
    function("aeSetSoftening",     &ae_set_softening);
    function("aeSetDamping",       &ae_set_damping);
    function("aeSetBonding",       &ae_set_bonding);
    function("aeSetToggle",        &ae_set_toggle);
    function("aeGetToggle",        &ae_get_toggle);
    function("aeGetForceDiag",     &get_ae_force_diag);
    function("aeAtomCount",        &ae_atom_count);
    function("aeClear",            &ae_clear);
}
