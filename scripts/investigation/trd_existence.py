"""
═══════════════════════════════════════════════════════════════════════════════
    TRD EXISTENCE: A Complete Visualization of Ternary Realization Dynamics
═══════════════════════════════════════════════════════════════════════════════

    "The void is not empty space—it is a null substrate awaiting activation."

    This script generates a complete visual representation of the TRD framework:

    LAYER 1: THE SUBSTRATE
        - Heptagonal antiprism cells (b₃ = 7 Gauss constraints)
        - 14 vertices per cell encoding duality (+1/-1)
        - Central void points (state 0)
        - Twisted edges (reflexive temporal coupling)

    LAYER 2: THE FLUX FIELD
        - Vector arrows showing J field flow
        - Density visualization (|J| magnitude)
        - Divergence indicators (∇·J sources/sinks)

    LAYER 3: MANIFESTATION DYNAMICS
        - Genesis events (0 → ±1) as particle births
        - Annihilation events (+1 + -1 → 0) as energy bursts
        - Stable triads (proto-nucleons)

    LAYER 4: EMERGENT STRUCTURE
        - Shell formations (electron orbitals)
        - Bound states (atoms)
        - Hierarchical organization

    LAYER 5: THE sLOOP
        - Observer-system coupling visualization
        - Self-referential causal structure

    Framework Constants Encoded:
        b₃ = 7      (Gauss constraints, heptagonal symmetry)
        N_c = 3     (color charges, triad structure)
        n_eff = 13  (F₇, effective dimension)
        N_base = 4  (base structure, tetrahedral cores)
        φ = 1.618   (golden ratio, growth/binding)
        α = 1/137   (fine structure, coupling strength)

    Author: TRD Visualization System
    Date: January 2026
═══════════════════════════════════════════════════════════════════════════════
"""

import bpy
import bmesh
import math
import random
from mathutils import Vector, Matrix, Euler
from collections import defaultdict

# =============================================================================
# ██████╗  █████╗ ██████╗  █████╗ ███╗   ███╗███████╗████████╗███████╗██████╗ ███████╗
# ██╔══██╗██╔══██╗██╔══██╗██╔══██╗████╗ ████║██╔════╝╚══██╔══╝██╔════╝██╔══██╗██╔════╝
# ██████╔╝███████║██████╔╝███████║██╔████╔██║█████╗     ██║   █████╗  ██████╔╝███████╗
# ██╔═══╝ ██╔══██║██╔══██╗██╔══██║██║╚██╔╝██║██╔══╝     ██║   ██╔══╝  ██╔══██╗╚════██║
# ██║     ██║  ██║██║  ██║██║  ██║██║ ╚═╝ ██║███████╗   ██║   ███████╗██║  ██║███████║
# ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚══════╝
# =============================================================================

class TRDConstants:
    """Framework integers and physical constants."""

    # The Four Integers (uniquely determined by Fibonacci constraints)
    B3 = 7          # Gauss constraints on 2³ lattice
    NC = 3          # Color charges (SU(3))
    NEFF = 13       # Effective dimension (F₇)
    NBASE = 4       # Base structure

    # Derived quantities
    DOF = 16        # Physical degrees of freedom: 24 - 7 - 1
    PHI = (1 + math.sqrt(5)) / 2  # Golden ratio
    ALPHA = 1 / 137.036  # Fine structure constant

    # The lemniscatic constant G*
    # G* = √2 × Γ(1/4)² / (2π) ≈ 2.9587
    G_STAR = 2.9586751192

    # Fibonacci sequence for mode coupling
    FIB = [1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144]


class VisualConfig:
    """Visual and rendering parameters."""

    # === LATTICE STRUCTURE ===
    GRID_SIZE = (3, 3, 2)           # Cells in each dimension
    CELL_RADIUS = 1.0               # Heptagon radius
    CELL_HEIGHT = 1.8               # Antiprism height
    CELL_SPACING = 2.8              # Inter-cell distance

    # === VERTEX APPEARANCE ===
    VERTEX_RADIUS_POSITIVE = 0.12   # +1 state vertices
    VERTEX_RADIUS_NEGATIVE = 0.10   # -1 state vertices (slightly smaller)
    VERTEX_RADIUS_VOID = 0.06       # Central void points

    # === EDGE APPEARANCE ===
    EDGE_RADIUS = 0.025             # Connection thickness
    FLUX_ARROW_SCALE = 0.4          # Flux vector arrow size

    # === MANIFESTATION ===
    TRIAD_SCALE = 0.3               # Size of triad formations
    SHELL_RADIUS_MULTIPLIER = 1.5   # Electron shell distance

    # === ANIMATION ===
    ANIMATION_FRAMES = 250          # Total frames
    PULSE_SPEED = 0.1               # Manifestation pulse rate
    WAVE_SPEED = 0.05               # Flux wave propagation

    # === RENDER SETTINGS ===
    RESOLUTION = (1920, 1080)
    SAMPLES = 256
    USE_BLOOM = True
    BLOOM_INTENSITY = 0.1


