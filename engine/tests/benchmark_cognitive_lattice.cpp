/**
 * @file benchmark_cognitive_lattice.cpp
 * @brief Non-physics timing probe for the ternary cognitive sidecar.
 *
 * Usage:
 *   benchmark_cognitive_lattice --L=64 --items=5000 --iters=5 --mode=all
 */

#include "ftd/cognition/cognitive_lattice.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using namespace ftd::cog;

namespace {

struct Args {
  int L = 64;
  int items = 5000;
  int iters = 5;
  std::string mode = "all";
};

Args parse(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; ++i) {
    const char* s = argv[i];
    auto eat_int = [&](const char* key, int& dst) {
      const std::size_t n = std::strlen(key);
      if (std::strncmp(s, key, n) == 0 && s[n] == '=') {
        dst = std::atoi(s + n + 1);
        return true;
      }
      return false;
    };
    if (eat_int("--L", a.L)) continue;
    if (eat_int("--items", a.items)) continue;
    if (eat_int("--iters", a.iters)) continue;
    constexpr const char* mode_key = "--mode=";
    constexpr std::size_t mode_len = 7;
    if (std::strncmp(s, mode_key, mode_len) == 0) a.mode = s + mode_len;
  }
  if (a.L < 4) a.L = 4;
  if (a.items < 1) a.items = 1;
  if (a.iters < 1) a.iters = 1;
  return a;
}

SourceRef source(SourceKind kind, const std::string& id, int trust = 0) {
  SourceRef s;
  s.kind = kind;
  s.source_id = id;
  s.trust_tier = trust;
  return s;
}

std::vector<CognitiveEvent> build_events(int count) {
  std::vector<CognitiveEvent> events;
  events.reserve(static_cast<std::size_t>(count));
  for (int i = 0; i < count; ++i) {
    CognitiveEvent e;
    e.canonical_key = "claim:" + std::to_string(i);
    e.kind = (i % 5 == 0) ? NodeKind::Goal : NodeKind::Claim;
    e.polarity = (i % 11 == 0) ? Polarity::Contradicted
                               : Polarity::Supported;
    e.confidence = 0.55 + 0.004 * static_cast<double>(i % 100);
    e.source = source((i % 7 == 0) ? SourceKind::ToolObservation
                                   : SourceKind::UserInstruction,
                      "source:" + std::to_string(i % 17),
                      i % 10);
    e.ttl_ticks = 1000;
    e.durable = (i % 13 == 0);
    e.domain_tag = "coding";
    if (i > 0) {
      CognitiveEdge edge;
      edge.target_key = "claim:" + std::to_string(std::max(0, i - 3));
      edge.kind = (i % 19 == 0) ? EdgeKind::Contradicts : EdgeKind::Supports;
      edge.weight = 0.5 + 0.01 * static_cast<double>(i % 20);
      e.edges.push_back(edge);
    }
    events.push_back(e);
  }
  return events;
}

template <typename Fn>
double seconds_for(Fn&& fn, double& checksum) {
  using clock = std::chrono::steady_clock;
  const auto t0 = clock::now();
  checksum = fn();
  const auto t1 = clock::now();
  return std::chrono::duration<double>(t1 - t0).count();
}

struct Row {
  std::string mode;
  double reference_seconds = 0.0;
  double lattice_seconds = 0.0;
  double speedup = 0.0;
  double reference_checksum = 0.0;
  double lattice_checksum = 0.0;
};

Row bench_active_iteration(CognitiveLattice& lattice, int iters) {
  Row r;
  r.mode = "active-iteration";
  r.reference_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it) {
      for (std::size_t i = 0; i < lattice.fields().size(); ++i) {
        if (lattice.ternary().state_at(i) != 0) {
          sum += lattice.fields().confidence[i];
        }
      }
    }
    return sum;
  }, r.reference_checksum);
  r.lattice_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it) {
      for (int idx : lattice.ternary().active_indices()) {
        sum += lattice.fields().confidence[static_cast<std::size_t>(idx)];
      }
    }
    return sum;
  }, r.lattice_checksum);
  r.speedup = r.lattice_seconds > 0.0
      ? r.reference_seconds / r.lattice_seconds
      : 0.0;
  return r;
}

