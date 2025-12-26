extends Node3D

@onready var voxel_generator: VoxelGenerator = $VoxelEngine/VoxelGenerator
@onready var picele: CharacterBody3D = $"../Picele"

var custom_distances = PackedFloat64Array([16, 32, 64, 128, 256, 512, 1024, 2048])
var t := 1.0

func _ready() -> void:
	
	# Find WorldManager to get world size
	var world_manager = get_tree().get_first_node_in_group("world")
	
	voxel_generator.cancel_generation()
	voxel_generator.reset()
	
	# Enable the forcefield (if not already)
	voxel_generator.forcefield_enabled = true
	voxel_generator.forcefield_height = world_manager.WALL_HEIGHT
	voxel_generator.forcefield_collision_enabled = true
	voxel_generator.forcefield_detection_enabled = true
	voxel_generator.forcefield_buffer = -1.0
	
	# Load shader resource and wrap in ShaderMaterial
	var shader_res = load("res://scenes/shaders/forcefield.gdshader")
	if not shader_res:
		push_error("Could not load forcefield shader: res://scenes/shaders/forcefield.gdshader")
		return

	var mat := ShaderMaterial.new()
	mat.shader = shader_res

	# Assign the ShaderMaterial to the VoxelGenerator forcefield (C++ binding)
	# This calls `VoxelGenerator::set_forcefield_shader_material(Ref<ShaderMaterial>)` exposed in C++
	voxel_generator.set_forcefield_shader_material(mat)
	
	# Set some shader parameters (example names; adjust to your shader's uniforms)
	# You can either set via the generator helper or directly on the material:
	# generator helper (calls into C++):
	voxel_generator.set_forcefield_shader_param("u_color", Color(0.0, 0.8, 1.0))
	voxel_generator.set_forcefield_shader_param("u_time_scale", 1.5)
	voxel_generator.set_forcefield_shader_param("base_alpha", 0.0)
	
	# Or directly on the ShaderMaterial (if you hold a reference):
	# mat.set_shader_parameter("u_color", Color(1,0,0))

	# Example: animate a parameter over time
	set_process(true)
		
	# Setup Debug
	voxel_generator.debug_mode = true
	voxel_generator.debug_verbosity = 2
	voxel_generator.visualize_noise_values = false
	voxel_generator.auto_generate = false # Make sure this is false before setting world_size
	
	# Configure plains generator
	voxel_generator.world_size = Vector3i(world_manager.WORLD_SIZE, world_manager.WORLD_DEPTH, world_manager.WORLD_SIZE) # Immediately calls .generate() if .auto_generate is set to true
	
	voxel_generator.chunk_size = 8
	voxel_generator.resolution = 2
	voxel_generator.generation_mode = 1 # HEIGHTMAP_FIRST (optimized)
	voxel_generator.use_textures = true
	voxel_generator.surface_band = 1.5
	voxel_generator.max_chunks_per_frame = 1
	voxel_generator.signal_every_n_chunks = 20
	voxel_generator.lod_distances = custom_distances
	voxel_generator.enable_distance_lod = true
	voxel_generator.lod_level = 6
	voxel_generator.lod_reference_position = picele.global_position # no verbose
	voxel_generator.lod_distance_multiplier = 5.0
	voxel_generator.show_lod_colors = false # no verbose
	voxel_generator.heightmap_vertex_limit = 534000000
	
	# Enable debug visualization
	voxel_generator.show_voxel_grid = false
	voxel_generator.show_chunk_grid = false
		
	# Configure terrain
	voxel_generator.terrain_height = 0.0
	voxel_generator.terrain_amplitude = 0.0
	voxel_generator.rock_influence = 0.0
	voxel_generator.cutoff = 0.1
	
	# Confugure biome generator
	var biome_gen = BiomeGenerator.new()
	biome_gen.seed = 12345 # no verbose
	biome_gen.sea_level = 0.0 # no verbose
	setup_biomes(biome_gen)
	
	# Connect to signals
	voxel_generator.chunk_ready.connect(_on_chunk_ready)
	voxel_generator.generation_progress.connect(_on_progress)
	voxel_generator.generation_complete.connect(_on_complete)

func generate_plains() -> void:
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
	# Plains
	var grass: Array[int] = [Voxel.GRASS]
	var dirt: Array[int] = [Voxel.DIRT]
	
	# Use normalized height range (0.0 - 1.0) to match BiomeGenerator API
	biome_gen.add_biome_extended(
		"Plains",
		0.0, # Min height
		0.3, # Max height
		0.3, # Min temp
		0.4, # Max temp
		0.2, # Min humidity
		0.4, # Max humidity
		grass, # Surface Blocks Array
		dirt, # Sub-surface Blocks Array
		3, # Depth
		Voxel.STONE, # Bedrock block
		Voxel.STONE # Filler Block
		)

	## Mountains
	#var stone: Array[int] = [Voxel.STONE]
	#biome_gen.add_biome_extended(
		#"Mountains", 
		#0.7, 
		#1.0, 
		#0.0, 
		#0.5, 
		#0.2, 
		#0.8,
		#stone, 
		#stone, 
		#2, 
		#Voxel.STONE, 
		#Voxel.STONE
		#)
#
	## Desert
	#var sand: Array[int] = [Voxel.SAND]
	#biome_gen.add_biome_extended(
		#"Desert", 
		#0.2, 
		#0.5, 
		#0.7, 
		#1.0, 
		#0.0, 
		#0.3,
		#sand, 
		#sand, 
		#2, 
		#Voxel.STONE, 
		#Voxel.SAND
		#)
		
	voxel_generator.biome_generator = biome_gen

	# Print sampled biome height range for debugging (normalized units)
	_print_biome_height_range(biome_gen)

func _process(delta):
	voxel_generator.lod_reference_position = picele.global_position
	var changed_count = voxel_generator.update_chunks_lod()
	if changed_count > 0:
		voxel_generator.regenerate_dirty_chunks()
	
	if not voxel_generator:
		return
	t += delta
	
	# Oscillate brightness uniform if shader exposes one
	# NOTE: There is no brightness in the shader
	var brightness = 0.5 + 0.5 * sin(t * 2.0)
	voxel_generator.set_forcefield_shader_param("u_brightness", brightness)

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
			
	print("[TerrainPlains] Biome Height Range: Min = %.2f, Max = %.2f" % [min_h, max_h])
	print("[TerrainPlains] Biome Generator Details: ", biome_gen)