class ColorPalette:
    """
    Color scheme encoding TRD ontology.

    The palette is not arbitrary—colors encode meaning:
    - Warm tones: positive manifestation, creation, matter
    - Cool tones: negative manifestation, antimatter
    - Purple: the void substrate, potential
    - Cyan: flux field, information flow
    - Gold: stable structures, phi-ratio
    """

    # Primary states
    POSITIVE = (1.0, 0.85, 0.75, 1.0)      # Warm cream-pink
    NEGATIVE = (0.75, 0.85, 1.0, 1.0)      # Cool ice-blue
    VOID = (0.15, 0.0, 0.25, 1.0)          # Deep purple

    # Flux field
    FLUX_LOW = (0.2, 0.4, 0.6, 1.0)        # Low density: dark blue
    FLUX_MED = (0.4, 0.8, 0.9, 1.0)        # Medium: cyan
    FLUX_HIGH = (0.9, 0.95, 1.0, 1.0)      # High density: white

    # Edges and structure
    EDGE_STANDARD = (0.6, 0.9, 0.95, 1.0)  # Cyan connections
    EDGE_ACTIVE = (1.0, 1.0, 0.8, 1.0)     # Active flux: golden

    # Manifestation events
    GENESIS = (1.0, 0.9, 0.5, 1.0)         # Birth: golden white
    ANNIHILATION = (1.0, 0.3, 0.8, 1.0)    # Annihilation: magenta flash

    # Stable structures
    TRIAD = (0.9, 0.7, 0.3, 1.0)           # Triads: amber/gold
    SHELL = (0.5, 0.7, 1.0, 0.3)           # Shells: transparent blue

    # Background
    BACKGROUND = (0.0, 0.0, 0.02, 1.0)     # Near-black with hint of blue


# =============================================================================
# ███╗   ███╗ █████╗ ████████╗███████╗██████╗ ██╗ █████╗ ██╗     ███████╗
# ████╗ ████║██╔══██╗╚══██╔══╝██╔════╝██╔══██╗██║██╔══██╗██║     ██╔════╝
# ██╔████╔██║███████║   ██║   █████╗  ██████╔╝██║███████║██║     ███████╗
# ██║╚██╔╝██║██╔══██║   ██║   ██╔══╝  ██╔══██╗██║██╔══██║██║     ╚════██║
# ██║ ╚═╝ ██║██║  ██║   ██║   ███████╗██║  ██║██║██║  ██║███████╗███████║
# ╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚══════╝╚══════╝
# =============================================================================

class MaterialFactory:
    """Creates physically-meaningful materials for TRD visualization."""

    _cache = {}

    @classmethod
    def clear_cache(cls):
        cls._cache = {}

    @classmethod
    def get_or_create(cls, name, color, emission=0.0, alpha=1.0, metallic=0.0):
        """Get cached material or create new one."""
        key = (name, color, emission, alpha, metallic)
        if key not in cls._cache:
            cls._cache[key] = cls._create_material(name, color, emission, alpha, metallic)
        return cls._cache[key]

    @staticmethod
    def _create_material(name, color, emission, alpha, metallic):
        """Create a new material with the given properties."""
        mat = bpy.data.materials.new(name=name)
        mat.use_nodes = True

        nodes = mat.node_tree.nodes
        links = mat.node_tree.links
        nodes.clear()

        # Create shader nodes
        output = nodes.new('ShaderNodeOutputMaterial')
        principled = nodes.new('ShaderNodeBsdfPrincipled')

        # Configure
        principled.inputs['Base Color'].default_value = color
        principled.inputs['Metallic'].default_value = metallic
        principled.inputs['Roughness'].default_value = 0.3

        if emission > 0:
            principled.inputs['Emission Color'].default_value = color
            principled.inputs['Emission Strength'].default_value = emission

        if alpha < 1.0:
            mat.blend_method = 'BLEND'
            principled.inputs['Alpha'].default_value = alpha

        links.new(principled.outputs['BSDF'], output.inputs['Surface'])

        output.location = (300, 0)
        principled.location = (0, 0)

        return mat

    @classmethod
    def create_gradient_material(cls, name, color1, color2, emission=1.0):
        """Create a material that transitions between two colors based on position."""
        mat = bpy.data.materials.new(name=name)
        mat.use_nodes = True

        nodes = mat.node_tree.nodes
        links = mat.node_tree.links
        nodes.clear()

        output = nodes.new('ShaderNodeOutputMaterial')
        principled = nodes.new('ShaderNodeBsdfPrincipled')
        mix = nodes.new('ShaderNodeMixRGB')
        geometry = nodes.new('ShaderNodeNewGeometry')
        separate = nodes.new('ShaderNodeSeparateXYZ')
        math_node = nodes.new('ShaderNodeMath')

        # Use Z position to blend colors
        math_node.operation = 'MULTIPLY'
        math_node.inputs[1].default_value = 0.5

        mix.inputs[1].default_value = color1
        mix.inputs[2].default_value = color2

        links.new(geometry.outputs['Position'], separate.inputs['Vector'])
        links.new(separate.outputs['Z'], math_node.inputs[0])
        links.new(math_node.outputs['Value'], mix.inputs['Fac'])
        links.new(mix.outputs['Color'], principled.inputs['Base Color'])
        links.new(mix.outputs['Color'], principled.inputs['Emission Color'])
        principled.inputs['Emission Strength'].default_value = emission
        links.new(principled.outputs['BSDF'], output.inputs['Surface'])

        return mat


# =============================================================================
#  ██████╗ ███████╗ ██████╗ ███╗   ███╗███████╗████████╗██████╗ ██╗   ██╗
# ██╔════╝ ██╔════╝██╔═══██╗████╗ ████║██╔════╝╚══██╔══╝██╔══██╗╚██╗ ██╔╝
# ██║  ███╗█████╗  ██║   ██║██╔████╔██║█████╗     ██║   ██████╔╝ ╚████╔╝
# ██║   ██║██╔══╝  ██║   ██║██║╚██╔╝██║██╔══╝     ██║   ██╔══██╗  ╚██╔╝
# ╚██████╔╝███████╗╚██████╔╝██║ ╚═╝ ██║███████╗   ██║   ██║  ██║   ██║
#  ╚═════╝ ╚══════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝   ╚═╝
# =============================================================================

