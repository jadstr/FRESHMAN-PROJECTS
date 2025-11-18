#include <iostream>
#include <vector>
#include <string>

using namespace std;

void trainArc() {
    
    int leftTurns = 42;

    // Keigan Base Stats
    int HP = 21;
    int ATK = 11;
    int END = 15;
    int EPool = 15;
    double EPRegen = 2.5;

    // Stat Counter
    int nATK = 1;
    int nHP = 1;
    int nEND = 1; 
    int nEP = 1;
    int nEPRegen = 1;

    while (leftTurns > 0) {
        cout<<"============================================================================================================="<<endl;
        cout << "You have " << leftTurns << " training turns left." << endl;
        cout << "Choose a stat to train:" << endl;
        cout << "[1]. ATK (Current: " << ATK << ", Upgrades: " << nATK << ")" << endl;
        cout << "[2]. HP (Current: " << HP << ", Upgrades: " << nHP << ")" << endl;
        cout << "[3]. END (Current: " << END << ", Upgrades: " << nEND << ")" << endl;
        cout << "[4]. EP Pool (Current: " << EPool << ", Upgrades: " << nEP << ")" << endl;
        cout << "[5]. EP Regen (Current: " << EPRegen << ", Upgrades: " << nEPRegen << ")" << endl;
        cout << "[6]. Self Discovery " <<endl;
        cout << "Enter the number of the stat you want to train: ";

        int statChoice;
        cin >> statChoice;

        switch (statChoice) {
            case 1:
                ATK = 11 + 2 *(nATK - 1);
                nATK++;
                break;
            case 2:
                HP = 21 + 4 * (nHP - 1);
                nHP++;
                break;
            case 3:
                END = 15 + 3 * (nEND - 1);
                nEND++;
                break;
            case 4:
                EPool = 15 + 3 * (nEP - 1);
                nEP++;
                break;
            case 5:
                EPRegen = 2.5 + 0.5 * (nEPRegen - 1);
                nEPRegen++;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
                continue; // Skip decrementing leftTurns
        }

        leftTurns--;   
        
        cout<<"============================================================================================================="<<endl;
        cout<< "Stat updated! Current stats:" << endl;
        cout << "ATK: " << ATK << ", HP: " << HP << ", END: " << END << ", EP Pool: " << EPool << ", EP Regen: " << EPRegen << endl;
        cout<<"============================================================================================================="<<endl;


    }

}

int main() 
{ 
    trainArc();
    return 0;
}
