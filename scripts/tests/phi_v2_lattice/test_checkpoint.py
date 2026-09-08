import base64
import json
import pytest
from phi_v2_lattice import state as S, staged as P


@pytest.mark.parametrize("phase", range(4))
def test_complete_checkpoint_roundtrip_and_identical_continuation(phase):
    st = S.blank(3)
    st.bank[0, [0, 34]] = True
    a = P.initialize(st)
    for _ in range(phase): a, _ = P.step(a)
    blob = P.checkpoint(a)
    b = P.restore(blob)
    assert P.checkpoint(b) == blob
    for _ in range(8):
        a, ae = P.step(a)
        b, be = P.step(b)
        assert ae == be
        assert P.checkpoint(a) == P.checkpoint(b)


@pytest.mark.parametrize("key", ["law", "schema", "collision", "encoding", "boundary"])
def test_incompatible_identity_is_rejected(key):
    payload = json.loads(P.checkpoint(P.initialize(S.blank(3))))
    payload[key] = "different"
    with pytest.raises(ValueError): P.restore(json.dumps(payload).encode())


def test_missing_arrays_lengths_boolean_encoding_and_duplicate_keys_rejected():
    original = P.checkpoint(P.initialize(S.blank(3)))
    payload = json.loads(original); del payload["arrays"]["ell"]
    with pytest.raises(ValueError): P.restore(json.dumps(payload).encode())
    payload = json.loads(original); payload["arrays"]["bank"] = ""
    with pytest.raises(ValueError): P.restore(json.dumps(payload).encode())
    payload = json.loads(original)
    raw = bytearray(base64.b64decode(payload["arrays"]["bank"])); raw[0] = 2
    payload["arrays"]["bank"] = base64.b64encode(raw).decode()
    with pytest.raises(ValueError): P.restore(json.dumps(payload).encode())
    with pytest.raises(ValueError): P.restore(original.replace(b'"L":3', b'"L":3,"L":3'))
    with pytest.raises(ValueError): P.restore(b"garbage")


@pytest.mark.parametrize("name", ["bank", "admitted_sc", "gate_sc", "gate_fcc"])
def test_noncanonical_boolean_storage_rejected_before_checkpoint(name):
    state = P.initialize(S.blank(3))
    target = state.lattice if name == "bank" else state
    getattr(target, name).view("uint8").flat[0] = 2
    with pytest.raises(ValueError, match="noncanonical boolean"):
        P.validate(state)
    with pytest.raises(ValueError, match="noncanonical boolean"):
        P.checkpoint(state)
