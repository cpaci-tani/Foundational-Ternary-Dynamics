#include "ftd/cognition/cognitive_lattice.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace ftd::cog {
namespace {

constexpr double kConflictThreshold = 0.20;
constexpr double kDecayThreshold = 0.03;
constexpr double kLocalPropagation = 0.015;
constexpr double kEdgePropagation = 0.20;

bool contains_string(const std::vector<std::string>& values,
                     const std::string& needle) {
  return std::find(values.begin(), values.end(), needle) != values.end();
}

int base_source_rank(SourceKind kind) {
  switch (kind) {
    case SourceKind::ModelDraft: return 1;
    case SourceKind::AssistantInference: return 2;
    case SourceKind::RetrievedSource: return 3;
    case SourceKind::UserInstruction: return 4;
    case SourceKind::ToolObservation: return 5;
    case SourceKind::DeletionTombstone: return 6;
  }
  return 0;
}

double edge_multiplier(EdgeKind kind) {
  switch (kind) {
    case EdgeKind::Contradicts: return 1.5;
    case EdgeKind::Updates: return 1.2;
    case EdgeKind::Supports: return 1.1;
    case EdgeKind::Evidential: return 1.0;
    case EdgeKind::DependsOn: return 0.9;
    case EdgeKind::Causal: return 0.8;
    case EdgeKind::Temporal: return 0.6;
    case EdgeKind::Semantic: return 0.5;
  }
  return 0.5;
}

}  // namespace

void CognitiveTernaryField::resize(std::size_t n) {
  state_.assign(n, 0);
  const std::size_t words = (n + 63u) / 64u;
  pos_bits_.assign(words, 0);
  neg_bits_.assign(words, 0);
  occupied_bits_.assign(words, 0);
  active_indices_.clear();
  ordered_active_indices_.clear();
  active_pos_.assign(n, -1);
  ordered_active_dirty_ = false;
  supported_count_ = 0;
  contradicted_count_ = 0;
  net_polarity_ = 0;
}

void CognitiveTernaryField::clear() {
  std::fill(state_.begin(), state_.end(), int8_t{0});
  std::fill(pos_bits_.begin(), pos_bits_.end(), std::uint64_t{0});
  std::fill(neg_bits_.begin(), neg_bits_.end(), std::uint64_t{0});
  std::fill(occupied_bits_.begin(), occupied_bits_.end(), std::uint64_t{0});
  active_indices_.clear();
  ordered_active_indices_.clear();
  std::fill(active_pos_.begin(), active_pos_.end(), -1);
  ordered_active_dirty_ = false;
  supported_count_ = 0;
  contradicted_count_ = 0;
  net_polarity_ = 0;
}

Polarity CognitiveTernaryField::polarity_at(std::size_t idx) const {
  if (state_[idx] > 0) return Polarity::Supported;
  if (state_[idx] < 0) return Polarity::Contradicted;
  return Polarity::Unknown;
}

int8_t CognitiveTernaryField::set_state(std::size_t idx, Polarity polarity) {
  return set_state(idx, static_cast<int8_t>(polarity));
}

int8_t CognitiveTernaryField::set_state(std::size_t idx, int8_t raw_state) {
  const int8_t s = normalize(raw_state);
  const int8_t old = state_[idx];
  if (old == s) return s;

  if (old > 0) --supported_count_;
  else if (old < 0) --contradicted_count_;
  net_polarity_ -= static_cast<long long>(old);

  if (old == 0 && s != 0) {
    active_pos_[idx] = static_cast<int>(active_indices_.size());
    active_indices_.push_back(static_cast<int>(idx));
    ordered_active_dirty_ = true;
  } else if (old != 0 && s == 0) {
    remove_active(idx);
  }

  state_[idx] = s;
  if (s > 0) ++supported_count_;
  else if (s < 0) ++contradicted_count_;
  net_polarity_ += static_cast<long long>(s);
  set_bits(idx, s);
  return s;
}

