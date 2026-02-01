# Testing Guide - Voxel Terrain Generation Fix

## Pre-Testing Checklist

- [x] Code compiled successfully
- [x] DLL copied to `demo/bin/windows/`
- [x] Godot 4.5+ installed
- [x] Demo project ready to open

---

## Test Procedure

### 1. Open Godot and Load Demo

```bash
# From command line
& 'C:\path\to\Godot\Godot_v4.5-stable_mono_win64.exe' .\demo
```

### 2. Verify Extension Loaded

In Godot console, you should see:

```
[VoxelGenerator] VoxelGenerator is ready and auto generation is enabled. Voxel grid generated.
```

---

## Test Case 1: Heightmap-First Mode (FIXED)

### Setup

1. Open **Scene** → `demo/world.tscn`
2. Select **VoxelGenerator** node in tree
3. In **Inspector**, find **VoxelGenerator** section
4. Set **Generation Mode** → `Heightmap First` (value: 1)
5. Verify these settings:
   - **World Size**: Vector3i(5, 3, 5)
   - **Surface Band**: 4.0
   - **Chunk Size**: 16
   - **Resolution**: 1

### Generate Terrain

- Press **F5** to open debug menu
- Click **Generate Async** button
- Wait for generation to complete (watch console for progress)

### Inspect Results

- ✅ Terrain should appear **complete** with no gaps
- ✅ Surface should be **smooth** and **continuous**
- ✅ Check FPS in top-left corner

**EXPECTED**: Full terrain mesh without holes or missing columns

**BUGGY BEHAVIOR (before fix)**: Scattered gaps where Y-index boundaries misaligned

---

## Test Case 2: Voxels-First Mode (Baseline)

### Setup

1. Select **VoxelGenerator** node
2. In **Inspector**:
   - Set **Generation Mode** → `Voxels First` (value: 0)
   - Keep all other settings same as Test Case 1

### Generate Terrain

- Press **F5** to open debug menu
- Click **Reset** or **Generate Async**
- Wait for generation (slower than heightmap-first - will process full 3D volume)

### Inspect Results

- ✅ Should produce complete terrain
- ✅ Note the generation time (slower due to full 3D sampling)

**EXPECTED**: Full terrain mesh (baseline reference)

---

## Test Case 3: Visual Comparison

### Side-by-Side Testing

1. **Generate Heightmap-First** as per Test Case 1
2. Take screenshot or note visual appearance
3. **Switch to Voxels-First** as per Test Case 2
4. Compare terrain appearance

**EXPECTED AFTER FIX**:

- Heightmap-first should look very similar to voxels-first
- No visible gaps or holes in either mode
- Heightmap-first should be significantly faster

**BUGGY BEHAVIOR (before fix)**:

- Heightmap-first would have gaps
- Voxels-first would appear complete
- Visual discrepancy obvious when compared

---

## Test Case 4: Debug Visualization

### Enable Debug Grids

1. Select **VoxelGenerator** node
2. In **Inspector**:
   - ✓ Enable **Show Voxel Grids** (checkbox)
   - ✓ Enable **Show Chunk Grids** (checkbox)

### Regenerate with Debug Visualization

- Press **F5** → **Generate Async**
- Observe wireframe overlay

**EXPECTED**:

- Wireframe cubes show voxel sampling grid
- Grid should align with terrain surface
- Y-coordinates should match terrain height consistently

**BUGGY BEHAVIOR (before fix)**:

- Grid misalignment visible at surface band edges
- Asymmetric grid pattern showing truncation issues

---

## Test Case 5: Performance Benchmark

### Heightmap-First Performance

1. Set **Generation Mode** → Heightmap First
2. Set **World Size** → Vector3i(10, 5, 10) (larger terrain)
3. Set **Resolution** → 2
4. Click **Generate Async**
5. Note FPS and generation time in console

### Voxels-First Performance

1. Set **Generation Mode** → Voxels First
2. Keep all other settings same
3. Click **Generate Async**
4. Note FPS and generation time in console

**EXPECTED**:

- Heightmap-First: ~1-3 seconds for 10x5x10 world at resolution 2
- Voxels-First: ~5-10 seconds for same world (full 3D sampling)
- Both should maintain stable FPS during generation

