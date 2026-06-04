import json
import os

# The 24 standard orientation mappings for a cube's faces
ROTATIONS = [
    [0,1,2,3,4,5], [0,1,3,2,5,4], [0,1,5,4,2,3], [0,1,4,5,3,2],
    [1,0,2,3,5,4], [1,0,3,2,4,5], [1,0,4,5,2,3], [1,0,5,4,3,2],
    [2,3,0,1,5,4], [3,2,0,1,4,5], [4,5,0,1,2,3], [5,4,0,1,3,2],
    [2,3,1,0,4,5], [3,2,1,0,5,4], [5,4,1,0,2,3], [4,5,1,0,3,2],
    [2,3,4,5,0,1], [3,2,5,4,0,1], [5,4,2,3,0,1], [4,5,3,2,0,1],
    [2,3,5,4,1,0], [3,2,4,5,1,0], [4,5,2,3,1,0], [5,4,3,2,1,0]
]

# The Topology #02 Base Wang Signature
topology_02 = [3, 7, 2, 8, 6, 12]

# Generate the 24 unique tiles from the base topology
tiles = []
for r in range(24):
    t = []
    for f in range(6):
        t.append(topology_02[ROTATIONS[r][f]])
    tiles.append(t)

T = [-1] * 27

def solve(depth):
    if depth == 27:
        return True
    
    x = depth % 3
    y = (depth // 3) % 3
    z = (depth // 9) % 3
    
    for t_idx in range(24):
        valid = True
        
        # Check boundary rules: Faces are 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z
        # To match, adjacent faces must have identical colors
        if x > 0:
            if tiles[t_idx][1] != tiles[T[depth - 1]][0]:
                valid = False
        if valid and y > 0:
            if tiles[t_idx][3] != tiles[T[depth - 3]][2]:
                valid = False
        if valid and z > 0:
            if tiles[t_idx][5] != tiles[T[depth - 9]][4]:
                valid = False
                
        if valid:
            T[depth] = t_idx
            if solve(depth + 1):
                return True
    return False

print("Computing 3x3x3 Macro-Lattice via Backtracking...")
if solve(0):
    print("SUCCESS: Found perfect 27-block mathematical solution.")
    out = []
    for depth in range(27):
        x = depth % 3
        y = (depth // 3) % 3
        z = (depth // 9) % 3
        out.append({
            "id": f"{x}_{y}_{z}",
            "x": x - 1, # Center around 0
            "y": y - 1,
            "z": z - 1,
            "rot_idx": T[depth],
            "faces": tiles[T[depth]]
        })
        
    output_path = os.path.join(os.path.dirname(__file__), "solution_3x3x3.json")
    with open(output_path, "w") as f:
        json.dump(out, f, indent=2)
    print(f"Exported lattice configuration to: {output_path}")
else:
    print("FAILED: No valid 3x3x3 tiling exists for this topology.")
