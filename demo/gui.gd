extends CanvasLayer
class_name gui

# Dynamic terrain binding for multi-terrain support
var terrain_manager: MultiTerrainManager = null
var current_voxel_generator: VoxelGenerator = null
var current_terrain_name: String = ""

# Prevent multiple repeated UI connections
var ui_initialized: bool = false
var ui_bound_to_generator: bool = false

# Get the player
var player: Picele = null

# Color coding for terrains
var terrain_colors: Dictionary = {
	"Plains": Color(0.4, 0.8, 0.4), # Green Grass
	"Mountains": Color(0.7, 0.5, 0.3), # Brown
	"Desert": Color(0.889, 0.793, 0.674, 1.0), # Tan
	"Forest": Color(0.16, 0.393, 0.098, 1.0) # Forst Green
}

@onready var terraform_settings: Control = $TerraformSettings
@onready var terraform_panel: Panel = $TerraformSettings/TerraformPanel
@onready var terraform_container: VBoxContainer = $TerraformSettings/TerraformPanel/TerraformContainer
@onready var terraform_title: Label = $TerraformSettings/TerraformPanel/TerraformContainer/TerraformTitle
@onready var slider_settings_grid: GridContainer = $TerraformSettings/TerraformPanel/TerraformContainer/SliderSettingsGrid
@onready var dig_sphere_radius_label: Label = $TerraformSettings/TerraformPanel/TerraformContainer/SliderSettingsGrid/DigSphereRadiusLabel
@onready var dig_sphere_radius_spinbox: SpinBox = $TerraformSettings/TerraformPanel/TerraformContainer/SliderSettingsGrid/DigSphereRadiusSpinbox
@onready var dig_sphere_strength_label: Label = $TerraformSettings/TerraformPanel/TerraformContainer/SliderSettingsGrid/DigSphereStrengthLabel
@onready var dig_sphere_strength_spinbox: SpinBox = $TerraformSettings/TerraformPanel/TerraformContainer/SliderSettingsGrid/DigSphereStrengthSpinbox
@onready var build_sphere_radius_label: Label = $TerraformSettings/TerraformPanel/TerraformContainer/SliderSettingsGrid/BuildSphereRadiusLabel
@onready var build_sphere_radius_spinbox: SpinBox = $TerraformSettings/TerraformPanel/TerraformContainer/SliderSettingsGrid/BuildSphereRadiusSpinbox
@onready var build_sphere_strength_label: Label = $TerraformSettings/TerraformPanel/TerraformContainer/SliderSettingsGrid/BuildSphereStrengthLabel
@onready var build_sphere_strength_spinbox: SpinBox = $TerraformSettings/TerraformPanel/TerraformContainer/SliderSettingsGrid/BuildSphereStrengthSpinbox

