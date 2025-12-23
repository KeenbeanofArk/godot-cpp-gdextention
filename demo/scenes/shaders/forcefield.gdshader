shader_type spatial;
render_mode blend_add, depth_draw_opaque, cull_disabled, unshaded;

// Parameters
uniform vec4 forcefield_color : source_color = vec4(0.3, 0.6, 1.0, 1.0);
uniform float edge_intensity : hint_range(0.0, 5.0) = 2.0;
uniform float scan_speed : hint_range(0.0, 2.0) = 0.5;
uniform float scan_scale : hint_range(1.0, 50.0) = 10.0;
uniform float base_alpha : hint_range(0.0, 1.0) = 0.0;

// Ripple effect parameters
uniform vec3 ripple_pos_1 = vec3(0.0, 0.0, 0.0);
uniform vec3 ripple_pos_2 = vec3(0.0, 0.0, 0.0);
uniform vec3 ripple_pos_3 = vec3(0.0, 0.0, 0.0);
uniform float ripple_time_1 : hint_range(0.0, 10.0) = 10.0;
uniform float ripple_time_2 : hint_range(0.0, 10.0) = 10.0;
uniform float ripple_time_3 : hint_range(0.0, 10.0) = 10.0;

void fragment() {
	// Calculate Fresnel effect for edge glow
	vec3 normal = normalize(NORMAL);
	vec3 view = normalize(VIEW);
	float fresnel = pow(1.0 - abs(dot(normal, view)), 3.0);
	
	// Animated scanlines
	float scan_pattern = sin((VERTEX.y + VERTEX.x * 0.3 + TIME * scan_speed) * scan_scale);
	scan_pattern = scan_pattern * 0.5 + 0.5; // Remap to 0-1
	
	// Create hexagonal grid pattern
	vec2 uv = UV * 8.0;
	vec2 grid_uv = uv;
	float hex_pattern = abs(sin(grid_uv.x * 3.14159)) * abs(sin(grid_uv.y * 3.14159));
	hex_pattern = smoothstep(0.3, 0.5, hex_pattern);
	
	// Calculate ripple effects
	float ripple_effect = 0.0;
	
	// Ripple 1
	if (ripple_time_1 < 2.0) {
		float dist1 = length(VERTEX - ripple_pos_1);
		float ripple1 = sin(dist1 * 3.0 - ripple_time_1 * 8.0) * exp(-ripple_time_1 * 2.0) * exp(-dist1 * 0.5);
		ripple_effect += max(0.0, ripple1);
	}
	
	// Ripple 2
	if (ripple_time_2 < 2.0) {
		float dist2 = length(VERTEX - ripple_pos_2);
		float ripple2 = sin(dist2 * 3.0 - ripple_time_2 * 8.0) * exp(-ripple_time_2 * 2.0) * exp(-dist2 * 0.5);
		ripple_effect += max(0.0, ripple2);
	}
	
	// Ripple 3
	if (ripple_time_3 < 2.0) {
		float dist3 = length(VERTEX - ripple_pos_3);
		float ripple3 = sin(dist3 * 3.0 - ripple_time_3 * 8.0) * exp(-ripple_time_3 * 2.0) * exp(-dist3 * 0.5);
		ripple_effect += max(0.0, ripple3);
	}
	
	// Combine all effects
	float combined_intensity = fresnel * edge_intensity + scan_pattern * 0.3 + hex_pattern * 0.2 + ripple_effect * 2.0;
	
	// Final color and alpha
	vec3 final_color = forcefield_color.rgb * combined_intensity;
	float final_alpha = base_alpha * (combined_intensity * 0.5 + 0.5);
	
	ALBEDO = final_color;
	ALPHA = clamp(final_alpha, 0.0, 0.7);
	EMISSION = final_color * 0.5;
}
