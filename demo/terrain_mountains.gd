extends Node3D

@onready var picele: CharacterBody3D = $"../Picele"

var custom_distances = PackedFloat64Array([20, 40, 80, 160, 320, 640, 1280, 2560])
var voxel_generator: VoxelGenerator

func _ready() -> void:
	set_process(true)
	
	# Connect to MultiTerrainManager signal for dynamic generator injection
	var terrain_manager = get_node_or_null("/root/MultiTerrainManager")
	if terrain_manager:
		terrain_manager.connect("terrain_selected", Callable(self, "_on_terrain_selected"))
	
	# If already initialized on Mountains, set up immediately
	if terrain_manager and terrain_manager.current_terrain_name == "Mountains":
		voxel_generator = terrain_manager.current_voxel_generator
		_setup_terrain_instance()

func _on_terrain_selected(terrain_name: String, generator: VoxelGenerator) -> void:
	# Only initialize if this is the Mountains terrain
	if terrain_name == "Mountains":
		voxel_generator = generator
		_setup_terrain_instance()

func _setup_terrain_instance() -> void:
	if not voxel_generator:
		push_error("[TerrainMountains] No voxel_generator available for setup")
		return
	
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
		push_error("[TerrainMountains] Could not load forcefield shader: res://scenes/shaders/forcefield.gdshader")
		return

	var mat := ShaderMaterial.new()
	mat.shader = shader_res

	# Assign the ShaderMaterial to the VoxelGenerator forcefield
	voxel_generator.set_forcefield_shader_material(mat)
	
	# Set shader parameters - mountains-themed cool blue
	voxel_generator.set_forcefield_shader_param("u_color", Color(0.0, 0.8, 1.0))
	voxel_generator.set_forcefield_shader_param("u_time_scale", 1.5)
	voxel_generator.set_forcefield_shader_param("base_alpha", 0.0)

	# Shared terrain material (shader reads biome id from CUSTOM0)
	voxel_generator.terrain_material = preload("res://scenes/shaders/TerrainBiomeTriplanar.tres")
	var terrain_mat := voxel_generator.terrain_material
	if terrain_mat is ShaderMaterial:
		preload("res://scenes/shaders/terrain_texture_arrays.gd").new().ensure_default_arrays(terrain_mat)
	voxel_generator.use_textures = false
	
	# Setup Debug
	voxel_generator.debug_mode = true
	voxel_generator.debug_verbosity = 2
	voxel_generator.visualize_noise_values = true
	voxel_generator.auto_generate = false # Make sure this is false before setting world_size
	
	# Load terrain configuration from resource and apply to generator
	var mountains_config = TerrainLoader.get_config("Mountains")
	if mountains_config:
		mountains_config.apply_to_voxel_generator(voxel_generator)
	
	# Override LOD and visual settings specific to mountains
	voxel_generator.generation_mode = 1 # HEIGHTMAP_FIRST (optimized)
	voxel_generator.surface_band = 1.5
	voxel_generator.max_chunks_per_frame = 4
	voxel_generator.signal_every_n_chunks = 20
	voxel_generator.lod_distances = custom_distances
	voxel_generator.enable_distance_lod = true
	voxel_generator.lod_level = 7
	voxel_generator.lod_reference_position = picele.global_position
	voxel_generator.lod_distance_multiplier = 1.0
	voxel_generator.show_lod_colors = true
	
	# Enable debug visualization
	voxel_generator.show_voxel_grid = false
	voxel_generator.show_chunk_grid = false
		
	# Configure terrain - Mountains have dramatic height
	voxel_generator.terrain_height = 0.0
	voxel_generator.terrain_amplitude = 0.0
	voxel_generator.rock_influence = 0.0
	voxel_generator.cutoff = 0.1
	
	# Configure biome generator for mountains
	var biome_gen = BiomeGenerator.new()
	biome_gen.seed = 12346
	biome_gen.sea_level = 0.0
	setup_biomes(biome_gen)

	# Connect to signals
	voxel_generator.chunk_ready.connect(_on_chunk_ready)
	voxel_generator.generation_progress.connect(_on_progress)
	voxel_generator.generation_complete.connect(_on_complete)
	
	# Generate terrain
	generate_mountains()

func generate_mountains() -> void:
	# Start async generation
	voxel_generator.generate_async()
	
	# Initialize central debug UI if present (safe lookup)
	var world = get_parent()
	if not world:
		world = get_tree().get_root().get_child(0) if get_tree().get_root().get_child_count() > 0 else null
	if world:
		var central_gui = world.get_node_or_null("CentralDebugGUI")
		if central_gui and central_gui.has_method("create_debug_ui"):
			central_gui.create_debug_ui()
		
func _on_chunk_ready(_chunk_index: int, _chunk_coord: Vector3i):
	pass
	#print("Chunk %s ready" % chunk_coord)

func _on_progress(_completed: int, _total: int):
	pass
	#print("Progress: %d/%d" % [completed, total])
	
func _on_complete():
	pass
	#print("Chunks complete")

func setup_biomes(biome_gen: BiomeGenerator):
	## Plains
	#var grass: Array[int] = [Voxel.GRASS]
	#var dirt: Array[int] = [Voxel.DIRT]
	#biome_gen.add_biome_extended("Plains", 5, 15, 0.3, 0.7, 0.3, 0.7,
								  #grass, dirt, 2, Voxel.STONE, Voxel.STONE)
	# Mountains
	var stone: Array[int] = [Voxel.STONE]
	
	# Mountains occupy higher normalized elevation range
	biome_gen.add_biome_extended(
		"Mountains",
		0.7,
		1.0,
		0.0,
		0.5,
		0.2,
		0.8,
		stone,
		stone,
		2,
		Voxel.STONE,
		Voxel.STONE
		)
#
	## Desert
	#var sand: Array[int] = [Voxel.SAND]
	#biome_gen.add_biome_extended("Desert", 3, 10, 0.7, 1.0, 0.0, 0.3,
								  #sand, sand, 2, Voxel.STONE, Voxel.SAND)

	voxel_generator.biome_generator = biome_gen

	# Print sampled biome height range for debugging (normalized units)
	_print_biome_height_range(biome_gen, 20, 20.0)

func _process(_delta):
	voxel_generator.lod_reference_position = picele.global_position
	var changed_count = voxel_generator.update_chunks_lod()
	if changed_count > 0:
		voxel_generator.regenerate_dirty_chunks()

func _print_biome_height_range(biome_gen: BiomeGenerator, samples: int = 8, spacing: float = 10.0) -> void:
	if not biome_gen:
		print("No biome generator to sample")
		return
	var min_h := 1e9
	var max_h := -1e9
	for i in range(-samples, samples):
		for j in range(-samples, samples):
			var x = float(i) * spacing
			var z = float(j) * spacing
			var h = biome_gen.get_blended_height_at(x, z)
			min_h = min(min_h, h)
			max_h = max(max_h, h)
			
	print("[TerrainMountains] Biome Height Range: Min = %.2f, Max = %.2f" % [min_h, max_h])
	print("[TerrainMountains] Biome Generator Details: ", biome_gen)
