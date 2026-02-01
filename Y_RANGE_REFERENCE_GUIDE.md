# P3 Y-Range Biome Layer System - Complete Reference Guide

## Overview

The Y-Range Biome Layer System is a sophisticated subsurface layer management system that allows you to define terrain stratification at specific Y-coordinates. This enables realistic geology with distinct layers of different block types at different depths.

**Status**: ✅ Fully Implemented (P0-P3)
**Compiled**: ✅ Clean build
**Deployed**: ✅ DLL in demo/bin/windows/
**Documentation**: ✅ Complete with examples

---

## Core Architecture

### Data Structure: SubsurfaceLayer

```cpp
struct SubsurfaceLayer {
    int block_type;      // VoxelType to place in this layer
    int y_min;          // Minimum Y coordinate (inclusive)
    int y_max;          // Maximum Y coordinate (inclusive)
    float density;      // Placement probability (0.0-1.0)
                        // 1.0 = always solid, 0.5 = 50% placement
};
```

### Layer Matching Algorithm

When querying a voxel at position (x, y, z):

1. **Find matching layers**: Check all subsurface layers defined for the biome
2. **Y-range check**: Find layers where `y_min ≤ y ≤ y_max`
3. **Priority resolution**: Use FIRST matching layer (earliest in definition order)
4. **Density application**: Apply density probabilistically with deterministic seeding
5. **Fallback**: If no layer matches, use bedrock block type

**Example**:

```
Layer 1: STONE, y_min=50, y_max=100   (first to match at Y=75)
Layer 2: DIRT, y_min=40, y_max=60     (not used - Layer 1 matched first)
Layer 3: SAND, y_min=0, y_max=40      (used if Y < 50)
```

---

## API Reference

### 1. Add Biome with Y-Range Layers

**Method**: `add_biome_with_y_ranges()`

**C++ Signature**:

```cpp
void add_biome_with_y_ranges(
    const String &name,
    float min_height, float max_height,
    float min_temperature, float max_temperature,
    float min_humidity, float max_humidity,
    const TypedArray<int32_t> &surface_blocks,
    const Array &subsurface_layers_array,
    int bedrock_block,
    int filler_block
);
```

**GDScript Usage**:

```gdscript
var biome_gen = BiomeGenerator.new()

# Define subsurface layers
var layers = [
    {
        "block_type": 3,      # STONE
        "y_min": 50,
        "y_max": 100,
        "density": 1.0        # Always solid
    },
    {
        "block_type": 5,      # SAND
        "y_min": 20,
        "y_max": 50,
        "density": 0.8        # 80% placement
    },
    {
        "block_type": 3,      # STONE (bedrock)
        "y_min": 0,
        "y_max": 20,
        "density": 1.0
    }
]

# Create biome
biome_gen.add_biome_with_y_ranges(
    "Mountain",                           # Name
    -10.0, 50.0,                         # Height range
    -0.5, 0.5,                           # Temperature (cold)
    -1.0, 0.0,                           # Humidity (dry)
    PackedInt32Array([3]),               # Surface blocks (STONE)
    layers,                              # Subsurface layers (Y-range defined)
    3,                                   # Bedrock block (STONE)
    3                                    # Filler block (STONE)
)
```

**Parameters**:

| Parameter                 | Type              | Description                             |
| ------------------------- | ----------------- | --------------------------------------- |
| `name`                    | String            | Unique biome identifier                 |
| `min_height`              | float             | Minimum terrain height for biome        |
| `max_height`              | float             | Maximum terrain height for biome        |
| `min_temperature`         | float             | Minimum temperature (-1.0 to 1.0)       |
| `max_temperature`         | float             | Maximum temperature (-1.0 to 1.0)       |
| `min_humidity`            | float             | Minimum humidity (-1.0 to 1.0)          |
| `max_humidity`            | float             | Maximum humidity (-1.0 to 1.0)          |
| `surface_blocks`          | PackedInt32Array  | Block types for surface layer           |
| `subsurface_layers_array` | Array[Dictionary] | Layer definitions (see structure below) |
| `bedrock_block`           | int               | Block type below all Y-range layers     |
| `filler_block`            | int               | Block type for undefined Y positions    |

