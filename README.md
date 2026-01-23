# godot-cpp GDExtension Template (Voxel Engine)

This repository is a starter template for building a Godot 4 GDExtension in C++ using the `godot-cpp` bindings. It contains a minimal voxel engine example (source in `src/`) and a small demo Godot project (`demo/`) so you can build and test the extension quickly.

**Contents**

- `demo/`: example Godot project configured to load the built `.gdextension` file.
- `godot-cpp/`: the `godot-cpp` bindings submodule (required).
- `src/`: C++ source for the example extension (`register_types.cpp`, classes, generators, core code).
- `doc_classes/`: generated XML docs for the GDExtension API.
- `SConstruct`, `CMakeLists.txt`: build scaffolding (SCons is the classic build used here; CMake is available too).

**Quickstart (local)**

1. Initialize the `godot-cpp` submodule:

```powershell
git submodule update --init --recursive
```

2. Build with SCons (example for Windows PowerShell):

```powershell
# From repository root
scons -j12 target=template_debug debug_symbols=yes
```

3. (Optional) Use CMake if you prefer the CMake workflow — follow the `godot-cpp` README for details.

4. Copy or point the built `.gdextension` file into `demo/bin/` (the SConstruct and demo files are preconfigured to match names). Then open the `demo/` folder in the Godot editor and run the project to test the extension.

**Important files to edit when renaming your library**

- `SConstruct`: change the `libname` string to control the output library name.
- `demo/bin/*.gdextension`: update the `library`/`entry_symbol` fields to match your library name and exported entry function (the entry symbol is the C function exported from C++ e.g. `GDExtensionBool GDE_EXPORT voxel_engine_library_init(...)`).

**Where classes are registered**

- The module registers classes with Godot in `src/register_types.cpp` (see `initialize_voxel_engine_module` and the exported init function `voxel_engine_library_init`). Use `GDREGISTER_CLASS(YourClass);` to expose classes.

**Build tips**

- Ensure you have a compatible C++ toolchain for your platform and the correct Godot headers (managed via `godot-cpp`).
- If you run into linking or symbol issues, re-run the `godot-cpp` build step (CMake or SCons) to regenerate bindings for your compiler.

**Documentation**

- **[USAGE_GUIDE.md](USAGE_GUIDE.md)** - Complete guide to using VoxelGenerator and VoxelEngine (START HERE!)
- **[GENERATION_GUIDE.md](GENERATION_GUIDE.md)** - `generate()` vs `generate_async()` comparison
- **[Terraforming.md](doc_classes/Terraforming.md)** - Terraforming API and runtime terrain editing
- **[SAVE_LOAD_MAP.md](docs/SAVE_LOAD_MAP.md)** - Save and load map persistence
- **[BUILD_WINDOWS.md](BUILD_WINDOWS.md)** - Windows build instructions

**CI / Releases**

- This template includes a GitHub Actions workflow (`.github/workflows/builds.yml`) that builds cross-platform release artifacts when creating releases.

**License**

- See `LICENSE.md` for licensing details.

If you want, I can also:

- Add a short `BUILD_WINDOWS.md` with step-by-step PowerShell commands for common Windows toolchains.
- Confirm and update the extension name and `entry_symbol` across `SConstruct` and `demo/bin/*.gdextension` for you.

Enjoy building — tell me if you want me to run the build or adjust the demo configuration.
