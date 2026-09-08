"""Declared preparations: frozen doubly-occupied background plus a Bernoulli fluid."""
from __future__ import annotations
from fractions import Fraction
import numpy as np
from .. import geometry as G
from .._proofs import encode
from . import channels as H, state as S

C2 = Fraction(sum(sum(x * x for x in v) for v in H.VELOCITIES), 24)  # mean |v|^2 = 3/2


def frozen_background(L: int) -> S.LatticeState:
    return with_background(S.blank(L))


def with_background(st: S.LatticeState) -> S.LatticeState:
    z = S.idx_of(encode(0, +1))
    st.sc[:] = z; st.fcc[:] = z
    return st


def occupancy_probabilities(L: int, density, drift=None, base=None):
    """p[x, v] = base[v] (1 + u(x) . v / C2), clipped to [0,1]; base defaults to density for every v."""
    coords = np.array([G.coords(L, i) for i in range(L ** 3)], dtype=float)
    u = np.zeros((L ** 3, 3)) if drift is None else np.asarray(drift(coords), dtype=float)
    V = np.array(H.VELOCITIES, dtype=float)
    base = np.full(24, float(density)) if base is None else np.asarray(base, dtype=float)
    return np.clip(base[None, :] * (1.0 + (u @ V.T) / float(C2)), 0.0, 1.0)


def fluid(L: int, seed: int, density, pol: int = 0, drift=None, base=None) -> S.LatticeState:
    """Bernoulli fluid on the frozen background; phase labels drawn uniformly."""
    rng = np.random.default_rng(seed)
    st = frozen_background(L)
    probs = occupancy_probabilities(L, density, drift, base)
    occupied = rng.random((L ** 3, 24)) < probs
    phases = rng.integers(0, 4, size=(L ** 3, 24))
    xs, vs = np.nonzero(occupied)
    for x, v in zip(xs.tolist(), vs.tolist()):
        st.bank[x, H.channel(pol, int(phases[x, v]), v)] = True
    return st


def _k(direction, m, L):
    return 2 * np.pi * m * np.array(direction, dtype=float) / L


def shear_wave(L, seed, density, eps, direction=(1, 0, 0), m=1, transverse=(0, 1, 0), base=None, background=None):
    k = _k(direction, m, L); t = np.array(transverse, dtype=float)
    u0 = np.zeros(3) if background is None else np.asarray(background, dtype=float)
    return fluid(L, seed, density, base=base,
                 drift=lambda c: u0[None, :] + float(eps) * np.cos(c @ k)[:, None] * t[None, :])


def sound_wave(L, seed, density, eps, direction=(1, 0, 0), m=1):
    k = _k(direction, m, L); n = np.array(direction, dtype=float); n /= np.linalg.norm(n)
    return fluid(L, seed, density, drift=lambda c: float(eps) * np.cos(c @ k)[:, None] * n[None, :])


def density_wave(L, seed, density, eps, direction=(1, 0, 0), m=1):
    k = _k(direction, m, L)
    st = frozen_background(L)
    rng = np.random.default_rng(seed)
    coords = np.array([G.coords(L, i) for i in range(L ** 3)], dtype=float)
    probs = np.clip(float(density) * (1.0 + float(eps) * np.cos(coords @ k)), 0.0, 1.0)
    occupied = rng.random((L ** 3, 24)) < probs[:, None]
    phases = rng.integers(0, 4, size=(L ** 3, 24))
    xs, vs = np.nonzero(occupied)
    for x, v in zip(xs.tolist(), vs.tolist()):
        st.bank[x, H.channel(0, int(phases[x, v]), v)] = True
    return st


def taylor_green(L, seed, density, eps):
    kx = 2 * np.pi / L
    def drift(c):
        u = np.zeros_like(c)
        u[:, 0] = float(eps) * np.sin(kx * c[:, 0]) * np.cos(kx * c[:, 1])
        u[:, 1] = -float(eps) * np.cos(kx * c[:, 0]) * np.sin(kx * c[:, 1])
        return u
    return fluid(L, seed, density, drift=drift)


def shear_layer(L, seed, density, eps):
    def drift(c):
        u = np.zeros_like(c); u[:, 0] = float(eps) * np.where(c[:, 1] < L / 2, 1.0, -1.0); return u
    return fluid(L, seed, density, drift=drift)


def vortex_pair(L, seed, density, eps):
    kx = 2 * np.pi / L
    def drift(c):
        u = np.zeros_like(c)
        u[:, 0] = float(eps) * np.sin(kx * c[:, 1]) * np.cos(kx * c[:, 2])
        u[:, 2] = -float(eps) * np.cos(kx * c[:, 1]) * np.sin(kx * c[:, 2])
        return u
    return fluid(L, seed, density, drift=drift)
