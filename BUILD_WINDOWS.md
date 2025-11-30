# BUILD_WINDOWS.md

This document explains how to build the GDExtension on Windows (MSVC toolchain) using PowerShell and SCons.

## Prerequisites

- Visual Studio (or Build Tools) with the "Desktop development with C++" workload installed (MSVC toolchain).
- Python 3 and `scons` available. Install SCons if needed:

```powershell
pip install --user scons
```

## Initialize submodules

From the repository root run:

```powershell
git submodule update --init --recursive
```

## Build the `godot-cpp` bindings (recommended)

This builds the C++ bindings for your platform/toolchain. From the repo root:

```powershell
Set-Location -Path .\godot-cpp
scons -j12 platform=windows
Set-Location -Path ..
```

## Build the extension with SCons

From the repository root (replace path as needed):

```powershell
Set-Location -Path "C:\Users\Brian\godot-cpp-gdextention"
scons -j12 target=template_debug debug_symbols=yes platform=windows
```

## Copy built DLL(s) into the demo folder

SCons produces the library at `bin\windows\<libname>`. Copy it into the demo folder so the `.gdextension` file can find it:

```powershell
New-Item -ItemType Directory -Force -Path .\demo\bin\windows
Copy-Item -Path .\bin\windows\* -Destination .\demo\bin\windows\ -Force
```

## Verify or update the demo `.gdextension`

Open `demo/bin/keen-voxel-engine.gdextension` and verify the `library` path and `entry_symbol` match the produced library and the exported init symbol in `src/register_types.cpp` (`voxel_engine_library_init`). You can inspect quickly with:

```powershell
Get-ChildItem .\bin\windows -File
```powershell
Get-ChildItem .\VoxelEngine\bin\windows -File
```powershell
Get-Content .\demo\bin\keen-voxel-engine.gdextension | Select-String -Pattern "library|entry_symbol"
```

## Run the demo in Godot

Start Godot and open the `demo` project. Example (replace with your Godot executable path):

```powershell
Start-Process -FilePath "C:\dev\Godot Files\Godot_v4.5.1-stable_mono_win64\Godot_v4.5.1-stable_mono_win64\Godot_v4.5.1-stable_mono_win64_console.exe" -ArgumentList ".\demo"
```

## Notes

- Make sure the MSVC toolchain used to build `godot-cpp` and this project match (same compiler/ABI). If you use a different compiler (MinGW), build `godot-cpp` the same way.
- The `SConstruct` script normalizes the `suffix` (it strips `.dev` and `.universal`) so the produced filename will match the keys inside `.gdextension` files. If you change suffix or platform names, update the `.gdextension` accordingly.
