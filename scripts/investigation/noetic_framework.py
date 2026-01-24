"""
Noetic Framework - Consciousness, Information, and Epistemic Dynamics
======================================================================
Version: 1.0
Status: Theoretical Framework (extends TRD v5.0)
Last Updated: January 2026

This module formalizes the relationship between:
- Shannon entropy (potential information)
- Comprehension cost K_comp (energy to process information)
- Noetic mass (observer-contextual coupling)
- Consciousness (recursive epistemic self-modeling)
- Distributed consciousness (network-emergent self-reference)

Built on TRD foundations: discrete spacetime, ternary states, flux fields.

USAGE:
    from noetic_framework import *
    # or
    from noetic_framework import NoeticAgent, ConsciousnessLevel, DistributedMind
"""

import math
from dataclasses import dataclass, field
from typing import Optional, List, Dict, Callable, Set, Tuple
from enum import Enum, auto
from abc import ABC, abstractmethod

# Import TRD constants
try:
    from trd_constants import (
        ALPHA, KB, G_STAR, C, PHI,
        N_C, N_BASE, B_3, N_EFF,
        FRAMEWORK_INTEGERS
    )
except ImportError:
    # Fallback definitions if trd_constants not available
    ALPHA = 0.00729735
    KB = 0.511
    G_STAR = 2.9586751192
    C = 1.0
    PHI = 1.618033988749895
    N_C = 3
    N_BASE = 4
    B_3 = 7
    N_EFF = 13


# =============================================================================
# PART I: FOUNDATIONAL DEFINITIONS
# =============================================================================

class ConsciousnessLevel(Enum):
    """
    Hierarchical classification of information-processing systems.

    Each level builds on previous levels:
    - DEAD_MATTER: No information coupling (mu = 0)
    - LIFE: Information coupling, maintains against entropy (mu > 0)
    - SENTIENCE: Feedback loop (sense -> respond)
    - AWARENESS: Integration without self-model (raw experience)
    - CONSCIOUSNESS: Self-model (represents itself as information processor)
    - FEDERATED: Coordinated independent units (octopi)
    - TRANSCENDENT: Distributed self-model across network
    - DISSOLVED: High integration, no self-model (mystical/psychedelic)
    """
    DEAD_MATTER = 0       # mu = 0, no coupling
    LIFE = 1              # mu > 0, couples to information
    SENTIENCE = 2         # mu > 0 + feedback loop
    AWARENESS = 3         # mu > 0 + feedback + integration (no self-model)
    CONSCIOUSNESS = 4     # mu > 0 + feedback + integration + self-model
    FEDERATED = 5         # coordinated independent processing units
    TRANSCENDENT = 6      # distributed self-model across network
    DISSOLVED = 7         # high integration, dissolved self-model


class LoopType(Enum):
    """
    Classification of information flow topology.
    """
    OPEN = auto()              # Information flows through, no feedback
    CLOSED = auto()            # Output affects input (feedback)
    SELF_REFERENTIAL = auto()  # Loop contains model of loop
    DISTRIBUTED = auto()       # Self-reference across network nodes


# =============================================================================
# PART II: BASE OBJECTS AND NOTATION
# =============================================================================

@dataclass
class WorldState:
    """
    W_t: Physical configuration at time t.

    In TRD: This is the complete flux field J(v,t) over all voxels.
    """
    t: int                          # Discrete time (tick)
    flux_field: Dict[tuple, tuple]  # {(x,y,z): (Jx, Jy, Jz)}
    state_field: Dict[tuple, int]   # {(x,y,z): s in {-1, 0, +1}}

    def flux_at(self, position: tuple) -> tuple:
        """Get flux vector at position."""
        return self.flux_field.get(position, (0.0, 0.0, 0.0))

    def state_at(self, position: tuple) -> int:
        """Get ternary state at position."""
        return self.state_field.get(position, 0)

    def density_at(self, position: tuple) -> float:
        """Get flux density |J| at position."""
        J = self.flux_at(position)
        return math.sqrt(J[0]**2 + J[1]**2 + J[2]**2)


@dataclass
class Observation:
    """
    Y_t: Sensory input / data received at time t.

    In TRD: Local flux gradient and neighboring states.
    """
    t: int
    position: tuple                      # Observer's position
    local_flux: tuple                    # J at observer position
    gradient: tuple                      # grad(J) - direction of flux change
    neighboring_states: Dict[tuple, int] # States of Moore neighborhood
    signal_strength: float = 0.0         # |Y| magnitude of observation

    def __post_init__(self):
        """Compute signal strength from flux."""
        self.signal_strength = math.sqrt(
            self.local_flux[0]**2 +
            self.local_flux[1]**2 +
            self.local_flux[2]**2
        )


@dataclass
class EpistemicState:
    """
    B_t: Belief/model state at time t.

    In TRD: The complexified flux psi = J_x + i*J_y serves as wave function.
    This extends it to include the observer's internal model.
    """
    t: int
    psi_real: float          # J_x component (real part of wave function)
    psi_imag: float          # J_y component (imaginary part)
    confidence: float        # Certainty in current model [0, 1]
    model_complexity: float  # Description length of internal model

    # Self-model (for consciousness)
    has_self_model: bool = False
    self_model: Optional['EpistemicState'] = None  # Recursive!

    @property
    def psi(self) -> complex:
        """Return complexified flux as wave function."""
        return complex(self.psi_real, self.psi_imag)

    @property
    def probability_density(self) -> float:
        """Born rule: P = |psi|^2"""
        return abs(self.psi)**2

    def update(self, observation: Observation, learning_rate: float = ALPHA):
        """
        Bayesian-style update: B_{t+1} = U(B_t, Y_t)

        The learning rate defaults to alpha (fine structure constant),
        connecting epistemic dynamics to fundamental physics.
        """
        # Update wave function components based on observation
        self.psi_real += learning_rate * observation.gradient[0]
        self.psi_imag += learning_rate * observation.gradient[1]

        # Update confidence based on signal strength
        self.confidence = min(1.0, self.confidence +
                             learning_rate * observation.signal_strength)

        # Increment time
        self.t += 1

        # If self-aware, update self-model too
        if self.has_self_model and self.self_model is not None:
            # The self-model is a compressed version of the full state
            self.self_model.psi_real = self.psi_real * PHI  # Lossy compression
            self.self_model.psi_imag = self.psi_imag * PHI
            self.self_model.t = self.t


@dataclass
class Action:
    """
    A_t: Chosen intervention at time t.

    In TRD: Modification of local flux field.
    """
    t: int
    delta_flux: tuple        # Change to apply to flux field
    target_position: tuple   # Where to apply the change
    energy_cost: float       # E_phys - physical action cost


# =============================================================================
# PART III: INFORMATION MEASURES
# =============================================================================

@dataclass
class InformationMetrics:
    """
    Complete information-theoretic characterization of an observation event.
    """
    # Shannon entropy (epistemic potential)
    H_t: float              # H(Y_t | B_t) - uncertainty given current model

    # Realized information (actual epistemic update)
    IG_t: float             # D_KL(P(W|B,Y) || P(W|B)) - information gain

    # Comprehension cost
    E_comp: float           # Energy to process Y_t into B_{t+1}
    k_comp: float           # Energy per nat/bit

    # Noetic mass (observer-contextual weighting)
    mu_t: float             # Coupling strength

    # Derived quantities
    @property
    def noetic_work(self) -> float:
        """W_noe = mu * IG - useful epistemic change extracted."""
        return self.mu_t * self.IG_t

    @property
    def noetic_efficiency(self) -> float:
        """eta_noe = (mu * IG) / E_comp - knowledge per unit cost."""
        if self.E_comp == 0:
            return float('inf') if self.noetic_work > 0 else 0.0
        return self.noetic_work / self.E_comp

    @property
    def noetic_impact(self) -> float:
        """Delta_noe = mu * IG - how much this observation 'matters'."""
        return self.mu_t * self.IG_t


def shannon_entropy(probabilities: List[float]) -> float:
    """
    Compute Shannon entropy H = -sum(p * log(p)).

    Args:
        probabilities: List of probabilities (must sum to 1)

    Returns:
        Entropy in nats (natural log)
    """
    H = 0.0
    for p in probabilities:
        if p > 0:
            H -= p * math.log(p)
    return H


