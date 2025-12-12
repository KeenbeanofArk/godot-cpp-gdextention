extends Node3D
## Marching Cubes Case Visualizer
## Displays all 256 cases with corners, edges, and generated triangles.
## Use Previous/Next buttons or the spinner to navigate through cases.

@onready var case_label: Label = $UI/PanelContainer/VBoxContainer/CaseLabel
@onready var triangle_count_label: Label = $UI/PanelContainer/VBoxContainer/TriangleCountLabel
@onready var prev_button: Button = $UI/PanelContainer/VBoxContainer/HBoxContainer/PreviousButton
@onready var next_button: Button = $UI/PanelContainer/VBoxContainer/HBoxContainer/NextButton
@onready var case_spinner: SpinBox = $UI/PanelContainer/VBoxContainer/CaseSpinner
@onready var visualization: Node3D = $Visualization

var current_case: int = 0
var mesh_instance: MeshInstance3D
var corner_spheres: Array[MeshInstance3D] = []
var edge_lines: MeshInstance3D

# Rotation variables
var rotation_x: float = 0.45 # Initial tilt
var rotation_y: float = 0.8 # Initial spin
const ROTATION_SPEED: float = 2.0 # Radians per second when key is held

# Cube corner positions (8 corners of a unit cube)
const CORNER_POSITIONS = [
	Vector3(0, 0, 0), # 0: bottom-left-front
	Vector3(1, 0, 0), # 1: bottom-right-front
	Vector3(1, 1, 0), # 2: bottom-right-back
	Vector3(0, 1, 0), # 3: bottom-left-back
	Vector3(0, 0, 1), # 4: top-left-front
	Vector3(1, 0, 1), # 5: top-right-front
	Vector3(1, 1, 1), # 6: top-right-back
	Vector3(0, 1, 1), # 7: top-left-back
]

# Edge definitions (which two corners form each edge)
const EDGES = [
	[0, 1], [1, 2], [2, 3], [3, 0], # Bottom face (z=0)
	[4, 5], [5, 6], [6, 7], [7, 4], # Top face (z=1)
	[0, 4], [1, 5], [2, 6], [3, 7] # Vertical edges
]

# Corner index from edge lookup (from Constants.h marching cubes)
const CORNER_INDEX_A_FROM_EDGE = [
	0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3
]
const CORNER_INDEX_B_FROM_EDGE = [
	1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7
]

