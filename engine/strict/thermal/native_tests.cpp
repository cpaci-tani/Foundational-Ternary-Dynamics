#include "finite_thermal.h"
#include "../hydro/hydro_sha256.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

namespace {
using namespace ftd::thermal;
using Bytes = std::vector<std::uint8_t>;
unsigned checks = 0;

void check(bool condition, const char* label) {
    ++checks;
    if (!condition) throw std::runtime_error(label);
}

template<class Error, class Function>
void rejects(Function&& operation, const char* label) {
    bool rejected = false;
    try { operation(); } catch (const Error&) { rejected = true; }
    check(rejected, label);
}

bool same(const Records& a, const Records& b) {
    return a.L == b.L && a.microtick == b.microtick && a.bank == b.bank;
}

bool same(const Totals& a, const Totals& b) {
    return a.N == b.N && a.E2 == b.E2 && a.P == b.P;
}

std::size_t site(unsigned x, unsigned y, unsigned z, unsigned L) {
    return x + std::size_t(L) * (y + std::size_t(L) * z);
}

std::array<int, 4> pair_key(Pair pair) {
    const auto a = velocity(pair[0]), b = velocity(pair[1]);
    std::array<int, 4> key{};
    for (unsigned i = 0; i < 3; ++i) {
        key[i] = int(a[i]) + int(b[i]);
        key[3] += int(a[i]) * int(a[i]) + int(b[i]) * int(b[i]);
    }
    return key;
}

#if defined(FTD_THERMAL_EQUIVARIANT)
std::array<int, 2> shell_energies(Pair pair) {
    std::array<int, 2> result{};
    for (unsigned i = 0; i < 2; ++i) {
        const auto v = velocity(pair[i]);
        for (const auto component : v) result[i] += int(component) * int(component);
    }
    std::sort(result.begin(), result.end());
    return result;
}

using ChannelTransform = std::array<std::uint16_t, CHANNELS>;

Pair transform_pair(Pair pair, const ChannelTransform& transform) {
    Pair result{transform[pair[0]], transform[pair[1]]};
    if (result[0] > result[1]) std::swap(result[0], result[1]);
    return result;
}

void test_pair_cubic_covariance() {
    std::array<unsigned, 3> permutation{0, 1, 2};
    unsigned transformations = 0;
    do {
        for (unsigned signs = 0; signs < 8; ++signs) {
            ChannelTransform transform{};
            for (std::size_t c = 0; c < CHANNELS; ++c) {
                const auto v = velocity(c);
                std::array<int, 3> transformed{};
                for (unsigned axis = 0; axis < 3; ++axis)
                    transformed[axis] = int(v[permutation[axis]]) * ((signs >> axis) & 1u ? -1 : 1);
                transform[c] = std::uint16_t(channel_index(transformed[0], transformed[1], transformed[2]));
            }
            for (std::size_t a = 0; a < CHANNELS; ++a)
                for (std::size_t b = a + 1; b < CHANNELS; ++b) {
                    const auto input = transform_pair({std::uint16_t(a), std::uint16_t(b)}, transform);
                    check(collision_successor(input[0], input[1])
                          == transform_pair(collision_successor(a, b), transform),
                          "pair involution commutes with every signed axis permutation");
                }
            ++transformations;
        }
    } while (std::next_permutation(permutation.begin(), permutation.end()));
    check(transformations == 48, "all 48 cubic transformations checked");
}
#endif

void rehash(Bytes& bytes) {
    const auto digest = ftd::hydro::sha256(bytes.data(), bytes.size() - CHECKSUM_BYTES);
    std::copy(digest.begin(), digest.end(), bytes.end() - CHECKSUM_BYTES);
}

Records fixture() {
    Records records(5);
    const auto origin = site(2, 2, 2, records.L);
    set_occupied(records.bank[origin], channel_index(0, 0, 0));
    set_occupied(records.bank[origin], channel_index(2, 0, 0));
    set_occupied(records.bank[site(0, 1, 3, records.L)], channel_index(-3, 2, -1));
    set_occupied(records.bank[site(4, 0, 2, records.L)], channel_index(3, -2, 1));
    for (const auto v : {Velocity{-1, 0, 0}, Velocity{0, 1, 0}, Velocity{0, 0, -1}})
        set_occupied(records.bank[site(3, 3, 0, records.L)], channel_index(v[0], v[1], v[2]));
    return records;
}

std::string table_hash_hex() {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (const auto byte : collision_table_hash()) out << std::setw(2) << unsigned(byte);
    return out.str();
}

void test_alphabet_and_collision_table() {
    check(velocity(0) == Velocity{-3, -3, -3}, "first velocity");
    check(velocity(342) == Velocity{3, 3, 3}, "last velocity");
    for (std::size_t c = 0; c < CHANNELS; ++c) {
        const auto v = velocity(c);
        check(channel_index(v[0], v[1], v[2]) == c, "velocity index inverse");
    }
    rejects<std::out_of_range>([] { (void)velocity(CHANNELS); }, "invalid channel rejected");
    rejects<std::out_of_range>([] { (void)channel_index(4, 0, 0); }, "invalid velocity rejected");
    rejects<std::invalid_argument>([] { (void)collision_successor(1, 1); }, "nondistinct pair rejected");
    rejects<std::invalid_argument>([] { (void)collision_successor(2, 1); }, "unordered pair rejected");
    rejects<std::invalid_argument>([] { (void)collision_successor(0, CHANNELS); }, "out-of-range pair rejected");
    std::vector<bool> seen(CHANNELS * CHANNELS, false);
    std::size_t destinations = 0;
#if defined(FTD_THERMAL_EQUIVARIANT)
    std::size_t moved = 0, shell_exchanges = 0;
    Pair first_shell_input{}, first_shell_output{};
#endif
    for (std::size_t a = 0; a < CHANNELS; ++a)
        for (std::size_t b = a + 1; b < CHANNELS; ++b) {
            const auto out = collision_successor(a, b);
            check(out[0] < out[1] && out[1] < CHANNELS, "pair remains valid");
            check(pair_key({std::uint16_t(a), std::uint16_t(b)}) == pair_key(out),
                  "every pair conserves P and E2");
            const auto index = std::size_t(out[0]) * CHANNELS + out[1];
            check(!seen[index], "collision table is injective");
            seen[index] = true;
            ++destinations;
#if defined(FTD_THERMAL_EQUIVARIANT)
            const Pair input{std::uint16_t(a), std::uint16_t(b)};
            check(collision_successor(out[0], out[1]) == input, "every pair map is an involution");
            moved += out != input;
            if (shell_energies(input) != shell_energies(out)) {
                if (shell_exchanges == 0) { first_shell_input = input; first_shell_output = out; }
                ++shell_exchanges;
            }
#endif
        }
    check(destinations == PAIRS, "complete pair permutation");
#if defined(FTD_THERMAL_EQUIVARIANT)
    check(moved == 44856 && PAIRS - moved == 13797, "independent equivariant moved/fixed pair counts");
    check(shell_exchanges != 0, "equivariant pair law exchanges individual shell energy");
    Records shell_records(3);
    for (const auto c : first_shell_input) set_occupied(shell_records.bank[0], c);
    const auto shell_before = totals(shell_records);
    Scratch shell_scratch;
    step(shell_records, shell_scratch);
    Bank expected{};
    for (const auto c : first_shell_output) set_occupied(expected, c);
    check(shell_records.bank[0] == expected && same(totals(shell_records), shell_before),
          "actual equivariant tick exchanges shell energy and conserves complete totals");
    check(table_hash_hex() == "726908e917ef3723d02b1f9d50285fe64b1ac4e0b614b45ef3e5b1a6971f7d6d",
          "complete equivariant table matches independent frozen enumeration");
    test_pair_cubic_covariance();
#else
    // Independent Python enumeration's canonical <4H a,b,next_a,next_b> digest.
    check(table_hash_hex() == "fa4a959896e7b0aa8fdf75271296866d777fa86aa97b6230205f0b5448cfe6cf",
          "complete table matches independent frozen enumeration");
    const auto shell_exchange = collision_successor(channel_index(0, 0, 0), channel_index(2, 0, 0));
    check(shell_exchange == Pair{std::uint16_t(channel_index(1, -1, 0)),
                                std::uint16_t(channel_index(1, 1, 0))},
          "pinned lex successor exchanges 0+4 shell energy for 2+2");
    const auto next = collision_successor(shell_exchange[0], shell_exchange[1]);
    check(next == Pair{std::uint16_t(channel_index(1, 0, -1)),
                      std::uint16_t(channel_index(1, 0, 1))}, "pinned middle class successor");
    check(collision_successor(next[0], next[1])
          == Pair{std::uint16_t(channel_index(0, 0, 0)), std::uint16_t(channel_index(2, 0, 0))},
          "pinned class wraps to first pair");
#endif
}

void test_collision_and_identity() {
    Records records(3);
    set_occupied(records.bank[0], 0);
    set_occupied(records.bank[0], 342);
    set_occupied(records.bank[1], channel_index(0, 0, 0));
    for (const auto c : {0u, 100u, 342u}) set_occupied(records.bank[2], c);
    Bank full{};
    for (std::size_t c = 0; c < CHANNELS; ++c) set_occupied(full, c);
    records.bank[3] = full;
    const auto before = records;
    const auto conserved = totals(records);
    Scratch scratch;
    step(records, scratch);
    check(same(totals(records), conserved), "phase zero conserves complete totals");
    check(pair_key(collision_successor(0, 342))[3] == 54, "maximum-energy pair retained");
    check(records.bank[1] == before.bank[1] && records.bank[2] == before.bank[2]
          && records.bank[3] == full && records.bank[4] == Bank{},
          "zero one three and full occupancy are collision identities");
    check(manifestation(Bank{}) == 0, "empty manifestation");
    check(manifestation(records.bank[0]) == -1 && manifestation(records.bank[1]) == 1
          && manifestation(records.bank[2]) == 1 && manifestation(full) == 1,
          "nonempty manifestation quotient uses parity");
}

void test_locality_and_cycle_displacement() {
    // Complete channel coverage at center and seam witnesses; inspect only
    // one microtick at a time so no four-tick movement is mistaken for one hop.
    for (unsigned phase = 0; phase < 4; ++phase)
        for (const auto position : {std::array<unsigned, 3>{0, 0, 0},
                                    std::array<unsigned, 3>{3, 3, 3},
                                    std::array<unsigned, 3>{6, 6, 6}})
            for (std::size_t c = 0; c < CHANNELS; ++c) {
                Records records(7);
                records.microtick = phase;
                set_occupied(records.bank[site(position[0], position[1], position[2], 7)], c);
                const auto v = velocity(c);
                std::array<unsigned, 3> target = position;
                for (unsigned axis = 0; axis < 3; ++axis) {
                    int move = 0;
                    if (phase != 0 && (v[axis] >= int(phase) || v[axis] <= -int(phase)))
                        move = v[axis] > 0 ? 1 : -1;
                    target[axis] = unsigned((int(position[axis]) + move + 7) % 7);
                    const auto distance = unsigned(std::abs(int(target[axis]) - int(position[axis])));
                    check(std::min(distance, 7 - distance) <= 1, "periodic one-hop support");
                }
                Scratch scratch;
                step(records, scratch);
                check(totals(records).N == 1
                      && occupied(records.bank[site(target[0], target[1], target[2], 7)], c),
                      "single channel streams to declared one-hop target");
                check(records.microtick == phase + 1, "one step advances one real tick");
            }
    for (const auto v : {Velocity{-3, 2, -1}, Velocity{3, -3, 3}, Velocity{0, 0, 0}}) {
        Records records(9);
        const auto c = channel_index(v[0], v[1], v[2]);
        set_occupied(records.bank[site(4, 4, 4, 9)], c);
        Scratch scratch;
        advance(records, scratch, 4);
        check(occupied(records.bank[site(unsigned(4 + v[0]), unsigned(4 + v[1]),
                                         unsigned(4 + v[2]), 9)], c),
              "four real ticks produce displacement v");
    }
}

void test_checkpoint_replay_and_conservation() {
    auto records = fixture();
    const auto conserved = totals(records);
    Scratch scratch;
    for (unsigned tick = 0; tick < 32; ++tick) {
        const auto bytes = encode(records);
        check(bytes.size() == HEADER_BYTES + records.bank.size() * BYTES_PER_SITE + CHECKSUM_BYTES,
              "exact checkpoint length");
        auto restored = decode(bytes);
        check(same(records, restored) && encode(restored) == bytes, "every phase checkpoint exact round trip");
        Scratch restored_scratch;
        step(records, scratch);
        step(restored, restored_scratch);
        check(encode(records) == encode(restored), "every phase restore and replay is byte-exact");
        check(same(totals(records), conserved), "all phases conserve exact N P E2");
    }
    for (unsigned initial_phase = 0; initial_phase < 4; ++initial_phase) {
        auto serial = fixture();
        serial.microtick = initial_phase;
        auto batch = serial;
        Scratch a, b;
        for (unsigned tick = 0; tick < 19; ++tick) step(serial, a);
        advance(batch, b, 19);
        check(encode(serial) == encode(batch), "serial steps equal atomic batch at every initial phase");
        const auto before = encode(batch);
        advance(batch, b, 0);
        check(encode(batch) == before, "zero batch is identity");
    }
}

void test_rejection_and_horizons() {
    rejects<std::invalid_argument>([] { Records invalid(2); }, "L below range rejected");
    rejects<std::invalid_argument>([] { Records invalid(129); }, "L above range rejected");
    Records max_shape;
    max_shape.L = 128;
    rejects<std::invalid_argument>([&] { validate(max_shape); }, "L128 requires its complete bank shape");
    auto bad_shape = fixture();
    bad_shape.bank.pop_back();
    const auto bad_shape_before = bad_shape;
    Scratch scratch;
    rejects<std::invalid_argument>([&] { step(bad_shape, scratch); }, "shape rejected before step");
    check(same(bad_shape, bad_shape_before), "malformed shape leaves records unchanged");
    auto bad_bits = fixture();
    bad_bits.bank.back().back() |= std::uint64_t{1} << 23;
    const auto bad_bits_before = bad_bits;
    rejects<std::invalid_argument>([&] { advance(bad_bits, scratch, 1); }, "unused bits rejected before batch");
    check(same(bad_bits, bad_bits_before), "malformed bits leave records unchanged");
    rejects<std::invalid_argument>([&] { (void)encode(bad_bits); }, "malformed records cannot encode");
    rejects<std::invalid_argument>([&] { (void)manifestation(bad_bits.bank.back()); }, "quotient rejects malformed bank");
    auto horizon = fixture();
    horizon.microtick = std::numeric_limits<std::uint64_t>::max() - 1;
    const auto before = encode(horizon);
    rejects<std::overflow_error>([&] { advance(horizon, scratch, 2); }, "batch horizon rejected");
    check(encode(horizon) == before, "batch overflow is atomic");
    step(horizon, scratch);
    check(horizon.microtick == std::numeric_limits<std::uint64_t>::max(), "last representable step succeeds");
    const auto terminal = encode(horizon);
    rejects<std::overflow_error>([&] { step(horizon, scratch); }, "terminal step rejected");
    check(encode(horizon) == terminal && encode(decode(terminal)) == terminal,
          "terminal clock remains checkpointable and unchanged");

    const auto valid = encode(fixture());
#if defined(FTD_THERMAL_EQUIVARIANT)
    constexpr char foreign_law[] = "phi-thermal-pair-candidate-1";
#else
    constexpr char foreign_law[] = "phi-thermal-equivariant-candidate-2";
#endif
    auto foreign = valid;
    const auto foreign_hash = ftd::hydro::sha256(
        reinterpret_cast<const std::uint8_t*>(foreign_law), sizeof(foreign_law) - 1);
    std::copy(foreign_hash.begin(), foreign_hash.end(), foreign.begin() + 24);
    rehash(foreign);
    rejects<std::invalid_argument>([&] { (void)decode(foreign); },
                                   "other candidate version rejected despite valid checksum");
    for (const auto offset : {std::size_t{0}, std::size_t{8}, std::size_t{12},
                              std::size_t{24}, std::size_t{56}, std::size_t{88}}) {
        auto bytes = valid;
        bytes[offset] ^= 0x80;
        rehash(bytes);
        rejects<std::invalid_argument>([&] { (void)decode(bytes); }, "foreign metadata rejected with valid checksum");
    }
    auto corrupt = valid;
    corrupt[HEADER_BYTES] ^= 1;
    rejects<std::invalid_argument>([&] { (void)decode(corrupt); }, "payload checksum rejects corruption");
    corrupt = valid;
    corrupt.back() ^= 1;
    rejects<std::invalid_argument>([&] { (void)decode(corrupt); }, "checksum corruption rejected");
    corrupt = valid;
    corrupt.pop_back();
    rejects<std::invalid_argument>([&] { (void)decode(corrupt); }, "truncated checkpoint rejected");
    corrupt = valid;
    corrupt.push_back(0);
    rejects<std::invalid_argument>([&] { (void)decode(corrupt); }, "trailing checkpoint garbage rejected");
    corrupt = valid;
    // Last site's high word, bit23 is the first forbidden bit.
    corrupt[HEADER_BYTES + (fixture().bank.size() - 1) * BYTES_PER_SITE + 5 * 8 + 2] |= 0x80;
    rehash(corrupt);
    rejects<std::invalid_argument>([&] { (void)decode(corrupt); }, "forbidden bits rejected with valid checksum");
    rejects<std::invalid_argument>([] { (void)decode(Bytes{}); }, "empty checkpoint rejected");
}

std::uint64_t parse_ticks(const std::string& text) {
    if (text.empty()) throw std::invalid_argument("empty tick count");
    std::uint64_t value = 0;
    for (const auto c : text) {
        if (c < '0' || c > '9'
            || value > (std::numeric_limits<std::uint64_t>::max() - unsigned(c - '0')) / 10)
            throw std::invalid_argument("tick count must be uint64 decimal");
        value = value * 10 + unsigned(c - '0');
    }
    return value;
}

int checkpoint_harness(const char* input_path, const char* output_path, const char* ticks) {
    std::ifstream input(input_path, std::ios::binary);
    if (!input) throw std::runtime_error("cannot open input checkpoint");
    const Bytes bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (input.bad()) throw std::runtime_error("cannot read input checkpoint");
    auto records = decode(bytes);
    Scratch scratch;
    advance(records, scratch, parse_ticks(ticks));
    const auto result = encode(records);
    std::ofstream output(output_path, std::ios::binary | std::ios::trunc);
    if (!output) throw std::runtime_error("cannot open output checkpoint");
    output.write(reinterpret_cast<const char*>(result.data()), std::streamsize(result.size()));
    output.close();
    if (!output) throw std::runtime_error("cannot write output checkpoint");
    return 0;
}
}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--table-hash") {
            std::cout << table_hash_hex() << '\n';
            return 0;
        }
        if (argc == 5 && std::string(argv[1]) == "--checkpoint")
            return checkpoint_harness(argv[2], argv[3], argv[4]);
        if (argc != 1)
            throw std::invalid_argument("usage: ftd_finite_thermal_tests [--table-hash | --checkpoint INPUT OUTPUT TICKS]");
        test_alphabet_and_collision_table();
        test_collision_and_identity();
        test_locality_and_cycle_displacement();
        test_checkpoint_replay_and_conservation();
        test_rejection_and_horizons();
        std::cout << LAW_ID << ": " << checks << " exact native contract checks PASS\n"
                  << "Research candidate only; thermal equilibrium and gas closure are not established.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "finite thermal candidate FAIL: " << error.what() << '\n';
        return 1;
    }
}
