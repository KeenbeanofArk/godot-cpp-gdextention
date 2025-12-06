# Voxel Engine Generators

This document describes the generator system in the Keen Voxel Engine, including:

- **VoxelGenerator** - Main terrain generation with per-chunk mesh rendering
- **NoiseGenerator** - Procedural noise for terrain generation
- **BiomeGenerator** - Biome-based terrain with blending
- **FeatureGenerator** - Decoration and structure placement

---

## VoxelGenerator

The [`VoxelGenerator`](../VoxelGenerator.h) is the main class that orchestrates terrain generation. It uses a **per-chunk mesh system** where each chunk has its own `MeshInstance3D` for efficient rendering and incremental updates.

### Generation Modes

| Mode              | Value | Description                                                                  |
| ----------------- | ----- | ---------------------------------------------------------------------------- |
| `VOXELS_FIRST`    | 0     | Full 3D density evaluation - evaluates every voxel in the volume             |
| `HEIGHTMAP_FIRST` | 1     | **Optimized** - Uses 2D heightmap, only evaluates voxels within surface band |

### Properties

| Property            | Type     | Range    | Default | Description                                             |
| ------------------- | -------- | -------- | ------- | ------------------------------------------------------- |
| `world_size`        | Vector3i | 1-100    | (1,1,1) | Number of chunks in each dimension                      |
| `chunk_size`        | int      | 8-64     | 8       | Size of each chunk in voxels                            |
| `resolution`        | int      | 1-10     | 1       | Marching cubes samples per voxel (higher = more detail) |
| `generation_mode`   | int      | 0-1      | 0       | VOXELS_FIRST or HEIGHTMAP_FIRST                         |
| `surface_band`      | float    | 1.0-20.0 | 4.0     | Vertical band around terrain surface (heightmap mode)   |
| `lod_level`         | int      | 0-7      | 0       | Level of detail (0 = highest, 7 = lowest)               |
| `cutoff`            | float    | -1.0-1.0 | 0.0     | Density threshold for solid/air boundary                |
| `terrain_height`    | float    | -100-100 | 4.0     | Base terrain height in world units                      |
| `terrain_amplitude` | float    | 0-100    | 8.0     | Height variation range                                  |
| `rock_influence`    | float    | 0-1      | 0.3     | Strength of 3D rocky detail noise                       |
| `auto_generate`     | bool     | -        | true    | Auto-regenerate when properties change                  |
| `vertex_limit`      | bool     | -        | false   | Stop generation when vertex limit reached               |

### Terrain Noise Properties

| Property        | Type           | Description                      |
| --------------- | -------------- | -------------------------------- |
| `terrain_noise` | NoiseGenerator | 2D noise for base terrain height |
| `detail_noise`  | NoiseGenerator | 3D noise for rocky detail        |

### Debug Visualization

| Property          | Type | Description                           |
| ----------------- | ---- | ------------------------------------- |
| `show_voxel_grid` | bool | Show wireframe cubes for solid voxels |
| `show_chunk_grid` | bool | Show red lines at chunk boundaries    |
| `show_centers`    | bool | Show debug points at voxel centers    |
| `debug_mode`      | bool | Enable debug logging                  |
| `debug_verbosity` | int  | Log detail level (0-3)                |

### Per-Chunk Mesh System

The VoxelGenerator creates a `Chunk` node for each chunk in the world. Each chunk manages its own mesh:

```
VoxelGenerator (Node3D)
├── Chunk_0_0_0 (Chunk)
│   └── ChunkMesh (MeshInstance3D)
├── Chunk_1_0_0 (Chunk)
│   └── ChunkMesh (MeshInstance3D)
├── Chunk_0_1_0 (Chunk)
│   └── ChunkMesh (MeshInstance3D)
...
├── MeshInstanceVoxelGrid (debug visualization)
├── MeshInstanceChunkGrid (debug visualization)
└── MeshInstanceCenters (debug visualization)
```

### Incremental Updates

The per-chunk system supports incremental updates - only dirty chunks need regeneration:

```gdscript
# Mark a specific chunk as needing regeneration
voxel_gen.mark_chunk_dirty(Vector3i(2, 0, 3))

# Mark all chunks as dirty
voxel_gen.mark_all_chunks_dirty()

# Regenerate only the dirty chunks
voxel_gen.regenerate_dirty_chunks()

# Invalidate density cache for a region (e.g., after terrain edit)
voxel_gen.invalidate_density_region(Vector3i(10, 0, 10), Vector3i(20, 5, 20))
```

### Async Generation

For large worlds, use async generation to avoid blocking the main thread:

