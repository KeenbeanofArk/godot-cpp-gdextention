extends CanvasLayer
class_name gui

@onready var fps_counter: Label = $UI/FPS/FpsCounter
@onready var debug_display: Control = $DebugDisplay
@onready var debug_container: VBoxContainer = $DebugDisplay/DebugContainer
@onready var title: Label = $DebugDisplay/DebugContainer/Title
@onready var debug_mode: CheckButton = $DebugDisplay/DebugContainer/DebugMode
@onready var visualize_noise: CheckButton = $DebugDisplay/DebugContainer/VisualizeNoise
@onready var auto_generate: CheckButton = $DebugDisplay/DebugContainer/AutoGenerate
@onready var show_voxel_grids: CheckButton = $DebugDisplay/DebugContainer/ShowVoxelGrids
@onready var show_chunk_grids: CheckButton = $DebugDisplay/DebugContainer/ShowChunkGrids
@onready var show_centers: CheckButton = $DebugDisplay/DebugContainer/ShowCenters
@onready var slider_grid: GridContainer = $DebugDisplay/DebugContainer/SliderGrid
@onready var verbosity_label: Label = $DebugDisplay/DebugContainer/SliderGrid/VerbosityLabel
@onready var verbosity_slider: HSlider = $DebugDisplay/DebugContainer/SliderGrid/VerbositySlider
@onready var chunk_size_label: Label = $DebugDisplay/DebugContainer/SliderGrid/ChunkSizeLabel
@onready var chunk_size_slider: HSlider = $DebugDisplay/DebugContainer/SliderGrid/ChunkSizeSlider
@onready var resolution_label: Label = $DebugDisplay/DebugContainer/SliderGrid/ResolutionLabel
@onready var resolution_spinner: SpinBox = $DebugDisplay/DebugContainer/SliderGrid/ResolutionSpinner
@onready var surface_band_label: Label = $DebugDisplay/DebugContainer/SliderGrid/SurfaceBandLabel
@onready var surface_band_slider: HSlider = $DebugDisplay/DebugContainer/SliderGrid/SurfaceBandSlider
@onready var lod_label: Label = $DebugDisplay/DebugContainer/SliderGrid/LodLabel
@onready var lod_spinner: SpinBox = $DebugDisplay/DebugContainer/SliderGrid/LodSpinner
@onready var world_size_x_label: Label = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeXLabel
@onready var world_size_x_spinner: SpinBox = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeXSpinner
@onready var world_size_y_label: Label = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeYLabel
@onready var world_size_y_spinner: SpinBox = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeYSpinner
@onready var world_size_z_label: Label = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeZLabel
@onready var world_size_z_spinner: SpinBox = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeZSpinner
@onready var terrain_height_label: Label = $DebugDisplay/DebugContainer/SliderGrid/TerrainHeightLabel
@onready var terrain_height_spinner: SpinBox = $DebugDisplay/DebugContainer/SliderGrid/TerrainHeightSpinner
@onready var terrain_amplitude_label: Label = $DebugDisplay/DebugContainer/SliderGrid/TerrainAmplitudeLabel
@onready var terrain_amplitude_spinner: SpinBox = $DebugDisplay/DebugContainer/SliderGrid/TerrainAmplitudeSpinner
@onready var rock_influence_label: Label = $DebugDisplay/DebugContainer/SliderGrid/RockInfluenceLabel
@onready var rock_influence_spinner: SpinBox = $DebugDisplay/DebugContainer/SliderGrid/RockInfluenceSpinner
@onready var generate_button: Button = $DebugDisplay/DebugContainer/GenerateButton
@onready var print_state_button: Button = $DebugDisplay/DebugContainer/PrintStateButton

var voxel_generator: VoxelGenerator
var chunk: Chunk

@export_category("Debug Settings")
@export var debug_show: bool = false

func _ready() -> void:
	# Get references after node is in tree
	voxel_generator = get_node_or_null("../Terrain/VoxelGenerator")
	chunk = get_node_or_null("../Terrain/Chunk")
	
	if voxel_generator == null:
		push_error("GUI: VoxelGenerator not found at ../Terrain/VoxelGenerator")
	if chunk == null:
		push_error("GUI: Chunk not found at ../Terrain/Chunk")
	
	debug_display.visible = debug_show
	
func _process(_delta: float) -> void:
	# Update FPS counter
	fps_counter.text = "FPS: %d" % [Engine.get_frames_per_second()]

