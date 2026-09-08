#pragma once
// Research candidate only: four physical ticks, no production physics adoption.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ftd::strict {
inline constexpr char LAW_ID[] = "phi-v2-staged-candidate-1";
inline constexpr std::size_t HEADER_BYTES = 116;
inline constexpr std::size_t BYTES_PER_SITE = 416;

struct State {
    std::uint32_t L = 0;
    std::uint64_t microtick = 0;
    std::vector<std::int8_t> s;
    std::vector<std::uint8_t> ell, bank, sc, fcc, admitted_sc, gate_sc, gate_fcc;
    explicit State(std::uint32_t size = 0);
    unsigned phase() const noexcept { return unsigned(microtick % 4); }
};
struct Collision {
    std::size_t site;
    int polarity;
    std::array<unsigned, 2> before, after;
};
struct RelationEvent {
    bool fcc;
    std::size_t owner;
    unsigned axis, diagonal;
    int direction;
};
struct Events {
    // x, channel, owner, axis; order matches the Python reference.
    std::vector<std::array<std::size_t, 4>> absorptions;
    std::vector<Collision> collisions;
    std::vector<RelationEvent> crossings, gate_holds;
};
struct StepResult { State state; Events events; };

void validate(const State& state);
std::uint64_t work_units(const State& state);
StepResult step(const State& state);
// Strong exception guarantee: state commits only after the entire batch succeeds.
std::vector<Events> advance(State& state, std::uint64_t ticks);
std::vector<std::uint8_t> encode(const State& state);
State decode(const std::vector<std::uint8_t>& bytes);
std::string events_json(const std::vector<Events>& events);
}  // namespace ftd::strict
