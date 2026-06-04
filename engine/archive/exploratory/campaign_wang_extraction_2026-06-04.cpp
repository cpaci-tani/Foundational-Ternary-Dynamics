#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

// Archived 2026-06-04 from engine/tests/campaign_wang_extraction.cpp.
// Standalone local exploration prototype; not registered in CMake/CTest.
// Restore deliberately before use.

const int ROTATIONS[24][6] = {
    {0,1,2,3,4,5}, {0,1,3,2,5,4}, {0,1,5,4,2,3}, {0,1,4,5,3,2},
    {1,0,2,3,5,4}, {1,0,3,2,4,5}, {1,0,4,5,2,3}, {1,0,5,4,3,2},
    {2,3,0,1,5,4}, {3,2,0,1,4,5}, {4,5,0,1,2,3}, {5,4,0,1,3,2},
    {2,3,1,0,4,5}, {3,2,1,0,5,4}, {5,4,1,0,2,3}, {4,5,1,0,3,2},
    {2,3,4,5,0,1}, {3,2,5,4,0,1}, {5,4,2,3,0,1}, {4,5,3,2,0,1},
    {2,3,5,4,1,0}, {3,2,4,5,1,0}, {4,5,2,3,1,0}, {5,4,3,2,1,0}
};

std::mutex print_mutex;

void check_range(int start_idx, int end_idx) {
    for (int idx = start_idx; idx < end_idx; ++idx) {
        int colors[6];
        colors[0] = idx % 16;
        colors[1] = (idx / 16) % 16;
        colors[2] = (idx / 256) % 16;
        colors[3] = (idx / 4096) % 16;
        colors[4] = (idx / 65536) % 16;
        colors[5] = (idx / 1048576) % 16;
        
        int tiles[24][6];
        for(int r = 0; r < 24; r++) {
            for(int f = 0; f < 6; f++) {
                tiles[r][f] = colors[ROTATIONS[r][f]];
            }
        }
        
        bool periodic_1x1 = false;
        for(int r = 0; r < 24; r++) {
            if (tiles[r][0] == tiles[r][1] && 
                tiles[r][2] == tiles[r][3] && 
                tiles[r][4] == tiles[r][5]) {
                periodic_1x1 = true;
                break;
            }
        }
        if (periodic_1x1) continue;
        
        int T_per[8];
        int stack_per[8];
        for(int i = 0; i < 8; i++) stack_per[i] = 0;
        
        int depth_per = 0;
        bool is_periodic = false;
        int steps_per = 0;
        
        while(depth_per >= 0 && steps_per < 50000) {
            steps_per++;
            if (depth_per == 8) {
                bool periodic = true;
                for(int y = 0; y <= 1; y++) {
                    for(int z = 0; z <= 1; z++) {
                        if (tiles[T_per[0 + 2*y + 4*z]][1] != tiles[T_per[1 + 2*y + 4*z]][0]) periodic = false;
                    }
                }
                for(int x = 0; x <= 1; x++) {
                    for(int z = 0; z <= 1; z++) {
                        if (tiles[T_per[x + 0 + 4*z]][3] != tiles[T_per[x + 2 + 4*z]][2]) periodic = false;
                    }
                }
                for(int x = 0; x <= 1; x++) {
                    for(int y = 0; y <= 1; y++) {
                        if (tiles[T_per[x + 2*y + 0]][5] != tiles[T_per[x + 2*y + 4]][4]) periodic = false;
                    }
                }
                if (periodic) { is_periodic = true; break; }
                depth_per--; stack_per[depth_per]++;
                continue;
            }
            
            int t = stack_per[depth_per];
            if (t >= 24) {
                depth_per--;
                if (depth_per >= 0) stack_per[depth_per]++;
                continue;
            }
            
            bool valid = true;
            int x = depth_per % 2;
            int y = (depth_per / 2) % 2;
            int z = (depth_per / 4) % 2;
            
            if (x == 1) { if (tiles[t][1] != tiles[T_per[depth_per - 1]][0]) valid = false; }
            if (valid && y == 1) { if (tiles[t][3] != tiles[T_per[depth_per - 2]][2]) valid = false; }
            if (valid && z == 1) { if (tiles[t][5] != tiles[T_per[depth_per - 4]][4]) valid = false; }
            
            if (valid) {
                T_per[depth_per] = t;
                depth_per++;
                if (depth_per < 8) stack_per[depth_per] = 0;
            } else {
                stack_per[depth_per]++;
            }
        }
        if (is_periodic) continue;
        
        int T[64];
        int stack[64];
        for(int i = 0; i < 64; i++) stack[i] = 0;
        
        int depth = 0;
        bool can_tile_4x4x4 = false;
        int steps = 0;
        
        while(depth >= 0 && steps < 50000000) {
            steps++;
            if (depth == 64) {
                can_tile_4x4x4 = true;
                break;
            }
            
            int t = stack[depth];
            if (t >= 24) {
                depth--;
                if (depth >= 0) stack[depth]++;
                continue;
            }
            
            bool valid = true;
            int x = depth % 4;
            int y = (depth / 4) % 4;
            int z = (depth / 16) % 4;
            
            if (x > 0) { if (tiles[t][1] != tiles[T[depth - 1]][0]) valid = false; }
            if (valid && y > 0) { if (tiles[t][3] != tiles[T[depth - 4]][2]) valid = false; }
            if (valid && z > 0) { if (tiles[t][5] != tiles[T[depth - 16]][4]) valid = false; }
            
            if (valid) {
                T[depth] = t;
                depth++;
                if (depth < 64) stack[depth] = 0;
            } else {
                stack[depth]++;
            }
        }
        
        if (can_tile_4x4x4) {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "CANDIDATE: " << colors[0] << "," << colors[1] << "," 
                      << colors[2] << "," << colors[3] << "," 
                      << colors[4] << "," << colors[5] << std::endl;
        }
    }
}

int main() {
    int total_configs = 16777216;
    int num_threads = 32; // Optimized for 9950X3D
    int chunk_size = total_configs / num_threads;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? total_configs : start + chunk_size;
        threads.emplace_back(check_range, start, end);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