const std::vector<int>& CognitiveTernaryField::ordered_active_indices() const {
  if (ordered_active_dirty_) {
    ordered_active_indices_ = active_indices_;
    std::sort(ordered_active_indices_.begin(), ordered_active_indices_.end());
    ordered_active_dirty_ = false;
  }
  return ordered_active_indices_;
}

bool CognitiveTernaryField::check_invariants(std::string* error) const {
  if (state_.size() != active_pos_.size()) {
    if (error) *error = "state and active_pos sizes differ";
    return false;
  }
  if (pos_bits_.size() != neg_bits_.size() ||
      pos_bits_.size() != occupied_bits_.size()) {
    if (error) *error = "bit-plane sizes differ";
    return false;
  }

  int supported = 0;
  int contradicted = 0;
  long long net = 0;
  std::vector<int> seen(state_.size(), 0);
  for (std::size_t i = 0; i < state_.size(); ++i) {
    const int8_t s = state_[i];
    if (s < -1 || s > 1) {
      if (error) *error = "state outside ternary alphabet";
      return false;
    }
    if (s > 0) ++supported;
    if (s < 0) ++contradicted;
    net += s;

    const bool p = bit_at(pos_bits_, i);
    const bool m = bit_at(neg_bits_, i);
    const bool o = bit_at(occupied_bits_, i);
    if (p && m) {
      if (error) *error = "positive and negative bit overlap";
      return false;
    }
    if (p != (s > 0) || m != (s < 0) || o != (s != 0)) {
      if (error) *error = "bit plane does not match state";
      return false;
    }
    if ((s == 0 && active_pos_[i] != -1) ||
        (s != 0 && active_pos_[i] < 0)) {
      if (error) *error = "active position does not match state";
      return false;
    }
  }

  for (std::size_t k = 0; k < active_indices_.size(); ++k) {
    const int idx = active_indices_[k];
    if (idx < 0 || static_cast<std::size_t>(idx) >= state_.size()) {
      if (error) *error = "active index out of range";
      return false;
    }
    if (state_[idx] == 0) {
      if (error) *error = "active index points at inactive state";
      return false;
    }
    if (active_pos_[idx] != static_cast<int>(k)) {
      if (error) *error = "active position back-reference mismatch";
      return false;
    }
    if (seen[idx]++) {
      if (error) *error = "duplicate active index";
      return false;
    }
  }

  if (!ordered_active_dirty_) {
    std::vector<int> sorted = active_indices_;
    std::sort(sorted.begin(), sorted.end());
    if (ordered_active_indices_ != sorted) {
      if (error) *error = "ordered active cache mismatch";
      return false;
    }
  }

  if (supported != supported_count_ ||
      contradicted != contradicted_count_ ||
      net != net_polarity_ ||
      static_cast<int>(active_indices_.size()) != supported + contradicted) {
    if (error) *error = "cached counts do not match state";
    return false;
  }
  return true;
}

int8_t CognitiveTernaryField::normalize(int8_t s) {
  if (s > 0) return 1;
  if (s < 0) return -1;
  return 0;
}

void CognitiveTernaryField::remove_active(std::size_t idx) {
  const int pos = active_pos_[idx];
  if (pos < 0) return;
  const int last = active_indices_.back();
  active_indices_[pos] = last;
  active_pos_[last] = pos;
  active_indices_.pop_back();
  active_pos_[idx] = -1;
  ordered_active_dirty_ = true;
}

void CognitiveTernaryField::set_bits(std::size_t idx, int8_t state) {
  const std::size_t word = idx / 64u;
  const std::uint64_t mask = std::uint64_t{1} << (idx % 64u);
  pos_bits_[word] &= ~mask;
  neg_bits_[word] &= ~mask;
  occupied_bits_[word] &= ~mask;
  if (state > 0) {
    pos_bits_[word] |= mask;
    occupied_bits_[word] |= mask;
  } else if (state < 0) {
    neg_bits_[word] |= mask;
    occupied_bits_[word] |= mask;
  }
}

bool CognitiveTernaryField::bit_at(const std::vector<std::uint64_t>& bits,
                                   std::size_t idx) {
  const std::size_t word = idx / 64u;
  if (word >= bits.size()) return false;
  return (bits[word] & (std::uint64_t{1} << (idx % 64u))) != 0;
}

