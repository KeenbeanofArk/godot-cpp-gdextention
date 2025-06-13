extends CharacterBody3D

# Swimming properties
@export_category("Swimming")
@export var swim_speed = 5.0
@export var swim_acceleration = 4.0
@export var swim_deceleration = 8.0
@export var look_sensitivity = 0.003
@export var buoyancy_force = 1.0
@export var gravity = 9.8

# Walking properties
@export_category("Walking")
@export var walk_speed = 7.0
@export var sprint_speed = 10.0

# Resource collection
@export_category("Resources")
@export var collection_range = 3.0
@export var terraforming_strength = 1.0

# Oxygen system
@export_category("Breathing")
@export var max_oxygen = 100.0
@export var oxygen_depletion_rate = 0.0 # Oxygen lost per second
@export var oxygen_damage_rate = 5.0 # Damage taken when out of oxygen
var current_oxygen = 100.0

# Health system
@export_category("Health")
@export var max_health = 100.0
var current_health = 100.0
var in_habitat = false

# Inventory system
var inventory = {
	"metal": 1000,
	"crystal": 500,
	"coral": 500,
	"algae": 400
}
var max_inventory_size = 100

# References
@onready var camera = $Camera3D
@onready var ray_cast = $Camera3D/RayCast3D
@onready var oxygen_bar = get_node_or_null("../UI/OxygenBar")
@onready var health_bar = get_node_or_null("../UI/HealthBar")
@onready var depth_gauge = get_node_or_null("../UI/DepthGauge")


func _ready():
	# Lock mouse cursor to center of screen
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
	add_to_group("player")
	current_health = max_health
	current_oxygen = max_oxygen
	# Set position immediately - ensure we have closing parenthesis
	# global_position = Vector3.ZERO
	global_position = Vector3(0.0, 20.0, 0.0)

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

	# Highlight resources
	_highlight_resources()
	
	# Handle resource collection, terraforming and voxel placement
	#if Input.is_action_just_pressed("primary_action"):
		#collect_resource()
	#if Input.is_action_just_pressed("secondary_action"):
		#terraform()
	
	# Handle oxygen depletion
	_process_oxygen(delta)
	
	# Update UI
	_update_ui()
	
func _input(event):
	# Camera rotation with mouse
	if event is InputEventMouseMotion:
		rotate_y(-event.relative.x * look_sensitivity)
		camera.rotate_x(-event.relative.y * look_sensitivity)
		camera.rotation.x = clamp(camera.rotation.x, -PI / 2, PI / 2)
	
	# Toggle mouse capture with escape
	if event.is_action_pressed("ui_cancel"):
		if Input.get_mouse_mode() == Input.MOUSE_MODE_CAPTURED:
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
		else:
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func collect_resource():
	if ray_cast.is_colliding():
		var collider = ray_cast.get_collider()
		if collider != null and collider.is_in_group("resources") and collider.has_method("collect"):
			var resource = collider.collect()
			add_to_inventory(resource)

func terraform():
	if ray_cast.is_colliding():
		var collision_point = ray_cast.get_collision_point()
		var terrain = ray_cast.get_collider()
		if terrain != null and terrain.get_parent() == get_parent().get_node("UnderwaterWorld"):
			if terrain.get_parent().has_method("modify_terrain"):
				terrain.get_parent().modify_terrain(collision_point, terraforming_strength)

func add_to_inventory(resource):
	if not inventory.has(resource.type):
		inventory[resource.type] = 0
	
	if get_inventory_weight() < max_inventory_size:
		inventory[resource.type] += resource.amount
		print("Collected ", resource.amount, " of ", resource.type)
		return true
	else:
		print("Inventory full!")
		return false

func get_inventory_weight():
	var total = 0
	for item in inventory:
		total += inventory[item]
	return total

func _highlight_resources():
	# Check if raycast hits a resource
	if ray_cast.is_colliding():
		var collider = ray_cast.get_collider()
		
		# Unhighlight all resources first
		var resources = get_tree().get_nodes_in_group("resources")
		for resource in resources:
			if resource.has_method("highlight_toggle"):
				resource.highlight_toggle(false)
		
		# Highlight the targeted resource
		if collider != null and collider.is_in_group("resources") and collider.has_method("highlight_toggle"):
			collider.highlight_toggle(true)

func _process_oxygen(delta):
	# Check if player is inside a habitat
	var buildings = get_tree().get_nodes_in_group("buildings")
	in_habitat = false
	
	for building in buildings:
		if building.has_method("is_inside") and building.is_inside(global_position):
			in_habitat = true
			break
	
	# Manage oxygen levels
	if in_habitat:
		# Replenish oxygen when in habitat
		current_oxygen = min(current_oxygen + (oxygen_depletion_rate * 2 * delta), max_oxygen)
	else:
		# Deplete oxygen when underwater
		current_oxygen = max(current_oxygen - (oxygen_depletion_rate * delta), 0)
		
		# Apply damage when out of oxygen
		if current_oxygen <= 0:
			take_damage(oxygen_damage_rate * delta)

func take_damage(amount):
	current_health = max(current_health - amount, 0)
	if current_health <= 0:
		_die()

func heal(amount):
	current_health = min(current_health + amount, max_health)

func _die():
	# Handle player death
	print("Player died!")
	# Could show death screen, restart level, etc.
	# For now just reset health and teleport to origin
	current_health = max_health
	current_oxygen = max_oxygen
	var t = global_transform
	t.origin = Vector3.ZERO
	global_transform = t
	
func _update_ui():
	if oxygen_bar:
		oxygen_bar.value = current_oxygen
	
	if health_bar:
		health_bar.value = current_health
	
	if depth_gauge:
		# Display depth in meters (negative Y position)
		var depth = - global_position.y
		depth_gauge.text = "Depth: %d m" % [int(depth)]

func _place_building():
	# Delegate to building system
	pass
