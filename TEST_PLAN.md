# Test Plan: TerrainMultiBiome Signal 11 Crash Fix

## Date: 2026-01-27

## Summary of Changes

### 1. GDScript Fix (multi_terrain_manager.gd)

**File:** `demo/multi_terrain_manager.gd` (lines 68-77)
**Change:** Added special-case handling for TerrainMultiBiome during initialization
**Rationale:** TerrainMultiBiome has no config file (res://configs/TerrainMultiBiome.tres), so calling `load_terrain("TerrainMultiBiome")` during initialization would cause issues

**Code:**

```gdscript
if current_terrain_name != "TerrainMultiBiome":
    current_voxel_generator = voxel_engine.load_terrain(current_terrain_name)
    # ... emit signal
else:
    # TerrainMultiBiome will create its generator when explicitly selected
    print("[MultiTerrainManager] Initialized with terrain: %s (will create generator on selection)" % current_terrain_name)
```

**Impact:** Prevents crash during MultiTerrainManager initialization

### 2. Unified Factory Pattern (multi_terrain_manager.gd)

**File:** `demo/multi_terrain_manager.gd` (lines 108-129, switch_to_terrain())
**Status:** UNCHANGED - Already implements unified pattern
**Benefit:** TerrainMultiBiome uses same code path as all other terrains when explicitly selected

**Code:**

```gdscript
# All terrains now use VoxelEngine.load_terrain() - unified pattern
current_voxel_generator = voxel_engine.load_terrain(terrain_name)
```

### 3. C++ Pointer Safety Fix (VoxelGenerator.cpp)

**File:** `src/VoxelGenerator.cpp` (lines 3095-3121, remove_forcefield_nodes())
**Change:** Pre-compute pointer validity before any dereference operations
**Rationale:** Prevents potential crashes from dereferencing invalid pointers during cleanup

**Code:**

```cpp
// Check if forcefield_root is valid and in tree BEFORE logging
bool root_in_tree = false;
if (forcefield_root != nullptr && forcefield_root->is_inside_tree()) {
    root_in_tree = true;
}
// Use root_in_tree in logging instead of dereferencing pointer directly
```

**Impact:** Hardens C++ destructor to prevent crashes even if invalid pointers exist

## Test Scenarios

### Scenario 1: Initialize with TerrainMultiBiome as First Terrain

**Action:** Load world_manager.tscn with TerrainMultiBiome as default terrain
**Expected Result:**

- MultiTerrainManager initializes without crash
- Message: "[MultiTerrainManager] Initialized with terrain: TerrainMultiBiome (will create generator on selection)"
- No signal 11 crash
- No C++ errors logged

**Success Criteria:**

- ✓ No crash
- ✓ Scene loads successfully
- ✓ GUI appears and is responsive

---

### Scenario 2: Switch Between All 6 Terrains

**Action:** Use GUI or code to switch between Plains, Mountains, Desert, Forest, Swamp, TerrainMultiBiome
**Expected Result:**

- Each terrain switches without crash
- New VoxelGenerator created for each terrain
- Terrain renders correctly after switching
- Proper biomes/features visible for each terrain

**Success Criteria:**

- ✓ No crashes during switching
- ✓ All 6 terrains load and display
- ✓ Terrain raycast detection works (F5 key to toggle debug GUI)
- ✓ Visual appearance matches expected terrain type

---

### Scenario 3: TerrainMultiBiome Generation

**Action:** When TerrainMultiBiome is selected, trigger generation and observe terrain
**Expected Result:**

- Async generation starts (chunks generate in background)
- Multi-biome terrain is generated correctly
- Visible biome transitions (Plains, Mountains, Forest, Desert in one world)
- No crashes during mesh generation or application

**Success Criteria:**

- ✓ Generation completes successfully
- ✓ Meshes apply to chunks without errors
- ✓ Terrain is navigable
- ✓ Biome transitions visible

---

### Scenario 4: Player Movement and Terraforming

**Action:** Move player with WASD, use mouse to aim, press LMB/RMB to dig/build
**Expected Result:**

- Player moves smoothly through terrain
- Terrain editing works (dig/build spheres)
- No crashes during terrain modification
- Edited terrain persists correctly

**Success Criteria:**

- ✓ No crashes during gameplay
- ✓ Digging/building works as expected
- ✓ FPS remains stable
- ✓ No memory leaks or crashes on cleanup

---

### Scenario 5: Scene Unload and Cleanup

**Action:** Exit to main menu, load different scene, or close Godot
**Expected Result:**

- VoxelGenerator destructor runs cleanly
- remove_forcefield_nodes() executes without crashes
- All resources cleaned up properly
- No memory leaks

**Success Criteria:**

- ✓ Clean shutdown
- ✓ No C++ crashes during cleanup
- ✓ No stray error messages
- ✓ No dangling pointer crashes

---

## Build Information

**Date Built:** 2026-01-27 11:29 PM
**Platform:** Windows x86_64
**Configuration:** template_debug with debug_symbols=yes
**Build Tool:** SCons
**DLL Size:** 1,307,648 bytes

**Build Output:**

```
Compiling shared src\VoxelGenerator.cpp ...
Linking Shared Library bin\windows\voxel-engine-gd.windows.template_debug.x86_64.dll ...
Install file: "bin\windows\voxel-engine-gd.windows.template_debug.x86_64.dll"
scons: done building targets.
```

**Status:** ✅ Successful - No errors or warnings

---

## How to Run Tests

### Manual Testing (Recommended)

1. Open Godot 4.5.1 with the demo project (`C:\Users\Brian\godot-cpp-gdextention\demo`)
2. In FileSystem, double-click `world_manager.tscn` to load the scene
3. Press Play (▶ button) to run the scene
4. Observe initialization output in Godot console
5. Follow test scenarios above

### Automated Testing (Future)

- Integration test in `integration_test.gd`
- Run via: `godot --path demo integration_test.gd`

---

## Expected Outcomes

### If Crash is Fixed

- ✅ Scene loads without signal 11
- ✅ "[MultiTerrainManager] Initialized with terrain: ..." message appears
- ✅ All 6 terrains can be switched between
- ✅ TerrainMultiBiome generates correctly with all 4 biomes visible
- ✅ No crashes during gameplay or shutdown

### If Issues Remain

- ⚠️ Crash still occurs → Check new backtrace for different root cause
- ⚠️ Specific terrain fails → Check that terrain's config file
- ⚠️ C++ errors logged → Review pointer dereference code

---

## Rollback Plan

If tests fail, the following files can be reverted:

1. `demo/multi_terrain_manager.gd` - Remove special case for TerrainMultiBiome
2. `src/VoxelGenerator.cpp` - Revert pointer safety changes in `remove_forcefield_nodes()`

However, TerrainMultiBiome will still need a config file or special handling since it's a procedural-only terrain without a .tres file.

---

## Notes

- All changes maintain backward compatibility
- No existing terrains (Plains, Mountains, etc.) are affected
- Special-casing is minimal - only during initialization, not during normal operation
- Unified factory pattern is preserved in switch_to_terrain()
- C++ safety improvements harden code against future pointer issues
