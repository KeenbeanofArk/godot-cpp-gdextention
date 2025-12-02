extends Node3D

@onready var voxel_generator: VoxelGenerator = $Terrain/VoxelGenerator
@onready var chunk: Chunk = $Terrain/Chunk
@onready var terrain: Node3D = $Terrain
@onready var debug_gui: gui = $DebugGUI

func _ready():
	# Debug Settings
	voxel_generator.set_debug_mode(true)
	voxel_generator.set_debug_verbosity(2)
	voxel_generator.set_visualize_noise_values(false)
	voxel_generator.set_vertex_limit(false)
	
	# Generator Setttings
	voxel_generator.set_generation_mode(1)
	voxel_generator.set_world_size(Vector3i(5, 5, 5))
	voxel_generator.set_auto_generate(false)
	voxel_generator.set_chunk_size(8)
	
	# More Settings
	voxel_generator.set_resolution(1)
	voxel_generator.set_cutoff(0.0)
	voxel_generator.set_seeder(1240)
	voxel_generator.set_terrain_height(2.0)
	voxel_generator.set_terrain_amplitude(20.5)
	voxel_generator.set_rock_influence(0.42)
	voxel_generator.set_randomizer(false)
	voxel_generator.set_show_centers(false)
	voxel_generator.set_show_chunk_grid(false)
	voxel_generator.set_show_voxel_grid(false)
	
	# Print initial state
	voxel_generator.debug_print_state()
	
	# Set Detail Noise
	#voxel_generator.detail_noise.noise = FastNoiseLite.new()
	voxel_generator.detail_noise.seed = 1241
	voxel_generator.detail_noise.octaves = 3
	voxel_generator.detail_noise.period = 10.0
	voxel_generator.detail_noise.persistence = 0.6
	voxel_generator.detail_noise.lacunarity = 2.5
	
	## Start the generator
	voxel_generator.generate()
	var bio_gen = BiomeGenerator.new()
	bio_gen.default_voxel = Voxel.DIRT
	chunk.set_biome_generator(bio_gen)
	
	chunk.generate()
	
	# Initialize debug UI
	debug_gui.create_debug_ui()

func _init() -> void:
	pass
	
func _process(_delta):
	pass