void CognitiveFieldSoA::resize(std::size_t n) {
  salience.assign(n, 0.0);
  confidence.assign(n, 0.0);
  recency.assign(n, 0.0);
  trust.assign(n, 0.0);
  retrieval_pressure.assign(n, 0.0);
  conflict_pressure.assign(n, 0.0);
}

void CognitiveFieldSoA::clear() {
  salience.clear();
  confidence.clear();
  recency.clear();
  trust.clear();
  retrieval_pressure.clear();
  conflict_pressure.clear();
}

bool CognitiveFieldSoA::sized() const {
  return confidence.size() == salience.size() &&
         recency.size() == salience.size() &&
         trust.size() == salience.size() &&
         retrieval_pressure.size() == salience.size() &&
         conflict_pressure.size() == salience.size();
}

void CognitiveEdgeStore::resize(std::size_t n) {
  adjacency_.assign(n, {});
  edges_.clear();
}

void CognitiveEdgeStore::clear() {
  adjacency_.clear();
  edges_.clear();
}

bool CognitiveEdgeStore::add_edge(int from, int to, EdgeKind kind, double weight) {
  if (from < 0 || to < 0 ||
      static_cast<std::size_t>(from) >= adjacency_.size() ||
      static_cast<std::size_t>(to) >= adjacency_.size()) {
    return false;
  }
  auto& out = adjacency_[static_cast<std::size_t>(from)];
  for (auto& edge : out) {
    if (edge.to == to && edge.kind == kind) {
      edge.weight = std::max(edge.weight, weight);
      return false;
    }
  }
  StoredEdge edge{from, to, kind, weight};
  out.push_back(edge);
  edges_.push_back(edge);
  return true;
}

CognitiveLattice::CognitiveLattice(int lattice_size)
    : lattice_(std::max(2, lattice_size)) {
  const std::size_t n = static_cast<std::size_t>(lattice_.total_sites());
  ternary_.resize(n);
  fields_.resize(n);
  meta_.resize(n);
  edges_.resize(n);
  site_keys_.assign(n, {});
}

void CognitiveLattice::observe(const std::vector<CognitiveEvent>& events) {
  for (const auto& event : events) {
    if (!apply_event(event)) ++project_failure_count_;
  }
}

void CognitiveLattice::commit(const std::vector<CognitiveEvent>& events) {
  std::vector<CognitiveEvent> durable = events;
  for (auto& event : durable) event.durable = true;
  observe(durable);
}

void CognitiveLattice::tick() {
  propagate();
  decay();
  ++tick_;
}

ContextBrief CognitiveLattice::query(const QueryIntent& intent,
                                     const ContextBudget& budget) const {
  ContextBrief brief;
  const int max_items = std::max(0, budget.max_items);
  const int max_retrieval = std::max(0, budget.max_retrieval_keys);
  const int max_tokens = std::max(0, budget.max_estimated_tokens);
  const auto ranked = ranked_sites(intent, budget);

  for (int idx : ranked) {
    if (idx < 0 || static_cast<std::size_t>(idx) >= site_keys_.size()) continue;
    if (meta_.meta[idx].deleted) continue;
    if (max_tokens > 0 && brief.estimated_tokens >= max_tokens) break;

    const auto item = make_item(idx);
    const int8_t state = ternary_.state_at(static_cast<std::size_t>(idx));
    if (state > 0 && static_cast<int>(brief.commitments.size()) < max_items) {
      brief.commitments.push_back(item);
    } else if ((state < 0 || item.conflict_pressure > kConflictThreshold) &&
               intent.include_conflicts &&
               static_cast<int>(brief.conflicts.size()) < max_items) {
      brief.conflicts.push_back(item);
    } else if (state == 0 && intent.include_unknowns &&
               static_cast<int>(brief.open_questions.size()) < max_items) {
      brief.open_questions.push_back(item);
    }

    if (static_cast<int>(brief.retrieval_keys.size()) < max_retrieval &&
        fields_.retrieval_pressure[idx] > 0.0 &&
        !site_keys_[idx].empty()) {
      brief.retrieval_keys.push_back(site_keys_[idx]);
    }
    brief.estimated_tokens +=
        static_cast<int>(site_keys_[idx].size() / 4u) + 8;
  }

  if (!brief.conflicts.empty()) brief.routing_hints.push_back("resolve-conflict");
  if (!brief.retrieval_keys.empty()) brief.routing_hints.push_back("retrieve-evidence");
  if (!brief.open_questions.empty()) brief.routing_hints.push_back("ask-or-abstain");
  return brief;
}