```gdscript
# Start async generation
voxel_gen.generate_async()

# Cancel if needed
voxel_gen.cancel_generation()

# Check status
if voxel_gen.is_generating():
    print("Still generating...")

# Connect to signals
voxel_gen.chunk_ready.connect(_on_chunk_ready)
voxel_gen.generation_progress.connect(_on_progress)
voxel_gen.generation_complete.connect(_on_complete)

func _on_chunk_ready(chunk_index: int, chunk_coord: Vector3i):
    print("Chunk %s ready" % chunk_coord)

func _on_progress(completed: int, total: int):
    print("Progress: %d/%d" % [completed, total])
```

### LOD System

The LOD level affects effective resolution and surface band:

| LOD Level | Effective Resolution | Surface Band Multiplier |
| --------- | -------------------- | ----------------------- |
| 0         | resolution           | 1.0x                    |
| 1         | resolution / 2       | 1.5x                    |
| 2         | resolution / 4       | 2.0x                    |
| 3         | resolution / 8       | 2.5x                    |
| ...       | ...                  | ...                     |

### GDScript Example

```gdscript
var voxel_gen = $VoxelGenerator

# Configure world
voxel_gen.world_size = Vector3i(4, 2, 4)
voxel_gen.chunk_size = 16
voxel_gen.resolution = 2

# Use optimized heightmap mode
voxel_gen.generation_mode = 1  # HEIGHTMAP_FIRST
voxel_gen.surface_band = 6.0
voxel_gen.lod_level = 0

# Configure terrain
voxel_gen.terrain_height = 0.0
voxel_gen.terrain_amplitude = 15.0
voxel_gen.rock_influence = 0.3
voxel_gen.cutoff = 0.0

# Generate
voxel_gen.generate()
```

---

## NoiseGenerator

The [`NoiseGenerator`](NoiseGenerator.h) class wraps Godot's `FastNoiseLite` to provide configurable procedural noise for terrain height and density calculations.

### Properties

| Property      | Type  | Range    | Default       | Description                                 |
| ------------- | ----- | -------- | ------------- | ------------------------------------------- |
| `seed`        | int   | -        | 12345         | Random seed for reproducible results        |
| `noise`       | Noise | -        | FastNoiseLite | The underlying noise resource               |
| `octaves`     | int   | 1-8      | 4             | Number of noise layers (detail levels)      |
| `period`      | float | 0.1-1000 | 50.0          | Scale of noise features (larger = smoother) |
| `persistence` | float | 0.0-1.0  | 0.5           | How much each octave contributes            |
| `lacunarity`  | float | 1.0-4.0  | 2.0           | Frequency multiplier between octaves        |

### Methods

```cpp
// Get 3D noise value at position (returns -1.0 to 1.0)
float get_noise_3d(float x, float y, float z) const;

// Get 2D noise value at position (returns -1.0 to 1.0)
float get_noise_2d(float x, float z) const;
```

### GDScript Example

```gdscript
# Create and configure a noise generator
var noise_gen = NoiseGenerator.new()
noise_gen.seed = 42
noise_gen.period = 100.0      # Large features
noise_gen.octaves = 4         # Good detail
noise_gen.persistence = 0.5   # Balanced octave contribution
noise_gen.lacunarity = 2.0    # Standard frequency doubling

# Sample noise values
var height = noise_gen.get_noise_2d(x, z)  # For heightmaps
var density = noise_gen.get_noise_3d(x, y, z)  # For 3D caves
```

### C++ Example

```cpp
Ref<NoiseGenerator> terrain_noise;
terrain_noise.instantiate();
terrain_noise->set_period(50.0f);
terrain_noise->set_octaves(4);
terrain_noise->set_persistence(0.5f);
terrain_noise->set_lacunarity(2.0f);
terrain_noise->set_seed(1240);

float value = terrain_noise->get_noise_2d(x, z);
```

---

## BiomeGenerator

The [`BiomeGenerator`](BiomeGenerator.h) class handles biome-based terrain generation with support for multiple biomes, smooth blending at boundaries, and layered block placement.

### Properties

| Property            | Type           | Default      | Description                                     |
| ------------------- | -------------- | ------------ | ----------------------------------------------- |
| `height_noise`      | NoiseGenerator | auto-created | Controls terrain elevation                      |
| `temperature_noise` | NoiseGenerator | auto-created | Controls temperature distribution               |
| `humidity_noise`    | NoiseGenerator | auto-created | Controls humidity distribution                  |
| `seed`              | int            | 12345        | Master seed for all noise generators            |
| `sea_level`         | float          | 0.0          | Y-level for water generation                    |
| `default_voxel`     | int            | 0 (AIR)      | Voxel type for empty spaces                     |
| `blend_distance`    | int            | 16           | Distance in blocks for biome transitions (8-32) |

### Biome Data Structure

Each biome is defined with the following parameters:

