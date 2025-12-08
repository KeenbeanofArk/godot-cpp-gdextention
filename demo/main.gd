extends Node3D

@onready var terrain_plains: Node3D = $TerrainPlains
@onready var terrain_mountains: Node3D = $TerrainMountains

func _ready():
	terrain_plains.generate_plains()
	terrain_mountains.generate_mountains()
