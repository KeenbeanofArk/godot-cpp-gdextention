extends Node3D

@onready var voxel_generator: VoxelGenerator = $Terrain/VoxelGenerator
@onready var debug_gui: gui = $DebugGUI
@onready var picele: Picele = $Picele

func _ready():
	# Setup Debug
	voxel_generator.debug_mode = true
	voxel_generator.debug_verbosity = 2
	
	# Configure main generator
	voxel_generator.world_size = Vector3i(2, 2, 2)
	voxel_generator.chunk_size = 8
	voxel_generator.resolution = 3
	voxel_generator.generation_mode = 1  # HEIGHTMAP_FIRST (optimized)
	voxel_generator.surface_band = 3.5
	voxel_generator.auto_generate = false
	voxel_generator.max_chunks_per_frame = 4
	voxel_generator.signal_every_n_chunks = 20

	var custom_distances = PackedFloat64Array([10, 20, 40, 80, 160, 360, 720, 1440])
	voxel_generator.enable_distance_lod = true
	voxel_generator.lod_level = 5
	voxel_generator.lod_reference_position = picele.global_position
	voxel_generator.lod_distance_multiplier = 1.0
	voxel_generator.lod_distances = custom_distances
	voxel_generator.show_lod_colors = true
		
	# Configure terrain
	voxel_generator.terrain_height = 0.3
	voxel_generator.terrain_amplitude = 10.0
	voxel_generator.rock_influence = 0.2
	voxel_generator.seeder = 1240
	
	# Configure terrain noise
	var ter_noise = NoiseGenerator.new()
	ter_noise.seed = 1240
	ter_noise.octaves = 3
	ter_noise.period = 10.0
	ter_noise.persistence = 0.6
	ter_noise.lacunarity = 2.5
	voxel_generator.detail_noise = ter_noise
	
	# Configure detail noise
	var det_noise = NoiseGenerator.new()	
	det_noise.seed = 1241
	det_noise.octaves = 3
	det_noise.period = 10.0
	det_noise.persistence = 0.6
	det_noise.lacunarity = 2.5
	voxel_generator.detail_noise = det_noise

	# Setup biome generator
	var biome_gen = BiomeGenerator.new()
	biome_gen.seed = 1240
	biome_gen.sea_level = -2.0
	setup_biomes(biome_gen)
	voxel_generator.biome_generator = biome_gen

	# Enable debug visualization
	voxel_generator.show_voxel_grid = false
	voxel_generator.show_chunk_grid = false

	# Generate terrain
	#voxel_generator.generate()
	# Start async generation
	voxel_generator.generate_async()
	
	# Initialize debug UI
	debug_gui.create_debug_ui()

	# Cancel if needed
	#voxel_generator.cancel_generation()

	# Connect to signals
	voxel_generator.chunk_ready.connect(_on_chunk_ready)
	voxel_generator.generation_progress.connect(_on_progress)
	voxel_generator.generation_complete.connect(_on_complete)
	
func _on_chunk_ready(_chunk_index: int, chunk_coord: Vector3i):
	print("Chunk %s ready" % chunk_coord)

func _on_progress(completed: int, total: int):
	print("Progress: %d/%d" % [completed, total])
	
func _on_complete():
	print("Chunks complete")

func setup_biomes(biome_gen: BiomeGenerator):
	# Plains
	var grass: Array[int] = [Voxel.GRASS]
	var dirt: Array[int] = [Voxel.DIRT]
	biome_gen.add_biome_extended("Plains", 0.0, 0.5, 0.3, 0.7, 0.3, 0.7,
								  grass, dirt, 3, Voxel.STONE, Voxel.STONE)

	# Mountains
	var stone: Array[int] = [Voxel.STONE]
	biome_gen.add_biome_extended("Mountains", 0.5, 1.0, 0.0, 0.5, 0.2, 0.8,
								  stone, stone, 2, Voxel.STONE, Voxel.STONE)

	# Desert
	var sand: Array[int] = [Voxel.SAND]
	biome_gen.add_biome_extended("Desert", 0.0, 0.3, 0.7, 1.0, 0.0, 0.3,
								  sand, sand, 5, Voxel.STONE, Voxel.SAND)

# In _process, update LOD when player moves
func _process(_delta):
	# Check status
	if voxel_generator.is_generating():
		print("Still generating...")
		
	voxel_generator.lod_reference_position = picele.global_position
	var changed_count = voxel_generator.update_chunks_lod()	
	if changed_count > 0:
		voxel_generator.regenerate_dirty_chunks()
