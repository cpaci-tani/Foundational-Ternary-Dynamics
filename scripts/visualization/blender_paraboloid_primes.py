"""
Paraboloid of Primes — Blender Python Script

Run inside Blender: Scripting workspace > Open > Run Script
Generates a 3D paraboloid of Gaussian primes where Z = a^2 + b^2 = p.

The discrete rings correspond to prime orbits on the paraboloid.
"""

import bpy
import math

def is_prime(n):
    if n < 2:
        return False
    if n < 4:
        return True
    if n % 2 == 0 or n % 3 == 0:
        return False
    i = 5
    while i * i <= n:
        if n % i == 0 or n % (i + 2) == 0:
            return False
        i += 6
    return True

# Clear existing mesh objects
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)

# Parameters
BOUND = 30
SPHERE_RADIUS = 0.15
SCALE_Z = 0.01  # Scale down Z to keep paraboloid manageable

# Create glowing emission material
mat = bpy.data.materials.new(name="PrimeGlow")
mat.use_nodes = True
nodes = mat.node_tree.nodes
links = mat.node_tree.links

# Clear default nodes
for node in nodes:
    nodes.remove(node)

# Add emission shader
emission = nodes.new(type='ShaderNodeEmission')
emission.inputs['Color'].default_value = (0.2, 0.6, 1.0, 1.0)  # Blue glow
emission.inputs['Strength'].default_value = 3.0

# Add output
output = nodes.new(type='ShaderNodeOutputMaterial')
links.new(emission.outputs['Emission'], output.inputs['Surface'])

# Create a different material for inert axis primes
mat_inert = bpy.data.materials.new(name="InertGlow")
mat_inert.use_nodes = True
nodes_i = mat_inert.node_tree.nodes
links_i = mat_inert.node_tree.links

for node in nodes_i:
    nodes_i.remove(node)

emission_i = nodes_i.new(type='ShaderNodeEmission')
emission_i.inputs['Color'].default_value = (1.0, 0.3, 0.2, 1.0)  # Red glow
emission_i.inputs['Strength'].default_value = 3.0

output_i = nodes_i.new(type='ShaderNodeOutputMaterial')
links_i.new(emission_i.outputs['Emission'], output_i.inputs['Surface'])

# Generate primes on the paraboloid
count = 0
for a in range(-BOUND, BOUND + 1):
    for b in range(-BOUND, BOUND + 1):
        norm2 = a * a + b * b
        if norm2 == 0:
            continue

        # Check if this is a Gaussian prime
        if a == 0 or b == 0:
            # Axis: check if |a| or |b| is prime ≡ 3 mod 4
            val = abs(a) if b == 0 else abs(b)
            if is_prime(val) and val % 4 == 3:
                bpy.ops.mesh.primitive_uv_sphere_add(
                    radius=SPHERE_RADIUS * 1.5,
                    location=(a, b, val * SCALE_Z),
                    segments=12, ring_count=8
                )
                obj = bpy.context.active_object
                obj.name = f"Inert_p{val}_({a},{b})"
                obj.data.materials.append(mat_inert)
                count += 1
        else:
            # Quadrant: check if a^2 + b^2 is prime
            if is_prime(norm2):
                bpy.ops.mesh.primitive_uv_sphere_add(
                    radius=SPHERE_RADIUS,
                    location=(a, b, norm2 * SCALE_Z),
                    segments=8, ring_count=6
                )
                obj = bpy.context.active_object
                obj.name = f"Split_p{norm2}_({a},{b})"
                obj.data.materials.append(mat)
                count += 1

print(f"Generated {count} Gaussian prime spheres on the paraboloid.")
print(f"Blue (split, quadrant): p ≡ 1 mod 4, Z = a^2 + b^2")
print(f"Red (inert, axis): p ≡ 3 mod 4, on coordinate axes")

# Set viewport shading to Material Preview for glow visibility
for area in bpy.context.screen.areas:
    if area.type == 'VIEW_3D':
        for space in area.spaces:
            if space.type == 'VIEW_3D':
                space.shading.type = 'MATERIAL'

# Add a camera looking at the paraboloid
bpy.ops.object.camera_add(location=(40, -40, 20))
cam = bpy.context.active_object
cam.rotation_euler = (math.radians(60), 0, math.radians(45))
bpy.context.scene.camera = cam

# Set dark background
bpy.context.scene.world.use_nodes = True
bg = bpy.context.scene.world.node_tree.nodes['Background']
bg.inputs['Color'].default_value = (0.01, 0.01, 0.02, 1.0)
bg.inputs['Strength'].default_value = 0.5
