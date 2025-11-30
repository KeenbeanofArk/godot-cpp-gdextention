extends Node3D

@onready var voxel_generator: VoxelGenerator = $Terrain/VoxelGenerator
@onready var chunk: Chunk = $Terrain/Chunk
@onready var terrain: Node3D = $Terrain

func _ready():
	# Debug Settings
	voxel_generator.set_debug_mode(true)
	voxel_generator.set_debug_verbosity(1)
	voxel_generator.set_visualize_noise_values(false)
	voxel_generator.set_vertex_limit(false)
	
	# Generator Setttings
	voxel_generator.set_world_size(Vector3i(8, 4, 8))
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
	
	#var det_noise = NoiseGenerator.new()
	#det_noise.seed = 12345
	#det_noise.octaves = 4
	#det_noise.period = 50.0
	#det_noise.persistence = 0.5
	#det_noise.lacunarity = 2.0
	#
	#voxel_generator.set_terrain_noise(det_noise)
	#voxel_generator.set_detail_noise(det_noise)
	
	# Start the generator
	voxel_generator.generate()
	chunk.generate()
	
func _process(_delta):
	pass
