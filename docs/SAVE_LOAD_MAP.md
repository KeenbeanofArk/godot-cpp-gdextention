# Save / Load Map

This document explains how the VoxelGenerator save/load system works and how to use it from GDScript.

Overview

- Per-chunk binary files are saved into a map directory: `<base_dir>/<map_name>/chunk_x_y_z.meshbin`.
- A `metadata.json` file is written alongside the chunk files with generator parameters and terrain edits.
- Saving is performed by the main thread snapshotting mesh arrays and enqueueing `MeshWriteJob`s for a background writer thread which writes `.meshbin` files using standard file I/O.

Files and format

- `metadata.json`: JSON dictionary containing generator parameters, `terrain_edits`, `feature_edits`, and `chunks` (array of `{"coord": [x,y,z], "file": "chunk_x_y_z.meshbin"}`).
- `.meshbin` (custom binary): header `VCHN` then int32 version, int32 vertex_count, vertex float data (x,y,z floats), int32 normal_count, normal float data, int32 color_byte_count, color bytes (RGBA bytes), int32 custom0_byte_count, custom0 bytes.

Usage (GDScript)

Save a map (example):

```gdscript
var vg: VoxelGenerator = $VoxelGenerator
var map_name = "my_saved_map"
vg.save_map("user://saved_maps", map_name)
# This will create folder user://saved_maps/my_saved_map and write chunk files + metadata.json
```

Load a map (example):

```gdscript
var vg: VoxelGenerator = $VoxelGenerator
vg.load_map("user://saved_maps", "my_saved_map", true)
# The loader reads metadata.json, restores terrain edits, and applies chunk mesh data to existing chunk nodes.
```

Notes

- The background writer uses plain file I/O and does not touch Godot resources or the scene tree; the main thread constructs the Packed arrays and applies them to chunks.
- Terrain edits (`terrain_edits` and `feature_edits`) are serialized under mutex locks to ensure consistency.
- The loader expects chunk nodes to exist (matching world size) and applies saved mesh arrays to chunks by coordinate.

Troubleshooting

- If no chunk meshes appear after loading, ensure the `world_size` and `chunk_size` match the saved map's layout.
- Check `user://saved_maps/<map_name>/metadata.json` for the `chunks` array and file names.

Contact
For issues, open an issue in the repository with reproduction steps and the `metadata.json` file attached.