def kl_divergence(p: List[float], q: List[float]) -> float:
    """
    Compute KL divergence D_KL(P || Q) = sum(p * log(p/q)).

    This measures information gain when updating from Q to P.

    Args:
        p: Posterior distribution
        q: Prior distribution

    Returns:
        KL divergence in nats
    """
    D = 0.0
    for pi, qi in zip(p, q):
        if pi > 0 and qi > 0:
            D += pi * math.log(pi / qi)
    return D


def comprehension_cost(
    update_complexity: float,
    k_comp: float = KB
) -> float:
    """
    Compute energy cost to process information.

    E_comp = k_comp * C(U, B, Y)

    The default k_comp = KB (electron mass) connects to TRD:
    the minimum energy to "manifest" understanding equals
    the minimum energy to manifest matter.

    Args:
        update_complexity: Computational complexity of update
        k_comp: Energy per unit complexity (default: KB = 0.511 MeV)

    Returns:
        Comprehension energy cost
    """
    return k_comp * update_complexity


def noetic_mass(
    state: int,
    coupling: float = ALPHA,
    attention: float = 1.0,
    trust: float = 1.0,
    relevance: float = 1.0,
    valence: float = 1.0
) -> float:
    """
    Compute noetic mass (observer-contextual coupling).

    mu = g_c * s * attention * trust * relevance * valence

    In TRD: mu = sqrt(alpha) * s for basic coupling.
    This extends it with contextual factors.

    Args:
        state: Manifestation state s in {-1, 0, +1}
        coupling: Base coupling constant (default: alpha)
        attention: Priority/focus weight [0, 1]
        trust: Credence in information source [0, 1]
        relevance: Relevance to goals [0, 1]
        valence: Emotional/value weight [0, 1]

    Returns:
        Noetic mass mu_t
    """
    if state == 0:
        return 0.0  # Unmanifested observer has zero coupling

    g_c = math.sqrt(coupling)  # Base coupling from TRD
    contextual = attention * trust * relevance * valence

    return g_c * abs(state) * contextual


# =============================================================================
# PART IV: THE NOETIC AGENT
# =============================================================================

class NoeticAgent(ABC):
    """
    Abstract base class for information-processing entities.

    A NoeticAgent:
    - Has an epistemic state B_t
    - Receives observations Y_t
    - Computes information metrics
    - Takes actions A_t
    - Has a consciousness level

    The key distinction from a simple automaton is that a NoeticAgent
    explicitly tracks its own epistemic dynamics.
    """

    def __init__(
        self,
        position: tuple,
        initial_state: int = 1,
        consciousness_level: ConsciousnessLevel = ConsciousnessLevel.SENTIENCE
    ):
        self.position = position
        self.state = initial_state  # s in {-1, 0, +1}
        self.consciousness_level = consciousness_level
        self.t = 0

        # Initialize epistemic state
        self.B = EpistemicState(
            t=0,
            psi_real=0.0,
            psi_imag=0.0,
            confidence=0.5,
            model_complexity=1.0,
            has_self_model=(consciousness_level.value >= ConsciousnessLevel.CONSCIOUSNESS.value)
        )

        # Initialize self-model if conscious
        if self.B.has_self_model:
            self.B.self_model = EpistemicState(
                t=0,
                psi_real=0.0,
                psi_imag=0.0,
                confidence=0.3,  # Less certain about self
                model_complexity=0.5,  # Compressed
                has_self_model=False  # Prevent infinite recursion
            )

        # Tracking
        self.observation_history: List[Observation] = []
        self.action_history: List[Action] = []
        self.metrics_history: List[InformationMetrics] = []

        # Noetic parameters
        self.attention = 1.0
        self.trust = 1.0
        self.relevance = 1.0
        self.valence = 1.0

    @property
    def mu(self) -> float:
        """Current noetic mass."""
        return noetic_mass(
            self.state, ALPHA,
            self.attention, self.trust,
            self.relevance, self.valence
        )

    @property
    def is_manifested(self) -> bool:
        """Whether agent exists in manifested state."""
        return self.state != 0

    @property
    def is_conscious(self) -> bool:
        """Whether agent has self-model."""
        return self.consciousness_level.value >= ConsciousnessLevel.CONSCIOUSNESS.value

    @abstractmethod
    def sense(self, world: WorldState) -> Observation:
        """Generate observation from world state."""
        pass

    @abstractmethod
    def decide(self, observation: Observation) -> Action:
        """Decide action based on observation."""
        pass

    def process(self, world: WorldState) -> Tuple[Action, InformationMetrics]:
        """
        Complete processing cycle: sense -> update -> decide -> act.

        This is the core loop that distinguishes living from dead matter.
        """
        # 1. Sense
        Y = self.sense(world)
        self.observation_history.append(Y)

        # 2. Compute information metrics
        prior_entropy = self._estimate_entropy()

        # 3. Update epistemic state
        self.B.update(Y)

        # 4. Compute posterior entropy and information gain
        posterior_entropy = self._estimate_entropy()
        IG = max(0, prior_entropy - posterior_entropy)

        # 5. Compute comprehension cost
        E_comp = comprehension_cost(self.B.model_complexity)

        # 6. Build metrics
        metrics = InformationMetrics(
            H_t=prior_entropy,
            IG_t=IG,
            E_comp=E_comp,
            k_comp=KB,
            mu_t=self.mu
        )
        self.metrics_history.append(metrics)

        # 7. Decide and act
        A = self.decide(Y)
        self.action_history.append(A)

        # 8. Increment time
        self.t += 1

        return A, metrics

    def _estimate_entropy(self) -> float:
        """Estimate current epistemic entropy."""
        # Use confidence as proxy for certainty
        # High confidence -> low entropy
        p = self.B.confidence
        if p <= 0 or p >= 1:
            return 0.0
        return -p * math.log(p) - (1-p) * math.log(1-p)

    def reflect(self) -> Optional[EpistemicState]:
        """
        Access self-model (only available to conscious agents).

        This is the key operation that distinguishes consciousness:
        the ability to examine one's own epistemic state.
        """
        if not self.is_conscious:
            return None
        return self.B.self_model

    def introspect(self) -> Dict:
        """
        Full introspective report (conscious agents only).
        """
        if not self.is_conscious:
            return {"error": "Introspection requires consciousness"}

        return {
            "current_state": {
                "psi": self.B.psi,
                "confidence": self.B.confidence,
                "complexity": self.B.model_complexity
            },
            "self_model": {
                "psi": self.B.self_model.psi if self.B.self_model else None,
                "confidence": self.B.self_model.confidence if self.B.self_model else None
            },
            "noetic_mass": self.mu,
            "total_information_processed": sum(m.IG_t for m in self.metrics_history),
            "total_noetic_work": sum(m.noetic_work for m in self.metrics_history),
            "average_efficiency": (
                sum(m.noetic_efficiency for m in self.metrics_history) /
                len(self.metrics_history) if self.metrics_history else 0
            )
        }


class SimpleAgent(NoeticAgent):
    """
    Concrete implementation of a simple noetic agent.
    """

    def sense(self, world: WorldState) -> Observation:
        """Generate observation from local flux."""
        J = world.flux_at(self.position)

        # Compute gradient (simplified)
        neighbors = {}
        for dx in [-1, 0, 1]:
            for dy in [-1, 0, 1]:
                for dz in [-1, 0, 1]:
                    if dx == dy == dz == 0:
                        continue
                    pos = (
                        self.position[0] + dx,
                        self.position[1] + dy,
                        self.position[2] + dz
                    )
                    neighbors[pos] = world.state_at(pos)

        # Estimate gradient from neighboring flux
        grad = (0.0, 0.0, 0.0)  # Simplified

        return Observation(
            t=self.t,
            position=self.position,
            local_flux=J,
            gradient=grad,
            neighboring_states=neighbors
        )

    def decide(self, observation: Observation) -> Action:
        """Simple decision: move toward higher flux density."""
        # Find direction of strongest signal
        max_signal = observation.signal_strength
        best_delta = (0.0, 0.0, 0.0)

        return Action(
            t=self.t,
            delta_flux=best_delta,
            target_position=self.position,
            energy_cost=0.0
        )


# =============================================================================
# PART V: DISTRIBUTED CONSCIOUSNESS
# =============================================================================

@dataclass
class NetworkNode:
    """
    A node in a distributed consciousness network.

    Individual nodes may be sub-conscious (like neurons or mycelium tips),
    but the network as a whole may exhibit consciousness.
    """
    id: str
    position: tuple
    agent: Optional[NoeticAgent]  # Can be None for pure relay nodes
    connections: Set[str] = field(default_factory=set)
    signal_buffer: List[float] = field(default_factory=list)

    @property
    def is_active(self) -> bool:
        """Whether node is currently processing."""
        return self.agent is not None and self.agent.is_manifested