class GeometryBuilder:
    """Low-level geometry creation utilities."""

    @staticmethod
    def create_sphere(location, radius, material, name="Sphere"):
        """Create a UV sphere."""
        mesh = bpy.data.meshes.new(f"{name}_mesh")
        obj = bpy.data.objects.new(name, mesh)

        bm = bmesh.new()
        bmesh.ops.create_uvsphere(bm, u_segments=16, v_segments=12, radius=radius)
        bm.to_mesh(mesh)
        bm.free()

        obj.location = location
        obj.data.materials.append(material)

        # Smooth shading
        for poly in mesh.polygons:
            poly.use_smooth = True

        return obj

    @staticmethod
    def create_cylinder(p1, p2, radius, material, name="Cylinder"):
        """Create a cylinder between two points."""
        p1, p2 = Vector(p1), Vector(p2)
        mid = (p1 + p2) / 2
        direction = p2 - p1
        length = direction.length

        if length < 0.001:
            return None

        mesh = bpy.data.meshes.new(f"{name}_mesh")
        obj = bpy.data.objects.new(name, mesh)

        bm = bmesh.new()
        bmesh.ops.create_cone(bm, cap_ends=True, cap_tris=False,
                              segments=12, radius1=radius, radius2=radius, depth=length)
        bm.to_mesh(mesh)
        bm.free()

        obj.location = mid
        obj.data.materials.append(material)

        # Rotate to align with direction
        up = Vector((0, 0, 1))
        direction_norm = direction.normalized()

        if abs(direction_norm.dot(up)) < 0.9999:
            rot_axis = up.cross(direction_norm).normalized()
            rot_angle = math.acos(max(-1, min(1, up.dot(direction_norm))))
            obj.rotation_mode = 'AXIS_ANGLE'
            obj.rotation_axis_angle = (rot_angle, rot_axis.x, rot_axis.y, rot_axis.z)
        elif direction_norm.dot(up) < 0:
            obj.rotation_euler = (math.pi, 0, 0)

        for poly in mesh.polygons:
            poly.use_smooth = True

        return obj

    @staticmethod
    def create_arrow(origin, direction, scale, material, name="Arrow"):
        """Create an arrow representing a flux vector."""
        origin = Vector(origin)
        direction = Vector(direction).normalized()

        # Arrow shaft
        shaft_end = origin + direction * scale * 0.7
        shaft = GeometryBuilder.create_cylinder(
            origin, shaft_end, scale * 0.05, material, f"{name}_shaft"
        )

        # Arrow head (cone)
        mesh = bpy.data.meshes.new(f"{name}_head_mesh")
        head = bpy.data.objects.new(f"{name}_head", mesh)

        bm = bmesh.new()
        bmesh.ops.create_cone(bm, cap_ends=True, cap_tris=True,
                              segments=12, radius1=scale * 0.12, radius2=0, depth=scale * 0.3)
        bm.to_mesh(mesh)
        bm.free()

        head.location = origin + direction * scale * 0.85
        head.data.materials.append(material)

        # Rotate head
        up = Vector((0, 0, 1))
        if abs(direction.dot(up)) < 0.9999:
            rot_axis = up.cross(direction).normalized()
            rot_angle = math.acos(max(-1, min(1, up.dot(direction))))
            head.rotation_mode = 'AXIS_ANGLE'
            head.rotation_axis_angle = (rot_angle, rot_axis.x, rot_axis.y, rot_axis.z)
        elif direction.dot(up) < 0:
            head.rotation_euler = (math.pi, 0, 0)

        return [shaft, head] if shaft else [head]

    @staticmethod
    def create_torus(location, major_radius, minor_radius, material, name="Torus"):
        """Create a torus (for shells/orbitals)."""
        mesh = bpy.data.meshes.new(f"{name}_mesh")
        obj = bpy.data.objects.new(name, mesh)

        bm = bmesh.new()
        bmesh.ops.create_uvsphere(bm, u_segments=32, v_segments=16, radius=major_radius)
        bm.to_mesh(mesh)
        bm.free()

        # Actually let's use the torus primitive
        bpy.ops.mesh.primitive_torus_add(
            major_radius=major_radius,
            minor_radius=minor_radius,
            major_segments=48,
            minor_segments=12,
            location=location
        )
        obj = bpy.context.active_object
        obj.name = name
        obj.data.materials.append(material)

        for poly in obj.data.polygons:
            poly.use_smooth = True

        return obj


# =============================================================================
#  ██████╗███████╗██╗     ██╗         ███████╗████████╗██████╗ ██╗   ██╗ ██████╗████████╗
# ██╔════╝██╔════╝██║     ██║         ██╔════╝╚══██╔══╝██╔══██╗██║   ██║██╔════╝╚══██╔══╝
# ██║     █████╗  ██║     ██║         ███████╗   ██║   ██████╔╝██║   ██║██║        ██║
# ██║     ██╔══╝  ██║     ██║         ╚════██║   ██║   ██╔══██╗██║   ██║██║        ██║
# ╚██████╗███████╗███████╗███████╗    ███████║   ██║   ██║  ██║╚██████╔╝╚██████╗   ██║
#  ╚═════╝╚══════╝╚══════╝╚══════╝    ╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝   ╚═╝
# =============================================================================

