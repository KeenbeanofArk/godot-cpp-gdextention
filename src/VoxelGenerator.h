// VoxelGenerator.h
#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class VoxelGenerator : public Node3D {
    GDCLASS(VoxelGenerator, Node3D);

private:
    int size = 1;
    int resolution = 1;
    float cutoff = 0.1f;
    bool show_centers = true;
    bool show_grid = true;
    int seeder = 1;
    bool auto_generate = false;

protected:
    static void _bind_methods();
    // This method is called to notify the object of various events.
    void _notification(int p_what);

public:
    void _ready() override;

    void set_size(int value);
    int get_size() const;

    void set_resolution(int value);
    int get_resolution() const;

    void set_cutoff(float value);
    float get_cutoff() const;

    void set_randomizer(bool value);
    bool get_randomizer() const;

    void set_show_centers(bool value);
    bool get_show_centers() const;

    void set_show_grid(bool value);
    bool get_show_grid() const;

    void set_auto_generate(bool value);
    bool get_auto_generate() const;

    void reset();

    void generate();

private:
    void remove_children();
    void randomize_seed();
    Vector<Vector3> create_cube_vertices(const Vector3 &pos);
    void add_cubes_vertices(Ref<ImmediateMesh> mesh, const Vector<Vector3> &vertices);
    std::vector<float> get_cube_values(Ref<FastNoiseLite> noise, const Vector<Vector3> &cube_vertices);
    int get_lookup_index(const std::vector<float> &cube_values, float cutoff);
    Vector3 interpolate(const Vector3 &vertex_1, float value_1, const Vector3 &vertex_2, float value_2);
    void add_cube_edges(Ref<ImmediateMesh> mesh, const std::vector<Vector3> &v);

};  // VOXEL_GENERATOR_H