CandidateScore CognitiveLattice::score(const CandidateDraft& draft) const {
  CandidateScore score;
  for (const auto& claim : draft.claims) {
    const int idx = site_for_key(claim.canonical_key);
    const int8_t proposed = to_int(claim.polarity);
    if (idx < 0) {
      score.novelty_score += 1.0;
      score.unsupported.push_back(claim.canonical_key);
      continue;
    }
    if (meta_.meta[idx].deleted) {
      score.conflict_score += 1.0;
      score.conflicts.push_back(claim.canonical_key);
      continue;
    }
    const int8_t current = ternary_.state_at(static_cast<std::size_t>(idx));
    if (current != 0 && proposed != 0 && current == proposed) {
      score.support_score += fields_.confidence[idx] + fields_.trust[idx];
      score.evidence_score += fields_.retrieval_pressure[idx];
    } else if (current != 0 && proposed != 0 && current != proposed) {
      score.conflict_score += 1.0 + fields_.conflict_pressure[idx];
      score.conflicts.push_back(claim.canonical_key);
    } else {
      score.novelty_score += 0.5;
      score.unsupported.push_back(claim.canonical_key);
    }
  }

  for (const auto& key : draft.referenced_keys) {
    const int idx = site_for_key(key);
    if (idx >= 0 && !meta_.meta[idx].deleted) {
      score.evidence_score += fields_.trust[idx] * 0.5;
    } else {
      score.unsupported.push_back(key);
    }
  }

  score.total_score = score.support_score + score.evidence_score +
                      score.novelty_score * 0.2 -
                      score.conflict_score * 2.0;
  return score;
}

void CognitiveLattice::forget(const PrivacyScope& scope) {
  for (std::size_t i = 0; i < site_keys_.size(); ++i) {
    if (site_keys_[i].empty() || meta_.meta[i].deleted) continue;
    const bool match_all = scope.all;
    const bool match_key = contains_string(scope.canonical_keys, site_keys_[i]);
    const bool match_source =
        contains_string(scope.source_ids, meta_.meta[i].source_id);
    const bool match_domain =
        !scope.domain_tag.empty() && meta_.meta[i].domain_tag == scope.domain_tag;
    if (match_all || match_key || match_source || match_domain) {
      mark_deleted(static_cast<int>(i));
    }
  }
}

CognitiveDiagnostics CognitiveLattice::diagnostics() const {
  CognitiveDiagnostics d;
  d.tick = tick_;
  d.active_count = ternary_.active_count();
  d.supported_count = ternary_.supported_count();
  d.contradicted_count = ternary_.contradicted_count();
  d.unknown_count = static_cast<int>(ternary_.size()) - d.active_count;
  d.net_polarity = ternary_.net_polarity();
  d.deleted_count = deleted_count_;
  d.decay_count = decay_count_;
  d.invalidated_count = invalidated_count_;
  d.allocation_failure_count = allocation_failure_count_;
  d.project_failure_count = project_failure_count_;
  d.occupancy = ternary_.size() == 0
      ? 0.0
      : static_cast<double>(d.active_count) / static_cast<double>(ternary_.size());

  double conf_sum = 0.0;
  double salience_sum = 0.0;
  double salience_total = 0.0;
  int field_count = 0;
  for (std::size_t i = 0; i < fields_.size(); ++i) {
    if (meta_.meta[i].deleted) continue;
    if (fields_.conflict_pressure[i] > kConflictThreshold) ++d.conflict_count;
    if (fields_.retrieval_pressure[i] > 0.05) ++d.retrieval_candidate_count;
    if (fields_.salience[i] > 0.0 || fields_.confidence[i] > 0.0) {
      conf_sum += fields_.confidence[i];
      salience_sum += fields_.salience[i];
      salience_total += fields_.salience[i];
      ++field_count;
    }
  }
  if (field_count > 0) {
    d.mean_confidence = conf_sum / field_count;
    d.mean_salience = salience_sum / field_count;
  }
  if (salience_total > 0.0) {
    for (std::size_t i = 0; i < fields_.size(); ++i) {
      if (meta_.meta[i].deleted || fields_.salience[i] <= 0.0) continue;
      const double p = fields_.salience[i] / salience_total;
      d.activation_entropy -= p * std::log(p);
    }
  }

  std::string error;
  d.constraint_residual = ternary_.check_invariants(&error) ? 0.0 : 1.0;
  return d;
}

