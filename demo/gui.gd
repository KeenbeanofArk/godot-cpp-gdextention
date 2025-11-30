extends CanvasLayer
class_name gui

@onready var fps_counter: Label = $UI/FPS/FpsCounter
@onready var debug_display: Control = $DebugDisplay
@onready var debug_container: VBoxContainer = $DebugDisplay/DebugContainer
@onready var title: Label = $DebugDisplay/DebugContainer/Title
@onready var debug_mode: CheckButton = $DebugDisplay/DebugContainer/DebugMode
@onready var visualize_noise: CheckButton = $DebugDisplay/DebugContainer/VisualizeNoise
@onready var auto_generate: CheckButton = $DebugDisplay/DebugContainer/AutoGenerate
@onready var slider_grid: GridContainer = $DebugDisplay/DebugContainer/SliderGrid
@onready var verbosity_label: Label = $DebugDisplay/DebugContainer/SliderGrid/VerbosityLabel
@onready var verbosity_slider: HSlider = $DebugDisplay/DebugContainer/SliderGrid/VerbositySlider
@onready var chunk_size_label: Label = $DebugDisplay/DebugContainer/SliderGrid/ChunkSizeLabel
@onready var chunk_size_slider: HSlider = $DebugDisplay/DebugContainer/SliderGrid/ChunkSizeSlider
@onready var resolution_label: Label = $DebugDisplay/DebugContainer/SliderGrid/ResolutionLabel
@onready var resolution_slider: HSlider = $DebugDisplay/DebugContainer/SliderGrid/ResolutionSlider
@onready var world_size_x_label: Label = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeXLabel
@onready var world_size_x_slider: HSlider = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeXSlider
@onready var world_size_y_label: Label = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeYLabel
@onready var world_size_y_slider: HSlider = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeYSlider
@onready var world_size_z_label: Label = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeZLabel
@onready var world_size_z_slider: HSlider = $DebugDisplay/DebugContainer/SliderGrid/WorldSizeZSlider
@onready var generate_button: Button = $DebugDisplay/DebugContainer/GenerateButton
@onready var slice_button: Button = $DebugDisplay/DebugContainer/SliceButton
@onready var print_state_button: Button = $DebugDisplay/DebugContainer/PrintStateButton

var voxel_generator: VoxelGenerator
var chunk: Chunk

func _ready() -> void:
	# Get references after node is in tree
	voxel_generator = get_node_or_null("../Terrain/VoxelGenerator")
	chunk = get_node_or_null("../Terrain/Chunk")
	
	if voxel_generator == null:
		push_error("GUI: VoxelGenerator not found at ../Terrain/VoxelGenerator")
	if chunk == null:
		push_error("GUI: Chunk not found at ../Terrain/Chunk")
	
	# Initialize debug UI
	create_debug_ui()
	
func _process(_delta: float) -> void:
	# Update FPS counter
	fps_counter.text = "FPS: %d" % [Engine.get_frames_per_second()]

func update_label(label: Label, prefix: String, value: int) -> void:
	label.text = prefix + str(value)

func create_debug_ui():
	# Debug Container
	debug_container.position = Vector2(10, 10)
	debug_container.size = Vector2(300, 400)
	
	# Debug Title
	title.text = "VoxelGenerator Debug"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER

	# Always connect Generate button to test interaction
	generate_button.text = "Generate"
	generate_button.pressed.connect(_on_generate_pressed)
	
	# Debug slice button
	slice_button.text = "Draw Noise Slice at Y=0"
	slice_button.pressed.connect(_on_slice_pressed)

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
		auto_generate.text = "Auto Regenerate"
		auto_generate.button_pressed = voxel_generator.auto_generate
		auto_generate.toggled.connect(func(pressed): voxel_generator.auto_generate = pressed)

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
		chunk_size_slider.max_value = 64
		chunk_size_slider.step = 8
		chunk_size_slider.value = voxel_generator.chunk_size
		chunk_size_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		chunk_size_slider.value_changed.connect(func(value):
			voxel_generator.chunk_size = int(value)
			update_label(chunk_size_label, "Chunk Size: ", int(value))
		)

		# Resolution slider
		update_label(resolution_label, "Resolution: ", voxel_generator.resolution)
		resolution_slider.min_value = 1
		resolution_slider.max_value = 10
		resolution_slider.step = 1
		resolution_slider.value = voxel_generator.resolution
		resolution_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		resolution_slider.value_changed.connect(func(value):
			voxel_generator.resolution = int(value)
			update_label(resolution_label, "Resolution: ", int(value))
		)

		# World Size X slider
		update_label(world_size_x_label, "World X: ", voxel_generator.world_size.x)
		world_size_x_slider.min_value = 1
		world_size_x_slider.max_value = 100
		world_size_x_slider.step = 1
		world_size_x_slider.value = voxel_generator.world_size.x
		world_size_x_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		world_size_x_slider.value_changed.connect(func(value):
			var ws = voxel_generator.world_size
			voxel_generator.world_size = Vector3i(int(value), ws.y, ws.z)
			update_label(world_size_x_label, "World X: ", int(value))
		)

		# World Size Y slider
		update_label(world_size_y_label, "World Y: ", voxel_generator.world_size.y)
		world_size_y_slider.min_value = 1
		world_size_y_slider.max_value = 100
		world_size_y_slider.step = 1
		world_size_y_slider.value = voxel_generator.world_size.y
		world_size_y_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		world_size_y_slider.value_changed.connect(func(value):
			var ws = voxel_generator.world_size
			voxel_generator.world_size = Vector3i(ws.x, int(value), ws.z)
			update_label(world_size_y_label, "World Y: ", int(value))
		)

		# World Size Z slider
		update_label(world_size_z_label, "World Z: ", voxel_generator.world_size.z)
		world_size_z_slider.min_value = 1
		world_size_z_slider.max_value = 100
		world_size_z_slider.step = 1
		world_size_z_slider.value = voxel_generator.world_size.z
		world_size_z_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		world_size_z_slider.value_changed.connect(func(value):
			var ws = voxel_generator.world_size
			voxel_generator.world_size = Vector3i(ws.x, ws.y, int(value))
			update_label(world_size_z_label, "World Z: ", int(value))
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
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
		else:
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