```gdscript
add_biome_extended(
    name,              # String: Biome identifier
    min_height,        # float: Minimum terrain height (0.0-1.0 normalized)
    max_height,        # float: Maximum terrain height
    min_temperature,   # float: Minimum temperature (0.0-1.0)
    max_temperature,   # float: Maximum temperature
    min_humidity,      # float: Minimum humidity (0.0-1.0)
    max_humidity,      # float: Maximum humidity
    surface_blocks,    # Array[int]: Block types for surface layer
    subsurface_blocks, # Array[int]: Block types below surface
    depth,             # int: Depth of surface/subsurface layers
    bedrock_block,     # int: Block type at world bottom
    filler_block       # int: Block type between subsurface and bedrock
)
```

### Methods

```cpp
// Add a biome with default bedrock/filler (STONE)
void add_biome(name, min_height, max_height, min_temp, max_temp,
               min_humid, max_humid, surface_blocks, subsurface_blocks, depth);

// Add a biome with custom layer blocks
void add_biome_extended(name, min_height, max_height, min_temp, max_temp,
                        min_humid, max_humid, surface_blocks, subsurface_blocks,
                        depth, bedrock_block, filler_block);

// Query methods
float get_height_at(float x, float z) const;
int get_biome_index_at(float x, float z) const;
Ref<Voxel> get_voxel_at(int x, int y, int z) const;
float get_temperature_at(float x, float z) const;
float get_humidity_at(float x, float z) const;

// Blending methods
Array get_blended_biome_weights(float x, float z) const;
float get_blended_height_at(float x, float z) const;

// Management
void clear_biomes();
int get_biome_count() const;
Dictionary get_biome_data(int index) const;
```

### GDScript Example

```gdscript
var biome_gen = BiomeGenerator.new()
biome_gen.seed = 42
biome_gen.sea_level = 10.0
biome_gen.blend_distance = 16

# Configure noise generators (optional - auto-created if not set)
biome_gen.height_noise = NoiseGenerator.new()
biome_gen.height_noise.period = 100.0

# Add a plains biome
var grass_blocks: Array[int] = [Voxel.GRASS]
var dirt_blocks: Array[int] = [Voxel.DIRT]
biome_gen.add_biome_extended(
    "Plains",
    0.0, 0.4,        # height range
    0.3, 0.7,        # temperature range
    0.3, 0.7,        # humidity range
    grass_blocks,    # surface
    dirt_blocks,     # subsurface
    3,               # depth
    Voxel.STONE,     # bedrock
    Voxel.STONE      # filler
)

# Add a desert biome
var sand_blocks: Array[int] = [Voxel.SAND]
biome_gen.add_biome_extended(
    "Desert",
    0.0, 0.3,
    0.7, 1.0,        # hot
    0.0, 0.3,        # dry
    sand_blocks,
    sand_blocks,
    5,
    Voxel.STONE,
    Voxel.SAND
)

# Query terrain
var height = biome_gen.get_height_at(x, z)
var biome_idx = biome_gen.get_biome_index_at(x, z)
var voxel = biome_gen.get_voxel_at(x, y, z)
```

### Layer System

The BiomeGenerator uses a layered approach for voxel placement:

1. **Air/Water** - Above terrain height (water if below sea_level)
2. **Surface Layer** - Top `depth` blocks use `surface_blocks`
3. **Subsurface Layer** - Next `depth * 4` blocks use `subsurface_blocks`
4. **Filler Layer** - Below subsurface uses `filler_block`
5. **Bedrock Layer** - Bottom 5 blocks use `bedrock_block`

### Biome Blending

Biomes blend smoothly at boundaries using weighted sampling:

```gdscript
# Get blend weights at a position
var weights = biome_gen.get_blended_biome_weights(x, z)
for w in weights:
    print("Biome %d: %.2f%%" % [w["biome_index"], w["weight"] * 100])
```

---

## FeatureGenerator

The [`FeatureGenerator`](FeatureGenerator.h) class handles placement of decorations and structures like trees, rocks, and ore deposits.

### Integration with VoxelGenerator

The feature generator integrates with the main [`VoxelGenerator`](../VoxelGenerator.h) class:

```gdscript
var voxel_gen = $VoxelGenerator
voxel_gen.feature_generator = FeatureGenerator.new()
voxel_gen.biome_generator = BiomeGenerator.new()
```

### Methods (VoxelGenerator)

```cpp
// Apply features to a generated chunk
void apply_features_to_chunk(Chunk* chunk);

// Get surface heights for feature placement
TypedArray<int> get_chunk_surface_heights(Chunk* chunk) const;

// Get biome indices for biome-specific features
TypedArray<int> get_chunk_biome_indices(Chunk* chunk) const;
```

---

## Complete Setup Example

Here's a full example showing how to configure all generators together:

### GDScript (demo/main.gd style)

