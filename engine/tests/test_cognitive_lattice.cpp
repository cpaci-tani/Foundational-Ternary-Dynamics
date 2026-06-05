#include "ftd/cognition/cognitive_lattice.h"
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

using namespace ftd::cog;

namespace {

SourceRef source(SourceKind kind, const std::string& id, int trust = 0) {
  SourceRef s;
  s.kind = kind;
  s.source_id = id;
  s.trust_tier = trust;
  return s;
}

CognitiveEvent event(const std::string& key, Polarity polarity,
                     SourceKind kind, const std::string& source_id,
                     double confidence = 0.8) {
  CognitiveEvent e;
  e.canonical_key = key;
  e.polarity = polarity;
  e.confidence = confidence;
  e.source = source(kind, source_id);
  return e;
}

bool has_key(const std::vector<std::string>& keys, const std::string& key) {
  return std::find(keys.begin(), keys.end(), key) != keys.end();
}

void test_ternary_invariants() {
  CognitiveTernaryField field(16);
  field.set_state(1, Polarity::Supported);
  field.set_state(2, Polarity::Contradicted);
  field.set_state(3, int8_t{7});
  field.set_state(4, int8_t{-9});
  assert(field.supported_count() == 2);
  assert(field.contradicted_count() == 2);
  assert(field.active_count() == 4);
  assert(field.net_polarity() == 0);

  field.set_state(2, Polarity::Unknown);
  assert(field.active_count() == 3);
  assert(field.net_polarity() == 1);

  std::string error;
  assert(field.check_invariants(&error));
  assert(error.empty());
  const auto& ordered = field.ordered_active_indices();
  assert(std::is_sorted(ordered.begin(), ordered.end()));
}

void test_source_precedence_and_deletion() {
  CognitiveLattice lattice(8);
  lattice.observe({event("repo:branch", Polarity::Supported,
                         SourceKind::ModelDraft, "draft")});
  int idx = lattice.site_for_key("repo:branch");
  assert(idx >= 0);
  assert(lattice.ternary().polarity_at(static_cast<std::size_t>(idx)) ==
         Polarity::Supported);

  lattice.observe({event("repo:branch", Polarity::Contradicted,
                         SourceKind::UserInstruction, "user")});
  assert(lattice.ternary().polarity_at(static_cast<std::size_t>(idx)) ==
         Polarity::Contradicted);

  lattice.observe({event("repo:branch", Polarity::Supported,
                         SourceKind::RetrievedSource, "stale-doc")});
  assert(lattice.ternary().polarity_at(static_cast<std::size_t>(idx)) ==
         Polarity::Contradicted);
  assert(lattice.fields().conflict_pressure[static_cast<std::size_t>(idx)] >
         0.0);

  lattice.observe({event("repo:branch", Polarity::Unknown,
                         SourceKind::DeletionTombstone, "forget")});
  assert(lattice.ternary().polarity_at(static_cast<std::size_t>(idx)) ==
         Polarity::Unknown);
  assert(lattice.meta().meta[static_cast<std::size_t>(idx)].deleted);

  lattice.observe({event("repo:branch", Polarity::Supported,
                         SourceKind::ToolObservation, "git")});
  assert(lattice.ternary().polarity_at(static_cast<std::size_t>(idx)) ==
         Polarity::Unknown);

  ContextBrief brief = lattice.query({}, {});
  assert(!has_key(brief.retrieval_keys, "repo:branch"));
}

void test_equal_precedence_conflict_and_ttl() {
  CognitiveLattice lattice(8);
  lattice.observe({event("claim:api-shape", Polarity::Supported,
                         SourceKind::RetrievedSource, "doc-a")});
  lattice.observe({event("claim:api-shape", Polarity::Contradicted,
                         SourceKind::RetrievedSource, "doc-b")});
  const int conflict_idx = lattice.site_for_key("claim:api-shape");
  assert(conflict_idx >= 0);
  assert(lattice.ternary().polarity_at(static_cast<std::size_t>(conflict_idx)) ==
         Polarity::Unknown);
  assert(lattice.fields().conflict_pressure[static_cast<std::size_t>(conflict_idx)] >=
         1.0);

  CognitiveEvent ttl = event("todo:temporary", Polarity::Supported,
                             SourceKind::UserInstruction, "user");
  ttl.ttl_ticks = 1;
  lattice.observe({ttl});
  const int ttl_idx = lattice.site_for_key("todo:temporary");
  assert(lattice.ternary().polarity_at(static_cast<std::size_t>(ttl_idx)) ==
         Polarity::Supported);
  lattice.tick();
  assert(lattice.ternary().polarity_at(static_cast<std::size_t>(ttl_idx)) ==
         Polarity::Unknown);
  assert(lattice.diagnostics().decay_count == 1);
}

void test_edges_query_score_and_snapshot() {
  CognitiveLattice lattice(8);
  CognitiveEvent e = event("goal:ship-layer", Polarity::Supported,
                           SourceKind::UserInstruction, "user", 0.9);
  e.durable = true;
  e.edges.push_back({"evidence:plan", EdgeKind::Evidential, 1.0});
  e.edges.push_back({"claim:blocked", EdgeKind::Contradicts, 1.0});
  lattice.observe({e});
  lattice.tick();

  const int evidence_idx = lattice.site_for_key("evidence:plan");
  assert(evidence_idx >= 0);
  assert(lattice.fields().retrieval_pressure[static_cast<std::size_t>(evidence_idx)] >
         0.0);

  QueryIntent intent;
  intent.focus_keys.push_back("goal:ship-layer");
  intent.include_unknowns = true;
  ContextBudget budget;
  budget.max_items = 8;
  budget.max_retrieval_keys = 8;
  ContextBrief brief = lattice.query(intent, budget);
  assert(!brief.commitments.empty());
  assert(has_key(brief.retrieval_keys, "evidence:plan"));
  assert(!brief.routing_hints.empty());

  CandidateDraft draft;
  draft.claims.push_back(event("goal:ship-layer", Polarity::Supported,
                               SourceKind::AssistantInference, "assistant"));
  draft.claims.push_back(event("missing:claim", Polarity::Supported,
                               SourceKind::AssistantInference, "assistant"));
  draft.referenced_keys.push_back("evidence:plan");
  CandidateScore score = lattice.score(draft);
  assert(score.support_score > 0.0);
  assert(score.evidence_score > 0.0);
  assert(!score.unsupported.empty());

  const auto snap = lattice.snapshot();
  CognitiveLattice restored(2);
  restored.load_snapshot(snap);
  assert(restored.lattice_size() == lattice.lattice_size());
  assert(restored.site_for_key("goal:ship-layer") >= 0);
  assert(restored.query(intent, budget).commitments.size() ==
         brief.commitments.size());

  PrivacyScope scope;
  scope.canonical_keys.push_back("evidence:plan");
  restored.forget(scope);
  assert(restored.meta().meta[static_cast<std::size_t>(
      restored.site_for_key("evidence:plan"))].deleted);
  assert(!has_key(restored.query(intent, budget).retrieval_keys,
                  "evidence:plan"));
}

}  // namespace

int main() {
  test_ternary_invariants();
  test_source_precedence_and_deletion();
  test_equal_precedence_conflict_and_ttl();
  test_edges_query_score_and_snapshot();
  return 0;
}
