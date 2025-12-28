extends CharacterBody3D
class_name Picele # Picture Element or Pixel

# Swimming properties
@export_category("Swimming")
@export var swim_speed = 5.0
@export var swim_acceleration = 4.0
@export var swim_deceleration = 8.0
@export var look_sensitivity = 0.003
@export var buoyancy_force = 0.0 # Normally 1.0
@export var gravity = 9.8

# Walking properties
@export_category("Walking")
@export var walk_speed = 75.0
@export var sprint_speed = 12.0

# Stamina system
@export_category("Stamina")
@export var stamina: float = 100.0

# Advanced Movement
@export_category("Advanced Movement")
@export var jump_velocity: float = 4.5
@export var mouse_sensitivity: float = 0.003
@export var is_sprinting: bool = false
@export var is_gravity: bool = false

# Terraform Settings
@export_category("Terraform Settings")
@export var dig_radius: float = 1.0
@export var dig_strength: float = 2.0
@export var build_radius: float = 1.0
@export var build_strength: float = 2.0

# References
@onready var picele_camera: Camera3D = $PiceleCamera
@onready var picele_ray_cast: RayCast3D = $PiceleCamera/PiceleRayCast
@onready var gui_node: Node = get_node_or_null("/root/World/CentralDebugGUI")

var terrain_manager: MultiTerrainManager = null
var voxel_generator_plains: VoxelGenerator

func _ready() -> void:
	# Get references after node is in tree
	terrain_manager = get_node_or_null("/root/MultiTerrainManager")
	voxel_generator_plains = get_node_or_null("../Plains/VoxelEngine/VoxelGenerator")
	
	if terrain_manager == null:
		push_error("[Picele] MultiTerrainManager not found as autoload")
	
	if voxel_generator_plains == null:
		push_error("[Picele] VoxelGenPlains not found at ../Plains/VoxelEngine/VoxelGenerator")
		
	# Lock mouse cursor to center of screen
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
	add_to_group("player")
	global_position = Vector3(0.0, 15.0, 0.0)
	# Ensure the raycast can hit world geometry (layer 1) while keeping existing masks
	picele_ray_cast.set_collision_mask_value(1, true)

	print("[Picele] Picele initial position set in _ready: ", global_position)

func _physics_process(delta):
	# Get input direction
	var input_dir = Input.get_vector("move_left", "move_right", "move_forward", "move_backward")
	var direction = (transform.basis * Vector3(input_dir.x, 0, input_dir.y)).normalized()
	
	# Handle vertical movement (swimming up/down)
	if Input.is_action_pressed("move_up"):
		direction.y += 1.0
	if Input.is_action_pressed("move_down"):
		direction.y -= 1.0
	
	# Apply buoyancy (slight upward force underwater)
	if not is_on_floor():
		direction.y += buoyancy_force * delta
	
	# Calculate velocity
	var target_velocity = direction * swim_speed
	
	# Apply acceleration or deceleration
	if direction != Vector3.ZERO:
		velocity = velocity.lerp(target_velocity, swim_acceleration * delta)
	else:
		velocity = velocity.lerp(Vector3.ZERO, swim_deceleration * delta)
	
	# Apply gravity when on land
	if is_gravity:
		if is_on_floor() and not Input.is_action_pressed("move_up"):
			velocity.y -= gravity * delta
	
	move_and_slide()

	# Disable terraforming when GUI is open
	if gui_node and (gui_node.debug_display.visible or gui_node.terraform_settings.visible):
		return

	# Handle terraforming and voxel placement
	if Input.is_action_just_pressed("primary_action"):
		terraform_dig()
	
	# Handle terraforming and voxel placement
	if Input.is_action_just_pressed("secondary_action"):
		terraform_build()
	
	# Add gravity
	if is_gravity:
		if not is_on_floor():
			velocity += get_gravity() * delta

	# Handle jump
	if Input.is_action_just_pressed("jump") and is_on_floor():
		if stamina >= 10.0:
			velocity.y = jump_velocity
			stamina -= 5.0 # Jump cost
	
	# Handle sprint
	is_sprinting = Input.is_action_pressed("sprint") and stamina > 0.0
	
func _input(event):
	# Only process camera rotation when mouse is captured
	if event is InputEventMouseMotion and Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
		rotate_y(-event.relative.x * look_sensitivity)
		picele_camera.rotate_x(-event.relative.y * look_sensitivity)
		picele_camera.rotation.x = clamp(picele_camera.rotation.x, -PI / 2, PI / 2)
	
	# Toggle mouse capture with escape
	if event.is_action_pressed("ui_cancel"):
		if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
		else:
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func terraform_dig():
	if not terrain_manager:
		push_error("[Picele] Terraform - MultiTerrainManager not available")
		return
	
	var raycast_info = terrain_manager.get_raycast_info()
	
	if not raycast_info.get("hit", false):
		print("[Picele] Terraform called with no collision hit")
		return
	
	var hit_position = raycast_info["position"]
	var all_generators = terrain_manager.get_all_voxel_generators()
	
	# Apply terraform to all terrain generators (each will handle collisions independently)
	for voxel_gen in all_generators:
		voxel_gen.dig_sphere(hit_position, dig_radius, dig_strength)
	
	var distance = raycast_info["distance"]
	print("[Picele] Terraforming at: %s (distance: %.2f)" % [hit_position, distance])

func terraform_build():
	if not terrain_manager:
		push_error("[Picele] Terraform - MultiTerrainManager not available")
		return
	
	var raycast_info = terrain_manager.get_raycast_info()
	
	if not raycast_info.get("hit", false):
		print("[Picele] Terraform called with no collision hit")
		return
	
	var hit_position = raycast_info["position"]
	var all_generators = terrain_manager.get_all_voxel_generators()
	
	# Apply terraform to all terrain generators (each will handle collisions independently)
	for voxel_gen in all_generators:
		voxel_gen.build_sphere(hit_position, build_radius, build_strength)
	
	var distance = raycast_info["distance"]
	print("[Picele] Terraforming at: %s (distance: %.2f)" % [hit_position, distance])
