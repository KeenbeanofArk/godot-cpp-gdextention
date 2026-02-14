# Getting Started with VoxelEngine

This guide walks you through the basics of setting up and using the Keen Voxel Engine for terrain generation in Godot.

## Installation & Setup

### Prerequisites

- Godot 4.2+ with GDScript support
- Keen Voxel Engine extension (compiled C++ module)
- Basic Godot scene knowledge

### Basic Scene Setup

```gdscript
extends Node3D

func _ready():
	# Method 1: Using the VoxelEngine facade (recommended for most cases)
	var engine = VoxelEngine.new()
	add_child(engine)

	var generator = engine.create_generator()
	if generator == null:
		print("Failed to create generator")
		return

	# Configure basic world
	generator.set_world_size(Vector3i(8, 8, 8))  # 8×8×8 chunks
	generator.set_chunk_size(8)  # 8 voxels per chunk edge
	generator.set_resolution(2)   # Higher = smoother but slower

	# Start generation
	generator.generate()
	print("Terrain generated successfully!")
```

### Method 2: Direct VoxelGenerator Instantiation

For more control, you can instantiate VoxelGenerator directly:

```gdscript
extends Node3D

func _ready():
	var generator = VoxelGenerator.new()
	add_child(generator)

	# Configure and generate
	generator.set_world_size(Vector3i(8, 8, 8))
	generator.set_chunk_size(8)
	generator.generate()
```

## Key Configuration Parameters

### World Structure

- **world_size** (Vector3i): Number of chunks in each dimension
  - Example: `Vector3i(16, 16, 16)` = 16×16×16 chunks total
  - Each chunk is `chunk_size³` voxels

- **chunk_size** (int): Voxels per chunk edge
  - Common values: 4, 8, 16, 32, 64
  - Default: 8
  - Larger chunks = fewer total chunks but slower individual generation

### Terrain Appearance

- **terrain_height** (float): Base height of terrain center
  - Default: 4.0
  - Example: Set to 20.0 for higher terrain

- **terrain_amplitude** (float): Height variation range
  - Default: 8.0
  - Range: terrain_height ± amplitude
  - Higher = more dramatic mountains/valleys

- **resolution** (int): Sampling resolution (samples per voxel edge)
  - Default: 1 (standard marching cubes)
  - Higher = smoother surface but 2-4× slower
  - Typical range: 1-3

- **cutoff** (float): Marching cubes density threshold
  -Default: 0.0
  - Negative = more solid terrain
  - Positive = more air/caves

### Detail & Roughness

- **rock_influence** (float): Strength of detail noise (0.0-1.0)
  - Default: 0.3
  - Controls how rocky/textured terrain looks
  - Set to 0.0 for smooth terrain, 1.0 for very rocky

## Adding Noise to Terrain

The terrain shape comes from noise generators. You must configure noise before generation:

```gdscript
func _ready():
	var generator = VoxelGenerator.new()
	add_child(generator)

	# Create and configure terrain noise (base height)
	var terrain_noise = NoiseGenerator.new()
	terrain_noise.set_period(64.0)      # Wavelength of mountains
	terrain_noise.set_octaves(6)         # Number of detail layers
	terrain_noise.set_persistence(0.5)  # How much each octave contributes
	terrain_noise.set_lacunarity(2.0)   # Frequency multiplier per octave
	generator.set_terrain_noise(terrain_noise)

	# Optional: add detail rocky noise
	var detail_noise = NoiseGenerator.new()
	detail_noise.set_period(16.0)
	detail_noise.set_octaves(4)
	generator.set_detail_noise(detail_noise)

	# Configure generation
	generator.set_terrain_height(10.0)
	generator.set_terrain_amplitude(15.0)
	generator.set_rock_influence(0.3)

	# Now generate
	generator.generate()
```

## Asynchronous Generation (Non-Blocking)

For gameplay, use async generation to avoid freezing:

```gdscript
func _ready():
	var generator = VoxelGenerator.new()
	add_child(generator)

	# Connect signals for progress feedback
	generator.generation_started.connect(_on_generation_started)
	generator.generation_progress.connect(_on_generation_progress)
	generator.generation_completed.connect(_on_generation_completed)
	generator.generation_cancelled.connect(_on_generation_cancelled)

	# Configure
	generator.set_world_size(Vector3i(16, 16, 16))

	# ... setup noise, biomes, etc ...

	# Start async generation (returns immediately)
	generator.generate_async()

	print("Generation started (non-blocking)")

func _on_generation_started(total_chunks: int):
	print("Generating %d chunks..." % total_chunks)
	get_tree().root.get_node("UI").show_progress_bar()

func _on_generation_progress(completed: int, total: int):
	var percent = (completed / float(total)) * 100.0
	print("Progress: %.1f%%" % percent)
	get_tree().root.get_node("UI").update_progress_bar(percent)

func _on_generation_completed():
	print("Terrain generation complete!")
	get_tree().root.get_node("UI").hide_progress_bar()

func _on_generation_cancelled():
	print("Generation was cancelled")
```

### Tuning Async Performance

Control how many chunks are applied per frame to avoid stuttering:

```gdscript
# More conservative (very smooth, slower)
generator.set_max_chunks_per_frame(2)
generator.set_signal_every_n_chunks(4)

# More aggressive (faster, potential stuttering)
generator.set_max_chunks_per_frame(8)
generator.set_signal_every_n_chunks(16)
```

## Using Terrain Configs (Presets)

Pre-configure entire terrains with TerrainConfig resources:

```gdscript
func _ready():
	# Load a saved configuration
	var config = ResourceLoader.load("res://configs/mountains.tres")

	var generator = VoxelGenerator.new()
	add_child(generator)

	# Apply all settings from config in one call
	if config.apply_to_voxel_generator(generator):
		generator.generate_async()

# Or create a config in code and save it
func save_custom_config():
	var config = TerrainConfig.new()
	config.set_config_name("My Terrain")
	config.set_world_size(Vector3i(20, 20, 20))
	config.set_chunk_size(16)
	config.set_terrain_height(15.0)
	config.set_terrain_amplitude(20.0)

	# Create noise
	var noise = NoiseGenerator.new()
	noise.set_period(100.0)
	noise.set_octaves(8)
	config.set_noise_generator(noise)

	# Save for reuse
	ResourceSaver.save(config, "res://configs/my_terrain.tres")
	print("Config saved!")
```

## Adding Biomes

Biomes determine which voxel types (sand, stone, grass, etc.) appear where:

```gdscript
func _ready():
	var generator = VoxelGenerator.new()
	add_child(generator)

	# Create biome generator
	var biome_gen = BiomeGenerator.new()

	# Add biomes by height range
	# Parameters: name, min_height, max_height, temperature, humidity, surface_blocks, subsurface_blocks
	biome_gen.add_biome("grassland", 5.0, 15.0, 0.5, 0.5, [1], [2])  # 1=grass, 2=dirt
	biome_gen.add_biome("mountains", 15.0, 30.0, 0.3, 0.3, [3], [3])  # 3=stone
	biome_gen.add_biome("desert", 2.0, 8.0, 0.8, 0.1, [4], [4])       # 4=sand

	generator.set_biome_generator(biome_gen)

	# Setup everything else...
	generator.generate_async()
```

## Adding Features (Trees, Ores, etc.)

Place decorative features after terrain generation:

```gdscript
func _ready():
	var generator = VoxelGenerator.new()
	add_child(generator)

	# Create feature generator
	var feature_gen = FeatureGenerator.new()

	# Add feature placement rules
	# Parameters: feature_type, probability, biome_filters, min_height, max_height, voxel_type, cluster_size
	feature_gen.add_tree_rule(0.1, [BIOME_GRASSLAND], 5, 20)         # 10% trees in grassland
	feature_gen.add_ore_rule(VOXEL_IRON, 0.3, 0, 50)                 # Iron ore 30% chance
	feature_gen.add_rock_rule(0.2, [BIOME_MOUNTAINS], 15, 30)        # Rocks in mountains

	generator.set_feature_generator(feature_gen)

	# ... rest of setup ...
	generator.generate_async()
```

## Enabling World Boundaries (Forcefields)

Contain players within the world with invisible/visible walls:

```gdscript
func _ready():
	var generator = VoxelGenerator.new()
	add_child(generator)

	# ... configure and generate ...
	generator.generate()

	# Add forcefields
	generator.set_forcefield_enabled(true)
	generator.set_forcefield_height(300.0)  # Tall walls
	generator.set_forcefield_collision_enabled(true)  # Solid walls
	generator.create_forcefield_nodes()  # Build the wall geometry

	print("Forcefields created")
```