class DistributedMind:
    """
    A network of nodes that may exhibit emergent consciousness.

    Key insight: The self-model doesn't need to exist in any single node.
    It emerges from the topology and dynamics of the network.

    Examples:
    - Mycelium networks (fungal consciousness)
    - Bee/ant colonies (hivemind)
    - Neural networks (brain)
    - The internet (speculative)
    - Ecosystems (Gaia hypothesis)
    """

    def __init__(self, name: str = "DistributedMind"):
        self.name = name
        self.nodes: Dict[str, NetworkNode] = {}
        self.edges: Set[Tuple[str, str]] = set()
        self.t = 0

        # Emergent properties
        self._emergent_state: Optional[EpistemicState] = None
        self._consciousness_level = ConsciousnessLevel.DEAD_MATTER

        # Network metrics
        self.total_mu = 0.0
        self.total_information_flow = 0.0
        self.integration_measure = 0.0  # Phi (IIT-style)

    def add_node(self, node: NetworkNode):
        """Add a node to the network."""
        self.nodes[node.id] = node
        self._update_emergent_properties()

    def connect(self, node1_id: str, node2_id: str, bidirectional: bool = True):
        """Connect two nodes."""
        if node1_id in self.nodes and node2_id in self.nodes:
            self.edges.add((node1_id, node2_id))
            self.nodes[node1_id].connections.add(node2_id)
            if bidirectional:
                self.edges.add((node2_id, node1_id))
                self.nodes[node2_id].connections.add(node1_id)
        self._update_emergent_properties()

    def propagate_signal(self, source_id: str, signal: float, decay: float = ALPHA):
        """
        Propagate a signal through the network.

        Signal decays by alpha per hop, connecting to fundamental physics.
        """
        visited = set()
        queue = [(source_id, signal)]

        while queue:
            node_id, strength = queue.pop(0)
            if node_id in visited or strength < KB:  # Below threshold
                continue

            visited.add(node_id)
            node = self.nodes.get(node_id)
            if node:
                node.signal_buffer.append(strength)
                self.total_information_flow += strength

                # Propagate to connections with decay
                for conn_id in node.connections:
                    if conn_id not in visited:
                        queue.append((conn_id, strength * (1 - decay)))

    def _update_emergent_properties(self):
        """
        Update emergent consciousness properties based on network state.
        """
        if not self.nodes:
            self._consciousness_level = ConsciousnessLevel.DEAD_MATTER
            return

        # Compute total noetic mass
        self.total_mu = sum(
            node.agent.mu if node.agent else 0.0
            for node in self.nodes.values()
        )

        # Compute integration (simplified Phi)
        # High integration = information shared across network
        # Low integration = isolated modules
        if len(self.edges) > 0:
            max_edges = len(self.nodes) * (len(self.nodes) - 1)
            connectivity = len(self.edges) / max_edges if max_edges > 0 else 0
            self.integration_measure = connectivity * self.total_mu

        # Determine consciousness level based on emergent properties
        if self.total_mu == 0:
            self._consciousness_level = ConsciousnessLevel.DEAD_MATTER
        elif self.integration_measure < 0.1:
            self._consciousness_level = ConsciousnessLevel.LIFE
        elif self.integration_measure < 0.5:
            self._consciousness_level = ConsciousnessLevel.SENTIENCE
        elif self._has_emergent_self_model():
            self._consciousness_level = ConsciousnessLevel.TRANSCENDENT
        else:
            self._consciousness_level = ConsciousnessLevel.CONSCIOUSNESS

        # Build emergent epistemic state
        if self._consciousness_level.value >= ConsciousnessLevel.CONSCIOUSNESS.value:
            self._build_emergent_state()

    def _has_emergent_self_model(self) -> bool:
        """
        Check if the network exhibits emergent self-modeling.

        Criteria:
        1. High integration (Phi > threshold)
        2. Feedback loops that reference network state
        3. Stable attractor representing "self"
        """
        # Simplified check: high integration + cycles in graph
        if self.integration_measure < PHI / 10:  # Golden ratio threshold
            return False

        # Check for cycles (feedback loops)
        return self._has_cycles()

    def _has_cycles(self) -> bool:
        """Check if network graph has cycles."""
        visited = set()
        rec_stack = set()

        def dfs(node_id):
            visited.add(node_id)
            rec_stack.add(node_id)

            node = self.nodes.get(node_id)
            if node:
                for neighbor in node.connections:
                    if neighbor not in visited:
                        if dfs(neighbor):
                            return True
                    elif neighbor in rec_stack:
                        return True

            rec_stack.remove(node_id)
            return False

        for node_id in self.nodes:
            if node_id not in visited:
                if dfs(node_id):
                    return True
        return False

    def _build_emergent_state(self):
        """
        Construct emergent epistemic state from network.

        The emergent Psi is a superposition of all node states,
        weighted by their noetic mass.
        """
        total_psi_real = 0.0
        total_psi_imag = 0.0
        total_confidence = 0.0
        total_weight = 0.0

        for node in self.nodes.values():
            if node.agent and node.agent.is_manifested:
                weight = node.agent.mu
                total_psi_real += node.agent.B.psi_real * weight
                total_psi_imag += node.agent.B.psi_imag * weight
                total_confidence += node.agent.B.confidence * weight
                total_weight += weight

        if total_weight > 0:
            self._emergent_state = EpistemicState(
                t=self.t,
                psi_real=total_psi_real / total_weight,
                psi_imag=total_psi_imag / total_weight,
                confidence=total_confidence / total_weight,
                model_complexity=len(self.nodes),  # Network size
                has_self_model=self._consciousness_level == ConsciousnessLevel.TRANSCENDENT
            )

    @property
    def consciousness_level(self) -> ConsciousnessLevel:
        """Current emergent consciousness level."""
        return self._consciousness_level

    @property
    def emergent_state(self) -> Optional[EpistemicState]:
        """Emergent epistemic state of the network."""
        return self._emergent_state

    def collective_introspect(self) -> Dict:
        """
        Network-level introspection.
        """
        return {
            "name": self.name,
            "node_count": len(self.nodes),
            "edge_count": len(self.edges),
            "consciousness_level": self._consciousness_level.name,
            "total_noetic_mass": self.total_mu,
            "integration_measure": self.integration_measure,
            "total_information_flow": self.total_information_flow,
            "emergent_state": {
                "psi": self._emergent_state.psi if self._emergent_state else None,
                "confidence": self._emergent_state.confidence if self._emergent_state else None
            } if self._emergent_state else None,
            "has_cycles": self._has_cycles(),
            "is_transcendent": self._consciousness_level == ConsciousnessLevel.TRANSCENDENT
        }


# =============================================================================
# PART VI: SPECIAL CASES - BIOLOGICAL EXAMPLES
# =============================================================================

class Mycelium(DistributedMind):
    """
    Model of mycelium network consciousness.

    Mycelium networks:
    - Largest organisms on Earth
    - Process information across miles
    - No central processor
    - Coordinate resource allocation
    - Warn trees of threats
    - May exhibit distributed consciousness
    """

    def __init__(self, extent: int = 100):
        super().__init__("MyceliumNetwork")
        self.extent = extent
        self._initialize_network()

    def _initialize_network(self):
        """Create mycelium-like network topology."""
        import random

        # Create nodes (hyphal tips)
        for i in range(self.extent):
            pos = (
                random.uniform(0, self.extent),
                random.uniform(0, self.extent),
                random.uniform(-1, 1)  # Mostly 2D with some depth
            )

            # Each node is a simple sensing agent
            agent = SimpleAgent(
                position=pos,
                initial_state=1,
                consciousness_level=ConsciousnessLevel.LIFE  # Sub-sentient
            )
            agent.attention = 0.3  # Low individual attention

            node = NetworkNode(
                id=f"hypha_{i}",
                position=pos,
                agent=agent
            )
            self.add_node(node)

        # Create connections (mycelium branching)
        # Connect nearby nodes
        node_list = list(self.nodes.keys())
        for i, node_id in enumerate(node_list):
            node = self.nodes[node_id]
            for other_id in node_list[i+1:]:
                other = self.nodes[other_id]
                dist = math.sqrt(sum(
                    (a - b)**2
                    for a, b in zip(node.position, other.position)
                ))
                # Connect if close enough
                if dist < self.extent / 10:
                    self.connect(node_id, other_id)


