# VoxelGenerator Quick Reference

**One-page cheat sheet for common tasks.**

---

## Initialization

```gdscript
var vg = VoxelGenerator.new()
add_child(vg)

vg.world_size = Vector3i(5, 5, 5)
vg.resolution = 1
vg.terrain_height = 4.0
vg.terrain_amplitude = 8.0
```

## Generation

| Method                   | Use Case              | Notes                             |
| ------------------------ | --------------------- | --------------------------------- |
| `vg.generate()`          | Small worlds, startup | Blocking, returns when done       |
| `vg.generate_async()`    | Large worlds, runtime | Non-blocking, returns immediately |
| `vg.cancel_generation()` | Stop async generation | Only works with async             |
| `vg.is_generating()`     | Check if running      | Returns bool                      |

## Display Debug Grids

```gdscript
vg.show_voxel_grid = true       # Wireframe voxel cubes
vg.show_chunk_grid = true       # Chunk boundaries (red)
vg.show_centers = false         # Voxel centers
vg.debug_mode = true            # Enable logging
vg.debug_verbosity = 2          # 0-3, higher = more verbose
```

## Configure Terrain

```gdscript
# Shape
vg.terrain_height = 4.0         # Base height
vg.terrain_amplitude = 8.0      # Height variation
vg.rock_influence = 0.3         # Detail blend (0-1)

# Performance
vg.resolution = 1               # 1=normal, 2=high (2x detail)
vg.chunk_size = 32              # 8-64, larger = fewer nodes
vg.vertex_limit = false         # Cap max vertices
```

## Noise Generators

```gdscript
var noise = NoiseGenerator.new()
noise.seed = 12345
noise.period = 50.0             # Frequency
noise.octaves = 4               # Complexity
noise.persistence = 0.5         # Falloff
noise.lacunarity = 2.0          # Frequency multiplier

vg.set_terrain_noise(noise)
```

## Biome Generator

```gdscript
var biome = BiomeGenerator.new()
biome.seed = 42
biome.blend_distance = 16

biome.add_biome(
	"Plains",
	min_height=-2, max_height=6,
	min_temperature=-0.5, max_temperature=0.5,
	min_humidity=-0.3, max_humidity=0.3,
	surface_blocks=[0], subsurface_blocks=[1],
	depth=4
)

vg.set_biome_generator(biome)
```

## Generation Modes

```gdscript
# Full 3D (slower, better for caves)
vg.generation_mode = VoxelGenerator.VOXELS_FIRST

# Heightmap + surface band (faster, default)
vg.generation_mode = VoxelGenerator.HEIGHTMAP_FIRST
vg.surface_band = 4.0           # Vertical band depth
```

## LOD (Level of Detail)

### Global LOD

```gdscript
vg.lod_level = 0                # 0=full, 7=minimal
```

### Distance-Based LOD

```gdscript
vg.enable_distance_lod = true
vg.lod_reference_position = player_pos

# Auto formula
vg.lod_distance_multiplier = 2.0
vg.reset_lod_distances_to_default()

# Or custom thresholds
vg.lod_distances = PackedFloat64Array([16, 32, 64, 128, 256, 512, 1024, 2048])

# Update at runtime
if vg.update_chunks_lod() > 0:
	vg.regenerate_dirty_chunks()

# Visualize LOD levels
vg.show_lod_colors = true
```

## Terraforming

```gdscript
# Dig hole
vg.dig_sphere(position, radius=3.0, strength=1.0)

# Build terrain
vg.build_sphere(position, radius=2.0, strength=1.0)

# Custom modification
vg.modify_terrain(center, radius, delta)  # delta: -=dig, +=build

# Apply changes
vg.regenerate_dirty_chunks()

# Manage edits
vg.clear_terrain_edits()        # Reset to original
vg.get_terrain_edit_count()     # Query edits
```

## Dirty Chunk System

```gdscript
# Mark for regeneration
vg.mark_chunk_dirty(Vector3i(0, 0, 0))
vg.mark_all_chunks_dirty()

# Region invalidation
vg.invalidate_density_region(
	Vector3i(0, 0, 0),
	Vector3i(10, 10, 10)
)

# Regenerate only dirty chunks
vg.regenerate_dirty_chunks()
```

