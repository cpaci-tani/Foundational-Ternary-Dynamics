#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <fstream>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;

// Compute linear index for edge (u,v) in an N-vertex graph
inline int edge_idx(int u, int v, int N) {
    if (u > v) swap(u, v);
    return u * N - (u * (u + 1)) / 2 + (v - u - 1);
}

// Full O(N^4) count to get the initial baseline
int count_mono_K4_full(const vector<int>& edges, int N) {
    int mono_count = 0;
    for (int i=0; i<N; ++i) {
        for (int j=i+1; j<N; ++j) {
            for (int k=j+1; k<N; ++k) {
                for (int l=k+1; l<N; ++l) {
                    int c1 = edges[edge_idx(i,j,N)];
                    if (edges[edge_idx(i,k,N)] == c1 && edges[edge_idx(i,l,N)] == c1 &&
                        edges[edge_idx(j,k,N)] == c1 && edges[edge_idx(j,l,N)] == c1 &&
                        edges[edge_idx(k,l,N)] == c1) {
                        mono_count++;
                    }
                }
            }
        }
    }
    return mono_count;
}

// Incremental update: count how many monochromatic K4s contain edge (u,v) with its CURRENT color
int count_local_K4(const vector<int>& edges, int N, int u, int v) {
    int local_count = 0;
    int c = edges[edge_idx(u, v, N)];
    for (int x = 0; x < N; ++x) {
        if (x == u || x == v) continue;
        for (int y = x + 1; y < N; ++y) {
            if (y == u || y == v) continue;
            if (edges[edge_idx(u, x, N)] == c && edges[edge_idx(u, y, N)] == c &&
                edges[edge_idx(v, x, N)] == c && edges[edge_idx(v, y, N)] == c &&
                edges[edge_idx(x, y, N)] == c) {
                local_count++;
            }
        }
    }
    return local_count;
}

int main(int argc, char** argv) {
    int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 24;
    
    int N = 128; // Default to the true goal
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    
    int num_edges = N * (N - 1) / 2;
    
    // Precompute edge to (u,v) mapping for fast O(1) lookup
    vector<pair<int, int>> edge_to_uv(num_edges);
    int e_idx = 0;
    for (int u = 0; u < N; ++u) {
        for (int v = u + 1; v < N; ++v) {
            edge_to_uv[e_idx++] = {u, v};
        }
    }
    
    cout << "FTD Engine: Ramsey Multicolor R(4,4,4) /Goal Campaign" << endl;
    cout << "Target Graph size N=" << N << " with " << num_edges << " edges." << endl;
    cout << "Starting Hyper-Optimized Incremental Simulated Annealing on " << num_threads << " threads..." << endl;
    
    atomic<bool> found(false);
    vector<int> best_global_edges(num_edges);
    atomic<int> global_min_k4(-1);
    
    auto worker = [&](int seed) {
        mt19937 gen(1337 + seed);
        uniform_int_distribution<> dis(-1, 1);
        uniform_int_distribution<> edge_dis(0, num_edges - 1);
        uniform_real_distribution<> prob(0.0, 1.0);
        
        vector<int> edges(num_edges);
        for(int& e : edges) e = dis(gen);
        
        int current_k4 = count_mono_K4_full(edges, N);
        
        if (global_min_k4.load() == -1 || current_k4 < global_min_k4.load()) {
            int expected = global_min_k4.load();
            while (expected == -1 || current_k4 < expected) {
                if (global_min_k4.compare_exchange_weak(expected, current_k4)) break;
            }
        }
        
        long long max_steps = 10000000; // 10 million steps per thread
        
        for (long long step = 0; step < max_steps && !found.load(); ++step) {
            if (current_k4 == 0) {
                bool expected = false;
                if (found.compare_exchange_strong(expected, true)) {
                    best_global_edges = edges;
                }
                break;
            }
            
            int e = edge_dis(gen);
            int u = edge_to_uv[e].first;
            int v = edge_to_uv[e].second;
            
            int old_color = edges[e];
            int new_color = dis(gen);
            if (old_color == new_color) continue;
            
            // Incremental evaluation
            int k4_before = count_local_K4(edges, N, u, v);
            edges[e] = new_color;
            int k4_after = count_local_K4(edges, N, u, v);
            
            int delta_k4 = k4_after - k4_before;
            int new_k4 = current_k4 + delta_k4;
            
            // Simulated Annealing acceptance criteria
            double temp = max(0.01, 10.0 * (1.0 - (double)step / max_steps));
            
            if (delta_k4 <= 0 || prob(gen) < exp(-delta_k4 / temp)) {
                current_k4 = new_k4; // Accept
                
                int current_min = global_min_k4.load();
                while (current_k4 < current_min) {
                    if (global_min_k4.compare_exchange_weak(current_min, current_k4)) break;
                }
            } else {
                edges[e] = old_color; // Revert
            }
        }
    };
    
    vector<thread> threads;
    for(int i=0; i<num_threads; ++i) {
        threads.emplace_back(worker, i);
    }
    for(auto& t : threads) {
        t.join();
    }
    
    if (found) {
        cout << "SUCCESS! Valid R(4,4,4) ternary coloring found for N=" << N << "!" << endl;
        string filename = "ramsey_R444_N" + to_string(N) + "_solution.txt";
        ofstream out(filename);
        out << "N=" << N << "\n";
        out << "Edges (flattened upper triangular): \n";
        for(int e : best_global_edges) {
            out << e << " ";
        }
        out << "\n";
        cout << "Exported mathematical proof to " << filename << endl;
    } else {
        cout << "Completed search. Minimum K4 count reached: " << global_min_k4.load() << endl;
        cout << "Did not fully resolve N=" << N << " to zero." << endl;
    }
    
    return 0;
}
