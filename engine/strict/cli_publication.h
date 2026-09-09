#pragma once
// File publication only: no kernel, clock, or wire-format changes.
// Requires exclusive output ownership. Ordinary exceptions roll back the pair;
// process crashes and concurrent writers are not a multi-file transaction.
#include <chrono>
#include <atomic>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ftd::strict::cli {
namespace fs = std::filesystem;

inline void validate_paths(const fs::path& input, const fs::path& output, const fs::path& events) {
    const fs::path paths[] = {fs::weakly_canonical(fs::absolute(input)),
                             fs::weakly_canonical(fs::absolute(output)),
                             fs::weakly_canonical(fs::absolute(events))};
    if (!fs::is_regular_file(paths[0])) throw std::runtime_error("input must be a regular file");
    for (const auto& path : {output, events}) {
        if (fs::is_symlink(fs::symlink_status(path)))
            throw std::runtime_error("output symlinks are unsupported");
        const auto absolute = fs::weakly_canonical(fs::absolute(path));
        if (!fs::is_directory(absolute.parent_path()) || (fs::exists(absolute) && !fs::is_regular_file(absolute)))
            throw std::runtime_error("output parent must exist and output must be a regular file");
    }
    for (unsigned i = 0; i < 3; ++i) for (unsigned j = i + 1; j < 3; ++j)
        if (paths[i] == paths[j] || (fs::exists(paths[i]) && fs::exists(paths[j]) && fs::equivalent(paths[i], paths[j])))
            throw std::runtime_error("input, checkpoint output, and events must have distinct file identities");
}

namespace detail {
struct StagedOutput {
    fs::path target, directory, payload, backup;
    bool backup_saved = false, published = false;
    explicit StagedOutput(fs::path path) : target(std::move(path)) {}
    StagedOutput(const StagedOutput&) = delete;
    StagedOutput& operator=(const StagedOutput&) = delete;
    ~StagedOutput() {
        // Any failed restoration retains the original backup and its location.
        if (directory.empty() || backup_saved) return;
        std::error_code ignored;
        fs::remove(payload, ignored);
        fs::remove(directory, ignored);
    }
};

inline fs::path staging_directory(const fs::path& target) {
    static std::atomic<unsigned long long> sequence{0};
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    for (unsigned attempt = 0; attempt < 64; ++attempt) {
        auto directory = target.parent_path() / (".ftd-publication-" + std::to_string(stamp) + "-" + std::to_string(sequence++));
        if (fs::create_directory(directory)) return directory;
    }
    throw std::runtime_error("cannot create an exclusive publication staging directory");
}

inline void stage(StagedOutput& out, const char* data, std::size_t size) {
    if (size > std::size_t(std::numeric_limits<std::streamsize>::max()))
        throw std::overflow_error("publication payload exceeds stream size");
    out.directory = staging_directory(out.target);
    out.payload = out.directory / "payload";
    out.backup = out.directory / "previous";
    std::ofstream stream(out.payload, std::ios::binary);
    if (!stream) throw std::runtime_error("cannot open staged output");
    stream.write(data, static_cast<std::streamsize>(size));
    stream.flush();
    if (!stream) throw std::runtime_error("cannot write staged output");
    stream.close();
    if (!stream) throw std::runtime_error("cannot close staged output");
}

// The rename callable is an exception-injection seam for filesystem tests.
// Production supplies std::filesystem::rename; rollback always uses it directly.
template<class Rename>
void publish_pair(const fs::path& input, const fs::path& output, const fs::path& events,
                  const std::vector<std::uint8_t>& checkpoint, const std::string& journal, Rename rename) {
    validate_paths(input, output, events);
    StagedOutput staged[] = {StagedOutput(fs::weakly_canonical(fs::absolute(output))),
                             StagedOutput(fs::weakly_canonical(fs::absolute(events)))};
    stage(staged[0], reinterpret_cast<const char*>(checkpoint.data()), checkpoint.size());
    stage(staged[1], journal.data(), journal.size());
    validate_paths(input, output, events);
    try {
        for (auto& out : staged) if (fs::exists(out.target)) {
            rename(out.target, out.backup);
            out.backup_saved = true;
        }
        for (auto& out : staged) {
            rename(out.payload, out.target);
            out.published = true;
        }
    } catch (...) {
        const auto original = std::current_exception();
        std::string failures;
        for (auto& out : staged) {
            std::error_code error;
            if (out.published) fs::remove(out.target, error);
            if (!error && out.backup_saved) {
                fs::rename(out.backup, out.target, error);
                if (!error) out.backup_saved = false;
            }
            if (error) failures += " [" + out.directory.string() + ": " + error.message() + "]";
        }
        if (!failures.empty()) throw std::runtime_error("publication failed; rollback incomplete; retained staging/backup:" + failures);
        std::rethrow_exception(original);
    }
    // Both files are published. Cleanup cannot turn a successful pair into a
    // misleading failure; undeletable backups remain in their owned directory.
    for (auto& out : staged) if (out.backup_saved) {
        std::error_code error;
        fs::remove(out.backup, error);
        if (!error) out.backup_saved = false;
    }
}
} // namespace detail

inline void publish_pair(const fs::path& input, const fs::path& output, const fs::path& events,
                         const std::vector<std::uint8_t>& checkpoint, const std::string& journal) {
    detail::publish_pair(input, output, events, checkpoint, journal,
                         [](const fs::path& from, const fs::path& to) { fs::rename(from, to); });
}
} // namespace ftd::strict::cli