```gdscript
extends Node3D

@onready var voxel_generator: VoxelGenerator = $Terrain/VoxelGenerator

func _ready():
    # Configure main generator
    voxel_generator.world_size = Vector3i(10, 3, 10)
    voxel_generator.chunk_size = 8
    voxel_generator.resolution = 4
    voxel_generator.generation_mode = 1  # HEIGHTMAP_FIRST (optimized)
    voxel_generator.surface_band = 4.0
    voxel_generator.lod_level = 0
    voxel_generator.auto_generate = false

    # Configure terrain
    voxel_generator.terrain_height = 2.0
    voxel_generator.terrain_amplitude = 20.5
    voxel_generator.rock_influence = 0.42
    voxel_generator.seeder = 1240

    # Configure detail noise
    voxel_generator.detail_noise.seed = 1241
    voxel_generator.detail_noise.octaves = 3
    voxel_generator.detail_noise.period = 10.0
    voxel_generator.detail_noise.persistence = 0.6
    voxel_generator.detail_noise.lacunarity = 2.5

    # Setup biome generator
    var biome_gen = BiomeGenerator.new()
    biome_gen.seed = 1240
    biome_gen.sea_level = 5.0
    setup_biomes(biome_gen)
    voxel_generator.biome_generator = biome_gen

    # Enable debug visualization
    voxel_generator.show_voxel_grid = false
    voxel_generator.show_chunk_grid = false

    # Generate terrain
    voxel_generator.generate()

func setup_biomes(biome_gen: BiomeGenerator):
    # Plains
    var grass: Array[int] = [Voxel.GRASS]
    var dirt: Array[int] = [Voxel.DIRT]
    biome_gen.add_biome_extended("Plains", 0.0, 0.5, 0.3, 0.7, 0.3, 0.7,
                                  grass, dirt, 3, Voxel.STONE, Voxel.STONE)

    # Mountains
    var stone: Array[int] = [Voxel.STONE]
    biome_gen.add_biome_extended("Mountains", 0.5, 1.0, 0.0, 0.5, 0.2, 0.8,
                                  stone, stone, 2, Voxel.STONE, Voxel.STONE)

    # Desert
    var sand: Array[int] = [Voxel.SAND]
    biome_gen.add_biome_extended("Desert", 0.0, 0.3, 0.7, 1.0, 0.0, 0.3,
                                  sand, sand, 5, Voxel.STONE, Voxel.SAND)
```

---

## Voxel Types

Available voxel types (defined in [`voxel.h`](../core/voxel.h)):

| Type      | Value | Description        |
| --------- | ----- | ------------------ |
| `AIR`     | 0     | Empty space        |
| `DIRT`    | 1     | Basic soil         |
| `GRASS`   | 2     | Grass-covered soil |
| `STONE`   | 3     | Rock               |
| `WATER`   | 4     | Water              |
| `SAND`    | 5     | Sand               |
| `LAVA`    | 6     | Lava               |
| `GOLD`    | 7     | Gold ore           |
| `DIAMOND` | 8     | Diamond            |
| `IRON`    | 9     | Iron ore           |
| `COAL`    | 10    | Coal               |

---

## Performance Tips

1. **Use HEIGHTMAP_FIRST mode** for terrain-focused worlds. It skips voxels outside the surface band, dramatically reducing computation. Look for log messages like "X% voxels skipped due to surface band".

2. **Adjust surface_band** based on terrain complexity:

   - Flat terrain: 2-4
   - Rolling hills: 4-8
   - Mountains with caves: 8-16

3. **Use LOD levels** for distant terrain. Higher LOD = lower resolution = faster generation.

4. **Per-chunk updates**: Use `mark_chunk_dirty()` and `regenerate_dirty_chunks()` instead of full regeneration when editing terrain.

5. **Async generation**: For large worlds (>100 chunks), use `generate_async()` to prevent frame drops.

6. **Seed Consistency**: Use related seeds for coherent generation:

   - Terrain noise: `seed`
   - Detail noise: `seed + 1`
   - Temperature: `seed + 1000`
   - Humidity: `seed + 2000`

7. **Biome Blending**: Higher `blend_distance` values (16-32) create smoother transitions but may reduce biome variety in small areas.

---

## Debug Output

With `debug_mode = true` and appropriate `debug_verbosity`, you'll see:

```
[VoxelGenerator] Generation mode: Heightmap First, LOD level: 0, Effective resolution: 4
[VoxelGenerator] Heightmap mode: surface_band=4.0, effective_band=4.0
[VoxelGenerator] Generated 24 chunk meshes
[VoxelGenerator] Voxel grid (HEIGHTMAP): 12480 vertices, 67% voxels skipped due to surface band
[VoxelGenerator] HEIGHTMAP_FIRST completed: 8432 triangles, 25296 vertices, 67% voxels skipped
```
