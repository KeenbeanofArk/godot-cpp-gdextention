extends Node3D

@onready var picele: CharacterBody3D = $"../Picele"

var custom_distances = PackedFloat64Array([16, 32, 64, 128, 256, 512, 1024, 2048])
var t := 1.0
var voxel_generator: VoxelGenerator
var _pending_generation: bool = false # Flag to track if generation was requested but pending

func _ready() -> void:
	set_process(true)
	
	# Connect to MultiTerrainManager signal for dynamic generator injection
	var terrain_manager = get_node_or_null("/root/MultiTerrainManager")
	if terrain_manager:
		terrain_manager.connect("terrain_selected", Callable(self, "_on_terrain_selected"))
	
	# If already initialized on Desert, set up immediately
	if terrain_manager and terrain_manager.current_terrain_name == "Desert":
		voxel_generator = terrain_manager.current_voxel_generator
		_setup_terrain_instance()

func _on_terrain_selected(terrain_name: String, generator: VoxelGenerator) -> void:
	# Only initialize if this is the Desert terrain
	if terrain_name == "Desert":
		voxel_generator = generator
		_setup_terrain_instance()
		
		# If generation was pending, trigger it now that we have the generator
		if _pending_generation:
			_pending_generation = false
			generate_desert()

func _setup_terrain_instance() -> void:
	# Validate generator exists
	if not voxel_generator:
		push_error("[TerrainDesert] No voxel_generator available for setup")
		return
	
	# Find WorldManager to get world size
	var world_manager = get_tree().get_first_node_in_group("world")
	
	# Enable the forcefield (if not already)
	voxel_generator.forcefield_enabled = true
	voxel_generator.forcefield_height = world_manager.WALL_HEIGHT if world_manager else 300
	voxel_generator.forcefield_collision_enabled = true
	voxel_generator.forcefield_detection_enabled = true
	voxel_generator.forcefield_buffer = -1.0
	voxel_generator.forcefield_detection_voxels = 10
	
	# Load shader resource and wrap in ShaderMaterial
	var shader_res = load("res://scenes/shaders/forcefield.gdshader")
	if not shader_res:
		push_error("[TerrainDesert] Could not load forcefield shader: res://scenes/shaders/forcefield.gdshader")
	else:
		var mat := ShaderMaterial.new()
		mat.shader = shader_res

		# Assign the ShaderMaterial to the VoxelGenerator forcefield (C++ binding)
		# This calls `VoxelGenerator::set_forcefield_shader_material(Ref<ShaderMaterial>)` exposed in C++
		voxel_generator.set_forcefield_shader_material(mat)
		
		# Set some shader parameters (example names; adjust to your shader's uniforms)		voxel_generator.set_forcefield_shader_param("u_color", Color(1.0, 0.6, 0.2))
		voxel_generator.set_forcefield_shader_param("u_time_scale", 1.5)
		voxel_generator.set_forcefield_shader_param("base_alpha", 0.0)

		# Shared terrain material (shader reads biome id from CUSTOM0)
	voxel_generator.terrain_material = preload("res://scenes/shaders/TerrainBiomeTriplanar.tres")
	var terrain_mat := voxel_generator.terrain_material
	if terrain_mat is ShaderMaterial:
		preload("res://scenes/shaders/terrain_texture_arrays.gd").new().ensure_default_arrays(terrain_mat)
	voxel_generator.use_textures = true
	
	# Setup Debug
	voxel_generator.debug_mode = true
	voxel_generator.debug_verbosity = 2
	voxel_generator.visualize_noise_values = false
	voxel_generator.auto_generate = false # Make sure this is false before setting world_size
	
	# Load terrain configuration from resource and apply to generator
	var desert_config = TerrainLoader.get_config("Desert")
	if desert_config:
		desert_config.apply_to_voxel_generator(voxel_generator)
		# Apply material via script instead of .tres
		var material = load("res://assets/textures/ground/terrain_material.tres")
		voxel_generator.set_terrain_material(material)
	
	# Override LOD and visual settings specific to this instance
	voxel_generator.generation_mode = 1 # HEIGHTMAP_FIRST (optimized)
	voxel_generator.use_textures = true
	voxel_generator.surface_band = 1.5
	voxel_generator.max_chunks_per_frame = 4
	voxel_generator.signal_every_n_chunks = 20
	voxel_generator.lod_distances = custom_distances
	voxel_generator.enable_distance_lod = true
	voxel_generator.lod_level = 6
	voxel_generator.lod_reference_position = picele.global_position
	voxel_generator.lod_distance_multiplier = 5.0
	voxel_generator.show_lod_colors = false
	voxel_generator.heightmap_vertex_limit = 534000000
	voxel_generator.show_voxel_grid = false
	voxel_generator.show_chunk_grid = false
	
	# Configure biome generator for desert
	var biome_gen = BiomeGenerator.new()
	biome_gen.seed = 33003 # no verbose
	biome_gen.sea_level = 1.0 # no verbose
	setup_biomes(biome_gen)
	
	# Connect to signals
	voxel_generator.chunk_ready.connect(_on_chunk_ready)
	voxel_generator.generation_progress.connect(_on_progress)
	voxel_generator.generation_complete.connect(_on_complete)
	
	# Generate terrain
	generate_desert()

