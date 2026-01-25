extends Node3D
class_name TerrainMultiBiome

# A unified 4-biome terrain with fixed world size
# Demonstrates: Plains, Mountains, Desert, and Forest biomes from one BiomeGenerator

var voxel_generator: VoxelGenerator = null
var forcefield_active: bool = false

# Four biomes configuration - non-overlapping height ranges
var active_biomes = {
	"Plains": {
		"height_range": [-1.0, 0.0],
		"temp_range": [0.3, 0.7],
		"humidity_range": [0.3, 0.6],
		"seed_offset": 1001
	},
	"Desert": {
		"height_range": [0.0, 0.3],
		"temp_range": [0.6, 1.0],
		"humidity_range": [0.0, 0.3],
		"seed_offset": 3003
	},
	"Forest": {
		"height_range": [0.3, 0.6],
		"temp_range": [0.3, 0.6],
		"humidity_range": [0.5, 1.0],
		"seed_offset": 4004
	},
	"Mountains": {
		"height_range": [0.6, 1.0],
		"temp_range": [0.0, 0.4],
		"humidity_range": [0.2, 0.8],
		"seed_offset": 2002
	}
}

func _ready() -> void:
	set_process(true)
	var terrain_manager = get_node_or_null("/root/MultiTerrainManager")
	if terrain_manager:
		terrain_manager.connect("terrain_selected", Callable(self, "_on_terrain_selected"))
	if terrain_manager and terrain_manager.current_terrain_name == "MultiB iome":
		_setup_terrain_instance()


func _on_terrain_selected(terrain_name: String, generator: VoxelGenerator) -> void:
	# MultiB iome manages its own generator, ignore factory generators
	if terrain_name == "MultiB iome":
		_setup_terrain_instance()


func _setup_terrain_instance() -> void:
	"""Setup multi-biome terrain instance with signal integration"""
	if voxel_generator:
		return # Already initialized
	
	# Create VoxelGenerator internally (MultiB iome pattern)
	voxel_generator = VoxelGenerator.new()
	add_child(voxel_generator)
	voxel_generator.set_name("VoxelGenerator")
	
	# Fixed world size: 7x7x7 chunks (not dynamic from WorldManager)
	voxel_generator.world_size = Vector3i(25, 12, 25)
	voxel_generator.chunk_size = 8
	
	# Generation settings
	voxel_generator.resolution = 2
	voxel_generator.generation_mode = 1 # HEIGHTMAP_FIRST for performance
	voxel_generator.surface_band = 4.0
	voxel_generator.cutoff = 0.1
	voxel_generator.lod_level = 6
	
	# Terrain generation parameters
	voxel_generator.terrain_height = 0.1
	voxel_generator.terrain_amplitude = 0.1
	voxel_generator.rock_influence = 0.1
	
	# LOD configuration
	voxel_generator.enable_distance_lod = false
	voxel_generator.lod_distances = PackedFloat64Array([
		16.0, 32.0, 64.0, 128.0, 256.0, 512.0, 1024.0, 2048.0
	])
	voxel_generator.lod_distance_multiplier = 2.0
	
	# Debug settings
	voxel_generator.debug_mode = true
	voxel_generator.debug_verbosity = 1
	voxel_generator.show_voxel_grid = false
	voxel_generator.show_chunk_grid = false
	voxel_generator.show_centers = false
	voxel_generator.auto_generate = false
	
	# Async settings
	voxel_generator.max_chunks_per_frame = 4
	voxel_generator.signal_every_n_chunks = 8
	
	# Setup biomes
	setup_multi_biomes()
	
	# Setup forcefield
	setup_forcefield()
	
	# Connect generation signals
	voxel_generator.connect("generation_complete", Callable(self, "_on_generation_complete"))
	voxel_generator.connect("generation_progress", Callable(self, "_on_generation_progress"))
	
	# Print biome configuration
	print_active_biome_config()
	
	# Do NOT auto-generate here - WorldManager will call generate_terrain()
	print("[TerrainMultiBiome] Ready - awaiting generate_terrain() call")


