# Keen Voxel Engine - AI Coding Instructions

This is a Godot 4 C++ GDExtension implementing a voxel terrain engine with dynamic LOD, terraforming, and multi-generator support. Focus on these core patterns when contributing.

## Architecture Overview

**Three-layer hierarchy:**

- [VoxelGenerator](../src/VoxelGenerator.h) (Node3D): Main orchestrator managing async generation, LOD, terrain modification
- [Chunk](../src/core/chunk.h) (Node3D): Discrete 8-64 voxel cubes; owns MeshInstance3D and collision
- [Voxel](../src/core/voxel.h): Core density/material data; stored in flat 1D arrays for cache efficiency

**Key design decisions:**

- **Async generation**: Worker threads + `WorkerThreadPool` + task groups to prevent frame stalls
- **Density caching**: Pre-computed corner densities for marching cubes (optimization, checked by `cache_is_valid`)
- **Two generation modes**: VOXELS_FIRST (3D full density) vs HEIGHTMAP_FIRST (2D heightmap + surface band)
- **Per-chunk LOD**: Distance-based LOD (8 levels) adjusts effective resolution per chunk
- **Flat array storage**: Chunks use `std::unique_ptr<Ref<Voxel>[]>` for locality (no std::vector<Voxel>)

## Build & Test Workflow

**Windows build (SCons):**

```powershell
# First time: initialize godot-cpp submodule
git submodule update --init --recursive

# Build bindings (optional but recommended)
cd godot-cpp && scons -j12 platform=windows && cd ..

# Build extension (from root)
scons -j12 target=template_debug debug_symbols=yes platform=windows

# Copy to demo
New-Item -ItemType Directory -Force -Path .\demo\bin\windows
Copy-Item -Path .\bin\windows\* -Destination .\demo\bin\windows\ -Force

# Run demo in Godot
# Open .\demo in Godot 4.5+
```

**Key files:**

- [SConstruct](../SConstruct): Build config; `libname="voxel-engine-gd"`, output → `bin/<platform>/`
- [BUILD_WINDOWS.md](../BUILD_WINDOWS.md): Detailed Windows walkthrough
- [register_types.cpp](../src/register_types.cpp): Class registration entry point

## Critical Patterns & Conventions

### 1. Thread-Safe Operations

- Use `std::atomic<bool>` for simple flags: `mesh_ready`, `cache_is_valid`
- Use `std::shared_mutex` + read/write locks for multi-read/single-write sections
- **Never call Godot API from worker threads** — queue results in `ChunkMeshData` struct, apply in `_process()`
- Example: [VoxelGenerator::\_process()](../src/VoxelGenerator.cpp) dequeues `pending_meshes` and applies to scenes

### 2. Marching Cubes Density Evaluation

- Core function: [evaluate_density(Vector3)](../src/VoxelGenerator.cpp) — samples terrain_noise + detail_noise + biome
- Heightmap mode uses pre-cached 2D heights; voxel mode evaluates all 8 corner densities
- LOD scaling: `effective_resolution = resolution >> lod_level` reduces voxel size by powers of 2
- **Surface band concept**: HEIGHTMAP_FIRST only meshes within `surface_band` units of terrain surface (performance optimization)

### 3. Generation Mode Logic

Check `generation_mode` before major operations:

```cpp
if (generation_mode == HEIGHTMAP_FIRST) {
    // Use heightmap_cache, apply surface_band
} else {
    // Full 3D voxel evaluation
}
```

Heightmap mode caches are **invalidated on LOD/mode/noise changes**; see [set_generation_mode()](../src/VoxelGenerator.cpp)

### 4. Chunk Indexing & Coordinates

- Linear index ↔ 3D coord: [index_to_chunk_coord()](../src/VoxelGenerator.cpp) / [chunk_coord_to_index()](../src/VoxelGenerator.cpp)
- Chunk 0 is at local (0,0,0); chunks arranged in XYZ order
- World position: `chunk_coord * chunk_size * voxel_scale` (see [get_chunk_center_world_position()](../src/VoxelGenerator.cpp))
- **Always validate indices** with `chunk_coord_to_index()` returning -1 for out-of-bounds

### 5. Logging & Debug Output

- Call `log_message(String msg, int verbosity_level)` instead of `print()`
- Verbosity: 1=important, 2=verbose, 3=very verbose
- Check `debug_mode` and `debug_verbosity` before expensive debug work
- Example: [recalculate_voxel_scale()](../src/VoxelGenerator.cpp)

