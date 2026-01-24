"""
═══════════════════════════════════════════════════════════════════════════════
    TRD ANIMATION SYSTEM: Breathing Life into Existence
═══════════════════════════════════════════════════════════════════════════════

    "Between moments, waves exist. Within moments, events are definite."

    This module adds temporal dynamics to the TRD visualization:

    ANIMATION 1: THE HEARTBEAT
        - Cells pulse with manifestation probability
        - Frequency encodes energy (E = hf)
        - Phase relationships show entanglement

    ANIMATION 2: FLUX WAVES
        - Ripples propagate at speed C = 1
        - Interference patterns emerge
        - Standing waves form stable structures

    ANIMATION 3: GENESIS/ANNIHILATION
        - Pair production events (0 → +1, -1)
        - Annihilation bursts (+1 + -1 → 0 + energy)
        - Flux redistribution to neighbors

    ANIMATION 4: THE sLOOP ROTATION
        - Self-referential structure rotates
        - Observer-observed coupling visualized
        - Bell correlation emergence

    ANIMATION 5: SHELL DYNAMICS
        - Electrons orbit nuclei
        - Quantum jumps between shells
        - Photon emission/absorption

    Run this AFTER trd_existence.py to add animation.

    Author: TRD Animation System
    Date: January 2026
═══════════════════════════════════════════════════════════════════════════════
"""

import bpy
import math
import random
from mathutils import Vector

# =============================================================================
# CONFIGURATION
# =============================================================================

class AnimConfig:
    """Animation parameters."""

    # Timeline
    FRAME_START = 1
    FRAME_END = 300
    FPS = 30

    # Heartbeat (manifestation pulse)
    PULSE_FREQUENCY = 0.05      # Cycles per frame
    PULSE_AMPLITUDE = 0.3       # Scale variation
    PULSE_PHASE_SPREAD = 2.0    # Phase difference between cells

    # Flux waves
    WAVE_SPEED = 0.1            # Propagation rate
    WAVE_AMPLITUDE = 0.5        # Displacement magnitude
    WAVE_DECAY = 0.02           # Damping per unit distance

    # Genesis/Annihilation
    EVENT_PROBABILITY = 0.01    # Chance per cell per frame
    BURST_DURATION = 15         # Frames for energy burst
    BURST_SCALE = 3.0           # Maximum burst size

    # sLoop
    SLOOP_ROTATION_SPEED = 0.02  # Radians per frame

    # Shells
    ELECTRON_ORBITAL_SPEED = 0.05  # Radians per frame
    QUANTUM_JUMP_PROB = 0.005      # Jump probability per frame


# =============================================================================
# ANIMATION UTILITIES
# =============================================================================

def get_objects_by_prefix(prefix):
    """Find all objects with names starting with prefix."""
    return [obj for obj in bpy.data.objects if obj.name.startswith(prefix)]


def get_objects_in_collection(collection_name):
    """Get all objects in a specific collection."""
    if collection_name in bpy.data.collections:
        return list(bpy.data.collections[collection_name].objects)
    return []


def insert_keyframe(obj, data_path, frame, value=None):
    """Insert a keyframe for an object property."""
    if value is not None:
        # Set the value first
        parts = data_path.split('.')
        target = obj
        for part in parts[:-1]:
            if '[' in part:
                attr, idx = part.split('[')
                idx = int(idx.rstrip(']'))
                target = getattr(target, attr)[idx]
            else:
                target = getattr(target, part)

        final_attr = parts[-1]
        if '[' in final_attr:
            attr, idx = final_attr.split('[')
            idx = int(idx.rstrip(']'))
            getattr(target, attr)[idx] = value
        else:
            setattr(target, final_attr, value)

    obj.keyframe_insert(data_path=data_path, frame=frame)


def set_interpolation(obj, data_path, interpolation='BEZIER'):
    """Set keyframe interpolation type."""
    if obj.animation_data and obj.animation_data.action:
        for fcurve in obj.animation_data.action.fcurves:
            if fcurve.data_path == data_path:
                for keyframe in fcurve.keyframe_points:
                    keyframe.interpolation = interpolation


