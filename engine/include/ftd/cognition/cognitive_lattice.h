#pragma once
/**
 * @file cognitive_lattice.h
 * @brief Ternary cognitive sidecar for LLM memory and retrieval routing.
 *
 * This module is intentionally separate from RenderBridge and the physics
 * engine. It reuses only neutral lattice/data-layout ideas: ternary authority,
 * SoA pressure fields, active-index iteration, deterministic diagnostics.
 */

#include "ftd/lattice.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ftd::cog {

enum class Polarity : int8_t {
  Contradicted = -1,
  Unknown = 0,
  Supported = 1,
};

enum class NodeKind : uint8_t {
  Claim,
  Entity,
  Event,
  Goal,
  Evidence,
  Tool,
  Preference,
  Constraint,
};

enum class SourceKind : uint8_t {
  ModelDraft,
  AssistantInference,
  RetrievedSource,
  UserInstruction,
  ToolObservation,
  DeletionTombstone,
};

enum class EdgeKind : uint8_t {
  Semantic,
  Temporal,
  Causal,
  Evidential,
  Supports,
  Contradicts,
  Updates,
  DependsOn,
};

struct SourceRef {
  std::string source_id;
  SourceKind kind = SourceKind::ModelDraft;
  int trust_tier = 0;
};

struct CognitiveEdge {
  std::string target_key;
  EdgeKind kind = EdgeKind::Semantic;
  double weight = 1.0;
};

struct CognitiveEvent {
  std::string canonical_key;
  NodeKind kind = NodeKind::Claim;
  Polarity polarity = Polarity::Unknown;
  double confidence = 1.0;
  SourceRef source;
  std::vector<CognitiveEdge> edges;
  int ttl_ticks = 0;
  bool durable = false;
  std::string domain_tag;
  std::uint64_t memory_id = 0;
};

struct QueryIntent {
  std::vector<std::string> focus_keys;
  bool include_conflicts = true;
  bool include_unknowns = false;
};

struct ContextBudget {
  int max_items = 24;
  int max_retrieval_keys = 12;
  int max_estimated_tokens = 2048;
};

struct ContextItem {
  std::string canonical_key;
  std::uint64_t memory_id = 0;
  Polarity polarity = Polarity::Unknown;
  double confidence = 0.0;
  double salience = 0.0;
  double trust = 0.0;
  double conflict_pressure = 0.0;
  std::string source_id;
  NodeKind kind = NodeKind::Claim;
};

struct ContextBrief {
  std::vector<ContextItem> commitments;
  std::vector<ContextItem> conflicts;
  std::vector<ContextItem> open_questions;
  std::vector<std::string> retrieval_keys;
  std::vector<std::string> routing_hints;
  int estimated_tokens = 0;
};

struct CandidateDraft {
  std::vector<CognitiveEvent> claims;
  std::vector<std::string> referenced_keys;
};

struct CandidateScore {
  double support_score = 0.0;
  double conflict_score = 0.0;
  double evidence_score = 0.0;
  double novelty_score = 0.0;
  double total_score = 0.0;
  std::vector<std::string> conflicts;
  std::vector<std::string> unsupported;
};

struct PrivacyScope {
  bool all = false;
  std::vector<std::string> canonical_keys;
  std::vector<std::string> source_ids;
  std::string domain_tag;
};

class CognitiveTernaryField {
public:
  CognitiveTernaryField() = default;
  explicit CognitiveTernaryField(std::size_t n) { resize(n); }

  void resize(std::size_t n);
  void clear();

  std::size_t size() const { return state_.size(); }
  int8_t state_at(std::size_t idx) const { return state_[idx]; }
  Polarity polarity_at(std::size_t idx) const;
  bool is_active(std::size_t idx) const { return state_[idx] != 0; }

  int supported_count() const { return supported_count_; }
  int contradicted_count() const { return contradicted_count_; }
  int active_count() const { return supported_count_ + contradicted_count_; }
  long long net_polarity() const { return net_polarity_; }

  int8_t set_state(std::size_t idx, Polarity polarity);
  int8_t set_state(std::size_t idx, int8_t raw_state);

  const std::vector<int>& active_indices() const { return active_indices_; }
  const std::vector<int>& ordered_active_indices() const;
  const std::vector<int8_t>& states() const { return state_; }
  const std::vector<std::uint64_t>& pos_bits() const { return pos_bits_; }
  const std::vector<std::uint64_t>& neg_bits() const { return neg_bits_; }
  const std::vector<std::uint64_t>& occupied_bits() const {
    return occupied_bits_;
  }

  bool check_invariants(std::string* error = nullptr) const;
  static int8_t normalize(int8_t s);

private:
  void remove_active(std::size_t idx);
  void set_bits(std::size_t idx, int8_t state);
  static bool bit_at(const std::vector<std::uint64_t>& bits, std::size_t idx);

  std::vector<int8_t> state_;
  std::vector<std::uint64_t> pos_bits_;
  std::vector<std::uint64_t> neg_bits_;
  std::vector<std::uint64_t> occupied_bits_;
  std::vector<int> active_indices_;
  mutable std::vector<int> ordered_active_indices_;
  std::vector<int> active_pos_;
  mutable bool ordered_active_dirty_ = false;
  int supported_count_ = 0;
  int contradicted_count_ = 0;
  long long net_polarity_ = 0;
};

