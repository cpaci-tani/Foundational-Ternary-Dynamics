#pragma once
/**
 * @file ftd/eft/history_event_journal.h
 * @brief Read-only event instrumentation for the native charge gate.
 *
 * The journal is an observer. It is disabled by default, consumes no random
 * numbers, and never writes lattice, voxel, toggle, or integrator state.
 * Event order is the production tick order; callers may sort a returned copy
 * when comparing runs whose independent events were recorded concurrently.
 */

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "ftd/voxel.h"

namespace ftd {

namespace eft {

enum class HistoryEventKind : std::uint8_t {
    Movement,
    Genesis,
    Evaporation,
    PairProduction,
    Annihilation,
    WeakTransmutation,
};

struct HistorySiteState {
    int index = -1;
    std::int8_t state = 0;
    std::int8_t chirality_sign = 0;
    Vec3 flux{};
    Vec3 flux_L{};
    Vec3 flux_R{};
    // FTD-0449: complete read-only mechanical snapshot.  The legacy scalar
    // and flux members above remain for conserved-feature consumers.  The
    // full copy is required to reconstruct velocity/remainder transport,
    // wave state, persistent IDs, and internal labels without rereading a
    // lattice that has already advanced.
    Voxel voxel{};
};

struct HistoryEvent {
    HistoryEventKind kind = HistoryEventKind::Movement;
    int tick = 0;
    int site_count = 0;
    std::array<HistorySiteState, 2> before{};
    std::array<HistorySiteState, 2> after{};
};

HistorySiteState capture_history_site(int index, const Voxel& voxel);

class HistoryEventJournal {
  public:
    HistoryEventJournal();
    ~HistoryEventJournal();

    HistoryEventJournal(const HistoryEventJournal&) = delete;
    HistoryEventJournal& operator=(const HistoryEventJournal&) = delete;

    void set_enabled(bool enabled);
    bool enabled() const;
    void clear();
    void record(const HistoryEvent& event);
    std::vector<HistoryEvent> snapshot() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace eft
}  // namespace ftd
