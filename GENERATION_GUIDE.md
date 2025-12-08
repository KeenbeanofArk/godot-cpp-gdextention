# VoxelGenerator: `generate()` vs `generate_async()`

## Quick Comparison

| Feature         | `generate()`                      | `generate_async()`                    |
| --------------- | --------------------------------- | ------------------------------------- |
| **Execution**   | Synchronous (blocking)            | Asynchronous (non-blocking)           |
| **Thread**      | Main thread only                  | WorkerThreadPool (background threads) |
| **Frame Rate**  | ⚠️ Freezes game during generation | ✅ Maintains 60 FPS                   |
| **Mesh Output** | Immediate, all chunks ready       | Progressive, chunks ready over time   |
| **Best For**    | Small worlds, startup, editor     | Large worlds, runtime generation      |
| **Setup Time**  | Instant                           | Slightly slower (async overhead)      |

---

## When to Use Each

### Use `generate()` When:

- **Initializing at startup** - Generate small test worlds before gameplay starts
- **Editor previews** - Quick visualization of small terrain patches
- **Debug/development** - Testing with small world_size (e.g., 5×5×5)
- **One-time full regeneration** - Resetting entire world with `auto_generate = true`
- **Guaranteed completion** - Need all chunks ready before proceeding

**Example:**

```gdscript
# Game startup - small test world
voxel_gen.world_size = Vector3i(5, 5, 5)
voxel_gen.generate()  # Returns when ALL 125 chunks are ready
print("World ready:", voxel_gen.chunks.size())
```

### Use `generate_async()` When:

- **Runtime in large worlds** - 10×10×10 chunks or larger
- **Maintain frame rate** - Smooth 60 FPS gameplay required
- **Streaming gameplay** - Player moves through infinite/large terrain
- **With LOD system** - Distance-based LOD updating chunks progressively
- **Player experience matters** - Don't freeze the game

**Example:**

```gdscript
# Runtime with biome generator - large world
voxel_gen.enable_distance_lod = true
voxel_gen.world_size = Vector3i(10, 22, 10)  # 2200 chunks
voxel_gen.generate_async()  # Returns immediately, generates in background

func _process(_delta):
    # Game continues at 60 FPS while chunks generate
    update_player_position()
```

---

## Detailed Workflow Comparison

### `generate()` - Synchronous Pipeline

```
┌─────────────────────────────────────────┐
│ VoxelGenerator.generate() called        │
├─────────────────────────────────────────┤
│ 1. Prevent concurrent generation        │
│ 2. Initialize noise generators          │
│ 3. Expand world Y for biome heights     │ ← ensure_vertical_extent_for_biomes()
│ 4. Recalculate voxel scale              │
│ 5. Create chunk nodes                   │
│ 6. Prepare procedural features          │
│ 7. Build heightmap/density cache        │
│ 8. Generate mesh for EACH chunk         │ ← BLOCKING loop
│    (all chunks sequentially)            │
│ 9. Create debug visualizations          │
│   (voxel grid, chunk grid)              │
│ 10. Release generation lock             │
└─────────────────────────────────────────┘
         ALL DONE - Returns
      (may have frozen game!)
```

**Cost:** Synchronous loop generates all chunks one by one:

- 125 chunks × ~10ms each = ~1250ms (1.25 sec freeze)
- 2200 chunks × ~10ms each = ~22000ms (22 sec freeze!) ❌

### `generate_async()` - Asynchronous Pipeline

```
┌─────────────────────────────────────────┐
│ VoxelGenerator.generate_async() called  │
├─────────────────────────────────────────┤
│ 1. Initialize noise generators          │
│ 2. Expand world Y for biome heights     │ ← ensure_vertical_extent_for_biomes()
│ 3. Recalculate voxel scale              │
│ 4. Create chunk nodes                   │
│ 5. Prepare procedural features          │
│ 6. Build heightmap/density cache        │
│ 7. Submit worker tasks to thread pool   │
│    (one task per chunk)                 │
│ 8. Enable _process() to collect results │
│ 9. Return immediately                   │
└─────────────────────────────────────────┘
       Returns in ~10ms ✅

   ╭─────────────────────────────────────╮
   │ Background (WorkerThreadPool)       │
   ├─────────────────────────────────────┤
   │ _chunk_generation_task() running    │
   │ on N worker threads in parallel:    │
   │                                     │
   │ Generate mesh for chunk 0           │
   │ Generate mesh for chunk 1           │
   │ Generate mesh for chunk 2           │
   │ ... (many in parallel)              │
   │ Generate mesh for chunk N-1         │
   │                                     │
   │ Results queued in pending_meshes    │
   ╰─────────────────────────────────────╯

   ╭─────────────────────────────────────╮
   │ Main Thread (_process each frame)   │
   ├─────────────────────────────────────┤
   │ apply_pending_meshes() - apply      │
   │ completed chunks to scene tree      │
   │                                     │
   │ emit_signal("chunk_ready")          │
   │ progress_count += max_chunks_per_   │
   │                      frame          │
   │ emit_signal("generation_progress")  │
   ╰─────────────────────────────────────╯

   On completion:
   ┌─────────────────────────────────────┐
   │ emit_signal("generation_complete")  │
   │ set_process(false)                  │
   │ generation_in_progress = false      │
   └─────────────────────────────────────┘
```

