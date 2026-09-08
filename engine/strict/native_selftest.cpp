#include "staged_runtime.h"
#include <iostream>
#include <limits>
#include <stdexcept>

int main() {
    try {
        ftd::strict::State state(3);
        state.microtick=std::numeric_limits<std::uint64_t>::max()-1;
        const auto before=ftd::strict::encode(state);
        bool rejected=false;
        try { ftd::strict::advance(state,2); } catch(const std::overflow_error&) { rejected=true; }
        if(!rejected||ftd::strict::encode(state)!=before) throw std::runtime_error("overflow atomicity failed");
        state.bank[0]=2;
        rejected=false;
        try { ftd::strict::step(state); } catch(const std::invalid_argument&) { rejected=true; }
        if(!rejected) throw std::runtime_error("Boolean validation failed");
        std::cout<<"strict native overflow atomicity and finite validation PASS\n";
        return 0;
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
