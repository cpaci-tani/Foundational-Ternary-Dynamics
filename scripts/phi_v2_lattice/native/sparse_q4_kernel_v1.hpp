#ifndef FTD_SPARSE_Q4_KERNEL_V1_HPP
#define FTD_SPARSE_Q4_KERNEL_V1_HPP

// Offline exact representation of the frozen nineteen-stage law.
// No observer labels, catalogue data, random source or engine dependency.
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ftd::q4native_v1 {
inline constexpr char LAW_ID[] = "phi-flux-credit-exchange-binding-candidate-1";
inline constexpr char RULE_HASH[] = "ff47ad30df96caa97a0b2c9dc7126eb5b13a9cef6aa2a7105b4879d697baae5c";
inline constexpr char FRAME_HASH[] = "8086a3308540928b7b8f88eac229bb9c5425f378b8dff4cbab3b0aabd45000d2";
inline constexpr char DENSE_ENCODING[] = "f74a9bb36fabb75e9885fff1e340f144949fbff724321675cbbec8ba1eb4d382";
inline constexpr char BACKEND_ID[] = "native-credit-exchange-q4-sparse-1";
inline constexpr char WIRE_ID[] = "credit-exchange-q4-native-wire-1";
inline constexpr char WIRE_MAGIC[] = "FTD-Q4-NATIVE-1\n";
inline constexpr std::uint32_t MAX_SITE = 64U*64U*64U-1U;

enum class ErrorCategory { INVALID_RECORD, UNSUPPORTED_DOMAIN, INVALID_WIRE,
                           ALLOCATION_FAILURE, INTERNAL_INVARIANT_FAILURE };
const char* category_name(ErrorCategory) noexcept;
class Error : public std::runtime_error {
public:
    Error(ErrorCategory category, const std::string& message);
    ErrorCategory category() const noexcept { return category_; }
private:
    ErrorCategory category_;
};

struct Carrier {
    std::uint32_t site{};
    std::uint8_t slot{}, direction{}, credit{}, attempted{};
};
struct Edge {
    std::uint32_t owner{};
    std::uint8_t axis{};
    std::int8_t q{};
};
struct Payload {
    std::uint16_t L{};
    std::uint8_t origin_code{}, eta{}, edge_count{};
    std::array<Carrier,4> carriers{};
    std::array<Edge,4> edges{};
};
class State {
public:
    State(const State&) = default;
    State(State&&) noexcept = default;
    State& operator=(const State&) = default;
    State& operator=(State&&) noexcept = default;
private:
    State(const Payload&, std::string);
    Payload payload_;
    std::string microtick_;
    friend State admit(const Payload&, std::string_view);
    friend const Payload& payload(const State&);
    friend const std::string& microtick_hex(const State&);
};
struct MatchingEdge { std::uint32_t owner{},head{}; std::uint8_t axis{}; };
template<std::size_t Width, std::size_t Capacity> struct EventList {
    std::array<std::array<std::int32_t,Width>,Capacity> rows{};
    std::uint8_t count{};
};
enum Reason : std::int32_t { accepted=0, capacity=1, flux_capacity=2,
                            credit_deficit=3, credit_capacity=4 };
struct Events {
    EventList<9,2> moves;
    EventList<5,2> redirects;
    EventList<3,1> capacity_holds;
    EventList<3,2> attempt_marks;
    EventList<2,4> attempt_expiries;
    EventList<5,2> onsite_alignments;
    EventList<13,2> credit_exchanges;
};
struct StepResult { State state; Events events; };

State admit(const Payload&, std::string_view microtick_hex);
const Payload& payload(const State&);
const std::string& microtick_hex(const State&);
unsigned phase(const State&);
unsigned background_code(const State&, std::uint32_t site);
std::uint32_t shift_site(const State&, std::uint32_t site, unsigned axis, int sign);
MatchingEdge matching_edge(const State&, std::uint32_t site, unsigned column, unsigned parity);
StepResult step(const State&);
std::vector<std::uint8_t> encode_state(const State&);
State decode_state(const std::uint8_t* bytes, std::size_t length);
std::vector<std::uint8_t> encode_events(const Events&);
Events decode_events(const std::uint8_t* bytes, std::size_t length);
} // namespace ftd::q4native_v1
#endif
