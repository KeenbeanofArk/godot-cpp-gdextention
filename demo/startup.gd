extends Control

# Improved startup scene controller with better UI
# Lists saved maps and lets the user pick one or start new.
# If a map is selected the script sets StartupState.pending_map and then opens world.tscn.

@onready var main_panel: Panel = $MainPanel
@onready var title_label: Label = $MainPanel/TitleLabel
@onready var subtitle: Label = $MainPanel/Subtitle
@onready var new_map_btn: Button = null
@onready var load_map_btn: Button = null
@onready var exit_btn: Button = null
@onready var map_list: ItemList = null

# Terrain buttons
var terrain_buttons: Dictionary = {}  # terrain_name -> Button
var selected_terrain: String = "Plains"
var terrain_names: Array = ["Plains", "Mountains", "Forest", "Desert", "Swamp", "TerrainMultiBiome"]
var terrain_positions: Dictionary = {
	"Plains": 300,
	"Mountains": 550,
	"Forest": 800,
	"Desert": 1050,
	"Swamp": 1300,
	"TerrainMultiBiome": 1550
}

func _ready() -> void:
	_create_startup_ui()

func _create_startup_ui() -> void:
	# Setup main panel
	main_panel.size = Vector2(2400, 1600)
	main_panel.position = (get_viewport_rect().size - main_panel.size) / 2

	# Panel background style
	var style = StyleBoxFlat.new()
	style.bg_color = Color(0.1, 0.1, 0.2, 0.9)
	style.border_color = Color(0.3, 0.3, 0.5)
	style.border_width_left = 2
	style.border_width_right = 2
	style.border_width_top = 2
	style.border_width_bottom = 2
	style.corner_radius_top_left = 10
	style.corner_radius_top_right = 10
	style.corner_radius_bottom_left = 10
	style.corner_radius_bottom_right = 10
	main_panel.add_theme_stylebox_override("panel", style)

	# Title
	title_label.text = "Voxel Engine"
	title_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	title_label.size = Vector2(600, 40)
	title_label.add_theme_font_size_override("font_size", 40)
	title_label.add_theme_color_override("font_color", Color(1, 1, 1))

	# Subtitle
	subtitle.text = "Choose an option to start"
	subtitle.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	subtitle.position = Vector2(20, 70)
	subtitle.size = Vector2(600, 30)
	subtitle.add_theme_font_size_override("font_size", 20)
	subtitle.add_theme_color_override("font_color", Color(0.8, 0.8, 0.8))

	# New Map button
	new_map_btn = Button.new()
	new_map_btn.text = "New Map"
	new_map_btn.position = Vector2(50, 120)
	new_map_btn.size = Vector2(200, 50)
	new_map_btn.add_theme_font_size_override("font_size", 20)
	new_map_btn.pressed.connect(_on_new_map_pressed)
	main_panel.add_child(new_map_btn)

	# Load Map button
	load_map_btn = Button.new()
	load_map_btn.text = "Load Map"
	load_map_btn.position = Vector2(50, 180)
	load_map_btn.size = Vector2(200, 50)
	load_map_btn.add_theme_font_size_override("font_size", 20)
	load_map_btn.pressed.connect(_on_load_map_pressed)
	main_panel.add_child(load_map_btn)

	# Create terrain selection buttons (Y=120)
	_create_terrain_buttons()

	# Map list (initially hidden)
	map_list = ItemList.new()
	map_list.position = Vector2(300, 120)
	map_list.size = Vector2(250, 250)
	map_list.visible = false
	main_panel.add_child(map_list)

	# Load selected map button (for list)
	var load_selected_btn = Button.new()
	load_selected_btn.text = "Load Selected"
	load_selected_btn.position = Vector2(300, 380)
	load_selected_btn.size = Vector2(120, 40)
	load_selected_btn.visible = false
	load_selected_btn.pressed.connect(_on_load_selected_pressed)
	main_panel.add_child(load_selected_btn)

	# Back button (for list)
	var back_btn = Button.new()
	back_btn.text = "Back"
	back_btn.position = Vector2(430, 380)
	back_btn.size = Vector2(120, 40)
	back_btn.visible = false
	back_btn.pressed.connect(_on_back_pressed)
	main_panel.add_child(back_btn)

	# Exit button
	exit_btn = Button.new()
	exit_btn.text = "Exit"
	exit_btn.position = Vector2(250, 440)
	exit_btn.size = Vector2(100, 40)
	exit_btn.pressed.connect(_on_exit_pressed)
	main_panel.add_child(exit_btn)

	# Store references for later
	main_panel.set_meta("load_selected_btn", load_selected_btn)
	main_panel.set_meta("back_btn", back_btn)