# =============================================================================
# ANIMATION 1: THE HEARTBEAT
# =============================================================================

class HeartbeatAnimation:
    """
    Makes cells pulse with manifestation probability.

    The pulse represents the flux density |J| oscillating around
    the manifestation threshold K_B. When |J| > K_B, genesis can occur.

    Different cells have different phases, creating a breathing pattern
    across the lattice—like a living organism.
    """

    def __init__(self):
        self.void_objects = []
        self.positive_objects = []
        self.negative_objects = []

    def find_objects(self):
        """Locate all vertex objects in the lattice."""
        self.void_objects = get_objects_by_prefix("Cell")
        self.void_objects = [o for o in self.void_objects if "Void" in o.name]

        self.positive_objects = [o for o in bpy.data.objects
                                  if "Top" in o.name and "Cell" in o.name]

        self.negative_objects = [o for o in bpy.data.objects
                                  if "Bot" in o.name and "Cell" in o.name]

        print(f"  Found {len(self.void_objects)} void centers")
        print(f"  Found {len(self.positive_objects)} positive vertices")
        print(f"  Found {len(self.negative_objects)} negative vertices")

    def animate(self):
        """Create the heartbeat animation."""
        print("\n▓▓▓ Animating Heartbeat ▓▓▓")
        self.find_objects()

        # Animate void centers (the "heart" of each cell)
        for i, obj in enumerate(self.void_objects):
            # Each void has a unique phase based on position
            phase = (obj.location.x + obj.location.y + obj.location.z) * AnimConfig.PULSE_PHASE_SPREAD

            for frame in range(AnimConfig.FRAME_START, AnimConfig.FRAME_END + 1, 5):
                t = frame * AnimConfig.PULSE_FREQUENCY
                pulse = 1.0 + AnimConfig.PULSE_AMPLITUDE * math.sin(2 * math.pi * t + phase)

                obj.scale = (pulse, pulse, pulse)
                obj.keyframe_insert(data_path="scale", frame=frame)

                # Also pulse emission
                if obj.data.materials:
                    mat = obj.data.materials[0]
                    if mat.use_nodes:
                        principled = mat.node_tree.nodes.get('Principled BSDF')
                        if principled:
                            emission = 0.8 + 0.7 * math.sin(2 * math.pi * t + phase)
                            principled.inputs['Emission Strength'].default_value = emission
                            principled.inputs['Emission Strength'].keyframe_insert(
                                data_path='default_value', frame=frame
                            )

        # Animate positive vertices (slower, offset phase)
        for i, obj in enumerate(self.positive_objects):
            phase = (obj.location.x - obj.location.y) * AnimConfig.PULSE_PHASE_SPREAD + math.pi/4

            for frame in range(AnimConfig.FRAME_START, AnimConfig.FRAME_END + 1, 8):
                t = frame * AnimConfig.PULSE_FREQUENCY * 0.7
                pulse = 1.0 + AnimConfig.PULSE_AMPLITUDE * 0.5 * math.sin(2 * math.pi * t + phase)

                obj.scale = (pulse, pulse, pulse)
                obj.keyframe_insert(data_path="scale", frame=frame)

        # Animate negative vertices (complementary to positive)
        for i, obj in enumerate(self.negative_objects):
            phase = (obj.location.x - obj.location.y) * AnimConfig.PULSE_PHASE_SPREAD + math.pi/4 + math.pi

            for frame in range(AnimConfig.FRAME_START, AnimConfig.FRAME_END + 1, 8):
                t = frame * AnimConfig.PULSE_FREQUENCY * 0.7
                pulse = 1.0 + AnimConfig.PULSE_AMPLITUDE * 0.5 * math.sin(2 * math.pi * t + phase)

                obj.scale = (pulse, pulse, pulse)
                obj.keyframe_insert(data_path="scale", frame=frame)

        print("  Heartbeat animation complete")


# =============================================================================
# ANIMATION 2: FLUX WAVES
# =============================================================================

