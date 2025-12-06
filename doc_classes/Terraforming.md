# Terraforming API

The VoxelGenerator provides a terraforming system that allows you to dynamically modify terrain at runtime. Edits are stored separately from the procedural terrain, enabling save/load functionality and the ability to reset to the original terrain.

## Quick Start

```gdscript
# Get reference to your VoxelGenerator node
@onready var voxel_gen: VoxelGenerator = $VoxelGenerator

func _on_dig(hit_position: Vector3):
    # Dig a spherical hole at the hit position
    voxel_gen.dig_sphere(hit_position, 3.0, 1.0)
    # Apply changes (batched for performance)
    voxel_gen.regenerate_dirty_chunks()

func _on_build(hit_position: Vector3):
    # Build terrain at the hit position
    voxel_gen.build_sphere(hit_position, 2.0, 1.0)
    voxel_gen.regenerate_dirty_chunks()
```

## API Reference

### Core Methods

#### `dig_sphere(center: Vector3, radius: float, strength: float = 1.0)`

Removes terrain in a spherical region with smooth falloff.

| Parameter  | Type    | Description                               |
| ---------- | ------- | ----------------------------------------- |
| `center`   | Vector3 | World position of the dig center          |
| `radius`   | float   | Radius of effect in world units           |
| `strength` | float   | How much terrain to remove (default: 1.0) |

```gdscript
# Dig a small hole
voxel_gen.dig_sphere(position, 2.0)

# Dig a large crater
voxel_gen.dig_sphere(position, 8.0, 2.0)
```

---

#### `build_sphere(center: Vector3, radius: float, strength: float = 1.0)`

Adds terrain in a spherical region with smooth falloff.

| Parameter  | Type    | Description                            |
| ---------- | ------- | -------------------------------------- |
| `center`   | Vector3 | World position of the build center     |
| `radius`   | float   | Radius of effect in world units        |
| `strength` | float   | How much terrain to add (default: 1.0) |

```gdscript
# Build a small mound
voxel_gen.build_sphere(position, 3.0)

# Build a large hill
voxel_gen.build_sphere(position, 10.0, 1.5)
```

---

#### `modify_terrain(center: Vector3, radius: float, delta: float)`

Low-level terrain modification with direct control over the density delta.

| Parameter | Type    | Description                                       |
| --------- | ------- | ------------------------------------------------- |
| `center`  | Vector3 | World position of the edit center                 |
| `radius`  | float   | Radius of effect in world units                   |
| `delta`   | float   | Density change (negative = dig, positive = build) |

```gdscript
# Custom terrain modification
voxel_gen.modify_terrain(position, 5.0, -0.5)  # Gentle dig
voxel_gen.modify_terrain(position, 5.0, 0.5)   # Gentle build
```

---

#### `regenerate_dirty_chunks()`

Regenerates all chunks marked as dirty. **Call this after terrain modifications to apply changes.**

This method enables batching - you can make multiple modifications before calling this once:

```gdscript
# Multiple edits, single regeneration (efficient)
voxel_gen.dig_sphere(pos1, 2.0)
voxel_gen.dig_sphere(pos2, 2.0)
voxel_gen.build_sphere(pos3, 3.0)
voxel_gen.regenerate_dirty_chunks()  # Apply all at once
```

---

### Utility Methods

#### `clear_terrain_edits()`

Removes all terrain modifications and resets to the original procedural terrain.

```gdscript
func _on_reset_terrain():
    voxel_gen.clear_terrain_edits()
    voxel_gen.regenerate_dirty_chunks()
```

---

#### `get_terrain_edit_count() -> int`

Returns the number of stored terrain edits. Useful for UI or debugging.

```gdscript
print("Terrain edits: ", voxel_gen.get_terrain_edit_count())
```

---

### Save/Load Methods

#### `get_terrain_edits_data() -> PackedFloat32Array`

Serializes all terrain edits into a packed array for saving.

#### `set_terrain_edits_data(data: PackedFloat32Array)`

Loads terrain edits from a previously saved packed array.

```gdscript
# Save terrain edits
func save_terrain():
    var data = voxel_gen.get_terrain_edits_data()
    var file = FileAccess.open("user://terrain_edits.dat", FileAccess.WRITE)
    file.store_var(data)
    file.close()

# Load terrain edits
func load_terrain():
    if FileAccess.file_exists("user://terrain_edits.dat"):
        var file = FileAccess.open("user://terrain_edits.dat", FileAccess.READ)
        var data = file.get_var()
        file.close()
        voxel_gen.set_terrain_edits_data(data)
        voxel_gen.regenerate_dirty_chunks()
```

---

## Complete Example: First-Person Terraforming

```gdscript
extends CharacterBody3D

@onready var voxel_gen: VoxelGenerator = $"../VoxelGenerator"
@onready var raycast: RayCast3D = $Camera3D/RayCast3D

@export var dig_radius: float = 3.0
@export var build_radius: float = 2.5
@export var edit_strength: float = 1.0

func _input(event):
    if event is InputEventMouseButton and event.pressed:
        if raycast.is_colliding():
            var hit_pos = raycast.get_collision_point()
            var hit_normal = raycast.get_collision_normal()

            if event.button_index == MOUSE_BUTTON_LEFT:
                # Dig - offset slightly into surface
                voxel_gen.dig_sphere(hit_pos - hit_normal * 0.5, dig_radius, edit_strength)
                voxel_gen.regenerate_dirty_chunks()

            elif event.button_index == MOUSE_BUTTON_RIGHT:
                # Build - offset slightly away from surface
                voxel_gen.build_sphere(hit_pos + hit_normal * 0.5, build_radius, edit_strength)
                voxel_gen.regenerate_dirty_chunks()

func _on_reset_pressed():
    voxel_gen.clear_terrain_edits()
    voxel_gen.regenerate_dirty_chunks()
```

---

## Technical Details

### Smooth Falloff

All modifications use a smoothstep falloff function, creating natural-looking edits:

- Full strength at the center
- Gradually decreases toward the edge
- Zero effect outside the radius

### Thread Safety

The terraforming system is thread-safe and can be called from any thread. Internal mutex protection ensures safe concurrent access to the terrain edits map.

### Performance Tips

1. **Batch modifications**: Make multiple edits before calling `regenerate_dirty_chunks()`
2. **Use appropriate radii**: Larger radii affect more chunks
3. **Consider edit density**: Many small edits are stored individually; for extensive modifications, consider using larger radii with lower strength

### Coordinate System

- Terrain edits use world coordinates (same as Godot's global position)
- Internally mapped to voxel indices using floor rounding
- Supports coordinates in the range ±524,287 units