## Debugging & Visualization

### Enable Debug Output

```gdscript
generator.set_debug_mode(true)
generator.set_debug_verbosity(2)  # 0=silent, 1=errors, 2=info, 3=verbose
```

### Visualize Chunks

```gdscript
# Show chunk boundaries
generator.set_show_chunk_grid(true)

# Show voxel grid (slower)
generator.set_show_voxel_grid(true)

# Show chunk centers
generator.set_show_centers(true)
```

### LOD Color Visualization

```gdscript
# When using distance LOD, visualize LOD levels with colors
generator.set_enable_distance_lod(true)
generator.set_show_lod_colors(true)  # Green = LOD 0 (detail), Red = LOD 7 (coarse)
```

### Highlight Specific Voxel

```gdscript
func _process(delta):
	var raycast_result = my_raycast.get_collider()
	if raycast_result:
		var hit_pos = my_raycast.get_collision_point()
		generator.set_voxel_highlight_enabled(true)
		generator.set_highlight_position(hit_pos)
		generator.set_highlight_color(Color.GREEN)
```

## Common Patterns

### Simple Flat Terrain

```gdscript
func create_flatlands(generator: VoxelGenerator):
	generator.set_terrain_height(5.0)
	generator.set_terrain_amplitude(1.0)  # Very small variation
	generator.set_rock_influence(0.1)     # Minimal detail

	var noise = NoiseGenerator.new()
	noise.set_period(200.0)               # Large features
	noise.set_octaves(2)                  # Few detail layers
	generator.set_terrain_noise(noise)
```

### Mountainous Terrain

```gdscript
func create_mountains(generator: VoxelGenerator):
	generator.set_terrain_height(20.0)
	generator.set_terrain_amplitude(30.0)  # Large variation
	generator.set_rock_influence(0.5)      # Rocky

	var noise = NoiseGenerator.new()
	noise.set_period(100.0)
	noise.set_octaves(8)                   # Lots of detail
	noise.set_persistence(0.7)
	generator.set_terrain_noise(noise)
```

### Underground Caves

```gdscript
func create_cave_world(generator: VoxelGenerator):
	generator.set_generation_mode(0)  # VOXELS_FIRST for full 3D
	generator.set_terrain_amplitude(50.0)

	var noise = NoiseGenerator.new()
	noise.set_period(50.0)
	noise.set_octaves(6)
	generator.set_terrain_noise(noise)

	var detail = NoiseGenerator.new()
	detail.set_period(20.0)
	detail.set_octaves(5)
	generator.set_detail_noise(detail)
	generator.set_rock_influence(0.7)  # Very cave-heavy
```

## Troubleshooting

### Terrain Won't Generate

- **Check**: Is noise_generator set? `if generator.get_terrain_noise() == null: print("No noise!")`
- **Check**: Is world_size valid? Must be > 0 in each dimension
- **Check**: Are you calling `generate()` or `generate_async()`?

### Generation is Slow

- **Reduce** `world_size` or `chunk_size`
- **Reduce** `resolution` (1 is fastest)
- **Reduce** `lod_level` or `rock_influence`
- **Switch** to `HEIGHTMAP_FIRST` mode for faster generation

### Terrain Looks Blocky

- **Increase** `resolution` (2 or 3 for smoother)
- **Adjust** `cutoff` for better surface extraction
- **Use** `HEIGHTMAP_FIRST` mode (10-20% faster)

### Can't See Terrain

- **Check**: Is the generator a child of the scene? `add_child(generator)`
- **Check**: Is camera positioned above terrain? (Y > terrain_height - terrain_amplitude)
- **Check**: Are there any material/visibility issues? Try `generator.set_show_chunk_grid(true)`

### Async Generation Hangs

- **Check**: Are you calling `generate_async()` or `generate()`?
- **Check**: Is `_process()` being called? (Scene must be running)
- **Try**: Reducing `max_chunks_per_frame` to allow processing

## Next Steps

- Read [Terrain Generation](TerrainGeneration.md) for generation modes and LOD system
- Read [Biome System](BiomeSystem.md) for advanced biome configuration
- Read [Async Generation](AsyncGeneration.md) for streaming large worlds
- Read [Terraforming](Terraforming.md) for runtime terrain editing
- Read [Forcefields](Forcefields.md) for world boundary setup