@onready var debug_display: Control = $DebugDisplay
@onready var debug_panel: Panel = $DebugDisplay/DebugPanel
@onready var debug_container: VBoxContainer = $DebugDisplay/DebugPanel/DebugContainer
@onready var title: Label = $DebugDisplay/DebugPanel/DebugContainer/Title
@onready var subtitle: Label = $DebugDisplay/DebugPanel/DebugContainer/Subtitle
@onready var debug_mode: CheckButton = $DebugDisplay/DebugPanel/DebugContainer/DebugMode
@onready var visualize_noise: CheckButton = $DebugDisplay/DebugPanel/DebugContainer/VisualizeNoise
@onready var auto_generate: CheckButton = $DebugDisplay/DebugPanel/DebugContainer/AutoGenerate
@onready var show_voxel_grids: CheckButton = $DebugDisplay/DebugPanel/DebugContainer/ShowVoxelGrids
@onready var show_chunk_grids: CheckButton = $DebugDisplay/DebugPanel/DebugContainer/ShowChunkGrids
@onready var show_centers: CheckButton = $DebugDisplay/DebugPanel/DebugContainer/ShowCenters
@onready var use_textures: CheckButton = $DebugDisplay/DebugPanel/DebugContainer/UseTextures
@onready var slider_grid: GridContainer = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid
@onready var verbosity_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/VerbosityLabel
@onready var verbosity_slider: HSlider = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/VerbositySlider
@onready var chunk_size_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/ChunkSizeLabel
@onready var chunk_size_slider: HSlider = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/ChunkSizeSlider
@onready var resolution_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/ResolutionLabel
@onready var resolution_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/ResolutionSpinner
@onready var surface_band_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/SurfaceBandLabel
@onready var surface_band_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/SurfaceBandSpinner
@onready var lod_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/LodLabel
@onready var lod_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/LodSpinner
@onready var lod_distance_mult_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/LodDistanceMultLabel
@onready var lod_distance_mult_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/LodDistanceMultSpinner
@onready var world_size_x_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/WorldSizeXLabel
@onready var world_size_x_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/WorldSizeXSpinner
@onready var world_size_y_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/WorldSizeYLabel
@onready var world_size_y_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/WorldSizeYSpinner
@onready var world_size_z_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/WorldSizeZLabel
@onready var world_size_z_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/WorldSizeZSpinner
@onready var terrain_height_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/TerrainHeightLabel
@onready var terrain_height_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/TerrainHeightSpinner
@onready var terrain_amplitude_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/TerrainAmplitudeLabel
@onready var terrain_amplitude_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/TerrainAmplitudeSpinner
@onready var rock_influence_label: Label = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/RockInfluenceLabel
@onready var rock_influence_spinner: SpinBox = $DebugDisplay/DebugPanel/DebugContainer/SliderGrid/RockInfluenceSpinner
@onready var generate_async_button: Button = $DebugDisplay/DebugPanel/DebugContainer/GenerateAsyncButton
@onready var generate_button: Button = $DebugDisplay/DebugPanel/DebugContainer/GenerateButton
@onready var print_state_button: Button = $DebugDisplay/DebugPanel/DebugContainer/PrintStateButton
@onready var save_map_button: Button = $DebugDisplay/DebugPanel/DebugContainer/SaveMapButton
@onready var load_map_button: Button = $DebugDisplay/DebugPanel/DebugContainer/LoadMapButton
@onready var exit_menu_button: Button = $DebugDisplay/DebugPanel/DebugContainer/ExitMenuButton
@onready var close_demo_button: Button = $DebugDisplay/DebugPanel/DebugContainer/CloseDemoButton
@onready var fps_counter: Label = $FPSControl/FPS/FpsCounter
@onready var cross_hair: Control = $CrossHair

@export_category("Debug Settings")
@export var debug_show: bool = false
@export var terraform_settings_show: bool = false

func _ready() -> void:
	# Get terrain manager (now autoload)
	terrain_manager = get_node_or_null("/root/MultiTerrainManager")
	if not terrain_manager:
		push_error("GUI: MultiTerrainManager autoload not found")
		return
	
	# Get player
	player = get_node_or_null("/root/World/Picele")
	if not player:
		push_error("GUI: Player not found")
		return
		
	# Connect to terrain selection signal
	terrain_manager.connect("terrain_selected", Callable(self, "_on_terrain_selected"))
	
	# Initialize with first terrain
	if not terrain_manager.terrain_registry.is_empty():
		current_terrain_name = terrain_manager.current_terrain_name
		current_voxel_generator = terrain_manager.current_voxel_generator
	
	debug_display.visible = debug_show
	terraform_settings.visible = terraform_settings_show
	create_debug_ui()
	create_terraform_settings_ui()

	# Initialize UI from current selected generator if already available
	if current_voxel_generator:
		update_ui_from_voxel_generator()
		# Show startup load dialog if saved maps exist
		maybe_show_startup_load_dialog()
	
	# Initialize MultiTerrainManager after scene is loaded
	var mtm = get_node_or_null("/root/MultiTerrainManager")
	if mtm:
		mtm.initialize()
	
	# Check for pending map from startup
	var ss = get_node_or_null("/root/StartupState")
	if ss and ss.pending_map != "":
		_prompt_and_load_map(ss.pending_map)
		ss.pending_map = ""
	elif current_voxel_generator:
		# New map, generate
		current_voxel_generator.generate()
	
func _process(_delta: float) -> void:
	# Update FPS counter
	fps_counter.add_theme_font_size_override("font_size", 30)
	fps_counter.text = "FPS: %d" % [Engine.get_frames_per_second()]
	
	# If debug GUI is visible, update terrain based on raycast
	if debug_display.visible and terrain_manager:
		var new_terrain = terrain_manager.get_terrain_from_raycast()
		if new_terrain != current_terrain_name:
			terrain_manager.switch_to_terrain(new_terrain)
		
		# Update visual feedback
		update_visual_feedback()

