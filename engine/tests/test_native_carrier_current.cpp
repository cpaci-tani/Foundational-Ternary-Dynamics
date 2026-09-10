// Bounded exact observation gate; reuses the frozen native Q4 transition.
// No particle catalogue, continuum fit, or large quotient campaign.
#include "carrier_observer.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace Q = ftd::q4native_v1;
namespace O = ftd::carrier_current;
namespace {
using Point = std::array<std::int64_t, 3>;
using Bytes = std::vector<std::uint8_t>;
constexpr unsigned axes[6][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
std::uint64_t checks = 0, transactions = 0, seam_currents = 0, credit_exchanges = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
template<class Function> void rejects(Function operation, const char* message) {
    bool rejected = false;
    try { operation(); } catch (const std::exception&) { rejected = true; }
    check(rejected, message);
}
Point direction(unsigned h) {
    Point v{}; v[h / 2] = h % 2 ? -1 : 1; return v;
}
Point add(Point a, const Point& b) { for (unsigned j = 0; j < 3; ++j) a[j] += b[j]; return a; }
Point scale(Point a, int m) { for (auto& x : a) x *= m; return a; }
std::uint32_t site(Point p, unsigned L = 16) {
    for (auto& x : p) { x %= L; if (x < 0) x += L; }
    return std::uint32_t((p[0] * L + p[1]) * L + p[2]);
}
Point xyz(std::uint32_t x, unsigned L = 16) { return {x / (L * L), (x / L) % L, x % L}; }
unsigned physical_heading(unsigned heading, unsigned permutation) {
    return 2 * axes[permutation][heading / 2] + heading % 2;
}
bool carrier_less(const Q::Carrier& a, const Q::Carrier& b) { return std::tie(a.site,a.slot) < std::tie(b.site,b.slot); }
bool edge_less(const Q::Edge& a, const Q::Edge& b) { return std::tie(a.owner,a.axis) < std::tie(b.owner,b.axis); }
bool same_mobile(const Q::Payload& a, const Q::Payload& b) {
    if (a.L != b.L || a.eta != b.eta || a.origin_code != b.origin_code || a.edge_count != b.edge_count) return false;
    for (unsigned i = 0; i < 4; ++i) {
        const auto& x = a.carriers[i]; const auto& y = b.carriers[i];
        if (std::tie(x.site,x.slot,x.direction,x.credit,x.attempted)
            != std::tie(y.site,y.slot,y.direction,y.credit,y.attempted)) return false;
        const auto& e = a.edges[i]; const auto& f = b.edges[i];
        if (std::tie(e.owner,e.axis,e.q) != std::tie(f.owner,f.axis,f.q)) return false;
    }
    return true;
}
Q::Payload translated(Q::Payload p, Point d) {
    for (auto& c : p.carriers) c.site = site(add(xyz(c.site,p.L),d),p.L);
    for (unsigned i = 0; i < p.edge_count; ++i) p.edges[i].owner = site(add(xyz(p.edges[i].owner,p.L),d),p.L);
    std::sort(p.carriers.begin(),p.carriers.end(),carrier_less);
    std::sort(p.edges.begin(),p.edges.begin()+p.edge_count,edge_less);
    return p;
}
bool translated_background(const Q::State& a, const Q::State& b, Point d) {
    // The reconstructed background has period two on each axis, so these
    // eight values exhaust its spatial alphabet without a volume allocation.
    for (unsigned x = 0; x < 2; ++x) for (unsigned y = 0; y < 2; ++y) for (unsigned z = 0; z < 2; ++z) {
        const Point p{std::int64_t(x),std::int64_t(y),std::int64_t(z)};
        if (Q::background_code(a,site(p)) != Q::background_code(b,site(add(p,d)))) return false;
    }
    return true;
}
struct Lift { std::array<Point,4> positions{}; std::array<unsigned,4> component{}; };
struct Prepared { Q::State state; Lift lift; Point heading; };

Prepared prepare(unsigned mode, unsigned h, unsigned order = 0, unsigned eta = 0,
                 unsigned permutation = 0, unsigned origin_bits = 0, bool seam = false) {
    check(mode <= 3 && h < 6 && order < 2 && eta < 2 && permutation < 6 && origin_bits < 8,
          "native preparation parameters in frozen domain");
    const unsigned t = 2 * ((h / 2 + 1) % 3), ph = physical_heading(h,permutation);
    std::vector<unsigned> path = mode == 0 ? std::vector<unsigned>{h}
        : mode == 1 ? std::vector<unsigned>{h,h} : std::vector<unsigned>{h,t};
    std::array<unsigned,2> heading = mode == 0 ? std::array<unsigned,2>{h,h}
        : mode == 1 ? std::array<unsigned,2>{h^1,h}
        : mode == 2 ? std::array<unsigned,2>{h^1,t} : std::array<unsigned,2>{t^1,h};
    std::array<unsigned,2> credit{mode == 0 ? 1U : 0U,0};
    Point anchor{4,4,4};
    if (seam) anchor[ph/2] = ph%2 ? 0 : 15;
    Point end = anchor;
    for (auto d : path) end = add(end,direction(physical_heading(d,permutation)));
    if (order) {
        anchor = end;
        std::reverse(path.begin(),path.end());
        for (auto& d : path) d ^= 1;
        std::swap(heading[0],heading[1]); std::swap(credit[0],credit[1]);
    }
    Q::Payload payload{}; payload.L = 16; payload.origin_code = std::uint8_t(8*permutation+origin_bits); payload.eta = std::uint8_t(eta);
    struct Row { Q::Carrier carrier; Point point; unsigned component; };
    std::array<Row,4> rows{};
    for (unsigned copy = 0; copy < 2; ++copy) {
        Point plus = anchor; plus[(ph/2+1)%3] += 8*copy;
        Point minus = plus;
        for (auto d : path) {
            const auto v = direction(physical_heading(d,permutation));
            const unsigned axis = physical_heading(d,permutation)/2;
            const auto next = add(minus,v);
            const int sign = v[axis] > 0 ? 1 : -1;
            payload.edges[payload.edge_count++] = {site(sign==1?minus:next),std::uint8_t(axis),std::int8_t(sign*(eta?-1:1))};
            minus = next;
        }
        for (unsigned endpoint = 0; endpoint < 2; ++endpoint) {
            const auto p = endpoint ? minus : plus;
            rows[2*copy+endpoint] = {{site(p),std::uint8_t(endpoint^eta),
                std::uint8_t(physical_heading(heading[endpoint],permutation)+1),std::uint8_t(credit[endpoint]),0},p,copy};
        }
    }
    std::sort(rows.begin(),rows.end(),[](const auto& a,const auto& b){return carrier_less(a.carrier,b.carrier);});
    Lift lift;
    for (unsigned i = 0; i < 4; ++i) { payload.carriers[i] = rows[i].carrier; lift.positions[i] = rows[i].point; lift.component[i] = rows[i].component; }
    std::sort(payload.edges.begin(),payload.edges.begin()+payload.edge_count,edge_less);
    return {Q::admit(payload,"0"),lift,direction(ph)};
}

Lift advance_lift(const Q::State& before, const Q::State& after, const Q::Events& events, Lift lift) {
    auto records = Q::payload(before).carriers;
    for (unsigned j = 0; j < events.moves.count; ++j) {
        const auto& r = events.moves.rows[j];
        const unsigned slot = r[0] == 1 ? 0 : 1;
        unsigned i = 0;
        while (i < 4 && !(records[i].site == std::uint32_t(r[1]) && records[i].slot == slot)) ++i;
        check(i < 4,"lift source record exists");
        const auto v = direction(2*unsigned(r[4]) + (r[1] == r[3] ? 0 : 1));
        lift.positions[i] = add(lift.positions[i],v);
        records[i].site = std::uint32_t(r[2]);
    }
    Lift result;
    for (unsigned j = 0; j < 4; ++j) {
        const auto& c = Q::payload(after).carriers[j]; unsigned i = 0;
        while (i < 4 && !(records[i].site == c.site && records[i].slot == c.slot)) ++i;
        check(i < 4 && site(lift.positions[i],Q::payload(after).L) == c.site,"unwrapped position matches complete endpoint");
        result.positions[j] = lift.positions[i]; result.component[j] = lift.component[i];
    }
    return result;
}

void separated(const Q::State& state, const Lift& lift) {
    const auto& p = Q::payload(state);
    std::map<std::uint32_t,unsigned> labels;
    for (unsigned i = 0; i < 4; ++i) {
        const auto inserted = labels.emplace(p.carriers[i].site,lift.component[i]);
        check(inserted.second || inserted.first->second == lift.component[i],"independent copies do not overlap");
    }
    for (unsigned pass = 0; pass < 4; ++pass) for (unsigned i = 0; i < p.edge_count; ++i) {
        const auto& e = p.edges[i]; const auto head = Q::shift_site(state,e.owner,e.axis,1);
        const auto a = labels.find(e.owner), b = labels.find(head);
        if (a != labels.end() && b != labels.end()) check(a->second == b->second,"flux edge does not join independent components");
        else if (a != labels.end()) labels.emplace(head,a->second);
        else if (b != labels.end()) labels.emplace(e.owner,b->second);
    }
    for (unsigned i = 0; i < p.edge_count; ++i)
        check(labels.count(p.edges[i].owner) != 0,"all flux support belongs to a witnessed component");
    for (const auto& a : labels) for (const auto& b : labels) if (a.second != b.second) {
        const auto x = xyz(a.first), y = xyz(b.first); std::int64_t distance = 0;
        for (unsigned j = 0; j < 3; ++j) {
            const auto delta = std::abs(x[j]-y[j]); distance = std::max(distance,std::min(delta,16-delta));
        }
        check(distance > 4,"radius-one transaction dependency neighborhoods remain disjoint");
    }
}

std::array<std::int64_t,3> moment(const Q::Payload& p,const Lift& lift,unsigned slot) {
    Point sum{}; for (unsigned i = 0; i < 4; ++i) if (p.carriers[i].slot == slot) sum = add(sum,lift.positions[i]); return sum;
}
std::array<int,6> block_totals(const O::BlockObservation& block) {
    std::array<int,6> result{};
    for (std::size_t i = 0; i < block.blocks.count; ++i) {
        const auto& r = block.blocks.rows[i];
        for (unsigned p = 0; p < 2; ++p) { result[p] += r.before_number[p]; result[p+2] += r.after_number[p]; }
        result[4] += r.before_credit+r.before_flux_account; result[5] += r.after_credit+r.after_flux_account;
    }
    return result;
}
O::Observation verify(const Q::State& before, const Q::StepResult& result) {
    ++transactions;
    const auto bytes = Q::encode_state(before), event_bytes = Q::encode_events(result.events);
    const auto observation = O::observe(before,result.state,result.events);
    check(Q::encode_state(before) == bytes && Q::encode_events(result.events) == event_bytes,"observer input immutability");
    check(observation.context.start_tick_hex == Q::microtick_hex(before)
        && observation.context.end_tick_hex == Q::microtick_hex(result.state),"exact observation tick interval");
    check(!observation.context.mechanical_energy_available && !observation.context.mechanical_momentum_available
        && !observation.context.spin_available && !observation.context.rest_mass_available
        && !observation.context.particle_identification_available && !observation.complete_transition_law_validated,
        "unrecovered physical quantities unavailable");
    for (unsigned width : {1U,2U,4U,8U,16U}) {
        const auto block = O::restrict_blocks(observation,width);
        check(block_totals(block) == std::array<int,6>{2,2,2,2,4,4},"all block resolutions conserve exact totals");
        std::array<int,2> divergence{}; int account = 0;
        for (std::size_t i = 0; i < block.blocks.count; ++i) {
            const auto& r = block.blocks.rows[i];
            for (unsigned p = 0; p < 2; ++p) { divergence[p] += r.number_divergence[p]; check(r.number_residual[p] == 0,"block number balance"); }
            account += r.account_divergence; check(r.account_residual == 0,"block account balance");
        }
        check(divergence == std::array<int,2>{0,0} && account == 0,"periodic boundary currents cancel exactly");
        if (width == 16) check(block.boundary_currents.count == 0,"whole-domain internal faces cancel");
    }
    const auto two = O::restrict_blocks(observation,2), four = O::restrict_blocks(observation,4);
    for (std::size_t j = 0; j < four.blocks.count; ++j) {
        const auto& r = four.blocks.rows[j]; std::array<int,6> sums{};
        for (std::size_t i = 0; i < two.blocks.count; ++i) {
            const auto& q = two.blocks.rows[i]; const auto c = xyz(q.site,8);
            const auto coarse = site({c[0]/2,c[1]/2,c[2]/2},4);
            if (coarse != r.site) continue;
            for (unsigned p=0;p<2;++p) { sums[p]+=q.before_number[p]; sums[p+2]+=q.after_number[p]; }
            sums[4]+=q.before_credit+q.before_flux_account; sums[5]+=q.after_credit+q.after_flux_account;
        }
        check(sums == std::array<int,6>{r.before_number[0],r.before_number[1],r.after_number[0],r.after_number[1],
             r.before_credit+r.before_flux_account,r.after_credit+r.after_flux_account},"nested spatial density restriction is exact");
    }
    for (std::size_t i=0;i<observation.edges.count;++i) {
        const auto& e=observation.edges.rows[i]; check(e.flux_residual==0,"signed current exactly updates owned flux");
        if ((e.number_current[0] || e.number_current[1]) && std::abs(xyz(e.owner)[e.axis]-xyz(e.head)[e.axis])==15) ++seam_currents;
    }
    credit_exchanges += result.events.credit_exchanges.count;
    const auto restored = Q::decode_state(bytes.data(),bytes.size());
    const auto replay = Q::step(restored);
    check(Q::encode_state(replay.state)==Q::encode_state(result.state)
        && Q::encode_events(replay.events)==event_bytes,"complete checkpoint continuation at every reached phase");
    return observation;
}

void translating_controls() {
    for (unsigned h=0;h<6;++h) for (unsigned order=0;order<2;++order)
    for (unsigned eta=0;eta<2;++eta) for (unsigned permutation=0;permutation<6;++permutation) {
        auto prep=prepare(0,h,order,eta,permutation,0,true);
        auto reset=Q::step(prep.state); verify(prep.state,reset);
        auto lift=advance_lift(prep.state,reset.state,reset.events,prep.lift);
        auto state=reset.state; const auto start=state; const auto initial_lift=lift;
        std::array<Point,2> integrated{};
        for (unsigned tick=1;tick<=38;++tick) {
            const auto result=Q::step(state); const auto observation=verify(state,result);
            for (std::size_t i=0;i<observation.edges.count;++i) {
                const auto& e=observation.edges.rows[i];
                for (unsigned p=0;p<2;++p) integrated[p][e.axis]+=e.number_current[p];
            }
            lift=advance_lift(state,result.state,result.events,lift); state=result.state; separated(state,lift);
            if (tick==19 || tick==38) {
                const auto displacement=scale(prep.heading,int(tick/19));
                check(same_mobile(translated(Q::payload(start),displacement),Q::payload(state)),"all mobile records have prospective translation");
                check(translated_background(start,state,displacement)==(tick==38),"odd translation excludes copied background; even translation restores it");
            }
        }
        check(Q::phase(state)==Q::phase(start) && Q::microtick_hex(state)=="27", "38 physical ticks recur in phase and retain advanced ordinal");
        check(Q::encode_state(state)!=Q::encode_state(start),"spatial recurrence never implies checkpoint equality");
        for (unsigned p=0;p<2;++p) {
            const auto before=moment(Q::payload(start),initial_lift,p), after=moment(Q::payload(state),lift,p);
            check(integrated[p]==scale(prep.heading,4) && add(before,integrated[p])==after,
                  "unwrapped per-polarity displacement equals integrated microscopic current");
        }
        for (const Point k : {Point{0,0,0},Point{1,0,0},Point{1,2,-3}}) {
            const auto value=O::advective_multiplier(16,k,scale(prep.heading,2));
            std::int64_t dot=0;for(unsigned a=0;a<3;++a)dot+=k[a]*2*prep.heading[a];
            check(value.residue==unsigned(((-dot%16)+16)%16) && value.denominator==16
                && !value.rest_mass_dispersion_available,"exact finite-torus advective phase, no rest-mass identification");
        }
    }
    check(seam_currents>0,"registered controls include signed boundary crossings");
}

std::int64_t separation(const Lift& lift,unsigned copy) {
    Point a{},b{}; unsigned count=0;
    for(unsigned i=0;i<4;++i)if(lift.component[i]==copy){if(count++==0)a=lift.positions[i];else b=lift.positions[i];}
    check(count==2,"two constituents per prepared component");
    std::int64_t distance=0;for(unsigned j=0;j<3;++j)distance+=std::abs(a[j]-b[j]);return distance;
}
bool in_translating_family(const Q::State& state,const Lift& lift,unsigned copy) {
    unsigned first=4,second=4;
    for(unsigned i=0;i<4;++i)if(lift.component[i]==copy){if(first==4)first=i;else second=i;}
    const auto& a=Q::payload(state).carriers[first]; const auto& b=Q::payload(state).carriers[second];
    if(Q::phase(state)!=1 || separation(lift,copy)!=1 || a.attempted || b.attempted
       || a.direction!=b.direction || a.credit+b.credit!=1)return false;
    const auto donor=a.credit?first:second, recipient=a.credit?second:first;
    return add(lift.positions[donor],direction(a.direction-1))==lift.positions[recipient];
}
void restoration_controls() {
    for(unsigned mode=1;mode<=3;++mode)for(unsigned h=0;h<6;++h)
    for(unsigned order=0;order<2;++order)for(unsigned eta=0;eta<2;++eta){
        auto prep=prepare(mode,h,order,eta);auto reset=Q::step(prep.state);verify(prep.state,reset);
        auto lift=advance_lift(prep.state,reset.state,reset.events,prep.lift);auto state=reset.state;
        std::array<bool,2> restored{};
        for(unsigned tick=1;tick<=76;++tick){
            const auto result=Q::step(state);verify(state,result);lift=advance_lift(state,result.state,result.events,lift);state=result.state;
            separated(state,lift);
            if(tick<=38)for(unsigned copy=0;copy<2;++copy)restored[copy]=restored[copy]||separation(lift,copy)<=1;
        }
        check(restored[0]&&restored[1],"extended preparations restore adjacency by38 physical ticks");
        check(in_translating_family(state,lift,0)&&in_translating_family(state,lift,1),"extended preparations enter translating family by76 physical ticks");
    }
    check(credit_exchanges>0,"restoration controls exercise explicit credit exchange");
}

void negative_controls() {
    auto prep=prepare(0,1);auto state=prep.state;
    for(unsigned tick=0;tick<38;++tick){
        const auto result=Q::step(state);const auto observation=O::observe(state,result.state,result.events);
        if(result.events.moves.count==2){
            auto corrupt=result.events;std::swap(corrupt.moves.rows[0][2],corrupt.moves.rows[1][2]);
            rejects([&]{O::observe(state,result.state,corrupt);},"far swapped destinations cannot masquerade as local moves");
            corrupt=result.events;corrupt.moves.count=0;corrupt.attempt_marks.count=0;
            rejects([&]{O::observe(state,result.state,corrupt);},"missing movements rejected by endpoint account");
            corrupt=result.events;corrupt.moves.rows[0][7]^=1;
            rejects([&]{O::observe(state,result.state,corrupt);},"corrupt credit event rejected");
            rejects([&]{O::restrict_blocks(observation,3);},"invalid block divisor rejected");
            auto forged=observation;forged.sites.rows[0].before_credit=std::numeric_limits<std::int32_t>::max();
            rejects([&]{O::restrict_blocks(forged,16);},"forged account cannot overflow restriction arithmetic");
            forged=observation;forged.edges.rows[0].head=forged.edges.rows[0].owner;
            rejects([&]{O::restrict_blocks(forged,16);},"forged edge geometry rejected before internal cancellation");
            forged=observation;forged.context.mechanical_energy_available=true;
            rejects([&]{O::restrict_blocks(forged,16);},"physical capability promotion rejected");
            forged=observation;forged.context.start_tick_hex="g0";forged.context.end_tick_hex="g1";
            rejects([&]{O::restrict_blocks(forged,16);},"invalid ordinal prefix rejected");
            auto bad_context=Q::payload(result.state);bad_context.origin_code^=1;
            const auto wrong=Q::admit(bad_context,Q::microtick_hex(result.state));
            rejects([&]{O::observe(state,wrong,result.events);},"background change rejected");
            rejects([&]{O::observe(state,state,result.events);},"nonconsecutive checkpoint rejected");
            // Same exact density/flux/credit preparation, changed headings only.
            auto changed=Q::payload(state);
            for(auto& c:changed.carriers)c.direction=std::uint8_t(((c.direction-1)^1)+1);
            const auto other=Q::admit(changed,Q::microtick_hex(state));const auto other_step=Q::step(other);
            const auto other_observation=O::observe(other,other_step.state,other_step.events);
            check(Q::payload(other).edge_count==Q::payload(state).edge_count,"closure control retains flux support");
            const auto a=O::restrict_blocks(observation,1),b=O::restrict_blocks(other_observation,1);
            check(a.blocks.count==b.blocks.count || Q::encode_state(result.state)!=Q::encode_state(other_step.state),"closure supports may differ after evolution");
            for(const auto& c:Q::payload(state).carriers){
                auto found=std::find_if(changed.carriers.begin(),changed.carriers.end(),[&](const auto& d){return c.site==d.site&&c.slot==d.slot;});
                check(found!=changed.carriers.end()&&c.credit==found->credit,"identical initial number and credit observations");
            }
            check(Q::encode_events(result.events)!=Q::encode_events(other_step.events)
                && !same_mobile(Q::payload(result.state),Q::payload(other_step.state)),"density/flux/credit observations do not autonomously determine currents");
            break;
        }
        state=result.state;
        check(tick!=37,"found two reverse moves for corruption control");
    }
    auto huge=Q::admit(Q::payload(prep.state),"ffffffffffffffffffffffffffffffff");
    const auto huge_step=Q::step(huge);verify(huge,huge_step);
    check(Q::microtick_hex(huge_step.state)=="100000000000000000000000000000000","arbitrary-length ordinal carry remains exact");
    const auto extreme=O::advective_multiplier(16,{std::numeric_limits<std::int64_t>::min(),1,0},
        {std::numeric_limits<std::int64_t>::max(),2,0});
    check(extreme.residue==14,"Fourier modular multiplication avoids int64 overflow");
    rejects([&]{O::advective_multiplier(0,{0,0,0},{0,0,0});},"invalid Fourier domain rejected");
}

unsigned decimal(const std::string& text,unsigned limit) {
    check(!text.empty()&&(text.size()==1||text.front()!='0'),"fixture canonical decimal");
    unsigned value=0;for(char c:text){check(c>='0'&&c<='9',"fixture decimal alphabet");const unsigned d=unsigned(c-'0');check(value<=(limit-d)/10&&d<=limit,"fixture decimal bound");value=value*10+d;}
    check(value<=limit,"fixture decimal maximum");return value;
}
Bytes unhex(const std::string& text) {
    check(!text.empty()&&text.size()%2==0&&text.size()<=20U*1024*1024,"fixture hex frame shape/cap");
    Bytes result;result.reserve(text.size()/2);
    auto digit=[](char c){check((c>='0'&&c<='9')||(c>='a'&&c<='f'),"fixture lowercase hexadecimal");return c<='9'?unsigned(c-'0'):unsigned(c-'a'+10);};
    for(std::size_t i=0;i<text.size();i+=2)result.push_back(std::uint8_t(16*digit(text[i])+digit(text[i+1])));
    return result;
}
std::string line(std::ifstream& input) {
    std::string text;check(bool(std::getline(input,text)),"truncated fixture stream");
    check(!text.empty()&&text.find_first_of("\r\t") == std::string::npos,"fixture ASCII line grammar");
    for(unsigned char c:text)check(c>=32&&c<=126,"fixture printable ASCII");return text;
}
int fixtures(const std::string& path) {
    std::ifstream input(path,std::ios::binary);check(bool(input),"fixture file open");input.seekg(0,std::ios::end);
    const auto size=input.tellg();check(size>=0&&size<=10*1024*1024,"fixture file10MiB cap");input.seekg(0);
    check(line(input)=="FTD-CARRIER-CURRENT-FIXTURES-1","fixture magic");
    const unsigned cases=decimal(line(input),256);check(cases>0,"fixture requires cases");
    std::uint64_t total=0;std::map<std::string,bool> ids;
    for(unsigned c=0;c<cases;++c){
        const auto header=line(input);const auto first=header.find(' '),last=header.rfind(' ');
        check(first==4&&last>first&&header.substr(0,4)=="CASE"&&header.find(' ',first+1)==last,"fixture CASE grammar");
        const auto id=header.substr(first+1,last-first-1);check(!id.empty()&&id.size()<=128,"fixture case identifier length");
        for(char x:id)check((x>='a'&&x<='z')||(x>='A'&&x<='Z')||(x>='0'&&x<='9')||x=='_'||x=='-',"fixture case identifier alphabet");
        check(ids.emplace(id,true).second,"fixture case identifiers unique");
        const auto steps=decimal(header.substr(last+1),96);check(steps>0,"fixture requires transitions");
        const auto initial=unhex(line(input));auto state=Q::decode_state(initial.data(),initial.size());
        check(Q::payload(state).L==16,"fixed fixture lattice size");check(Q::encode_state(state)==initial,"canonical initial fixture wire");
        for(unsigned tick=0;tick<steps;++tick){
            const auto record=line(input);const auto separator=record.find(' ');
            check(separator!=std::string::npos&&record.find(' ',separator+1)==std::string::npos,"fixture step has two frames");
            const auto expected_state=unhex(record.substr(0,separator)),expected_events=unhex(record.substr(separator+1));
            const auto result=Q::step(state);
            check(Q::encode_state(result.state)==expected_state,"native successor differs from independent fixture");
            check(Q::encode_events(result.events)==expected_events,"native complete events differ from independent fixture");
            verify(state,result);state=result.state;++total;
        }
    }
    check(input.peek()==std::char_traits<char>::eof(),"fixture trailing bytes rejected");
    std::cout<<"FTD_CARRIER_CURRENT_FIXTURES_PASS cases="<<cases<<" transactions="<<total<<'\n';return 0;
}
} // namespace

int main(int argc,char** argv) {
    try {
        if(argc==3&&std::string(argv[1])=="--fixtures")return fixtures(argv[2]);
        check(argc==1,"usage: test_native_carrier_current [--fixtures PATH]");
        translating_controls();restoration_controls();negative_controls();
        std::cout<<"native_carrier_current PASS checks="<<checks<<" transactions="<<transactions
                 <<" seam_currents="<<seam_currents<<" credit_exchanges="<<credit_exchanges<<'\n'
                 <<"scope=exact candidate number/current/credit-flux accounting; physical mass, momentum, spin and SM recovery unavailable\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"native_carrier_current FAIL: "<<error.what()<<'\n';return 1;
    }
}