---

## Troubleshooting

### Issue: "DLL not found" or "Failed to load extension"

**Solution**:

1. Verify DLL exists: `demo/bin/windows/voxel-engine-gd.windows.template_debug.x86_64.dll`
2. Check Godot console for detailed error
3. Rebuild if needed: `scons -j12 target=template_debug debug_symbols=yes platform=windows`
4. Recopy DLL to demo folder

### Issue: Terrain still has gaps in heightmap-first mode

**Possible Causes**:

1. DLL not reloaded - Close Godot completely and reopen (hard refresh)
2. Compiled without fix - Verify line 1841 has `std::floor()` in code
3. Wrong DLL - Check timestamp of `demo/bin/windows/voxel-engine-gd*.dll`

**Debug Steps**:

1. Open `src/VoxelGenerator.cpp`
2. Go to line 1841
3. Verify code reads:
   ```cpp
   int iy_min = std::max(0, static_cast<int>(std::floor((y_min_world + physical_extent.y * 0.5f) / eff_voxel_size.y)));
   ```
4. If missing `std::floor()`, code wasn't fixed - rebuild

### Issue: Generation is very slow

**Expected for certain settings**:

- Voxels-First mode is slower than Heightmap-First (full 3D)
- High resolution (>3) generates many more vertices
- Large world sizes (>5x5x5) take longer

**Optimization**:

- Use Heightmap-First mode for most terrain
- Keep resolution ≤ 2 for good balance
- Use LOD (Level of Detail) for large worlds

---

## Success Criteria

✅ **Fix Verified If**:

1. Heightmap-First generates complete terrain (no gaps)
2. Heightmap-First and Voxels-First produce visually similar results
3. Heightmap-First is noticeably faster than Voxels-First
4. Debug grids show consistent Y-coordinate alignment
5. Performance is stable across both modes

---

## Console Output Examples

### Successful Heightmap-First Generation

```
[VoxelGenerator] Starting HEIGHTMAP_FIRST generation mode
[VoxelGenerator] Building heightmap cache: 80x80 = 6400 entries
[VoxelGenerator] Heightmap cache built successfully
[VoxelGenerator] Building density cache: 81x49x81 = 320049 entries
[VoxelGenerator] Density cache built successfully
[VoxelGenerator] HEIGHTMAP_FIRST completed: 1234 triangles, 3702 vertices, 85% voxels skipped
[VoxelGenerator] Voxel grid (HEIGHTMAP): 4860 vertices, 95% voxels skipped due to surface band
[VoxelGenerator] generate() completed
```

### Successful Voxels-First Generation

```
[VoxelGenerator] Starting VOXELS_FIRST generation mode
[VoxelGenerator] Building density cache: 81x49x81 = 320049 entries
[VoxelGenerator] Building density cache complete
[VoxelGenerator] VOXELS_FIRST completed: 2456 triangles, 7368 vertices
[VoxelGenerator] Voxel grid (VOXELS_FIRST): 19680 vertices
[VoxelGenerator] generate() completed
```

---

## Quick Reference: Generation Mode Settings

### Heightmap-First (OPTIMIZED)

```
Generation Mode: 1
Surface Band: 4.0 (vertical range around terrain height)
LOD Level: 7 (full details within band)
Heightmap Vertex Limit: 268435456 (high budget for band only)
Expected: Fast, complete, efficient
```

### Voxels-First (THOROUGH)

```
Generation Mode: 0
LOD Level: 7 (full details)
Vertex Limit: 65536 (standard budget)
Expected: Slower, complete, full 3D details
```

---

## Questions to Answer During Testing

1. **Does heightmap-first terrain appear complete?** (Yes = Fix Working)
2. **Are there any visual gaps or holes?** (None = Fix Verified)
3. **Does heightmap-first generate faster than voxels-first?** (Yes = Optimization Works)
4. **Do both modes show consistent terrain height?** (Yes = Fix Correct)
5. **Is FPS stable during generation?** (Yes = Performance Good)

---

## Next Steps After Testing

- ✅ If all tests pass → Fix verified, ready for production
- ❌ If tests fail → Check troubleshooting steps above
- 📝 Document any issues or unexpected behavior
- 🔄 If needed, rebuild and retest