### 6. Property Binding & Godot API

- Bind C++ methods with `ClassDB::bind_method(D_METHOD("name"), &Class::method)` in `_bind_methods()`
- Use `ADD_PROPERTY()` for Inspector exposure
- Setters should call `invalidate_density_region()` or `mark_chunk_dirty()` if they affect generation
- Constructor initializes only POD types; lazy-create Godot objects (NoiseGenerator, etc.) in `_notification(READY)` to avoid "used as default" warnings

### 7. Dirty Tracking & Incremental Updates

- `mark_chunk_dirty(chunk_coord)` + `regenerate_dirty_chunks()` allow incremental updates without full rebuild
- `invalidate_density_region(min_voxel, max_voxel)` clears density cache for region
- Used by terraforming (`dig_sphere`, `build_sphere`) to apply deformations efficiently

### 8. Async Task Management

- Wrap async work in `WorkerThreadPool::GroupID` tasks
- Cancel with `WorkerThreadPool::get_singleton()->wait_for_task_completion(async_task_group)` in destructor
- Signal completion: emit `generation_complete` signal when done
- Track progress with `generation_progress(completed, total)` signal every `signal_every_n_chunks`

## Generator Interface

Three independent generators can be composed:

1. **[NoiseGenerator](../src/generators/NoiseGenerator.h)**: Wraps FastNoiseLite (Perlin, Simplex, etc.)

   - Methods: `sample_2d(x, z)`, `sample_3d(x, y, z)`

2. **[BiomeGenerator](../src/generators/BiomeGenerator.h)**: Produces biome data (temperature, humidity, material type)

   - Sibling to FeatureGenerator; used for terrain stratification

3. **[FeatureGenerator](../src/generators/FeatureGenerator.h)**: Places features (trees, ore veins, caves)
   - Sibling to BiomeGenerator; decorative, not structural

All called in [evaluate_density()](../src/VoxelGenerator.cpp) to modulate terrain shape and material.

## Voxel/Chunk Lifecycle

1. **Chunk creation** ([VoxelGenerator::create_chunk()](../src/VoxelGenerator.cpp)): Allocate voxels, queue for generation
2. **Density evaluation**: Worker thread fills voxel densities via noise + generators
3. **Mesh generation**: Worker thread runs marching cubes → vertices/normals/colors → `ChunkMeshData`
4. **Main thread apply**: `_process()` dequeues, creates MeshInstance3D, applies material, adds collision
5. **Cleanup**: Destructor clears all chunks + cancels async tasks

## Common Pitfalls

- **Accessing Chunks from threads**: Only read voxel densities; never modify scene tree from worker
- **Forgetting cache invalidation**: Changing LOD or noise → always clear `heightmap_cache` and `density_cache`
- **Marching cubes ambiguity**: Use `corner_from_edge[]` tables to map edge indices to vertices
- **Chunk coord overflow**: Out-of-bounds checks return -1; test before dereferencing
- **Memory leaks**: All dynamically allocated chunks/meshes must be freed in destructor

## File Organization

```
src/
  VoxelGenerator.cpp/.h      ← Main class; ~3500 lines
  register_types.cpp/.h      ← Module init
  Constants.h                ← Marching cubes lookup tables
  core/
    chunk.cpp/.h             ← Chunk Node3D
    voxel.cpp/.h             ← Voxel data struct
    voxel_constants.h        ← Size constants
    voxel_registry.cpp/.h    ← Voxel type registry
    voxel_material.cpp/.h    ← Material/color mapping
    direction.h              ← Cardinal directions enum
  generators/
    NoiseGenerator.cpp/.h    ← FastNoiseLite wrapper
    BiomeGenerator.cpp/.h    ← Biome sampling
    FeatureGenerator.cpp/.h  ← Feature placement
demo/                        ← Godot project (test scene)
```

## Debugging Tips

- **Enable verbose logging**: Set `debug_verbosity = 3` in inspector
- **Visualize meshes**: Set `show_voxel_grid = true`, `show_chunk_grid = true`
- **Check cache state**: Log `cache_is_valid`, `cache_resolution`; mismatch = stale data
- **Performance**: Monitor `max_chunks_per_frame` (default 4); reduce if frame time spikes
- **GDScript calls**: Demo uses [main.gd](../demo/main.gd), [gui.gd](../demo/gui.gd) for inspector binding examples