struct CognitiveFieldSoA {
  std::vector<double> salience;
  std::vector<double> confidence;
  std::vector<double> recency;
  std::vector<double> trust;
  std::vector<double> retrieval_pressure;
  std::vector<double> conflict_pressure;

  void resize(std::size_t n);
  void clear();
  std::size_t size() const { return salience.size(); }
  bool sized() const;
};

struct CognitiveMeta {
  std::uint64_t memory_id = 0;
  std::string source_id;
  SourceKind source_kind = SourceKind::ModelDraft;
  int source_precedence = 0;
  int trust_tier = 0;
  int ttl_ticks = 0;
  NodeKind kind = NodeKind::Claim;
  std::string domain_tag;
  bool pinned = false;
  bool deleted = false;
};

struct CognitiveMetaSoA {
  std::vector<CognitiveMeta> meta;

  void resize(std::size_t n) { meta.assign(n, {}); }
  void clear() { meta.clear(); }
  std::size_t size() const { return meta.size(); }
};

struct StoredEdge {
  int from = -1;
  int to = -1;
  EdgeKind kind = EdgeKind::Semantic;
  double weight = 1.0;
};

class CognitiveEdgeStore {
public:
  void resize(std::size_t n);
  void clear();
  bool add_edge(int from, int to, EdgeKind kind, double weight);
  const std::vector<StoredEdge>& outgoing(int idx) const {
    return adjacency_[static_cast<std::size_t>(idx)];
  }
  const std::vector<StoredEdge>& edges() const { return edges_; }

private:
  std::vector<std::vector<StoredEdge>> adjacency_;
  std::vector<StoredEdge> edges_;
};

struct CognitiveDiagnostics {
  std::uint64_t tick = 0;
  int active_count = 0;
  int supported_count = 0;
  int contradicted_count = 0;
  int unknown_count = 0;
  long long net_polarity = 0;
  int conflict_count = 0;
  int deleted_count = 0;
  int decay_count = 0;
  int invalidated_count = 0;
  int retrieval_candidate_count = 0;
  int allocation_failure_count = 0;
  int project_failure_count = 0;
  double occupancy = 0.0;
  double mean_confidence = 0.0;
  double mean_salience = 0.0;
  double activation_entropy = 0.0;
  double constraint_residual = 0.0;
};

struct CognitiveSnapshot {
  int lattice_size = 0;
  std::uint64_t tick = 0;
  std::vector<std::string> site_keys;
  std::vector<CognitiveMeta> meta;
  std::vector<StoredEdge> edges;
  std::vector<int8_t> states;
  CognitiveFieldSoA fields;
  CognitiveDiagnostics counters;
};

class CognitiveLattice {
public:
  explicit CognitiveLattice(int lattice_size = 32);

  int lattice_size() const { return lattice_.size(); }
  std::int64_t capacity() const { return lattice_.total_sites(); }
  std::uint64_t tick_count() const { return tick_; }

  void observe(const std::vector<CognitiveEvent>& events);
  void tick();
  ContextBrief query(const QueryIntent& intent, const ContextBudget& budget) const;
  CandidateScore score(const CandidateDraft& draft) const;
  void commit(const std::vector<CognitiveEvent>& events);
  void forget(const PrivacyScope& scope);

  CognitiveDiagnostics diagnostics() const;
  CognitiveSnapshot snapshot() const;
  void load_snapshot(const CognitiveSnapshot& snapshot);

  int site_for_key(const std::string& canonical_key) const;
  const CognitiveTernaryField& ternary() const { return ternary_; }
  const CognitiveFieldSoA& fields() const { return fields_; }
  const CognitiveMetaSoA& meta() const { return meta_; }
  const CognitiveEdgeStore& edges() const { return edges_; }

private:
  int allocate_site(const std::string& canonical_key);
  bool apply_event(const CognitiveEvent& event);
  void add_event_edges(int from, const CognitiveEvent& event);
  void mark_deleted(int idx);
  void propagate();
  void decay();

  std::vector<int> ranked_sites(const QueryIntent& intent,
                                const ContextBudget& budget) const;
  ContextItem make_item(int idx) const;
  double item_score(int idx) const;

  static std::uint64_t stable_hash(const std::string& key);
  static int source_precedence(const SourceRef& source);
  static double clamp01(double value);
  static int8_t to_int(Polarity polarity);
  static Polarity to_polarity(int8_t state);

  Lattice lattice_;
  std::uint64_t tick_ = 0;
  std::uint64_t next_memory_id_ = 1;
  CognitiveTernaryField ternary_;
  CognitiveFieldSoA fields_;
  CognitiveMetaSoA meta_;
  CognitiveEdgeStore edges_;
  std::vector<std::string> site_keys_;
  std::unordered_map<std::string, int> key_to_site_;

  int deleted_count_ = 0;
  int decay_count_ = 0;
  int invalidated_count_ = 0;
  int allocation_failure_count_ = 0;
  int project_failure_count_ = 0;
};

}  // namespace ftd::cog
