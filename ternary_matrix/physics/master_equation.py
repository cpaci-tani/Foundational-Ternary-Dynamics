"""
FTD Master Equation
Implements the 12-Phase Update Cycle defined in Chapter 5.
"""
import numpy as np
from ..config import CONSTANTS
from . import waves, forces, interactions, binding

def apply_decay(universe):
    """
    Phase 2: Entropy
    Apply decay to unlocked manifested voxels.
    """
    # Decay applied flux. 
    # If locked, decay is suppressed (Phase 11 logic applied in Phase 2 context)
    
    decay_factor = 1.0 - CONSTANTS.DECAY_RATE
    
    # Apply to all flux
    # In full FTD, decay might be selective. Here, global damping is in 'waves'.
    # Ch 4.3 says: if not is_locked: flux *= (1 - gamma)
    
    # We apply this additional decay to UNLOCKED voxels
    unlocked_mask = ~universe.is_locked
    
    # Create a factor array where unlocked = decay, locked = 1.0
    factor = np.ones_like(universe.density) # (N,N,N)
    factor[unlocked_mask] = decay_factor
    
    # Reshape to (N,N,N,1) for broadcasting against (N,N,N,3)
    factor = factor[..., np.newaxis]
    
    universe.flux *= factor


def update_manifestation(universe):
    """
    Phase 3: Existence Transitions
    Handles Genesis (0 -> +/-1) and Evaporation (+/-1 -> 0).
    """
    # 1. Evaporation: State -> 0 if Density < KB
    evaporation_mask = (universe.states != 0) & (universe.density < CONSTANTS.KB)
    universe.states[evaporation_mask] = 0
    
    # 2. Genesis: 0 -> State if Density > KB
    candidate_mask = (universe.states == 0) & (universe.density > CONSTANTS.KB)
    
    if np.any(candidate_mask):
        divergence = forces.calculate_divergence(universe)
        pos_mask = candidate_mask & (divergence > 0)
        neg_mask = candidate_mask & (divergence < 0)
        
        # Genesis Probability (Ch 4.1)
        rho = universe.density
        p_manifest = 1.0 - np.exp(-(rho - CONSTANTS.KB) / CONSTANTS.KB)
        p_manifest = np.clip(p_manifest, 0, 1)
        
        roll = np.random.random(universe.states.shape)
        success_mask = roll < p_manifest
        
        # DEBUG
        n_pos = np.count_nonzero(pos_mask & success_mask)
        n_neg = np.count_nonzero(neg_mask & success_mask)
        if n_pos > 0 or n_neg > 0:
            print(f"DEBUG: Genesis events - Pos: {n_pos}, Neg: {n_neg}")
        else:
             print(f"DEBUG: No Genesis. Candidates: {np.count_nonzero(candidate_mask)}")
        
        universe.states[pos_mask & success_mask] = 1
        universe.states[neg_mask & success_mask] = -1


def tick(universe):
    """
    Advance the universe by one discrete time step (dt=1).
    """
    
    # PHASE 2: Entropy
    apply_decay(universe)
    
    # PHASE 3: Existence Transitions
    update_manifestation(universe)
    
    # PHASE 4: Wave Propagation
    waves.propagate_flux(universe)
    
    # PHASE 5: Field Computation
    forces.calculate_density(universe)
    
    # PHASE 9: Collisions / Annihilation
    interactions.process_interactions(universe)
    
    # PHASE 11: Binding
    binding.update_bindings(universe)