CognitiveSnapshot CognitiveLattice::snapshot() const {
  CognitiveSnapshot s;
  s.lattice_size = lattice_.size();
  s.tick = tick_;
  s.site_keys = site_keys_;
  s.meta = meta_.meta;
  s.edges = edges_.edges();
  s.states = ternary_.states();
  s.fields = fields_;
  s.counters = diagnostics();
  return s;
}

void CognitiveLattice::load_snapshot(const CognitiveSnapshot& snapshot) {
  *this = CognitiveLattice(snapshot.lattice_size);
  tick_ = snapshot.tick;
  site_keys_ = snapshot.site_keys;
  if (site_keys_.size() != static_cast<std::size_t>(capacity())) {
    site_keys_.resize(static_cast<std::size_t>(capacity()));
  }
  for (std::size_t i = 0; i < site_keys_.size(); ++i) {
    if (!site_keys_[i].empty()) key_to_site_[site_keys_[i]] = static_cast<int>(i);
  }
  if (snapshot.meta.size() == static_cast<std::size_t>(capacity())) {
    meta_.meta = snapshot.meta;
  }
  if (snapshot.fields.size() == static_cast<std::size_t>(capacity()) &&
      snapshot.fields.sized()) {
    fields_ = snapshot.fields;
  }
  const std::size_t n = std::min(snapshot.states.size(),
                                 static_cast<std::size_t>(capacity()));
  for (std::size_t i = 0; i < n; ++i) {
    ternary_.set_state(i, snapshot.states[i]);
    if (meta_.meta[i].memory_id >= next_memory_id_) {
      next_memory_id_ = meta_.meta[i].memory_id + 1;
    }
    if (meta_.meta[i].deleted) ++deleted_count_;
  }
  for (const auto& edge : snapshot.edges) {
    edges_.add_edge(edge.from, edge.to, edge.kind, edge.weight);
  }
  deleted_count_ = snapshot.counters.deleted_count;
  decay_count_ = snapshot.counters.decay_count;
  invalidated_count_ = snapshot.counters.invalidated_count;
  allocation_failure_count_ = snapshot.counters.allocation_failure_count;
  project_failure_count_ = snapshot.counters.project_failure_count;
}

int CognitiveLattice::site_for_key(const std::string& canonical_key) const {
  const auto it = key_to_site_.find(canonical_key);
  return it == key_to_site_.end() ? -1 : it->second;
}

int CognitiveLattice::allocate_site(const std::string& canonical_key) {
  if (canonical_key.empty()) return -1;
  const auto found = key_to_site_.find(canonical_key);
  if (found != key_to_site_.end()) return found->second;

  const std::size_t n = site_keys_.size();
  if (n == 0) return -1;
  std::size_t pos = stable_hash(canonical_key) % n;
  for (std::size_t step = 0; step < n; ++step) {
    const std::size_t idx = (pos + step) % n;
    if (site_keys_[idx].empty()) {
      site_keys_[idx] = canonical_key;
      key_to_site_[canonical_key] = static_cast<int>(idx);
      return static_cast<int>(idx);
    }
  }
  ++allocation_failure_count_;
  return -1;
}

