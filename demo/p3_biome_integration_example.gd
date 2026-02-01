extends Node3D
## P3 Integration Example - Using Y-Range Layers in Terrain Generation
##
## This example demonstrates how to:
## 1. Create biomes with Y-range layers
## 2. Assign them to VoxelGenerator
## 3. Generate terrain with proper layer placement
## 4. Verify visual correctness of layer distribution

class_name P3_BiomeIntegrationExample

## Scene references - initialized dynamically in _ready()
var voxel_engine: VoxelEngine = null
var voxel_generator: VoxelGenerator = null

## Configuration
var biome_generator: BiomeGenerator = null
var feature_generator: FeatureGenerator = null
var terrains_created: int = 0

func _ready() -> void:
	var sep = ""
	for i in range(80):
		sep += "="
	print("\n" + sep)
	print("P3 INTEGRATION EXAMPLE - Y-RANGE LAYER TERRAIN GENERATION")
	print(sep + "\n")
	
	# Find VoxelEngine in the scene
	voxel_engine = get_tree().root.find_child("VoxelEngine", true, false)
	
	if voxel_engine == null:
		push_error("VoxelEngine node not found in the scene. Please add a VoxelEngine node.")
		return
	
	# Create a VoxelGenerator from VoxelEngine
	if voxel_engine.has_method("create_generator"):
		voxel_generator = voxel_engine.create_generator()
		if voxel_generator == null:
			push_error("Failed to create VoxelGenerator from VoxelEngine")
			return
		print("✓ VoxelGenerator created successfully from VoxelEngine")
	else:
		push_error("VoxelEngine does not have create_generator() method")
		return
	
	# Create and configure biome generator
	setup_biome_system()
	
	# Create and configure feature generator
	setup_feature_system()
	
	# Set debug mode value
	voxel_generator.set_debug_mode(2)
	
	# Assign generators to VoxelGenerator
	voxel_generator.set_biome_generator(biome_generator)
	voxel_generator.set_feature_generator(feature_generator)
	
	# Make VoxelGenerator visible in the scene by adding as child
	if voxel_generator.get_parent() == null:
		add_child(voxel_generator)
		voxel_generator.set_position(Vector3.ZERO)
		print("✓ VoxelGenerator added to P3_BiomeIntegrationExample scene")
	
	# Ensure this node is at origin and visible
	set_position(Vector3.ZERO)
	set_visible(true)
	print("✓ P3_BiomeIntegrationExample positioned at origin (0,0,0)")
	
	# Generate example terrain
	generate_example_terrain()
	
	print("\n" + sep)
	print("INTEGRATION EXAMPLE COMPLETE")
	print(sep + "\n")

## SETUP: Create biome generator with Y-range layers
func setup_biome_system() -> void:
	print("[SETUP] Creating Biome System with Y-Range Layers\n")
	
	biome_generator = BiomeGenerator.new()
	biome_generator.set_seed(42)
	biome_generator.set_blend_distance(16)
	
	# Create noise generators
	var height_noise = NoiseGenerator.new()
	height_noise.set_seed(111)
	height_noise.set_period(50.0)
	height_noise.set_octaves(4)
	height_noise.set_persistence(0.5)
	
	var temp_noise = NoiseGenerator.new()
	temp_noise.set_seed(222)
	temp_noise.set_period(100.0)
	temp_noise.set_octaves(3)
	
	var humidity_noise = NoiseGenerator.new()
	humidity_noise.set_seed(333)
	humidity_noise.set_period(75.0)
	humidity_noise.set_octaves(3)
	
	biome_generator.set_height_noise(height_noise)
	biome_generator.set_temperature_noise(temp_noise)
	biome_generator.set_humidity_noise(humidity_noise)
	
	# Create PLAINS biome with Y-range layers
	_create_plains_biome()
	
	# Create MOUNTAIN biome with Y-range layers
	_create_mountain_biome()
	
	# Create DESERT biome with Y-range layers
	_create_desert_biome()
	
	print("✓ Biome system ready with %d biomes\n" % biome_generator.get_biome_count())

