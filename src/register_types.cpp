#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "VoxelGenerator.h"
#include "core/chunk.h"
#include "core/voxel.h"

using namespace godot;
using namespace voxel_engine;

void initialize_voxel_engine_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	// Register the Voxel class
	GDREGISTER_CLASS(Voxel);
	// Register the Chunk class
	GDREGISTER_CLASS(Chunk);
	// Register the VoxelGenerator class
	GDREGISTER_CLASS(VoxelGenerator);
}

void uninitialize_voxel_engine_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT voxel_engine_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_voxel_engine_module);
	init_obj.register_terminator(uninitialize_voxel_engine_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}