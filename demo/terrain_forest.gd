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
		terrain_manager.connect("terrain_selected", Callable(self , "_on_terrain_selected"))
	
	# If already initialized on Forest, set up immediately
	if terrain_manager and terrain_manager.current_terrain_name == "Forest":
		voxel_generator = terrain_manager.current_voxel_generator
		_setup_terrain_instance()

	picele.global_position = Vector3(0.0, 25.0, 0.0)
	
func _on_terrain_selected(terrain_name: String, generator: VoxelGenerator) -> void:
	# Only initialize if this is the Forest terrain
	if terrain_name == "Forest":
		voxel_generator = generator
		_setup_terrain_instance()
		
		# If generation was pending, trigger it now that we have the generator
		if _pending_generation:
			_pending_generation = false
			generate_forest()

func _setup_terrain_instance() -> void:
	# Validate generator exists
	if not voxel_generator:
		push_error("[TerrainForest] No voxel_generator available for setup")
		return
	
	# Find WorldManager to get world size
	var world_manager = get_tree().get_first_node_in_group("world")
	
	# Enable the forcefield (if not already)
	voxel_generator.forcefield_enabled = false
	voxel_generator.forcefield_height = world_manager.WALL_HEIGHT if world_manager else 300
	voxel_generator.forcefield_collision_enabled = false
	voxel_generator.forcefield_detection_enabled = false
	voxel_generator.forcefield_buffer = -1.0
	voxel_generator.forcefield_detection_voxels = 10
	
	# Load shader resource and wrap in ShaderMaterial
	var shader_res = load("res://scenes/shaders/forcefield.gdshader")
	if not shader_res:
		push_error("[TerrainForest] Could not load forcefield shader: res://scenes/shaders/forcefield.gdshader")
	else:
		var mat := ShaderMaterial.new()
		mat.shader = shader_res

		# Assign the ShaderMaterial to the VoxelGenerator forcefield (C++ binding)
		# This calls `VoxelGenerator::set_forcefield_shader_material(Ref<ShaderMaterial>)` exposed in C++
		voxel_generator.set_forcefield_shader_material(mat)
		
		# Set shader parameters - forest-themed green color
		voxel_generator.set_forcefield_shader_param("u_color", Color(0.2, 0.8, 0.3))
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
	var forest_config = TerrainLoader.get_config("Forest")
	if forest_config:
		forest_config.apply_to_voxel_generator(voxel_generator)
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
	voxel_generator.show_voxel_grid = true
	voxel_generator.show_chunk_grid = true
	
	# Configure biome generator for forest
	var biome_gen = BiomeGenerator.new()
	biome_gen.seed = 44004 # no verbose
	biome_gen.sea_level = 0.0 # no verbose
	setup_biomes(biome_gen)

	# Connect to signals
	voxel_generator.chunk_ready.connect(_on_chunk_ready)
	voxel_generator.generation_progress.connect(_on_progress)
	voxel_generator.generation_complete.connect(_on_complete)
	
	# Generate terrain
	generate_forest()

func generate_forest() -> void:
	# Check if generator is available
	if not voxel_generator:
		print("[TerrainForest] VoxelGenerator not yet available, marking generation as pending")
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
		
func _on_chunk_ready(_chunk_index: int, _chunk_coord: Vector3i):
	pass
	#print("Chunk %s ready" % chunk_coord)

func _on_progress(_completed: int, _total: int) -> void:
	pass
	#print("Progress: %d/%d" % [completed, total])
	
func _on_complete() -> void:
	pass
	#print("Chunks complete")

func setup_biomes(biome_gen: BiomeGenerator) -> void:
	# Clear existing biomes
	biome_gen.clear_biomes()
	
	# Create noise generators for biome selection
	var height_noise = NoiseGenerator.new()
	height_noise.set_seed(biome_gen.seed)
	height_noise.set_octaves(4)
	height_noise.set_period(75.0)
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
	
	# Forest biome: moderate temperature, high humidity, lush terrain
	# VoxelType: GRASS=2, DIRT=1
	biome_gen.add_biome_extended(
		"Forest",
		0.40, 0.60, # height range (moderate elevation)
		0.35, 0.55, # temperature range (moderate)
		0.6, 0.9, # humidity range (humid)
		[2], # surface blocks (GRASS)
		[1], # subsurface blocks (DIRT)
		4, # depth
		3, # bedrock (STONE)
		1 # filler (DIRT)
	)
	
	# Assign biome generator to voxel generator
	voxel_generator.biome_generator = biome_gen
	print("[TerrainForest] Forest biome configured")

func _process(delta: float) -> void:
	# Check if generator is valid before accessing it
	if not voxel_generator or not is_instance_valid(voxel_generator):
		return
	
	t += delta
	# Animate forcefield parameter
	voxel_generator.set_forcefield_shader_param("time", t)
	
	# Update LOD reference position
	if picele:
		voxel_generator.lod_reference_position = picele.global_position