# Hardcoded marching cubes lookup table (256 cases)
const MARCHING_TRIANGLES = [
	[],
	[0, 8, 3],
	[0, 1, 9],
	[1, 8, 3, 9, 8, 1],
	[1, 2, 10],
	[0, 8, 3, 1, 2, 10],
	[9, 2, 10, 0, 2, 9],
	[2, 8, 3, 10, 8, 2, 10, 9, 8],
	[3, 11, 2],
	[0, 11, 2, 8, 11, 0],
	[1, 9, 0, 2, 3, 11],
	[1, 11, 2, 1, 9, 11, 9, 8, 11],
	[3, 10, 1, 11, 10, 3],
	[0, 10, 1, 0, 8, 10, 8, 11, 10],
	[3, 9, 0, 3, 11, 9, 11, 10, 9],
	[9, 8, 10, 11, 10, 8],
	[4, 7, 8],
	[4, 3, 0, 7, 3, 4],
	[0, 1, 9, 8, 4, 7],
	[4, 1, 9, 4, 7, 1, 7, 3, 1],
	[1, 2, 10, 8, 4, 7],
	[3, 4, 7, 3, 0, 4, 1, 2, 10],
	[9, 2, 10, 9, 0, 2, 8, 4, 7],
	[2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 4, 9],
	[8, 4, 7, 3, 11, 2],
	[11, 4, 7, 11, 2, 4, 2, 0, 4],
	[9, 0, 1, 8, 4, 7, 2, 3, 11],
	[4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1],
	[3, 10, 1, 3, 11, 10, 7, 8, 4],
	[1, 11, 10, 7, 11, 1, 7, 1, 4, 0, 7, 4],
	[4, 7, 8, 9, 0, 11, 9, 11, 10, 0, 3, 11],
	[4, 7, 11, 4, 11, 9, 9, 11, 10],
	[9, 5, 4],
	[9, 5, 4, 0, 8, 3],
	[0, 5, 4, 1, 5, 0],
	[8, 5, 4, 8, 3, 5, 3, 1, 5],
	[1, 2, 10, 9, 5, 4],
	[3, 0, 8, 1, 2, 10, 4, 9, 5],
	[5, 2, 10, 5, 4, 2, 4, 0, 2],
	[2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8],
	[9, 5, 4, 2, 3, 11],
	[0, 11, 2, 0, 8, 11, 4, 9, 5],
	[0, 5, 4, 0, 1, 5, 2, 3, 11],
	[2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5],
	[10, 3, 11, 10, 1, 3, 9, 5, 4],
	[4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10],
	[5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3],
	[5, 4, 8, 5, 8, 10, 10, 8, 11],
	[9, 7, 8, 5, 7, 9],
	[9, 3, 0, 9, 5, 3, 5, 7, 3],
	[0, 7, 8, 0, 1, 7, 1, 5, 7],
	[1, 5, 3, 7, 3, 5],
	[9, 7, 8, 9, 5, 7, 10, 1, 2],
	[10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3],
	[8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2],
	[2, 10, 5, 2, 5, 3, 3, 5, 7],
	[7, 9, 5, 7, 8, 9, 3, 11, 2],
	[9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11],
	[2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7],
	[11, 2, 1, 11, 1, 7, 7, 1, 5],
	[3, 11, 1, 3, 1, 10, 7, 8, 9, 7, 9, 5],
	[11, 10, 7, 10, 1, 7, 1, 0, 7, 0, 9, 7, 9, 5, 7],
	[3, 11, 10, 0, 3, 10, 0, 10, 5, 0, 5, 8, 8, 5, 7],
	[11, 10, 5, 7, 11, 5],
	[10, 6, 5],
	[0, 8, 3, 5, 10, 6],
	[9, 0, 1, 5, 10, 6],
	[1, 8, 3, 1, 9, 8, 5, 10, 6],
	[1, 6, 5, 2, 6, 1],
	[1, 6, 5, 1, 2, 6, 3, 0, 8],
	[9, 6, 5, 9, 0, 6, 0, 2, 6],
	[5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8],
	[2, 3, 11, 10, 6, 5],
	[11, 0, 8, 11, 2, 0, 10, 6, 5],
	[0, 1, 9, 2, 3, 11, 5, 10, 6],
	[5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11],
	[6, 3, 11, 6, 5, 3, 5, 1, 3],
	[0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6],
	[3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9],
	[6, 5, 9, 6, 9, 11, 11, 9, 8],
	[5, 10, 6, 4, 7, 8],
	[4, 3, 0, 4, 7, 3, 6, 5, 10],
	[1, 9, 0, 5, 10, 6, 8, 4, 7],
	[10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 4, 9],
	[6, 1, 2, 6, 5, 1, 4, 7, 8],
	[1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7],
	[8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6],
	[7, 3, 4, 4, 3, 2, 4, 2, 6, 2, 5, 6],
	[5, 10, 6, 7, 8, 4, 2, 3, 11],
	[4, 7, 11, 4, 11, 2, 4, 2, 0, 2, 11, 6, 2, 6, 5, 10],
	[2, 3, 11, 6, 5, 10, 4, 7, 8, 9, 0, 1],
	[1, 6, 2, 5, 6, 1, 3, 4, 7, 3, 9, 4, 3, 1, 9],
	[6, 1, 5, 6, 3, 1, 3, 7, 1, 7, 4, 1],
	[7, 6, 5, 7, 5, 4, 3, 6, 1, 3, 1, 0, 0, 1, 4],
	[9, 0, 4, 5, 6, 3, 5, 3, 1, 6, 3, 7],
	[5, 6, 4, 6, 3, 4, 6, 1, 3, 4, 1, 9],
	[0, 3, 8, 5, 10, 2, 5, 2, 6, 1, 2, 10],
	[6, 5, 10, 2, 1, 9, 2, 9, 0, 8, 0, 3],
]

