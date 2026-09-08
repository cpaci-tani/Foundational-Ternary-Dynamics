import pytest

from phi_v2_lattice.hydro import codec as N_, prepare as R, staged as S


@pytest.mark.parametrize("L", [3, 4])
def test_native_transport_round_trip_and_length(L):
    state = S.initialize(R.fluid(L, seed=1, density=1 / 4))
    blob = N_.encode(state)
    assert len(blob) == N_.HEADER.size + N_.BYTES_PER_SITE * L ** 3 and N_.BYTES_PER_SITE == 229
    back = N_.decode(blob)
    assert (back.lattice.bank == state.lattice.bank).all() and back.microtick == state.microtick


def test_decode_rejects_wrong_magic():
    state = S.initialize(R.fluid(3, seed=1, density=1 / 4))
    blob = bytearray(N_.encode(state))
    blob[0] ^= 1
    with pytest.raises(ValueError):
        N_.decode(bytes(blob))


def test_json_checkpoint_round_trip():
    state = S.initialize(R.fluid(3, seed=2, density=1 / 4))
    back = N_.restore(N_.checkpoint(state))
    assert (back.lattice.bank == state.lattice.bank).all()
