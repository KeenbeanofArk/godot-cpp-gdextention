extends Node3D
class_name WorldManager

@onready var terrain_plains: Node3D = $TerrainPlains

func _ready():
	terrain_plains.generate_plains()
	
