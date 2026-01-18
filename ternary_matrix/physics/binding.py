"""
FTD Binding Logic
Phase 11: Structure Detection (Triads).
Refined: Uses 26-connected Moore Neighborhood.
"""
import numpy as np
import itertools

def update_bindings(universe):
    """
    Detect stable structures and set lock flags.
    Target: Triads (3 mutually adjacent same-sign particles).
    Rule: A particle is locked if it has >= 2 neighbors of the same sign
          within its 26-connected Moore neighborhood.
    """
    states = universe.states
    
    # Reset locks
    universe.is_locked.fill(False)
    
    pos_matter = (states == 1)
    neg_matter = (states == -1)
    
    def count_neighbors_moore(mask):
        count = np.zeros_like(states, dtype=np.int8)
        
        # Generate all 26 offsets
        offsets = [-1, 0, 1]
        for sx, sy, sz in itertools.product(offsets, repeat=3):
            if sx == 0 and sy == 0 and sz == 0:
                continue
                
            # Roll and add
            count += np.roll(mask, (sx, sy, sz), axis=(0, 1, 2)).astype(np.int8)
        return count

    n_pos = count_neighbors_moore(pos_matter)
    n_neg = count_neighbors_moore(neg_matter)
    
    # Rule: If I have >= 2 neighbors of my type, I am locked.
    pos_lock = pos_matter & (n_pos >= 2)
    neg_lock = neg_matter & (n_neg >= 2)
    
    universe.is_locked[pos_lock] = True
    universe.is_locked[neg_lock] = True