## Create Plains biome: Grass → Dirt → Sand → Stone → Bedrock
func _create_plains_biome() -> void:
	var surface_blocks = PackedInt32Array([2, 1])  # GRASS, DIRT
	
	var subsurface_layers = [
		{
			"block_type": 1,  # DIRT
			"y_min": 30,
			"y_max": 50,
			"density": 0.85
		},
		{
			"block_type": 5,  # SAND
			"y_min": 15,
			"y_max": 30,
			"density": 0.7
		},
		{
			"block_type": 3,  # STONE
			"y_min": 5,
			"y_max": 15,
			"density": 0.95
		}
	]
	
	biome_generator.add_biome_with_y_ranges(
		"Plains",
		-5.0, 20.0,  # height range (low, rolling)
		-0.5, 0.5,  # temperature: temperate
		0.0, 1.0,  # humidity: moist
		surface_blocks,
		subsurface_layers,
		3,  # bedrock_block = STONE
		3  # filler_block = STONE
	)
	print("  ✓ Created Plains biome")
	print("    - Surface: Grass/Dirt")
	print("    - Layer 1 (30-50): Dirt (85% density)")
	print("    - Layer 2 (15-30): Sand (70% density)")
	print("    - Layer 3 (5-15): Stone (95% density)")

## Create Mountain biome: Stone → Stone with ore → Bedrock
func _create_mountain_biome() -> void:
	var surface_blocks = PackedInt32Array([3])  # STONE
	
	var subsurface_layers = [
		{
			"block_type": 3,  # STONE
			"y_min": 40,
			"y_max": 80,
			"density": 1.0
		},
		{
			"block_type": 9,  # COAL (ore)
			"y_min": 20,
			"y_max": 40,
			"density": 0.15  # Sparse ore vein
		},
		{
			"block_type": 3,  # STONE
			"y_min": 10,
			"y_max": 20,
			"density": 0.9
		}
	]
	
	biome_generator.add_biome_with_y_ranges(
		"Mountain",
		15.0, 50.0,  # height range (high, steep)
		-1.0, 0.0,  # temperature: cold
		-1.0, 0.0,  # humidity: dry
		surface_blocks,
		subsurface_layers,
		3,  # bedrock_block = STONE
		3  # filler_block = STONE
	)
	print("  ✓ Created Mountain biome")
	print("    - Surface: Stone")
	print("    - Layer 1 (40-80): Stone (100% density)")
	print("    - Layer 2 (20-40): Coal ore (15% density - sparse veins)")
	print("    - Layer 3 (10-20): Stone (90% density)")

## Create Desert biome: Sand → Sand → Stone → Bedrock
func _create_desert_biome() -> void:
	var surface_blocks = PackedInt32Array([5])  # SAND
	
	var subsurface_layers = [
		{
			"block_type": 5,  # SAND
			"y_min": 20,
			"y_max": 50,
			"density": 1.0
		},
		{
			"block_type": 3,  # STONE (bedrock under desert)
			"y_min": 5,
			"y_max": 20,
			"density": 0.95
		}
	]
	
	biome_generator.add_biome_with_y_ranges(
		"Desert",
		5.0, 25.0,  # height range (low, flat)
		0.5, 1.0,  # temperature: hot
		-1.0, 0.0,  # humidity: dry
		surface_blocks,
		subsurface_layers,
		3,  # bedrock_block = STONE
		3  # filler_block = STONE
	)
	print("  ✓ Created Desert biome")
	print("    - Surface: Sand")
	print("    - Layer 1 (20-50): Sand (100% density)")
	print("    - Layer 2 (5-20): Stone (95% density)")