class FluxWaveAnimation:
    """
    Animates flux vectors to show wave propagation.

    Flux waves propagate at speed C = 1 (one lattice unit per tick).
    They carry energy and can interfere constructively or destructively.
    """

    def __init__(self):
        self.flux_objects = []

    def find_objects(self):
        """Locate flux arrow objects."""
        self.flux_objects = get_objects_by_prefix("Flux_")
        print(f"  Found {len(self.flux_objects)} flux vectors")

    def animate(self):
        """Create wave propagation animation."""
        print("\n▓▓▓ Animating Flux Waves ▓▓▓")
        self.find_objects()

        if not self.flux_objects:
            print("  No flux objects found, skipping")
            return

        # Wave source at center
        source = Vector((3, 2, 1))

        for obj in self.flux_objects:
            distance = (Vector(obj.location) - source).length

            for frame in range(AnimConfig.FRAME_START, AnimConfig.FRAME_END + 1, 3):
                # Wave equation: amplitude * sin(k*r - omega*t) * exp(-decay*r)
                t = frame * AnimConfig.WAVE_SPEED
                k = 1.0  # Wave number
                omega = 0.5  # Angular frequency

                wave = math.sin(k * distance - omega * t) * math.exp(-AnimConfig.WAVE_DECAY * distance)

                # Modulate scale
                scale = 1.0 + AnimConfig.WAVE_AMPLITUDE * wave
                obj.scale = (scale, scale, scale)
                obj.keyframe_insert(data_path="scale", frame=frame)

                # Modulate rotation (flux direction oscillates)
                if hasattr(obj, 'rotation_euler'):
                    obj.rotation_euler.z = wave * 0.3
                    obj.keyframe_insert(data_path="rotation_euler", frame=frame)

        print("  Flux wave animation complete")


# =============================================================================
# ANIMATION 3: GENESIS/ANNIHILATION EVENTS
# =============================================================================

