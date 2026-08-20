#pragma once

#include "native_desktop/ui_command.h"
#include "native_desktop/ui_journal.h"

#include <vector>

namespace ftd {
class RenderBridge;
}

namespace ftd::native_desktop {

class NativeEngineSession;

class ParameterJournal {
public:
    void append(JournalEntry entry);
    const std::vector<JournalEntry>& entries() const { return entries_; }
    void clear();

    // Re-applies each recorded request onto `bridge` from its current state.
    // `applied` is whatever the engine holds after that command lands.
    void replay_requests(RenderBridge& bridge, NativeEngineSession* session = nullptr);

private:
    std::vector<JournalEntry> entries_;
};

JValue read_journal_key(const RenderBridge& bridge, const std::string& key);
bool same_term_toggles(const ftd::TermToggles& a, const ftd::TermToggles& b);
bool same_bridge_knobs(const RenderBridge& a, const RenderBridge& b);

}  // namespace ftd::native_desktop