func update_visual_feedback() -> void:
	if not terrain_manager or not subtitle:
		return
	
	var raycast_info = terrain_manager.get_raycast_info()
	var terrain_color = terrain_colors.get(current_terrain_name, Color.WHITE)
	
	if raycast_info.get("hit", false):
		var distance = raycast_info["distance"]
		subtitle.text = "[Looking at: %s] Distance: %.1f units" % [current_terrain_name, distance]
		subtitle.add_theme_color_override("font_color", terrain_color)
	else:
		subtitle.text = "[Looking at: %s] No collision" % current_terrain_name
		subtitle.add_theme_color_override("font_color", Color.GRAY)

func _on_terrain_selected(terrain_name: String, voxel_gen: VoxelGenerator) -> void:
	current_terrain_name = terrain_name
	current_voxel_generator = voxel_gen
	print("[GUI] Switched to terrain: %s" % terrain_name)
	update_ui_from_voxel_generator()
	# Ensure UI layout exists, then bind controls to the newly selected generator
	if not ui_initialized:
		create_debug_ui()

	# Bind controls to the current generator (idempotent)
	if not ui_bound_to_generator:
		bind_debug_controls()
	# If a startup autoload has requested a pending map, prompt to load it now
	# Use Engine.has_singleton to avoid errors when the autoload isn't registered
	if Engine.has_singleton("StartupState"):
		var ss = Engine.get_singleton("StartupState")
		if ss and ss.pending_map != "":
			# Call the existing helper to compare params and load safely
			_prompt_and_load_map(ss.pending_map)
			# Clear the pending_map so it isn't re-applied
			ss.pending_map = ""
	
func update_ui_from_voxel_generator() -> void:
	if not current_voxel_generator:
		return
	
	# Update all UI elements to reflect current generator state
	debug_mode.set_pressed(current_voxel_generator.debug_mode)
	visualize_noise.set_pressed(current_voxel_generator.visualize_noise_values)
	auto_generate.set_pressed(current_voxel_generator.auto_generate)
	show_voxel_grids.set_pressed(current_voxel_generator.show_voxel_grid)
	show_chunk_grids.set_pressed(current_voxel_generator.show_chunk_grid)
	show_centers.set_pressed(current_voxel_generator.show_centers)
	use_textures.set_pressed(current_voxel_generator.use_textures)

	update_label(verbosity_label, "Verbosity: ", current_voxel_generator.debug_verbosity)
	verbosity_slider.value = current_voxel_generator.debug_verbosity

	update_label(chunk_size_label, "Chunk Size: ", current_voxel_generator.chunk_size)
	chunk_size_slider.value = current_voxel_generator.chunk_size

	update_label(resolution_label, "Resolution: ", current_voxel_generator.resolution)
	resolution_spinner.value = current_voxel_generator.resolution

	update_label(surface_band_label, "Surface Band: ", current_voxel_generator.surface_band)
	surface_band_spinner.value = current_voxel_generator.surface_band

	update_label(lod_label, "LOD: ", current_voxel_generator.lod_level)
	lod_spinner.value = current_voxel_generator.lod_level

	update_label(lod_distance_mult_label, "LOD Distance Multiplier: ", current_voxel_generator.lod_distance_multiplier)
	lod_distance_mult_spinner.value = current_voxel_generator.lod_distance_multiplier

	update_label(world_size_x_label, "World X: ", current_voxel_generator.world_size.x)
	world_size_x_spinner.value = current_voxel_generator.world_size.x

	update_label(world_size_y_label, "World Y: ", current_voxel_generator.world_size.y)
	world_size_y_spinner.value = current_voxel_generator.world_size.y

	update_label(world_size_z_label, "World Z: ", current_voxel_generator.world_size.z)
	world_size_z_spinner.value = current_voxel_generator.world_size.z

	update_label(terrain_height_label, "Terrain Height: ", current_voxel_generator.terrain_height)
	terrain_height_spinner.value = current_voxel_generator.terrain_height

	update_label(terrain_amplitude_label, "Terrain Amplitude: ", current_voxel_generator.terrain_amplitude)
	terrain_amplitude_spinner.value = current_voxel_generator.terrain_amplitude

	update_label(rock_influence_label, "Rock Influence: ", current_voxel_generator.rock_influence)
	rock_influence_spinner.value = current_voxel_generator.rock_influence

	# Note: layout remains initialized by create_debug_ui(); do not override here.

