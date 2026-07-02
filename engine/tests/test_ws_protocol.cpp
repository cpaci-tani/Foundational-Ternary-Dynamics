// ============================================================================
// test_ws_protocol.cpp — WebSocket remote-control protocol unit tests
// (revision 1.4). First automated coverage for the ws_server surface.
//
// Scope: the PURE parts of the wire protocol — SHA-1, base64, the RFC 6455
// Sec-WebSocket-Accept derivation (the exact computation ws_handshake
// performs, ws_protocol.cpp:81-86), and the string-search JSON helpers the
// command dispatcher relies on. Socket-coupled framing (ws_read_frame /
// ws_send_frame) needs a loopback pair harness — tracked as follow-up in
// the revision plan (Phase 5 coverage), not silently skipped.
// ============================================================================

#include "ftd/ws_sha1.h"
#include "ftd/ws_protocol.h"
#include "ftd/test_telemetry.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <string>

namespace ftd { namespace test {

static std::string sha1_hex(const std::string& msg) {
    SHA1 h;
    h.update(msg.data(), msg.size());
    auto d = h.final_hash();
    char buf[41];
    for (int i = 0; i < 20; ++i) std::snprintf(buf + 2 * i, 3, "%02x", d[i]);
    return std::string(buf, 40);
}

void test_sha1_fips_vectors() {
    section("SHA-1 FIPS 180-1 test vectors");
    check("SHA1(\"abc\")",
          sha1_hex("abc") == "a9993e364706816aba3e25717850c26c9cd0d89d",
          "FIPS vector 1 mismatch");
    check("SHA1(\"\")",
          sha1_hex("") == "da39a3ee5e6b4b0d3255bfef95601890afd80709",
          "empty-message vector mismatch");
    check("SHA1(\"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq\")",
          sha1_hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
              "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
          "FIPS vector 2 mismatch");
    // Multi-block (>64 bytes) message exercises the block loop.
    check("SHA1(1,000,000 x 'a')", [] {
        SHA1 h;
        std::string a(1000, 'a');
        for (int i = 0; i < 1000; ++i) h.update(a.data(), a.size());
        auto d = h.final_hash();
        char buf[41];
        for (int i = 0; i < 20; ++i) std::snprintf(buf + 2 * i, 3, "%02x", d[i]);
        return std::string(buf, 40) == "34aa973cd4c4daa4f61eeb2bdbad27316534016f";
    }(), "FIPS vector 3 (million-a) mismatch");
}

void test_base64_vectors() {
    section("base64 RFC 4648 test vectors");
    auto b64 = [](const std::string& s) {
        return base64_encode(reinterpret_cast<const uint8_t*>(s.data()), s.size());
    };
    check("b64(\"\") == \"\"",       b64("") == "",         "empty");
    check("b64(\"f\") == Zg==",      b64("f") == "Zg==",    "1-byte pad");
    check("b64(\"fo\") == Zm8=",     b64("fo") == "Zm8=",   "2-byte pad");
    check("b64(\"foo\") == Zm9v",    b64("foo") == "Zm9v",  "3-byte");
    check("b64(\"foobar\") == Zm9vYmFy", b64("foobar") == "Zm9vYmFy", "6-byte");
}

void test_rfc6455_accept_derivation() {
    section("RFC 6455 Sec-WebSocket-Accept derivation");
    // The canonical example from RFC 6455 section 1.3 / 4.2.2: the exact
    // computation ws_handshake performs (SHA1(key + GUID) -> base64).
    const std::string key  = "dGhlIHNhbXBsZSBub25jZQ==";
    const std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    SHA1 h;
    const std::string concat = key + guid;
    h.update(concat.data(), concat.size());
    auto hash = h.final_hash();
    const std::string accept = base64_encode(hash.data(), hash.size());
    std::printf("[ws] accept(\"%s\") = %s\n", key.c_str(), accept.c_str());
    check("accept key matches RFC 6455 canonical example",
          accept == "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=",
          "handshake derivation drifted from RFC 6455 — every browser "
          "client will refuse the upgrade");
}

void test_json_helpers() {
    section("string-search JSON helpers (command dispatch dependencies)");
    const std::string j =
        R"({"cmd":"set_toggle","name":"dual_substrate","value":true,"x":8,"rate":0.25,"off":false})";
    check("json_string extracts cmd", json_string(j, "cmd") == "set_toggle", "");
    check("json_string extracts name", json_string(j, "name") == "dual_substrate", "");
    check("json_number integer", json_number(j, "x") == 8.0, "");
    check("json_number fraction", json_number(j, "rate") == 0.25, "");
    check("json_bool true",  json_bool(j, "value") == true, "");
    check("json_bool false", json_bool(j, "off") == false, "");
    check("json_string missing key is empty", json_string(j, "nope").empty(), "");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_ws_protocol");
    ftd::test::test_sha1_fips_vectors();
    ftd::test::test_base64_vectors();
    ftd::test::test_rfc6455_accept_derivation();
    ftd::test::test_json_helpers();
    return ftd::test::finalize();
}
