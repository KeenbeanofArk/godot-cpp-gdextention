# Terrain Generation Modes, LOD System, and Best Practices

This guide explains the two terrain generation modes, the LOD (Level of Detail) system, and how to optimize terrain generation for your game.

## Generation Modes

The VoxelGenerator supports two fundamentally different approaches to terrain generation:

### VOXELS_FIRST (Mode 0) - Full 3D Evaluation

Evaluates density at every corner point in the world for complete 3D terrain generation.

**Characteristics:**

- Generates full 3D density fields
- Supports caves, overhangs, floating islands
- More flexible but slower (2-4× slower than HEIGHTMAP_FIRST)
- Best for caves, complex underground structures

**When to Use:**

- Underground cave systems
- Flying terrain with overhangs
- Complex rock formations
- Single-player games where generation speed is less critical

**Code:**

```gdscript
generator.set_generation_mode(0)  # VOXELS_FIRST
generator.set_terrain_height(10.0)
generator.set_terrain_amplitude(15.0)
generator.generate_async()
```

**Performance:**

- Generation time: ~500ms per chunk (8×8×8 voxels, resolution=2)
- Memory: Higher (full 3D cache)
- Detail: Maximum

---

### HEIGHTMAP_FIRST (Mode 1) - Optimized 2D→3D

Generates a 2D heightmap first, then samples density only in a surface band around it.

**Characteristics:**

- Fast 2D heightmap generation
- Only evaluates density near the surface
- Cannot generate caves above the heightmap
- 2-4× faster than VOXELS_FIRST
- Best for typical terrain (grass, sand, stone)

**When to Use:**

- Surface-focused terrain (mountains, valleys, plains)
- Large worlds that need fast streaming
- Games where performance is critical
- Multiplayer where server-side generation needs speed

**Code:**

```gdscript
generator.set_generation_mode(1)  # HEIGHTMAP_FIRST
generator.set_surface_band(4.0)   # ±4 voxels around surface
generator.set_terrain_height(10.0)
generator.set_terrain_amplitude(15.0)
generator.generate_async()
```

**Surface Band:**
The `surface_band` parameter controls how far above and below the heightmap to sample density:

- Small band (2.0): Very fast, can miss underground features
- Medium band (4.0): Good balance, catches most features
- Large band (8.0): Slower but ensures feature coverage

**Performance:**

- Generation time: ~150ms per chunk (8×8×8 voxels, resolution=2)
- Memory: Lower (2D cache only)
- Detail: Limited to surface band region

---

## Choosing Your Mode

**Quick Decision Tree:**

```
Do you need caves above the surface?
  → YES: Use VOXELS_FIRST
  → NO: Use HEIGHTMAP_FIRST (faster)

Large world (100+ chunks)?
  → YES: Use HEIGHTMAP_FIRST
  → NO: Either mode

Performance critical (mobile, server)?
  → YES: Use HEIGHTMAP_FIRST
  → NO: Either mode
```

---

## LOD (Level of Detail) System

The LOD system reduces mesh quality for distant chunks to improve performance.

### Global LOD Level

Set a single LOD level (0-7) for all chunks:

```gdscript
generator.set_lod_level(0)  # Highest detail, all chunks use LOD 0
generator.set_lod_level(3)  # Medium detail
generator.set_lod_level(7)  # Lowest detail, very fast
```

**LOD Levels:**

- **0** = Highest detail (full resolution)
- **1** = 50% reduction in samples
- **2** = 75% reduction
- **3-7** = Progressive coarsening

**Example - Preview Mode:**

```gdscript
# Quick preview with lower quality
generator.set_lod_level(4)
generator.generate()

# Later: full quality generation
generator.reset()
generator.set_lod_level(0)
generator.generate_async()
```

---

### Distance-Based LOD (Per-Chunk)

Automatically assign different LOD levels to chunks based on distance from player/camera.

**How It Works:**

1. Set reference position (usually player camera)
2. Define distance thresholds for each LOD level
3. Chunks closer to reference use finer LOD
4. Distant chunks use coarser LOD automatically

**Code:**

```gdscript
# Enable distance LOD
generator.set_enable_distance_lod(true)

# Set distance thresholds (8 values, one per LOD 0-7)
var distances = PackedFloat64Array([
	16.0,   # LOD 0: within 16 units = highest detail
	32.0,   # LOD 1: within 32 units
	64.0,   # LOD 2: within 64 units
	128.0,  # LOD 3: within 128 units
	256.0,  # LOD 4: within 256 units
	512.0,  # LOD 5: within 512 units
	1024.0, # LOD 6: within 1024 units
	2048.0  # LOD 7: within 2048 units
])
generator.set_lod_distances(distances)

# In _process(), update reference position
func _process(delta):
	if generator.get_enable_distance_lod():
		generator.set_lod_reference_position(player.global_position)
		var changed = generator.update_chunks_lod()
		if changed > 0:
			print("LOD changed for %d chunks" % changed)
```

**Visualization - LOD Colors:**
Enable LOD visualization to debug LOD assignment:

```gdscript
generator.set_show_lod_colors(true)  # Green=LOD0 (close), Red=LOD7 (far)
generator.generate()
```

**Performance Impact:**

- Near chunks (LOD 0): Full generation cost
- Middle chunks (LOD 2-4): ~25-50% cost
- Far chunks (LOD 7): ~5-10% cost
- **Result:** 2-3× faster overall for large worlds

---

## Resolution & Quality

### Sampling Resolution

`resolution` parameter controls samples per voxel edge:

```gdscript
generator.set_resolution(1)  # Standard marching cubes, 1 sample per corner
generator.set_resolution(2)  # 2×2 samples per edge, smoother
generator.set_resolution(3)  # 3×3 samples per edge, very smooth
```

