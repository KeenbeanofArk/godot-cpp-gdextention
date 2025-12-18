extends RefCounted

# Builds and assigns placeholder Texture2DArray resources for the biome triplanar shader.
# This is purely to make the demo work out-of-the-box; replace these arrays with real
# imported textures when ready.

func ensure_default_arrays(material: ShaderMaterial, biome_layers: int = 16, image_size: int = 64) -> void:
	if material == null:
		return

	# Only assign if not already set in the material.
	var existing_base = material.get_shader_parameter("biome_base_textures")
	var existing_slope = material.get_shader_parameter("biome_slope_textures")
	if existing_base != null and existing_slope != null:
		return

	var base_array := _build_debug_texture_array(biome_layers, image_size, false)
	var slope_array := _build_debug_texture_array(biome_layers, image_size, true)

	material.set_shader_parameter("biome_base_textures", base_array)
	material.set_shader_parameter("biome_slope_textures", slope_array)

	# Sensible defaults for immediate visual feedback.
	if material.get_shader_parameter("triplanar_scale") == null:
		material.set_shader_parameter("triplanar_scale", 6.0)
	if material.get_shader_parameter("slope_start") == null:
		material.set_shader_parameter("slope_start", 0.25)
	if material.get_shader_parameter("slope_end") == null:
		material.set_shader_parameter("slope_end", 0.65)


func _build_debug_texture_array(biome_layers: int, image_size: int, slope_variant: bool) -> Texture2DArray:
	biome_layers = clampi(biome_layers, 1, 16)
	image_size = max(8, image_size)

	var images: Array[Image] = []
	images.resize(biome_layers)

	for i in range(biome_layers):
		var base_hue := float(i) / float(max(1, biome_layers))
		var c1 := Color.from_hsv(base_hue, 0.55, 0.55 if slope_variant else 0.85)
		var c2 := Color.from_hsv(base_hue, 0.35, 0.35 if slope_variant else 0.65)
		# Make slope textures a bit greyer to read as "rock".
		if slope_variant:
			c1 = c1.lerp(Color(0.55, 0.55, 0.55), 0.55)
			c2 = c2.lerp(Color(0.35, 0.35, 0.35), 0.55)

		images[i] = _make_checker_image(image_size, c1, c2)

	var tex := Texture2DArray.new()
	tex.create_from_images(images)
	return tex


func _make_checker_image(size: int, a: Color, b: Color) -> Image:
	var img := Image.create(size, size, false, Image.FORMAT_RGBA8)
	var step: int = maxi(2, int(size / 8.0))

	for y in range(size):
		for x in range(size):
			var cx: int = int(x / float(step))
			var cy: int = int(y / float(step))
			img.set_pixel(x, y, a if ((cx + cy) % 2 == 0) else b)

	return img
