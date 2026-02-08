shader_type spatial;
render_mode blend_mix, depth_draw_opaque, cull_back, diffuse_burley, specular_schlick_ggx;

uniform bool use_textures = false;

// Optional quality toggle for large worlds:
// false = full 3-axis triplanar, true = cheaper 2-axis blend (fewer texture fetches)
uniform bool triplanar_fast = false;

// Voxel highlight overlay
uniform bool highlight_enabled = false;
uniform vec3 highlight_world_position = vec3(0.0);
uniform float highlight_radius = 1.5;
uniform vec4 highlight_color : source_color = vec4(1.0, 1.0, 0.0, 0.6);

// Up to 16 biome layers: layer index 0..15
uniform sampler2DArray biome_base_textures;
uniform sampler2DArray biome_slope_textures;

// Optional PBR extras (recommended for realistic materials).
// If these are not assigned, the shader falls back to mesh normals and constant roughness.
uniform sampler2DArray biome_base_normal_textures;
uniform sampler2DArray biome_slope_normal_textures;
uniform sampler2DArray biome_base_roughness_textures;
uniform sampler2DArray biome_slope_roughness_textures;

// PBR toggles. Default off to avoid sampling unassigned arrays.
uniform bool use_normal_textures = false;
uniform bool use_roughness_textures = false;

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
	vec2 uv_z = wpos.xy * triplanar_scale;

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

vec3 unpack_normal(vec3 n) {
	// Normal maps are stored in 0..1; convert to -1..1
	return n * 2.0 - 1.0;
}

// Convert a tangent-space normal sampled for a planar projection into world space.
// Projection planes:
// - X projection uses the YZ plane (u=Z, v=Y, normal=±X)
// - Y projection uses the XZ plane (u=X, v=Z, normal=±Y)
// - Z projection uses the XY plane (u=X, v=Y, normal=±Z)
vec3 triplanar_normal_world(sampler2DArray tex, float layer, vec3 wpos, vec3 wnorm) {
	vec3 n = abs(wnorm);
	float sum = max(n.x + n.y + n.z, 1e-5);
	vec3 w = n / sum;

	vec2 uv_x = wpos.zy * triplanar_scale;
	vec2 uv_y = wpos.xz * triplanar_scale;
	vec2 uv_z = wpos.xy * triplanar_scale;

	// Sample tangent-space normals per projection.
	vec3 nx_ts = unpack_normal(texture(tex, vec3(uv_x, layer)).xyz);
	vec3 ny_ts = unpack_normal(texture(tex, vec3(uv_y, layer)).xyz);
	vec3 nz_ts = unpack_normal(texture(tex, vec3(uv_z, layer)).xyz);

	// Turn tangent-space normals into world-space normals for each projection.
	// For each projection, we pick a planar tangent basis aligned to world axes.
	float sx = (wnorm.x >= 0.0) ? 1.0 : -1.0;
	float sy = (wnorm.y >= 0.0) ? 1.0 : -1.0;
	float sz = (wnorm.z >= 0.0) ? 1.0 : -1.0;

	// X projection (YZ plane): T=+Z, B=+Y, N=±X
	vec3 nx = normalize(vec3(nx_ts.z * sx, nx_ts.y, nx_ts.x));
	// Y projection (XZ plane): T=+X, B=+Z, N=±Y
	vec3 ny = normalize(vec3(ny_ts.x, ny_ts.z * sy, ny_ts.y));
	// Z projection (XY plane): T=+X, B=+Y, N=±Z
	vec3 nz = normalize(vec3(nz_ts.x, nz_ts.y, nz_ts.z * sz));

	// Blend and normalize.
	vec3 blended = nx * w.x + ny * w.y + nz * w.z;
	return normalize(blended);
}

float triplanar_roughness(sampler2DArray tex, float layer, vec3 wpos, vec3 wnorm) {
	vec3 n = abs(wnorm);
	float sum = max(n.x + n.y + n.z, 1e-5);
	vec3 w = n / sum;

	vec2 uv_x = wpos.zy * triplanar_scale;
	vec2 uv_y = wpos.xz * triplanar_scale;
	vec2 uv_z = wpos.xy * triplanar_scale;

	// Roughness is typically stored in the R channel.
	float rx = texture(tex, vec3(uv_x, layer)).r;
	float ry = texture(tex, vec3(uv_y, layer)).r;
	float rz = texture(tex, vec3(uv_z, layer)).r;
	return rx * w.x + ry * w.y + rz * w.z;
}

void vertex() {
	v_world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	mat3 normal_matrix = transpose(inverse(mat3(MODEL_MATRIX)));
	v_world_normal = normalize(normal_matrix * NORMAL);
	v_custom0 = CUSTOM0;
}

void fragment() {
	vec3 out_albedo = COLOR.rgb;
	vec3 out_world_normal = v_world_normal;
	float out_roughness = 0.9;

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
			if (use_normal_textures) {
				out_world_normal = triplanar_normal_world(biome_base_normal_textures, layer, v_world_pos, v_world_normal);
			}
			if (use_roughness_textures) {
				out_roughness = triplanar_roughness(biome_base_roughness_textures, layer, v_world_pos, v_world_normal);
			}
		} else if (t >= 0.999) {
			vec4 slope = sample_triplanar(biome_slope_textures, layer, v_world_pos, v_world_normal);
			out_albedo = slope.rgb;
			if (use_normal_textures) {
				out_world_normal = triplanar_normal_world(biome_slope_normal_textures, layer, v_world_pos, v_world_normal);
			}
			if (use_roughness_textures) {
				out_roughness = triplanar_roughness(biome_slope_roughness_textures, layer, v_world_pos, v_world_normal);
			}
		} else {
			vec4 base = sample_triplanar(biome_base_textures, layer, v_world_pos, v_world_normal);
			vec4 slope = sample_triplanar(biome_slope_textures, layer, v_world_pos, v_world_normal);
			out_albedo = mix(base.rgb, slope.rgb, t);
			if (use_normal_textures) {
				vec3 base_n = triplanar_normal_world(biome_base_normal_textures, layer, v_world_pos, v_world_normal);
				vec3 slope_n = triplanar_normal_world(biome_slope_normal_textures, layer, v_world_pos, v_world_normal);
				out_world_normal = normalize(mix(base_n, slope_n, t));
			}
			if (use_roughness_textures) {
				float base_r = triplanar_roughness(biome_base_roughness_textures, layer, v_world_pos, v_world_normal);
				float slope_r = triplanar_roughness(biome_slope_roughness_textures, layer, v_world_pos, v_world_normal);
				out_roughness = mix(base_r, slope_r, t);
			}
		}
	}

	// Apply voxel highlight if enabled
	if (highlight_enabled) {
		float dist = distance(v_world_pos, highlight_world_position);
		if (dist < highlight_radius) {
			float factor = 1.0 - (dist / highlight_radius);
			factor = factor * factor; // Quadratic falloff for sharper edge
			out_albedo = mix(out_albedo, highlight_color.rgb, factor * highlight_color.a);
		}
	}

	ALBEDO = out_albedo;
	ROUGHNESS = clamp(out_roughness, 0.0, 1.0);
	METALLIC = 0.0;

	// Godot expects NORMAL in view space when written in fragment().
	// Convert our blended world normal to view space.
	vec3 view_n = normalize((VIEW_MATRIX * vec4(out_world_normal, 0.0)).xyz);
	NORMAL = view_n;
}