func _ready() -> void:
	# Create mesh instance for triangles
	mesh_instance = MeshInstance3D.new()
	visualization.add_child(mesh_instance)
	
	# Create edge line visualization
	edge_lines = MeshInstance3D.new()
	visualization.add_child(edge_lines)
	
	# Create corner spheres
	for i in range(8):
		var sphere = MeshInstance3D.new()
		sphere.mesh = SphereMesh.new()
		sphere.mesh.radius = 0.08
		sphere.mesh.height = 0.16
		visualization.add_child(sphere)
		corner_spheres.append(sphere)
	
	# Connect buttons
	prev_button.pressed.connect(_on_previous_pressed)
	next_button.pressed.connect(_on_next_pressed)
	case_spinner.value_changed.connect(_on_case_changed)
	
	# Initial setup
	update_visualization()
	
	# Enable processing for input handling
	set_process(true)

func _on_previous_pressed() -> void:
	current_case = wrapi(current_case - 1, 0, 256)
	case_spinner.set_value_no_signal(current_case)
	update_visualization()

func _on_next_pressed() -> void:
	current_case = wrapi(current_case + 1, 0, 256)
	case_spinner.set_value_no_signal(current_case)
	update_visualization()

func _on_case_changed(value: float) -> void:
	current_case = int(value) % 256
	update_visualization()

func _process(delta: float) -> void:
	# Handle arrow key rotation
	if Input.is_action_pressed("ui_left"):
		rotation_y += ROTATION_SPEED * delta
	if Input.is_action_pressed("ui_right"):
		rotation_y -= ROTATION_SPEED * delta
	if Input.is_action_pressed("ui_up"):
		rotation_x += ROTATION_SPEED * delta
	if Input.is_action_pressed("ui_down"):
		rotation_x -= ROTATION_SPEED * delta
	
	# Apply rotation to visualization node
	visualization.rotation.x = rotation_x
	visualization.rotation.y = rotation_y

func update_visualization() -> void:
	# Update labels
	case_label.text = "Case: %d / 255" % current_case
	
	# Determine which corners are inside (solid) based on case index binary representation
	var inside_corners = []
	for i in range(8):
		if current_case & (1 << i):
			inside_corners.append(i)
	
	# Create binary representation label
	var binary_str = ""
	for i in range(7, -1, -1):
		binary_str += "1" if (current_case & (1 << i)) else "0"
	
	# Update corner sphere colors (red = inside/solid, blue = outside/air)
	for i in range(8):
		var material = StandardMaterial3D.new()
		var is_inside = i in inside_corners
		material.albedo_color = Color.RED if is_inside else Color.BLUE
		material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
		
		corner_spheres[i].position = CORNER_POSITIONS[i]
		corner_spheres[i].set_surface_override_material(0, material)
	
	# Draw cube edges (gray lines)
	draw_edges()
	
	# Generate and display triangles for this case
	var triangles = get_triangles_for_case(current_case)
	var triangle_count = int(float(triangles.size()) / 3.0)
	triangle_count_label.text = "Triangles: %d (Binary: %s)" % [triangle_count, binary_str]
	
	draw_triangles(triangles)

func draw_edges() -> void:
	var immediate_mesh = ImmediateMesh.new()
	immediate_mesh.surface_begin(Mesh.PRIMITIVE_LINES)
	
	for edge in EDGES:
		var p1 = CORNER_POSITIONS[edge[0]]
		var p2 = CORNER_POSITIONS[edge[1]]
		
		immediate_mesh.surface_set_color(Color.GRAY)
		immediate_mesh.surface_add_vertex(p1)
		immediate_mesh.surface_add_vertex(p2)
	
	immediate_mesh.surface_end()
	
	var line_material = StandardMaterial3D.new()
	line_material.albedo_color = Color.GRAY
	line_material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	
	edge_lines.mesh = immediate_mesh
	edge_lines.set_surface_override_material(0, line_material)