**Subsurface Layer Dictionary Structure**:

```gdscript
{
    "block_type": int,      # VoxelType enum value
    "y_min": int,           # Minimum Y coordinate
    "y_max": int,           # Maximum Y coordinate
    "density": float        # 0.0 to 1.0 placement probability
}
```

### 2. Get Voxel at Position

**Method**: `get_voxel_at()`

**C++ Signature**:

```cpp
Ref<Voxel> get_voxel_at(int x, int y, int z) const;
```

**GDScript Usage**:

```gdscript
# Get voxel at world position
var voxel = biome_gen.get_voxel_at(100, 45, 200)

if voxel != null:
    var voxel_type = voxel.get_type()
    match voxel_type:
        0: print("Air")
        1: print("Dirt")
        3: print("Stone")
        5: print("Sand")

    var position = voxel.get_position()
    print("Position: ", position)
```

**Returns**: `Ref<Voxel>` or null

**Behavior**:

1. Checks height at (x, z) to determine surface level
2. Scans all subsurface layers for matching Y-range
3. Applies density probabilistically
4. Returns appropriate voxel type with coordinates set

### 3. Get Blended Biome Weights

**Method**: `get_blended_biome_weights()`

**C++ Signature**:

```cpp
Array get_blended_biome_weights(float x, float z) const;
```

**GDScript Usage**:

```gdscript
# Get smooth biome transitions
var weights = biome_gen.get_blended_biome_weights(100.0, 200.0)

for weight_entry in weights:
    var biome_index = weight_entry["biome_index"]
    var weight = weight_entry["weight"]
    print("Biome %d: %.2f%%" % [biome_index, weight * 100])
```

**Returns**: Array of Dictionary entries

**Entry Structure**:

```gdscript
{
    "biome_index": int,     # Biome index (0 = first added)
    "weight": float         # Blend weight (0.0 to 1.0)
}
```

### 4. Select Voxel with Blending

**Method**: `select_voxel_with_blending()`

**C++ Signature**:

```cpp
int select_voxel_with_blending(
    int x, int y, int z,
    const Array &biome_weights,
    bool is_surface
) const;
```

**GDScript Usage**:

```gdscript
# Get blended weights
var weights = biome_gen.get_blended_biome_weights(100.0, 200.0)

# Select voxel with smooth transitions
var voxel_type = biome_gen.select_voxel_with_blending(100, 45, 200, weights, false)

print("Selected voxel type: ", voxel_type)
```

---

## VoxelType Enumeration

Standard voxel types for layer definition:

```gdscript
enum VoxelType {
    AIR = 0,
    DIRT = 1,
    GRASS = 2,
    STONE = 3,
    WATER = 4,
    SAND = 5,
    LAVA = 6,
    GOLD = 7,
    DIAMOND = 8,
    COAL = 9,
    IRON = 10
}
```

---

## Practical Examples

### Example 1: Simple Stratified Terrain

```gdscript
# Mountain with distinct layers: Stone → Mixed → Bedrock
var mountain_layers = [
    {"block_type": 3, "y_min": 40, "y_max": 100, "density": 1.0},   # Stone peak
    {"block_type": 1, "y_min": 20, "y_max": 40, "density": 0.6},    # Mixed dirt/stone
    {"block_type": 3, "y_min": 0, "y_max": 20, "density": 1.0}      # Bedrock
]

biome_gen.add_biome_with_y_ranges(
    "Peak", 0.0, 50.0, -1.0, 0.0, -1.0, 0.0,
    PackedInt32Array([3]),  # Stone surface
    mountain_layers, 3, 3
)
```

### Example 2: Ore Distribution

