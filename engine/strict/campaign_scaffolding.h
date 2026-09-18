#pragma once
// Shared run scaffolding for the registered recovery campaign runners
// (recovery/, recovery_hydro/, recovery_mixed/).
//
// Mechanics only: manifest-row parsing, the SHA-256 record digest, the endpoint
// snapshot, and the publish-by-rename-on-completion trace staging. Nothing here
// observes, evolves, or interprets a state. Every registered constant (domain L,
// horizon, case count) stays in its own runner, and every diagnostic message is
// supplied by the caller, so a runner's observable behaviour is unchanged by
// routing these mechanics through this header.
#include "staged_runtime.h"

#include <openssl/sha.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ftd::strict::campaign {

// Lowercase, zero-padded hexadecimal SHA-256 of an encoded record.
inline std::string digest(const std::vector<std::uint8_t>& bytes) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(bytes.data(), bytes.size(), hash);
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (auto byte : hash) out << std::setw(2) << unsigned(byte);
    return out.str();
}

// Manifests may be authored with CRLF endings; a trailing CR is never data.
inline void strip_carriage_return(std::string& line) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
}

struct ManifestRow {
    std::string id, path;
};

// Splits the registered "caseID<TAB>path" row at its first tab. The caller owns
// the diagnostic because each campaign registers its own manifest contract.
inline ManifestRow split_manifest_row(const std::string& line, const char* message) {
    const auto separator = line.find('\t');
    if (separator == std::string::npos) throw std::runtime_error(message);
    return {line.substr(0, separator), line.substr(separator + 1)};
}

// The registered caseID alphabet: lowercase, digits and underscore, never empty.
inline void validate_case_id(const std::string& id) {
    if (id.empty() || id.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_") != std::string::npos)
        throw std::runtime_error("invalid registered caseID");
}

// A horizon argument is decimal digits only; the caller names what it counts.
inline unsigned long long parse_decimal_count(const std::string& text, const char* message) {
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
        throw std::runtime_error(message);
    return std::stoull(text);
}

// The endpoint record is published beside its preparation as "<preparation>.final.bin".
inline void write_endpoint_snapshot(const std::filesystem::path& preparation, const State& state) {
    auto final_path = preparation;
    final_path.replace_extension(".final.bin");
    std::ofstream final(final_path, std::ios::binary);
    const auto final_bytes = encode(state);
    final.write(reinterpret_cast<const char*>(final_bytes.data()), std::streamsize(final_bytes.size()));
    if (!final) throw std::runtime_error("endpoint snapshot write failed");
}

// A campaign writes TARGET.part and publishes TARGET by rename only once the run
// completes, so an interrupted campaign can never leave a trace that looks final.
inline std::filesystem::path partial_trace_path(const std::filesystem::path& target) {
    return std::filesystem::path(target.string() + ".part");
}

inline void refuse_existing_trace(const std::filesystem::path& target, const char* message) {
    if (std::filesystem::exists(target)) throw std::runtime_error(message);
}

inline std::ofstream open_partial_trace(const std::filesystem::path& target, const char* message) {
    std::ofstream output(partial_trace_path(target));
    if (!output) throw std::runtime_error(message);
    return output;
}

inline void publish_trace(std::ofstream& output, const std::filesystem::path& target) {
    output.close();
    if (!output) throw std::runtime_error("trace close failed");
    std::filesystem::rename(partial_trace_path(target), target);
}

}  // namespace ftd::strict::campaign
