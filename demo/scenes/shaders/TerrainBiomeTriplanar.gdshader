shader_type spatial;
render_mode blend_mix, depth_draw_opaque, cull_back, diffuse_burley, specular_schlick_ggx;

uniform bool use_textures = false;

// Optional quality toggle for large worlds:
// false = full 3-axis triplanar, true = cheaper 2-axis blend (fewer texture fetches)
uniform bool triplanar_fast = false;

// Up to 16 biome layers: layer index 0..15
uniform sampler2DArray biome_base_textures;
uniform sampler2DArray biome_slope_textures;

// Triplanar settings
uniform float triplanar_scale = 0.25; // world-units-to-texture scale

// Slope blending settings
// slope_amount = 1 - abs(world_normal.y)
uniform float slope_start = 0.4;
uniform float slope_end = 0.75;

varying vec3 v_world_pos;
varying vec3 v_world_normal;
varying vec4 v_custom0;

float saturate(float x) {
	return clamp(x, 0.0, 1.0);
}

vec4 sample_triplanar(sampler2DArray tex, float layer, vec3 wpos, vec3 wnorm) {
	vec3 n = abs(wnorm);
	float sum = max(n.x + n.y + n.z, 1e-5);
	vec3 w = n / sum;

	vec2 uv_x = wpos.zy * triplanar_scale;
	vec2 uv_y = wpos.xz * triplanar_scale;
	tvec2 uv_z = wpos.xy * triplanar_scale;

	// Full 3-axis blend (quality)
	if (!triplanar_fast) {
		vec4 x = texture(tex, vec3(uv_x, layer));
		vec4 y = texture(tex, vec3(uv_y, layer));
		vec4 z = texture(tex, vec3(uv_z, layer));
		return x * w.x + y * w.y + z * w.z;
	}

	// Fast 2-axis blend:
	// Drop the smallest-weight axis and blend the other two.
	// This reduces sampling from 3 -> 2 per texture.
	float wx = w.x;
	float wy = w.y;
	float wz = w.z;

	// Keep two largest weights.
	if (wx <= wy && wx <= wz) {
		float denom = max(wy + wz, 1e-5);
		float by = wy / denom;
		float bz = wz / denom;
		vec4 y = texture(tex, vec3(uv_y, layer));
		vec4 z = texture(tex, vec3(uv_z, layer));
		return y * by + z * bz;
	} else if (wy <= wx && wy <= wz) {
		float denom = max(wx + wz, 1e-5);
		float bx = wx / denom;
		float bz = wz / denom;
		vec4 x = texture(tex, vec3(uv_x, layer));
		vec4 z = texture(tex, vec3(uv_z, layer));
		return x * bx + z * bz;
	} else {
		float denom = max(wx + wy, 1e-5);
		float bx = wx / denom;
		float by = wy / denom;
		vec4 x = texture(tex, vec3(uv_x, layer));
		vec4 y = texture(tex, vec3(uv_y, layer));
		return x * bx + y * by;
	}
}

void vertex() {
	v_world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	mat3 normal_matrix = transpose(inverse(mat3(MODEL_MATRIX)));
	v_world_normal = normalize(normal_matrix * NORMAL);
	v_custom0 = CUSTOM0;
}

void fragment() {
	vec3 out_albedo = COLOR.rgb;

	if (use_textures) {
		// CUSTOM0.r carries biome_id / 255.0 (packed as RGBA8_UNORM)
		float biome_raw = v_custom0.r * 255.0;
		float biome_id = floor(biome_raw + 0.5);
		float layer = clamp(biome_id, 0.0, 15.0);

		float slope_amount = 1.0 - abs(v_world_normal.y);
		float t = smoothstep(slope_start, slope_end, slope_amount);

		// Avoid sampling both textures when blend is near 0 or 1.
		if (t <= 0.001) {
			vec4 base = sample_triplanar(biome_base_textures, layer, v_world_pos, v_world_normal);
			out_albedo = base.rgb;
		} else if (t >= 0.999) {
			vec4 slope = sample_triplanar(biome_slope_textures, layer, v_world_pos, v_world_normal);
			out_albedo = slope.rgb;
		} else {
			vec4 base = sample_triplanar(biome_base_textures, layer, v_world_pos, v_world_normal);
			vec4 slope = sample_triplanar(biome_slope_textures, layer, v_world_pos, v_world_normal);
			out_albedo = mix(base.rgb, slope.rgb, t);
		}
	}

	ALBEDO = out_albedo;
}
