"""
FTD Interactions
Phase 9: Collisions and Annihilation.
"""
import numpy as np

def process_interactions(universe):
    """
    Handle particle-particle interactions.
    Key Rule: Annihilation (+1 and -1 adjacent -> 0)
    """
    states = universe.states
    
    # Create masks for +1 and -1
    pos_matter = (states == 1)
    neg_matter = (states == -1)
    
    # We need to detect if a +1 has any -1 neighbor, and vice versa.
    # We can use convolution or simple shifting.
    # Let's use shifting for the 6-neighborhood for simplicity (matching Laplacian topology).
    # Ideally FTD uses 26-Moore, but 6-VonNeumann is easier to implement quickly and usually sufficient for adjacency.
    
    has_neg_neighbor = np.zeros_like(states, dtype=bool)
    has_pos_neighbor = np.zeros_like(states, dtype=bool)
    
    shifts = [
        (0, 0, 1), (0, 0, -1),
        (0, 1, 0), (0, -1, 0),
        (1, 0, 0), (-1, 0, 0)
    ]
    
    for sx, sy, sz in shifts:
        # Check if neighbor is -1
        shifted_neg = np.roll(neg_matter, (sx, sy, sz), axis=(0, 1, 2))
        has_neg_neighbor |= shifted_neg
        
        # Check if neighbor is +1
        shifted_pos = np.roll(pos_matter, (sx, sy, sz), axis=(0, 1, 2))
        has_pos_neighbor |= shifted_pos
        
    # Annihilation conditions
    # If I am +1 and I have a -1 neighbor -> Die
    kill_pos = pos_matter & has_neg_neighbor
    
    # If I am -1 and I have a +1 neighbor -> Die
    kill_neg = neg_matter & has_pos_neighbor
    
    # Apply death
    # Note: In full FTD, this releases energy (Flux Burst). 
    # For now, just state transition 0.
    universe.states[kill_pos] = 0
    universe.states[kill_neg] = 0
