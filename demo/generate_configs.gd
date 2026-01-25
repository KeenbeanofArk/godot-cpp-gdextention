extends Node
## Script to auto-generate missing terrain config resources
## Note: TerrainConfig is a C++ GDExtension class, not a GDScript class

func _ready() -> void:
	generate_configs()

func generate_configs() -> void:
	# Check if TerrainConfig class exists (C++ GDExtension)
	if not ClassDB.class_exists(&"TerrainConfig"):
		push_warning("[ConfigGenerator] TerrainConfig class not available - GDExtension may not be loaded")
		return
	
	# Create configs directory if it doesn't exist
	var dir_path = "user://configs"
	if not DirAccess.dir_exists_absolute(dir_path):
		DirAccess.make_dir_absolute(dir_path)
	
	# Create TerrainConfig instances and save them
	var configs = [
		{
			"name": "Desert",
			"file": "desert.tres",
			"world_size": Vector3i(4, 2, 4),
			"chunk_size": 32,
			"resolution": 2,
			"cutoff": 0.1,
			"seeder": 9012,
			"terrain_height": 2.0,
			"terrain_amplitude": 4.0,
			"rock_influence": 0.1,
		},
		{
			"name": "Plains",
			"file": "plains.tres",
			"world_size": Vector3i(3, 2, 3),
			"chunk_size": 32,
			"resolution": 2,
			"cutoff": 0.1,
			"seeder": 1240,
			"terrain_height": 4.0,
			"terrain_amplitude": 8.0,
			"rock_influence": 0.3,
		},
		{
			"name": "Mountains",
			"file": "mountains.tres",
			"world_size": Vector3i(3, 3, 3),
			"chunk_size": 32,
			"resolution": 2,
			"cutoff": 0.1,
			"seeder": 5678,
			"terrain_height": 12.0,
			"terrain_amplitude": 16.0,
			"rock_influence": 0.5,
		},
		{
			"name": "Forest",
			"file": "forest.tres",
			"world_size": Vector3i(3, 2, 3),
			"chunk_size": 32,
			"resolution": 2,
			"cutoff": 0.1,
			"seeder": 3456,
			"terrain_height": 6.0,
			"terrain_amplitude": 10.0,
			"rock_influence": 0.2,
		},
	]
	
	for config_data in configs:
		if ResourceLoader.exists(dir_path + "/" + config_data["file"]):
			continue
		
		# Use ClassDB to instantiate C++ GDExtension class
		var config: Resource = ClassDB.instantiate(&"TerrainConfig")
		if config == null:
			push_error("[ConfigGenerator] Failed to instantiate TerrainConfig")
			continue
		config.set_config_name(config_data["name"])
		config.set_world_size(config_data["world_size"])
		config.set_chunk_size(config_data["chunk_size"])
		config.set_resolution(config_data["resolution"])
		config.set_cutoff(config_data["cutoff"])
		config.set_seeder(config_data["seeder"])
		config.set_terrain_height(config_data["terrain_height"])
		config.set_terrain_amplitude(config_data["terrain_amplitude"])
		config.set_rock_influence(config_data["rock_influence"])
		config.resource_name = config_data["name"] + " Terrain"
		
		var error = ResourceSaver.save(config, dir_path + "/" + config_data["file"])
		if error == OK:
			print("[ConfigGenerator] Saved: %s" % config_data["file"])
		else:
			push_error("[ConfigGenerator] Failed to save: %s (error %d)" % [config_data["file"], error])