func update_label(label: Label, prefix: String, value) -> void:
	label.text = prefix + str(value)

func create_debug_ui():
	# Debug Container
	debug_container.anchor_left = 1.0
	debug_container.anchor_right = 1.0
	debug_container.size = Vector2(300, 400)
	
	# Debug Title
	title.text = "VoxelGenerator Debug"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER

	# Always connect Generate button to test interaction
	generate_button.text = "Generate"
	generate_button.pressed.connect(_on_generate_pressed)

	# Print State button
	print_state_button.text = "Print Debug State"
	print_state_button.pressed.connect(_on_print_state_pressed)

	if voxel_generator:
		# Debug Mode Toggle
		debug_mode.text = "Debug Mode"
		debug_mode.button_pressed = voxel_generator.debug_mode
		debug_mode.toggled.connect(func(pressed): voxel_generator.debug_mode = pressed)

		# Visualize Noise toggle
		visualize_noise.text = "Visualize Noise"
		visualize_noise.button_pressed = voxel_generator.visualize_noise_values
		visualize_noise.toggled.connect(func(pressed): voxel_generator.visualize_noise_values = pressed)

		# Auto Generate toggle
		auto_generate.text = "Auto-Generate"
		auto_generate.button_pressed = voxel_generator.auto_generate
		auto_generate.toggled.connect(func(pressed): voxel_generator.auto_generate = pressed)

		# Show Voxel Grids toggle
		show_voxel_grids.text = "Show Voxel Grids"
		show_voxel_grids.button_pressed = voxel_generator.show_voxel_grid
		show_voxel_grids.toggled.connect(func(pressed): voxel_generator.show_voxel_grid = pressed)
		
		# Show Chunk Grids toggle
		show_chunk_grids.text = "Show Chunk Grids"
		show_chunk_grids.button_pressed = voxel_generator.show_chunk_grid
		show_chunk_grids.toggled.connect(func(pressed): voxel_generator.show_chunk_grid = pressed)
		
		# Show Centers toggle
		show_centers.text = "Show Centers"
		show_centers.button_pressed = voxel_generator.show_centers
		show_centers.toggled.connect(func(pressed): voxel_generator.show_centers = pressed)
		
		# Verbosity slider and label
		update_label(verbosity_label, "Verbosity: ", voxel_generator.debug_verbosity)
		verbosity_slider.min_value = 0
		verbosity_slider.max_value = 3
		verbosity_slider.step = 1
		verbosity_slider.value = voxel_generator.debug_verbosity
		verbosity_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		verbosity_slider.value_changed.connect(func(value):
			voxel_generator.debug_verbosity = int(value)
			update_label(verbosity_label, "Verbosity: ", int(value))
		)

		# Chunk size slider
		update_label(chunk_size_label, "Chunk Size: ", voxel_generator.chunk_size)
		chunk_size_slider.min_value = 8
		chunk_size_slider.max_value = 16
		chunk_size_slider.step = 8
		chunk_size_slider.value = voxel_generator.chunk_size
		chunk_size_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		chunk_size_slider.value_changed.connect(func(value):
			voxel_generator.chunk_size = int(value)
			update_label(chunk_size_label, "Chunk Size: ", int(value))
		)

		# Resolution slider
		update_label(resolution_label, "Resolution: ", voxel_generator.resolution)
		resolution_spinner.min_value = 1
		resolution_spinner.max_value = 10
		resolution_spinner.step = 1
		resolution_spinner.value = voxel_generator.resolution
		resolution_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		resolution_spinner.value_changed.connect(func(value):
			voxel_generator.resolution = int(value)
			update_label(resolution_label, "Resolution: ", int(value))
		)

		# Surface Band slider
		update_label(surface_band_label, "Surface Band: ", voxel_generator.surface_band)
		surface_band_slider.min_value = 1.0
		surface_band_slider.max_value = 20.0
		surface_band_slider.step = 0.5
		surface_band_slider.value = voxel_generator.surface_band
		surface_band_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		surface_band_slider.value_changed.connect(func(value):
			voxel_generator.surface_band = value
			update_label(surface_band_label, "Surface Band: ", value)
		)
		
		# LOD Spinner
		update_label(lod_label, "LOD: ", voxel_generator.lod_level)
		lod_spinner.min_value = 1
		lod_spinner.max_value = 7
		lod_spinner.step = 1
		lod_spinner.value = voxel_generator.lod_level
		lod_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		lod_spinner.value_changed.connect(func(value):
			voxel_generator.lod_level = int(value)
			update_label(lod_label, "LOD: ", int(value))
		)
		
		# World Size X spinner
		update_label(world_size_x_label, "World X: ", voxel_generator.world_size.x)
		world_size_x_spinner.min_value = 1
		world_size_x_spinner.max_value = 100
		world_size_x_spinner.step = 1
		world_size_x_spinner.value = voxel_generator.world_size.x
		world_size_x_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		world_size_x_spinner.value_changed.connect(func(value):
			var ws = voxel_generator.world_size
			voxel_generator.world_size = Vector3i(int(value), ws.y, ws.z)
			update_label(world_size_x_label, "World X: ", int(value))
		)

		# World Size Y spinner
		update_label(world_size_y_label, "World Y: ", voxel_generator.world_size.y)
		world_size_y_spinner.min_value = 1
		world_size_y_spinner.max_value = 100
		world_size_y_spinner.step = 1
		world_size_y_spinner.value = voxel_generator.world_size.y
		world_size_y_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		world_size_y_spinner.value_changed.connect(func(value):
			var ws = voxel_generator.world_size
			voxel_generator.world_size = Vector3i(ws.x, int(value), ws.z)
			update_label(world_size_y_label, "World Y: ", int(value))
		)

		# World Size Z spinner
		update_label(world_size_z_label, "World Z: ", voxel_generator.world_size.z)
		world_size_z_spinner.min_value = 1
		world_size_z_spinner.max_value = 100
		world_size_z_spinner.step = 1
		world_size_z_spinner.value = voxel_generator.world_size.z
		world_size_z_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		world_size_z_spinner.value_changed.connect(func(value):
			var ws = voxel_generator.world_size
			voxel_generator.world_size = Vector3i(ws.x, ws.y, int(value))
			update_label(world_size_z_label, "World Z: ", int(value))
		)
		
		# Terrain Height spinbox
		update_label(terrain_height_label, "Terrain Height: ", voxel_generator.terrain_height)
		terrain_height_spinner.min_value = -100.0
		terrain_height_spinner.max_value = 100.0
		terrain_height_spinner.step = 0.5
		terrain_height_spinner.value = voxel_generator.terrain_height
		terrain_height_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		terrain_height_spinner.value_changed.connect(func(value):
			voxel_generator.terrain_height = value
			update_label(terrain_height_label, "Terrain Height: ", value)
		)
		
		# Terrain Amplitude spinbox
		update_label(terrain_amplitude_label, "Terrain Amplitude: ", voxel_generator.terrain_amplitude)
		terrain_amplitude_spinner.min_value = 0.0
		terrain_amplitude_spinner.max_value = 100.0
		terrain_amplitude_spinner.step = 0.5
		terrain_amplitude_spinner.value = voxel_generator.terrain_amplitude
		terrain_amplitude_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		terrain_amplitude_spinner.value_changed.connect(func(value):
			voxel_generator.terrain_amplitude = value
			update_label(terrain_amplitude_label, "Terrain Amplitude: ", value)
		)
		
		# Rock Influence spinbox
		update_label(rock_influence_label, "Rock Influence: ", voxel_generator.rock_influence)
		rock_influence_spinner.min_value = 0.0
		rock_influence_spinner.max_value = 1.0
		rock_influence_spinner.step = 0.01
		rock_influence_spinner.value = voxel_generator.rock_influence
		rock_influence_spinner.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		rock_influence_spinner.value_changed.connect(func(value):
			voxel_generator.rock_influence = value
			update_label(rock_influence_label, "Rock Influence: ", value)
		)
	else:
		push_warning("GUI: VoxelGenerator not available, sliders not connected")

func _on_generate_pressed():
	print("Generate button pressed!")
	if voxel_generator:
		voxel_generator.generate()

func _on_slice_pressed():
	print("Slice button pressed!")
	if voxel_generator:
		voxel_generator.debug_draw_noise_slice(0.0)

func _on_print_state_pressed():
	print("Print State button pressed!")
	if voxel_generator:
		voxel_generator.debug_print_state()

func _input(event: InputEvent) -> void:
	# Toggle mouse capture with F5 for debug UI access
	if event is InputEventKey and event.pressed and event.keycode == KEY_F5:
		if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
			debug_display.visible = true
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
		else:
			debug_display.visible = false
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
