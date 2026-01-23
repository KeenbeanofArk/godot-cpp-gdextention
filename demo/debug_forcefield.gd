extends Node3D
## Debug script to diagnose forcefield detection issues

#@export var voxel_generator: VoxelGenerator = null
@onready var voxel_generator: VoxelGenerator = $"../VoxelEngine/VoxelGenerator"
@export var verbose: bool = true

func _ready():
	if not voxel_generator:
		push_error("DebugForcefield: VoxelGenerator not assigned")
		return
	
	# Schedule a delayed check to ensure forcefield is fully initialized
	await get_tree().create_timer(1.0).timeout
	debug_print_forcefield_state()

func _process(_delta):
	if Input.is_action_just_pressed("ui_accept"):
		debug_print_forcefield_state()

func debug_print_forcefield_state():
	if not voxel_generator:
		print("[DebugForcefield] No VoxelGenerator assigned")
		return
	
	print("\n" + "=".repeat(60))
	print("FORCEFIELD DEBUG STATE")
	print("=".repeat(60))
	
	# Check basic forcefield state
	var enabled = voxel_generator.forcefield_enabled
	var detection_enabled = voxel_generator.forcefield_detection_enabled
	var collision_enabled = voxel_generator.forcefield_collision_enabled
	
	print("Forcefield Enabled: %s" % enabled)
	print("Detection Enabled: %s" % detection_enabled)
	print("Collision Enabled: %s" % collision_enabled)
	print("Height: %.2f" % voxel_generator.forcefield_height)
	print("Buffer: %.2f" % voxel_generator.forcefield_buffer)
	print("Detection Voxels: %.2f" % voxel_generator.forcefield_detection_voxels)
	
	# Find the forcefield root
	var root = voxel_generator.find_child("ForcefieldRoot")
	if not root:
		# Try asking the generator to (re)create forcefield nodes, then re-check briefly
		print("\n[WARN] ForcefieldRoot not found, requesting creation and retrying...")
		if voxel_generator.has_method("create_forcefield_nodes"):
			voxel_generator.create_forcefield_nodes()
			# allow a short time for nodes to be added to the scene tree
			await get_tree().create_timer(0.05).timeout
			root = voxel_generator.find_child("ForcefieldRoot")
		if not root:
			print("\n[ERROR] ForcefieldRoot not found after create_forcefield_nodes()")
			return
	
	print("\nForcefieldRoot found: %s (in_tree: %s)" % [root.name, root.is_inside_tree()])
	print("ForcefieldRoot children: %d" % root.get_child_count())
	
	# Check each wall
	var wall_names = ["NORTH", "SOUTH", "EAST", "WEST", "TOP", "BOTTOM"]
	for i in range(6):
		print("\n--- Wall %d (%s) ---" % [i, wall_names[i]])
		var wall_name = "ForcefieldWall_%d" % i
		var mi = root.find_child(wall_name)
		if mi:
			print("  Visual Mesh: Found ✓")
			print("    Visible: %s" % mi.visible)
			print("    Position: %s" % mi.position)
			print("    Mesh: %s" % mi.mesh)
		else:
			print("  Visual Mesh: NOT FOUND ✗")
		
		# Find Area3D
		var area_name = "ForcefieldArea_%d" % i
		var area = root.find_child(area_name)
		if area:
			print("  Detection Area: Found ✓")
			print("    Position: %s" % area.position)
			print("    Monitoring: %s" % area.is_monitoring())
			print("    Monitorable: %s" % area.is_monitorable())
			print("    Children: %d" % area.get_child_count())
			
			# Check collision shape
			if area.get_child_count() > 0:
				var cs = area.get_child(0)
				if cs is CollisionShape3D:
					print("    CollisionShape3D: Found ✓")
					print("      Position: %s" % cs.position)
					print("      Disabled: %s" % cs.disabled)
					print("      Shape: %s" % cs.shape)
					if cs.shape:
						print("      Shape Size: %s" % cs.shape.size)
				else:
					print("    CollisionShape3D: NOT FOUND ✗")
			else:
				print("    CollisionShape3D: NO CHILDREN ✗")
		else:
			print("  Detection Area: NOT FOUND ✗")
	
	print("\n" + "=".repeat(60))
	print("CONNECTED SIGNALS")
	print("=".repeat(60))
	
	# Check if signals are connected
	for i in range(6):
		var area_name = "ForcefieldArea_%d" % i
		var area = root.find_child(area_name)
		if area:
			var connections = area.get_signal_connection_list("body_entered")
			print("Area %d body_entered connections: %d" % [i, connections.size()])
			for conn in connections:
				print("  -> %s::%s" % [conn.signal.get_object().name, conn.signal.get_name()])
	
	print("\n" + "=".repeat(60))
