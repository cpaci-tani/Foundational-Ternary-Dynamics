// FTD-0687: exact batched-regional execution of the frozen campaign.
#define FTD_SEPARATION_PROTOCOL_SHA256 \
  "BA1800AFB9E1B9B5715DD3A4A89908963E6BB1E6443C1C535F5D8DE1BF86D3CB"
#define FTD_SEPARATION_IDENTIFIER "FTD-0687"
#define FTD_SEPARATION_RESULT_DIRECTORY "results/ftd_0687"
#define FTD_SEPARATION_RESULT_STEM "ftd_0687_causal_excitation_separation_v3"
#define FTD_SEPARATION_CENTER_TOLERANCE 1e-12
#define FTD_SEPARATION_USE_BATCHED_REGIONAL 1
#include "test_causal_excitation_separation_v1.cpp"