func update_label(label: Label, prefix: String, value) -> void:
	label.text = prefix + str(value)
	
func create_terraform_settings_ui():
	# Debug Container - centered on screen
	terraform_panel.anchor_left = 0.5
	terraform_panel.anchor_right = 0.5
	terraform_panel.anchor_top = 0.5
	terraform_panel.anchor_bottom = 0.5
	terraform_panel.offset_left = -300
	terraform_panel.offset_right = 300
	terraform_panel.offset_top = -450
	terraform_panel.offset_bottom = 450
	
	# Debug Title
	terraform_title.text = "Terraform Settings"
	terraform_title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	
	# Dig sphere radius slider
	update_label(dig_sphere_radius_label, "Dig Radius: ", player.dig_radius)
	dig_sphere_radius_spinbox.min_value = 1.0
	dig_sphere_radius_spinbox.max_value = 10.0
	dig_sphere_radius_spinbox.step = 1.0
	dig_sphere_radius_spinbox.value = player.dig_radius
	dig_sphere_radius_spinbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	dig_sphere_radius_spinbox.value_changed.connect(func(value):
		player.dig_radius = value
		update_label(dig_sphere_radius_label, "Dig Radius: ", value)
	)
	
	# Dig strength slider
	update_label(dig_sphere_strength_label, "Dig Strength: ", player.dig_strength)
	dig_sphere_strength_spinbox.min_value = 1.0
	dig_sphere_strength_spinbox.max_value = 10.0
	dig_sphere_strength_spinbox.step = 1.0
	dig_sphere_strength_spinbox.value = player.dig_strength
	dig_sphere_strength_spinbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	dig_sphere_strength_spinbox.value_changed.connect(func(value):
		player.dig_strength = value
		update_label(dig_sphere_strength_label, "Dig Strength: ", value)
	)
	
	# Build sphere radius slider
	update_label(build_sphere_radius_label, "Build Radius: ", player.build_radius)
	build_sphere_radius_spinbox.min_value = 1.0
	build_sphere_radius_spinbox.max_value = 10.0
	build_sphere_radius_spinbox.step = 1.0
	build_sphere_radius_spinbox.value = player.build_radius
	build_sphere_radius_spinbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	build_sphere_radius_spinbox.value_changed.connect(func(value):
		player.build_radius = value
		update_label(build_sphere_radius_label, "Build Radius: ", value)
	)
	
	# Build sphere strength slider
	update_label(build_sphere_strength_label, "Build Strength: ", player.build_strength)
	build_sphere_strength_spinbox.min_value = 1.0
	build_sphere_strength_spinbox.max_value = 10.0
	build_sphere_strength_spinbox.step = 1.0
	build_sphere_strength_spinbox.value = player.build_strength
	build_sphere_strength_spinbox.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	build_sphere_strength_spinbox.value_changed.connect(func(value):
		player.build_strength = value
		update_label(build_sphere_strength_label, "Build Strength: ", value)
	)
	
func create_debug_ui():
	if ui_initialized:
		return
	
	# Debug Container - centered on screen
	debug_panel.anchor_left = 0.5
	debug_panel.anchor_right = 0.5
	debug_panel.anchor_top = 0.5
	debug_panel.anchor_bottom = 0.5
	debug_panel.offset_left = -300
	debug_panel.offset_right = 300
	debug_panel.offset_top = -450
	debug_panel.offset_bottom = 450
	
	# Debug Title
	title.text = "VoxelGenerator Debug"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER

	# Connect Generate Async button to test interaction
	generate_async_button.text = "Call .generate_async()"
	generate_async_button.pressed.connect(_on_generate_async_pressed)
	
	# Connect Generate button to test interaction
	generate_button.text = "Call .generate()"
	generate_button.pressed.connect(_on_generate_pressed)

	# Save / Load map buttons
	save_map_button.text = "Save Map"
	save_map_button.size_flags_horizontal = Control.SIZE_EXPAND_FILL

	load_map_button.text = "Load Map"
	load_map_button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	
	# Print State button
	print_state_button.text = "Print Debug State"
	print_state_button.pressed.connect(_on_print_state_pressed)

	# If a generator is available, bind controls to it (de-duplicated in bind_debug_controls)
	if current_voxel_generator:
		bind_debug_controls()

	# Mark UI layout as created so we don't recreate controls repeatedly
	ui_initialized = true


