"""Import shim: the certified cell-level functions from scripts/proofs. Never re-implement them."""
from __future__ import annotations
import sys
from pathlib import Path

PROOFS = Path(__file__).resolve().parents[1] / "proofs"
if str(PROOFS) not in sys.path:
    sys.path.insert(0, str(PROOFS))

from proof_v3_common_action_phi_v2 import (  # noqa: E402
    A9, BLANK, PHASES, readout, rotate, phase_index, encode, relation_tick)
from proof_hodge_flag_pair_collision_invariant_space import (  # noqa: E402
    one_particle_states, field_value, PHASE_COORDINATES)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import (  # noqa: E402
    internal_tick, layer_value)
from proof_shared_edge_hodge_flag_bcc_propagation import SC_DIRECTIONS  # noqa: E402
import proof_global_c3_cotangent_layer_equivariant_collision as _collision  # noqa: E402


def collision_main():
    _collision.main()


def collision_data():
    return _collision.CERTIFICATE_DATA