func setup_multi_biomes() -> void:
	"""Register four biomes to a single BiomeGenerator"""
	var biome_gen = BiomeGenerator.new()
	
	# Base seed for height/temperature/humidity noise
	var base_seed = 42
	
	# Configure noise generators with base seed
	var height_noise = NoiseGenerator.new()
	height_noise.set_seed(base_seed)
	height_noise.set_octaves(4)
	height_noise.set_period(50.0)
	height_noise.set_persistence(0.5)
	height_noise.set_lacunarity(2.0)
	
	var temp_noise = NoiseGenerator.new()
	temp_noise.set_seed(base_seed + 1)
	temp_noise.set_octaves(3)
	temp_noise.set_period(80.0)
	temp_noise.set_persistence(0.4)
	temp_noise.set_lacunarity(2.2)
	
	var humidity_noise = NoiseGenerator.new()
	humidity_noise.set_seed(base_seed + 2)
	humidity_noise.set_octaves(3)
	humidity_noise.set_period(60.0)
	humidity_noise.set_persistence(0.5)
	humidity_noise.set_lacunarity(2.0)
	
	biome_gen.set_height_noise(height_noise)
	biome_gen.set_temperature_noise(temp_noise)
	biome_gen.set_humidity_noise(humidity_noise)
	biome_gen.set_seed(base_seed)
	biome_gen.set_sea_level(0.0)
	biome_gen.set_default_voxel(0) # VoxelType.AIR
	biome_gen.set_blend_distance(8)
	
	# Biome 1: Plains - low areas, moderate temp, moderate humidity
	biome_gen.add_biome_extended(
		"Plains",
		-1.0, 0.0, # height range (lowest)
		0.3, 0.7, # temperature range
		0.3, 0.6, # humidity range
		[2], # surface blocks (GRASS)
		[1], # subsurface blocks (DIRT)
		4, # depth
		3, # bedrock (STONE)
		1 # filler (DIRT)
	)
	
	# Biome 2: Desert - low-medium areas, hot, dry
	biome_gen.add_biome_extended(
		"Desert",
		0.0, 0.3, # height range
		0.6, 1.0, # temperature range (hot)
		0.0, 0.3, # humidity range (dry)
		[5], # surface blocks (SAND)
		[5], # subsurface blocks (SAND)
		2, # depth
		3, # bedrock (STONE)
		5 # filler (SAND)
	)
	
	# Biome 3: Forest - medium areas, moderate temp, wet
	biome_gen.add_biome_extended(
		"Forest",
		0.3, 0.6, # height range
		0.3, 0.6, # temperature range
		0.5, 1.0, # humidity range (wet)
		[2], # surface blocks (GRASS)
		[1], # subsurface blocks (DIRT)
		5, # depth (thicker soil)
		3, # bedrock (STONE)
		1 # filler (DIRT)
	)
	
	# Biome 4: Mountains - high areas, cold
	biome_gen.add_biome_extended(
		"Mountains",
		0.6, 1.0, # height range (highest)
		0.0, 0.4, # temperature range (cold)
		0.2, 0.8, # humidity range
		[3], # surface blocks (STONE)
		[10], # subsurface blocks (COAL)
		3, # depth
		3, # bedrock (STONE)
		3 # filler (STONE)
	)
	
	# Assign biome generator to voxel generator
	voxel_generator.set_biome_generator(biome_gen)


func setup_forcefield() -> void:
	"""Setup forcefield for world boundary containment"""
	voxel_generator.set_forcefield_enabled(true)
	voxel_generator.set_forcefield_height(200.0)
	voxel_generator.set_forcefield_collision_enabled(true)
	voxel_generator.set_forcefield_detection_enabled(true)
	voxel_generator.set_forcefield_buffer(-1.0)
	voxel_generator.set_forcefield_detection_voxels(10.0)
	
	# Setup forcefield shader material if available
	var shader_material = ShaderMaterial.new()
	if shader_material:
		voxel_generator.set_forcefield_shader_material(shader_material)
	
	# Enable individual walls
	voxel_generator.set_forcefield_north_enabled(true)
	voxel_generator.set_forcefield_south_enabled(true)
	voxel_generator.set_forcefield_east_enabled(true)
	voxel_generator.set_forcefield_west_enabled(true)
	voxel_generator.set_forcefield_top_enabled(true)
	voxel_generator.set_forcefield_bottom_enabled(true)
	
	forcefield_active = true


