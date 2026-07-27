/**
 * FTD-0460 v2 wrapper: preserve the locked v1 campaign body while replacing
 * only its pathological observer tick with the algebraically identical
 * snapshot implementation.
 */
#include "ftd/eft/coupled_wave_tick.h"
#include "ftd/eft/coupled_wave_tick_snapshot.h"

#define advance_coupled_wave_tick advance_coupled_wave_tick_snapshot
#define reverse_coupled_wave_tick reverse_coupled_wave_tick_snapshot
#include "campaign_blocked_hop_work_decomposition.cpp"