```gdscript
# Mine-friendly depths with ore veins
var mining_layers = [
    {"block_type": 1, "y_min": 60, "y_max": 100, "density": 0.8},   # Dirt (common)
    {"block_type": 9, "y_min": 40, "y_max": 60, "density": 0.05},   # Coal (1/20 blocks)
    {"block_type": 10, "y_min": 20, "y_max": 40, "density": 0.02},  # Diamond (1/50)
    {"block_type": 3, "y_min": 0, "y_max": 20, "density": 1.0}      # Bedrock
]

biome_gen.add_biome_with_y_ranges(
    "MineZone", -5.0, 25.0, 0.0, 1.0, -1.0, 1.0,
    PackedInt32Array([1, 2]),  # Dirt/Grass surface
    mining_layers, 3, 1
)
```

### Example 3: Desert with Water Table

```gdscript
# Desert with subsurface aquifer
var desert_layers = [
    {"block_type": 5, "y_min": 30, "y_max": 100, "density": 1.0},   # Sand
    {"block_type": 4, "y_min": 15, "y_max": 30, "density": 0.5},    # Water aquifer
    {"block_type": 3, "y_min": 0, "y_max": 15, "density": 1.0}      # Stone
]

biome_gen.add_biome_with_y_ranges(
    "Desert", 5.0, 25.0, 0.5, 1.0, -1.0, 0.0,
    PackedInt32Array([5]),  # Sand surface
    desert_layers, 3, 3
)
```

---

## Integration with VoxelGenerator

### Setting Up Y-Range Biomes in Your Scene

```gdscript
extends Node3D

@onready var voxel_generator: VoxelGenerator = $VoxelGenerator

func _ready() -> void:
    # Create biome generator with Y-range layers
    var biome_gen = BiomeGenerator.new()

    # Add your biomes with layers...
    biome_gen.add_biome_with_y_ranges(...)

    # Assign to VoxelGenerator
    voxel_generator.set_biome_generator(biome_gen)

    # Generate terrain
    voxel_generator.generate()
```

### Debugging Layer Placement

```gdscript
# Check what voxel type is at a specific location
var voxel = biome_gen.get_voxel_at(x, y, z)
if voxel:
    print("At (%d, %d, %d): Type %d" % [x, y, z, voxel.get_type()])

# Enable debug visualization (F5 in demo)
voxel_generator.set_show_voxel_grid(true)
voxel_generator.set_debug_mode(true)
voxel_generator.set_debug_verbosity(2)
```

---

## Performance Considerations

### Density Impact

- **High density (0.9-1.0)**: Minimal performance cost, near-solid layers
- **Medium density (0.4-0.6)**: Normal performance, sparse placement
- **Low density (0.01-0.1)**: Most efficient, ore veins and features
- **Zero density (0.0)**: Not recommended; use fallback instead

### Layer Count

- **Recommended**: 3-5 layers per biome
- **Maximum tested**: 20+ layers (still efficient)
- **Optimization**: Order layers by expected frequency (most common first)

### Y-Range Optimization

```gdscript
# Efficient: Non-overlapping ranges
var good_layers = [
    {"block_type": 5, "y_min": 50, "y_max": 100, "density": 1.0},
    {"block_type": 3, "y_min": 20, "y_max": 50, "density": 1.0},
    {"block_type": 1, "y_min": 0, "y_max": 20, "density": 1.0}
]

# Inefficient: Overlapping ranges (processes multiple checks)
var bad_layers = [
    {"block_type": 5, "y_min": 30, "y_max": 100, "density": 1.0},
    {"block_type": 3, "y_min": 0, "y_max": 50, "density": 1.0}
]
```

---

## Testing & Validation

### Run Included Test Suite

```bash
# The P3 test suite validates:
# 1. Biome creation with Y-range layers
# 2. Layer matching at specific Y coordinates
# 3. Density-based probabilistic placement
# 4. Voxel type selection per layer
# 5. Biome blending functionality
# 6. VoxelGenerator integration
```

### Manual Verification

1. **Load demo project** in Godot 4.5+
2. **Add integration example** to your scene
3. **Play scene** to run automated tests
4. **Check console** for pass/fail results
5. **Enable F5 visualization** to see terrain
6. **Verify layer transitions** at different Y levels

---

## Migration from Deprecated System

### Old System (Depth-Based)

```gdscript
biome_gen.add_biome(
    "OldBiome", 0.0, 10.0, -0.5, 0.5, 0.0, 1.0,
    PackedInt32Array([2, 1]),           # Surface
    PackedInt32Array([1, 3]),           # Subsurface
    4                                   # Depth in blocks
)
```