class HeptagonalAntiprismCell:
    """
    The fundamental unit of TRD spacetime: a heptagonal antiprism.

    Structure:
        - 7 vertices on top polygon (+1 states)
        - 7 vertices on bottom polygon (-1 states)
        - 1 central void point (0 state)
        - 14 lateral edges (twisted, encoding reflexivity)
        - 14 polygon edges (7 top + 7 bottom)

    The 7-fold symmetry encodes b₃ = 7 Gauss constraints.
    The duality encodes the ternary state structure.
    The twist encodes the reflexive Lagrangian coupling.
    """

    def __init__(self, center, cell_id=0):
        self.center = Vector(center)
        self.cell_id = cell_id
        self.objects = []

        # Vertex data
        self.top_vertices = []      # +1 states
        self.bottom_vertices = []   # -1 states
        self.void_center = None     # 0 state

        # Flux data (for animation)
        self.flux_vectors = []
        self.flux_magnitude = random.uniform(0.3, 1.0)

        # State
        self.is_manifested = random.random() > 0.3
        self.has_triad = random.random() > 0.7

    def generate_vertices(self):
        """Calculate vertex positions for the antiprism."""
        n = TRDConstants.B3  # 7-fold symmetry
        radius = VisualConfig.CELL_RADIUS
        height = VisualConfig.CELL_HEIGHT
        twist = math.pi / n  # Antiprism twist angle

        cx, cy, cz = self.center

        self.top_vertices = []
        self.bottom_vertices = []

        for i in range(n):
            # Top heptagon
            angle_top = 2 * math.pi * i / n
            top_pos = Vector((
                cx + radius * math.cos(angle_top),
                cy + radius * math.sin(angle_top),
                cz + height / 2
            ))
            self.top_vertices.append(top_pos)

            # Bottom heptagon (twisted)
            angle_bot = 2 * math.pi * i / n + twist
            bot_pos = Vector((
                cx + radius * math.cos(angle_bot),
                cy + radius * math.sin(angle_bot),
                cz - height / 2
            ))
            self.bottom_vertices.append(bot_pos)

        self.void_center = self.center.copy()

    def build(self, collection):
        """Construct all geometry for this cell."""
        self.generate_vertices()

        # Materials
        mat_pos = MaterialFactory.get_or_create(
            "Positive", ColorPalette.POSITIVE, emission=2.5
        )
        mat_neg = MaterialFactory.get_or_create(
            "Negative", ColorPalette.NEGATIVE, emission=2.0
        )
        mat_void = MaterialFactory.get_or_create(
            "Void", ColorPalette.VOID, emission=0.8
        )
        mat_edge = MaterialFactory.get_or_create(
            "Edge", ColorPalette.EDGE_STANDARD, emission=1.0
        )

        n = TRDConstants.B3

        # === CREATE VERTICES ===

        # Top vertices (+1 states)
        for i, pos in enumerate(self.top_vertices):
            sphere = GeometryBuilder.create_sphere(
                pos, VisualConfig.VERTEX_RADIUS_POSITIVE,
                mat_pos, f"Cell{self.cell_id}_Top{i}"
            )
            collection.objects.link(sphere)
            self.objects.append(sphere)

        # Bottom vertices (-1 states)
        for i, pos in enumerate(self.bottom_vertices):
            sphere = GeometryBuilder.create_sphere(
                pos, VisualConfig.VERTEX_RADIUS_NEGATIVE,
                mat_neg, f"Cell{self.cell_id}_Bot{i}"
            )
            collection.objects.link(sphere)
            self.objects.append(sphere)

        # Void center (0 state)
        void_sphere = GeometryBuilder.create_sphere(
            self.void_center, VisualConfig.VERTEX_RADIUS_VOID,
            mat_void, f"Cell{self.cell_id}_Void"
        )
        collection.objects.link(void_sphere)
        self.objects.append(void_sphere)

        # === CREATE EDGES ===

        # Top polygon edges
        for i in range(n):
            p1 = self.top_vertices[i]
            p2 = self.top_vertices[(i + 1) % n]
            edge = GeometryBuilder.create_cylinder(
                p1, p2, VisualConfig.EDGE_RADIUS,
                mat_edge, f"Cell{self.cell_id}_EdgeTop{i}"
            )
            if edge:
                collection.objects.link(edge)
                self.objects.append(edge)

        # Bottom polygon edges
        for i in range(n):
            p1 = self.bottom_vertices[i]
            p2 = self.bottom_vertices[(i + 1) % n]
            edge = GeometryBuilder.create_cylinder(
                p1, p2, VisualConfig.EDGE_RADIUS,
                mat_edge, f"Cell{self.cell_id}_EdgeBot{i}"
            )
            if edge:
                collection.objects.link(edge)
                self.objects.append(edge)

        # Lateral edges (the antiprism twist)
        for i in range(n):
            # Top[i] to Bot[i]
            edge1 = GeometryBuilder.create_cylinder(
                self.top_vertices[i], self.bottom_vertices[i],
                VisualConfig.EDGE_RADIUS, mat_edge,
                f"Cell{self.cell_id}_Lateral{i}a"
            )
            if edge1:
                collection.objects.link(edge1)
                self.objects.append(edge1)

            # Top[i] to Bot[i-1]
            edge2 = GeometryBuilder.create_cylinder(
                self.top_vertices[i], self.bottom_vertices[(i - 1) % n],
                VisualConfig.EDGE_RADIUS, mat_edge,
                f"Cell{self.cell_id}_Lateral{i}b"
            )
            if edge2:
                collection.objects.link(edge2)
                self.objects.append(edge2)

        return self.objects


# =============================================================================
# ████████╗██████╗ ██╗ █████╗ ██████╗ ███████╗
# ╚══██╔══╝██╔══██╗██║██╔══██╗██╔══██╗██╔════╝
#    ██║   ██████╔╝██║███████║██║  ██║███████╗
#    ██║   ██╔══██╗██║██╔══██║██║  ██║╚════██║
#    ██║   ██║  ██║██║██║  ██║██████╔╝███████║
#    ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝
# =============================================================================

