"""Optional corruption detection for opaque immutable checkpoint bytes.

No authentication, physics identity or historical provenance is inferred.
Original checkpoint formats are unchanged; only callers of this API opt in.
"""
import hashlib
import struct


MAGIC = b"FTDIN01\0"
HEADER = struct.Struct("<8sQ32s")


def pack(payload: bytes) -> bytes:
    if type(payload) is not bytes:
        raise ValueError("checkpoint payload must be concrete immutable bytes")
    return HEADER.pack(MAGIC, len(payload), hashlib.sha256(payload).digest()) + payload


def unpack(envelope: bytes) -> bytes:
    if type(envelope) is not bytes or len(envelope) < HEADER.size:
        raise ValueError("incomplete checkpoint integrity envelope")
    magic, length, digest = HEADER.unpack_from(envelope)
    if magic != MAGIC or len(envelope) - HEADER.size != length:
        raise ValueError("checkpoint integrity schema or length mismatch")
    payload = envelope[HEADER.size:]
    if hashlib.sha256(payload).digest() != digest:
        raise ValueError("checkpoint integrity digest mismatch")
    return payload