func generate_desert() -> void:
	# Check if generator is available
	if not voxel_generator:
		print("[TerrainDesert] VoxelGenerator not yet available, marking generation as pending")
		_pending_generation = true
		return
	
	# Start async generation
	voxel_generator.generate_async()
	
	# Initialize central debug UI if present (safe lookup)
	var world = get_parent()
	if not world:
		world = get_tree().get_root().get_child(0) if get_tree().get_root().get_child_count() > 0 else null
	if world:
		var central_gui = world.get_node_or_null("CentralDebugGUI")
		if central_gui and central_gui.has_method("initialize_from_voxel_generator"):
			central_gui.initialize_from_voxel_generator(voxel_generator)

func setup_biomes(biome_gen: BiomeGenerator) -> void:
	# Clear existing biomes
	biome_gen.clear_biomes()
	
	# Create noise generators for biome selection
	var height_noise = NoiseGenerator.new()
	height_noise.set_seed(biome_gen.seed)
	height_noise.set_octaves(4)
	height_noise.set_period(50.0)
	height_noise.set_persistence(0.5)
	height_noise.set_lacunarity(2.0)
	
	var temp_noise = NoiseGenerator.new()
	temp_noise.set_seed(biome_gen.seed + 1)
	temp_noise.set_octaves(3)
	temp_noise.set_period(80.0)
	temp_noise.set_persistence(0.4)
	temp_noise.set_lacunarity(2.2)
	
	var humidity_noise = NoiseGenerator.new()
	humidity_noise.set_seed(biome_gen.seed + 2)
	humidity_noise.set_octaves(3)
	humidity_noise.set_period(60.0)
	humidity_noise.set_persistence(0.5)
	humidity_noise.set_lacunarity(2.0)
	
	biome_gen.set_height_noise(height_noise)
	biome_gen.set_temperature_noise(temp_noise)
	biome_gen.set_humidity_noise(humidity_noise)
	
	# Desert biome: hot, dry, sandy terrain
	# VoxelType: SAND=5
	biome_gen.add_biome_extended(
		"Desert",
		0.15, 0.4, # height range (low to moderate elevation)
		0.7, 1.0, # temperature range (hot)
		0.0, 0.2, # humidity range (dry)
		[5], # surface blocks (SAND)
		[5, 3], # subsurface blocks (SAND, STONE)
		3, # depth
		3, # bedrock (STONE)
		5 # filler (SAND)
	)
	
	# Assign biome generator to voxel generator
	voxel_generator.biome_generator = biome_gen
	print("[TerrainDesert] Desert biome configured")

func _process(delta: float) -> void:
	# Check if generator is valid before accessing it
	if not voxel_generator or not is_instance_valid(voxel_generator):
		return
	
	voxel_generator.lod_reference_position = picele.global_position
	var changed_count = voxel_generator.update_chunks_lod()
	if changed_count > 0:
		voxel_generator.regenerate_dirty_chunks()
	
	t += delta
	# Animate forcefield parameter
	voxel_generator.set_forcefield_shader_param("time", t)
	
	# Update LOD reference position
	if picele:
		voxel_generator.lod_reference_position = picele.global_position

func _on_chunk_ready(_chunk_index: int, _coord: Vector3i) -> void:
	pass

func _on_progress(done: int, total: int) -> void:
	print("[TerrainDesert] Progress: %d/%d" % [done, total])

func _on_complete() -> void:
	print("[TerrainDesert] Generation complete!")