### New System (Y-Range-Based)

```gdscript
var layers = [
    {"block_type": 1, "y_min": 10, "y_max": 20, "density": 0.9},
    {"block_type": 3, "y_min": 0, "y_max": 10, "density": 1.0}
]

biome_gen.add_biome_with_y_ranges(
    "NewBiome", 0.0, 10.0, -0.5, 0.5, 0.0, 1.0,
    PackedInt32Array([2]),              # Surface (grass)
    layers,                             # Y-range layers
    3,                                  # Bedrock
    3                                   # Filler
)
```

**Benefits of Y-Range System**:

- ✅ Explicit control over layer positioning
- ✅ Non-destructive transitions (no abrupt cutoffs)
- ✅ Density support for sparse placement
- ✅ No depth-based limitations
- ✅ Better performance for sparse features

---

## Advanced Techniques

### Creating Ore Veins with Density

```gdscript
# Sparse ore distribution at depth
var ore_layer = {
    "block_type": 10,        # DIAMOND
    "y_min": 5,
    "y_max": 20,
    "density": 0.02          # Only 2% placement = sparse veins
}
```

### Multi-Layer Gradients

```gdscript
# Smooth transition from sand to stone
var gradient_layers = [
    {"block_type": 5, "y_min": 60, "y_max": 100, "density": 1.0},    # Pure sand
    {"block_type": 5, "y_min": 40, "y_max": 60, "density": 0.7},     # Mostly sand
    {"block_type": 3, "y_min": 20, "y_max": 40, "density": 0.7},     # Mostly stone
    {"block_type": 3, "y_min": 0, "y_max": 20, "density": 1.0}       # Pure stone
]
```

### Biome-Specific Features

```gdscript
# Cave systems (water channels)
var cave_layer = {
    "block_type": 4,         # WATER
    "y_min": 30,
    "y_max": 50,
    "density": 0.05          # Rare water channels
}

# Lava chambers
var lava_layer = {
    "block_type": 6,         # LAVA
    "y_min": 5,
    "y_max": 15,
    "density": 0.01          # Very rare lava pockets
}
```

---

## Troubleshooting

### Issue: Layers not appearing correctly

**Cause**: Y-range overlaps or incorrect density
**Solution**:

- Check Y-range boundaries don't conflict
- Verify density > 0 (0 means no placement)
- Use get_voxel_at() to debug specific positions

### Issue: Performance degradation

**Cause**: Too many layers or high density at every level
**Solution**:

- Reduce layer count to essential types
- Use density < 1.0 for sparse features
- Profile with VoxelGenerator debug output

### Issue: Biome not being used

**Cause**: Height or climate ranges don't match terrain
**Solution**:

- Check height_range matches your terrain height
- Verify temperature/humidity range overlap
- Use biome blending to bridge transitions

---

## File Reference

| File                                   | Purpose                | Status      |
| -------------------------------------- | ---------------------- | ----------- |
| `src/core/voxel_constants.h`           | SubsurfaceLayer struct | ✅ Complete |
| `src/generators/BiomeGenerator.h`      | Method declarations    | ✅ Complete |
| `src/generators/BiomeGenerator.cpp`    | Y-range implementation | ✅ Complete |
| `demo/p3_biome_integration_example.gd` | Integration guide      | ✅ Complete |
| `demo/test_y_range_biomes.gd`          | Test suite             | ✅ Complete |

---

## Next Steps (P4+)

### Performance Optimization

- Caching layer lookups for repeated queries
- Parallel biome calculation across chunks
- GPU acceleration for density sampling

### Extended Features

- Layer animation (gradual transitions)
- Dynamic layer modification at runtime
- Procedural ore distribution algorithms

### Quality of Life

- Visual biome editor in Godot
- Layer preview tool
- Performance profiling dashboard

---

**Status**: P3 Complete ✅  
**Next**: Ready for P4 optimization phase  
**Questions?** Check test_y_range_biomes.gd for comprehensive examples
