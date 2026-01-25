extends Node

# Terrain registry: maps terrain names to their data
var terrain_registry: Dictionary = {}
var current_terrain_name: String = ""
var current_voxel_generator: VoxelGenerator = null

var player: CharacterBody3D = null
var player_raycast: RayCast3D = null

# Debug output control
@export var verbose: bool = false

# Signals
signal terrain_selected(terrain_name: String, voxel_generator: VoxelGenerator)
# signal terrain_raycast_changed(terrain_name: String)

func _ready() -> void:
	# For autoload, don't setup here - wait for manual initialization
	pass

func initialize() -> void:
	# Setup player and raycast
	setup_player()
	# Register all terrain nodes
	register_terrains()

func setup_player() -> void:
	# Get player and raycast (search relative to this manager's parent / scene root)
	var world = get_parent()
	if not world:
		world = get_tree().get_root().get_child(0) if get_tree().get_root().get_child_count() > 0 else null

	if world:
		player = world.find_child("Picele", true, false) as CharacterBody3D
	if not player:
		push_error("MultiTerrainManager: Player (Picele) not found in scene")
	else:
		player_raycast = player.get_node_or_null("PiceleCamera/PiceleRayCast")
		if not player_raycast:
			push_error("MultiTerrainManager: RayCast3D not found at Picele/PiceleCamera/PiceleRayCast")

func register_terrains() -> void:
	var world = get_parent()
	if not world:
		world = get_tree().get_root().get_child(0) if get_tree().get_root().get_child_count() > 0 else null

	# Find the single VoxelEngine instance (child of World)
	var voxel_engine = null
	if world:
		voxel_engine = world.find_child("VoxelEngine", true, false) as VoxelEngine
	
	if not voxel_engine:
		push_error("[MultiTerrainManager] VoxelEngine not found in World - terrain switching will not work")
		return

	# Register all terrain types (for raycast detection)
	var terrain_names = ["Plains", "Mountains", "Desert", "Forest", "TerrainMultiBiome"]
	for terrain_name in terrain_names:
		var terrain_node = null
		if world:
			terrain_node = world.find_child(terrain_name, true, false)
		if terrain_node:
			# Just store the terrain node reference - VoxelEngine handles generator creation
			terrain_registry[terrain_name] = {
				"node": terrain_node,
				"voxel_engine": voxel_engine # Single shared VoxelEngine for all terrains
			}
			print("[MultiTerrainManager] Registered terrain: %s" % terrain_name)
		else:
			if verbose:
				print("[MultiTerrainManager] Terrain node '%s' not found (skipping)" % terrain_name)

	# Set first terrain as default and load it
	if not terrain_registry.is_empty():
		var keys = terrain_registry.keys()
		current_terrain_name = keys[0]
		# Load the first terrain (this creates the VoxelGenerator)
		current_voxel_generator = voxel_engine.load_terrain(current_terrain_name)
		print("[MultiTerrainManager] Initialized with terrain: %s" % current_terrain_name)
		# Emit initial selection so listeners (GUI, etc.) can bind to the current generator
		if current_voxel_generator:
			emit_signal("terrain_selected", current_terrain_name, current_voxel_generator)

func get_terrain_from_raycast() -> String:
	if not player_raycast:
		return current_terrain_name

	player_raycast.force_raycast_update()

	if player_raycast.is_colliding():
		var collider = player_raycast.get_collider()
		if collider:
			# Find which terrain node this collider belongs to
			var current = collider
			while current:
				if terrain_registry.has(current.name):
					return current.name
				current = current.get_parent()

	return current_terrain_name

func switch_to_terrain(terrain_name: String) -> void:
	if not terrain_registry.has(terrain_name):
		push_error("Terrain '%s' not registered" % terrain_name)
		return

	if terrain_name == current_terrain_name:
		return

	current_terrain_name = terrain_name
	var terrain_data = terrain_registry[terrain_name]
	
	# Load terrain using the single VoxelEngine instance
	if terrain_data.has("voxel_engine") and terrain_data["voxel_engine"] != null:
		var voxel_engine = terrain_data["voxel_engine"] as VoxelEngine
		if voxel_engine:
			# load_terrain() handles destroying old generator and creating a new one
			current_voxel_generator = voxel_engine.load_terrain(terrain_name)
	
	print("[MultiTerrainManager] Switched to terrain: %s" % current_terrain_name)
	emit_signal("terrain_selected", current_terrain_name, current_voxel_generator)

func get_all_voxel_generators() -> Array[VoxelGenerator]:
	var generators: Array[VoxelGenerator] = []
	for terrain_data in terrain_registry.values():
		generators.append(terrain_data["voxel_gen"])
	return generators

func get_raycast_info() -> Dictionary:
	if not player_raycast:
		return {}

	player_raycast.force_raycast_update()

	if player_raycast.is_colliding():
		var hit_pos = player_raycast.get_collision_point()
		var distance = 0.0
		if player:
			distance = hit_pos.distance_to(player.global_position)
		return {
			"hit": true,
			"position": hit_pos,
			"distance": distance,
			"collider": player_raycast.get_collider(),
			"normal": player_raycast.get_collision_normal()
		}

	return {"hit": false}
