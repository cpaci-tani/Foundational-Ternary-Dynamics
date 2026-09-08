#pragma once
// Research candidate only: native C++ port of phi-hydro-staged-candidate-1
// (scripts/phi_v2_lattice/hydro/*.py). No production physics adoption.
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ftd::hydro {

inline constexpr std::size_t HEADER_BYTES = 116;
inline constexpr std::size_t BYTES_PER_SITE = 229;

// s(n), bank(192n), sc(6n), fcc(12n), admitted_sc(3n), admitted_fcc(6n), gate_sc(3n),
// gate_fcc(6n); no `ell` (the successor law has no collision-layer schedule).
struct State {
    std::uint32_t L = 0;
    std::uint64_t microtick = 0;
    std::vector<std::int8_t> s;
    std::vector<std::uint8_t> bank;
    std::vector<std::uint8_t> sc;
    std::vector<std::uint8_t> fcc;
    std::vector<std::uint8_t> admitted_sc;
    std::vector<std::uint8_t> admitted_fcc;
    std::vector<std::uint8_t> gate_sc;
    std::vector<std::uint8_t> gate_fcc;
    explicit State(std::uint32_t size = 0);
    unsigned phase() const noexcept { return unsigned(microtick % 4); }
};

// (x, c, kind(fcc?), owner, idx0, idx1); idx1 = 0 for an sc absorption.
struct Absorption {
    std::size_t x;
    unsigned c;
    bool fcc;
    std::size_t owner;
    unsigned idx0, idx1;
};

// (site, polarity(+1/-1), before-mask, after-mask); present only when after != before.
struct Collision {
    std::size_t site;
    int polarity;
    std::uint32_t before, after;
};

// crossing: direction != 0; gate_hold: direction field unused (see events_json).
struct RelationEvent {
    bool fcc;
    std::size_t owner;
    unsigned axis, diagonal;
    int direction;
};

struct Events {
    std::vector<Absorption> absorptions;
    std::vector<Collision> collisions;
    std::vector<RelationEvent> crossings, gate_holds;
};

void validate(const State& state);
std::uint64_t work_units(const State& state);
// Advances `state` in place by exactly one microtick; returns that microtick's events.
Events step(State& state);
// Advances `state` in place by `microticks` microticks (strong exception guarantee:
// state is committed only after every microtick in the batch succeeds); returns the
// per-category concatenation of every microtick's events, in tick order.
Events advance(State& state, std::uint64_t microticks);
std::vector<std::uint8_t> encode(const State& state);
State decode(const std::vector<std::uint8_t>& bytes);
std::string events_json(const Events& events);
// Loads and SHA-256-verifies the 2^24-entry collision blob (against tables::TABLE_HASH);
// throws on any I/O, length, or hash mismatch.
void load_table(const std::filesystem::path& path);
// Same verification and install from an in-memory blob (the WASM bindings cannot read the
// filesystem); throws on length or hash mismatch. load_table(path) delegates to this.
void load_table(const std::vector<std::uint8_t>& bytes);
// Throws if load_table() has not yet succeeded.
const std::vector<std::uint32_t>& table();

}  // namespace ftd::hydro