class GenesisAnimation:
    """
    Animates manifestation events.

    Genesis: A void voxel (0) manifests as +1 or -1
    Annihilation: Adjacent +1 and -1 return to void with energy burst

    These events are probabilistic, governed by |J| > K_B.
    """

    def __init__(self):
        self.event_objects = []

    def create_burst(self, location, frame_start, is_genesis=True):
        """Create an energy burst at a location."""
        # Create a sphere that expands and fades
        bpy.ops.mesh.primitive_uv_sphere_add(
            radius=0.1,
            location=location,
            segments=16,
            ring_count=8
        )
        burst = bpy.context.active_object
        burst.name = f"{'Genesis' if is_genesis else 'Annihilation'}Burst_{frame_start}"

        # Material
        color = (1.0, 0.9, 0.5, 1.0) if is_genesis else (1.0, 0.3, 0.8, 1.0)
        mat = bpy.data.materials.new(name=f"Burst_{frame_start}")
        mat.use_nodes = True
        mat.blend_method = 'BLEND'

        principled = mat.node_tree.nodes.get('Principled BSDF')
        if principled:
            principled.inputs['Base Color'].default_value = color
            principled.inputs['Emission Color'].default_value = color
            principled.inputs['Emission Strength'].default_value = 10.0
            principled.inputs['Alpha'].default_value = 1.0

        burst.data.materials.append(mat)

        # Animate: start small, expand, fade
        duration = AnimConfig.BURST_DURATION

        # Frame 0: small, bright
        burst.scale = (0.1, 0.1, 0.1)
        burst.keyframe_insert(data_path="scale", frame=frame_start)
        principled.inputs['Emission Strength'].default_value = 10.0
        principled.inputs['Emission Strength'].keyframe_insert(
            data_path='default_value', frame=frame_start
        )
        principled.inputs['Alpha'].default_value = 1.0
        principled.inputs['Alpha'].keyframe_insert(
            data_path='default_value', frame=frame_start
        )

        # Frame mid: expanded
        mid_frame = frame_start + duration // 2
        burst.scale = (AnimConfig.BURST_SCALE, AnimConfig.BURST_SCALE, AnimConfig.BURST_SCALE)
        burst.keyframe_insert(data_path="scale", frame=mid_frame)
        principled.inputs['Emission Strength'].default_value = 5.0
        principled.inputs['Emission Strength'].keyframe_insert(
            data_path='default_value', frame=mid_frame
        )

        # Frame end: faded
        end_frame = frame_start + duration
        burst.scale = (AnimConfig.BURST_SCALE * 1.5, AnimConfig.BURST_SCALE * 1.5, AnimConfig.BURST_SCALE * 1.5)
        burst.keyframe_insert(data_path="scale", frame=end_frame)
        principled.inputs['Emission Strength'].default_value = 0.0
        principled.inputs['Emission Strength'].keyframe_insert(
            data_path='default_value', frame=end_frame
        )
        principled.inputs['Alpha'].default_value = 0.0
        principled.inputs['Alpha'].keyframe_insert(
            data_path='default_value', frame=end_frame
        )

        # Hide before and after
        burst.hide_viewport = True
        burst.hide_render = True
        burst.keyframe_insert(data_path="hide_viewport", frame=frame_start - 1)
        burst.keyframe_insert(data_path="hide_render", frame=frame_start - 1)

        burst.hide_viewport = False
        burst.hide_render = False
        burst.keyframe_insert(data_path="hide_viewport", frame=frame_start)
        burst.keyframe_insert(data_path="hide_render", frame=frame_start)

        burst.hide_viewport = True
        burst.hide_render = True
        burst.keyframe_insert(data_path="hide_viewport", frame=end_frame)
        burst.keyframe_insert(data_path="hide_render", frame=end_frame)

        self.event_objects.append(burst)
        return burst

    def animate(self):
        """Create genesis and annihilation events."""
        print("\n▓▓▓ Animating Genesis/Annihilation Events ▓▓▓")

        # Find void centers to use as event locations
        void_objects = get_objects_by_prefix("Cell")
        void_objects = [o for o in void_objects if "Void" in o.name]

        if not void_objects:
            print("  No void centers found, creating events at fixed locations")
            void_objects = [type('obj', (object,), {'location': Vector((2, 2, 1))})()]

        # Create several random events
        num_events = min(10, len(void_objects))
        event_frames = sorted(random.sample(range(30, AnimConfig.FRAME_END - 50), num_events))

        for i, frame in enumerate(event_frames):
            void = random.choice(void_objects)
            is_genesis = random.random() > 0.3  # More genesis than annihilation

            # Offset slightly from void center
            offset = Vector((
                random.uniform(-0.3, 0.3),
                random.uniform(-0.3, 0.3),
                random.uniform(-0.2, 0.2)
            ))
            location = Vector(void.location) + offset

            self.create_burst(location, frame, is_genesis)

            event_type = "Genesis" if is_genesis else "Annihilation"
            print(f"  {event_type} at frame {frame}, location {tuple(location)[:3]}")

        print(f"  Created {len(self.event_objects)} events")


# =============================================================================
# ANIMATION 4: sLOOP ROTATION
# =============================================================================

class SLoopAnimation:
    """
    Animates the self-referential loop structure.

    The sLoop rotates slowly, representing the continuous
    self-observation of the system. The rotation speed
    could be related to the frame rate of consciousness.
    """

    def __init__(self):
        self.sloop_objects = []

    def find_objects(self):
        """Locate sLoop objects."""
        self.sloop_objects = get_objects_by_prefix("sLoop")
        print(f"  Found {len(self.sloop_objects)} sLoop segments")

    def animate(self):
        """Create sLoop rotation animation."""
        print("\n▓▓▓ Animating sLoop ▓▓▓")
        self.find_objects()

        if not self.sloop_objects:
            print("  No sLoop objects found, skipping")
            return

        # Find the center of the sLoop
        if self.sloop_objects:
            center = Vector((0, 0, 0))
            for obj in self.sloop_objects:
                center += Vector(obj.location)
            center /= len(self.sloop_objects)

            # Create an empty as rotation parent
            bpy.ops.object.empty_add(type='PLAIN_AXES', location=center)
            pivot = bpy.context.active_object
            pivot.name = "sLoop_Pivot"

            # Parent all sLoop objects
            for obj in self.sloop_objects:
                obj.parent = pivot

            # Animate rotation
            for frame in range(AnimConfig.FRAME_START, AnimConfig.FRAME_END + 1, 5):
                angle = frame * AnimConfig.SLOOP_ROTATION_SPEED
                pivot.rotation_euler = (0, 0, angle)
                pivot.keyframe_insert(data_path="rotation_euler", frame=frame)

            # Also add slight wobble
            for frame in range(AnimConfig.FRAME_START, AnimConfig.FRAME_END + 1, 10):
                wobble_x = 0.1 * math.sin(frame * 0.05)
                wobble_y = 0.1 * math.cos(frame * 0.07)
                pivot.rotation_euler.x = wobble_x
                pivot.rotation_euler.y = wobble_y
                pivot.keyframe_insert(data_path="rotation_euler", frame=frame)

        print("  sLoop rotation animation complete")