**Quality vs. Speed:**
| Resolution | Detail | Speed | Use Case |
|-----------|--------|-------|----------|
| 1 | Blocky | Fast | Preview, far LOD |
| 2 | Balanced | 2-4× slower | Most games |
| 3+ | Smooth | 8-16× slower | High-end graphics |

**Recommendation:**

```gdscript
# Good balance
generator.set_resolution(2)

# With LOD: use resolution 1 for distant chunks
# Distance LOD automatically handles this
```

---

## Generating Terrain

### Synchronous (Blocking)

```gdscript
generator.set_world_size(Vector3i(4, 4, 4))  # Small world
generator.generate()  # Blocks until done
print("Ready!")
```

**Use:** Offline tools, single-player, small worlds

### Asynchronous (Non-Blocking)

```gdscript
generator.set_world_size(Vector3i(16, 16, 16))  # Large world

generator.generation_started.connect(_on_gen_started)
generator.generation_progress.connect(_on_gen_progress)
generator.generation_completed.connect(_on_gen_completed)

generator.generate_async()  # Returns immediately
print("Generating in background...")

func _on_gen_started(total):
	print("Starting: %d chunks" % total)

func _on_gen_progress(done, total):
	print("Progress: %d/%d" % [done, total])

func _on_gen_completed():
	print("Terrain ready!")
```

**Tuning Async:**

```gdscript
# More chunks per frame = faster, more stuttering
generator.set_max_chunks_per_frame(8)

# Fewer chunks per frame = slower, smoother
generator.set_max_chunks_per_frame(2)

# Signal frequency (progress updates)
generator.set_signal_every_n_chunks(4)  # Progress every 4 chunks
```

---

## Performance Optimization Checklist

**For Fast Generation:**

```gdscript
# 1. Use HEIGHTMAP_FIRST mode
generator.set_generation_mode(1)

# 2. Reduce resolution
generator.set_resolution(1)

# 3. Use distance LOD for large worlds
generator.set_enable_distance_lod(true)
generator.set_lod_level(0)

# 4. Reasonable world size
generator.set_world_size(Vector3i(8, 8, 8))  # Not too large

# 5. Tune async settings
generator.set_max_chunks_per_frame(6)
generator.set_signal_every_n_chunks(8)
```

**For Large Worlds (100+ chunks):**

```gdscript
# Stream chunks with distance LOD
generator.set_enable_distance_lod(true)

# Regenerate only chunks that need it
generator.mark_chunk_dirty(chunk_coord)
generator.regenerate_dirty_chunks()  # Incremental update

# Set appropriate LOD distances
generator.set_lod_distances(my_lod_array)
```

**For High Quality (Single-Player):**

```gdscript
# Use VOXELS_FIRST for caves/detail
generator.set_generation_mode(0)

# Higher resolution
generator.set_resolution(2)

# More octaves on noise
var noise = NoiseGenerator.new()
noise.set_octaves(8)
generator.set_terrain_noise(noise)
```

---

## Common Patterns

### Pattern 1: Fast Preview Then Quality

```gdscript
func _ready():
	# Show quick preview
	generator.set_lod_level(5)
	generator.set_resolution(1)
	generator.generate_async()
	show_loading_screen()

func _on_generation_completed():
	hide_loading_screen()

	# Now start quality generation
	generator.set_lod_level(0)
	generator.set_resolution(2)
	generator.mark_all_chunks_dirty()
	generator.regenerate_dirty_chunks()
```

### Pattern 2: Adaptive LOD Based on FPS

```gdscript
var frame_time = 0.0

func _process(delta):
	frame_time += delta

	if frame_time > 0.1:  # Every 100ms
		var fps = Engine.get_frames_per_second()
		if fps < 30:
			# Struggling, increase LOD (coarser)
			generator.set_lod_level(mini(generator.get_lod_level() + 1, 7))
		elif fps > 60:
			# Good performance, reduce LOD (finer)
			generator.set_lod_level(maxi(generator.get_lod_level() - 1, 0))

		frame_time = 0.0
```

### Pattern 3: Streaming Large Chunks

```gdscript
# Configuration for large world
generator.set_world_size(Vector3i(32, 16, 32))  # 16,384 chunks
generator.set_enable_distance_lod(true)

func _process(delta):
	# Update LOD every frame based on player position
	generator.set_lod_reference_position(player.global_position)

	var chunks_changed = generator.update_chunks_lod()
	if chunks_changed > 0:
		# Only regenerate chunks that changed LOD
		generator.regenerate_dirty_chunks()
```

---

## Troubleshooting

**Terrain generation is slow:**

- Switch to HEIGHTMAP_FIRST mode
- Reduce resolution from 2 to 1
- Enable distance LOD for large worlds
- Reduce world_size

**Terrain looks wrong:**

- Check noise.set_octaves() (too few = smooth, too many = rough)
- Check terrain_height and terrain_amplitude ranges
- Visualize noise: `generator.set_visualize_noise_values(true)`

**Stuttering during async generation:**

- Reduce max_chunks_per_frame from 8 to 2-4
- Use distance LOD to prioritize nearby chunks
- Increase signal_every_n_chunks to reduce signal overhead

**Can't see terrain in distance:**

- Enable distance LOD: `generator.set_enable_distance_lod(true)`
- Check LOD distances are reasonable
- Test with `generator.set_show_lod_colors(true)`

---

## Next Steps

- Read [Biome System](BiomeSystem.md) for multi-terrain biomes
- Read [Async Generation](AsyncGeneration.md) for streaming strategies
- Read [Performance](Performance.md) for benchmarking and profiling