class Triad:
    """
    A stable three-particle bound state (proto-nucleon).

    Structure:
        - 3 same-sign vertices arranged in equilateral triangle
        - Pairwise distance ≈ √2 lattice units
        - Binding energy ≈ KB × φ

    N_c = 3 is encoded in the triangular structure.
    The golden ratio φ appears in the binding dynamics.
    """

    def __init__(self, center, scale=1.0, polarity=1):
        self.center = Vector(center)
        self.scale = scale * VisualConfig.TRIAD_SCALE
        self.polarity = polarity  # +1 or -1
        self.objects = []

    def build(self, collection):
        """Create the triad geometry."""
        # Equilateral triangle vertices
        n = TRDConstants.NC  # 3 vertices
        radius = self.scale * 0.5

        vertices = []
        for i in range(n):
            angle = 2 * math.pi * i / n - math.pi / 2  # Start from top
            pos = self.center + Vector((
                radius * math.cos(angle),
                radius * math.sin(angle),
                0
            ))
            vertices.append(pos)

        # Material based on polarity
        if self.polarity > 0:
            mat = MaterialFactory.get_or_create(
                "TriadPositive", ColorPalette.TRIAD, emission=3.0
            )
        else:
            mat = MaterialFactory.get_or_create(
                "TriadNegative",
                (ColorPalette.TRIAD[2], ColorPalette.TRIAD[1], ColorPalette.TRIAD[0], 1.0),
                emission=3.0
            )

        mat_bond = MaterialFactory.get_or_create(
            "TriadBond", ColorPalette.EDGE_ACTIVE, emission=2.0
        )

        # Create vertices
        for i, pos in enumerate(vertices):
            sphere = GeometryBuilder.create_sphere(
                pos, self.scale * 0.15, mat, f"Triad_V{i}"
            )
            collection.objects.link(sphere)
            self.objects.append(sphere)

        # Create bonds
        for i in range(n):
            p1 = vertices[i]
            p2 = vertices[(i + 1) % n]
            bond = GeometryBuilder.create_cylinder(
                p1, p2, self.scale * 0.04, mat_bond, f"Triad_Bond{i}"
            )
            if bond:
                collection.objects.link(bond)
                self.objects.append(bond)

        # Central gluon field indicator
        gluon = GeometryBuilder.create_sphere(
            self.center, self.scale * 0.08,
            MaterialFactory.get_or_create("Gluon", ColorPalette.VOID, emission=1.5),
            "Triad_Gluon"
        )
        collection.objects.link(gluon)
        self.objects.append(gluon)

        return self.objects


# =============================================================================
# ███████╗██╗  ██╗███████╗██╗     ██╗     ███████╗
# ██╔════╝██║  ██║██╔════╝██║     ██║     ██╔════╝
# ███████╗███████║█████╗  ██║     ██║     ███████╗
# ╚════██║██╔══██║██╔══╝  ██║     ██║     ╚════██║
# ███████║██║  ██║███████╗███████╗███████╗███████║
# ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
# =============================================================================

class ElectronShell:
    """
    A quasi-stable orbital shell around a nucleus.

    Shell radii follow n² scaling (hydrogen-like).
    Encodes the emergence of atomic structure from TRD.
    """

    def __init__(self, center, n_level=1, num_electrons=1):
        self.center = Vector(center)
        self.n_level = n_level
        self.num_electrons = min(num_electrons, 2 * n_level * n_level)
        self.objects = []

    def build(self, collection):
        """Create shell visualization."""
        # Shell radius scales as n²
        radius = self.n_level ** 2 * VisualConfig.SHELL_RADIUS_MULTIPLIER * 0.3

        # Create orbital torus
        mat_shell = MaterialFactory.get_or_create(
            f"Shell_n{self.n_level}", ColorPalette.SHELL,
            emission=0.5, alpha=0.3
        )

        # Create a thin torus representing the orbital
        bpy.ops.mesh.primitive_torus_add(
            major_radius=radius,
            minor_radius=0.02,
            major_segments=64,
            minor_segments=8,
            location=self.center
        )
        torus = bpy.context.active_object
        torus.name = f"Shell_n{self.n_level}"
        torus.data.materials.append(mat_shell)

        # Add another torus perpendicular for 3D effect
        bpy.ops.mesh.primitive_torus_add(
            major_radius=radius,
            minor_radius=0.02,
            major_segments=64,
            minor_segments=8,
            location=self.center,
            rotation=(math.pi/2, 0, 0)
        )
        torus2 = bpy.context.active_object
        torus2.name = f"Shell_n{self.n_level}_perp"
        torus2.data.materials.append(mat_shell)

        collection.objects.link(torus)
        collection.objects.link(torus2)
        bpy.context.collection.objects.unlink(torus)
        bpy.context.collection.objects.unlink(torus2)

        self.objects.extend([torus, torus2])

        # Place electrons on the shell
        mat_electron = MaterialFactory.get_or_create(
            "Electron", ColorPalette.NEGATIVE, emission=3.0
        )

        for i in range(self.num_electrons):
            angle = 2 * math.pi * i / self.num_electrons
            pos = self.center + Vector((
                radius * math.cos(angle),
                radius * math.sin(angle),
                0
            ))
            electron = GeometryBuilder.create_sphere(
                pos, 0.05, mat_electron, f"Electron_n{self.n_level}_{i}"
            )
            collection.objects.link(electron)
            self.objects.append(electron)

        return self.objects


