#include <iostream>
#include <vector>
#include <random>
#include <fstream>
using namespace std;

int main() {
    int L = 20;
    int N = L * L * L;
    vector<int> grid(N, 0);
    
    mt19937 gen(99);
    uniform_int_distribution<> dis(-1, 1);
    
    // Seed center
    for(int i=0; i<100; ++i) {
        int x = L/2 + (gen()%5 - 2);
        int y = L/2 + (gen()%5 - 2);
        int z = L/2 + (gen()%5 - 2);
        grid[x*L*L + y*L + z] = dis(gen);
    }
    
    auto idx = [&](int x, int y, int z) {
        return ((x+L)%L)*L*L + ((y+L)%L)*L + ((z+L)%L);
    };
    
    // Random ternary CA rule table mapping sum (-26 to 26) to next state
    vector<int> rule(53);
    for(int i=0; i<53; ++i) rule[i] = dis(gen);
    rule[26] = 0; // Empty space stays empty if sum is 0
    
    for(int step=0; step<10; ++step) {
        vector<int> next_grid(N, 0);
        for(int x=0; x<L; ++x) {
            for(int y=0; y<L; ++y) {
                for(int z=0; z<L; ++z) {
                    int sum = 0;
                    for(int dx=-1; dx<=1; ++dx) {
                        for(int dy=-1; dy<=1; ++dy) {
                            for(int dz=-1; dz<=1; ++dz) {
                                if(dx==0 && dy==0 && dz==0) continue;
                                sum += grid[idx(x+dx, y+dy, z+dz)];
                            }
                        }
                    }
                    next_grid[idx(x,y,z)] = rule[sum + 26];
                }
            }
        }
        grid = next_grid;
    }
    
    ofstream out("ca_snapshot.txt");
    out << L << "\n";
    for(int x=0; x<L; ++x) {
        for(int y=0; y<L; ++y) {
            for(int z=0; z<L; ++z) {
                if (grid[idx(x,y,z)] != 0) {
                    out << x << " " << y << " " << z << " " << grid[idx(x,y,z)] << "\n";
                }
            }
        }
    }
    cout << "3D CA generation complete." << endl;
    return 0;
}