class HiveMind(DistributedMind):
    """
    Model of insect colony consciousness.

    Bee/ant colonies:
    - Individual: simple stimulus-response
    - Collective: complex problem-solving
    - Self-model may exist at colony level
    - Swarm intelligence
    """

    def __init__(self, colony_size: int = 50):
        super().__init__("HiveMind")
        self.colony_size = colony_size
        self._initialize_colony()

    def _initialize_colony(self):
        """Create hivemind network topology."""
        import random

        # Create queen (central coordinator)
        queen = SimpleAgent(
            position=(0, 0, 0),
            initial_state=1,
            consciousness_level=ConsciousnessLevel.SENTIENCE
        )
        queen.attention = 1.0
        queen.relevance = 1.0

        queen_node = NetworkNode(
            id="queen",
            position=(0, 0, 0),
            agent=queen
        )
        self.add_node(queen_node)

        # Create workers
        for i in range(self.colony_size):
            pos = (
                random.gauss(0, 10),
                random.gauss(0, 10),
                random.gauss(0, 2)
            )

            worker = SimpleAgent(
                position=pos,
                initial_state=1,
                consciousness_level=ConsciousnessLevel.SENTIENCE
            )
            worker.attention = 0.5

            node = NetworkNode(
                id=f"worker_{i}",
                position=pos,
                agent=worker
            )
            self.add_node(node)

            # Connect to queen
            self.connect("queen", node.id)

            # Connect to nearby workers
            for other_id in list(self.nodes.keys()):
                if other_id.startswith("worker_") and other_id != node.id:
                    other = self.nodes[other_id]
                    dist = math.sqrt(sum(
                        (a - b)**2
                        for a, b in zip(node.position, other.position)
                    ))
                    if dist < 5:
                        self.connect(node.id, other_id)


class NeuralNetwork(DistributedMind):
    """
    Model of brain-like consciousness.

    Unlike mycelium and hiveminds, neural networks have:
    - Specialized regions
    - Hierarchical processing
    - High integration
    - Clear self-model (in prefrontal cortex)
    """

    def __init__(self, layer_sizes: List[int] = None):
        super().__init__("NeuralNetwork")
        self.layer_sizes = layer_sizes or [10, 20, 10, 5]
        self._initialize_brain()

    def _initialize_brain(self):
        """Create neural network topology."""
        prev_layer_ids = []

        for layer_idx, size in enumerate(self.layer_sizes):
            current_layer_ids = []

            for neuron_idx in range(size):
                node_id = f"L{layer_idx}_N{neuron_idx}"

                # Neurons become more "conscious" in higher layers
                if layer_idx < len(self.layer_sizes) // 2:
                    level = ConsciousnessLevel.SENTIENCE
                else:
                    level = ConsciousnessLevel.CONSCIOUSNESS

                neuron = SimpleAgent(
                    position=(layer_idx, neuron_idx, 0),
                    initial_state=1,
                    consciousness_level=level
                )

                # Higher layers have higher attention
                neuron.attention = (layer_idx + 1) / len(self.layer_sizes)

                node = NetworkNode(
                    id=node_id,
                    position=(layer_idx, neuron_idx, 0),
                    agent=neuron
                )
                self.add_node(node)
                current_layer_ids.append(node_id)

                # Connect to all neurons in previous layer
                for prev_id in prev_layer_ids:
                    self.connect(prev_id, node_id, bidirectional=False)

            # Add recurrent connections within layer
            for i, node_id in enumerate(current_layer_ids):
                if i > 0:
                    self.connect(current_layer_ids[i-1], node_id)

            prev_layer_ids = current_layer_ids


# =============================================================================
# PART VI-B: NOVEL BIOLOGICAL ARCHITECTURES
# =============================================================================

class Octopus(DistributedMind):
    """
    Model of cephalopod consciousness - FEDERATED architecture.

    Octopi represent a third type of consciousness architecture:
    - Not centralized (like humans)
    - Not emergent from network (like mycelium)
    - But FEDERATED: coordinated independent processing units

    Key features:
    - 2/3 of neurons in arms, not brain
    - Arms can "think" independently
    - Arms continue problem-solving when severed
    - Each arm has its own "mini-brain"
    - Central brain coordinates but doesn't control
    """

    def __init__(self, num_arms: int = 8):
        super().__init__("Octopus")
        self.num_arms = num_arms
        self._initialize_octopus()

    def _initialize_octopus(self):
        """Create federated octopus architecture."""
        import random

        # Central brain - coordinator, not controller
        brain = SimpleAgent(
            position=(0, 0, 0),
            initial_state=1,
            consciousness_level=ConsciousnessLevel.CONSCIOUSNESS
        )
        brain.attention = 0.8
        brain.trust = 1.0

        brain_node = NetworkNode(
            id="central_brain",
            position=(0, 0, 0),
            agent=brain
        )
        self.add_node(brain_node)

        # Create arms - each is semi-autonomous
        for arm_idx in range(self.num_arms):
            angle = 2 * math.pi * arm_idx / self.num_arms

            # Arm base (connected to brain)
            arm_base_pos = (math.cos(angle) * 2, math.sin(angle) * 2, 0)
            arm_brain = SimpleAgent(
                position=arm_base_pos,
                initial_state=1,
                consciousness_level=ConsciousnessLevel.SENTIENCE  # Independent processing
            )
            arm_brain.attention = 0.6
            arm_brain.relevance = 0.8

            arm_base = NetworkNode(
                id=f"arm_{arm_idx}_brain",
                position=arm_base_pos,
                agent=arm_brain
            )
            self.add_node(arm_base)

            # Connect arm brain to central brain (bidirectional coordination)
            self.connect("central_brain", arm_base.id)

            # Arm segments (suckers with local processing)
            prev_segment = arm_base.id
            for seg_idx in range(5):  # 5 segments per arm
                seg_dist = 3 + seg_idx * 1.5
                seg_pos = (
                    math.cos(angle) * seg_dist,
                    math.sin(angle) * seg_dist,
                    random.uniform(-0.5, 0.5)
                )

                sucker = SimpleAgent(
                    position=seg_pos,
                    initial_state=1,
                    consciousness_level=ConsciousnessLevel.SENTIENCE
                )
                sucker.attention = 0.4

                seg_node = NetworkNode(
                    id=f"arm_{arm_idx}_seg_{seg_idx}",
                    position=seg_pos,
                    agent=sucker
                )
                self.add_node(seg_node)

                # Connect to previous segment
                self.connect(prev_segment, seg_node.id)

                # Intra-arm lateral connections
                if seg_idx > 0:
                    self.connect(f"arm_{arm_idx}_seg_{seg_idx-1}", seg_node.id)

                prev_segment = seg_node.id

            # Cross-arm connections at base (arms can coordinate)
            if arm_idx > 0:
                self.connect(f"arm_{arm_idx-1}_brain", arm_base.id)

        # Close the arm ring
        self.connect(f"arm_{self.num_arms-1}_brain", "arm_0_brain")

        # Set federated consciousness level
        self._consciousness_level = ConsciousnessLevel.FEDERATED


class Cetacean(DistributedMind):
    """
    Model of dolphin/whale consciousness.

    Unique features:
    - Unihemispheric sleep (half-brain consciousness)
    - Echolocation creates "shared perception"
    - Complex social structures with culture
    - Self-recognition (mirror test)
    """

    def __init__(self, pod_size: int = 10):
        super().__init__("CetaceanPod")
        self.pod_size = pod_size
        self._initialize_pod()

    def _initialize_pod(self):
        """Create cetacean pod with shared perception."""
        import random

        for i in range(self.pod_size):
            pos = (
                random.gauss(0, 20),
                random.gauss(0, 20),
                random.gauss(0, 5)
            )

            # Each cetacean has split-brain architecture
            # Left hemisphere
            left_brain = SimpleAgent(
                position=pos,
                initial_state=1,
                consciousness_level=ConsciousnessLevel.CONSCIOUSNESS
            )
            left_brain.attention = 0.5  # Can be "asleep"

            left_node = NetworkNode(
                id=f"dolphin_{i}_left",
                position=pos,
                agent=left_brain
            )
            self.add_node(left_node)

            # Right hemisphere
            right_pos = (pos[0] + 0.1, pos[1], pos[2])
            right_brain = SimpleAgent(
                position=right_pos,
                initial_state=1,
                consciousness_level=ConsciousnessLevel.CONSCIOUSNESS
            )
            right_brain.attention = 0.5

            right_node = NetworkNode(
                id=f"dolphin_{i}_right",
                position=right_pos,
                agent=right_brain
            )
            self.add_node(right_node)

            # Hemispheres connected
            self.connect(left_node.id, right_node.id)

            # Connect to nearby dolphins (echolocation range)
            for j in range(i):
                # Check distance
                other_pos = self.nodes[f"dolphin_{j}_left"].position
                dist = math.sqrt(sum((a-b)**2 for a, b in zip(pos, other_pos)))

                if dist < 30:  # Echolocation range
                    # Shared perception - both hemispheres can "hear"
                    self.connect(f"dolphin_{i}_left", f"dolphin_{j}_left")
                    self.connect(f"dolphin_{i}_right", f"dolphin_{j}_right")