# =============================================================================
# ███████╗██╗     ██╗   ██╗██╗  ██╗    ███████╗██╗███████╗██╗     ██████╗
# ██╔════╝██║     ██║   ██║╚██╗██╔╝    ██╔════╝██║██╔════╝██║     ██╔══██╗
# █████╗  ██║     ██║   ██║ ╚███╔╝     █████╗  ██║█████╗  ██║     ██║  ██║
# ██╔══╝  ██║     ██║   ██║ ██╔██╗     ██╔══╝  ██║██╔══╝  ██║     ██║  ██║
# ██║     ███████╗╚██████╔╝██╔╝ ██╗    ██║     ██║███████╗███████╗██████╔╝
# ╚═╝     ╚══════╝ ╚═════╝ ╚═╝  ╚═╝    ╚═╝     ╚═╝╚══════╝╚══════╝╚═════╝
# =============================================================================

class FluxField:
    """
    Visualization of the continuous flux field J(v) ∈ ℝ³.

    The flux field:
        - Encodes potential energy density
        - Determines manifestation probability
        - Propagates as waves (speed C = 1)
        - Is the precursor to the quantum wave function (ψ = Jx + iJy)
    """

    def __init__(self, lattice_cells):
        self.cells = lattice_cells
        self.arrows = []

    def build(self, collection, density=0.3):
        """Create flux vector visualization."""
        mat_flux = MaterialFactory.get_or_create(
            "FluxVector", ColorPalette.FLUX_MED, emission=1.5
        )

        for cell in self.cells:
            # Only show flux in some cells (for clarity)
            if random.random() > density:
                continue

            # Generate a flux vector (simplified: random direction with curl)
            center = cell.center

            # Create swirling pattern suggesting curl
            theta = math.atan2(center.y, center.x)
            flux_dir = Vector((
                -math.sin(theta) + random.uniform(-0.3, 0.3),
                math.cos(theta) + random.uniform(-0.3, 0.3),
                random.uniform(-0.2, 0.2)
            )).normalized()

            magnitude = cell.flux_magnitude

            arrows = GeometryBuilder.create_arrow(
                center, flux_dir,
                VisualConfig.FLUX_ARROW_SCALE * magnitude,
                mat_flux, f"Flux_{cell.cell_id}"
            )

            for arrow in arrows:
                if arrow:
                    collection.objects.link(arrow)
                    self.arrows.append(arrow)

        return self.arrows


# =============================================================================
# ███████╗██╗      ██████╗  ██████╗ ██████╗
# ██╔════╝██║     ██╔═══██╗██╔═══██╗██╔══██╗
# ███████╗██║     ██║   ██║██║   ██║██████╔╝
# ╚════██║██║     ██║   ██║██║   ██║██╔═══╝
# ███████║███████╗╚██████╔╝╚██████╔╝██║
# ╚══════╝╚══════╝ ╚═════╝  ╚═════╝ ╚═╝
# =============================================================================

class SLoop:
    """
    The self-referential loop: observer-system coupling.

    Visual representation of the sLoop concept:
        - The measurement apparatus is part of the flux field
        - Observer and observed share common substrate
        - This enables Bell violations without superluminal signaling
    """

    def __init__(self, center, radius=1.5):
        self.center = Vector(center)
        self.radius = radius
        self.objects = []

    def build(self, collection):
        """Create sLoop visualization."""
        # Möbius-like structure to show self-reference
        mat = MaterialFactory.get_or_create(
            "sLoop", ColorPalette.GENESIS, emission=2.0, alpha=0.6
        )

        # Create a trefoil knot or figure-8 to represent self-reference
        # Using a torus knot as approximation
        vertices = []
        edges = []

        n_points = 100
        p, q = 2, 3  # Trefoil knot parameters

        for i in range(n_points):
            t = 2 * math.pi * i / n_points

            # Trefoil knot parametric equations
            r = self.radius * (math.cos(q * t) + 2)
            x = r * math.cos(p * t)
            y = r * math.sin(p * t)
            z = -self.radius * 0.5 * math.sin(q * t)

            vertices.append(self.center + Vector((x, y, z)))

        # Create tube along the knot
        for i in range(n_points):
            p1 = vertices[i]
            p2 = vertices[(i + 1) % n_points]

            tube = GeometryBuilder.create_cylinder(
                p1, p2, 0.03, mat, f"sLoop_seg{i}"
            )
            if tube:
                collection.objects.link(tube)
                self.objects.append(tube)

        return self.objects


# =============================================================================
# ███████╗██╗  ██╗██╗███████╗████████╗███████╗███╗   ██╗ ██████╗███████╗
# ██╔════╝╚██╗██╔╝██║██╔════╝╚══██╔══╝██╔════╝████╗  ██║██╔════╝██╔════╝
# █████╗   ╚███╔╝ ██║███████╗   ██║   █████╗  ██╔██╗ ██║██║     █████╗
# ██╔══╝   ██╔██╗ ██║╚════██║   ██║   ██╔══╝  ██║╚██╗██║██║     ██╔══╝
# ███████╗██╔╝ ██╗██║███████║   ██║   ███████╗██║ ╚████║╚██████╗███████╗
# ╚══════╝╚═╝  ╚═╝╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═══╝ ╚═════╝╚══════╝
# =============================================================================

