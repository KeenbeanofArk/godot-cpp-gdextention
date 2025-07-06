extends Node3D

@onready var voxel_generator = $VoxelGenerator
@onready var chunk = $Chunk
@onready var fps_counter = $UI/FPS/FPSLabel

func _ready():
	# Setup UI elements
	create_debug_ui()
	
	# Enable debug mode
	voxel_generator.debug_debug_mode = true
	voxel_generator.debug_debug_verbosity = 2
	voxel_generator.voxel_generator_auto_generate = true
	voxel_generator.voxel_generator_gen_size_x = 1
	voxel_generator.voxel_generator_gen_size_y = 1
	voxel_generator.voxel_generator_gen_size_z = 1
	voxel_generator.resolution = 1
	voxel_generator.show_centers = true
	voxel_generator.show_grid = true
	voxel_generator.debug_visualize_noise_values = false
	voxel_generator.cutoff = 0.1
	voxel_generator.seeder = 1234
	
	# Print initial state
	voxel_generator.debug_print_state()
	
	# Start the generator
	voxel_generator.generate()
	
	#chunk.generate()
	

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
	debug_toggle.button_pressed = voxel_generator.debug_debug_mode
	debug_toggle.toggled.connect(func(pressed): voxel_generator.debug_debug_mode = pressed)
	vbox.add_child(debug_toggle)

	# Visualize toggle
	var visualize_toggle = CheckButton.new()
	visualize_toggle.text = "Visualize Noise"
	visualize_toggle.button_pressed = voxel_generator.debug_visualize_noise_values
	visualize_toggle.toggled.connect(func(pressed): voxel_generator.debug_visualize_noise_values = pressed)
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
	slider.value = voxel_generator.debug_debug_verbosity
	slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	slider.value_changed.connect(func(value): voxel_generator.debug_debug_verbosity = int(value))
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
	
	 # Add chunk size slider
	var chunk_size_hbox = HBoxContainer.new()
	vbox.add_child(chunk_size_hbox)
	
	var chunk_size_label = Label.new()
	chunk_size_label.text = "Chunk Size: "
	chunk_size_hbox.add_child(chunk_size_label)

	var chunk_size_slider = HSlider.new()
	chunk_size_slider.min_value = 8
	chunk_size_slider.max_value = 64
	chunk_size_slider.step = 8
	chunk_size_slider.value = voxel_generator.voxel_generator_chunk_size
	chunk_size_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	chunk_size_slider.value_changed.connect(func(value): voxel_generator.voxel_generator_chunk_size = int(value))
	chunk_size_hbox.add_child(chunk_size_slider)

func _process(_delta):
	# Update FPS counter
	fps_counter.text = "FPS: %d" % [Engine.get_frames_per_second()]
