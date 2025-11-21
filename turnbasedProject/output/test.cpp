#include <iostream>
#include <string>
#include <iomanip>
#include <limits>

using namespace std;

// -------------------------
// BASE STATS
// -------------------------
double stats[5] = {50, 10, 15, 15, 0}; // HP, ATK, END, EPool, EPRegen
string statNames[5] = {"HP", "ATK", "END", "EP Pool", "EP Regen"};
double statGrowth[5][2] = {
    {21, 4},    // HP growth
    {11, 2},    // ATK growth
    {15, 3},    // END growth
    {15, 3},    // EP Pool growth
    {2.5, 0.5}  // EP Regen growth
};

// TRAIN COUNTERS
int upgrades[5] = {1, 1, 1, 1, 1};

// SELF-DISCOVERY SKILLS
string selfDiscovery[8] = {
    "Gate of Opening (G1)", "Chain Handling", "Keigan Barrage", "First Lotus",
    "Gate of Mastery", "Gate of Healing (G2)", "Chain Barrage", "Reverse Lotus"
};

bool skillsUnlocked[8] = {false, false, false, false, false, false, false, false};
int nDiscovery = 0;

// TRAINING FUNCTION
void trainArc() {
    int leftTurns = 42;

    while (leftTurns > 0) {
        cout << "=============================================================================================================\n";
        cout << "You have " << leftTurns << " training turns left." << endl;
        cout << "Choose a stat to train:" << endl;

        for (int i = 0; i < 5; i++) {
            cout << "[" << (i + 1) << "] " << statNames[i] 
                 << " (Current: " << fixed << setprecision(1) << stats[i] 
                 << ", Upgrades: " << upgrades[i] << ")" << endl;
        }
        cout << "[6] Self Discovery (Unlocked: " << nDiscovery << ")" << endl;
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;

        // Validate input
        if (cin.fail()) {
            cin.clear(); // Clear error state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Remove invalid input
            cout << "Invalid input. Please enter a number between 1-6." << endl;
            continue; // Ask again without consuming turn
        }

        if (choice >= 1 && choice <= 5) {
            int idx = choice - 1;
            stats[idx] += statGrowth[idx][0] + statGrowth[idx][1] * (upgrades[idx] - 1);
            upgrades[idx]++;
        } 
        else if (choice == 6) {
            if (nDiscovery < 8) {
                cout << "You unlocked: " << selfDiscovery[nDiscovery] << "!" << endl;
                skillsUnlocked[nDiscovery] = true; // Mark skill as unlocked
                nDiscovery++;
            } else {
                cout << "All self-discovery skills already unlocked!" << endl;
                continue; // don't consume turn
            }
        } 
        else {
            cout << "Invalid choice. Enter a number between 1-6." << endl;
            continue; // don't consume turn
        }

        leftTurns--;

        cout << "=============================================================================================================\n";
        cout << "Updated stats:" << endl;
        for (int i = 0; i < 5; i++) {
            cout << statNames[i] << ": " << fixed << setprecision(1) << stats[i] << " | ";
        }

        cout << "\nSelf Discovery Unlocked: " << nDiscovery << "/8" << endl;
        cout << "Skills Unlocked Status: ";
        for (int i = 0; i < 8; i++) {
            cout << (skillsUnlocked[i] ? "/" : "x") << " ";
        }
        cout << "\n=============================================================================================================\n\n";
    }
}

int main() {
    trainArc();

    cout << "Training complete! Final stats:" << endl;
    for (int i = 0; i < 5; i++) {
        cout << statNames[i] << ": " << fixed << setprecision(1) << stats[i] << " | ";
    }
    cout << "\nSelf Discovery unlocked: " << nDiscovery << "/8" << endl;
    cout << "Skills Unlocked Status: ";
    for (int i = 0; i < 8; i++) {
        cout << (skillsUnlocked[i] ? "/" : "X") << " ";
    }
    cout << endl;

    return 0;
}

