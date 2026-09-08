from phi_v2_lattice.hydro import invariants as I, channels as H


def test_fixed_kernel_is_number_and_momentum_only():
    basis = I.fixed_kernel()
    assert len(basis) == 4
    rows = [(1,) * 24] + [tuple(v[a] for v in H.VELOCITIES) for a in range(3)]
    assert I.span_membership(basis, rows) and I.span_membership(rows, basis)


def test_census_reports_isotropy_and_dimension():
    c = I.census()
    assert c["fixed_dimension"] == 4 and c["isotropy"]["isotropic4"] is True and c["table_hash"] == H.TABLE_HASH
