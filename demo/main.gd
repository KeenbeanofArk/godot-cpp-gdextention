extends Node3D

@onready var voxel_generator = $VoxelGenerator
@onready var chunk = $Chunk
@onready var fps_counter = $UI/FPS/FPSLabel

func _ready():
	# Setup UI elements
	create_debug_ui()
	
	# Enable debug mode
	voxel_generator.debug_mode = false
	voxel_generator.debug_verbosity = 1
	voxel_generator.auto_generate = false
	voxel_generator.generate_size = 1
	voxel_generator.resolution = 1
	voxel_generator.show_centers = false
	voxel_generator.show_grid = false
	voxel_generator.visualize_noise_values = false
	
	# Print initial state
	voxel_generator.debug_print_state()
	
	# Start the generator
	#voxel_generator.generate()
	
	chunk.generate()
	

func create_debug_ui():
	var control = Control.new()
	control.anchor_right = 1.0
	control.anchor_bottom = 1.0
	add_child(control)

	var vbox = VBoxContainer.new()
	vbox.position = Vector2(10, 10)
	vbox.size = Vector2(250, 300)
	control.add_child(vbox)

	# Title
	var title = Label.new()
	title.text = "VoxelGenerator Debug"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	vbox.add_child(title)

	# Debug toggle
	var debug_toggle = CheckButton.new()
	debug_toggle.text = "Debug Mode"
	debug_toggle.button_pressed = voxel_generator.debug_mode
	debug_toggle.toggled.connect(func(pressed): voxel_generator.debug_mode = pressed)
	vbox.add_child(debug_toggle)

	# Visualize toggle
	var visualize_toggle = CheckButton.new()
	visualize_toggle.text = "Visualize Noise"
	visualize_toggle.button_pressed = voxel_generator.visualize_noise_values
	visualize_toggle.toggled.connect(func(pressed): voxel_generator.visualize_noise_values = pressed)
	vbox.add_child(visualize_toggle)

	# Verbosity slider
	var hbox = HBoxContainer.new()
	vbox.add_child(hbox)

	var label = Label.new()
	label.text = "Verbosity: "
	hbox.add_child(label)

	var slider = HSlider.new()
	slider.min_value = 0
	slider.max_value = 3
	slider.step = 1
	slider.value = voxel_generator.debug_verbosity
	slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	slider.value_changed.connect(func(value): voxel_generator.debug_verbosity = int(value))
	hbox.add_child(slider)

	# Regenerate button
	var regen_button = Button.new()
	regen_button.text = "Regenerate"
	regen_button.pressed.connect(func(): voxel_generator.generate())
	vbox.add_child(regen_button)

	# Debug slice button
	var slice_button = Button.new()
	slice_button.text = "Draw Noise Slice at Y=0"
	slice_button.pressed.connect(func(): voxel_generator.debug_draw_noise_slice(0.0))
	vbox.add_child(slice_button)

	# Print State button
	var print_button = Button.new()
	print_button.text = "Print Debug State"
	print_button.pressed.connect(func(): voxel_generator.debug_print_state())
	vbox.add_child(print_button)

func _process(_delta):
	# Update FPS counter
	fps_counter.text = "FPS: %d" % [Engine.get_frames_per_second()]
