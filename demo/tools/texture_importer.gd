@tool
extends Node
class_name TextureImporter

## Tool script to import PBR textures into the voxel engine
## Add this to any Node in your scene and call import_textures() from the Output console

const TEXTURE_BASE_PATH = "res://assets/textures/ground/"
const TEXTURE_NAME = "Ground079S_1K-PNG"

class TextureSet:
	var color: Texture2D
	var normal_dx: Texture2D
	var normal_gl: Texture2D
	var roughness: Texture2D
	var ambient_occlusion: Texture2D
	var displacement: Texture2D

## Load all texture maps from disk
func load_texture_set() -> TextureSet:
	var textures = TextureSet.new()
	
	textures.color = load(TEXTURE_BASE_PATH + TEXTURE_NAME + "_Color.png")
	textures.normal_dx = load(TEXTURE_BASE_PATH + TEXTURE_NAME + "_NormalDX.png")
	textures.normal_gl = load(TEXTURE_BASE_PATH + TEXTURE_NAME + "_NormalGL.png")
	textures.roughness = load(TEXTURE_BASE_PATH + TEXTURE_NAME + "_Roughness.png")
	textures.ambient_occlusion = load(TEXTURE_BASE_PATH + TEXTURE_NAME + "_AmbientOcclusion.png")
	textures.displacement = load(TEXTURE_BASE_PATH + TEXTURE_NAME + "_Displacement.png")
	
	# Validate all textures loaded
	if not textures.color:
		push_error("Failed to load color texture")
		return null
	if not textures.normal_dx:
		push_error("Failed to load normal DX texture")
		return null
	if not textures.roughness:
		push_error("Failed to load roughness texture")
		return null
	if not textures.ambient_occlusion:
		push_error("Failed to load ambient occlusion texture")
		return null
	if not textures.displacement:
		push_error("Failed to load displacement texture")
		return null
	
	print("[TextureImporter] Successfully loaded all texture maps")
	return textures

## Create a StandardMaterial3D with the loaded textures
func create_pbr_material(textures: TextureSet) -> StandardMaterial3D:
	var material = StandardMaterial3D.new()
	
	# Set texture maps
	material.albedo_texture = textures.color
	material.normal_texture = textures.normal_gl # Use GL format for Godot
	material.roughness_texture = textures.roughness
	material.ao_texture = textures.ambient_occlusion
	
	# Configure material properties
	material.roughness = 0.7
	material.metallic = 0.0
	material.normal_scale = 1.0
	material.ao_light_affect = 0.5
	
	# Use triplanar mapping for better voxel terrain coverage
	material.uv1_triplanar = true
	material.uv1_triplanar_sharpness = 2.0
	material.uv1_scale = Vector3(4.0, 4.0, 4.0) # Adjust UV scale to fit voxels
	
	print("[TextureImporter] Created PBR material")
	return material

## Apply material to all VoxelGenerator nodes in the scene
func apply_material_to_generators(material: Material) -> void:
	var root = get_tree().root if is_node_ready() else get_parent()
	if not root:
		push_error("[TextureImporter] Cannot find scene root")
		return
	
	var generators = root.find_children("*", "VoxelGenerator", true, false)
	
	if generators.is_empty():
		push_warning("[TextureImporter] No VoxelGenerator nodes found in scene")
	else:
		for generator in generators:
			if generator.has_method("set_terrain_material"):
				generator.set_terrain_material(material)
				print("[TextureImporter] Applied material to generator: %s" % generator.name)

## Save material resource to disk for reuse
func save_material(material: Material, path: String = "res://assets/textures/ground/terrain_material.tres") -> bool:
	return ResourceSaver.save(material, path) == OK

## Load material resource from disk
func load_material(path: String = "res://assets/textures/ground/terrain_material.tres") -> Material:
	if ResourceLoader.exists(path):
		return load(path) as Material
	return null

## Main import function - call this from the editor
func import_textures(apply_to_scene: bool = true, save_to_disk: bool = true) -> StandardMaterial3D:
	print("[TextureImporter] Starting texture import...")
	
	var textures = load_texture_set()
	if not textures:
		push_error("[TextureImporter] Failed to load texture set")
		return null
	
	var material = create_pbr_material(textures)
	
	if save_to_disk:
		var save_path = "res://assets/textures/ground/terrain_material.tres"
		if save_material(material, save_path):
			print("[TextureImporter] Material saved to: %s" % save_path)
		else:
			push_error("[TextureImporter] Failed to save material")
	
	if apply_to_scene:
		apply_material_to_generators(material)
	
	print("[TextureImporter] Texture import complete!")
	return material

## EditorScript entry point
func _run() -> void:
	import_textures(true, true)

## Auto-run when added to scene in editor
func _ready() -> void:
	if Engine.is_editor_hint():
		# Only run once
		queue_free()
		import_textures(true, true)
