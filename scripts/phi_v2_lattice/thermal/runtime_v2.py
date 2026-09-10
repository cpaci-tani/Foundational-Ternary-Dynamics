"""Frozen equivariant successor; shares record encoding and streaming algorithms.

The collision law and checkpoint identity are versioned. This module is a
reference implementation, never another dashboard owner or scenario script.
"""
from . import runtime as _base
from .runtime import Records, VELOCITIES, CHANNELS, ENERGY2, MAX_TICK, channel, occupied, validate, totals, manifestation
from .equivariant import LAW_ID, collision_map, collision_identity


def step(state: Records) -> None:
    validate(state)
    _base._step(state, collision_map() if state.phase == 0 else None)


def advance(state: Records, microticks: int) -> None:
    validate(state)
    if type(microticks) is not int or not 0 <= microticks <= MAX_TICK-state.microtick:
        raise ValueError("invalid advance interval")
    candidate = Records(state.L, state.microtick, state.bank.copy())
    for _ in range(microticks):
        step(candidate)
    state.bank, state.microtick = candidate.bank, candidate.microtick


def encode(state: Records) -> bytes:
    return _base._encode(state, LAW_ID)


def decode(data: bytes) -> Records:
    return _base._decode(data, LAW_ID)
