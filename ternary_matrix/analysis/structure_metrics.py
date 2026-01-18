"""
FTD Structure Metrics
Analyzes cluster size distributions (Fibonacci Search).
"""
import numpy as np
from scipy.ndimage import label

def analyze_clusters(universe):
    """
    Returns a dictionary mapping cluster_size -> count.
    Uses 6-connectivity Structural Element.
    """
    # Combine +1 and -1 into a binary mask of "Matter"
    matter_mask = (universe.states != 0).astype(int)
    
    # Structure: 3x3x3 cross (6-neighbors)
    s = np.array([[[0,0,0],[0,1,0],[0,0,0]],
                  [[0,1,0],[1,1,1],[0,1,0]],
                  [[0,0,0],[0,1,0],[0,0,0]]])
    
    # Label connected components
    labeled_array, num_features = label(matter_mask, structure=s)
    
    if num_features == 0:
        return {}
    
    # Count volumes of each label
    # bincount calculates number of pixels per label
    # index 0 is background (0), so skip it
    counts = np.bincount(labeled_array.ravel())
    
    if len(counts) <= 1:
        return {}
        
    cluster_sizes = counts[1:] # Drop background
    
    # Convert to Size Frequency Histogram
    # e.g. We have 5 clusters of size 3, 2 clusters of size 4...
    size_freq = {}
    unique, frequency = np.unique(cluster_sizes, return_counts=True)
    
    for u, f in zip(unique, frequency):
        size_freq[int(u)] = int(f)
        
    return size_freq
