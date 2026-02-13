extends Node3D

@onready var voxel_generator: VoxelGenerator = $Terrain/VoxelGenerator

func _ready():
	# Configure main generator
	voxel_generator.world_size = Vector3i(10, 3, 10)
	voxel_generator.chunk_size = 8
	voxel_generator.resolution = 4
	voxel_generator.generation_mode = 1 # HEIGHTMAP_FIRST (optimized)
	voxel_generator.surface_band = 4.0
	voxel_generator.lod_level = 0
	voxel_generator.auto_generate = false

	# Configure terrain
	voxel_generator.terrain_height = 2.0
	voxel_generator.terrain_amplitude = 20.5
	voxel_generator.rock_influence = 0.42
	voxel_generator.seeder = 1240

	# Configure detail noise
	voxel_generator.detail_noise.seed = 1241
	voxel_generator.detail_noise.octaves = 3
	voxel_generator.detail_noise.period = 10.0
	voxel_generator.detail_noise.persistence = 0.6
	voxel_generator.detail_noise.lacunarity = 2.5

	# Setup biome generator
	var biome_gen = BiomeGenerator.new()
	biome_gen.seed = 1240
	biome_gen.sea_level = 5.0
	setup_biomes(biome_gen)
	voxel_generator.biome_generator = biome_gen

	# Enable debug visualization
	voxel_generator.show_voxel_grid = true
	voxel_generator.show_chunk_grid = true

	# Generate terrain
	voxel_generator.generate()

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
