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
@export var walk_speed = 7.0
@export var sprint_speed = 10.0

# Stamina system
@export_category("Stamina")
@export var stamina: float = 100.0

# Advanced Movement
@export_category("Advanced Movement")
@export var jump_velocity: float = 4.5
@export var mouse_sensitivity: float = 0.003
@export var is_sprinting: bool = false

# References
@onready var picele_camera: Camera3D = $PiceleCamera
@onready var picele_ray_cast: RayCast3D = $PiceleCamera/PiceleRayCast
@onready var oxygen_bar = get_node_or_null("../UI/OxygenBar")
@onready var health_bar = get_node_or_null("../UI/HealthBar")
@onready var depth_gauge = get_node_or_null("../UI/DepthGauge")


func _ready() -> void:
	# Lock mouse cursor to center of screen
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
	add_to_group("player")
	global_position = Vector3(0.0, 5.0, 0.0)
	# Ensure the raycast can hit world geometry (layer 1) while keeping existing masks
	picele_ray_cast.set_collision_mask_value(1, true)

	print("Player initial position set in _ready: ", global_position)

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
	if is_on_floor() and not Input.is_action_pressed("move_up"):
		velocity.y -= gravity * delta
	
	move_and_slide()

	# Handle terraforming and voxel placement
	if Input.is_action_just_pressed("primary_action"):
		terraform()
	
	# Add gravity
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

func terraform():
	var voxel_gen = get_node("/root/World/Terrain/VoxelGenerator") as VoxelGenerator
	picele_ray_cast.force_raycast_update()

	if not picele_ray_cast.is_colliding():
		push_error("Terraform called with no collision hit")
		return

	var hit_position = picele_ray_cast.get_collision_point()
		
	voxel_gen.dig_sphere(hit_position, 0.5, 2.0)
	voxel_gen.regenerate_dirty_chunks() # Manual batching
	print("Terraforming at: ", hit_position)
