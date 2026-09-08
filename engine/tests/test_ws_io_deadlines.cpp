#include "ftd/ws_protocol.h"
#include "support/ws_loopback_fixture.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
namespace {
using namespace std::chrono;
int checks=0,failures=0;
void check(bool ok,const char* label) { ++checks; if(!ok) { ++failures; std::cerr<<"FAIL "<<label<<'\n'; } }
ftd::WsIoPolicy policy() {
    ftd::WsIoPolicy p; p.handshake_timeout=milliseconds(160); p.read_timeout=milliseconds(160);
    p.write_timeout=milliseconds(160); p.poll_interval=milliseconds(5); return p;
}
bool bounded(steady_clock::time_point start) { const auto elapsed=steady_clock::now()-start; return elapsed>=milliseconds(100) && elapsed<seconds(2); }
}
int main() {
    using namespace ftd; ws_test::Platform platform;
    {
        ws_test::Pair pair; int pumped=0; WsIoScope scope(pair.server,policy(),[&]{++pumped;});
        char byte=0; auto start=steady_clock::now();
        check(!recv_exact(pair.server,&byte,1) && ws_last_io_status()==WsIoStatus::timeout,"idle receive has explicit absolute timeout");
        check(bounded(start) && pumped>0,"bounded socket wait pumps observations");
    }
    {
        ws_test::Pair pair; int pumped=0; WsIoScope scope(pair.server,policy(),[&]{++pumped;});
        std::atomic<bool> stop{false};
        std::thread sender([&]{const char byte='a'; for(int i=0;i<30 && !stop;++i) {if(!send_all(pair.client,&byte,1)) break; std::this_thread::sleep_for(milliseconds(25));}});
        char bytes[30]{}; auto start=steady_clock::now();
        const bool received=recv_exact(pair.server,bytes,sizeof(bytes)); const auto status=ws_last_io_status();
        stop=true; sender.join();
        check(!received && status==WsIoStatus::timeout && bounded(start),"slow drip does not restart the read deadline");
        check(bytes[0]=='a' && pumped>0,"partial progress and pumping occurred before deadline");
    }
    {
        ws_test::Pair pair; WsIoScope scope(pair.server,policy());
        const std::uint8_t prefix=0x81; send_all(pair.client,&prefix,1); pair.half_close_client();
        std::vector<std::uint8_t> payload{1}; const auto start=steady_clock::now();
        check(ws_read_frame(pair.server,payload)==0xff && ws_last_io_status()==WsIoStatus::disconnected && payload.empty(),"one-byte frame then half-close consumes EOF");
        check(steady_clock::now()-start<seconds(1),"partial EOF is not deferred until timeout");
    }
    {
        ws_test::Pair pair; WsIoScope scope(pair.server,policy());
        // One frame, three completed segments. Each arrives inside a hypothetical
        // per-segment timeout; the final payload exceeds the original deadline.
        std::thread sender([&]{
            std::this_thread::sleep_for(milliseconds(60)); const std::uint8_t head[2]={0x81,0x81}; send_all(pair.client,head,2);
            std::this_thread::sleep_for(milliseconds(60)); const std::uint8_t mask[4]={1,2,3,4}; send_all(pair.client,mask,4);
            std::this_thread::sleep_for(milliseconds(60)); const std::uint8_t payload='x'^1; send_all(pair.client,&payload,1);
        });
        auto start=steady_clock::now(); std::vector<std::uint8_t> payload;
        const auto result=ws_read_frame(pair.server,payload); const auto status=ws_last_io_status(); sender.join();
        check(result==0xff && status==WsIoStatus::timeout && payload.empty() && bounded(start),"frame header/mask/payload share original deadline");
    }
    {
        ws_test::Pair pair; int pumped=0; WsIoScope scope(pair.server,policy(),[&]{++pumped;});
        std::atomic<bool> stop{false};
        std::thread sender([&]{const std::string request="GET / HTTP/1.1\r\nhost: localhost\r\n"; for(char c:request) {if(stop || !send_all(pair.client,&c,1)) break; std::this_thread::sleep_for(milliseconds(20));}});
        auto start=steady_clock::now(); const bool ok=ws_handshake(pair.server); const auto status=ws_last_io_status(); stop=true; sender.join();
        check(!ok && status==WsIoStatus::timeout && bounded(start) && pumped>0,"slow HTTP header uses one handshake deadline");
    }
    {
        ws_test::Pair pair; WsIoScope scope(pair.server,policy());
        const std::string request="GET / HTTP/1.1\r\norigin: http://localhost:8080\r\nsec-websocket-key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
        check(send_all(pair.client,request.data(),request.size()) && ws_handshake(pair.server) && ws_last_io_status()==WsIoStatus::ok,"valid real TCP handshake succeeds under deadline policy");
        char prefix[12]{}; check(recv_exact(pair.client,prefix,sizeof(prefix)) && std::string(prefix,sizeof(prefix))=="HTTP/1.1 101","valid upgrade response preserved");
    }
    {
        ws_test::Pair pair(true); int pumped=0; WsIoScope scope(pair.server,policy(),[&]{++pumped;});
        const std::vector<std::uint8_t> payload(65536,0x55); auto start=steady_clock::now();
        // Winsock can accept one very large send wholesale despite SO_SNDBUF.
        // At most 128 reusable 64 KiB frames establish actual backpressure;
        // each operation retains its own deadline and the peer never reads.
        bool sent=true; int completed=0;
        for (;completed<128;++completed) { start=steady_clock::now(); sent=ws_send_binary(pair.server,payload); if(!sent) break; }
        std::cout << "nonreader sent=" << sent << " status=" << static_cast<int>(ws_last_io_status()) << " complete=" << completed << " pumps=" << pumped << '\n';
        check(!sent && ws_last_io_status()==WsIoStatus::timeout,"nonreading peer cannot block frame output indefinitely");
        check(bounded(start) && pumped>0,"partial output wait is bounded and pumps");
        check(!ws_send_text(pair.server,"later") && ws_last_io_status()==WsIoStatus::poisoned,"failed partial frame poisons subsequent writes");
        const std::size_t completed_bytes=static_cast<std::size_t>(completed)*(65536+10);
        std::vector<std::uint8_t> emitted(completed_bytes+2);
        check(completed<128 && recv_exact(pair.client,emitted.data(),emitted.size()) &&
              emitted[completed_bytes]==0x82 && emitted[completed_bytes+1]==127,
              "failed frame itself emitted a binary prefix after all completed frames");
    }
    {
        ws_test::Pair pair; bool nested_result=true;
        WsIoScope scope(pair.server,policy(),[&]{nested_result=ws_send_text(pair.server,"must not nest");});
        char byte=0; check(!recv_exact(pair.server,&byte,1) && !nested_result && ws_last_io_status()==WsIoStatus::reentrant_operation,"wait callback cannot enter a writer");
        // This nonblocking client has received nothing: a reentrant write must
        // reject before even the WebSocket header is emitted.
        send_all(pair.client,nullptr,0);
        check(::recv(pair.client,&byte,1,MSG_PEEK)<0,"reentrant writer emitted no bytes");
    }
    {
        ws_test::Pair pair(true);
        WsIoScope scope(pair.server,policy(),[]{throw std::runtime_error("telemetry callback failure");});
        bool thrown=false; int completed=0; const std::vector<std::uint8_t> payload(65536,0);
        try {for(;completed<128;++completed) if(!ws_send_binary(pair.server,payload)) break;} catch(const std::runtime_error& ex) {thrown=std::string(ex.what())=="telemetry callback failure";}
        check(thrown && ws_last_io_status()==WsIoStatus::system_error,"callback failure propagates with original message");
        bool rejected=false;
        try { rejected=!ws_send_text(pair.server,"later") && ws_last_io_status()==WsIoStatus::poisoned; } catch (...) {}
        check(rejected,"callback exception after partial send also poisons connection");
        const std::size_t completed_bytes=static_cast<std::size_t>(completed)*(65536+10);
        std::vector<std::uint8_t> emitted(completed_bytes+2);
        check(completed<128 && recv_exact(pair.client,emitted.data(),emitted.size()) &&
              emitted[completed_bytes]==0x82 && emitted[completed_bytes+1]==127,
              "callback failure interrupted a frame with its own prefix already emitted");
    }
    {
        ws_test::Pair pair; bool bad=false;
        auto invalid=policy(); invalid.read_timeout=milliseconds(0);
        try {WsIoScope scope(pair.server,invalid);} catch(const std::invalid_argument&) {bad=true;}
        check(bad,"zero deadline policy rejects before registration");
        WsIoScope scope(pair.server,policy()); bool nested=false;
        try {WsIoScope again(pair.client,policy());} catch(const std::logic_error&) {nested=true;}
        check(nested,"nested client scopes reject explicitly");
    }
    {
        ws_test::Pair pair; pair.close_client(); char byte=0;
        check(!recv_exact(pair.server,&byte,1) && ws_last_io_status()==WsIoStatus::disconnected,"unscoped helpers detect closed peer with finite defaults");
    }
    std::cout<<checks-failures<<'/'<<checks<<" checks passed\n"; return failures?1:0;
}
