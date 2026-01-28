extends Node3D
class_name WorldManager

@onready var plains: Node3D = $Plains
@onready var mountains: Node3D = $Mountains
@onready var desert: Node3D = $Desert
@onready var forest: Node3D = $Forest
@onready var swamp: Node3D = $Swamp
@onready var terrain_multi_biome: Node3D = $TerrainMultiBiome

# World settings
const WORLD_SIZE: int = 15 # In Chunks
const WORLD_DEPTH: int = 10 # In Chunks
const WALL_HEIGHT: float = 100.0

# Currently active terrain name
var active_terrain: String = "Plains"

func _ready():
	# Get the pending terrain from startup state (if any)
	var ss = get_node_or_null("/root/StartupState")
	if ss and ss.has_method("get_pending_terrain"):
		var pending = ss.get_pending_terrain()
		if pending != "":
			active_terrain = pending
	
	# Hide all terrains initially
	_hide_all_terrains()
	
	print("[WorldManager] Active terrain: %s" % active_terrain)
	
	# Initialize MultiTerrainManager - it will handle loading and generating terrain
	var terrain_manager = get_node_or_null("/root/MultiTerrainManager")
	if terrain_manager:
		terrain_manager.initialize()
		
		# Set the active terrain in the manager (this triggers generation via signal)
		if terrain_manager.terrain_registry.has(active_terrain):
			terrain_manager.switch_to_terrain(active_terrain)
		else:
			print("[WorldManager] Terrain '%s' not registered in manager, using first available" % active_terrain)
			# MultiTerrainManager.initialize() already loads the first terrain, so just inform
			active_terrain = terrain_manager.current_terrain_name
		
		# Make the selected terrain visible (critical for rendering)
		_show_selected_terrain()
	else:
		push_error("[WorldManager] MultiTerrainManager not found")

func _hide_all_terrains() -> void:
	if plains:
		plains.visible = false
	if mountains:
		mountains.visible = false
	if desert:
		desert.visible = false
	if forest:
		forest.visible = false
	if terrain_multi_biome:
		terrain_multi_biome.visible = false

func _show_selected_terrain() -> void:
	"""Show the currently selected terrain node"""
	match active_terrain:
		"Plains":
			if plains:
				plains.visible = true
		"Mountains":
			if mountains:
				mountains.visible = true
		"Desert":
			if desert:
				desert.visible = true
		"Forest":
			if forest:
				forest.visible = true
		"Swamp":
			if swamp:
				swamp.visible = true
		"TerrainMultiBiome":
			if terrain_multi_biome:
				terrain_multi_biome.visible = true
		_:
			push_error("[WorldManager] Unknown terrain: %s" % active_terrain)
	
	print("[WorldManager] Showing terrain: %s" % active_terrain)
	
func _on_voxel_generator_forcefield_body_entered(_wall_index: int, body: Object) -> void:
	# Get current voxel generator from terrain manager
	var terrain_manager = get_node_or_null("/root/MultiTerrainManager")
	if terrain_manager and terrain_manager.current_voxel_generator:
		terrain_manager.current_voxel_generator.set_forcefield_shader_param("base_alpha", 0.85)

	if body is CharacterBody3D:
		# example: give feedback, disable movement, etc.
		if body.has_method("apply_central_impulse"):
			body.apply_central_impulse(Vector3(0, 5, 0))
			
func _on_voxel_generator_forcefield_body_exited(_wall_index: int, _body: Object) -> void:
	# Get current voxel generator from terrain manager
	var terrain_manager = get_node_or_null("/root/MultiTerrainManager")
	if terrain_manager and terrain_manager.current_voxel_generator:
		terrain_manager.current_voxel_generator.set_forcefield_shader_param("base_alpha", 0.0)