**Cost:** Parallel generation with streaming output:

- 125 chunks in parallel = ~30-50ms per "batch" = <500ms total ✅
- 2200 chunks in parallel = ~500ms-1000ms total ✅

---

## Key Behavioral Differences

### Chunk Creation

- **`generate()`** - Creates chunks immediately before mesh generation
- **`generate_async()`** - Creates chunks before submitting async tasks

Both call `create_chunks()`, but async can start using them for worker threads while main thread continues.

### Cache Building

Both build the same caches, but at different points:

- **`generate()`** - Builds once, uses immediately in loop
- **`generate_async()`** - Builds once, workers access same cache in parallel

### Biome Height Expansion

**Both** call `ensure_vertical_extent_for_biomes()`:

```cpp
// If biome generator is set, automatically expands world Y
ensure_vertical_extent_for_biomes();  // May change world_size.y
recalculate_voxel_scale();             // Update extent for new size
```

This is why your world_size automatically expanded from (10, 5, 10) → (10, 22, 10).

### Mesh Application

- **`generate()`** - Mesh applied immediately to chunks (synchronous)
- **`generate_async()`** - Mesh queued in `pending_meshes`, applied in `_process()` via `apply_pending_meshes()`

### Debug Visualization

- **`generate()`** - Creates voxel grid and chunk grid debug meshes at the END
- **`generate_async()`** - NO debug meshes (background thread can't access Godot scene tree)

This is why your voxel grid appears with old world_size dimensions!

---

## Signals & Monitoring Progress

### Available Signals:

```gdscript
voxel_gen.chunk_ready.connect(_on_chunk_ready)
voxel_gen.generation_progress.connect(_on_progress)
voxel_gen.generation_complete.connect(_on_complete)

func _on_chunk_ready(chunk_index: int, chunk_coord: Vector3i):
    print("Chunk %s ready" % chunk_coord)

func _on_progress(completed: int, total: int):
    print("Progress: %d/%d (%.1f%%)" % [completed, total, 100.0 * completed / total])

func _on_complete():
    print("All chunks ready!")
```

### Only Available with `generate_async()`:

- These signals are NOT emitted by `generate()` (it's instant)
- Used to show progress bars, update UI, etc.

---

## Properties Affecting Both

These properties apply the same way to both functions:

| Property              | Effect                                                       |
| --------------------- | ------------------------------------------------------------ |
| `world_size`          | Number of chunks (X, Y, Z). Auto-expands Y if biome present. |
| `chunk_size`          | Voxels per chunk edge (default: 8)                           |
| `resolution`          | Marching cubes detail (1=low, 10=high)                       |
| `generation_mode`     | VOXELS_FIRST or HEIGHTMAP_FIRST                              |
| `surface_band`        | Y-thickness for HEIGHTMAP_FIRST mode                         |
| `lod_level`           | Global LOD (0=max detail, 7=min detail)                      |
| `enable_distance_lod` | Per-chunk LOD based on distance from camera                  |
| `biome_generator`     | Enables biome-based terrain                                  |
| `auto_generate`       | If true, calls `generate()` on property changes              |

---

## Common Patterns

### Pattern 1: Startup Generation

```gdscript
func _ready():
    # Small world at startup - use generate()
    voxel_gen.world_size = Vector3i(5, 5, 5)
    voxel_gen.generate()  # Blocks briefly (~100ms)
    player.position = Vector3(0, 10, 0)  # Safe to use world
```

### Pattern 2: Runtime with Loading Screen

```gdscript
func start_new_game():
    # Show loading screen
    show_loading_screen()

    # Set up large world with biome
    voxel_gen.world_size = Vector3i(20, 20, 20)
    voxel_gen.biome_generator = my_biome_gen

    voxel_gen.generation_progress.connect(_on_gen_progress)
    voxel_gen.generation_complete.connect(_on_gen_complete)

    voxel_gen.generate_async()  # Returns immediately

func _on_gen_progress(done: int, total: int):
    loading_bar.value = 100.0 * done / total

func _on_gen_complete():
    hide_loading_screen()
    enable_gameplay()
    player.position = Vector3(0, 10, 0)
```

### Pattern 3: Streaming Terrain with LOD

```gdscript
var voxel_gen: VoxelGenerator

func _ready():
    voxel_gen.enable_distance_lod = true
    voxel_gen.lod_reference_position = player.position
    voxel_gen.world_size = Vector3i(10, 22, 10)  # Large world
    voxel_gen.generate_async()  # Start generation

    voxel_gen.chunk_ready.connect(_on_chunk_ready)
    set_process(true)

func _process(_delta):
    # Update LOD based on player position
    voxel_gen.lod_reference_position = player.position

    # This marks chunks dirty if LOD changed
    var chunks_needing_regen = voxel_gen.update_chunks_lod()

    # Only regenerate a few dirty chunks per frame
    if chunks_needing_regen > 0:
        voxel_gen.regenerate_dirty_chunks()

func _on_chunk_ready(chunk_index: int, chunk_coord: Vector3i):
    # Chunks appear progressively as they're ready
    pass
```

---

## Performance Summary

### Small Worlds (≤125 chunks)

| Scenario          | `generate()`  | `generate_async()` |
| ----------------- | ------------- | ------------------ |
| Frame freeze      | ~100-500ms    | None               |
| First chunk ready | ~10ms         | ~30ms              |
| All chunks ready  | ~500ms        | ~500ms             |
| **Best choice**   | ✅ generate() | ❌ Overkill        |

### Medium Worlds (125-1000 chunks)

| Scenario          | `generate()` | `generate_async()`  |
| ----------------- | ------------ | ------------------- |
| Frame freeze      | ~1-10 sec    | None                |
| First chunk ready | ~10ms        | ~30ms               |
| All chunks ready  | ~5-10 sec    | ~2-5 sec            |
| **Best choice**   | ❌ Too slow  | ✅ generate_async() |

### Large Worlds (>1000 chunks)

| Scenario          | `generate()`  | `generate_async()`  |
| ----------------- | ------------- | ------------------- |
| Frame freeze      | ~20+ sec      | None                |
| First chunk ready | ~10ms         | ~50ms               |
| All chunks ready  | ~20+ sec      | ~5-15 sec           |
| **Best choice**   | ❌ Unplayable | ✅ generate_async() |

---

## Troubleshooting

### "My game freezes when calling generate()"

→ Use `generate_async()` instead for worlds > 125 chunks

### "Chunks appear slowly in async"

→ This is normal. Chunks appear as workers finish. Speed up by:

- Lowering `resolution` (less detail = faster)
- Increasing `lod_level` (less detail = faster)
- Using HEIGHTMAP_FIRST mode (faster than VOXELS_FIRST)

### "Debug visualization (voxel grid) is wrong size"

→ Debug meshes in `generate()` are created with world_size BEFORE biome expansion
→ Call `generate()` AFTER all biome settings are finalized, or don't use debug meshes with biomes

### "Can I call generate() while generate_async() is running?"

→ No. Both functions check `generation_in_progress` lock:

```cpp
if (generation_in_progress.exchange(true)) {
    log_message("Generation already in progress, aborting", 1);
    return;  // Exits early
}
```

→ Call `cancel_generation()` first if you need to interrupt

### "My mesh data is not visible after generate()"

→ Ensure chunks have meshes applied:

```gdscript
# Check one chunk
var chunk = voxel_gen.chunks[0]
print("Chunk has mesh:", chunk.get_mesh_instance() != null)
```

---

## Summary

| Use Case            | Function                    | Why                         |
| ------------------- | --------------------------- | --------------------------- |
| Tiny test world     | `generate()`                | Sub-100ms startup OK        |
| Editor preview      | `generate()`                | Instant feedback            |
| Game startup        | `generate()`                | Loading screen absorbs wait |
| Production gameplay | `generate_async()`          | Never freeze 60 FPS         |
| Streaming terrain   | `generate_async()`          | Chunks appear progressively |
| Terrain editing     | `regenerate_dirty_chunks()` | Fastest for partial updates |

**Default recommendation:** Use `generate_async()` unless you have a specific reason to block.