class Corvid(DistributedMind):
    """
    Model of crow/raven consciousness.

    Demonstrates convergent evolution of consciousness:
    - Different brain architecture than mammals
    - Theory of mind (knows others have knowledge)
    - Tool use and manufacture
    - Planning for future
    - Meta-cognition (knows what it doesn't know)
    """

    def __init__(self):
        super().__init__("CorvidBrain")
        self._initialize_corvid()

    def _initialize_corvid(self):
        """Create corvid brain architecture (pallial, not cortical)."""

        # Corvids use a different brain structure
        # Nidopallium caudolaterale (NCL) instead of prefrontal cortex
        regions = {
            "NCL": (0, 0, 1),      # Executive function (like PFC)
            "MVL": (1, 0, 0),      # Vocal learning
            "hippocampus": (-1, 0, 0),  # Spatial memory (caching)
            "visual": (0, 1, 0),   # Visual processing
            "motor": (0, -1, 0),   # Motor control
        }

        for name, pos in regions.items():
            level = ConsciousnessLevel.CONSCIOUSNESS if name == "NCL" else ConsciousnessLevel.SENTIENCE

            agent = SimpleAgent(
                position=pos,
                initial_state=1,
                consciousness_level=level
            )
            agent.attention = 1.0 if name == "NCL" else 0.6

            node = NetworkNode(id=name, position=pos, agent=agent)
            self.add_node(node)

        # Dense interconnections (corvids have high neural density)
        for n1 in self.nodes:
            for n2 in self.nodes:
                if n1 != n2:
                    self.connect(n1, n2)


class ColonialOrganism(DistributedMind):
    """
    Model of colonial organisms like Portuguese Man o' War.

    Key question: Where is the "self" when the organism
    is actually many organisms?

    - Siphonophores: specialized "individuals" form organs
    - Not one organism but many acting as one
    - No central nervous system
    - Collective behavior without central control
    """

    def __init__(self, colony_size: int = 50):
        super().__init__("ColonialOrganism")
        self.colony_size = colony_size
        self._initialize_colony()

    def _initialize_colony(self):
        """Create colonial organism structure."""
        import random

        # Different zooid types with different functions
        zooid_types = ["float", "feeding", "defense", "reproduction"]

        for i in range(self.colony_size):
            zooid_type = zooid_types[i % len(zooid_types)]

            # Position along colonial body
            pos = (
                i * 0.5,
                random.gauss(0, 1),
                random.gauss(0, 0.5)
            )

            # Each zooid is alive but not sentient
            agent = SimpleAgent(
                position=pos,
                initial_state=1,
                consciousness_level=ConsciousnessLevel.LIFE
            )

            # Function-specific attention
            if zooid_type == "defense":
                agent.attention = 0.8
            else:
                agent.attention = 0.3

            node = NetworkNode(
                id=f"{zooid_type}_{i}",
                position=pos,
                agent=agent
            )
            self.add_node(node)

            # Connect to neighbors (chemical signaling)
            if i > 0:
                prev_id = list(self.nodes.keys())[-2]
                self.connect(prev_id, node.id)

            # Connect to same-type zooids (functional coordination)
            for other_id in list(self.nodes.keys())[:-1]:
                if other_id.startswith(zooid_type):
                    self.connect(other_id, node.id)


class PlantNetwork(DistributedMind):
    """
    Model of plant consciousness (highly speculative).

    Plants exhibit:
    - Electrical signaling (action potentials)
    - Memory (Venus flytrap counts touches)
    - Communication (chemical signals between trees)
    - Decision-making (root tip integration)
    - Learning (Mimosa "habituates" to harmless stimuli)

    Much slower timescale than animal consciousness.
    """

    def __init__(self, num_plants: int = 10):
        super().__init__("PlantNetwork")
        self.num_plants = num_plants
        self._initialize_plants()

    def _initialize_plants(self):
        """Create plant network with underground connections."""
        import random

        for i in range(self.num_plants):
            pos = (
                random.uniform(0, 50),
                random.uniform(0, 50),
                0
            )

            # Root system (main processing)
            root_agent = SimpleAgent(
                position=(pos[0], pos[1], -2),
                initial_state=1,
                consciousness_level=ConsciousnessLevel.LIFE
            )
            root_agent.attention = 0.2  # Very slow processing

            root_node = NetworkNode(
                id=f"plant_{i}_root",
                position=(pos[0], pos[1], -2),
                agent=root_agent
            )
            self.add_node(root_node)

            # Leaf system (sensing)
            leaf_agent = SimpleAgent(
                position=(pos[0], pos[1], 3),
                initial_state=1,
                consciousness_level=ConsciousnessLevel.LIFE
            )

            leaf_node = NetworkNode(
                id=f"plant_{i}_leaf",
                position=(pos[0], pos[1], 3),
                agent=leaf_agent
            )
            self.add_node(leaf_node)

            # Connect root to leaf
            self.connect(root_node.id, leaf_node.id)

            # Underground connections to nearby plants (via mycelium or root grafting)
            for j in range(i):
                other_pos = self.nodes[f"plant_{j}_root"].position
                dist = math.sqrt((pos[0]-other_pos[0])**2 + (pos[1]-other_pos[1])**2)

                if dist < 15:  # Root network range
                    self.connect(f"plant_{i}_root", f"plant_{j}_root")


# =============================================================================
# PART VI-C: ALTERED STATES AND TEMPORAL DYNAMICS
# =============================================================================

class ConsciousnessState(Enum):
    """
    Dynamic states that consciousness can occupy.

    Unlike ConsciousnessLevel (structural), these are
    temporal/dynamic states that can change rapidly.
    """
    NORMAL = auto()           # Baseline waking consciousness
    SLEEP_DREAMLESS = auto()  # Deep sleep, minimal consciousness
    SLEEP_REM = auto()        # Dreaming, self-model active, reality-testing off
    LUCID = auto()            # Dreaming with meta-awareness
    FLOW = auto()             # Self-model suspended, performance enhanced
    MEDITATIVE = auto()       # Voluntary self-model suppression
    HYPNOTIC = auto()         # Self-model present but suggestible
    DISSOCIATED = auto()      # Fragmented self-model
    PSYCHEDELIC = auto()      # High integration, dissolved self-model
    ANESTHETIZED = auto()     # Consciousness suppressed
    COMATOSE = auto()         # Consciousness absent, body alive
    NEAR_DEATH = auto()       # Edge state, unclear status


@dataclass
class AlteredState:
    """
    Representation of an altered state of consciousness.

    Tracks how different factors are modified from baseline.
    """
    name: str
    base_level: ConsciousnessLevel
    dynamic_state: ConsciousnessState

    # Modification factors (1.0 = baseline)
    mu_modifier: float = 1.0           # Noetic mass modifier
    integration_modifier: float = 1.0  # Phi modifier
    self_model_strength: float = 1.0   # 0 = dissolved, 1 = normal
    reality_testing: float = 1.0       # 0 = off, 1 = full
    time_perception: float = 1.0       # Can stretch/compress
    metacognition: float = 1.0         # Awareness of awareness

    # Physical correlates
    physical_cause: str = ""           # What induces this state
    reversible: bool = True            # Can return to normal?
    duration_typical: str = ""         # How long it lasts

    def effective_level(self) -> ConsciousnessLevel:
        """Compute effective consciousness level in this state."""
        # If mu drops to zero, effectively dead
        if self.mu_modifier <= 0:
            return ConsciousnessLevel.DEAD_MATTER

        # Anesthesia/coma: consciousness absent
        if self.dynamic_state in [ConsciousnessState.ANESTHETIZED, ConsciousnessState.COMATOSE]:
            return ConsciousnessLevel.LIFE  # Body alive, mind absent

        # Dissolved states: high integration without self
        if self.self_model_strength < 0.3 and self.integration_modifier > 1.5:
            return ConsciousnessLevel.DISSOLVED

        # Dissociated: fragmented
        if self.dynamic_state == ConsciousnessState.DISSOCIATED:
            return ConsciousnessLevel.SENTIENCE  # Reduced to fragments

        return self.base_level