class TRDExistence:
    """
    The complete TRD simulation: a small existence.

    This is the main class that orchestrates the creation of:
        1. The lattice substrate (heptagonal antiprism cells)
        2. The flux field (vector arrows)
        3. Manifestation events (triads, shells)
        4. The sLoop structure (self-reference)
    """

    def __init__(self):
        self.cells = []
        self.triads = []
        self.shells = []
        self.sloop = None
        self.flux_field = None

        self.collections = {}

    def clear_scene(self):
        """Remove all existing objects."""
        # Deselect all
        bpy.ops.object.select_all(action='DESELECT')

        # Select all mesh objects
        for obj in bpy.data.objects:
            if obj.type in ['MESH', 'CURVE', 'LIGHT', 'CAMERA']:
                obj.select_set(True)

        # Delete selected
        bpy.ops.object.delete()

        # Clear orphan data
        for block in bpy.data.meshes:
            if block.users == 0:
                bpy.data.meshes.remove(block)

        for block in bpy.data.materials:
            if block.users == 0:
                bpy.data.materials.remove(block)

        # Clear material cache
        MaterialFactory.clear_cache()

    def setup_collections(self):
        """Create organizational collections."""
        collection_names = [
            "TRD_Lattice",
            "TRD_Flux",
            "TRD_Manifestations",
            "TRD_sLoop",
            "TRD_Environment"
        ]

        for name in collection_names:
            if name not in bpy.data.collections:
                coll = bpy.data.collections.new(name)
                bpy.context.scene.collection.children.link(coll)
            self.collections[name] = bpy.data.collections[name]

    def build_lattice(self):
        """Construct the cell lattice."""
        print("\n▓▓▓ Building Lattice Substrate ▓▓▓")

        gx, gy, gz = VisualConfig.GRID_SIZE
        spacing = VisualConfig.CELL_SPACING

        cell_id = 0
        for ix in range(gx):
            for iy in range(gy):
                for iz in range(gz):
                    # Hexagonal packing offset
                    x_offset = (iy % 2) * spacing * 0.5
                    y_scale = math.sqrt(3) / 2

                    center = (
                        ix * spacing + x_offset,
                        iy * spacing * y_scale,
                        iz * VisualConfig.CELL_HEIGHT * 1.1
                    )

                    cell = HeptagonalAntiprismCell(center, cell_id)
                    cell.build(self.collections["TRD_Lattice"])
                    self.cells.append(cell)

                    print(f"  Cell {cell_id}: {center}")
                    cell_id += 1

        print(f"  Total cells: {len(self.cells)}")
        print(f"  Total vertices: {len(self.cells) * 14}")

    def build_flux_field(self):
        """Add flux vector visualization."""
        print("\n▓▓▓ Building Flux Field ▓▓▓")

        self.flux_field = FluxField(self.cells)
        arrows = self.flux_field.build(self.collections["TRD_Flux"], density=0.5)

        print(f"  Flux vectors: {len(arrows)}")

    def build_manifestations(self):
        """Add triads and shells."""
        print("\n▓▓▓ Building Manifestations ▓▓▓")

        # Add some triads at strategic locations
        triad_positions = [
            (VisualConfig.CELL_SPACING * 0.5, VisualConfig.CELL_SPACING * 0.5, 0),
            (VisualConfig.CELL_SPACING * 1.5, VisualConfig.CELL_SPACING * 1.2, VisualConfig.CELL_HEIGHT * 0.5),
        ]

        for i, pos in enumerate(triad_positions):
            triad = Triad(pos, scale=1.2, polarity=1 if i % 2 == 0 else -1)
            triad.build(self.collections["TRD_Manifestations"])
            self.triads.append(triad)
            print(f"  Triad {i} at {pos}")

        # Add electron shells around one triad
        if self.triads:
            triad_center = self.triads[0].center
            for n in range(1, 3):  # n=1 and n=2 shells
                shell = ElectronShell(triad_center, n_level=n, num_electrons=n*2)
                shell.build(self.collections["TRD_Manifestations"])
                self.shells.append(shell)
                print(f"  Shell n={n} with {n*2} electrons")

    def build_sloop(self):
        """Add sLoop visualization."""
        print("\n▓▓▓ Building sLoop ▓▓▓")

        # Place sLoop at center of lattice
        gx, gy, gz = VisualConfig.GRID_SIZE
        center = (
            (gx - 1) * VisualConfig.CELL_SPACING / 2,
            (gy - 1) * VisualConfig.CELL_SPACING * math.sqrt(3) / 4,
            (gz - 1) * VisualConfig.CELL_HEIGHT * 0.55 + 0.5
        )

        self.sloop = SLoop(center, radius=0.8)
        self.sloop.build(self.collections["TRD_sLoop"])

        print(f"  sLoop at {center}")

    def setup_environment(self):
        """Configure lighting, camera, and world."""
        print("\n▓▓▓ Setting Up Environment ▓▓▓")

        coll = self.collections["TRD_Environment"]

        # === CAMERA ===
        gx, gy, gz = VisualConfig.GRID_SIZE

        cam_data = bpy.data.cameras.new("TRD_Camera")
        camera = bpy.data.objects.new("TRD_Camera", cam_data)

        # Position camera to see the whole lattice
        camera.location = (
            gx * VisualConfig.CELL_SPACING * 1.2,
            -gy * VisualConfig.CELL_SPACING * 0.8,
            gz * VisualConfig.CELL_HEIGHT * 2.5
        )

        # Point at center
        center = Vector((
            (gx - 1) * VisualConfig.CELL_SPACING / 2,
            (gy - 1) * VisualConfig.CELL_SPACING * math.sqrt(3) / 4,
            (gz - 1) * VisualConfig.CELL_HEIGHT * 0.5
        ))

        direction = center - camera.location
        rot_quat = direction.to_track_quat('-Z', 'Y')
        camera.rotation_euler = rot_quat.to_euler()

        coll.objects.link(camera)
        bpy.context.scene.camera = camera
        print(f"  Camera at {tuple(camera.location)}")

        # === LIGHTING ===

        # Key light (main illumination)
        key_data = bpy.data.lights.new("TRD_KeyLight", 'AREA')
        key_data.energy = 500
        key_data.size = 10
        key_data.color = (1.0, 0.98, 0.95)
        key_light = bpy.data.objects.new("TRD_KeyLight", key_data)
        key_light.location = (5, -8, 12)
        coll.objects.link(key_light)

        # Fill light (softer, from opposite side)
        fill_data = bpy.data.lights.new("TRD_FillLight", 'AREA')
        fill_data.energy = 200
        fill_data.size = 8
        fill_data.color = (0.9, 0.95, 1.0)
        fill_light = bpy.data.objects.new("TRD_FillLight", fill_data)
        fill_light.location = (-6, 5, 8)
        coll.objects.link(fill_light)

        # Rim light (edge definition)
        rim_data = bpy.data.lights.new("TRD_RimLight", 'SPOT')
        rim_data.energy = 1000
        rim_data.spot_size = math.radians(45)
        rim_data.color = (0.8, 0.9, 1.0)
        rim_light = bpy.data.objects.new("TRD_RimLight", rim_data)
        rim_light.location = (-3, -5, 15)
        rim_light.rotation_euler = (math.radians(30), 0, math.radians(-20))
        coll.objects.link(rim_light)

        print("  Lights: Key, Fill, Rim")

        # === WORLD ===
        world = bpy.context.scene.world
        if world is None:
            world = bpy.data.worlds.new("TRD_World")
            bpy.context.scene.world = world

        world.use_nodes = True
        nodes = world.node_tree.nodes

        bg = nodes.get('Background')
        if bg:
            bg.inputs['Color'].default_value = ColorPalette.BACKGROUND
            bg.inputs['Strength'].default_value = 0.1

        print("  World background: deep space")

        # === RENDER SETTINGS ===
        scene = bpy.context.scene
        scene.render.engine = 'CYCLES'
        scene.cycles.samples = VisualConfig.SAMPLES
        scene.render.resolution_x = VisualConfig.RESOLUTION[0]
        scene.render.resolution_y = VisualConfig.RESOLUTION[1]

        # Enable bloom in compositor if available
        scene.render.use_compositor = True
        scene.use_nodes = True

        print(f"  Render: {VisualConfig.RESOLUTION[0]}x{VisualConfig.RESOLUTION[1]}, {VisualConfig.SAMPLES} samples")

    def add_annotations(self):
        """Add text labels explaining the structure."""
        print("\n▓▓▓ Adding Annotations ▓▓▓")

        # Create text objects for key concepts
        annotations = [
            ("b₃ = 7", (0, -1, 3), "Gauss Constraints"),
            ("N_c = 3", (3, -1, 2), "Color Charges"),
            ("+1 / -1", (5, 0, 1.5), "Duality"),
            ("ψ = J_x + iJ_y", (-1, 2, 3), "Wave Function"),
        ]

        for text, location, description in annotations:
            bpy.ops.object.text_add(location=location)
            txt = bpy.context.active_object
            txt.data.body = text
            txt.data.size = 0.3
            txt.data.align_x = 'CENTER'

            # Make text face camera
            txt.rotation_euler = (math.radians(90), 0, 0)

            # Material
            mat = MaterialFactory.get_or_create(
                f"Text_{text}", ColorPalette.FLUX_HIGH, emission=1.0
            )
            txt.data.materials.append(mat)

            self.collections["TRD_Environment"].objects.link(txt)
            bpy.context.collection.objects.unlink(txt)

            print(f"  '{text}' - {description}")

    def create(self):
        """
        Main creation method: builds the complete TRD existence.
        """
        print("\n" + "=" * 70)
        print("  TRD EXISTENCE GENERATOR")
        print("  'The void is not empty—it is potential awaiting activation.'")
        print("=" * 70)

        # Phase 1: Clear and setup
        print("\n▓▓▓ Phase 1: Initialization ▓▓▓")
        self.clear_scene()
        self.setup_collections()

        # Phase 2: Build substrate
        print("\n▓▓▓ Phase 2: Substrate ▓▓▓")
        self.build_lattice()

        # Phase 3: Build flux field
        print("\n▓▓▓ Phase 3: Flux Field ▓▓▓")
        self.build_flux_field()

        # Phase 4: Build manifestations
        print("\n▓▓▓ Phase 4: Manifestations ▓▓▓")
        self.build_manifestations()

        # Phase 5: Build sLoop
        print("\n▓▓▓ Phase 5: sLoop ▓▓▓")
        self.build_sloop()

        # Phase 6: Environment
        print("\n▓▓▓ Phase 6: Environment ▓▓▓")
        self.setup_environment()

        # Phase 7: Annotations
        # self.add_annotations()  # Optional: uncomment for labeled version

        # Summary
        print("\n" + "=" * 70)
        print("  CREATION COMPLETE")
        print("=" * 70)
        print(f"""
  Statistics:
    Cells:          {len(self.cells)}
    Vertices:       {len(self.cells) * 15} (14 boundary + 1 void each)
    Triads:         {len(self.triads)}
    Shells:         {len(self.shells)}
    sLoop:          {'Yes' if self.sloop else 'No'}

  Framework Constants Encoded:
    b₃ = {TRDConstants.B3}    (cell symmetry)
    N_c = {TRDConstants.NC}     (triad structure)
    n_eff = {TRDConstants.NEFF}  (F₇)
    φ = {TRDConstants.PHI:.6f}  (binding ratio)

  "Events are ontic. Constraints are real. Meaning is emergent."
""")
        print("=" * 70)


# =============================================================================
# ███╗   ███╗ █████╗ ██╗███╗   ██╗
# ████╗ ████║██╔══██╗██║████╗  ██║
# ██╔████╔██║███████║██║██╔██╗ ██║
# ██║╚██╔╝██║██╔══██║██║██║╚██╗██║
# ██║ ╚═╝ ██║██║  ██║██║██║ ╚████║
# ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝
# =============================================================================

if __name__ == "__main__":
    existence = TRDExistence()
    existence.create()
