#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>

using namespace std;

// Erdős Unit Distance Problem Solver
// Maps FTD's Gaussian prime splitting (p = 1 mod 4) to generate dense unit distance graphs.
// The distance constraint is D^2 = R^2, where R^2 is a product of split primes.

int main(int argc, char* argv[]) {
    cout << "FTD Engine: Erdos Unit Distance Parallel Solver" << endl;
    
    int n = 50; // We want to find the densest graph of n points
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    
    // R^2 = 5 * 13 * 17 * 29 = 32045 (product of split primes)
    // This maximizes the number of representations as sum of two squares.
    long long R2 = 5LL * 13 * 17 * 29; 
    
    // Grid size to search within
    int L = 200; // [-L, L] x [-L, L]
    vector<pair<int,int>> points;
    for(int x = -L; x <= L; ++x) {
        for(int y = -L; y <= L; ++y) {
            points.push_back({x, y});
        }
    }
    int N = points.size();
    cout << "Generated Grid of " << N << " points." << endl;
    
    // Build adjacency list for D^2 = R^2
    vector<vector<int>> adj(N);
    int total_edges = 0;
    
    // Fast adjacency generation (only check bounding box)
    int radius = std::ceil(std::sqrt(R2));
    
    // To speed up, we can use a hash map or binary search, but since we are on a grid:
    // For each point, the valid neighbors are offsets (dx, dy) where dx^2 + dy^2 = R2
    vector<pair<int,int>> valid_offsets;
    for(int dx = -radius; dx <= radius; ++dx) {
        long long rem = R2 - (long long)dx * dx;
        if (rem < 0) continue;
        int dy = std::round(std::sqrt(rem));
        if ((long long)dy * dy == rem) {
            valid_offsets.push_back({dx, dy});
            if (dy != 0) valid_offsets.push_back({dx, -dy});
        }
    }
    
    // Remove duplicates
    sort(valid_offsets.begin(), valid_offsets.end());
    valid_offsets.erase(unique(valid_offsets.begin(), valid_offsets.end()), valid_offsets.end());
    
    cout << "Found " << valid_offsets.size() << " integer points on the circle of radius^2 = " << R2 << endl;
    
    for(int i = 0; i < N; ++i) {
        int px = points[i].first;
        int py = points[i].second;
        for(auto& off : valid_offsets) {
            int nx = px + off.first;
            int ny = py + off.second;
            if(nx >= -L && nx <= L && ny >= -L && ny <= L) {
                // Find neighbor index: index = (nx + L) * (2L + 1) + (ny + L)
                int nj = (nx + L) * (2 * L + 1) + (ny + L);
                adj[i].push_back(nj);
                total_edges++;
            }
        }
    }
    
    cout << "Graph constructed with " << total_edges / 2 << " edges." << endl;
    
    // Parallel search for densest n-subgraph
    atomic<int> global_max_edges(0);
    int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 24;
    
    auto start_time = chrono::steady_clock::now();
    bool keep_running = true;
    
    auto worker = [&]() {
        mt19937 gen(random_device{}());
        vector<int> current_set(n);
        vector<bool> in_set(N, false);
        
        while(keep_running) {
            in_set.assign(N, false);
            // Randomly select n points
            for(int i=0; i<n; ++i) {
                int p;
                do {
                    p = gen() % N;
                } while(in_set[p]);
                in_set[p] = true;
                current_set[i] = p;
            }
            
            // Local search: swap points to increase edges
            bool improved = true;
            int current_edges = 0;
            // Count initial edges
            for(int u : current_set) {
                for(int v : adj[u]) {
                    if (in_set[v]) current_edges++;
                }
            }
            current_edges /= 2; // Undirected
            
            while(improved) {
                improved = false;
                for(int i = 0; i < n; ++i) {
                    int u = current_set[i];
                    // Internal degree of u
                    int internal_deg = 0;
                    for(int v : adj[u]) if(in_set[v]) internal_deg++;
                    
                    // Try to find a better vertex outside
                    int best_outside = -1;
                    int best_gain = 0;
                    
                    // Instead of scanning all N (too slow), randomly sample 1000 outsiders
                    for(int tries = 0; tries < 1000; ++tries) {
                        int cand = gen() % N;
                        if(in_set[cand]) continue;
                        
                        int cand_internal_deg = 0;
                        for(int v : adj[cand]) if(in_set[v]) cand_internal_deg++;
                        
                        // If candidate adds more edges than u, swap!
                        if(cand_internal_deg > internal_deg) {
                            best_gain = cand_internal_deg - internal_deg;
                            best_outside = cand;
                            break; // Greedy fast accept
                        }
                    }
                    
                    if(best_outside != -1) {
                        in_set[u] = false;
                        in_set[best_outside] = true;
                        current_set[i] = best_outside;
                        current_edges += best_gain;
                        improved = true;
                    }
                }
            }
            
            int current_best = global_max_edges.load();
            if(current_edges > current_best) {
                int expected = current_best;
                while(current_edges > expected && !global_max_edges.compare_exchange_weak(expected, current_edges));
                if(current_edges > current_best) {
                    cout << "New best edges for n=" << n << ": " << current_edges << endl;
                }
            }
            
            auto now = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::seconds>(now - start_time).count() > 10) {
                break;
            }
        }
    };
    
    vector<thread> threads;
    for(int i=0; i<num_threads; ++i) threads.emplace_back(worker);
    for(auto& t : threads) t.join();
    
    cout << "Final Best Edges for n=" << n << " is: " << global_max_edges.load() << endl;
    return 0;
}