## EXAMPLE: Generate terrain with Y-range layers
func generate_example_terrain() -> void:
	print("\n[GENERATION] Example Terrain Generation\n")
	
	if not voxel_generator:
		print("✗ VoxelGenerator not available")
		return
	
	# Configure VoxelGenerator - MEDIUM test size for visibility
	voxel_generator.set_world_size(Vector3i(10, 10, 10))  # 10x10x10 chunks only
	voxel_generator.set_chunk_size(16)
	voxel_generator.set_resolution(3)  # 1x resolution (no marching cubes detail yet)
	voxel_generator.set_generation_mode(1)  # HEIGHTMAP_FIRST
	voxel_generator.set_surface_band(4.0)
	voxel_generator.set_terrain_height(10.0)
	voxel_generator.set_terrain_amplitude(15.0)
	voxel_generator.set_rock_influence(0.5)
	voxel_generator.set_seeder(12345)
	voxel_generator.set_lod_level(3)
	voxel_generator.set_show_lod_colors(false)  # CHANGED: Allow biome layer colors to show
	voxel_generator.set_show_voxel_grid(true)
	voxel_generator.set_show_chunk_grid(true)
	
	print("  Configuration:")
	print("    - World: 10x10x10 chunk")
	print("    - Chunk size: 16 voxels")
	print("    - Resolution: 3x")
	print("    - Generation: HEIGHTMAP_FIRST")
	print("    - Surface band: 4.0 units")
	print("    - Terrain: height=10.0, amplitude=15.0")
	
	# Set up noise for terrain variation
	var terrain_noise = NoiseGenerator.new()
	terrain_noise.set_seed(999)
	terrain_noise.set_period(50.0)
	terrain_noise.set_octaves(4)
	terrain_noise.set_persistence(0.5)
	terrain_noise.set_lacunarity(2.0)
	
	var detail_noise = NoiseGenerator.new()
	detail_noise.set_seed(888)
	detail_noise.set_period(10.0)
	detail_noise.set_octaves(3)
	detail_noise.set_persistence(0.6)
	detail_noise.set_lacunarity(2.5)
	
	voxel_generator.set_terrain_noise(terrain_noise)
	voxel_generator.set_detail_noise(detail_noise)
	voxel_generator.set_rock_influence(0.3)
	
	print("\n  Generating terrain...")
	voxel_generator.generate()
	terrains_created += 1
	
	# Log children to verify chunks were created
	var chunk_count = voxel_generator.get_child_count()
	print("  ✓ Terrain generated successfully!")
	print("    - Chunks created: %d" % chunk_count)
	
	# Print all child nodes to see what was created
	for i in range(chunk_count):
		var child = voxel_generator.get_child(i)
		print("      [%d] %s (%s)" % [i, child.name, child.get_class()])
	print("\n  LAYER VERIFICATION (Expected Placement):")
	print("    Plains zones:")
	print("      - Y 30-50: Dirt layer")
	print("      - Y 15-30: Sand layer")
	print("      - Y 5-15: Stone layer")
	print("    Mountain zones:")
	print("      - Y 40-80: Stone layer")
	print("      - Y 20-40: Coal ore veins (sparse)")
	print("      - Y 10-20: Stone layer")
	print("    Desert zones:")
	print("      - Y 20-50: Sand layer")
	print("      - Y 5-20: Stone layer")
	
	print("\n  Visual Verification:")
	print("    1. Enable debug visualization (F5 in demo)")
	print("    2. Look for layer transitions at chunk boundaries")
	print("    3. Mountain zones should show stone-dominated terrain")
	print("    4. Plains should have visible dirt/sand stratification")
	print("    5. Desert should show sandy terrain transitioning to stone")

