#include "ftd/ws_protocol.h"
#include "support/ws_loopback_fixture.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <vector>
namespace {
int checks=0,failures=0;
void check(bool ok,const char* label) { ++checks; if(!ok) { ++failures; std::cerr<<"FAIL "<<label<<'\n'; } }
}
int main() {
    using namespace ftd; ws_test::Platform platform;
    for(unsigned opcode=0;opcode<16;++opcode) {
        for(unsigned length:{0u,1u,2u,125u,126u,127u}) {
            std::uint8_t h[10]={static_cast<std::uint8_t>(0x80|opcode),static_cast<std::uint8_t>(0x80|length)};
            if(length==126) h[3]=126;
            if(length==127) h[7]=1; // 65536 in the 64-bit length.
            auto p=inspect_ws_client_frame_prefix(h,10);
            const bool known=opcode==WS_TEXT || opcode==WS_BINARY || opcode==WS_CLOSE || opcode==WS_PING || opcode==WS_PONG;
            const bool valid=known && (!(opcode&8) || length<=125) && !(opcode==WS_CLOSE && length==1);
            check((p.status==WsFramePrefixStatus::valid)==valid,"opcode/control boundary matrix");
        }
    }
    for(auto bytes:{std::size_t(0),std::size_t(125),std::size_t(126),std::size_t(65535),std::size_t(65536)}) {
        auto frame=ws_test::client_frame(WS_BINARY,bytes);
        auto p=inspect_ws_client_frame_prefix(frame.data(),frame.size());
        check(p.status==WsFramePrefixStatus::valid && p.payload_bytes==bytes && p.total_bytes==frame.size(),"canonical prefix size accounting");
        for(std::size_t n=0;n<p.prefix_bytes;++n) check(inspect_ws_client_frame_prefix(frame.data(),n).status==WsFramePrefixStatus::incomplete,"partial valid prefix needs more bytes");
        check(inspect_ws_client_frame_prefix(frame.data(),p.prefix_bytes).status==WsFramePrefixStatus::valid,"validation requires no payload allocation");
    }
    const std::vector<std::vector<std::uint8_t>> invalid{
        {0x81,0xfe,0,125},{0x82,0xff,0,0,0,0,0,0,255,255},
        {0x82,0xff,0,0,0,0,0,1,0,1},{0x82,0xff,0x80,0,0,0,0,0,0,0},
        {0x09,0x80},{0x89,0xfe},{0x8a,0xff},{0x88,0x81},{0x83,0x80},
        {0x80,0x80},{0xc1,0x80},{0x81,0x00}
    };
    for(const auto& h:invalid) {
        check(inspect_ws_client_frame_prefix(h.data(),h.size()).status==WsFramePrefixStatus::invalid,"invalid prefix rejected immediately");
        ws_test::Pair pair; WsIoPolicy policy; policy.read_timeout=std::chrono::milliseconds(200);
        WsIoScope scope(pair.server,policy);
        check(send_all(pair.client,h.data(),h.size()),"send invalid prefix without mask/payload");
        std::vector<std::uint8_t> payload{99};
        check(ws_read_frame(pair.server,payload)==0xff && payload.empty()
              && ws_last_io_status()==WsIoStatus::protocol_error,"actual frame parser rejects before awaiting mask/payload");
    }
    for(auto opcode:{WS_TEXT,WS_BINARY,WS_PING,WS_PONG,WS_CLOSE}) {
        const std::size_t size=opcode==WS_BINARY?65536:opcode==WS_CLOSE?2:125;
        auto frame=ws_test::client_frame(opcode,size); ws_test::Pair pair;
        bool sent=false; std::thread sender([&]{sent=send_all(pair.client,frame.data(),frame.size());});
        std::vector<std::uint8_t> payload; const auto result=ws_read_frame(pair.server,payload); sender.join();
        bool exact=payload.size()==size; for(std::size_t i=0;i<payload.size();++i) exact &= payload[i]==i%251;
        check(sent && result==opcode && exact && ws_last_io_status()==WsIoStatus::ok,"real loopback valid masked frame preserves payload");
    }
    {
        ws_test::Pair pair;
        for(auto opcode:{WS_PING,WS_PONG,WS_CLOSE}) {
            const std::uint8_t byte=0;
            check(!ws_send_frame(pair.server,opcode,&byte,126) && ws_last_io_status()==WsIoStatus::protocol_error,"server control oversize rejects before emission");
        }
        check(!ws_send_frame(pair.server,WS_CLOSE,"x",1),"server close length one rejects");
        check(!ws_send_frame(pair.server,0x03,nullptr,0),"server unsupported opcode rejects");
        check(ws_send_text(pair.server,"ok") && ws_send_frame(pair.server,WS_PONG,"p",1),"valid server frames still send in order");
        std::uint8_t wire[7]{}; const std::uint8_t expected[7]={0x81,2,'o','k',0x8a,1,'p'};
        check(recv_exact(pair.client,wire,sizeof(wire)) && std::equal(wire,wire+7,expected),"rejected frames emit no prefix and valid frame order remains intact");
    }
    check(inspect_ws_client_frame_prefix(nullptr,0).status==WsFramePrefixStatus::incomplete,"empty prefix safe");
    check(inspect_ws_client_frame_prefix(nullptr,1).status==WsFramePrefixStatus::invalid,"null nonempty prefix rejects");
    std::cout<<checks-failures<<'/'<<checks<<" checks passed\n"; return failures?1:0;
}