func draw_triangles(vertices: PackedVector3Array) -> void:
	if vertices.is_empty():
		mesh_instance.mesh = null
		return
	
	var array_mesh = ArrayMesh.new()
	var surface_array = []
	surface_array.resize(Mesh.ARRAY_MAX)
	surface_array[Mesh.ARRAY_VERTEX] = vertices
	
	# Generate normals
	var normals = PackedVector3Array()
	for i in range(0, vertices.size(), 3):
		var v0 = vertices[i]
		var v1 = vertices[i + 1]
		var v2 = vertices[i + 2]
		
		var normal = (v1 - v0).cross(v2 - v0).normalized()
		normals.push_back(normal)
		normals.push_back(normal)
		normals.push_back(normal)
	
	surface_array[Mesh.ARRAY_NORMAL] = normals
	
	# Add colors based on position for better visibility
	var colors = PackedColorArray()
	for v in vertices:
		var color = Color(v.x, v.y, v.z)
		color.a = 0.85
		colors.push_back(color)
	surface_array[Mesh.ARRAY_COLOR] = colors
	
	array_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, surface_array)
	
	var material = StandardMaterial3D.new()
	material.albedo_color = Color.WHITE
	# Godot 4.x: Vertex colors work automatically without flag_use_vertex_colors
	material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	material.cull_mode = BaseMaterial3D.CULL_DISABLED
	material.shading_mode = BaseMaterial3D.SHADING_MODE_PER_PIXEL
	
	array_mesh.surface_set_material(0, material)
	mesh_instance.mesh = array_mesh

func get_triangles_for_case(case_index: int) -> PackedVector3Array:
	if case_index < 0 or case_index >= MARCHING_TRIANGLES.size():
		return PackedVector3Array()
	
	var triangle_indices = MARCHING_TRIANGLES[case_index]
	if triangle_indices.is_empty():
		return PackedVector3Array()
	
	var vertices = PackedVector3Array()
	
	# Process triangle indices in groups of 3 (each triangle)
	for i in range(0, triangle_indices.size(), 3):
		if i + 2 >= triangle_indices.size():
			break
		
		var edge1 = triangle_indices[i]
		var edge2 = triangle_indices[i + 1]
		var edge3 = triangle_indices[i + 2]
		
		if edge1 < 0 or edge2 < 0 or edge3 < 0 or edge1 >= 12 or edge2 >= 12 or edge3 >= 12:
			break
		
		# Get corner positions for each edge and interpolate to get vertex position
		var corner_a1 = CORNER_INDEX_A_FROM_EDGE[edge1]
		var corner_b1 = CORNER_INDEX_B_FROM_EDGE[edge1]
		var pos1 = CORNER_POSITIONS[corner_a1].lerp(CORNER_POSITIONS[corner_b1], 0.5)
		
		var corner_a2 = CORNER_INDEX_A_FROM_EDGE[edge2]
		var corner_b2 = CORNER_INDEX_B_FROM_EDGE[edge2]
		var pos2 = CORNER_POSITIONS[corner_a2].lerp(CORNER_POSITIONS[corner_b2], 0.5)
		
		var corner_a3 = CORNER_INDEX_A_FROM_EDGE[edge3]
		var corner_b3 = CORNER_INDEX_B_FROM_EDGE[edge3]
		var pos3 = CORNER_POSITIONS[corner_a3].lerp(CORNER_POSITIONS[corner_b3], 0.5)
		
		vertices.push_back(pos1)
		vertices.push_back(pos2)
		vertices.push_back(pos3)
	
	return vertices