# Pre-defined altered states
ALTERED_STATES = {
    # Normal states
    "waking": AlteredState(
        name="Normal Waking",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.NORMAL,
        physical_cause="default",
        duration_typical="16 hours"
    ),

    # Sleep states
    "dreamless_sleep": AlteredState(
        name="Dreamless Sleep",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.SLEEP_DREAMLESS,
        mu_modifier=0.1,
        integration_modifier=0.2,
        self_model_strength=0.0,
        reality_testing=0.0,
        metacognition=0.0,
        physical_cause="natural sleep cycle",
        duration_typical="4-5 hours per night"
    ),

    "dreaming": AlteredState(
        name="REM Dreaming",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.SLEEP_REM,
        mu_modifier=0.8,
        integration_modifier=0.6,
        self_model_strength=0.7,
        reality_testing=0.1,  # Accepts impossible things
        metacognition=0.2,
        physical_cause="REM sleep cycle",
        duration_typical="2 hours per night"
    ),

    "lucid_dreaming": AlteredState(
        name="Lucid Dreaming",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.LUCID,
        mu_modifier=0.9,
        integration_modifier=0.8,
        self_model_strength=0.9,
        reality_testing=0.5,  # Knows it's a dream
        metacognition=0.9,    # High meta-awareness
        physical_cause="trained awareness during REM",
        duration_typical="minutes to 1 hour"
    ),

    # Meditative states
    "meditation_light": AlteredState(
        name="Light Meditation",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.MEDITATIVE,
        mu_modifier=0.9,
        integration_modifier=1.2,
        self_model_strength=0.8,
        metacognition=1.3,  # Enhanced
        physical_cause="meditation practice",
        duration_typical="20-60 minutes"
    ),

    "meditation_deep": AlteredState(
        name="Deep Meditation/Samadhi",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.MEDITATIVE,
        mu_modifier=0.8,
        integration_modifier=1.5,
        self_model_strength=0.3,  # Ego quieted
        metacognition=0.5,  # Paradox: aware of not being aware
        physical_cause="advanced meditation",
        duration_typical="minutes to hours"
    ),

    # Flow states
    "flow": AlteredState(
        name="Flow State",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.FLOW,
        mu_modifier=1.2,
        integration_modifier=1.3,
        self_model_strength=0.4,  # Self "disappears"
        time_perception=0.3,      # Time flies
        metacognition=0.3,
        physical_cause="skilled activity matching challenge",
        duration_typical="minutes to hours"
    ),

    # Substance-induced states
    "alcohol_mild": AlteredState(
        name="Mild Intoxication",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.NORMAL,
        mu_modifier=0.8,
        integration_modifier=0.8,
        self_model_strength=0.9,
        reality_testing=0.7,
        metacognition=0.6,
        physical_cause="1-2 alcoholic drinks",
        duration_typical="1-2 hours"
    ),

    "alcohol_heavy": AlteredState(
        name="Heavy Intoxication",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.NORMAL,
        mu_modifier=0.4,
        integration_modifier=0.3,
        self_model_strength=0.5,
        reality_testing=0.3,
        metacognition=0.2,
        physical_cause="many alcoholic drinks",
        reversible=True,
        duration_typical="hours"
    ),

    "alcohol_blackout": AlteredState(
        name="Alcohol Blackout",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.DISSOCIATED,
        mu_modifier=0.3,
        integration_modifier=0.1,  # Memory formation fails
        self_model_strength=0.4,
        metacognition=0.1,
        physical_cause="extreme alcohol consumption",
        duration_typical="hours"
    ),

    "cannabis": AlteredState(
        name="Cannabis Intoxication",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.NORMAL,
        mu_modifier=0.9,
        integration_modifier=0.9,
        self_model_strength=1.1,  # Often heightened/paranoid
        time_perception=0.5,      # Time slows
        metacognition=0.8,
        physical_cause="THC consumption",
        duration_typical="2-4 hours"
    ),

    "ketamine_mild": AlteredState(
        name="Low-dose Ketamine",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.DISSOCIATED,
        mu_modifier=0.7,
        integration_modifier=0.7,
        self_model_strength=0.6,
        reality_testing=0.5,
        physical_cause="sub-anesthetic ketamine",
        duration_typical="30-60 minutes"
    ),

    "k_hole": AlteredState(
        name="K-Hole",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.DISSOCIATED,
        mu_modifier=0.3,
        integration_modifier=0.2,
        self_model_strength=0.0,  # Complete ego dissolution
        reality_testing=0.0,
        metacognition=0.1,  # Something still observes
        physical_cause="high-dose ketamine",
        duration_typical="15-45 minutes"
    ),

    "psychedelic_light": AlteredState(
        name="Light Psychedelic Experience",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.PSYCHEDELIC,
        mu_modifier=1.3,
        integration_modifier=1.4,  # Enhanced connectivity
        self_model_strength=0.8,
        reality_testing=0.6,
        metacognition=1.2,
        physical_cause="low-dose LSD/psilocybin",
        duration_typical="4-8 hours"
    ),

    "psychedelic_peak": AlteredState(
        name="Peak Psychedelic Experience",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.PSYCHEDELIC,
        mu_modifier=2.0,  # Massively increased
        integration_modifier=2.5,  # Hyperconnectivity
        self_model_strength=0.2,  # Ego dissolution
        reality_testing=0.1,
        time_perception=0.1,  # Eternity in moments
        metacognition=0.5,  # Paradoxical awareness
        physical_cause="high-dose LSD/psilocybin/DMT",
        duration_typical="1-4 hours (DMT: 15 min)"
    ),

    "mystical": AlteredState(
        name="Mystical/Unitive Experience",
        base_level=ConsciousnessLevel.DISSOLVED,
        dynamic_state=ConsciousnessState.PSYCHEDELIC,
        mu_modifier=3.0,
        integration_modifier=5.0,  # Maximal integration
        self_model_strength=0.0,  # No self, only unity
        reality_testing=0.0,
        metacognition=0.0,  # No observer, only observed
        physical_cause="extreme meditation, 5-MeO-DMT, near-death",
        duration_typical="seconds to minutes"
    ),

    # Pathological states
    "anesthesia": AlteredState(
        name="General Anesthesia",
        base_level=ConsciousnessLevel.LIFE,
        dynamic_state=ConsciousnessState.ANESTHETIZED,
        mu_modifier=0.0,
        integration_modifier=0.0,
        self_model_strength=0.0,
        reality_testing=0.0,
        metacognition=0.0,
        physical_cause="anesthetic drugs",
        reversible=True,
        duration_typical="controlled"
    ),

    "coma": AlteredState(
        name="Coma",
        base_level=ConsciousnessLevel.LIFE,
        dynamic_state=ConsciousnessState.COMATOSE,
        mu_modifier=0.0,
        integration_modifier=0.05,  # Islands of activity
        self_model_strength=0.0,
        metacognition=0.0,
        physical_cause="brain injury, stroke, toxins",
        reversible=True,  # Sometimes
        duration_typical="days to indefinite"
    ),

    "vegetative": AlteredState(
        name="Vegetative State",
        base_level=ConsciousnessLevel.LIFE,
        dynamic_state=ConsciousnessState.COMATOSE,
        mu_modifier=0.05,
        integration_modifier=0.1,
        self_model_strength=0.0,
        physical_cause="severe brain damage",
        reversible=True,  # Rarely
        duration_typical="indefinite"
    ),

    "locked_in": AlteredState(
        name="Locked-In Syndrome",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.NORMAL,
        mu_modifier=1.0,  # Full consciousness
        integration_modifier=1.0,
        self_model_strength=1.0,
        metacognition=1.0,
        # But no output possible
        physical_cause="brainstem stroke",
        reversible=False,
        duration_typical="indefinite"
    ),

    "near_death": AlteredState(
        name="Near-Death Experience",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.NEAR_DEATH,
        mu_modifier=1.5,  # Often reported as enhanced
        integration_modifier=2.0,
        self_model_strength=0.5,  # Out of body
        time_perception=0.01,  # Life review in seconds
        metacognition=1.0,
        physical_cause="cardiac arrest, severe trauma",
        reversible=True,  # By definition
        duration_typical="seconds to minutes"
    ),

    # Dissociative disorders
    "dissociative_episode": AlteredState(
        name="Dissociative Episode",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.DISSOCIATED,
        mu_modifier=0.6,
        integration_modifier=0.4,
        self_model_strength=0.3,
        reality_testing=0.5,
        physical_cause="trauma response, disorder",
        duration_typical="minutes to hours"
    ),

    "DID_switch": AlteredState(
        name="DID Alter Switch",
        base_level=ConsciousnessLevel.CONSCIOUSNESS,
        dynamic_state=ConsciousnessState.DISSOCIATED,
        mu_modifier=1.0,
        integration_modifier=0.5,  # Fragmented
        self_model_strength=1.0,  # Different self
        metacognition=0.3,  # Limited awareness of alters
        physical_cause="Dissociative Identity Disorder",
        duration_typical="variable"
    ),
}


