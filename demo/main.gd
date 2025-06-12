extends Node3D

@onready var v_gen = $VoxelGenerator

func _ready() -> void:
	
	v_gen.auto_generate = false
	v_gen.debug_mode = true
	v_gen.debug_verbosity = 2
	v_gen.chunk_size = 16
	v_gen.resolution = 8
	v_gen.show_centers = false
	v_gen.show_grid = false
	v_gen.generate()
	
func _process(_delta: float) -> void:
	v_gen.log_message("Testing", 3)
