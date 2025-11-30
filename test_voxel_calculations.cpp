#include <cstdint>
#include <iostream>

// Simple test to verify voxel count calculations
int main() {
	// Test case 1: world_size = (10, 2, 10), chunk_size = 8
	// Expected: 10*8 * 2*8 * 10*8 = 80 * 16 * 80 = 102,400 voxels
	int64_t world_x = 10, world_y = 2, world_z = 10;
	int chunk_size = 8;

	int64_t total_x = world_x * chunk_size;
	int64_t total_y = world_y * chunk_size;
	int64_t total_z = world_z * chunk_size;
	int64_t total_voxels = total_x * total_y * total_z;

	std::cout << "Test Case 1: World Size (10, 2, 10), Chunk Size 8" << std::endl;
	std::cout << "Total X dimension: " << total_x << std::endl;
	std::cout << "Total Y dimension: " << total_y << std::endl;
	std::cout << "Total Z dimension: " << total_z << std::endl;
	std::cout << "Total voxels: " << total_voxels << std::endl;
	std::cout << "Expected: 102,400" << std::endl;
	std::cout << "Match: " << (total_voxels == 102400 ? "YES" : "NO") << std::endl;
	std::cout << std::endl;

	// Test case 2: world_size = (1, 1, 1), chunk_size = 8
	// Expected: 1*8 * 1*8 * 1*8 = 8 * 8 * 8 = 512 voxels
	world_x = 1;
	world_y = 1;
	world_z = 1;
	chunk_size = 8;

	total_x = world_x * chunk_size;
	total_y = world_y * chunk_size;
	total_z = world_z * chunk_size;
	total_voxels = total_x * total_y * total_z;

	std::cout << "Test Case 2: World Size (1, 1, 1), Chunk Size 8 (Default)" << std::endl;
	std::cout << "Total voxels: " << total_voxels << std::endl;
	std::cout << "Expected: 512" << std::endl;
	std::cout << "Match: " << (total_voxels == 512 ? "YES" : "NO") << std::endl;
	std::cout << std::endl;

	// Test case 3: world_size = (5, 5, 5), chunk_size = 16
	// Expected: 5*16 * 5*16 * 5*16 = 80 * 80 * 80 = 512,000 voxels
	world_x = 5;
	world_y = 5;
	world_z = 5;
	chunk_size = 16;

	total_x = world_x * chunk_size;
	total_y = world_y * chunk_size;
	total_z = world_z * chunk_size;
	total_voxels = total_x * total_y * total_z;

	std::cout << "Test Case 3: World Size (5, 5, 5), Chunk Size 16" << std::endl;
	std::cout << "Total voxels: " << total_voxels << std::endl;
	std::cout << "Expected: 512,000" << std::endl;
	std::cout << "Match: " << (total_voxels == 512000 ? "YES" : "NO") << std::endl;
	std::cout << std::endl;

	// Test case 4: world_size = (10, 2, 10), chunk_size = 64
	// Expected: 10*64 * 2*64 * 10*64 = 640 * 128 * 640 = 52,428,800 voxels
	world_x = 10;
	world_y = 2;
	world_z = 10;
	chunk_size = 64;

	total_x = world_x * chunk_size;
	total_y = world_y * chunk_size;
	total_z = world_z * chunk_size;
	total_voxels = total_x * total_y * total_z;

	std::cout << "Test Case 4: World Size (10, 2, 10), Chunk Size 64" << std::endl;
	std::cout << "Total voxels: " << total_voxels << std::endl;
	std::cout << "Expected: 52,428,800" << std::endl;
	std::cout << "Match: " << (total_voxels == 52428800 ? "YES" : "NO") << std::endl;

	return 0;
}