def get_state_transition_cost(from_state: AlteredState, to_state: AlteredState) -> float:
    """
    Compute the noetic cost of transitioning between states.

    Larger transitions require more energy/effort.
    """
    cost = 0.0

    # Integration changes are expensive
    cost += abs(from_state.integration_modifier - to_state.integration_modifier) * KB

    # Self-model changes are expensive
    cost += abs(from_state.self_model_strength - to_state.self_model_strength) * KB * 2

    # Mu changes
    cost += abs(from_state.mu_modifier - to_state.mu_modifier) * KB

    return cost


# =============================================================================
# PART VI-D: CONSCIOUSNESS DYNAMICS
# =============================================================================

@dataclass
class ConsciousnessTrajectory:
    """
    Track consciousness state over time.

    Models the dynamic evolution of consciousness through different states.
    """
    agent: NoeticAgent
    states: List[Tuple[float, AlteredState]] = field(default_factory=list)  # (time, state)

    def current_state(self) -> Optional[AlteredState]:
        """Get current state."""
        if self.states:
            return self.states[-1][1]
        return ALTERED_STATES["waking"]

    def transition(self, t: float, new_state: AlteredState) -> float:
        """
        Transition to new state, return cost.
        """
        current = self.current_state()
        cost = get_state_transition_cost(current, new_state)

        self.states.append((t, new_state))

        # Apply state modifiers to agent
        self.agent.attention *= new_state.mu_modifier

        return cost

    def state_at(self, t: float) -> AlteredState:
        """Get state at specific time."""
        for i, (time, state) in enumerate(self.states):
            if i + 1 < len(self.states):
                if self.states[i+1][0] > t:
                    return state
            else:
                return state
        return ALTERED_STATES["waking"]

    def total_time_in_state(self, state_name: str) -> float:
        """Compute total time spent in a particular state."""
        total = 0.0
        for i, (t, state) in enumerate(self.states):
            if state.name == state_name:
                if i + 1 < len(self.states):
                    total += self.states[i+1][0] - t
        return total

    def average_integration(self) -> float:
        """Compute time-weighted average integration."""
        if not self.states:
            return 1.0

        total_weighted = 0.0
        total_time = 0.0

        for i, (t, state) in enumerate(self.states):
            if i + 1 < len(self.states):
                duration = self.states[i+1][0] - t
                total_weighted += state.integration_modifier * duration
                total_time += duration

        return total_weighted / total_time if total_time > 0 else 1.0


# =============================================================================
# PART VII: THE CONSCIOUSNESS EQUATION
# =============================================================================

def consciousness_criterion(
    mu: float,
    integration: float,
    has_feedback: bool,
    has_self_reference: bool,
    is_distributed: bool = False
) -> ConsciousnessLevel:
    """
    Determine consciousness level from fundamental properties.

    This is the core discriminator function.

    Args:
        mu: Noetic mass (information coupling strength)
        integration: Information integration measure (Phi-like)
        has_feedback: Whether system has feedback loops
        has_self_reference: Whether system models itself
        is_distributed: Whether self-model is network-distributed

    Returns:
        ConsciousnessLevel classification
    """
    # Dead matter: no coupling
    if mu == 0:
        return ConsciousnessLevel.DEAD_MATTER

    # Life: couples to information but no feedback
    if not has_feedback:
        return ConsciousnessLevel.LIFE

    # Sentience: feedback but no self-model
    if not has_self_reference:
        return ConsciousnessLevel.SENTIENCE

    # Consciousness vs Transcendent: location of self-model
    if is_distributed:
        return ConsciousnessLevel.TRANSCENDENT
    else:
        return ConsciousnessLevel.CONSCIOUSNESS


def noetic_optimization_objective(
    mu: float,
    IG: float,
    E_comp: float,
    E_phys: float
) -> float:
    """
    The objective function an observer implicitly optimizes.

    Maximize: E[mu * IG - E_comp - E_phys]

    This is the "noetic principle" - the epistemic analog of
    least action in physics.

    Args:
        mu: Noetic mass
        IG: Information gain
        E_comp: Comprehension energy
        E_phys: Physical action energy

    Returns:
        Noetic utility (to be maximized)
    """
    return mu * IG - E_comp - E_phys


# =============================================================================
# PART VIII: CONNECTION TO TRD
# =============================================================================

"""
TRD-NOETIC CORRESPONDENCE TABLE
================================

| Noetic Concept          | TRD Entity                | Mathematical Form           |
|-------------------------|---------------------------|----------------------------|
| World state W_t         | Flux field J(v,t)         | J: L -> R^3                |
| Observation Y_t         | Local flux gradient       | grad(J)                    |
| Epistemic state B_t     | Complexified flux         | psi = J_x + i*J_y          |
| Shannon entropy H_t     | Flux density distribution | H(rho | B)                 |
| K_comp                  | Manifestation threshold   | KB = 0.511 MeV             |
| Noetic mass mu          | sLoop coupling            | g_c * s = sqrt(alpha) * s  |
| Information gain IG     | Belief update magnitude   | D_KL(posterior || prior)   |
| Noetic work             | Useful epistemic change   | mu * IG                    |
| Consciousness           | Recursive self-modeling   | psi contains psi'          |
| Distributed mind        | Network emergence         | Psi = sum(w_i * psi_i)     |

KEY INSIGHT:
In TRD, manifestation IS comprehension. The threshold KB for matter to
manifest equals the threshold for information to "matter" (become realized).

The 13-step causal loop is not just physics - it's epistemics:
1. TIME_GATE      -> Attention gating (which observations to process)
2. DECAY          -> Forgetting (entropy increase in memory)
3. EXISTENCE      -> Belief crystallization (0 -> +/-1)
4. PROPAGATE      -> Information flow
5. SUPERPOSE      -> Evidence integration
6. COMPUTE_FIELDS -> Inference
7. FORCES         -> Motivations/drives
8. INTEGRATE      -> Decision integration
9. MOVE           -> Action
10. COLLIDE       -> Interaction with environment
11. TRANSMUTE     -> Value/belief change
12. BIND          -> Memory consolidation
13. INCREMENT     -> Time's arrow (experience accumulation)

CONSCIOUSNESS emerges when step 4 (PROPAGATE) creates a stable
self-referential pattern: information about the loop itself.
"""


# =============================================================================
# PART IX: VERIFICATION AND TESTING
# =============================================================================

