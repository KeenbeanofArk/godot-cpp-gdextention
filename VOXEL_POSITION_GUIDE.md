# Voxel Position Calculation Guide

## Coordinate System Hierarchy

```
World Space (centered at origin)
    ↓
Chunk Coordinates (grid positions)
    ↓
Local Voxel Coordinates (within chunk)
    ↓
Flattened Array Index (1D storage)
```

## Configuration Example

- **World Size**: (5, 5, 5) chunks
- **Chunk Size**: 8 voxels per dimension
- **Total Voxels**: 40 × 40 × 40 = 64,000 voxels
- **Total Chunks**: 5 × 5 × 5 = 125 chunks

## World Layout

```
                    +Y (up)
                     |
                     |
        +20 ┌────────┼────────┐
            │        |        │
            │   Chunk(4,4,4)  │
            │        |        │
      -X ───┼────────┼────────┼─── +X
            │        |        │
            │  Chunk(0,0,0)   │
            │        |        │
        -20 └────────┼────────┘
                     |
                     |
                    -Y (down)

World extends: -20 to +20 in each axis
Chunk (0,0,0) corner at: (-20, -20, -20)
Chunk (4,4,4) corner at: (12, 12, 12)
```

## Conversion Formulas

### 1. Chunk Coordinate ↔ Chunk Index

**Coordinate → Index** (3D to 1D):

```
index = x + y * world_size.x + z * world_size.x * world_size.y
```

**Index → Coordinate** (1D to 3D):

```
z = index / (world_size.x * world_size.y)
remainder = index % (world_size.x * world_size.y)
y = remainder / world_size.x
x = remainder % world_size.x
```

### 2. Chunk Coordinate → World Position

**World Extents**:

```
world_extent_x = world_size.x * chunk_size = 5 * 8 = 40
world_extent_y = world_size.y * chunk_size = 5 * 8 = 40
world_extent_z = world_size.z * chunk_size = 5 * 8 = 40
```

**Chunk Corner Position**:

```
corner.x = chunk_coord.x * chunk_size - world_extent_x * 0.5
corner.y = chunk_coord.y * chunk_size - world_extent_y * 0.5
corner.z = chunk_coord.z * chunk_size - world_extent_z * 0.5
```

**Chunk Center Position**:

```
center.x = (chunk_coord.x + 0.5) * chunk_size - world_extent_x * 0.5
center.y = (chunk_coord.y + 0.5) * chunk_size - world_extent_y * 0.5
center.z = (chunk_coord.z + 0.5) * chunk_size - world_extent_z * 0.5
```

### 3. Local Voxel Position → Flattened Index

**Within Chunk** (size N):

```
voxel_index = x * N * N + y * N + z
```

For chunk_size = 8:

```
voxel_index = x * 64 + y * 8 + z
```

### 4. Complete: Any Position → Voxel World Position

Given a position anywhere in the world:

```
1. Find chunk coordinate:
   chunk_coord.x = floor((world_pos.x + world_extent_x * 0.5) / chunk_size)
   chunk_coord.y = floor((world_pos.y + world_extent_y * 0.5) / chunk_size)
   chunk_coord.z = floor((world_pos.z + world_extent_z * 0.5) / chunk_size)

2. Find local voxel coordinate within chunk:
   local.x = floor(world_pos.x - chunk_corner.x)
   local.y = floor(world_pos.y - chunk_corner.y)
   local.z = floor(world_pos.z - chunk_corner.z)

3. Get voxel index within chunk:
   voxel_index = local.x * 64 + local.y * 8 + local.z
```

## Worked Example

**Target**: Voxel at chunk **(2, 1, 3)**, local position **(5, 6, 7)**

### Step 1: Chunk Index

```
index = 2 + 1 * 5 + 3 * 5 * 5
      = 2 + 5 + 75
      = 82
```

### Step 2: Chunk Corner World Position

```
corner.x = 2 * 8 - 40 * 0.5 = 16 - 20 = -4
corner.y = 1 * 8 - 40 * 0.5 = 8 - 20 = -12
corner.z = 3 * 8 - 40 * 0.5 = 24 - 20 = 4

Chunk corner: (-4, -12, 4)
```

### Step 3: Chunk Center World Position

```
center.x = (2 + 0.5) * 8 - 20 = 20 - 20 = 0
center.y = (1 + 0.5) * 8 - 20 = 12 - 20 = -8
center.z = (3 + 0.5) * 8 - 20 = 28 - 20 = 8

Chunk center: (0, -8, 8)
```

### Step 4: Voxel World Position

```
voxel_world.x = corner.x + local.x = -4 + 5 = 1
voxel_world.y = corner.y + local.y = -12 + 6 = -6
voxel_world.z = corner.z + local.z = 4 + 7 = 11

Voxel world position: (1, -6, 11)
```

### Step 5: Flattened Voxel Index Within Chunk

```
voxel_index = 5 * 64 + 6 * 8 + 7
            = 320 + 48 + 7
            = 375
```

## Quick Reference Table: First Few Chunks

| Chunk Coord | Index | Corner Position | Center Position |
| ----------- | ----- | --------------- | --------------- |
| (0, 0, 0)   | 0     | (-20, -20, -20) | (-16, -16, -16) |
| (1, 0, 0)   | 1     | (-12, -20, -20) | (-8, -16, -16)  |
| (2, 0, 0)   | 2     | (-4, -20, -20)  | (0, -16, -16)   |
| (0, 1, 0)   | 5     | (-20, -12, -20) | (-16, -8, -16)  |
| (0, 0, 1)   | 25    | (-20, -20, -12) | (-16, -16, -8)  |
| (2, 1, 3)   | 82    | (-4, -12, 4)    | (0, -8, 8)      |
| (4, 4, 4)   | 124   | (12, 12, 12)    | (16, 16, 16)    |

## Quick Reference Table: Voxel Local Positions

For voxels within a chunk (chunk_size = 8):

| Local (x,y,z) | Flattened Index | Offset from Corner |
| ------------- | --------------- | ------------------ |
| (0, 0, 0)     | 0               | (0, 0, 0)          |
| (1, 0, 0)     | 64              | (1, 0, 0)          |
| (0, 1, 0)     | 8               | (0, 1, 0)          |
| (0, 0, 1)     | 1               | (0, 0, 1)          |
| (7, 7, 7)     | 511             | (7, 7, 7)          |
| (5, 6, 7)     | 375             | (5, 6, 7)          |

## Code References

- **Chunk indexing**: `src/VoxelGenerator.cpp:3927` (coord_to_index)
- **Index to coord**: `src/VoxelGenerator.cpp:3932` (index_to_chunk_coord)
- **World position**: `src/VoxelGenerator.cpp:1241` (get_chunk_center_world_position)
- **Voxel index**: `src/core/chunk.h:142` (get_voxel_index)

## Visual: Voxel Array Storage

```
Chunk array (flattened 1D):
[0][1][2]...[511]
 ↑           ↑
(0,0,0)   (7,7,7)

Index calculation pattern:
- X changes every 64 voxels (x * chunk_size²)
- Y changes every 8 voxels  (y * chunk_size)
- Z changes every 1 voxel   (z)

Example:
voxels[375] = position (5, 6, 7)
  5 * 64 = 320
  6 * 8  = 48
  7 * 1  = 7
  Total  = 375
```