## Save & Load

```gdscript
# Save
vg.save_map("user://saved_maps", "my_map")

# Load (restores generator params + meshes + edits)
vg.load_map("user://saved_maps", "my_map", strict_match=true)

# Serialize edits
var data = vg.get_terrain_edits_data()
vg.set_terrain_edits_data(data)
```

## Async Generation Settings

```gdscript
vg.max_chunks_per_frame = 4     # Chunks/frame (1-32)
vg.signal_every_n_chunks = 8    # Emit progress every N

# Signals
vg.chunk_ready.connect(func(idx, coord):
	print("Chunk ready: %s" % coord)
)

vg.generation_progress.connect(func(done, total):
	print("Progress: %d/%d" % [done, total])
)

vg.generation_complete.connect(func():
	print("Done!")
)
```

## Sampling Terrain

```gdscript
# Get density at world position
var density = vg.sample_density_at(Vector3(5, 10, 5))
if density < 0:
	print("Solid")

# Get voxel type at integer coords
var voxel_type = vg.get_voxel_at(Vector3i(5, 10, 5))
```

## Forcefield (World Boundary Walls)

```gdscript
vg.forcefield_enabled = true
vg.forcefield_height = 300.0
vg.forcefield_collision_enabled = true
vg.forcefield_detection_enabled = true
vg.forcefield_buffer = 0.5      # Inward offset

# Per-wall control
vg.set_forcefield_wall_enabled(VoxelGenerator.FF_NORTH, true)
vg.set_forcefield_wall_enabled(VoxelGenerator.FF_TOP, false)

# Available walls: FF_NORTH, FF_SOUTH, FF_EAST, FF_WEST, FF_TOP, FF_BOTTOM

# Events
vg.forcefield_body_entered.connect(func(wall_idx, body):
	print("Body hit wall %d" % wall_idx)
)
```

## Material

```gdscript
var mat = StandardMaterial3D.new()
mat.albedo_color = Color.WHITE

vg.set_terrain_material(mat)
```

## Debug Output

```gdscript
vg.debug_print_state()              # Print all settings
vg.debug_draw_noise_slice(0.0)      # Visualize noise at y=0
vg.log_message("Info", 1)           # 1=critical, 2=verbose, 3=very verbose
```

## Performance Tips

| Issue                | Solution                                               |
| -------------------- | ------------------------------------------------------ |
| Slow frame rate      | ↓ resolution, use HEIGHTMAP_FIRST, enable vertex_limit |
| Out of memory        | ↓ world_size, ↑ chunk_size, use distance LOD           |
| Terrain looks blocky | ↑ resolution (impacts FPS)                             |
| Need streaming       | enable_distance_lod = true, update every frame         |
| Small terrain OK     | Use generate() sync, good for testing                  |
| Large terrain OK     | Use generate_async(), tune max_chunks_per_frame        |

## Common Patterns

### Complete Setup

```gdscript
var vg = VoxelGenerator.new()
add_child(vg)

# Configure
vg.world_size = Vector3i(10, 10, 10)
vg.resolution = 2
vg.enable_distance_lod = true

# Setup noise
var noise = NoiseGenerator.new()
noise.seed = 42
vg.set_terrain_noise(noise)

# Generate
vg.generation_progress.connect(_on_progress)
vg.generation_complete.connect(_on_complete)
vg.generate_async()
```

### Runtime Terrain Editing

```gdscript
func _input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed:
		var result = raycast.get_collider()
		if result:
			vg.dig_sphere(result.position, 3.0, 1.0)
			vg.regenerate_dirty_chunks()
```

### Player-Following LOD

```gdscript
func _process(_delta: float) -> void:
	vg.lod_reference_position = player.global_position
	if vg.update_chunks_lod() > 0:
		vg.regenerate_dirty_chunks()
```

### Save/Load with Confirmation

```gdscript
func save_game(name: String) -> void:
	vg.save_map("user://saves", name)

func load_game(name: String) -> void:
	vg.load_map("user://saves", name, true)
```

---

See **[USAGE_GUIDE.md](USAGE_GUIDE.md)** for comprehensive documentation.
