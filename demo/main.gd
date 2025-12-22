extends Node3D
class_name WorldManager

@onready var plains: Node3D = $Plains

func _ready():
	plains.generate_plains()
	
