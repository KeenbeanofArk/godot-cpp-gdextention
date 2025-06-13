extends VoxelGenerator

# Using self instead of trying to find a child node
# since this script is attached directly to the VoxelGenerator node

func _ready():
	# Setup UI elements
	create_debug_ui()
	
	# Enable debug mode
	debug_mode = true
	debug_verbosity = 2
	auto_generate = false
	chunk_size = 8
	resolution = 4
	show_centers = false
	show_grid = false
	
	# Print initial state
	debug_print_state()
	
	# Start the generator
	generate()

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
	debug_toggle.button_pressed = debug_mode
	debug_toggle.toggled.connect(func(pressed): debug_mode = pressed)
	vbox.add_child(debug_toggle)

	# Visualize toggle
	var visualize_toggle = CheckButton.new()
	visualize_toggle.text = "Visualize Noise"
	visualize_toggle.button_pressed = visualize_noise_values
	visualize_toggle.toggled.connect(func(pressed): visualize_noise_values = pressed)
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
	slider.value = debug_verbosity
	slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	slider.value_changed.connect(func(value): debug_verbosity = int(value))
	hbox.add_child(slider)

	# Regenerate button
	var regen_button = Button.new()
	regen_button.text = "Regenerate"
	regen_button.pressed.connect(func(): generate())
	vbox.add_child(regen_button)

	# Debug slice button
	var slice_button = Button.new()
	slice_button.text = "Draw Noise Slice at Y=0"
	slice_button.pressed.connect(func(): debug_draw_noise_slice(0.0))
	vbox.add_child(slice_button)

	# Print State button
	var print_button = Button.new()
	print_button.text = "Print Debug State"
	print_button.pressed.connect(func(): debug_print_state())
	vbox.add_child(print_button)

func _process(_delta):
	# You can add additional debug functionality here
	pass
