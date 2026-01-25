extends Node3D
class_name WorldManager

@onready var plains: Node3D = $Plains
@onready var mountains: Node3D = $Mountains
@onready var desert: Node3D = $Desert
@onready var forest: Node3D = $Forest
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
	
	# Hide all terrains initially, then show only the selected one
	_hide_all_terrains()
	
	# Generate and show the active terrain
	match active_terrain:
		"Plains":
			if plains and plains.has_method("generate_plains"):
				plains.generate_plains()
				plains.visible = true
		"Mountains":
			if mountains and mountains.has_method("generate_mountains"):
				mountains.generate_mountains()
				mountains.visible = true
		"Desert":
			if desert and desert.has_method("generate_desert"):
				desert.generate_desert()
				desert.visible = true
		"Forest":
			if forest and forest.has_method("generate_forest"):
				forest.generate_forest()
				forest.visible = true
		"TerrainMultiBiome":
			if terrain_multi_biome and terrain_multi_biome.has_method("generate_terrain"):
				terrain_multi_biome.generate_terrain()
				terrain_multi_biome.visible = true
		_:
			# Default to Plains if unknown terrain
			if plains and plains.has_method("generate_plains"):
				plains.generate_plains()
				plains.visible = true
				active_terrain = "Plains"
	
	print("[WorldManager] Active terrain: %s" % active_terrain)
	
	# Initialize MultiTerrainManager after world setup
	var terrain_manager = get_node_or_null("/root/MultiTerrainManager")
	if terrain_manager:
		terrain_manager.initialize()
		
		# Set the active terrain in the manager
		if terrain_manager.terrain_registry.has(active_terrain):
			terrain_manager.switch_to_terrain(active_terrain)
		else:
			print("[WorldManager] Terrain '%s' not registered in manager" % active_terrain)

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
