"""State-completeness includes rejecting behavior absent from checkpoint bytes."""
import numpy as np
import pytest

from phi_v2_lattice import channels as C, native_codec as N, runtime as R
from phi_v2_lattice import staged as P, state as S


class HiddenArray(np.ndarray):
    change = False

    def copy(self, *args, **kwargs):
        result = super().copy(*args, **kwargs)
        if self.change:
            result.flat[0] = not result.flat[0]
        return result


class HiddenInteger(int):
    def __int__(self):
        return super().__int__() + 4


class HiddenBytes(bytes):
    def __len__(self):
        return super().__len__() - 1


class HiddenTable(dict):
    def __getitem__(self, key):
        return key  # items/hash remain the original table; collision changes.


@pytest.mark.parametrize("name", N.NAMES)
def test_every_complete_array_rejects_subclass_behavior(name):
    state = P.initialize(S.blank(3))
    owner = state.lattice if name in N.NAMES[:5] else state
    setattr(owner, name, getattr(owner, name).view(HiddenArray))
    for operation in (P.validate, P.checkpoint, N.encode, P.step):
        with pytest.raises(ValueError):
            operation(state)


def test_initialization_rejects_callbacks_before_copying():
    lattice = S.blank(3)
    lattice.bank = lattice.bank.view(HiddenArray)
    lattice.bank.change = True
    with pytest.raises(ValueError):
        P.initialize(lattice)
    with pytest.raises(ValueError):
        R.StrictRuntime(lattice)


@pytest.mark.parametrize("where", ["state", "lattice", "clock", "side"])
def test_record_and_integer_subclasses_are_not_ontic_state(where):
    state = P.initialize(S.blank(3))
    if where == "state":
        class Derived(P.StagedState):
            pass
        state = Derived(**vars(state))
    elif where == "lattice":
        class Derived(S.LatticeState):
            pass
        state.lattice = Derived(**vars(state.lattice))
    elif where == "clock":
        state.microtick = HiddenInteger(0)
    else:
        state.lattice.L = HiddenInteger(3)
    with pytest.raises(ValueError):
        P.validate(state)


@pytest.mark.parametrize("scalar", [int, np.int32, np.int64, np.uint64])
def test_concrete_integer_scalars_continue_identically(scalar):
    state = P.initialize(S.blank(3))
    state.microtick = scalar(0)
    state.lattice.L = scalar(3)
    out, _ = P.step(state)
    assert out.microtick == 1


def test_collision_lookup_cannot_disagree_with_hashed_table():
    state, _ = P.step(P.initialize(S.blank(3)))
    tables = tuple(HiddenTable(table) for table in C.load_collision_tables())
    assert C._hash_tables(tables) == C.COLLISION_HASH
    with pytest.raises(ValueError):
        P.step(state, tables)


def test_table_leaf_subclasses_rejected_before_hashing():
    state, _ = P.step(P.initialize(S.blank(3)))
    tables = tuple(dict(table) for table in C.load_collision_tables())
    key = next(iter(tables[0]))
    a, b = tables[0][key]
    tables[0][key] = (HiddenInteger(a), b)
    assert C._hash_tables(tables) == C.COLLISION_HASH
    with pytest.raises(ValueError):
        P.step(state, tables)


def test_runtime_rejects_behavior_bearing_request_scalars_without_mutation():
    owner = R.StrictRuntime(S.blank(3))
    before = owner.checkpoint()
    for operation in (lambda: owner.advance(HiddenInteger(1)),
                      lambda: owner.observe(HiddenInteger(1))):
        with pytest.raises(ValueError):
            operation()
        assert owner.checkpoint() == before


def test_bytes_subclasses_do_not_enter_checkpoint_decoders():
    state = P.initialize(S.blank(3))
    for decode, raw in ((P.restore, P.checkpoint(state)), (N.decode, N.encode(state))):
        with pytest.raises(ValueError):
            decode(HiddenBytes(raw))


def test_default_collision_table_is_immutable():
    table = P._default_tables()[0]
    key = next(iter(table))
    with pytest.raises(TypeError):
        table[key] = key


def test_scalar_type_whitelist_uses_identity_not_metaclass_equality():
    class Spoof(type):
        def __hash__(cls):
            return hash(int)

        def __eq__(cls, other):
            return other is int

    class Value(int, metaclass=Spoof):
        pass

    state = P.initialize(S.blank(3))
    state.microtick = Value(0)
    with pytest.raises(ValueError):
        P.validate(state)