func bind_debug_controls():
	if ui_bound_to_generator:
		return
	if not current_voxel_generator:
		return

	# Debug Mode Toggle
	debug_mode.text = "Debug Mode"
	debug_mode.set_pressed(current_voxel_generator.debug_mode)
	debug_mode.toggled.connect(func(pressed): current_voxel_generator.debug_mode = pressed)

	# Visualize Noise toggle
	visualize_noise.text = "Visualize Noise"
	visualize_noise.set_pressed(current_voxel_generator.visualize_noise_values)
	visualize_noise.toggled.connect(func(pressed): current_voxel_generator.visualize_noise_values = pressed)

	# Auto Generate toggle
	auto_generate.text = "Auto-Generate"
	auto_generate.set_pressed(current_voxel_generator.auto_generate)
	auto_generate.toggled.connect(func(pressed): current_voxel_generator.auto_generate = pressed)

	# Show Voxel Grids toggle
	show_voxel_grids.text = "Show Voxel Grids"
	show_voxel_grids.set_pressed(current_voxel_generator.show_voxel_grid)
	show_voxel_grids.toggled.connect(func(pressed): current_voxel_generator.show_voxel_grid = pressed)

	# Show Chunk Grids toggle
	show_chunk_grids.text = "Show Chunk Grids"
	show_chunk_grids.set_pressed(current_voxel_generator.show_chunk_grid)
	show_chunk_grids.toggled.connect(func(pressed): current_voxel_generator.show_chunk_grid = pressed)

	# Show Centers toggle
	show_centers.text = "Show Centers"
	show_centers.set_pressed(current_voxel_generator.show_centers)
	show_centers.toggled.connect(func(pressed): current_voxel_generator.show_centers = pressed)

	# Use Textures toggle
	use_textures.text = "Use Textures"
	use_textures.set_pressed(current_voxel_generator.use_textures)
	use_textures.toggled.connect(func(pressed): current_voxel_generator.use_textures = pressed)

	# Verbosity slider and label
	update_label(verbosity_label, "Verbosity: ", current_voxel_generator.debug_verbosity)
	verbosity_slider.min_value = 0
	verbosity_slider.max_value = 3
	verbosity_slider.step = 1
	verbosity_slider.value = current_voxel_generator.debug_verbosity
	verbosity_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	verbosity_slider.value_changed.connect(func(value):
		current_voxel_generator.debug_verbosity = int(value)
		update_label(verbosity_label, "Verbosity: ", int(value))
	)

	# Chunk size slider
	update_label(chunk_size_label, "Chunk Size: ", current_voxel_generator.chunk_size)
	chunk_size_slider.min_value = 8
	chunk_size_slider.max_value = 16
	chunk_size_slider.step = 8
	chunk_size_slider.value = current_voxel_generator.chunk_size
	chunk_size_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	chunk_size_slider.value_changed.connect(func(value):
		current_voxel_generator.chunk_size = int(value)
		update_label(chunk_size_label, "Chunk Size: ", int(value))
	)

	# Resolution spinner
	update_label(resolution_label, "Resolution: ", current_voxel_generator.resolution)
	resolution_spinner.min_value = 1
	resolution_spinner.max_value = 10
	resolution_spinner.step = 1
	resolution_spinner.value = current_voxel_generator.resolution
	resolution_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	resolution_spinner.value_changed.connect(func(value):
		current_voxel_generator.resolution = int(value)
		update_label(resolution_label, "Resolution: ", int(value))
	)

	# Surface Band spinner
	update_label(surface_band_label, "Surface Band: ", current_voxel_generator.surface_band)
	surface_band_spinner.min_value = 1.0
	surface_band_spinner.max_value = 20.0
	surface_band_spinner.step = 0.5
	surface_band_spinner.value = current_voxel_generator.surface_band
	surface_band_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	surface_band_spinner.value_changed.connect(func(value):
		current_voxel_generator.surface_band = value
		update_label(surface_band_label, "Surface Band: ", value)
	)

	# LOD Spinner
	update_label(lod_label, "LOD: ", current_voxel_generator.lod_level)
	lod_spinner.min_value = 0
	lod_spinner.max_value = 7
	lod_spinner.step = 1
	lod_spinner.value = current_voxel_generator.lod_level
	lod_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	lod_spinner.value_changed.connect(func(value):
		current_voxel_generator.lod_level = int(value)
		update_label(lod_label, "LOD: ", int(value))
	)

	# LOD Distance Multiplier
	update_label(lod_distance_mult_label, "LOD Distance Multiplier: ", current_voxel_generator.lod_distance_multiplier)
	lod_distance_mult_spinner.min_value = 1.0
	lod_distance_mult_spinner.max_value = 10.0
	lod_distance_mult_spinner.step = 0.1
	lod_distance_mult_spinner.value = current_voxel_generator.lod_distance_multiplier
	lod_distance_mult_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	lod_distance_mult_spinner.value_changed.connect(func(value):
		current_voxel_generator.lod_distance_multiplier = value
		update_label(lod_distance_mult_label, "LOD Distance Multiplier: ", value)
	)

	# World Size X spinner
	update_label(world_size_x_label, "World X: ", current_voxel_generator.world_size.x)
	world_size_x_spinner.min_value = 1
	world_size_x_spinner.max_value = 100
	world_size_x_spinner.step = 1
	world_size_x_spinner.value = current_voxel_generator.world_size.x
	world_size_x_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	world_size_x_spinner.value_changed.connect(func(value):
		var ws = current_voxel_generator.world_size
		current_voxel_generator.world_size = Vector3i(int(value), ws.y, ws.z)
		update_label(world_size_x_label, "World X: ", int(value))
	)

	# World Size Y spinner
	update_label(world_size_y_label, "World Y: ", current_voxel_generator.world_size.y)
	world_size_y_spinner.min_value = 1
	world_size_y_spinner.max_value = 100
	world_size_y_spinner.step = 1
	world_size_y_spinner.value = current_voxel_generator.world_size.y
	world_size_y_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	world_size_y_spinner.value_changed.connect(func(value):
		var ws = current_voxel_generator.world_size
		current_voxel_generator.world_size = Vector3i(ws.x, int(value), ws.z)
		update_label(world_size_y_label, "World Y: ", int(value))
	)

	# World Size Z spinner
	update_label(world_size_z_label, "World Z: ", current_voxel_generator.world_size.z)
	world_size_z_spinner.min_value = 1
	world_size_z_spinner.max_value = 100
	world_size_z_spinner.step = 1
	world_size_z_spinner.value = current_voxel_generator.world_size.z
	world_size_z_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	world_size_z_spinner.value_changed.connect(func(value):
		var ws = current_voxel_generator.world_size
		current_voxel_generator.world_size = Vector3i(ws.x, ws.y, int(value))
		update_label(world_size_z_label, "World Z: ", int(value))
	)

	# Terrain Height spinner
	update_label(terrain_height_label, "Terrain Height: ", current_voxel_generator.terrain_height)
	terrain_height_spinner.min_value = -100.0
	terrain_height_spinner.max_value = 100.0
	terrain_height_spinner.step = 0.5
	terrain_height_spinner.value = current_voxel_generator.terrain_height
	terrain_height_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	terrain_height_spinner.value_changed.connect(func(value):
		current_voxel_generator.terrain_height = value
		update_label(terrain_height_label, "Terrain Height: ", value)
	)

	# Terrain Amplitude spinner
	update_label(terrain_amplitude_label, "Terrain Amplitude: ", current_voxel_generator.terrain_amplitude)
	terrain_amplitude_spinner.min_value = 0.0
	terrain_amplitude_spinner.max_value = 100.0
	terrain_amplitude_spinner.step = 0.5
	terrain_amplitude_spinner.value = current_voxel_generator.terrain_amplitude
	terrain_amplitude_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	terrain_amplitude_spinner.value_changed.connect(func(value):
		current_voxel_generator.terrain_amplitude = value
		update_label(terrain_amplitude_label, "Terrain Amplitude: ", value)
	)

	# Rock Influence spinner
	update_label(rock_influence_label, "Rock Influence: ", current_voxel_generator.rock_influence)
	rock_influence_spinner.min_value = 0.0
	rock_influence_spinner.max_value = 1.0
	rock_influence_spinner.step = 0.01
	rock_influence_spinner.value = current_voxel_generator.rock_influence
	rock_influence_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	rock_influence_spinner.value_changed.connect(func(value):
		current_voxel_generator.rock_influence = value
		update_label(rock_influence_label, "Rock Influence: ", value)
	)

	ui_bound_to_generator = true


