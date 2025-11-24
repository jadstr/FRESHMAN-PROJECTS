#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

struct Stat {
    string name;
    double value;
    int upgrades;
    double baseGrowth;
    double perUpgradeGrowth;
};

Stat statList[5] = {
    {"HP", 150.0, 1, 21.0, 4.0},
    {"ATK", 20.0, 1, 11.0, 2.0},
    {"END", 30.0, 1, 15.0, 3.0},
    {"EP Pool", 40.0, 1, 15.0, 3.0},
    {"EP Regen", 2.0, 1, 2.5, 0.5}
};

string selfDiscovery[8] = {
    "Gate of Opening (G1)", "Chain Handling", "Keigan Barrage", "First Lotus",
    "Gate of Mastery", "Gate of Healing (G2)", "Chain Barrage", "Reverse Lotus"
};

bool skillsUnlocked[8] = {true,true,true,true,false,false,false,false}; // demo: first 4 unlocked
bool skillsActive[8]   = {false,false,false,false,false,false,false,false};

double skillMultiplier[8][5] = {
    {1.0, 1.8, 1.0, 1.0, 1.0}, // G1: ATK x1.8
    {1.0, 1.0, 1.0, 1.0, 1.0}, // Chain Handling: passive
    {1.0, 1.0, 1.0, 1.0, 1.0}, // Keigan Barrage: flat ATK add
    {1.0, 1.0, 1.0, 1.0, 1.0}, // First Lotus: flat adds
    {1.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0}
};

double skillFlat[8][5] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 6.0, 0, 0, 0},   // Keigan Barrage -> ATK +6
    {0, 3.0, 8.0, 0, 0}, // First Lotus -> ATK +3, END +8
    {0, 0, 0, 0, 0},
    {30.0,0,0,0,0},      // Gate Healing -> HP +30
    {0, 8.0,0,0,0},
    {0, 4.0,9.0,0,0}
};

// track applied deltas so we can revert toggles

double skillAppliedDelta[8][5] = {0};

void printStats() {
    cout << "\n-- Current Stats --\n";
    for (int i=0;i<5;++i) cout << statList[i].name << ": " << fixed << setprecision(1) << statList[i].value << " | ";
    cout << "\n";
}

void listSkills() {
    cout << "\nAvailable Skills:\n";
    for (int i=0;i<8;++i) {
        cout << "[" << (i+1) << "] " << selfDiscovery[i]
             << " - " << (skillsUnlocked[i] ? "Unlocked" : "Locked")
             << " - " << (skillsActive[i] ? "Active" : "Inactive") << "\n";
    }
}

void activateSkill(int idx) {
    if (idx<0 || idx>=8) return;
    if (!skillsUnlocked[idx]) { cout << "Skill not unlocked." << endl; return; }
    if (skillsActive[idx]) { cout << "Skill already active." << endl; return; }

    // Gate of Opening (G1) is multiplicative on ATK
    if (idx == 0) {
        double cur = statList[1].value; // ATK
        double delta = cur * (skillMultiplier[idx][1] - 1.0);
        statList[1].value += delta;
        skillAppliedDelta[idx][1] = delta;
    } else {
        for (int s=0;s<5;++s) {
            double delta = skillFlat[idx][s];
            statList[s].value += delta;
            skillAppliedDelta[idx][s] = delta;
        }
    }
    skillsActive[idx] = true;
    cout << "Activated: " << selfDiscovery[idx] << "\n";
}

void deactivateSkill(int idx) {
    if (idx<0 || idx>=8) return;
    if (!skillsActive[idx]) { cout << "Skill not active." << endl; return; }

    if (idx == 0) {
        statList[1].value -= skillAppliedDelta[idx][1];
        skillAppliedDelta[idx][1] = 0.0;
    } else {
        for (int s=0;s<5;++s) {
            statList[s].value -= skillAppliedDelta[idx][s];
            skillAppliedDelta[idx][s] = 0.0;
        }
    }
    skillsActive[idx] = false;
    cout << "Deactivated: " << selfDiscovery[idx] << "\n";
}

void toggleSkill(int idx) {
    if (idx<0 || idx>=8) return;
    if (!skillsUnlocked[idx]) { cout << "Not unlocked." << endl; return; }
    if (skillsActive[idx]) deactivateSkill(idx); else activateSkill(idx);
}

void bossFightSimulation() {
    cout << "\n=== Boss Fight Demo ===\n";
    printStats();
    cout << "Boss appears! (Damage per hit = 35)\n";

    for (int turn=1; turn<=3; ++turn) {
        cout << "\n-- Boss Turn " << turn << " --\n";
        // Simple boss damage: 35 reduced by END * 0.1
        double bossRaw = 35.0;
        double reduction = statList[2].value * 0.1; // END reduces damage
        double damage = max(0.0, bossRaw - reduction);
        statList[0].value -= damage;
        cout << "Boss deals " << fixed << setprecision(1) << damage << " damage.\n";
        printStats();
        if (statList[0].value <= 0) { cout << "You were defeated in demo!\n"; return; }
    }

    cout << "\nBoss fight demo ended. You survived the demo.\n";
}

int main() {
    cout << "Boss Scene Demo - toggle skills and simulate a short boss fight.\n";

    while (true) {
        cout << "\nMenu:\n[1] List Skills\n[2] Toggle Skill\n[3] Start Boss Fight Demo\n[4] Show Stats\n[5] Exit\nEnter choice: ";
        string line; getline(cin, line);
        if (line.empty()) continue;
        stringstream ss(line); int c; if (!(ss>>c)) { cout << "Invalid input\n"; continue; }

        if (c==1) listSkills();
        else if (c==2) {
            cout << "Enter skill number to toggle: "; string t; getline(cin,t); stringstream ts(t); int s; if (!(ts>>s)) { cout<<"Invalid\n"; continue; }
            toggleSkill(s-1);
        }
        else if (c==3) { bossFightSimulation(); }
        else if (c==4) { printStats(); }
        else if (c==5) { cout<<"Exiting demo.\n"; break; }
        else cout<<"Unknown choice\n";
    }

    return 0;
}
