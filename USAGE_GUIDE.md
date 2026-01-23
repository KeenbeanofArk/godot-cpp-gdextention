# Keen Voxel Engine: Usage Guide

This guide explains how to use the **VoxelGenerator** and **VoxelEngine** classes to create procedural voxel terrain in your Godot 4 project.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Architecture Overview](#architecture-overview)
3. [Basic Setup](#basic-setup)
4. [VoxelGenerator Configuration](#voxelgenerator-configuration)
5. [Generation Modes](#generation-modes)
6. [LOD System](#lod-system)
7. [Procedural Generators](#procedural-generators)
8. [Terraforming](#terraforming)
9. [Save & Load](#save--load)
10. [Performance Optimization](#performance-optimization)
11. [Advanced Topics](#advanced-topics)

---

## Quick Start

### Minimal Setup (GDScript with VoxelEngine)

```gdscript
extends Node3D

func _ready() -> void:
	# Create engine
	var engine = VoxelEngine.new()
	add_child(engine)

	# Create generator
	var voxel_gen = engine.create_generator()

	# Configure terrain
	voxel_gen.world_size = Vector3i(5, 5, 5)
	voxel_gen.resolution = 2
	voxel_gen.terrain_height = 4.0
	voxel_gen.terrain_amplitude = 8.0

	# Generate synchronously (good for small worlds)
	voxel_gen.generate()

	print("Terrain generated: %d chunks" % (voxel_gen.world_size.x * voxel_gen.world_size.y * voxel_gen.world_size.z))
```

### With Async Generation (Better for Large Worlds)

```gdscript
extends Node3D

var engine: VoxelEngine
var voxel_gen: VoxelGenerator

func _ready() -> void:
	# Create engine and generator
	engine = VoxelEngine.new()
	add_child(engine)
	voxel_gen = engine.create_generator()

	# Configure
	voxel_gen.world_size = Vector3i(10, 10, 10)

	# Connect to signals
	voxel_gen.generation_complete.connect(_on_generation_complete)
	voxel_gen.generation_progress.connect(_on_generation_progress)

	# Generate asynchronously (engine handles cleanup)
	voxel_gen.generate_async()

func _on_generation_progress(completed: int, total: int) -> void:
	print("Progress: %d/%d chunks" % [completed, total])

func _on_generation_complete() -> void:
	print("Terrain generation complete!")
```

---

## Architecture Overview

The voxel engine has four main components:

### 1. **VoxelEngine** (Facade & Lifecycle Manager)

- **Recommended** for production use
- Manages one or more VoxelGenerator instances
- Handles automatic cleanup and resource lifecycle
- Provides `create_generator()` factory method
- Prevents resource leaks from improper VoxelGenerator disposal

### 2. **VoxelGenerator** (Main Orchestrator)

- Creates and manages chunks
- Runs terrain generation (synchronous or async)
- Handles LOD and distance calculations
- Manages terraforming edits
- Provides save/load functionality
- **Note:** Use `VoxelEngine.create_generator()` instead of direct instantiation

### 3. **Chunk** (Discrete Terrain Units)

- Represents an 8×8×8 (or configurable size) cube of voxels
- Owns a MeshInstance3D for rendering
- Owns collision shapes
- Stores voxel data

### 4. **Voxel** (Terrain Data)

- Represents a single voxel with density and material
- Stored in flat 1D arrays for cache efficiency
- Supports VoxelType enum (AIR, DIRT, STONE, etc.)

### 5. **Noise/Biome/Feature Generators** (Sibling Systems)

- **NoiseGenerator**: Wraps FastNoiseLite for Perlin/Simplex noise
- **BiomeGenerator**: Produces height and biome data
- **FeatureGenerator**: Places decorative features (trees, ore, rocks)

---

## Basic Setup

### Recommended: Using VoxelEngine Facade

**VoxelEngine** is the recommended way to create and manage VoxelGenerator instances:

```gdscript
# Create VoxelEngine (lifecycle manager)
var engine = VoxelEngine.new()
add_child(engine)

# Create generators through the engine
var voxel_gen = engine.create_generator()

# Configure and generate
voxel_gen.world_size = Vector3i(5, 5, 5)
voxel_gen.generate()
```

**Benefits:**

- ✅ Automatic lifecycle management
- ✅ Prevents resource leaks
- ✅ Handles cleanup on scene exit
- ✅ Supports multiple generators per engine

### Alternative: Direct Instantiation (Not Recommended)

⚠️ **Deprecated** - may cause cleanup issues:

```gdscript
# Only use for small prototypes/testing
var voxel_gen = VoxelGenerator.new()
add_child(voxel_gen)
```

### Scene Tree Layout

```
World (Node3D)
├── VoxelEngine (manages generators)
│   └── VoxelGenerator (created via create_generator())
└── Player (Node3D)
```

### Required Properties

Before calling `generate()` or `generate_async()`, configure:

| Property            | Type     | Default | Description                      |
| ------------------- | -------- | ------- | -------------------------------- |
| `world_size`        | Vector3i | (1,1,1) | Number of chunks in X, Y, Z      |
| `chunk_size`        | int      | 32      | Voxels per chunk edge            |
| `resolution`        | int      | 1       | Marching cubes samples per voxel |
| `terrain_height`    | float    | 4.0     | Base terrain height              |
| `terrain_amplitude` | float    | 8.0     | Height variation range           |

```gdscript
var engine = VoxelEngine.new()
add_child(engine)

var voxel_gen = engine.create_generator()
voxel_gen.world_size = Vector3i(5, 5, 5)      # 125 chunks total
voxel_gen.chunk_size = 32                       # 32³ voxels per chunk
voxel_gen.resolution = 1                        # Standard Marching Cubes
voxel_gen.terrain_height = 4.0
voxel_gen.terrain_amplitude = 8.0
```

---

## VoxelGenerator Configuration

### Display Options

```gdscript
voxel_gen.show_voxel_grid = true      # Visualize voxel boundaries
voxel_gen.show_chunk_grid = true      # Visualize chunk boundaries
voxel_gen.show_centers = false        # Show voxel centers (slow)
voxel_gen.use_textures = true         # Use textured shader (requires material)
```

### Debug Options

```gdscript
voxel_gen.debug_mode = true           # Enable debug logging
voxel_gen.debug_verbosity = 2         # 0=off, 1=critical, 2=verbose, 3=very verbose
voxel_gen.visualize_noise_values = false  # Render noise slice visualization
```

### Generation Control

```gdscript
voxel_gen.auto_generate = true        # Auto-generate when ready
voxel_gen.vertex_limit = false        # Cap max vertices per chunk
voxel_gen.cutoff = 0.0                # Density threshold for solid/air
```

### Async Generation Tuning

```gdscript
voxel_gen.max_chunks_per_frame = 4    # Chunks to apply per frame (1-32)
voxel_gen.signal_every_n_chunks = 8   # Emit progress signal every N chunks
```

---

## Generation Modes

### Mode 1: VOXELS_FIRST (Full 3D)

Full 3D density evaluation. Evaluates terrain density at all voxel positions.

**Best for:** Complex 3D terrain (caves, overhangs, floating islands)

```gdscript
voxel_gen.generation_mode = VoxelGenerator.VOXELS_FIRST
voxel_gen.lod_level = 0  # Full resolution

voxel_gen.generate_async()
```

### Mode 2: HEIGHTMAP_FIRST (Surface Band Optimization)

**Default and Recommended.** Samples a 2D heightmap, then only meshes voxels within a vertical band around the surface.

**Best for:** Performance (processes ~80% fewer voxels), typical terrain

```gdscript
voxel_gen.generation_mode = VoxelGenerator.HEIGHTMAP_FIRST
voxel_gen.surface_band = 4.0  # Vertical band depth
voxel_gen.lod_level = 0

voxel_gen.generate_async()
```

---

## LOD System

### Global LOD Level

All chunks use the same LOD. Reduces marching cubes resolution at higher LOD levels:

```gdscript
voxel_gen.lod_level = 0  # Full detail (resolution = 1x)
voxel_gen.lod_level = 1  # Half detail (resolution = 0.5x)
voxel_gen.lod_level = 7  # Minimum detail (resolution = 1/128x)

voxel_gen.generate()
```

### Distance-Based LOD

Individual chunks get different LOD based on distance from a reference position (e.g., player):

```gdscript
voxel_gen.enable_distance_lod = true
voxel_gen.lod_reference_position = player_position

# Set LOD distance thresholds (8 levels)
voxel_gen.lod_distances = PackedFloat64Array([
	16.0,   # LOD 0 (nearest, full detail)
	32.0,   # LOD 1
	64.0,   # LOD 2
	128.0,  # LOD 3
	256.0,  # LOD 4
	512.0,  # LOD 5
	1024.0, # LOD 6
	2048.0  # LOD 7 (farthest, lowest detail)
])

# Or use default formula: chunk_size * multiplier^n
voxel_gen.lod_distance_multiplier = 2.0  # Double distance per LOD level
voxel_gen.reset_lod_distances_to_default()

voxel_gen.show_lod_colors = true  # Color-code chunks by LOD for visualization
```

### Update LOD at Runtime

```gdscript
func _process(_delta: float) -> void:
	# Update LOD reference position (e.g., player position)
	voxel_gen.lod_reference_position = player.global_position

	# Check for chunks that need LOD updates
	var chunks_to_regen = voxel_gen.update_chunks_lod()
	if chunks_to_regen > 0:
		voxel_gen.regenerate_dirty_chunks()
```

---

## Procedural Generators

### Noise Generator

Controls terrain shape with Perlin/Simplex noise:

```gdscript
var noise_gen = NoiseGenerator.new()
noise_gen.seed = 12345
noise_gen.period = 50.0       # Frequency of noise
noise_gen.octaves = 4         # Detail complexity
noise_gen.persistence = 0.5   # Falloff between octaves
noise_gen.lacunarity = 2.0    # Frequency multiplier per octave

voxel_gen.set_terrain_noise(noise_gen)

# Detail noise adds rocky surface detail
var detail_noise = NoiseGenerator.new()
detail_noise.seed = 12346
detail_noise.period = 10.0
detail_noise.octaves = 3
detail_noise.persistence = 0.6

voxel_gen.set_detail_noise(detail_noise)
voxel_gen.set_rock_influence(0.3)  # 0.0-1.0 blend
```

### Biome Generator

Produces different terrain heights and materials by biome:

```gdscript
var biome_gen = BiomeGenerator.new()
biome_gen.seed = 42
biome_gen.sea_level = 0.0
biome_gen.blend_distance = 16  # Smooth transitions between biomes

# Add a Plains biome
biome_gen.add_biome(
	"Plains",
	min_height = -2.0, max_height = 6.0,
	min_temperature = -0.5, max_temperature = 0.5,
	min_humidity = -0.3, max_humidity = 0.3,
	surface_blocks = [VoxelType.GRASS],
	subsurface_blocks = [VoxelType.DIRT],
	depth = 4
)

# Add a Mountain biome
biome_gen.add_biome(
	"Mountains",
	min_height = 10.0, max_height = 50.0,
	min_temperature = -1.0, max_temperature = -0.2,
	min_humidity = -0.5, max_humidity = 0.0,
	surface_blocks = [VoxelType.STONE],
	subsurface_blocks = [VoxelType.STONE],
	depth = 10
)

voxel_gen.set_biome_generator(biome_gen)
```

### Feature Generator

Places decorative features (trees, ore veins, rocks):

```gdscript
var feature_gen = FeatureGenerator.new()
feature_gen.seed = 99

# Add tree placement rule
feature_gen.add_tree_rule(
	probability = 0.1,  # 10% spawn rate
	biomes = [0, 1],    # Biome IDs 0, 1
	min_h = 5, max_h = 50
)

# Add ore vein rule
feature_gen.add_ore_rule(
	ore_voxel_type = VoxelType.COAL,
	probability = 0.05,
	min_h = -50, max_h = 30,
	cluster_size = 8
)

voxel_gen.set_feature_generator(feature_gen)
```

---

## Terraforming

Modify terrain at runtime with persistent edits.

### Digging (Removing Terrain)

```gdscript
var hit_pos = raycast_result.position
var radius = 3.0
var strength = 1.0

# Dig a spherical hole
voxel_gen.dig_sphere(hit_pos, radius, strength)

# Regenerate affected chunks
voxel_gen.regenerate_dirty_chunks()
```

### Building (Adding Terrain)

```gdscript
var build_pos = raycast_result.position
var radius = 2.0
var strength = 1.0

# Build terrain upward
voxel_gen.build_sphere(build_pos, radius, strength)

# Regenerate affected chunks
voxel_gen.regenerate_dirty_chunks()
```

### Manual Terrain Modification

```gdscript
# Smooth spherical falloff modification
voxel_gen.modify_terrain(
	center = Vector3(10.0, 5.0, 10.0),
	radius = 5.0,
	delta = -2.0  # Negative = dig, positive = build
)

voxel_gen.regenerate_dirty_chunks()
```

### Query and Manage Edits

```gdscript
# Get number of edits
var edit_count = voxel_gen.get_terrain_edit_count()
print("Terrain edits stored: %d" % edit_count)

# Serialize edits for save
var edits_data = voxel_gen.get_terrain_edits_data()

# Clear all edits and restore original terrain
voxel_gen.clear_terrain_edits()
voxel_gen.regenerate_dirty_chunks()
```

---

## Save & Load

### Saving a Map

```gdscript
# Save to user:// directory (persists between sessions)
voxel_gen.save_map("user://saved_maps", "my_terrain")

# Creates:
# - user://saved_maps/my_terrain/metadata.json
# - user://saved_maps/my_terrain/chunk_0_0_0.meshbin
# - user://saved_maps/my_terrain/chunk_1_0_0.meshbin
# - etc.
```

### Loading a Map

```gdscript
# Restore from saved map
voxel_gen.load_map("user://saved_maps", "my_terrain", strict_match = true)

# Restores:
# - Generator parameters (world_size, resolution, etc.)
# - Chunk meshes
# - Terrain edits (dig/build operations)
```

### What Gets Saved

- **Generator Parameters**: world_size, chunk_size, resolution, LOD settings
- **Chunk Meshes**: vertices, normals, colors per chunk
- **Terrain Edits**: all dig/build operations (sparse storage)
- **Feature Edits**: procedural feature density modifications
- **Metadata**: timestamp, map name, generator configuration

See [SAVE_LOAD_MAP.md](docs/SAVE_LOAD_MAP.md) for technical details.

---

## Performance Optimization

### Chunk Size Tuning

Larger chunks = fewer nodes but larger meshes:

```gdscript
voxel_gen.chunk_size = 16   # Small (more nodes, faster updates)
voxel_gen.chunk_size = 32   # Medium (balanced, recommended)
voxel_gen.chunk_size = 64   # Large (fewer nodes, slower updates)
```

### Resolution vs Frame Rate

Higher resolution = more triangles = slower rendering:

```gdscript
voxel_gen.resolution = 1    # Standard (normal detail)
voxel_gen.resolution = 2    # High (2x more triangles)
voxel_gen.resolution = 3    # Ultra (9x more triangles) - very slow

# Use vertex limit to cap output
voxel_gen.vertex_limit = true
voxel_gen.heightmap_vertex_limit = 8_000_000  # Max vertices for heightmap mode
```

### Async vs Sync Generation

```gdscript
# Small worlds: synchronous (instant, blocking)
if voxel_gen.world_size.x <= 5:
	voxel_gen.generate()

# Large worlds: asynchronous (non-blocking, progressive)
else:
	voxel_gen.max_chunks_per_frame = 4  # Tune this based on frame rate
	voxel_gen.generate_async()
```

### Dirty Chunk Regeneration

Mark specific chunks dirty instead of full world regen:

```gdscript
# Mark chunks affected by terraforming
voxel_gen.invalidate_density_region(
	Vector3i(0, 0, 0),
	Vector3i(10, 10, 10)
)

# Only regenerate those chunks
voxel_gen.regenerate_dirty_chunks()
```

### Distance LOD for Streaming

```gdscript
voxel_gen.enable_distance_lod = true
voxel_gen.lod_distance_multiplier = 2.5

# Update every frame as player moves
func _process(_delta: float) -> void:
	voxel_gen.lod_reference_position = player.global_position
	if voxel_gen.update_chunks_lod() > 0:
		voxel_gen.regenerate_dirty_chunks()
```

---

## Advanced Topics

### Custom Voxel Types

Define custom materials and colors:

```gdscript
# Register custom voxel types
var voxel_registry = VoxelRegistry.new()
voxel_registry.register_voxel(VoxelType.GRASS, "Grass", Color.GREEN)
voxel_registry.register_voxel(VoxelType.STONE, "Stone", Color.GRAY)
voxel_registry.register_voxel(VoxelType.WATER, "Water", Color.CYAN)
```

### Sampling Terrain Density

Query density at any world position:

```gdscript
var world_pos = Vector3(5.0, 10.0, 5.0)

# Get density (negative = solid, positive = air)
var density = voxel_gen.sample_density_at(world_pos)
if density < 0:
	print("Solid terrain")
else:
	print("Air")

# Get voxel type at integer coordinates
var voxel_type = voxel_gen.get_voxel_at(Vector3i(5, 10, 5))
```

### Signals and Events

```gdscript
# Async generation progress
voxel_gen.chunk_ready.connect(func(chunk_index, chunk_coord):
	print("Chunk %s ready" % chunk_coord)
)

voxel_gen.generation_progress.connect(func(completed, total):
	update_progress_bar(float(completed) / total)
)

voxel_gen.generation_complete.connect(func():
	print("All chunks generated!")
	start_gameplay()
)

# Forcefield events (world boundary walls)
voxel_gen.forcefield_body_entered.connect(func(wall_index, body):
	print("Body entered forcefield wall %d" % wall_index)
)
```

### Forcefield (World Boundary)

Create invisible walls at world edges:

```gdscript
voxel_gen.forcefield_enabled = true
voxel_gen.forcefield_height = 300.0
voxel_gen.forcefield_collision_enabled = true
voxel_gen.forcefield_detection_enabled = true

# Per-wall control
voxel_gen.set_forcefield_wall_enabled(VoxelGenerator.FF_NORTH, true)
voxel_gen.set_forcefield_wall_enabled(VoxelGenerator.FF_SOUTH, true)
voxel_gen.set_forcefield_wall_enabled(VoxelGenerator.FF_TOP, false)
```

### Material and Texturing

Apply custom shader materials:

```gdscript
var material = StandardMaterial3D.new()
material.albedo_color = Color.WHITE
material.texture_albedo = load("res://textures/terrain.png")

voxel_gen.set_terrain_material(material)
```

---

## Troubleshooting

### Terrain Not Appearing

1. Check `world_size` is not (0, 0, 0)
2. Verify `cutoff` threshold (default 0.0)
3. Ensure noise generators are set:
   ```gdscript
   if not voxel_gen.get_terrain_noise():
       voxel_gen.set_terrain_noise(NoiseGenerator.new())
   ```
4. Check chunk meshes exist: `voxel_gen.chunks.size()`

### Low Frame Rate

1. Reduce `resolution` (fewer triangles)
2. Use `generation_mode = HEIGHTMAP_FIRST` (processes fewer voxels)
3. Enable `vertex_limit` to cap mesh size
4. Use distance LOD to reduce detail at distance
5. Increase `max_chunks_per_frame` for faster async processing

### Artifacts or Missing Geometry

1. Increase `resolution` for more detail
2. Widen `surface_band` in HEIGHTMAP_FIRST mode
3. Check `cutoff` value (should be near terrain surface)
4. Verify noise generators have appropriate ranges

### Terrain Edits Don't Persist

1. Call `save_map()` before closing
2. Ensure `user://` directory is writable
3. Check `get_terrain_edit_count()` returns > 0
4. Verify edits were applied: `regenerate_dirty_chunks()`

---

## Examples

See the `demo/` folder for complete working examples:

- **main.gd**: Scene setup and terrain initialization
- **gui.gd**: Debug UI for real-time parameter tweaking
- **terrain_plains.gd**: Plains biome configuration
- **terrain_mountains.gd**: Mountain biome configuration
- **picele.gd**: Player controller with terraforming

---

## Next Steps

1. **Read** [GENERATION_GUIDE.md](GENERATION_GUIDE.md) for sync vs async comparison
2. **Learn** [Terraforming.md](doc_classes/Terraforming.md) for runtime terrain editing
3. **Explore** [SAVE_LOAD_MAP.md](docs/SAVE_LOAD_MAP.md) for persistence
4. **Run** the demo project to see everything in action

---

## Support

For issues or questions:

- Open an issue on [GitHub](https://github.com/KeenbeanofArk/godot-cpp-gdextention)
- Check existing documentation and examples
- Enable `debug_mode = true` and increase `debug_verbosity` for detailed logs

---

**Happy voxel engineering! 🎮**
