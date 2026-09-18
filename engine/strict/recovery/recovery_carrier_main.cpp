// Registered observations of accepted CUDA evolution. No kernel/law changes.
#include "campaign_scaffolding.h"
#include "staged_cuda.h"
#include "frozen_tables.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace {
using namespace ftd::strict;
void observe(std::ostream& out,const State& st) {
    out<<"{\"microtick\":\""<<st.microtick<<"\",\"sha256\":\""<<campaign::digest(encode(st))
       <<"\",\"L\":"<<st.L<<",\"work\":"<<work_units(st)<<",\"field\":[";
    bool first=true;
    for(std::size_t x=0;x<st.s.size();++x)for(unsigned c=0;c<384;++c)if(st.bank[x*384+c]) {
        if(!first)out<<',';first=false;
        out<<'['<<x<<','<<c<<','<<unsigned(st.ell[x])<<','<<int(st.s[x])<<']';
    }
    out<<"],\"relation\":[";first=true;
    for(unsigned kind=0;kind<2;++kind) {
        const auto& records=kind?st.fcc:st.sc;
        const auto& gates=kind?st.gate_fcc:st.gate_sc;
        unsigned orientations=kind?6:3;
        for(std::size_t x=0;x<st.s.size();++x)for(unsigned ori=0;ori<orientations;++ori)
            for(unsigned slot=0;slot<2;++slot) {
                auto token=records[(x*orientations+ori)*2+slot];
                if(token==frozen::BLANK)continue;
                if(!first)out<<',';first=false;
                out<<"[\""<<(kind?"fcc":"sc")<<"\","<<x<<','<<ori<<','<<slot<<','<<unsigned(token)
                   <<','<<unsigned(gates[x*orientations+ori])<<','<<(kind?0:unsigned(st.admitted_sc[x*3+ori]))<<']';
            }
    }
    out<<"]}";
}
} // namespace

int main(int argc,char** argv) {
    try {
        if(argc!=4)throw std::runtime_error("usage: carrier_campaign MANIFEST_TSV TRACE_JSONL TICKS");
        auto ticks=campaign::parse_decimal_count(argv[3],"invalid tick count");
        if(ticks!=64)throw std::runtime_error("registered carrier horizon is exactly64 physical ticks");
        std::ifstream manifest(argv[1]);if(!manifest)throw std::runtime_error("cannot open manifest");
        const std::filesystem::path target(argv[2]);
        campaign::refuse_existing_trace(target,"refuse to overwrite completed campaign trace");
        auto output=campaign::open_partial_trace(target,"cannot open trace");
        std::cerr<<gpu::device_json()<<'\n';
        unsigned cases=0;std::string line;
        while(std::getline(manifest,line)) {
            campaign::strip_carriage_return(line);
            const auto [id,path]=campaign::split_manifest_row(line,"manifest must contain caseID<TAB>binary path");
            campaign::validate_case_id(id);
            std::ifstream input(path,std::ios::binary);if(!input)throw std::runtime_error("cannot read preparation");
            std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});
            auto state=decode(bytes);
            if(state.L!=9||state.microtick!=0)throw std::runtime_error("unregistered domain/preparation tick");
            output<<"{\"case_id\":\""<<id<<"\",\"states\":[";observe(output,state);
            std::vector<Events> events;
            for(std::uint64_t tick=0;tick<ticks;++tick) {
                auto rows=gpu::advance(state,1);events.push_back(std::move(rows.front()));
                output<<',';observe(output,state);
            }
            output<<"],\"events\":"<<events_json(events)<<"}\n";
            if(!output)throw std::runtime_error("trace write failed");
            campaign::write_endpoint_snapshot(path,state);
            if(++cases%50==0)std::cerr<<"completed_cases="<<cases<<'\n';
        }
        if(cases!=600)throw std::runtime_error("registered matrix must contain exactly600 cases");
        campaign::publish_trace(output,target);
        std::cerr<<"completed_cases="<<cases<<" microticks="<<cases*ticks<<'\n';
        return 0;
    } catch(const std::exception& e) {std::cerr<<"carrier campaign failed: "<<e.what()<<'\n';return 1;}
}