bool CognitiveLattice::apply_event(const CognitiveEvent& event) {
  const int idx = allocate_site(event.canonical_key);
  if (idx < 0) return false;

  auto& meta = meta_.meta[idx];
  const int precedence = source_precedence(event.source);
  if (event.source.kind == SourceKind::DeletionTombstone) {
    meta.source_id = event.source.source_id;
    meta.source_kind = event.source.kind;
    meta.source_precedence = precedence;
    meta.trust_tier = event.source.trust_tier;
    meta.domain_tag = event.domain_tag;
    mark_deleted(idx);
    return true;
  }
  if (meta.deleted) return true;

  const int8_t current = ternary_.state_at(static_cast<std::size_t>(idx));
  const int8_t proposed = to_int(event.polarity);
  const bool empty = meta.memory_id == 0;
  const bool same_or_unknown =
      proposed == 0 || current == 0 || current == proposed;
  const bool stronger = precedence > meta.source_precedence;
  const bool tied = precedence == meta.source_precedence;

  if (empty || same_or_unknown || stronger) {
    if (!same_or_unknown && current != 0 && proposed != 0) {
      ++invalidated_count_;
      fields_.conflict_pressure[idx] =
          std::max(fields_.conflict_pressure[idx], 0.5);
    }
    if (proposed != 0 || stronger || empty) {
      ternary_.set_state(static_cast<std::size_t>(idx), proposed);
    }
    if (empty || precedence >= meta.source_precedence) {
      meta.memory_id = event.memory_id != 0 ? event.memory_id : next_memory_id_++;
      meta.source_id = event.source.source_id;
      meta.source_kind = event.source.kind;
      meta.source_precedence = precedence;
      meta.trust_tier = event.source.trust_tier;
      meta.kind = event.kind;
      meta.domain_tag = event.domain_tag;
      meta.pinned = event.durable;
      if (event.ttl_ticks > 0) meta.ttl_ticks = event.ttl_ticks;
    }
    const double trust = clamp01(static_cast<double>(base_source_rank(event.source.kind)) / 6.0 +
                                 static_cast<double>(event.source.trust_tier) * 0.01);
    fields_.confidence[idx] = std::max(fields_.confidence[idx],
                                       clamp01(event.confidence));
    fields_.salience[idx] = std::max(fields_.salience[idx],
                                     clamp01(event.confidence) + 0.1);
    fields_.recency[idx] = 1.0;
    fields_.trust[idx] = std::max(fields_.trust[idx], trust);
    fields_.retrieval_pressure[idx] =
        std::max(fields_.retrieval_pressure[idx],
                 fields_.confidence[idx] * fields_.trust[idx]);
  } else if (!same_or_unknown && tied) {
    ternary_.set_state(static_cast<std::size_t>(idx), Polarity::Unknown);
    fields_.conflict_pressure[idx] = 1.0;
    fields_.salience[idx] = std::max(fields_.salience[idx], 0.8);
  } else if (!same_or_unknown) {
    fields_.conflict_pressure[idx] =
        std::max(fields_.conflict_pressure[idx], 0.35);
  }

  add_event_edges(idx, event);
  return true;
}

void CognitiveLattice::add_event_edges(int from, const CognitiveEvent& event) {
  for (const auto& edge : event.edges) {
    const int to = allocate_site(edge.target_key);
    if (to < 0) continue;
    const double weight = std::max(0.0, edge.weight);
    edges_.add_edge(from, to, edge.kind, weight);
    if (edge.kind == EdgeKind::Contradicts) {
      fields_.conflict_pressure[from] =
          std::max(fields_.conflict_pressure[from], 0.45 * weight);
      fields_.conflict_pressure[to] =
          std::max(fields_.conflict_pressure[to], 0.45 * weight);
    } else if (edge.kind == EdgeKind::Supports ||
               edge.kind == EdgeKind::Evidential) {
      fields_.retrieval_pressure[to] =
          std::max(fields_.retrieval_pressure[to],
                   fields_.retrieval_pressure[from] * 0.5 * weight);
    }
  }
}