func _on_generate_async_pressed():
	if current_voxel_generator:
		current_voxel_generator.mark_all_chunks_dirty()
		current_voxel_generator.generate_async()
		
func _on_generate_pressed():
	if current_voxel_generator:
		current_voxel_generator.mark_all_chunks_dirty()
		current_voxel_generator.generate()

func _on_slice_pressed():
	if current_voxel_generator:
		current_voxel_generator.debug_draw_noise_slice(0.0)

func _on_print_state_pressed():
	if current_voxel_generator:
		current_voxel_generator.debug_print_state()

func _input(event: InputEvent) -> void:
	# Toggle mouse capture with F5 for debug UI access
	if event is InputEventKey and event.pressed and event.keycode == KEY_F5:
		var root = get_tree().root.get_child(0)
		var picele_node = root.find_child("Picele", true, false)
		if picele_node:
			picele_node.show()
			
		if terraform_settings.visible == true:
			terraform_settings.visible = false
				
		if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
			debug_display.visible = true
			cross_hair.visible = false
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
		else:
			debug_display.visible = false
			cross_hair.visible = true
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
			
	# Toggle mouse capture with F6 for terraform settings
	elif event is InputEventKey and event.pressed and event.keycode == KEY_F6:
		var root = get_tree().root.get_child(0)
		var picele_node = root.find_child("Picele", true, false)
		if picele_node:
			picele_node.show()
		
		if debug_display.visible == true:
			debug_display.visible = false
			
		if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
			terraform_settings.visible = true
			cross_hair.visible = false
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
		else:
			terraform_settings.visible = false
			cross_hair.visible = true
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
			
