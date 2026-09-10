#include "carrier_observer.h"
#include <algorithm>
#include <stdexcept>
#include <tuple>

namespace ftd::carrier_current {
namespace {
namespace Q = q4native_v1;
void require(bool condition, const char* reason) {
    if (!condition) throw std::invalid_argument(reason);
}
std::string next_tick(std::string value) {
    constexpr char digits[] = "0123456789abcdef";
    require(!value.empty() && (value.size() == 1 || value.front() != '0'), "noncanonical ordinal");
    for (const auto character : value)
        require(std::find(digits, digits + 16, character) != digits + 16, "noncanonical ordinal");
    for (std::size_t i = value.size(); i > 0; --i) {
        const auto digit = std::find(digits, digits + 16, value[i - 1]) - digits;
        require(digit < 16, "noncanonical ordinal");
        if (digit < 15) { value[i - 1] = digits[digit + 1]; return value; }
        value[i - 1] = '0';
    }
    value.insert(value.begin(), '1');
    return value;
}
template<class T, std::size_t N, class Predicate>
T& obtain(SparseRows<T, N>& rows, Predicate match) {
    for (std::size_t i = 0; i < rows.count; ++i)
        if (match(rows.rows[i])) return rows.rows[i];
    require(rows.count < N, "sparse observation capacity exceeded");
    return rows.rows[rows.count++];
}
SiteBalance& site_row(SparseRows<SiteBalance, 32>& rows, std::uint32_t site) {
    auto& row = obtain(rows, [site](const auto& x) { return x.site == site; });
    row.site = site;
    return row;
}
EdgeBalance& edge_row(Observation& out, const Q::State& state,
                      std::uint32_t owner, unsigned axis) {
    auto& row = obtain(out.edges, [=](const auto& x) {
        return x.owner == owner && x.axis == axis;
    });
    row.owner = owner;
    row.axis = static_cast<std::uint8_t>(axis);
    row.head = Q::shift_site(state, owner, axis, 1);
    site_row(out.sites, owner);
    site_row(out.sites, row.head);
    return row;
}
int carrier_index(const Q::Payload& p, std::uint32_t site, unsigned slot) {
    for (unsigned i = 0; i < 4; ++i)
        if (p.carriers[i].site == site && p.carriers[i].slot == slot) return int(i);
    throw std::invalid_argument("event carrier absent from before state");
}
int flux(const Q::Payload& p, std::uint32_t owner, unsigned axis) {
    for (unsigned i = 0; i < p.edge_count; ++i)
        if (p.edges[i].owner == owner && p.edges[i].axis == axis) return p.edges[i].q;
    return 0;
}
void finish(SiteBalance& row) {
    for (unsigned p = 0; p < 2; ++p) {
        row.number_residual[p] = row.after_number[p] - row.before_number[p]
            + row.number_divergence[p];
        require(row.number_residual[p] == 0, "local polarity continuity residual");
    }
    row.account_residual = row.after_credit + row.after_flux_account
        - row.before_credit - row.before_flux_account + row.account_divergence;
    require(row.account_residual == 0, "local credit/owned-flux account residual");
}
std::uint32_t block_index(std::uint32_t site, unsigned L, unsigned width) {
    require(site < L * L * L, "observation site outside lattice");
    const auto B = L / width;
    return ((site / (L * L)) / width * B + ((site / L) % L) / width) * B
        + (site % L) / width;
}
void add_density(SiteBalance& to, const SiteBalance& from) {
    for (unsigned p = 0; p < 2; ++p) {
        to.before_number[p] += from.before_number[p];
        to.after_number[p] += from.after_number[p];
    }
    to.before_credit += from.before_credit;
    to.after_credit += from.after_credit;
    to.before_flux_account += from.before_flux_account;
    to.after_flux_account += from.after_flux_account;
}
} // namespace

Observation observe(const Q::State& before, const Q::State& after, const Q::Events& events) {
    const auto& a = Q::payload(before);
    const auto& b = Q::payload(after);
    require(a.L == b.L && a.origin_code == b.origin_code && a.eta == b.eta,
            "transaction background, boundary size, or charge frame changed");
    (void)Q::phase(before);
    (void)Q::phase(after);
    require(next_tick(Q::microtick_hex(before)) == Q::microtick_hex(after),
            "transaction ordinals are not consecutive");
    (void)Q::encode_events(events); // Independent finite event grammar check.
    const auto phase = Q::phase(before);
    const bool reset = events.attempt_expiries.count || events.onsite_alignments.count;
    const bool exchange = events.credit_exchanges.count != 0;
    const bool hop = events.moves.count || events.redirects.count
        || events.capacity_holds.count || events.attempt_marks.count;
    require((!reset || phase == 0) && (!exchange || (phase >= 1 && phase <= 6))
        && (!hop || phase >= 7), "event category disagrees with physical stage");
    const auto volume = std::uint32_t(a.L) * a.L * a.L;
    auto site_valid = [&](std::int32_t site) {
        require(site >= 0 && std::uint32_t(site) < volume, "event site outside actual lattice");
    };
    for (unsigned i = 0; i < events.redirects.count; ++i) site_valid(events.redirects.rows[i][1]);
    for (unsigned i = 0; i < events.capacity_holds.count; ++i) site_valid(events.capacity_holds.rows[i][1]);
    for (unsigned i = 0; i < events.attempt_marks.count; ++i) site_valid(events.attempt_marks.rows[i][1]);
    for (unsigned i = 0; i < events.attempt_expiries.count; ++i) site_valid(events.attempt_expiries.rows[i][1]);
    for (unsigned i = 0; i < events.onsite_alignments.count; ++i) site_valid(events.onsite_alignments.rows[i][0]);

    Observation out;
    out.context.L = a.L;
    out.context.origin_code = a.origin_code;
    out.context.eta = a.eta;
    out.context.start_tick_hex = Q::microtick_hex(before);
    out.context.end_tick_hex = Q::microtick_hex(after);
    auto read_state = [&](const Q::State& state, bool terminal) {
        const auto& p = Q::payload(state);
        for (const auto& c : p.carriers) {
            auto& row = site_row(out.sites, c.site);
            (terminal ? row.after_number : row.before_number)[c.slot]++;
            (terminal ? row.after_credit : row.before_credit) += c.credit;
        }
        for (unsigned i = 0; i < p.edge_count; ++i) {
            const auto& e = p.edges[i];
            auto& edge = edge_row(out, before, e.owner, e.axis);
            (terminal ? edge.after_flux : edge.before_flux) = e.q;
            auto& row = site_row(out.sites, e.owner);
            (terminal ? row.after_flux_account : row.before_flux_account) += int(e.q) * e.q;
        }
    };
    read_state(before, false);
    read_state(after, true);
    auto expected = a.carriers;
    std::array<bool, 4> touched{};
    auto touch = [&](int index) {
        require(!touched[std::size_t(index)], "carrier appears in duplicate account transactions");
        touched[std::size_t(index)] = true;
    };
    for (unsigned i = 0; i < events.moves.count; ++i) {
        const auto& r = events.moves.rows[i];
        site_valid(r[1]); site_valid(r[2]); site_valid(r[3]);
        const auto owner = std::uint32_t(r[3]);
        const auto head = Q::shift_site(before, owner, unsigned(r[4]), 1);
        const int sigma = r[1] == r[3] ? 1 : -1;
        require((std::uint32_t(r[1]) == owner && std::uint32_t(r[2]) == head)
            || (std::uint32_t(r[1]) == head && std::uint32_t(r[2]) == owner),
                "move does not cross its declared nearest-neighbor edge");
        const unsigned slot = r[0] == 1 ? 0 : 1;
        require(slot == (((phase - 7) / 6) ^ a.eta), "move polarity disagrees with stage");
        const int index = carrier_index(a, std::uint32_t(r[1]), slot);
        touch(index);
        require(a.carriers[std::size_t(index)].credit == r[7], "move before credit mismatch");
        require(flux(a, owner, unsigned(r[4])) == r[5]
            && flux(b, owner, unsigned(r[4])) == r[6], "move edge flux mismatch");
        expected[std::size_t(index)].site = std::uint32_t(r[2]);
        expected[std::size_t(index)].credit = std::uint8_t(r[8]);
        auto& edge = edge_row(out, before, owner, unsigned(r[4]));
        edge.number_current[slot] += sigma;
        edge.account_current += sigma == 1 ? r[8] : -r[7];
    }
    for (unsigned i = 0; i < events.credit_exchanges.count; ++i) {
        const auto& r = events.credit_exchanges.rows[i];
        site_valid(r[0]); site_valid(r[3]); site_valid(r[5]);
        const auto owner = std::uint32_t(r[0]);
        const auto head = Q::shift_site(before, owner, unsigned(r[1]), 1);
        const bool forward = std::uint32_t(r[3]) == owner;
        require(std::uint32_t(r[forward ? 5 : 3]) == head
            && std::uint32_t(r[forward ? 3 : 5]) == owner, "credit exchange edge incidence mismatch");
        const unsigned donor_slot = r[2] == 1 ? 0 : 1;
        const int donor = carrier_index(a, std::uint32_t(r[3]), donor_slot);
        const int recipient = carrier_index(a, std::uint32_t(r[5]), donor_slot ^ 1);
        touch(donor); touch(recipient);
        require(a.carriers[std::size_t(donor)].credit == 1
            && a.carriers[std::size_t(recipient)].credit == 0, "credit exchange input mismatch");
        require(flux(a, owner, unsigned(r[1])) == (forward ? r[2] : -r[2])
            && flux(a, owner, unsigned(r[1])) == flux(b, owner, unsigned(r[1])),
            "credit exchange supporting flux mismatch");
        expected[std::size_t(donor)].credit = 0;
        expected[std::size_t(recipient)].credit = 1;
        edge_row(out, before, owner, unsigned(r[1])).account_current += forward ? 1 : -1;
    }
    std::sort(expected.begin(), expected.end(), [](const auto& x, const auto& y) {
        return std::tie(x.site, x.slot) < std::tie(y.site, y.slot);
    });
    for (unsigned i = 0; i < 4; ++i)
        require(expected[i].site == b.carriers[i].site && expected[i].slot == b.carriers[i].slot
            && expected[i].credit == b.carriers[i].credit,
            "events do not reproduce carrier occupancy and credits");

    for (std::size_t i = 0; i < out.edges.count; ++i) {
        auto& e = out.edges.rows[i];
        e.signed_charge_current = e.number_current[0] - e.number_current[1];
        e.flux_residual = e.after_flux - e.before_flux + e.signed_charge_current;
        require(e.flux_residual == 0, "edge current/flux update residual");
        auto& owner = site_row(out.sites, e.owner);
        auto& head = site_row(out.sites, e.head);
        for (unsigned p = 0; p < 2; ++p) {
            owner.number_divergence[p] += e.number_current[p];
            head.number_divergence[p] -= e.number_current[p];
        }
        owner.account_divergence += e.account_current;
        head.account_divergence -= e.account_current;
    }
    for (std::size_t i = 0; i < out.sites.count; ++i) finish(out.sites.rows[i]);
    std::sort(out.sites.rows.begin(), out.sites.rows.begin() + out.sites.count,
        [](const auto& x, const auto& y) { return x.site < y.site; });
    std::sort(out.edges.rows.begin(), out.edges.rows.begin() + out.edges.count,
        [](const auto& x, const auto& y) { return std::tie(x.owner, x.axis) < std::tie(y.owner, y.axis); });
    return out;
}

BlockObservation restrict_blocks(const Observation& observation, unsigned width) {
    const unsigned L = observation.context.L;
    const auto& context = observation.context;
    const Context expected_context;
    auto same_text = [](const char* a, const char* b) { return a && b && std::string(a) == b; };
    require(same_text(context.law_id, expected_context.law_id)
        && same_text(context.boundary, expected_context.boundary)
        && same_text(context.number_units, expected_context.number_units)
        && same_text(context.account_units, expected_context.account_units)
        && context.origin_code < 48 && context.eta < 2
        && !context.mechanical_energy_available && !context.mechanical_momentum_available
        && !context.spin_available && !context.rest_mass_available
        && !context.particle_identification_available && !observation.complete_transition_law_validated,
        "observation context or physical capability was altered");
    require(!context.start_tick_hex.empty()
        && (context.start_tick_hex.size() == 1 || context.start_tick_hex.front() != '0')
        && next_tick(context.start_tick_hex) == context.end_tick_hex,
        "observation clock context is not a canonical consecutive interval");
    require(L >= 4 && L <= 64 && L % 2 == 0 && width > 0 && width <= L && L % width == 0,
            "block width must divide the declared lattice size");
    require(observation.sites.count <= observation.sites.rows.size()
        && observation.edges.count <= observation.edges.rows.size(), "invalid sparse observation counts");
    // Observation is a public value type. Validate finite candidate bounds
    // before any arithmetic, including when a caller supplies a forged value.
    for (std::size_t i = 0; i < observation.sites.count; ++i) {
        const auto& row = observation.sites.rows[i];
        require(row.site < L * L * L && (i == 0 || observation.sites.rows[i - 1].site < row.site),
                "unsorted, duplicate, or out-of-domain observation site");
        for (unsigned p = 0; p < 2; ++p)
            require(row.before_number[p] >= 0 && row.before_number[p] <= 1
                && row.after_number[p] >= 0 && row.after_number[p] <= 1
                && row.number_divergence[p] >= -2 && row.number_divergence[p] <= 2
                && row.number_residual[p] == 0, "invalid site number account");
        require(row.before_credit >= 0 && row.before_credit <= row.before_number[0] + row.before_number[1]
            && row.after_credit >= 0 && row.after_credit <= row.after_number[0] + row.after_number[1]
            && row.before_flux_account >= 0 && row.before_flux_account <= 3
            && row.after_flux_account >= 0 && row.after_flux_account <= 3
            && row.account_divergence >= -4 && row.account_divergence <= 4
            && row.account_residual == 0, "invalid site credit/flux account");
        auto verified = row;
        finish(verified);
    }
    for (std::size_t i = 0; i < observation.edges.count; ++i) {
        const auto& edge = observation.edges.rows[i];
        require(edge.owner < L * L * L && edge.axis < 3
            && (i == 0 || std::tie(observation.edges.rows[i - 1].owner, observation.edges.rows[i - 1].axis)
                < std::tie(edge.owner, edge.axis)), "invalid observation edge key/order");
        const unsigned stride = edge.axis == 0 ? L * L : edge.axis == 1 ? L : 1;
        const unsigned coordinate = (edge.owner / stride) % L;
        const unsigned head = coordinate + 1 == L ? edge.owner - (L - 1) * stride : edge.owner + stride;
        require(edge.head == head && edge.before_flux >= -1 && edge.before_flux <= 1
            && edge.after_flux >= -1 && edge.after_flux <= 1
            && edge.account_current >= -1 && edge.account_current <= 1,
            "invalid observation edge geometry/account");
        for (auto current : edge.number_current)
            require(current >= -1 && current <= 1, "invalid observation number current");
        require(edge.signed_charge_current == edge.number_current[0] - edge.number_current[1]
            && edge.flux_residual == 0
            && edge.after_flux - edge.before_flux + edge.signed_charge_current == 0,
            "invalid observation flux residual");
    }
    auto derived = observation.sites;
    for (std::size_t i = 0; i < derived.count; ++i) {
        auto& row = derived.rows[i];
        row.number_divergence = {};
        row.account_divergence = row.before_flux_account = row.after_flux_account = 0;
    }
    auto existing = [&](std::uint32_t site) -> SiteBalance& {
        for (std::size_t i = 0; i < derived.count; ++i)
            if (derived.rows[i].site == site) return derived.rows[i];
        throw std::invalid_argument("observation omits edge endpoint support");
    };
    for (std::size_t i = 0; i < observation.edges.count; ++i) {
        const auto& edge = observation.edges.rows[i];
        auto& owner = existing(edge.owner);
        auto& head = existing(edge.head);
        owner.before_flux_account += edge.before_flux * edge.before_flux;
        owner.after_flux_account += edge.after_flux * edge.after_flux;
        for (unsigned p = 0; p < 2; ++p) {
            owner.number_divergence[p] += edge.number_current[p];
            head.number_divergence[p] -= edge.number_current[p];
        }
        owner.account_divergence += edge.account_current;
        head.account_divergence -= edge.account_current;
    }
    std::array<int, 2> before_totals{}, after_totals{};
    int before_account = 0, after_account = 0;
    for (std::size_t i = 0; i < derived.count; ++i) {
        const auto& row = observation.sites.rows[i];
        auto& actual = derived.rows[i];
        require(row.number_divergence == actual.number_divergence
            && row.account_divergence == actual.account_divergence
            && row.before_flux_account == actual.before_flux_account
            && row.after_flux_account == actual.after_flux_account,
            "observation rows disagree with oriented edge accounts");
        finish(actual);
        for (unsigned p = 0; p < 2; ++p) {
            before_totals[p] += actual.before_number[p];
            after_totals[p] += actual.after_number[p];
        }
        before_account += actual.before_credit + actual.before_flux_account;
        after_account += actual.after_credit + actual.after_flux_account;
    }
    require(before_totals == std::array<int, 2>{2, 2} && after_totals == before_totals
        && before_account == 4 && after_account == 4, "observation outside admitted Q4 account sector");
    BlockObservation out;
    out.context = observation.context;
    out.block_width = width;
    for (std::size_t i = 0; i < observation.sites.count; ++i) {
        const auto& row = observation.sites.rows[i];
        add_density(site_row(out.blocks, block_index(row.site, L, width)), row);
    }
    for (std::size_t i = 0; i < observation.edges.count; ++i) {
        const auto& edge = observation.edges.rows[i];
        const auto owner = block_index(edge.owner, L, width), head = block_index(edge.head, L, width);
        if (owner == head) continue;
        auto& boundary = out.boundary_currents.rows[out.boundary_currents.count++];
        boundary = {edge.owner, edge.head, owner, head, edge.axis, edge.number_current,
                    edge.signed_charge_current, edge.account_current};
        auto& a = site_row(out.blocks, owner);
        auto& b = site_row(out.blocks, head);
        for (unsigned p = 0; p < 2; ++p) {
            a.number_divergence[p] += edge.number_current[p];
            b.number_divergence[p] -= edge.number_current[p];
        }
        a.account_divergence += edge.account_current;
        b.account_divergence -= edge.account_current;
    }
    for (std::size_t i = 0; i < out.blocks.count; ++i) finish(out.blocks.rows[i]);
    std::sort(out.blocks.rows.begin(), out.blocks.rows.begin() + out.blocks.count,
        [](const auto& x, const auto& y) { return x.site < y.site; });
    return out;
}

AdvectiveMultiplier advective_multiplier(unsigned L,
    const std::array<std::int64_t, 3>& wavevector, const std::array<std::int64_t, 3>& translation) {
    require(L >= 4 && L <= 64 && L % 2 == 0, "Fourier torus outside candidate domain");
    std::int64_t residue = 0;
    for (unsigned axis = 0; axis < 3; ++axis)
        residue = (residue - (wavevector[axis] % L) * (translation[axis] % L)) % L;
    if (residue < 0) residue += L;
    return {unsigned(residue), L, false};
}
} // namespace ftd::carrier_current