void CognitiveLattice::mark_deleted(int idx) {
  if (idx < 0 || static_cast<std::size_t>(idx) >= meta_.size()) return;
  auto& meta = meta_.meta[idx];
  if (!meta.deleted) ++deleted_count_;
  if (ternary_.state_at(static_cast<std::size_t>(idx)) != 0) {
    ++invalidated_count_;
  }
  meta.deleted = true;
  meta.pinned = false;
  meta.ttl_ticks = 0;
  ternary_.set_state(static_cast<std::size_t>(idx), Polarity::Unknown);
  fields_.salience[idx] = 0.0;
  fields_.confidence[idx] = 0.0;
  fields_.recency[idx] = 0.0;
  fields_.trust[idx] = 0.0;
  fields_.retrieval_pressure[idx] = 0.0;
  fields_.conflict_pressure[idx] = 0.0;
}

void CognitiveLattice::propagate() {
  const std::size_t n = fields_.size();
  std::vector<double> delta_salience(n, 0.0);
  std::vector<double> delta_retrieval(n, 0.0);
  std::vector<double> delta_conflict(n, 0.0);
  std::vector<int> touched;
  std::vector<uint8_t> touched_mask(n, 0);

  auto touch = [&](int idx) {
    if (idx < 0 || static_cast<std::size_t>(idx) >= n) return;
    if (!touched_mask[static_cast<std::size_t>(idx)]) {
      touched_mask[static_cast<std::size_t>(idx)] = 1;
      touched.push_back(idx);
    }
  };

  const auto active = ternary_.active_indices();
  for (int idx : active) {
    if (meta_.meta[idx].deleted) continue;
    const double s = fields_.salience[idx];
    if (s <= 0.0) continue;
    for (int nbr : lattice_.neighbors_26(idx)) {
      if (meta_.meta[nbr].deleted) continue;
      delta_salience[nbr] += s * kLocalPropagation;
      delta_retrieval[nbr] += fields_.retrieval_pressure[idx] *
                              kLocalPropagation;
      touch(nbr);
    }
    for (const auto& edge : edges_.outgoing(idx)) {
      if (edge.to < 0 || meta_.meta[edge.to].deleted) continue;
      const double amount =
          s * kEdgePropagation * edge_multiplier(edge.kind) * edge.weight;
      delta_salience[edge.to] += amount;
      delta_retrieval[edge.to] += fields_.retrieval_pressure[idx] *
                                  kEdgePropagation * edge.weight;
      if (edge.kind == EdgeKind::Contradicts) {
        delta_conflict[idx] += 0.25 * edge.weight;
        delta_conflict[edge.to] += 0.25 * edge.weight;
        touch(idx);
      }
      touch(edge.to);
    }
  }

  for (int idx : touched) {
    fields_.salience[idx] =
        std::min(1.5, fields_.salience[idx] + delta_salience[idx]);
    fields_.retrieval_pressure[idx] =
        std::min(1.5, fields_.retrieval_pressure[idx] + delta_retrieval[idx]);
    fields_.conflict_pressure[idx] =
        std::min(1.5, fields_.conflict_pressure[idx] + delta_conflict[idx]);
  }
}

void CognitiveLattice::decay() {
  const auto active = ternary_.active_indices();
  for (int idx : active) {
    if (meta_.meta[idx].deleted) continue;
    auto& meta = meta_.meta[idx];
    fields_.recency[idx] *= 0.92;
    fields_.salience[idx] *= 0.96;
    fields_.retrieval_pressure[idx] *= 0.97;
    fields_.conflict_pressure[idx] *= 0.98;

    if (!meta.pinned && meta.ttl_ticks > 0) {
      --meta.ttl_ticks;
      if (meta.ttl_ticks == 0) {
        ternary_.set_state(static_cast<std::size_t>(idx), Polarity::Unknown);
        ++decay_count_;
        continue;
      }
    }

    if (!meta.pinned &&
        fields_.salience[idx] < kDecayThreshold &&
        fields_.recency[idx] < kDecayThreshold &&
        fields_.confidence[idx] < kDecayThreshold) {
      ternary_.set_state(static_cast<std::size_t>(idx), Polarity::Unknown);
      ++decay_count_;
    }
  }
}