func _on_exit_button_pressed() -> void:
	debug_display.visible = false
	terraform_settings.visible = false
	cross_hair.visible = true
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)


func _on_close_demo_button_pressed() -> void:
	if current_voxel_generator:
		current_voxel_generator.cancel_generation()
		await get_tree().create_timer(1.0).timeout
		get_tree().quit()
	else:
		push_error("No VoxelGenerator attached to Gui.")

func _on_save_map_button_pressed() -> void:
	if not current_voxel_generator:
		print("No voxel generator selected to save")
		return

	# Prompt for custom map name
	var name_dialog = AcceptDialog.new()
	name_dialog.dialog_text = "Enter a name for your saved map:"
	name_dialog.title = "Save Map As"
	var name_edit = LineEdit.new()
	name_edit.placeholder_text = "Map name..."
	name_dialog.add_child(name_edit)
	name_dialog.get_ok_button().text = "Save"
	add_child(name_dialog)

	name_dialog.popup_centered()

	name_dialog.connect("confirmed", func():
		var map_name = name_edit.text.strip_edges()
		if map_name == "":
			map_name = current_terrain_name if current_terrain_name != "" else "map_%d" % Time.get_unix_time_from_system()
		current_voxel_generator.save_map("user://saved_maps", map_name)
		print("[Startup] Saved map to user://saved_maps/%s" % map_name)
		name_dialog.queue_free()
	)
	name_dialog.connect("close_requested", Callable(name_dialog, "queue_free"))

func _on_load_map_button_pressed() -> void:
	# Open a dialog to choose a saved map to load
	maybe_show_startup_load_dialog()


