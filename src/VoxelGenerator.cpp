// VoxelGenerator.cpp
#include "VoxelGenerator.h"
#include "Constants.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

void VoxelGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("generate"), &VoxelGenerator::generate);

    ClassDB::bind_method(D_METHOD("set_size", "value"), &VoxelGenerator::set_size);
    ClassDB::bind_method(D_METHOD("get_size"), &VoxelGenerator::get_size);
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

    ADD_PROPERTY(PropertyInfo(Variant::INT, "size", PROPERTY_HINT_RANGE, "1,50,1"), "set_size", "get_size");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "resolution", PROPERTY_HINT_RANGE, "1,10,1"), "set_resolution", "get_resolution");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cutoff", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_cutoff", "get_cutoff");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "randomizer"), "set_randomizer", "get_randomizer");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_centers"), "set_show_centers", "get_show_centers");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_grid"), "set_show_grid", "get_show_grid");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_generate"), "set_auto_generate", "get_auto_generate");
}

void VoxelGenerator::_notification(int p_what)
{

}

void VoxelGenerator::reset() {
    remove_children();
    randomize_seed();
    size = 1;
    resolution = 1;
    cutoff = 0.1f;
    show_centers = true;
    show_grid = true;
    seeder = 1234;
    auto_generate = false;
}

void VoxelGenerator::_ready() {
    remove_children();
    randomize_seed();
    size = 1;
    resolution = 1;
    cutoff = 0.1f;
    show_centers = true;
    show_grid = true;
    seeder = 1234;
    auto_generate = false;

    if (auto_generate) 
        generate();
    else 
        UtilityFunctions::print("VoxelGenerator is ready, but auto generation is disabled. Call generate() to create the voxel grid.");
}

void VoxelGenerator::set_auto_generate(bool value) {
    auto_generate = value;
    if (auto_generate) generate();
}

bool VoxelGenerator::get_auto_generate() const {
    return auto_generate;
}

void VoxelGenerator::set_size(int value) {
    size = value;
    if (auto_generate) generate();
}

int VoxelGenerator::get_size() const {
    return size;
}

void VoxelGenerator::set_resolution(int value) {
    resolution = value;
    if (auto_generate) generate();
}

int VoxelGenerator::get_resolution() const {
    return resolution;
}

void VoxelGenerator::set_cutoff(float value) {
    cutoff = value;
    if (auto_generate) generate();
}

float VoxelGenerator::get_cutoff() const {
    return cutoff;
}

void VoxelGenerator::set_randomizer(bool value) {
    if (value) {
        randomize_seed();
        if (auto_generate) generate();
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
}

void VoxelGenerator::generate() {
    remove_children();

    Ref<FastNoiseLite> noise;
    noise.instantiate();
    noise->set_seed(seeder);

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

    int start = -size * resolution;
    int end = (size + 1) * resolution;

    for (int x = start; x < end; ++x) {
        for (int y = start; y < end; ++y) {
            for (int z = start; z < end; ++z) {
                // Calculate the center position of the voxel
                Vector3 center = Vector3((float)x / resolution, (float)y / resolution, (float)z / resolution);

                // Get the noise value at the center position
                float center_value = noise->get_noise_3d(center.x, center.y, center.z);

                // Create marching cube vertices
                Vector<Vector3> cube_vertices = create_cube_vertices(center);

                // Get the scalar value at the corners for the current cube
                std::vector<float> cube_values = get_cube_values(noise, cube_vertices);

                if (center_value < cutoff) {
                   add_cubes_vertices(mesh_cubes, cube_vertices);
                }

                // Get the lookup index for the current cube
                int lookup_index = get_lookup_index(cube_values, cutoff);

                // Get triangles
                std::vector<int> triangles(Constants::marching_triangles[lookup_index].begin(), Constants::marching_triangles[lookup_index].end());

                Color color(
                    (center.x + size) / (size * 2.0f),
                    (center.y + size) / (size * 2.0f),
                    (center.z + size) / (size * 2.0f)
                );

                if (triangles.size() > 1) {
                    mesh_centers->surface_set_color(color);
                    mesh_centers->surface_add_vertex(center);
                };

                for (size_t index = 0; index < triangles.size(); index += 3) {
                int point_1 = triangles[index];
                if (point_1 == -1) continue;

                int point_2 = triangles[index + 1];
                if (point_2 == -1) continue;

                int point_3 = triangles[index + 2];
                if (point_3 == -1) continue;

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
    
    // # End surfaces
    mesh_centers->surface_end();
    mesh_cubes->surface_end();
    mesh_triangles->surface_end();

    // # Create centers material
    Ref<StandardMaterial3D> material_centers;
    material_centers.instantiate();
    material_centers->set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
    material_centers->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
    material_centers->set_point_size(10.0);

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
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {0, 4}, {2, 6}, {5, 6}, {5, 4},
        {5, 1}, {6, 7}, {4, 7}, {3, 7}
    };
    for (const auto &line : lines) {
        mesh->surface_add_vertex(cube_vertices[line[0]]);
        mesh->surface_add_vertex(cube_vertices[line[1]]);
    }

}

int VoxelGenerator::get_lookup_index(const std::vector<float> &cube_values, float cutoff) {
    int cube_index = 0;
    if (cube_values[0] < cutoff) cube_index |= 1;
    if (cube_values[1] < cutoff) cube_index |= 2;
    if (cube_values[2] < cutoff) cube_index |= 4;
    if (cube_values[3] < cutoff) cube_index |= 8;
    if (cube_values[4] < cutoff) cube_index |= 16;
    if (cube_values[5] < cutoff) cube_index |= 32;
    if (cube_values[6] < cutoff) cube_index |= 64;
    if (cube_values[7] < cutoff) cube_index |= 128;

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
    mesh->surface_add_vertex(v[0]); mesh->surface_add_vertex(v[1]);
    mesh->surface_add_vertex(v[1]); mesh->surface_add_vertex(v[2]);
    mesh->surface_add_vertex(v[2]); mesh->surface_add_vertex(v[3]);
    mesh->surface_add_vertex(v[3]); mesh->surface_add_vertex(v[0]);

    mesh->surface_add_vertex(v[0]); mesh->surface_add_vertex(v[4]);
    mesh->surface_add_vertex(v[1]); mesh->surface_add_vertex(v[5]);
    mesh->surface_add_vertex(v[2]); mesh->surface_add_vertex(v[6]);
    mesh->surface_add_vertex(v[3]); mesh->surface_add_vertex(v[7]);

    mesh->surface_add_vertex(v[4]); mesh->surface_add_vertex(v[5]);
    mesh->surface_add_vertex(v[5]); mesh->surface_add_vertex(v[6]);
    mesh->surface_add_vertex(v[6]); mesh->surface_add_vertex(v[7]);
    mesh->surface_add_vertex(v[7]); mesh->surface_add_vertex(v[4]);
}

Vector3 VoxelGenerator::interpolate(const Vector3 &vertex1, float value1, const Vector3 &vertex2, float value2) {
    float t = (cutoff - value1) / (value2 - value1);
    return Vector3(
        vertex1.x + t * (vertex2.x - vertex1.x),
        vertex1.y + t * (vertex2.y - vertex1.y),
        vertex1.z + t * (vertex2.z - vertex1.z)
    );
}
