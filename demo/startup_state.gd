extends Node

# Autoload singleton to hold a pending map selection from the startup scene
# Register this script as an autoload named "StartupState" in Project Settings.

var pending_map: String = ""
var pending_terrain: String = "Plains"  # Default terrain selection

func set_pending_map(map_name: String) -> void:
	pending_map = map_name

func clear_pending_map() -> void:
	pending_map = ""

func set_pending_terrain(terrain_name: String) -> void:
	pending_terrain = terrain_name

func get_pending_terrain() -> String:
	return pending_terrain
