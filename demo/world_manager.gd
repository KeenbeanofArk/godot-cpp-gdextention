extends Node3D
class_name WorldManager

@onready var plains: Node3D = $Plains
@onready var voxel_generator: VoxelGenerator = $Plains/VoxelGenerator

# World settings
const WORLD_SIZE: int = 5 # In Chunks

func _ready():
	plains.generate_plains()
	
