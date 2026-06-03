#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

// Figure-Eight Knot (4 crossings, 4 arcs)
// Crossings:
// C0: x0 + x2 - 2x1 = 0 mod 3 => x0 + x2 + x1 = 0 mod 3
// C1: x1 + x3 - 2x2 = 0 mod 3 => x1 + x3 + x2 = 0 mod 3
// C2: x2 + x0 - 2x3 = 0 mod 3 => x2 + x0 + x3 = 0 mod 3
// C3: x3 + x1 - 2x0 = 0 mod 3 => x3 + x1 + x0 = 0 mod 3

int main() {
    ofstream out("fox_solution.txt");
    vector<int> states = {-1, 0, 1};
    
    bool found = false;
    for(int x0 : states) {
        for(int x1 : states) {
            for(int x2 : states) {
                for(int x3 : states) {
                    if (x0 == x1 && x1 == x2 && x2 == x3) continue; // Trivial
                    
                    if ((x0 + x1 + x2 + 30) % 3 == 0 &&
                        (x1 + x2 + x3 + 30) % 3 == 0 &&
                        (x2 + x3 + x0 + 30) % 3 == 0 &&
                        (x3 + x0 + x1 + 30) % 3 == 0) {
                        out << x0 << " " << x1 << " " << x2 << " " << x3 << "\n";
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            if (found) break;
        }
        if (found) break;
    }
    
    if (found) cout << "Valid non-trivial Fox 3-Coloring found for Figure-Eight Knot!" << endl;
    else cout << "No coloring found (expected for Figure-Eight)." << endl;
    
    // Trefoil (3 arcs)
    ofstream out2("trefoil_solution.txt");
    found = false;
    for(int y0 : states) {
        for(int y1 : states) {
            for(int y2 : states) {
                if (y0 == y1 && y1 == y2) continue;
                if ((y0 + y1 + y2 + 30) % 3 == 0) {
                    out2 << y0 << " " << y1 << " " << y2 << "\n";
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
    }
    cout << "Trefoil coloring done." << endl;
    return 0;
}
