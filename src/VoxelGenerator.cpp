// VoxelGenerator.cpp
#include "VoxelGenerator.h"
#include "Constants.h"

// Godot includes
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace voxel_engine 
{

    void VoxelGenerator::_bind_methods() {
        ClassDB::bind_method(D_METHOD("generate"), &VoxelGenerator::generate);

        ClassDB::bind_method(D_METHOD("set_chunk_size", "value"), &VoxelGenerator::set_chunk_size);
        ClassDB::bind_method(D_METHOD("get_chunk_size"), &VoxelGenerator::get_chunk_size);
        ClassDB::bind_method(D_METHOD("set_resolution", "value"), &VoxelGenerator::set_resolution);
        ClassDB::bind_method(D_METHOD("get_resolution"), &VoxelGenerator::get_resolution);
        ClassDB::bind_method(D_METHOD("set_cutoff", "value"), &VoxelGenerator::set_cutoff);
        ClassDB::bind_method(D_METHOD("get_cutoff"), &VoxelGenerator::get_cutoff);
        ClassDB::bind_method(D_METHOD("set_randomizer", "value"), &VoxelGenerator::set_randomizer);
        ClassDB::bind_method(D_METHOD("get_randomizer"), &VoxelGenerator::get_randomizer);
        ClassDB::bind_method(D_METHOD("set_show_centers", "value"), &VoxelGenerator::set_show_centers);
        ClassDB::bind_method(D_METHOD("get_show_centers"), &VoxelGenerator::get_show_centers);
        ClassDB::bind_method(D_METHOD("set_show_grid", "value"), &VoxelGenerator::set_show_grid);
        ClassDB::bind_method(D_METHOD("get_show_grid"), &VoxelGenerator::get_show_grid);
        ClassDB::bind_method(D_METHOD("set_auto_generate", "value"), &VoxelGenerator::set_auto_generate);
        ClassDB::bind_method(D_METHOD("get_auto_generate"), &VoxelGenerator::get_auto_generate);

        // Bind debug methods
        ClassDB::bind_method(D_METHOD("set_debug_mode", "enabled"), &VoxelGenerator::set_debug_mode);
        ClassDB::bind_method(D_METHOD("get_debug_mode"), &VoxelGenerator::get_debug_mode);

        ClassDB::bind_method(D_METHOD("set_visualize_noise_values", "enabled"), &VoxelGenerator::set_visualize_noise_values);
        ClassDB::bind_method(D_METHOD("get_visualize_noise_values"), &VoxelGenerator::get_visualize_noise_values);

        ClassDB::bind_method(D_METHOD("set_debug_verbosity", "level"), &VoxelGenerator::set_debug_verbosity);
        ClassDB::bind_method(D_METHOD("get_debug_verbosity"), &VoxelGenerator::get_debug_verbosity);

        ClassDB::bind_method(D_METHOD("debug_print_state"), &VoxelGenerator::debug_print_state);
        ClassDB::bind_method(D_METHOD("debug_draw_noise_slice", "y_level"), &VoxelGenerator::debug_draw_noise_slice);
        ClassDB::bind_method(D_METHOD("log_message", "message", "verbosity_level"), &VoxelGenerator::log_message, DEFVAL(1));

        ClassDB::bind_method(D_METHOD("reset"), &VoxelGenerator::reset);

        ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size", PROPERTY_HINT_RANGE, "8,32,8"), "set_chunk_size", "get_chunk_size");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "resolution", PROPERTY_HINT_RANGE, "1,10,1"), "set_resolution", "get_resolution");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cutoff", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_cutoff", "get_cutoff");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "randomizer"), "set_randomizer", "get_randomizer");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_centers"), "set_show_centers", "get_show_centers");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_grid"), "set_show_grid", "get_show_grid");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_generate"), "set_auto_generate", "get_auto_generate");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_mode"), "set_debug_mode", "get_debug_mode");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visualize_noise_values"), "set_visualize_noise_values", "get_visualize_noise_values");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "debug_verbosity", PROPERTY_HINT_RANGE, "0,3,1"), "set_debug_verbosity", "get_debug_verbosity");
    }

    void VoxelGenerator::_notification(int p_what){
        switch (p_what) {
            case NOTIFICATION_READY: {
                remove_children();
                randomize_seed();
                chunk_size = 8;
                resolution = 1;
                cutoff = 0.1f;
                show_centers = true;
                show_grid = true;
                seeder = 1234;

                if (auto_generate) {
                    generate();
                    create_chunks();
                } else {
                    UtilityFunctions::print("VoxelGenerator is ready, but auto generation is disabled. Call generate() to create the voxel grid.");
                }
                break;
            }
            default:
                break;
        }
    };

    void VoxelGenerator::reset() {
        remove_children();
        randomize_seed();
        chunk_size = 8;
        resolution = 1;
        cutoff = 0.1f;
        show_centers = true;
        show_grid = true;
        seeder = 1234;

        if (auto_generate) {
            generate();
        } else {
            UtilityFunctions::print("VoxelGenerator reset, but auto generation is disabled. Call generate() to create the voxel grid.");
        }
    }

    void VoxelGenerator::set_auto_generate(bool value) {
        auto_generate = value;
        if (auto_generate)
            generate();
    }

    bool VoxelGenerator::get_auto_generate() const {
        return auto_generate;
    }

    void VoxelGenerator::set_chunk_size(int value) {
        chunk_size = value;
        if (auto_generate)
            generate();
    }

    int VoxelGenerator::get_chunk_size() const {
        return chunk_size;
    }

    void VoxelGenerator::set_resolution(int value) {
        resolution = value;
        if (auto_generate)
            generate();
    }

    int VoxelGenerator::get_resolution() const {
        return resolution;
    }

    void VoxelGenerator::set_cutoff(float value) {
        cutoff = value;
        if (auto_generate)
            generate();
    }

    float VoxelGenerator::get_cutoff() const {
        return cutoff;
    }

    void VoxelGenerator::set_randomizer(bool value) {
        if (value) {
            randomize_seed();
            if (auto_generate)
                generate();
        }
    }

    bool VoxelGenerator::get_randomizer() const {
        return true; // This is a placeholder, as the randomizer is always enabled in this implementation.
    }

    void VoxelGenerator::set_show_centers(bool value) {
        show_centers = value;
        for (int i = 0; i < get_child_count(); ++i) {
            MeshInstance3D *child = Object::cast_to<MeshInstance3D>(get_child(i));
            if (child && child->get_name() == godot::StringName("MeshInstanceCenters")) {
                child->set_visible(value);
            }
        }
    }

    bool VoxelGenerator::get_show_centers() const {
        return show_centers;
    }

    void VoxelGenerator::set_show_grid(bool value) {
        show_grid = value;
        for (int i = 0; i < get_child_count(); ++i) {
            MeshInstance3D *child = Object::cast_to<MeshInstance3D>(get_child(i));
            if (child && child->get_name() == godot::StringName("MeshInstanceCubes")) {
                child->set_visible(value);
            }
        }
    }

    bool VoxelGenerator::get_show_grid() const {
        return show_grid;
    }

    void VoxelGenerator::remove_children() {
        while (get_child_count() > 0) {
            Node *child = get_child(0);
            remove_child(child);
            child->queue_free();
        }
    }

    void VoxelGenerator::randomize_seed() {
        seeder = UtilityFunctions::randi();
        log_message(String("Random seed generated: {0}").format(Array::make(seeder)), 1);
    }

    void VoxelGenerator::generate() {
        log_message("VoxelGenerator::generate() called", 2);
        log_message("Starting voxel generation with:", 2);
        log_message(String("  Chunk Size: {0}, Resolution: {1}, Cutoff: {2}, Seed: {3}")
                        .format(Array::make(chunk_size, resolution, cutoff, seeder)), 2);

        remove_children();

        Ref<FastNoiseLite> noise;
        noise.instantiate();
        noise->set_seed(seeder);

        log_message("Noise generator initialized", 2);

        // # Create centers mesh
        Ref<ImmediateMesh> mesh_centers;
        mesh_centers.instantiate();
        mesh_centers->surface_begin(Mesh::PRIMITIVE_POINTS);

        // # Create cubes mesh
        Ref<ImmediateMesh> mesh_cubes;
        mesh_cubes.instantiate();
        mesh_cubes->surface_begin(Mesh::PRIMITIVE_LINES);

        // # Create triangles mesh
        Ref<ImmediateMesh> mesh_triangles;
        mesh_triangles.instantiate();
        mesh_triangles->surface_begin(Mesh::PRIMITIVE_TRIANGLES);

        log_message("Meshes created", 2);

        int start = -chunk_size * resolution;
        int end = (chunk_size + 1) * resolution;

        int total_cubes = (end - start) * (end - start) * (end - start);
        int current_cube = 0;
        int triangle_count = 0;

        for (int x = start; x < end; ++x) {
            for (int y = start; y < end; ++y) {
                for (int z = start; z < end; ++z) {
                    current_cube++;
                    if (debug_mode && debug_verbosity >= 3) {
                        // Progress update for very verbose mode
                        if (current_cube % 1000 == 0 || current_cube == total_cubes) {
                            log_message(String("Processing cube {0}/{1} ({2}%)")
                                                .format(Array::make(current_cube, total_cubes, int(100.0f * current_cube / total_cubes))),
                                    3);
                        }
                    }

                    // Calculate the center position of the voxel
                    Vector3 center = Vector3((float)x / resolution, (float)y / resolution, (float)z / resolution);

                    // Get the noise value at the center position
                    float center_value = noise->get_noise_3d(center.x, center.y, center.z);

                    if (debug_mode && debug_verbosity >= 3) {
                        log_message(String("  Cube at {0},{1},{2}: noise={3}")
                                            .format(Array::make(center.x, center.y, center.z, center_value)),
                                3);
                    }

                    // Create marching cube vertices
                    Vector<Vector3> cube_vertices = create_cube_vertices(center);

                    // Get the scalar value at the corners for the current cube
                    std::vector<float> cube_values = get_cube_values(noise, cube_vertices);

                    if (center_value < cutoff) {
                        add_cubes_vertices(mesh_cubes, cube_vertices);
                    } // Get the lookup index for the current cube
                    int lookup_index = get_lookup_index(cube_values, cutoff); // Bounds check to prevent crash with incomplete lookup table
                    const auto &marching_triangles = Constants::get_marching_triangles();
                    if (lookup_index >= marching_triangles.size()) {
                        if (debug_mode && debug_verbosity >= 2) {
                            log_message(String("Warning: lookup_index {0} exceeds table size {1}, skipping cube")
                                                .format(Array::make(lookup_index, (int)marching_triangles.size())),
                                    1);
                        }
                        continue; // Skip this cube to prevent crash
                    }

                    // Get triangles
                    std::vector<int> triangles(marching_triangles[lookup_index].begin(), marching_triangles[lookup_index].end());

                    Color color(
                            (center.x + chunk_size) / (chunk_size * 2.0f),
                            (center.y + chunk_size) / (chunk_size * 2.0f),
                            (center.z + chunk_size) / (chunk_size * 2.0f));

                    if (triangles.size() > 1) {
                        mesh_centers->surface_set_color(color);
                        mesh_centers->surface_add_vertex(center);
                    };

                    for (size_t index = 0; index < triangles.size(); index += 3) {
                        int point_1 = triangles[index];
                        if (point_1 == -1)
                            continue;

                        int point_2 = triangles[index + 1];
                        if (point_2 == -1)
                            continue;

                        int point_3 = triangles[index + 2];
                        if (point_3 == -1)
                            continue;

                        triangle_count++;

                        int a0 = Constants::cornerIndexAFromEdge[point_1];
                        int b0 = Constants::cornerIndexBFromEdge[point_1];

                        int a1 = Constants::cornerIndexAFromEdge[point_2];
                        int b1 = Constants::cornerIndexBFromEdge[point_2];

                        int a2 = Constants::cornerIndexAFromEdge[point_3];
                        int b2 = Constants::cornerIndexBFromEdge[point_3];

                        Vector3 vertex1 = interpolate(cube_vertices[a0], cube_values[a0], cube_vertices[b0], cube_values[b0]);
                        Vector3 vertex2 = interpolate(cube_vertices[a1], cube_values[a1], cube_vertices[b1], cube_values[b1]);
                        Vector3 vertex3 = interpolate(cube_vertices[a2], cube_values[a2], cube_vertices[b2], cube_values[b2]);

                        Vector3 vector_a = vertex3 - vertex1;
                        Vector3 vector_b = vertex2 - vertex1;

                        Vector3 normal = vector_a.cross(vector_b).normalized();

                        mesh_triangles->surface_set_color(color);
                        mesh_triangles->surface_set_normal(normal);
                        mesh_triangles->surface_add_vertex(vertex1);
                        mesh_triangles->surface_add_vertex(vertex2);
                        mesh_triangles->surface_add_vertex(vertex3);
                    }
                }
            }
        }

        log_message(String("Generation completed: {0} triangles created").format(Array::make(triangle_count)), 2);

        // # End surfaces
        mesh_centers->surface_end();
        mesh_cubes->surface_end();
        mesh_triangles->surface_end();

        // # Create centers material
        Ref<StandardMaterial3D> material_centers;
        material_centers.instantiate();
        material_centers->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
        material_centers->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
        material_centers->set_point_size(20.0);

        // # Create cubes material
        Ref<StandardMaterial3D> material_cubes;
        material_cubes.instantiate();
        material_cubes->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
        material_cubes->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);

        // # Create triangles material
        Ref<StandardMaterial3D> material_triangles;
        material_triangles.instantiate();
        material_triangles->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);

        mesh_centers->surface_set_material(0, material_centers);
        mesh_cubes->surface_set_material(0, material_cubes);
        mesh_triangles->surface_set_material(0, material_triangles);

        // # Create mesh instance nodes and add them to the scene
        MeshInstance3D *mi_centers = memnew(MeshInstance3D);
        mi_centers->set_name("MeshInstanceCenters");
        mi_centers->set_visible(show_centers);
        mi_centers->set_mesh(mesh_centers);
        add_child(mi_centers);

        // # Create cubes mesh instance and add it to the scene
        MeshInstance3D *mi_cubes = memnew(MeshInstance3D);
        mi_cubes->set_name("MeshInstanceCubes");
        mi_cubes->set_visible(show_grid);
        mi_cubes->set_mesh(mesh_cubes);
        add_child(mi_cubes);

        // # Create triangles mesh instance and add it to the scene
        MeshInstance3D *mi_triangles = memnew(MeshInstance3D);
        mi_triangles->set_mesh(mesh_triangles);
        add_child(mi_triangles);

        if (visualize_noise_values) {
            log_message("Creating noise visualization", 2);
            visualize_noise_field();
        }
    }

    Vector<Vector3> VoxelGenerator::create_cube_vertices(const Vector3 &pos) {
        float offset = 1.0f / (float)resolution;
        float half = offset / 2.0f;
        return Vector<Vector3>{
            Vector3(pos.x - half, pos.y - half, pos.z - half),
            Vector3(pos.x + half, pos.y - half, pos.z - half),
            Vector3(pos.x + half, pos.y + half, pos.z - half),
            Vector3(pos.x - half, pos.y + half, pos.z - half),
            Vector3(pos.x - half, pos.y - half, pos.z + half),
            Vector3(pos.x + half, pos.y - half, pos.z + half),
            Vector3(pos.x + half, pos.y + half, pos.z + half),
            Vector3(pos.x - half, pos.y + half, pos.z + half),
        };
    }

    void VoxelGenerator::add_cubes_vertices(Ref<ImmediateMesh> mesh, const Vector<Vector3> &cube_vertices) {
        const int lines[][2] = {
            { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
            { 0, 4 }, { 2, 6 }, { 5, 6 }, { 5, 4 },
            { 5, 1 }, { 6, 7 }, { 4, 7 }, { 3, 7 }
        };
        for (const auto &line : lines) {
            mesh->surface_add_vertex(cube_vertices[line[0]]);
            mesh->surface_add_vertex(cube_vertices[line[1]]);
        }
    }

    int VoxelGenerator::get_lookup_index(const std::vector<float> &cube_values, float cutoff) {
        int cube_index = 0;
        if (cube_values[0] < cutoff)
            cube_index |= 1;
        if (cube_values[1] < cutoff)
            cube_index |= 2;
        if (cube_values[2] < cutoff)
            cube_index |= 4;
        if (cube_values[3] < cutoff)
            cube_index |= 8;
        if (cube_values[4] < cutoff)
            cube_index |= 16;
        if (cube_values[5] < cutoff)
            cube_index |= 32;
        if (cube_values[6] < cutoff)
            cube_index |= 64;
        if (cube_values[7] < cutoff)
            cube_index |= 128;

        return cube_index;
    }

    std::vector<float, std::allocator<float>> VoxelGenerator::get_cube_values(godot::Ref<godot::FastNoiseLite> noise, const godot::Vector<godot::Vector3> &cube_vertices) {
        std::vector<float, std::allocator<float>> values(8);
        for (int i = 0; i < 8; ++i) {
            values[i] = noise->get_noise_3d(cube_vertices[i].x, cube_vertices[i].y, cube_vertices[i].z);
        }
        return values;
    }

    void VoxelGenerator::add_cube_edges(Ref<ImmediateMesh> mesh, const std::vector<Vector3> &v) {
        mesh->surface_add_vertex(v[0]);
        mesh->surface_add_vertex(v[1]);
        mesh->surface_add_vertex(v[1]);
        mesh->surface_add_vertex(v[2]);
        mesh->surface_add_vertex(v[2]);
        mesh->surface_add_vertex(v[3]);
        mesh->surface_add_vertex(v[3]);
        mesh->surface_add_vertex(v[0]);

        mesh->surface_add_vertex(v[0]);
        mesh->surface_add_vertex(v[4]);
        mesh->surface_add_vertex(v[1]);
        mesh->surface_add_vertex(v[5]);
        mesh->surface_add_vertex(v[2]);
        mesh->surface_add_vertex(v[6]);
        mesh->surface_add_vertex(v[3]);
        mesh->surface_add_vertex(v[7]);

        mesh->surface_add_vertex(v[4]);
        mesh->surface_add_vertex(v[5]);
        mesh->surface_add_vertex(v[5]);
        mesh->surface_add_vertex(v[6]);
        mesh->surface_add_vertex(v[6]);
        mesh->surface_add_vertex(v[7]);
        mesh->surface_add_vertex(v[7]);
        mesh->surface_add_vertex(v[4]);
    }

    Vector3 VoxelGenerator::interpolate(const Vector3 &vertex1, float value1, const Vector3 &vertex2, float value2) {
        float t = (cutoff - value1) / (value2 - value1);
        return Vector3(
                vertex1.x + t * (vertex2.x - vertex1.x),
                vertex1.y + t * (vertex2.y - vertex1.y),
                vertex1.z + t * (vertex2.z - vertex1.z));
    }

    // Debug methods implementation
    void VoxelGenerator::set_debug_mode(bool p_enabled) {
        debug_mode = p_enabled;
        if (debug_mode) {
            log_message("Debug mode enabled");
        }
    }

    bool VoxelGenerator::get_debug_mode() const {
        return debug_mode;
    }

    void VoxelGenerator::set_visualize_noise_values(bool p_enabled) {
        visualize_noise_values = p_enabled;
        if (visualize_noise_values && is_inside_tree()) {
            visualize_noise_field();
        }
    }

    bool VoxelGenerator::get_visualize_noise_values() const {
        return visualize_noise_values;
    }

    void VoxelGenerator::set_debug_verbosity(int p_level) {
        debug_verbosity = CLAMP(p_level, 0, 3);
    }

    int VoxelGenerator::get_debug_verbosity() const {
        return debug_verbosity;
    }

    void VoxelGenerator::debug_print_state() {
        String debug_info = "VoxelGenerator Debug Information:\n";
        debug_info += String("- Chunk Size: {0}\n").format(Array::make(chunk_size));
        debug_info += String("- Resolution: {0}\n").format(Array::make(resolution));
        debug_info += String("- Cutoff: {0}\n").format(Array::make(cutoff));
        debug_info += String("- Seed: {0}\n").format(Array::make(seeder));
        debug_info += String("- Show Centers: {0}\n").format(Array::make(show_centers));
        debug_info += String("- Show Grid: {0}\n").format(Array::make(show_grid));
        debug_info += String("- Auto Generate: {0}\n").format(Array::make(auto_generate));

        UtilityFunctions::print(debug_info);
    }

    void VoxelGenerator::debug_draw_noise_slice(float y_level) {
        log_message(String("Drawing noise slice at y={0}").format(Array::make(y_level)));

        // Create a new mesh for visualizing the noise slice
        Ref<ImmediateMesh> slice_mesh;
        slice_mesh.instantiate();
        slice_mesh->surface_begin(Mesh::PRIMITIVE_TRIANGLES);

        Ref<FastNoiseLite> noise;
        noise.instantiate();
        noise->set_seed(seeder);

        int slice_resolution = resolution * 2; // Higher resolution for better visualization
        float step = 1.0f / slice_resolution;
        float width = chunk_size * 2.0f;

        for (float x = -chunk_size; x < chunk_size; x += step) {
            for (float z = -chunk_size; z < chunk_size; z += step) {
                float noise_val = noise->get_noise_3d(x, y_level, z);

                // Normalize noise to color (blue = negative, red = positive)
                Color color;
                if (noise_val < cutoff) {
                    // Below cutoff - blue to cyan
                    color = Color(0, (noise_val + 1.0f) / 2.0f, 1.0f);
                } else {
                    // Above cutoff - yellow to red
                    color = Color(1.0f, 1.0f - (noise_val - cutoff) / (1.0f - cutoff), 0);
                }

                // Draw a small quad for each sample point
                Vector3 v1 = Vector3(x, y_level, z);
                Vector3 v2 = Vector3(x + step, y_level, z);
                Vector3 v3 = Vector3(x + step, y_level, z + step);
                Vector3 v4 = Vector3(x, y_level, z + step);

                // First triangle
                slice_mesh->surface_set_color(color);
                slice_mesh->surface_add_vertex(v1);
                slice_mesh->surface_add_vertex(v2);
                slice_mesh->surface_add_vertex(v3);

                // Second triangle
                slice_mesh->surface_set_color(color);
                slice_mesh->surface_add_vertex(v1);
                slice_mesh->surface_add_vertex(v3);
                slice_mesh->surface_add_vertex(v4);
            }
        }

        slice_mesh->surface_end();

        // Create material
        Ref<StandardMaterial3D> slice_material;
        slice_material.instantiate();
        slice_material->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
        slice_material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
        slice_material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);

        slice_mesh->surface_set_material(0, slice_material);

        // Create mesh instance and add it to the scene
        MeshInstance3D *mi_slice = memnew(MeshInstance3D);
        mi_slice->set_name(String("NoiseSlice_Y{0}").format(Array::make(y_level)));
        mi_slice->set_mesh(slice_mesh);
        add_child(mi_slice);

        log_message("Noise slice visualization created");
    }

    void VoxelGenerator::log_message(const String &message, int verbosity_level) {
        if (!debug_mode && verbosity_level <= 1) {
            // Always print critical messages (level 1) even if debug mode is off
            UtilityFunctions::print(String("[VoxelGenerator] {0}").format(Array::make(message)));
        } else if (debug_mode && verbosity_level <= debug_verbosity) {
            // Print message if debug mode is on and message verbosity is within our debug level
            UtilityFunctions::print(String("[VoxelGenerator][DEBUG-{0}] {1}").format(Array::make(verbosity_level, message)));
        }
    }

    void VoxelGenerator::visualize_noise_field() {
        // Create a visualization of the 3D noise field at the zero plane
        debug_draw_noise_slice(0.0f);
    }

    void VoxelGenerator::create_debug_visualization() {
        if (!debug_mode) {
            log_message("Debug mode is not enabled, skipping visualization.", 1);
            return;
        }

        log_message("Creating debug visualization...", 2);
        visualize_noise_field();
    }

    void VoxelGenerator::create_chunks() {
        chunks.clear();
        Chunk *chunk = memnew(Chunk);
        fill_chunk_with_voxels(chunk);
        // Optionally store chunk in a container if needed
        add_child(chunk);
    }

    void VoxelGenerator::fill_chunk_with_voxels(Chunk *chunk) {
        for (int x = 0; x < Chunk::CHUNK_SIZE; ++x) {
            for (int y = 0; y < Chunk::CHUNK_SIZE; ++y) {
                for (int z = 0; z < Chunk::CHUNK_SIZE; ++z) {
                    chunk->set_voxel(Vector3i(x, y, z), Voxel::DIRT);
                }
            }
        }
    }
} // namespace voxel_engine
