extends Node

func _ready() -> void:
	# Wait one frame for the scene to initialize (Godot 4 style)
	await get_tree().process_frame

	var gen = get_tree().root.find_child("VoxelGenerator", true, false)
	if gen == null:
		print("VoxelGenerator not found (auto-run)")
		queue_free()
		return

	# Enable dump and set the chunk coord to inspect (center chunk)
	# Use exposed setters rather than direct property assignment
	gen.set_debug_dump_chunk_enabled(true)
	var center_chunk = Vector3i(int(gen.world_size.x / 2), int(gen.world_size.y / 2), int(gen.world_size.z / 2))
	gen.set_debug_dump_chunk_coord(center_chunk)

	# Trigger async generation so dumps occur when meshes are applied
	gen.generate_async()
	print("Auto requested dump for chunk: ", center_chunk)
	queue_free()
