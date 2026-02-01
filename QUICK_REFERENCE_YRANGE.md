# Y-Range Biome System - Quick Reference Card

## 30-Second Overview

The Y-Range Biome Layer System lets you define terrain layers at specific Y-coordinates with probabilistic placement.

```gdscript
# Define layers
var layers = [
    {"block_type": 3, "y_min": 50, "y_max": 100, "density": 1.0},  # Stone top
    {"block_type": 5, "y_min": 20, "y_max": 50, "density": 0.8},   # Sand middle
    {"block_type": 3, "y_min": 0, "y_max": 20, "density": 1.0}     # Stone bottom
]

# Create biome
biome_gen.add_biome_with_y_ranges("Mountain",
    0.0, 50.0, -1.0, 1.0, -1.0, 1.0,
    PackedInt32Array([3]), layers, 3, 3)

# Use it
var voxel = biome_gen.get_voxel_at(x, y, z)
```

---

## API at a Glance

| Method                            | Purpose                          | Returns     |
| --------------------------------- | -------------------------------- | ----------- |
| `add_biome_with_y_ranges()`       | Create biome with Y-range layers | void        |
| `get_voxel_at(x, y, z)`           | Get voxel type at position       | Ref<Voxel>  |
| `get_blended_biome_weights(x, z)` | Get biome blend                  | Array[Dict] |
| `select_voxel_with_blending()`    | Select with transitions          | int         |

---

## Layer Dictionary

```gdscript
{
    "block_type": int,    # VoxelType (0-10)
    "y_min": int,        # Layer start (inclusive)
    "y_max": int,        # Layer end (inclusive)
    "density": float     # Placement probability (0.0-1.0)
}
```

---

## VoxelType Quick Reference

```
0 = AIR       5 = SAND      10 = IRON
1 = DIRT      6 = LAVA
2 = GRASS     7 = GOLD
3 = STONE     8 = BEDROCK
4 = WATER     9 = COAL
```

---

## Common Patterns

### Solid Layer

```gdscript
{"block_type": 3, "y_min": 0, "y_max": 50, "density": 1.0}
```

### Sparse Ore Vein

```gdscript
{"block_type": 9, "y_min": 20, "y_max": 40, "density": 0.02}  # 2%
```

### Gradient Transition

```gdscript
# Pure sand
{"block_type": 5, "y_min": 60, "y_max": 100, "density": 1.0},
# Mostly sand
{"block_type": 5, "y_min": 40, "y_max": 60, "density": 0.7},
# Mostly stone
{"block_type": 3, "y_min": 20, "y_max": 40, "density": 0.7},
# Pure stone
{"block_type": 3, "y_min": 0, "y_max": 20, "density": 1.0}
```

---

## Real-World Examples

### Simple 3-Layer Mountain

```gdscript
var layers = [
    {"block_type": 3, "y_min": 40, "y_max": 100, "density": 1.0},  # Peak
    {"block_type": 1, "y_min": 20, "y_max": 40, "density": 0.6},   # Mid
    {"block_type": 3, "y_min": 0, "y_max": 20, "density": 1.0}     # Base
]

biome_gen.add_biome_with_y_ranges("Peak",
    0.0, 50.0, -1.0, 0.0, -1.0, 0.0,
    PackedInt32Array([3]), layers, 3, 3)
```

### Mining Zone with Ores

```gdscript
var layers = [
    {"block_type": 1, "y_min": 60, "y_max": 100, "density": 0.8},
    {"block_type": 9, "y_min": 40, "y_max": 60, "density": 0.05},  # Coal
    {"block_type": 10, "y_min": 20, "y_max": 40, "density": 0.02}, # Diamond
    {"block_type": 3, "y_min": 0, "y_max": 20, "density": 1.0}
]

biome_gen.add_biome_with_y_ranges("MineZone",
    -5.0, 25.0, 0.0, 1.0, -1.0, 1.0,
    PackedInt32Array([1, 2]), layers, 3, 1)
```

### Desert with Water Table

```gdscript
var layers = [
    {"block_type": 5, "y_min": 30, "y_max": 100, "density": 1.0},
    {"block_type": 4, "y_min": 15, "y_max": 30, "density": 0.5},   # Water
    {"block_type": 3, "y_min": 0, "y_max": 15, "density": 1.0}
]

biome_gen.add_biome_with_y_ranges("Desert",
    5.0, 25.0, 0.5, 1.0, -1.0, 0.0,
    PackedInt32Array([5]), layers, 3, 3)
```

---

## Integration with VoxelGenerator

```gdscript
extends Node3D

@onready var voxel_gen: VoxelGenerator = $VoxelGenerator

func _ready() -> void:
    var biome_gen = BiomeGenerator.new()

    # Add your biomes...
    biome_gen.add_biome_with_y_ranges(...)

    # Assign to generator
    voxel_gen.set_biome_generator(biome_gen)

    # Generate!
    voxel_gen.generate()
```

---

## Debug Tips

```gdscript
# Check what voxel type is at position
var voxel = biome_gen.get_voxel_at(100, 45, 200)
print("Type: ", voxel.get_type())

# Enable debug visualization
voxel_gen.set_debug_mode(true)
voxel_gen.set_show_voxel_grid(true)

# Check biome weights (for blending)
var weights = biome_gen.get_blended_biome_weights(100.0, 200.0)
print("Weights: ", weights)
```

---

## Performance Tips

| Aspect      | Recommendation        | Why                           |
| ----------- | --------------------- | ----------------------------- |
| Layer Count | 3-5 per biome         | Balance detail vs performance |
| Density     | <0.1 for ores         | Sparse = efficient            |
| Y-Ranges    | Non-overlapping       | No redundant checks           |
| Order       | Frequent layers first | Early match = fast            |

---

## Files to Check

- **Tests**: `demo/test_y_range_biomes.gd` (6 tests)
- **Example**: `demo/p3_biome_integration_example.gd` (3 biomes)
- **Reference**: `Y_RANGE_REFERENCE_GUIDE.md` (complete API)
- **Status**: `P3_COMPLETION_SUMMARY.md` (what was built)

---

## Status

✅ **Phase 3 Complete**  
✅ **6 Tests Ready**  
✅ **3 Examples Ready**  
✅ **1000+ Lines Documented**  
✅ **Production Ready**

---

Last Updated: January 29, 2026  
For detailed information, see Y_RANGE_REFERENCE_GUIDE.md
