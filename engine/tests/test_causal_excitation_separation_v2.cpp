// FTD-0685: sole correction is a numeric center-preflight tolerance.
#define FTD_SEPARATION_PROTOCOL_SHA256 \
  "FEDD4A5B09DBA6443A34159D9563456E652BA2B4060643A2693511621EED95DF"
#define FTD_SEPARATION_IDENTIFIER "FTD-0685"
#define FTD_SEPARATION_RESULT_DIRECTORY "results/ftd_0685"
#define FTD_SEPARATION_RESULT_STEM "ftd_0685_causal_excitation_separation_v2"
#define FTD_SEPARATION_CENTER_TOLERANCE 1e-12
#include "test_causal_excitation_separation_v1.cpp"
