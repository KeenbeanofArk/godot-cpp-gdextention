extends Node3D
class_name WorldManager

@onready var plains: Node3D = $Plains
@onready var voxel_generator: VoxelGenerator = $Plains/VoxelEngine/VoxelGenerator

@onready var voxel_engine: VoxelEngine = $Plains/VoxelEngine
# NOTE: $VoxelEngine.set_voxel_generator_node($Plains/VoxelEngine/VoxelGenerator)

# World settings
const WORLD_SIZE: int = 25 # In Chunks
const WORLD_DEPTH: int = 20 # In Chunks
const WALL_HEIGHT: float = 200.0 

func _ready():
	plains.generate_plains()
	
func _on_voxel_generator_forcefield_body_entered(_wall_index: int, body: Object) -> void:
	voxel_generator.set_forcefield_shader_param("base_alpha", 0.85)

	# NOTE: Need to make the mesh.visible = true for that _wall_index
	if body is CharacterBody3D:
		# example: give feedback, disable movement, etc.
		if body.has_method("apply_central_impulse"):
			body.apply_central_impulse(Vector3(0, 5, 0))
			
func _on_voxel_generator_forcefield_body_exited(_wall_index: int, _body: Object) -> void:
	voxel_generator.set_forcefield_shader_param("base_alpha", 0.0)
	# NOTE: Need to make the mesh.visible = false for that _wall_index
	