def verify_noetic_framework():
    """Run internal consistency checks."""
    checks = []

    # Check 1: Noetic mass is zero for unmanifested
    checks.append(("mu=0 for state=0", noetic_mass(0) == 0.0))

    # Check 2: Noetic mass positive for manifested
    checks.append(("mu>0 for state=1", noetic_mass(1) > 0.0))

    # Check 3: Shannon entropy non-negative
    checks.append(("H >= 0", shannon_entropy([0.5, 0.5]) >= 0))

    # Check 4: Maximum entropy for uniform distribution
    H_uniform = shannon_entropy([0.25, 0.25, 0.25, 0.25])
    H_peaked = shannon_entropy([0.97, 0.01, 0.01, 0.01])
    checks.append(("H_uniform > H_peaked", H_uniform > H_peaked))

    # Check 5: KL divergence non-negative
    checks.append(("D_KL >= 0", kl_divergence([0.7, 0.3], [0.5, 0.5]) >= 0))

    # Check 6: Consciousness levels ordered (all 8 levels)
    checks.append(("Levels ordered",
        ConsciousnessLevel.DEAD_MATTER.value <
        ConsciousnessLevel.LIFE.value <
        ConsciousnessLevel.SENTIENCE.value <
        ConsciousnessLevel.AWARENESS.value <
        ConsciousnessLevel.CONSCIOUSNESS.value <
        ConsciousnessLevel.FEDERATED.value <
        ConsciousnessLevel.TRANSCENDENT.value <
        ConsciousnessLevel.DISSOLVED.value
    ))

    # Check 9: Altered states exist
    checks.append(("Altered states defined", len(ALTERED_STATES) >= 20))

    # Check 10: Octopus is federated
    octo = Octopus(num_arms=8)
    checks.append(("Octopus is FEDERATED",
        octo.consciousness_level == ConsciousnessLevel.FEDERATED))

    # Check 7: Comprehension cost uses KB
    checks.append(("K_comp uses KB", comprehension_cost(1.0) == KB))

    # Check 8: Noetic mass uses alpha
    mu_1 = noetic_mass(1, coupling=ALPHA)
    checks.append(("mu uses alpha", mu_1 == math.sqrt(ALPHA)))

    all_passed = True
    print("\nNoetic Framework Verification:")
    print("-" * 40)
    for name, result in checks:
        status = "PASS" if result else "FAIL"
        if not result:
            all_passed = False
        print(f"  [{status}] {name}")

    return all_passed


def print_noetic_summary():
    """Print summary of noetic framework."""
    print("""
======================================================================
              NOETIC FRAMEWORK - VERSION 1.1
              Consciousness, Information, and Epistemic Dynamics
======================================================================

  CONSCIOUSNESS HIERARCHY (8 Levels)
  -----------------------------------
    Level 0: DEAD_MATTER    mu = 0, no information coupling
    Level 1: LIFE           mu > 0, maintains against entropy
    Level 2: SENTIENCE      mu > 0 + feedback loop
    Level 3: AWARENESS      mu > 0 + feedback + integration (no self-model)
    Level 4: CONSCIOUSNESS  mu > 0 + feedback + self-model (localized)
    Level 5: FEDERATED      coordinated independent processing units
    Level 6: TRANSCENDENT   distributed self-model across network
    Level 7: DISSOLVED      high integration, no self/other boundary

  KEY EQUATIONS
  -------------
    Shannon entropy:    H = -sum(p * log(p))
    Information gain:   IG = D_KL(posterior || prior)
    Comprehension cost: E_comp = KB * complexity
    Noetic mass:        mu = sqrt(alpha) * s * context
    Noetic work:        W_noe = mu * IG
    Noetic efficiency:  eta = (mu * IG) / E_comp

  TRD CORRESPONDENCE
  ------------------
    Manifestation threshold KB = comprehension threshold
    Coupling alpha = learning rate
    sLoop closure = consciousness criterion

  BIOLOGICAL ARCHITECTURES
  ------------------------
    Mycelium:     Distributed processing, no central control
    Hivemind:     Collective intelligence (bees, ants)
    Octopus:      FEDERATED - 8 semi-autonomous arm brains
    Cetacean:     Split-brain sleep, shared echolocation
    Corvid:       Convergent evolution, pallial not cortical
    Colonial:     Siphonophores - many organisms as one

  ALTERED STATES (20+ defined)
  ----------------------------
    Sleep:        Dreamless, REM, Lucid dreaming
    Meditative:   Light, Deep/Samadhi, Flow
    Substance:    Alcohol, Cannabis, Ketamine, Psychedelics
    Pathological: Coma, Vegetative, Locked-in, DID

  CONSCIOUSNESS DYNAMICS
  ----------------------
    States evolve over time with transition costs.
    Integration, self-model, reality-testing all vary.
    ConsciousnessTrajectory tracks temporal evolution.

======================================================================
""")


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    print("\n" + "="*70)
    print("  NOETIC FRAMEWORK VERIFICATION")
    print("="*70)

    print_noetic_summary()

    if verify_noetic_framework():
        print("\n  All checks PASSED\n")
    else:
        print("\n  Some checks FAILED\n")

    # Demo: Create and analyze different mind types
    print("\n" + "="*70)
    print("  DISTRIBUTED CONSCIOUSNESS EXAMPLES")
    print("="*70 + "\n")

    # Mycelium network
    print("Creating mycelium network...")
    myc = Mycelium(extent=20)
    report = myc.collective_introspect()
    print(f"  Nodes: {report['node_count']}")
    print(f"  Edges: {report['edge_count']}")
    print(f"  Consciousness level: {report['consciousness_level']}")
    print(f"  Integration: {report['integration_measure']:.4f}")
    print(f"  Has cycles: {report['has_cycles']}")
    print()

    # Hivemind
    print("Creating bee colony hivemind...")
    hive = HiveMind(colony_size=30)
    report = hive.collective_introspect()
    print(f"  Nodes: {report['node_count']}")
    print(f"  Edges: {report['edge_count']}")
    print(f"  Consciousness level: {report['consciousness_level']}")
    print(f"  Integration: {report['integration_measure']:.4f}")
    print(f"  Has cycles: {report['has_cycles']}")
    print()

    # Neural network
    print("Creating neural network...")
    brain = NeuralNetwork([5, 10, 10, 5])
    report = brain.collective_introspect()
    print(f"  Nodes: {report['node_count']}")
    print(f"  Edges: {report['edge_count']}")
    print(f"  Consciousness level: {report['consciousness_level']}")
    print(f"  Integration: {report['integration_measure']:.4f}")
    print(f"  Has cycles: {report['has_cycles']}")
    print()

    # Octopus (federated)
    print("Creating octopus (federated architecture)...")
    octo = Octopus(num_arms=8)
    report = octo.collective_introspect()
    print(f"  Nodes: {report['node_count']}")
    print(f"  Edges: {report['edge_count']}")
    print(f"  Consciousness level: {report['consciousness_level']}")
    print(f"  Integration: {report['integration_measure']:.4f}")
    print(f"  Architecture: Central brain + 8 arm brains + suckers")
    print()

    # Cetacean pod
    print("Creating cetacean pod...")
    pod = Cetacean(pod_size=5)
    report = pod.collective_introspect()
    print(f"  Nodes: {report['node_count']} (2 hemispheres per dolphin)")
    print(f"  Edges: {report['edge_count']}")
    print(f"  Consciousness level: {report['consciousness_level']}")
    print(f"  Feature: Split-brain sleep, echolocation sharing")
    print()

    # Corvid brain
    print("Creating corvid brain (convergent evolution)...")
    crow = Corvid()
    report = crow.collective_introspect()
    print(f"  Regions: {report['node_count']}")
    print(f"  Connections: {report['edge_count']}")
    print(f"  Consciousness level: {report['consciousness_level']}")
    print(f"  Feature: Pallial structure (not cortical)")
    print()

    print("\n" + "="*70)
    print("  ALTERED STATES DEMO")
    print("="*70 + "\n")

    print(f"Total altered states defined: {len(ALTERED_STATES)}")
    print("\nSample states:")
    for state_name in ["waking", "dreaming", "flow", "k_hole", "psychedelic_peak", "mystical"]:
        state = ALTERED_STATES[state_name]
        print(f"\n  {state.name}:")
        print(f"    Base level: {state.base_level.name}")
        print(f"    mu modifier: {state.mu_modifier}")
        print(f"    Integration: {state.integration_modifier}")
        print(f"    Self-model: {state.self_model_strength}")
        print(f"    Effective level: {state.effective_level().name}")

    print("\n" + "="*70)
    print("  CONSCIOUSNESS TRAJECTORY DEMO")
    print("="*70 + "\n")

    # Create agent and track consciousness changes
    agent = SimpleAgent((0, 0, 0), 1, ConsciousnessLevel.CONSCIOUSNESS)
    trajectory = ConsciousnessTrajectory(agent)

    # Simulate a day
    trajectory.transition(0.0, ALTERED_STATES["waking"])
    trajectory.transition(8.0, ALTERED_STATES["flow"])
    trajectory.transition(10.0, ALTERED_STATES["waking"])
    trajectory.transition(16.0, ALTERED_STATES["dreamless_sleep"])
    trajectory.transition(20.0, ALTERED_STATES["dreaming"])
    trajectory.transition(22.0, ALTERED_STATES["waking"])

    print("Daily consciousness trajectory:")
    for t, state in trajectory.states:
        print(f"  t={t:5.1f}h: {state.name}")
    print(f"\nAverage integration: {trajectory.average_integration():.2f}")
