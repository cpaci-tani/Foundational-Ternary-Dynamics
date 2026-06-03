#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <fstream>
using namespace std;

int main() {
    int L = 16;
    int N = L * L * L;
    vector<int> lattice(N);
    mt19937 gen(42);
    uniform_int_distribution<> dis(-1, 1);
    for(int i=0; i<N; ++i) lattice[i] = dis(gen);
    
    auto idx = [&](int x, int y, int z) {
        return ((x+L)%L)*L*L + ((y+L)%L)*L + ((z+L)%L);
    };
    
    auto get_energy = [&](int i, int x, int y, int z) {
        int e = 0;
        int s = lattice[i];
        int neighbors[] = {
            idx(x+1,y,z), idx(x-1,y,z), idx(x,y+1,z),
            idx(x,y-1,z), idx(x,y,z+1), idx(x,y,z-1)
        };
        for(int n : neighbors) {
            if(s == lattice[n]) e -= 1;
        }
        return e;
    };
    
    ofstream out("potts_energy.txt");
    uniform_real_distribution<> prob(0.0, 1.0);
    
    for(double T = 3.0; T > 0.1; T -= 0.05) {
        // Thermalize
        for(int step=0; step<100; ++step) {
            for(int i=0; i<N; ++i) {
                int z = i % L;
                int y = (i / L) % L;
                int x = i / (L * L);
                int old_s = lattice[i];
                int e_old = get_energy(i, x, y, z);
                
                int new_s = dis(gen);
                if(new_s == old_s) continue;
                lattice[i] = new_s;
                int e_new = get_energy(i, x, y, z);
                
                int delta = e_new - e_old;
                if(delta > 0 && prob(gen) > exp(-delta / T)) {
                    lattice[i] = old_s; // reject
                }
            }
        }
        // Measure
        double total_E = 0;
        for(int i=0; i<N; ++i) {
            int z = i % L;
            int y = (i / L) % L;
            int x = i / (L * L);
            total_E += get_energy(i, x, y, z);
        }
        out << T << " " << total_E / N << "\n";
    }
    cout << "Potts 3D simulation complete." << endl;
    return 0;
}