func _on_generation_complete() -> void:
	"""Called when async generation completes"""
	print("[TerrainMultiBiome] Generation complete!")
	print_active_biome_summary()


func _on_generation_progress(completed: int, total: int) -> void:
	"""Called periodically during async generation"""
	var percent = int(100.0 * completed / total) if total > 0 else 0
	print("[TerrainMultiBiome] Generation progress: %d/%d chunks (%d%%)" % [completed, total, percent])


func print_active_biome_config() -> void:
	"""Print all active biome configurations"""
	print("\n========== TERRAIN MULTI-BIOME CONFIGURATION ==========")
	print("World Size: %s chunks" % voxel_generator.world_size)
	print("Chunk Size: %d voxels" % voxel_generator.chunk_size)
	print("Resolution: %d samples/voxel" % voxel_generator.resolution)
	print("Generation Mode: %s" % ("HEIGHTMAP_FIRST" if voxel_generator.generation_mode == 1 else "VOXELS_FIRST"))
	print("\n--- BIOMES REGISTERED ---")
	for biome_name in active_biomes.keys():
		var config = active_biomes[biome_name]
		print("\n[%s]" % biome_name)
		print("  Height Range: %.2f - %.2f" % [config["height_range"][0], config["height_range"][1]])
		print("  Temperature Range: %.2f - %.2f" % [config["temp_range"][0], config["temp_range"][1]])
		print("  Humidity Range: %.2f - %.2f" % [config["humidity_range"][0], config["humidity_range"][1]])
		print("  Seed Offset: %d" % config["seed_offset"])
	print("\n======================================================\n")


func print_biome_height_range() -> void:
	"""Sample and print terrain heights at different X/Z positions"""
	if not voxel_generator or not voxel_generator.get_biome_generator():
		print("[TerrainMultiBiome] No biome generator available")
		return
	
	var biome_gen = voxel_generator.get_biome_generator()
	print("\n--- SAMPLED BIOME HEIGHTS ---")
	
	var test_positions = [
		[0.0, 0.0, "Center"],
		[32.0, 0.0, "East"],
		[-32.0, 0.0, "West"],
		[0.0, 32.0, "North"],
		[0.0, -32.0, "South"],
	]
	
	for pos in test_positions:
		var height = biome_gen.get_height_at(pos[0], pos[1])
		var biome_idx = biome_gen.get_biome_index_at(pos[0], pos[1])
		var biome_names = active_biomes.keys()
		var biome_name = biome_names[biome_idx] if biome_idx < biome_names.size() else "Unknown"
		print("  %s: height=%.2f, biome=%s" % [pos[2], height, biome_name])
	print()


func print_active_biome_summary() -> void:
	"""Print a summary of active biomes and generation status"""
	print("\n========== GENERATION SUMMARY ==========")
	print("Terrain: Multi-Biome (4 biomes)")
	print("World Size: %s" % voxel_generator.world_size)
	print("Active Biomes: %s" % ", ".join(active_biomes.keys()))
	print("Forcefield: %s" % ("ENABLED" if forcefield_active else "DISABLED"))
	print("=========================================\n")


# Public method for WorldManager compatibility
func generate_terrain() -> void:
	"""Generate the multi-biome terrain - called by WorldManager"""
	print("[TerrainMultiBiome] Starting terrain generation...")
	voxel_generator.generate_async()


# Optional: Public method to switch generation mode
func set_generation_mode(mode: int) -> void:
	"""Set generation mode (0=VOXELS_FIRST, 1=HEIGHTMAP_FIRST)"""
	voxel_generator.set_generation_mode(mode)
	print("[TerrainMultiBiome] Generation mode set to: %s" % ("HEIGHTMAP_FIRST" if mode == 1 else "VOXELS_FIRST"))


# Optional: Public method to trigger regeneration
func regenerate() -> void:
	"""Trigger async regeneration"""
	print("[TerrainMultiBiome] Triggering async regeneration...")
	voxel_generator.generate_async()


# Optional: Public method to manually set LOD level
func set_lod_level(lod: int) -> void:
	"""Set global LOD level (0-7)"""
	voxel_generator.set_lod_level(lod)
	print("[TerrainMultiBiome] LOD level set to: %d" % lod)
