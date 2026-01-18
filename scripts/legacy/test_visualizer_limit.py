import json
import random
import os

def generate_test_data(count=500000):
    voxels = []
    for _ in range(count):
        voxels.append({
            "x": random.randint(0, 200),
            "y": random.randint(0, 200),
            "z": random.randint(0, 200),
            "type": "matter",
            "val": 1
        })
    
    data = {"tick": 999, "voxels": voxels}
    
    path = "visualizer/public/data/frame_0199.json"
    if not os.path.exists("visualizer/public/data"):
        os.makedirs("visualizer/public/data")
        
    with open(path, "w") as f:
        json.dump(data, f)
    
    print(f"Generated {path} with {count} voxels. Size: {os.path.getsize(path)/1024/1024:.2f} MB")

if __name__ == "__main__":
    generate_test_data()