Row bench_observe(int L, const std::vector<CognitiveEvent>& events, int iters) {
  Row r;
  r.mode = "observe";
  r.lattice_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it) {
      CognitiveLattice lattice(L);
      lattice.observe(events);
      sum += lattice.diagnostics().active_count;
    }
    return sum;
  }, r.lattice_checksum);
  return r;
}

Row bench_tick(CognitiveLattice& lattice, int iters) {
  Row r;
  r.mode = "tick";
  r.lattice_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it) {
      lattice.tick();
      sum += lattice.diagnostics().mean_salience;
    }
    return sum;
  }, r.lattice_checksum);
  return r;
}

Row bench_query(CognitiveLattice& lattice, int iters) {
  Row r;
  r.mode = "query";
  QueryIntent intent;
  intent.focus_keys.push_back("claim:7");
  intent.include_unknowns = true;
  ContextBudget budget;
  budget.max_items = 32;
  budget.max_retrieval_keys = 24;
  r.lattice_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it) {
      auto brief = lattice.query(intent, budget);
      sum += static_cast<double>(brief.commitments.size() +
                                 brief.conflicts.size() +
                                 brief.retrieval_keys.size());
    }
    return sum;
  }, r.lattice_checksum);
  return r;
}

Row bench_snapshot(CognitiveLattice& lattice, int iters) {
  Row r;
  r.mode = "snapshot";
  r.lattice_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it) {
      const auto snap = lattice.snapshot();
      CognitiveLattice restored(4);
      restored.load_snapshot(snap);
      sum += restored.diagnostics().active_count;
    }
    return sum;
  }, r.lattice_checksum);
  return r;
}

void print_row(const Row& r, int L, int items, int iters, bool comma) {
  std::printf(
      "%s{\"mode\":\"%s\",\"L\":%d,\"items\":%d,\"iters\":%d,"
      "\"reference_seconds\":%.9g,\"lattice_seconds\":%.9g,"
      "\"speedup\":%.9g,\"reference_checksum\":%.17g,"
      "\"lattice_checksum\":%.17g}",
      comma ? "," : "",
      r.mode.c_str(),
      L,
      items,
      iters,
      r.reference_seconds,
      r.lattice_seconds,
      r.speedup,
      r.reference_checksum,
      r.lattice_checksum);
}

bool wants(const std::string& mode, const char* name) {
  return mode == "all" || mode == name;
}

}  // namespace

int main(int argc, char** argv) {
  const Args args = parse(argc, argv);
  const int capacity = args.L * args.L * args.L;
  const int items = std::min(args.items, std::max(1, capacity / 2));
  const auto events = build_events(items);
  CognitiveLattice lattice(args.L);
  lattice.observe(events);

  std::vector<Row> rows;
  if (wants(args.mode, "active-iteration")) {
    rows.push_back(bench_active_iteration(lattice, args.iters));
  }
  if (wants(args.mode, "observe")) {
    rows.push_back(bench_observe(args.L, events, args.iters));
  }
  if (wants(args.mode, "tick")) {
    rows.push_back(bench_tick(lattice, args.iters));
  }
  if (wants(args.mode, "query")) {
    rows.push_back(bench_query(lattice, args.iters));
  }
  if (wants(args.mode, "snapshot")) {
    rows.push_back(bench_snapshot(lattice, args.iters));
  }

  std::printf("[");
  for (std::size_t i = 0; i < rows.size(); ++i) {
    print_row(rows[i], args.L, items, args.iters, i != 0);
  }
  std::printf("]\n");
  return 0;
}