func maybe_show_startup_load_dialog() -> void:
	var da = DirAccess.open("user://saved_maps")
	if not da:
		return
	da.list_dir_begin()
	var map_name = da.get_next()
	var maps = []
	while map_name != "":
		if da.current_is_dir() and map_name != "." and map_name != "..":
			maps.append(map_name)
		map_name = da.get_next()
	da.list_dir_end()
	if maps.is_empty():
		return

	# Build a simple ConfirmationDialog with an ItemList for selection
	var dlg = ConfirmationDialog.new()
	dlg.title = "Load Saved Map"
	dlg.dialog_text = "Select a saved map to load:"
	var list = ItemList.new()
	list.allow_reselect = false
	list.custom_minimum_size = Vector2(360, 220)
	for m in maps:
		list.add_item(m)
	list.select(0)
	dlg.add_child(list)
	add_child(dlg)

	# When confirmed, call helper to prompt/compare params and load
	# Use default dialog buttons (OK/Cancel) to remain compatible across Godot versions
	dlg.popup_centered()
	dlg.connect("confirmed", Callable(self, "_on_startup_load_confirmed").bind(list, maps))
	dlg.connect("close_requested", Callable(dlg, "queue_free"))


func _on_startup_load_confirmed(list: ItemList, maps: Array) -> void:
	var sel = list.get_selected_items()
	if sel.size() == 0:
		return
	var idx = sel[0]
	var map_name = maps[idx]
	_prompt_and_load_map(map_name)


func _prompt_and_load_map(map_name: String) -> void:
	if not current_voxel_generator:
		if terrain_manager and not terrain_manager.terrain_registry.is_empty():
			var keys = terrain_manager.terrain_registry.keys()
			terrain_manager.switch_to_terrain(keys[0])
			current_terrain_name = keys[0]
			current_voxel_generator = terrain_manager.current_voxel_generator
		if not current_voxel_generator:
			print("No voxel generator available to load")
			return

	var meta_path = "user://saved_maps/%s/metadata.json" % map_name
	var f = FileAccess.open(meta_path, FileAccess.READ)
	if f == null:
		# No metadata, load directly
		current_voxel_generator.load_map("user://saved_maps", map_name, true)
		print("[Startup] Requested load from user://saved_maps/%s" % map_name)
		return

	var contents = f.get_as_text()
	f.close()
	var parsed = JSON.parse_string(contents)
	var meta
	if parsed is Dictionary and parsed.has("error"):
		if parsed.error != OK:
			current_voxel_generator.load_map("user://saved_maps", map_name, true)
			print("Requested load from user://saved_maps/%s (invalid metadata)" % map_name)
			return
		meta = parsed.result
	else:
		meta = parsed
	if meta.has("generator_params"):
		var gp = meta["generator_params"]
		var diffs = []
		if gp.has("world_size"):
			var ws = gp["world_size"]
			if ws.size() >= 3:
				var wx = int(ws[0]); var wy = int(ws[1]); var wz = int(ws[2])
				var cur = current_voxel_generator.world_size
				if cur.x != wx or cur.y != wy or cur.z != wz:
					diffs.append("world_size")
		if gp.has("chunk_size") and int(gp["chunk_size"]) != current_voxel_generator.chunk_size:
			diffs.append("chunk_size")
		if gp.has("resolution") and int(gp["resolution"]) != current_voxel_generator.resolution:
			diffs.append("resolution")
		if gp.has("cutoff") and float(gp["cutoff"]) != current_voxel_generator.cutoff:
			diffs.append("cutoff")
		if gp.has("seeder") and int(gp["seeder"]) != current_voxel_generator.seeder:
			diffs.append("seeder")
		if gp.has("generation_mode") and int(gp["generation_mode"]) != current_voxel_generator.generation_mode:
			diffs.append("generation_mode")
		if gp.has("surface_band") and float(gp["surface_band"]) != current_voxel_generator.surface_band:
			diffs.append("surface_band")
		if gp.has("lod_level") and int(gp["lod_level"]) != current_voxel_generator.lod_level:
			diffs.append("lod_level")

		if diffs.size() > 0:
			var dlg = ConfirmationDialog.new()
			dlg.title = "Load Saved Map"
			dlg.dialog_text = "Saved map '%s' differs in: %s\nLoad anyway?" % [map_name, String(", ").join(diffs)]
			add_child(dlg)
			dlg.popup_centered()
			dlg.connect("confirmed", Callable(current_voxel_generator, "load_map").bind("user://saved_maps", map_name, true))
			dlg.connect("close_requested", Callable(dlg, "queue_free"))
			return

	# No diffs or no generator_params: load immediately
	current_voxel_generator.load_map("user://saved_maps", map_name, true)
	print("[GUI] Requested load from user://saved_maps/%s" % map_name)
