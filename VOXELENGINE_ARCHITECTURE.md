# VoxelEngine Architecture - Clarification

## Relationship Between VoxelEngine and VoxelGenerator

### The Key Distinction

| Component          | Type           | Role                                                                | Lifecycle                              |
| ------------------ | -------------- | ------------------------------------------------------------------- | -------------------------------------- |
| **VoxelEngine**    | C++ Node class | **Factory & Manager** - Creates and owns VoxelGenerators            | Should be instantiated in scene        |
| **VoxelGenerator** | C++ Node class | **Terrain Generator** - Generates terrain meshes via marching cubes | Created by VoxelEngine, added as child |

---

## Current Architecture (After Restructuring)

### ❌ INCORRECT (What we did)

```
World (Node3D)
├── VoxelGenerator ← DIRECTLY in scene
│   └── (no parent VoxelEngine to manage it)
├── Plains
├── Mountains
└── ...
```

### ✅ CORRECT (What should be)

```
World (Node3D)
├── VoxelEngine ← Factory/Manager (should be in scene)
│   └── VoxelGenerator ← Created and owned by VoxelEngine
├── Plains
├── Mountains
└── ...
```

---

## How It Works

### VoxelEngine Factory Method

From `VoxelEngine.cpp`:

```cpp
VoxelGenerator *VoxelEngine::create_generator() {
    if (voxel_generator != nullptr) {
        destroy_generator(); // Clean up old one
    }

    // Create new VoxelGenerator
    voxel_generator = memnew(VoxelGenerator);
    voxel_generator->set_name("VoxelGenerator");

    // Add as CHILD to VoxelEngine
    add_child(voxel_generator);

    return voxel_generator;
}
```

### Key Points

1. **VoxelEngine owns the VoxelGenerator**
   - VoxelEngine is the parent Node
   - VoxelGenerator is a child Node
   - Memory is managed by VoxelEngine

2. **VoxelEngine.load_terrain()** does the full setup

   ```cpp
   VoxelGenerator *gen = create_generator(); // Creates and adds as child
   config_resource->apply_to_voxel_generator(gen); // Configure it
   ```

3. **Cleanup is handled by VoxelEngine**
   - When VoxelEngine is deleted → destroy_generator() is called
   - This cancels any ongoing generation
   - Removes VoxelGenerator from scene tree
   - Deletes the generator node

---

## What Needs to Change in world.tscn

### Current (Wrong)

```gdscene
[node name="VoxelGenerator" type="VoxelGenerator" parent="."]
visible = false
```

### Should Be

```gdscene
[node name="VoxelEngine" type="VoxelEngine" parent="."]

[node name="VoxelGenerator" type="VoxelGenerator" parent="VoxelEngine"]
visible = false
```

**OR** (Better) - Don't put VoxelGenerator in scene at all:

```gdscene
[node name="VoxelEngine" type="VoxelEngine" parent="."]

# VoxelGenerator will be created dynamically by VoxelEngine.create_generator()
```

---

## Recommended Architecture for Multi-Terrain

### Scene Hierarchy

```
World
├── VoxelEngine (auto-creates VoxelGenerator on demand)
│   └── VoxelGenerator (dynamically created)
├── Plains
├── Mountains
├── Desert
├── Forest
├── TerrainMultiBiome
├── GUI
├── Player
└── Camera
```

### GDScript Integration (terrain_plains.gd example)

```gdscript
extends Node3D

@onready var voxel_engine = get_node_or_null("/root/World/VoxelEngine")

func _ready():
    var mtm = get_node_or_null("/root/MultiTerrainManager")
    if mtm:
        mtm.connect("terrain_selected", Callable(self, "_on_terrain_selected"))

func _on_terrain_selected(name: String, voxel_gen: VoxelGenerator) -> void:
    if name == "Plains":
        visible = true
        # VoxelGenerator already created and configured by VoxelEngine
    else:
        visible = false
```

---

## Updated Integration Flow

```
1. Scene loads world.tscn
   └── VoxelEngine node created in World

2. MultiTerrainManager initializes (autoload)
   └── Stores reference to VoxelEngine

3. Terrain selection happens (e.g., player enters Plains)
   └── MultiTerrainManager gets terrain config
   └── VoxelEngine.load_terrain("Plains")
      └── VoxelEngine.create_generator()
         └── Creates VoxelGenerator as child of VoxelEngine
      └── Applies config to VoxelGenerator
      └── Returns VoxelGenerator reference

4. Terrain script receives VoxelGenerator via signal
   └── Terrain becomes visible
   └── Uses the VoxelGenerator for rendering
```

---

## Action Items

1. **Update world.tscn**
   - Remove VoxelGenerator node from root
   - Add VoxelEngine node to root instead
   - OR: Put only VoxelEngine in scene, let it create VoxelGenerator dynamically

2. **Update MultiTerrainManager integration**
   - Reference VoxelEngine instead of storing generators in registry
   - Call `voxel_engine.load_terrain(terrain_name)`
   - Distribute returned VoxelGenerator to terrain scripts via signal

3. **Update terrain scripts**
   - Get VoxelGenerator from signal, not from scene
   - Don't try to get VoxelGenerator from scene hierarchy

---

## Benefits of This Approach

✅ **Proper Ownership** - VoxelEngine owns VoxelGenerator  
✅ **Clear Lifecycle** - VoxelGenerator deleted when VoxelEngine deleted  
✅ **Single Responsibility** - VoxelEngine = Factory, VoxelGenerator = Terrain  
✅ **Memory Safe** - No manual memory management needed  
✅ **Reloadable** - Can create/destroy generators on terrain switch  
✅ **Follows Godot Patterns** - Parent-child node ownership

---

## Summary

**The VoxelEngine should be in world.tscn at the root level, and it creates/manages the VoxelGenerator as its child.**

Currently we have the VoxelGenerator in the scene directly, which skips the factory pattern. We need to:

1. Place **VoxelEngine** in world.tscn
2. Let it **create VoxelGenerator dynamically** via `load_terrain()`
3. **Pass VoxelGenerator to terrains** via signal injection
4. Keep the **MultiTerrainManager** coordinating the terrain switches
