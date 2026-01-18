"""
FTD Data Exporter
Saves simulation frames to JSON for the Web Visualizer.
"""
import json
import os
import numpy as np

class Exporter:
    def __init__(self, output_dir):
        self.output_dir = output_dir
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
            
    def save_frame(self, universe, tick):
        """
        Extract active voxels and save to json.
        """
        # Get coordinates of non-empty voxels
        # State != 0 OR Density > threshold (to show flux clouds?)
        # For V1: Show States + High Density Flux (>1.0)
        
        frame_data = {
            "tick": tick,
            "voxels": []
        }
        
        # 1. States (Matter)
        matter_mask = universe.states != 0
        indices = np.argwhere(matter_mask)
        
        for idx in indices:
            x, y, z = idx
            state = int(universe.states[x,y,z])
            rho = float(universe.density[x,y,z])
            locked = bool(universe.is_locked[x,y,z])
            
            frame_data["voxels"].append({
                "x": int(x), "y": int(y), "z": int(z),
                "type": "matter",
                "val": state,
                "rho": round(rho, 2),
                "locked": locked
            })
            
        # 2. Flux (Energy Clouds) - Optional aesthetic
        # Filter for high density but empty state
        flux_mask = (universe.states == 0) & (universe.density > 1.0)
        indices_f = np.argwhere(flux_mask)
        
        for idx in indices_f:
            x, y, z = idx
            rho = float(universe.density[x,y,z])
            frame_data["voxels"].append({
                "x": int(x), "y": int(y), "z": int(z),
                "type": "flux",
                "val": 0,
                "rho": round(rho, 2)
            })
            
        filename = os.path.join(self.output_dir, f"frame_{tick:04d}.json")
        with open(filename, 'w') as f:
            json.dump(frame_data, f)
            
        return len(frame_data["voxels"])