## VERIFICATION: Check layer placement at specific coordinates
func verify_layer_placement() -> void:
	print("\n[VERIFICATION] Layer Placement Check\n")
	
	var test_positions = [
		{"pos": Vector3i(8, 35, 8), "expected": "Dirt", "biome": "Plains"},
		{"pos": Vector3i(8, 22, 8), "expected": "Sand", "biome": "Plains"},
		{"pos": Vector3i(8, 10, 8), "expected": "Stone", "biome": "Plains"},
		{"pos": Vector3i(24, 50, 24), "expected": "Stone", "biome": "Mountain"},
		{"pos": Vector3i(24, 25, 24), "expected": "Coal", "biome": "Mountain"},
		{"pos": Vector3i(24, 15, 24), "expected": "Stone", "biome": "Mountain"},
		{"pos": Vector3i(40, 35, 40), "expected": "Sand", "biome": "Desert"},
		{"pos": Vector3i(40, 10, 40), "expected": "Stone", "biome": "Desert"},
	]
	
	print("  Position | Expected | Actual | Status")
	var sep = ""
	for i in range(60):
		sep += "-"
	print("  " + sep)
	
	for test in test_positions:
		var voxel = biome_generator.get_voxel_at(test.pos.x, test.pos.y, test.pos.z)
		var actual = "Unknown"
		var status = "?"
		
		if voxel != null:
			var voxel_type = voxel.get_type()
			actual = _voxel_type_name(voxel_type)
			status = "✓" if actual.to_lower() == test.expected.to_lower() else "~"
		else:
			status = "✗"
		
		var pos_str = "(%d,%d,%d)" % [test.pos.x, test.pos.y, test.pos.z]
		print("  %s | %s | %s | %s" % [pos_str.pad_zeros(15), test.expected.pad_zeros(8), actual.pad_zeros(8), status])

## SETUP: Feature System with Underground Ores and Surface Features
func setup_feature_system() -> void:
	print("[SETUP] Creating Feature System with Underground Ores and Surface Features\n")
	
	feature_generator = FeatureGenerator.new()
	feature_generator.set_seed(99)
	
	# Underground Features (with LOW spawn probabilities)
	print("  Underground Features:")
	print("    - Iron ore: 8% probability, Y -100 to 20, cluster size 3")
	feature_generator.add_ore_rule(9, 0.08, -100, 20, 3)  # Iron ore (voxel type 9)
	
	print("    - Coal ore: 6% probability, Y -80 to 30, cluster size 4")
	feature_generator.add_ore_rule(9, 0.06, -80, 30, 4)  # Coal ore (voxel type 9)
	
	# Surface Features (Plains biome only, with LOW spawn probabilities)
	var plains_biome = PackedInt32Array([0])  # Biome index 0 = Plains
	
	print("\n  Surface Features (Plains Biome Only):")
	print("    - Trees: 3% probability, Y 0-100")
	feature_generator.add_tree_rule(0.03, plains_biome, 0, 100)
	
	print("    - Rocks: 5% probability, Y 0-100")
	feature_generator.add_rock_rule(0.05, plains_biome, 0, 100)
	
	print("\n  ✓ Feature system ready with %d rules (seed=99)\n" % feature_generator.get_rule_count())

func _voxel_type_name(voxel_type: int) -> String:
	match voxel_type:
		0: return "Air"
		1: return "Dirt"
		2: return "Grass"
		3: return "Stone"
		5: return "Sand"
		8: return "Bedrock"
		9: return "Coal"
		10: return "Diamond"
		_: return "Type%d" % voxel_type

## DOCUMENTATION
##
## P3 INTEGRATION GUIDE
## ====================
##
## 1. ADD THIS SCRIPT TO YOUR SCENE:
##    - Create a Node3D in your scene
##    - Attach this script to it
##    - Make sure a VoxelGenerator node exists in the scene
##
## 2. RUN THE EXAMPLE:
##    - Play the scene
##    - Watch console for setup and generation logs
##    - Terrain will be generated with Y-range layer system
##
## 3. VERIFY LAYER PLACEMENT:
##    - Use debug visualization (F5) to see generated terrain
##    - Observe layer stratification at different heights
##    - Check biome transitions at chunk boundaries
##
## 4. CUSTOMIZE BIOMES:
##    - Modify _create_plains_biome(), _create_mountain_biome(), etc.
##    - Add new layers with different Y ranges
##    - Adjust density values for sparse/dense placement
##    - Change block types to experiment with materials
##
## 5. NEXT STEPS:
##    - Integrate Y-range layers into your own biome definitions
##    - Use density for probabilistic ores (coal, gold, diamonds)
##    - Create smooth transitions between biome types
##    - Optimize layer matching performance for large worlds