# =============================================================================
# ANIMATION 5: ELECTRON ORBITALS
# =============================================================================

class OrbitalAnimation:
    """
    Animates electrons in their shells.

    Electrons orbit nuclei at quantized radii (n² scaling).
    Occasionally, quantum jumps occur between shells.
    """

    def __init__(self):
        self.electrons = []
        self.shells = []

    def find_objects(self):
        """Locate electron and shell objects."""
        self.electrons = get_objects_by_prefix("Electron")
        self.shells = get_objects_by_prefix("Shell")
        print(f"  Found {len(self.electrons)} electrons")
        print(f"  Found {len(self.shells)} shells")

    def animate(self):
        """Create orbital animation."""
        print("\n▓▓▓ Animating Electron Orbitals ▓▓▓")
        self.find_objects()

        if not self.electrons:
            print("  No electrons found, skipping")
            return

        for electron in self.electrons:
            # Determine which shell this electron belongs to
            # by parsing the name (e.g., "Electron_n1_0")
            parts = electron.name.split('_')
            n_level = 1
            for part in parts:
                if part.startswith('n'):
                    try:
                        n_level = int(part[1:])
                    except:
                        pass

            # Orbital speed inversely proportional to n (Kepler-like)
            speed = AnimConfig.ELECTRON_ORBITAL_SPEED / n_level

            # Find orbital center (approximate from initial position)
            initial_pos = Vector(electron.location)
            # Assume center is at z-coordinate of electron, at xy = 0
            # This is a simplification
            center_z = initial_pos.z
            radius = math.sqrt(initial_pos.x**2 + initial_pos.y**2)

            if radius < 0.1:
                continue  # Skip if too close to center

            initial_angle = math.atan2(initial_pos.y, initial_pos.x)

            # Animate circular motion
            for frame in range(AnimConfig.FRAME_START, AnimConfig.FRAME_END + 1, 2):
                angle = initial_angle + frame * speed

                # Add slight elliptical perturbation
                r = radius * (1 + 0.05 * math.sin(3 * angle))

                new_x = r * math.cos(angle)
                new_y = r * math.sin(angle)

                # Small z oscillation
                new_z = center_z + 0.02 * math.sin(5 * angle)

                electron.location = (new_x, new_y, new_z)
                electron.keyframe_insert(data_path="location", frame=frame)

        print("  Orbital animation complete")


# =============================================================================
# CAMERA ANIMATION
# =============================================================================

