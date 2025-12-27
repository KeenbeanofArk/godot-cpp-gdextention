extends Node

# Autoload singleton to hold a pending map selection from the startup scene
# Register this script as an autoload named "StartupState" in Project Settings.

var pending_map: String = ""

func set_pending_map(map_name: String) -> void:
	pending_map = map_name

func clear_pending_map() -> void:
	pending_map = ""
