extends Node3D

func _ready() -> void:

	var v_gen = $VoxelGenerator 
	
	v_gen.generate()