func _create_terrain_buttons() -> void:
	# Create terrain selection buttons at Y=120 with specified X offsets
	for terrain_name in terrain_names:
		var btn = Button.new()
		btn.text = terrain_name
		btn.position = Vector2(terrain_positions[terrain_name], 120)
		btn.size = Vector2(200, 50)
		btn.add_theme_font_size_override("font_size", 16)
		btn.pressed.connect(_on_terrain_selected.bind(terrain_name))
		main_panel.add_child(btn)
		terrain_buttons[terrain_name] = btn
	
	# Highlight Plains as default selected terrain
	_update_terrain_button_highlights()

func _on_terrain_selected(terrain_name: String) -> void:
	selected_terrain = terrain_name
	
	# Update StartupState pending terrain
	var ss = get_node_or_null("/root/StartupState")
	if ss:
		ss.set_pending_terrain(terrain_name)
	
	# Update button highlights
	_update_terrain_button_highlights()

func _update_terrain_button_highlights() -> void:
	# Update all terrain buttons - highlight selected, normal for others
	for terrain_name in terrain_names:
		var btn = terrain_buttons[terrain_name]
		if terrain_name == selected_terrain:
			# Highlight the selected terrain with green text
			btn.add_theme_color_override("font_color", Color(0.0, 1.0, 0.0))  # Green
		else:
			# Normal white text for non-selected
			btn.add_theme_color_override("font_color", Color(1.0, 1.0, 1.0))  # White

func _on_new_map_pressed() -> void:
	# Start new map
	var ss = get_node_or_null("/root/StartupState")
	if ss:
		ss.pending_map = ""  # Empty means new map
		ss.set_pending_terrain(selected_terrain)  # Set the selected terrain
	_start_world()

func _on_load_map_pressed() -> void:
	# Show map list
	_populate_map_list()
	map_list.visible = true
	main_panel.get_meta("load_selected_btn").visible = true
	main_panel.get_meta("back_btn").visible = true
	new_map_btn.visible = false
	load_map_btn.visible = false

func _populate_map_list() -> void:
	var da = DirAccess.open("user://saved_maps")
	var maps = []
	if da:
		da.list_dir_begin()
		var map_name = da.get_next()
		while map_name != "":
			if da.current_is_dir() and map_name != "." and map_name != "..":
				maps.append(map_name)
			map_name = da.get_next()
		da.list_dir_end()

	map_list.clear()
	for m in maps:
		map_list.add_item(m)
	if maps.size() > 0:
		map_list.select(0)

func _on_load_selected_pressed() -> void:
	var sel = map_list.get_selected_items()
	if sel.size() == 0:
		return
	var idx = sel[0]
	var map_name = map_list.get_item_text(idx)
	
	var ss = get_node_or_null("/root/StartupState")
	if ss:
		ss.pending_map = map_name
		ss.set_pending_terrain(selected_terrain)  # Set the selected terrain
	_start_world()

func _on_back_pressed() -> void:
	# Hide map list
	map_list.visible = false
	main_panel.get_meta("load_selected_btn").visible = false
	main_panel.get_meta("back_btn").visible = false
	new_map_btn.visible = true
	load_map_btn.visible = true

func _on_exit_pressed() -> void:
	get_tree().quit()
	
func _start_world() -> void:
	# Replace with the project's world scene path
	var scene_path := "res://world.tscn"
	# Resolve to an absolute filesystem path and check file existence
	var fs_path = ProjectSettings.globalize_path(scene_path)
	if not FileAccess.file_exists(fs_path):
		push_error("[Startup] World scene not found: %s" % scene_path)
		return

	var tree = get_tree()
	if tree == null:
		push_error("[Startup] Cannot change scene: SceneTree is null (get_tree() returned null). Make sure this node is in the scene tree and ready.")
		return

	var err = tree.change_scene_to_file(scene_path)
	if err != OK:
		push_error("[Startup] Failed to open world.tscn from startup scene (change_scene_to_file returned %d)" % err)
