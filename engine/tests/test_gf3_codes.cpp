#include <iostream>
#include <vector>
#include <random>
#include <fstream>
using namespace std;

int main() {
    int n = 11;
    int k = 6;
    vector<vector<int>> G(k, vector<int>(n));
    mt19937 gen(123);
    uniform_int_distribution<> dis(-1, 1);
    
    // Initialize random G
    for(int i=0; i<k; ++i)
        for(int j=0; j<n; ++j)
            G[i][j] = dis(gen);
            
    // Maximize minimum distance via simple hill climbing
    int best_d = 0;
    
    auto get_d = [&]() {
        int min_d = n+1;
        // Check all 3^k non-zero messages
        int limit = 1;
        for(int i=0; i<k; ++i) limit *= 3;
        for(int m=1; m<limit; ++m) {
            int weight = 0;
            vector<int> msg(k);
            int temp = m;
            for(int i=0; i<k; ++i) {
                msg[i] = (temp % 3) - 1;
                temp /= 3;
            }
            for(int j=0; j<n; ++j) {
                int sum = 0;
                for(int i=0; i<k; ++i) sum += msg[i] * G[i][j];
                if ((sum % 3) != 0) weight++;
            }
            if (weight > 0 && weight < min_d) min_d = weight;
        }
        return min_d;
    };
    
    best_d = get_d();
    
    for(int step=0; step<1000; ++step) {
        int r = gen() % k;
        int c = gen() % n;
        int old_val = G[r][c];
        int new_val = dis(gen);
        if(old_val == new_val) continue;
        
        G[r][c] = new_val;
        int d = get_d();
        if(d >= best_d) {
            best_d = d;
        } else {
            G[r][c] = old_val; // revert
        }
    }
    
    ofstream out("gf3_code.txt");
    out << best_d << "\n";
    for(int i=0; i<k; ++i) {
        for(int j=0; j<n; ++j) {
            out << G[i][j] << " ";
        }
        out << "\n";
    }
    cout << "GF(3) Code optimized. Best d=" << best_d << endl;
    return 0;
}
