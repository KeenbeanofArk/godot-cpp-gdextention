extends Node3D
## Debug script to diagnose forcefield detection issues

@export var voxel_generator: VoxelGenerator = null
@export var verbose: bool = true

func _ready():
	# Defer VoxelGenerator search to ensure scene is fully loaded
	call_deferred("_find_voxel_generator")

func _find_voxel_generator():
	# Try to find VoxelGenerator if not exported
	if not voxel_generator:
		# First try searching from scene root (most reliable)
		var root = get_tree().get_root()
		if root and root.get_child_count() > 0:
			var world = root.get_child(0)
			voxel_generator = world.find_child("VoxelGenerator", true, false) as VoxelGenerator
	
	# Fallback: search parent hierarchy
	if not voxel_generator:
		var parent = get_parent()
		while parent:
			voxel_generator = parent.find_child("VoxelGenerator", true, false) as VoxelGenerator
			if voxel_generator:
				break
			parent = parent.get_parent()
	
	if not voxel_generator:
		if verbose:
			print("[DebugForcefield] VoxelGenerator not found (will retry when needed)")
		return
	
	# Schedule a delayed check to ensure forcefield is fully initialized
	await get_tree().create_timer(1.0).timeout
	debug_print_forcefield_state()

func _process(_delta):
	# F3 for debug print state
	if Input.is_action_just_pressed("debug_forcefield"):
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
	
	# Get the forcefield root - search properly through voxel generator's children
	var root = voxel_generator.find_child("ForcefieldRoot", true, false)
	if not root:
		# Try asking the generator to (re)create forcefield nodes, then re-check briefly
		print("\n[WARN] ForcefieldRoot not found, requesting creation and retrying...")
		if voxel_generator.has_method("create_forcefield_nodes"):
			voxel_generator.create_forcefield_nodes()
			# allow a short time for nodes to be added to the scene tree
			await get_tree().create_timer(0.1).timeout
			root = voxel_generator.find_child("ForcefieldRoot", true, false)
		if not root:
			print("\n[ERROR] ForcefieldRoot not found after create_forcefield_nodes()")
			print("[DEBUG] VoxelGenerator children: %d" % voxel_generator.get_child_count())
			#for i in range(voxel_generator.get_child_count()):
				#var child = voxel_generator.get_child(i)
				#print("  Child %d: %s" % [i, child.name])
			return
	
	print("\nForcefieldRoot found: %s (in_tree: %s)" % [root.name, root.is_inside_tree()])
	print("ForcefieldRoot children: %d" % root.get_child_count())
	
	# Debug: List all children of forcefield root
	print("ForcefieldRoot child list:")
	for i in range(root.get_child_count()):
		var child = root.get_child(i)
		print("  Child %d: %s (type: %s)" % [i, child.name, child.get_class()])
	
	# Check each wall
	var wall_names = ["NORTH", "SOUTH", "EAST", "WEST", "TOP", "BOTTOM"]
	for i in range(6):
		print("\n--- Wall %d (%s) ---" % [i, wall_names[i]])
		var wall_name = "ForcefieldWall_%d" % i
		
		# Debug: Check direct children first
		var mi = null
		for child in root.get_children():
			if child.name == wall_name:
				mi = child
				break
		
		if not mi:
			mi = root.find_child(wall_name, true, false)
		
		if mi:
			print("  Visual Mesh: Found ✓")
			print("    Type: %s" % mi.get_class())
			print("    Visible: %s" % mi.visible)
			print("    Position: %s" % mi.position)
			print("    Mesh: %s" % mi.mesh)
		else:
			print("  Visual Mesh: NOT FOUND ✗")
			print("    [Looking for: '%s']" % wall_name)
		
		# Find Area3D
		var area_name = "ForcefieldArea_%d" % i
		var area = null
		for child in root.get_children():
			if child.name == area_name:
				area = child
				break
		
		if not area:
			area = root.find_child(area_name, true, false)
		
		if area:
			print("  Detection Area: Found ✓")
			print("    Type: %s" % area.get_class())
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
					print("    CollisionShape3D: NOT FOUND ✗ (got %s instead)" % cs.get_class())
			else:
				print("    CollisionShape3D: NO CHILDREN ✗")
		else:
			print("  Detection Area: NOT FOUND ✗")
			print("    [Looking for: '%s']" % area_name)
	
	print("\n" + "=".repeat(60))
	print("CONNECTED SIGNALS")
	print("=".repeat(60))
	
	# Check if signals are connected
	var found_areas = 0
	for i in range(6):
		var area_name = "ForcefieldArea_%d" % i
		var area = null
		for child in root.get_children():
			if child.name == area_name:
				area = child
				break
		
		if not area:
			area = root.find_child(area_name, true, false)
		
		if area:
			found_areas += 1
			var connections = area.get_signal_connection_list("body_entered")
			print("Area %d body_entered connections: %d" % [i, connections.size()])
			for conn in connections:
				print("  -> %s::%s" % [conn.signal.get_object().name, conn.signal.get_name()])
		else:
			print("Area %d: NOT FOUND" % i)
	
	print("Total areas found: %d/6" % found_areas)
	print("\n" + "=".repeat(60))
