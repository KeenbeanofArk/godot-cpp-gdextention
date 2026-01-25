extends Node
## TerrainLoader - Autoload for loading terrain configurations
## Scans res://configs/ for .tres files and provides unified API

var configs: Dictionary = {} # config_name -> TerrainConfig resource
var default_config: Resource = null

func _ready() -> void:
	load_all_configs()
	if configs.is_empty():
		push_warning("TerrainLoader: No terrain configs found in res://configs/")
	else:
		print("[TerrainLoader] Loaded %d terrain configurations" % configs.size())

## Scan res://configs/ directory and load all .tres files as TerrainConfig resources
func load_all_configs() -> void:
	configs.clear()
	
	var dir_path = "res://configs"
	var abs_path = ProjectSettings.globalize_path(dir_path)
	if not DirAccess.dir_exists_absolute(abs_path):
		print("[TerrainLoader] Creating configs directory")
		DirAccess.make_dir_recursive_absolute(abs_path)
	
	var da = DirAccess.open(dir_path)
	if not da:
		push_error("[TerrainLoader] Failed to open configs directory: %s" % dir_path)
		return
	
	da.list_dir_begin()
	var file_name = da.get_next()
	var loaded_count = 0
	
	while file_name != "":
		if file_name.ends_with(".tres"):
			var config_path = dir_path + "/" + file_name
			var config = ResourceLoader.load(config_path) as Resource
			if config:
				configs[config.config_name] = config
				loaded_count += 1
				print("[TerrainLoader] Loaded: %s" % config.config_name)
			else:
				push_warning("[TerrainLoader] Failed to load: %s" % config_path)
		file_name = da.get_next()
	
	da.list_dir_end()
	
	# Set first config as default
	if not configs.is_empty():
		default_config = configs[configs.keys()[0]]
	
	print("[TerrainLoader] Loaded %d configuration(s)" % loaded_count)

## Get a terrain config by name
func get_config(config_name: String) -> Resource:
	if config_name in configs:
		return configs[config_name]
	push_warning("[TerrainLoader] Config not found: %s" % config_name)
	return default_config

## Get all available config names
func get_config_names() -> PackedStringArray:
	return PackedStringArray(configs.keys())

## Get default config
func get_default_config() -> Resource:
	return default_config