class CameraAnimation:
    """
    Animates the camera for a cinematic tour of the existence.
    """

    def __init__(self):
        self.camera = None

    def find_camera(self):
        """Locate the camera."""
        self.camera = bpy.context.scene.camera
        if not self.camera:
            for obj in bpy.data.objects:
                if obj.type == 'CAMERA':
                    self.camera = obj
                    break
        print(f"  Camera: {self.camera.name if self.camera else 'Not found'}")

    def animate(self):
        """Create camera movement."""
        print("\n▓▓▓ Animating Camera ▓▓▓")
        self.find_camera()

        if not self.camera:
            print("  No camera found, skipping")
            return

        # Define keyframe positions for a slow orbit
        # Start: wide shot
        # Mid: close-up on a cell
        # End: pull back to overview

        keyframes = [
            (1, (8, -6, 6), (1.0, 0, 0.5)),      # Wide establishing shot
            (75, (5, -3, 4), (0.9, 0, 0.6)),     # Move closer
            (150, (2, 0, 3), (0.7, 0, 0.8)),     # Close on center
            (200, (0, 4, 5), (0.8, 0.2, 0.5)),   # Orbit around
            (250, (6, 6, 4), (0.9, 0.1, 0.4)),   # Continue orbit
            (300, (8, -6, 6), (1.0, 0, 0.5)),    # Return to start
        ]

        # Lattice center (target to look at)
        target = Vector((2.5, 2.0, 1.0))

        for frame, location, _ in keyframes:
            self.camera.location = location
            self.camera.keyframe_insert(data_path="location", frame=frame)

            # Point at target
            direction = target - Vector(location)
            rot_quat = direction.to_track_quat('-Z', 'Y')
            self.camera.rotation_euler = rot_quat.to_euler()
            self.camera.keyframe_insert(data_path="rotation_euler", frame=frame)

        # Smooth the curves
        if self.camera.animation_data and self.camera.animation_data.action:
            for fcurve in self.camera.animation_data.action.fcurves:
                for keyframe in fcurve.keyframe_points:
                    keyframe.interpolation = 'BEZIER'
                    keyframe.handle_left_type = 'AUTO'
                    keyframe.handle_right_type = 'AUTO'

        print("  Camera animation complete")


# =============================================================================
# MASTER ANIMATOR
# =============================================================================

class TRDAnimator:
    """
    Master class that orchestrates all animations.
    """

    def __init__(self):
        self.heartbeat = HeartbeatAnimation()
        self.flux_waves = FluxWaveAnimation()
        self.genesis = GenesisAnimation()
        self.sloop = SLoopAnimation()
        self.orbitals = OrbitalAnimation()
        self.camera = CameraAnimation()

    def setup_timeline(self):
        """Configure animation timeline."""
        scene = bpy.context.scene
        scene.frame_start = AnimConfig.FRAME_START
        scene.frame_end = AnimConfig.FRAME_END
        scene.render.fps = AnimConfig.FPS
        scene.frame_current = AnimConfig.FRAME_START

        print(f"\n  Timeline: {AnimConfig.FRAME_START} to {AnimConfig.FRAME_END}")
        print(f"  Duration: {AnimConfig.FRAME_END / AnimConfig.FPS:.1f} seconds at {AnimConfig.FPS} FPS")

    def animate_all(self):
        """Run all animations."""
        print("\n" + "=" * 70)
        print("  TRD ANIMATION SYSTEM")
        print("  'Between moments, waves exist. Within moments, events are definite.'")
        print("=" * 70)

        self.setup_timeline()

        # Run each animation system
        self.heartbeat.animate()
        self.flux_waves.animate()
        self.genesis.animate()
        self.sloop.animate()
        self.orbitals.animate()
        self.camera.animate()

        # Summary
        print("\n" + "=" * 70)
        print("  ANIMATION COMPLETE")
        print("=" * 70)
        print(f"""
  Animated Elements:
    - Void heartbeat pulse (manifestation probability)
    - Flux wave propagation (information flow)
    - Genesis/Annihilation events (creation/destruction)
    - sLoop rotation (self-reference)
    - Electron orbitals (atomic structure)
    - Camera orbit (cinematic tour)

  Render Settings:
    Frames: {AnimConfig.FRAME_START} - {AnimConfig.FRAME_END}
    FPS: {AnimConfig.FPS}
    Duration: {AnimConfig.FRAME_END / AnimConfig.FPS:.1f}s

  To render:
    1. Set output path in Output Properties
    2. Choose format (PNG sequence or video)
    3. Render → Render Animation (Ctrl+F12)
""")
        print("=" * 70)


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    animator = TRDAnimator()
    animator.animate_all()