std::vector<int> CognitiveLattice::ranked_sites(const QueryIntent& intent,
                                                const ContextBudget& budget) const {
  std::vector<int> sites;
  std::vector<uint8_t> seen(site_keys_.size(), 0);
  auto add_site = [&](int idx) {
    if (idx < 0 || static_cast<std::size_t>(idx) >= site_keys_.size()) return;
    if (seen[static_cast<std::size_t>(idx)]) return;
    seen[static_cast<std::size_t>(idx)] = 1;
    sites.push_back(idx);
  };

  for (const auto& key : intent.focus_keys) {
    const int idx = site_for_key(key);
    add_site(idx);
    if (idx >= 0) {
      for (const auto& edge : edges_.outgoing(idx)) add_site(edge.to);
    }
  }
  for (int idx : ternary_.ordered_active_indices()) add_site(idx);
  for (std::size_t i = 0; i < fields_.size(); ++i) {
    if (fields_.conflict_pressure[i] > kConflictThreshold ||
        fields_.retrieval_pressure[i] > 0.25) {
      add_site(static_cast<int>(i));
    }
  }

  std::stable_sort(sites.begin(), sites.end(), [&](int a, int b) {
    const double sa = item_score(a);
    const double sb = item_score(b);
    if (sa != sb) return sa > sb;
    return site_keys_[a] < site_keys_[b];
  });
  const int hard_cap =
      std::max({budget.max_items * 3, budget.max_retrieval_keys * 2, 16});
  if (static_cast<int>(sites.size()) > hard_cap) sites.resize(hard_cap);
  return sites;
}

ContextItem CognitiveLattice::make_item(int idx) const {
  ContextItem item;
  item.canonical_key = site_keys_[idx];
  item.memory_id = meta_.meta[idx].memory_id;
  item.polarity = to_polarity(ternary_.state_at(static_cast<std::size_t>(idx)));
  item.confidence = fields_.confidence[idx];
  item.salience = fields_.salience[idx];
  item.trust = fields_.trust[idx];
  item.conflict_pressure = fields_.conflict_pressure[idx];
  item.source_id = meta_.meta[idx].source_id;
  item.kind = meta_.meta[idx].kind;
  return item;
}

double CognitiveLattice::item_score(int idx) const {
  if (idx < 0 || static_cast<std::size_t>(idx) >= fields_.size()) {
    return -std::numeric_limits<double>::infinity();
  }
  if (meta_.meta[idx].deleted || site_keys_[idx].empty()) {
    return -std::numeric_limits<double>::infinity();
  }
  return fields_.salience[idx] * 2.0 +
         fields_.confidence[idx] +
         fields_.trust[idx] +
         fields_.recency[idx] +
         fields_.retrieval_pressure[idx] +
         fields_.conflict_pressure[idx] * 1.5;
}

std::uint64_t CognitiveLattice::stable_hash(const std::string& key) {
  std::uint64_t h = 1469598103934665603ull;
  for (unsigned char c : key) {
    h ^= static_cast<std::uint64_t>(c);
    h *= 1099511628211ull;
  }
  return h;
}

int CognitiveLattice::source_precedence(const SourceRef& source) {
  const int tier = std::max(0, std::min(99, source.trust_tier));
  return base_source_rank(source.kind) * 100 + tier;
}

double CognitiveLattice::clamp01(double value) {
  if (value < 0.0) return 0.0;
  if (value > 1.0) return 1.0;
  return value;
}

int8_t CognitiveLattice::to_int(Polarity polarity) {
  return static_cast<int8_t>(polarity);
}

Polarity CognitiveLattice::to_polarity(int8_t state) {
  if (state > 0) return Polarity::Supported;
  if (state < 0) return Polarity::Contradicted;
  return Polarity::Unknown;
}

}  // namespace ftd::cog
