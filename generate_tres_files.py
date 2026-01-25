#!/usr/bin/env python3
"""
Generate Godot 4.x text resource (.tres) files for TerrainConfig resources
This script creates valid Godot text resources that can be loaded by the engine.
"""

import os
import sys

def generate_terrain_config_tres(name, filename, world_size, chunk_size, resolution, 
                                 cutoff, seeder, terrain_height, terrain_amplitude, rock_influence):
    """Generate a .tres file for a TerrainConfig resource."""
    
    # Build the resource content in Godot's text format
    content = f"""[gd_resource type="TerrainConfig" format=3 uid="uid://{''.join(format(ord(c), 'x') for c in name[:8]).ljust(16, '0')}"]

config_name = "{name}"
world_size = Vector3i({world_size[0]}, {world_size[1]}, {world_size[2]})
chunk_size = {chunk_size}
resolution = {resolution}
cutoff = {cutoff}
seeder = {seeder}
terrain_height = {terrain_height}
terrain_amplitude = {terrain_amplitude}
rock_influence = {rock_influence}
"""
    return content

def main():
    configs = [
        {
            "name": "Desert",
            "file": "desert.tres",
            "world_size": (4, 2, 4),
            "chunk_size": 32,
            "resolution": 2,
            "cutoff": 0.1,
            "seeder": 9012,
            "terrain_height": 2.0,
            "terrain_amplitude": 4.0,
            "rock_influence": 0.1,
        },
        {
            "name": "Plains",
            "file": "plains.tres",
            "world_size": (3, 2, 3),
            "chunk_size": 32,
            "resolution": 2,
            "cutoff": 0.1,
            "seeder": 1240,
            "terrain_height": 4.0,
            "terrain_amplitude": 8.0,
            "rock_influence": 0.3,
        },
        {
            "name": "Mountains",
            "file": "mountains.tres",
            "world_size": (3, 3, 3),
            "chunk_size": 32,
            "resolution": 2,
            "cutoff": 0.1,
            "seeder": 5678,
            "terrain_height": 12.0,
            "terrain_amplitude": 16.0,
            "rock_influence": 0.5,
        },
        {
            "name": "Forest",
            "file": "forest.tres",
            "world_size": (3, 2, 3),
            "chunk_size": 32,
            "resolution": 2,
            "cutoff": 0.1,
            "seeder": 3456,
            "terrain_height": 6.0,
            "terrain_amplitude": 10.0,
            "rock_influence": 0.2,
        },
    ]
    
    # Determine output directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.join(script_dir, "demo", "configs")
    
    # Create directory if needed
    os.makedirs(output_dir, exist_ok=True)
    
    # Generate each config file
    for config in configs:
        content = generate_terrain_config_tres(
            config["name"],
            config["file"],
            config["world_size"],
            config["chunk_size"],
            config["resolution"],
            config["cutoff"],
            config["seeder"],
            config["terrain_height"],
            config["terrain_amplitude"],
            config["rock_influence"],
        )
        
        output_path = os.path.join(output_dir, config["file"])
        try:
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"✓ Generated: {output_path}")
        except Exception as e:
            print(f"✗ Error generating {config['file']}: {e}", file=sys.stderr)
            return False
    
    print("\n✓ All terrain config files generated successfully!")
    print(f"  Output directory: {output_dir}")
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
