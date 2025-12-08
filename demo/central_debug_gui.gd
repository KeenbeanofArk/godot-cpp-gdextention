extends CanvasLayer

# Thin compatibility wrapper kept to avoid parse errors from older scene references.
# This file is intentionally minimal — the active GUI implementation lives in `res://gui.gd`.

func _ready() -> void:
	# No-op wrapper to satisfy older scene references.
	pass
