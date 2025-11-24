#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <limits>
#include <algorithm>

using namespace std;

struct Stat {
    string name;
    double value;
    int upgrades;
    double baseGrowth;
    double perUpgradeGrowth;
};

Stat statList[5] = {
    {"HP", 50.0, 1, 21.0, 4.0},
    {"ATK", 10.0, 1, 11.0, 2.0},
    {"END", 15.0, 1, 15.0, 3.0},
    {"EP Pool", 15.0, 1, 15.0, 3.0},
    {"EP Regen", 0.0, 1, 2.5, 0.5}
};

int months = 0;

string selfDiscovery[8] = {
    "Gate of Opening (G1)", "Chain Handling", "Keigan Barrage", "First Lotus",
    "Gate of Mastery", "Gate of Healing (G2)", "Chain Barrage", "Reverse Lotus"
};

bool skillsUnlocked[8] = {false,false,false,false,false,false,false,false};
int nDiscovery = 0;

// ---------- Skill metadata and runtime state ----------
struct Skill {
    string name;
    string type;
    bool isStance; // toggled/stance skills incur per-turn EP
    int epCast;    // EP cost when casting/activating
    int epPerTurn; // EP drain per turn when active
    int cooldown;  // cooldown in turns after use (0 if none)
    int cdRemaining; // runtime remaining cooldown
    bool requiresGateOn;
    bool requiresFrontLotusUsed;
    double multiplier[5];
    double flat[5];
};

Skill skills[8];
bool skillUsedOnce[8] = {false,false,false,false,false,false,false,false}; // track uses (for Reverse Lotus req)

// initialize skills metadata (values taken from attachments)
void initSkills() {
    // G1: Gate of Opening - stance: ATK x1.8, cast 35 EP, 7 EP/turn
    skills[0] = {"Gate of Opening (G1)", "Stance/Toggle", true, 35, 7, 0, 0, false, false,
                 {1.0,1.8,1.0,1.0,1.0}, {0,0,0,0,0}};
    // Chain Handling: passive
    skills[1] = {"Chain Handling", "Passive", false, 0, 0, 0, 0, false, false,
                 {1,1,1,1,1}, {0,0,0,0,0}};
    // Kei-ga-n Barrage: combo attack, EP 12 (modeled as flat ATK)
    skills[2] = {"Kei-ga-n Barrage", "Combo", false, 12, 0, 0, 0, false, false,
                 {1,1,1,1,1}, {0,4.0,0,0,0}};
    // Front Lotus (Omote Renge): finisher, EP 13
    skills[3] = {"Front Lotus", "Finisher", false, 13, 0, 0, 0, false, false,
                 {1,1,1,1,1}, {0,2.0,5.0,0,0}};
    // Gate of Mastery: passive/mastery (reduces costs) - model as passive
    skills[4] = {"Gate of Mastery", "Passive", false, 0, 0, 0, 0, false, false,
                 {1,1,1,1,1}, {0,0,0,0,0}};
    // Gate of Healing (G2): stance/toggle, ATK x2.2, cast 45 EP, 9 EP/turn
    skills[5] = {"Gate of Healing (G2)", "Stance/Toggle", true, 45, 9, 0, 0, false, false,
                 {1,2.2,1,1,1}, {0,0,0,0,0}};
    // Chain Barrage: combo, EP 14
    skills[6] = {"Chain Barrage", "Combo", false, 14, 0, 0, 0, false, false,
                 {1,1,1,1,1}, {0,5.0,0,0,0}};
    // Reverse Lotus: finisher, EP 25, requires Gate ON and Front Lotus used once, CD 5
    skills[7] = {"Reverse Lotus", "Finisher", false, 25, 0, 5, 0, true, true,
                 {1,1.25,1,1,1}, {0,3.0,6.0,0,0}};
}

// runtime arrays
int skillCooldownRemaining[8] = {0,0,0,0,0,0,0,0};


// ---------- Skill unlock notifications (no immediate stat changes) ----------
void unlock_GateOfOpening() { cout << "[Unlocked] Eight Gates - Gate of Opening (G1): Skill available." << endl; }
void unlock_ChainHandling() { cout << "[Unlocked] Chain Handling: Skill available." << endl; }
void unlock_KeiganBarrage() { cout << "[Unlocked] Kei-ga-n Barrage!: Skill available." << endl; }
void unlock_FirstLotus() { cout << "[Unlocked] Front Lotus (Omote Renge): Skill available." << endl; }
void unlock_GateOfMastery() { cout << "[Unlocked] Gate of Mastery: Skill available." << endl; }
void unlock_GateOfHealing() { cout << "[Unlocked] Gate of Healing (G2): Skill available." << endl; }
void unlock_ChainBarrage() { cout << "[Unlocked] Chain Barrage: Skill available." << endl; }
void unlock_ReverseLotus() { cout << "[Unlocked] Reverse Lotus: Skill available." << endl; }

// Skill activation data (applied later during combat scenes via toggle functions)
double skillMultiplier[8][5] = {
    {1.0, 1.8, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0}
};

double skillFlat[8][5] = {
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,4.0,0,0,0},
    {0,2.0,5.0,0,0},
    {0,0,0,0,0},
    {20.0,0,0,0,0},
    {0,5.0,0,0,0},
    {0,3.0,6.0,0,0}
};

double skillAppliedDelta[8][5] = {0};
bool skillsActive[8] = {false,false,false,false,false,false,false,false};

void activateSkillEffect(int idx) {
    if (idx < 0 || idx >= 8) return;
    if (!skillsUnlocked[idx]) { cout << "Skill not unlocked yet." << endl; return; }
    if (skillsActive[idx]) { cout << "Skill already active." << endl; return; }
    // check requirements
    if (skills[idx].requiresGateOn && !skillsActive[0]) {
        cout << "Requires Gate of Opening (G1) to be active." << endl; return;
    }
    if (skills[idx].requiresFrontLotusUsed && !skillUsedOnce[3]) {
        cout << "Requires Front Lotus to have been used at least once." << endl; return;
    }

    // check cooldown
    if (skillCooldownRemaining[idx] > 0) { cout << "Skill is on cooldown (" << skillCooldownRemaining[idx] << " turns)." << endl; return; }

    // check EP pool for cast cost
    if (skills[idx].epCast > 0) {
        if (statList[3].value < skills[idx].epCast) { cout << "Not enough EP to activate " << skills[idx].name << "." << endl; return; }
        statList[3].value -= skills[idx].epCast;
    }

    // apply effects
    if (idx == 4) {
        for (int i=0;i<5;++i) { statList[i].baseGrowth += 2.0; skillAppliedDelta[idx][i] = 2.0; }
    } else {
        for (int i=0;i<5;++i) {
            double current = statList[i].value;
            double delta = current * (skills[idx].multiplier[i] - 1.0) + skills[idx].flat[i];
            statList[i].value += delta;
            skillAppliedDelta[idx][i] = delta;
        }
    }

    skillsActive[idx] = true;
    skillUsedOnce[idx] = true;
    cout << "Skill activated: " << skills[idx].name << "\n";
}

void deactivateSkillEffect(int idx) {
    if (idx < 0 || idx >= 8) return;
    if (!skillsActive[idx]) { cout << "Skill is not active." << endl; return; }

    if (idx == 4) {
        for (int i=0;i<5;++i) { statList[i].baseGrowth -= skillAppliedDelta[idx][i]; skillAppliedDelta[idx][i]=0.0; }
    } else {
        for (int i=0;i<5;++i) { statList[i].value -= skillAppliedDelta[idx][i]; skillAppliedDelta[idx][i]=0.0; }
    }

    // apply cooldown if skill defines one
    if (skills[idx].cooldown > 0) skillCooldownRemaining[idx] = skills[idx].cooldown;

    skillsActive[idx] = false;
    cout << "Skill deactivated: " << skills[idx].name << "\n";
}

void toggleSkill(int idx) {
    if (idx < 0 || idx >= 8) return;
    if (!skillsUnlocked[idx]) { cout << "Cannot toggle. Skill not unlocked." << endl; return; }
    if (skillsActive[idx]) deactivateSkillEffect(idx); else activateSkillEffect(idx);
}

void listAvailableSkills() {
    cout << "Available Skills:\n";
    for (int i=0;i<8;++i) {
        cout << "[" << (i+1) << "] " << selfDiscovery[i]
             << " - " << (skillsUnlocked[i] ? "Unlocked" : "Locked")
             << " - " << (skillsActive[i] ? "Active" : "Inactive") << "\n";
    }
}

// Deduct per-turn EP for active stance skills. If insufficient EP, deactivate and apply a small crash penalty.
void applyPerTurnSkillCosts() {
    for (int i=0;i<8;++i) {
        if (!skillsActive[i]) continue;
        int cost = skills[i].epPerTurn;
        if (cost <= 0) continue;
        if (statList[3].value >= cost) {
            statList[3].value -= cost;
            cout << "EP " << cost << " consumed for " << skills[i].name << ". Remaining EP: " << statList[3].value << "\n";
        } else {
            cout << "Not enough EP to sustain " << skills[i].name << ". Deactivating and applying crash penalty." << endl;
            deactivateSkillEffect(i);
            // soft crash penalty: reduce ATK and END moderately (do not go below 1.0)
            statList[1].value = max(1.0, statList[1].value / 1.4);
            statList[2].value = max(1.0, statList[2].value / 1.4);
        }
    }
}

// Tick down cooldowns each turn
void tickCooldowns() {
    for (int i=0;i<8;++i) if (skillCooldownRemaining[i] > 0) --skillCooldownRemaining[i];
}

// ---------- Helpers & Output formatting (kept to preserve existing messages) ----------
const string DIVIDER = "=============================================================================================================\n";

void printDivider() { cout << DIVIDER; }

void printUpdatedStatsBlock() {
    cout << "=============================================================================================================\n";
    cout << "Updated stats:" << endl;
    for (int i=0;i<5;++i) cout << statList[i].name << ": " << fixed << setprecision(1) << statList[i].value << " | ";
    cout << "\nSelf Discovery Unlocked: " << nDiscovery << "/8" << endl;
    cout << months << "months has passed" << "\n";
    cout << "Skills Unlocked Status: ";
    for (int i=0;i<8;++i) cout << (skillsUnlocked[i] ? "/" : "X") << " ";
    cout << "\n" << DIVIDER << "\n";
}

int readChoiceInRange(int min, int max) {
    string input; getline(cin, input);
    stringstream ss(input);
    int choice; if (!(ss >> choice) || choice < min || choice > max) return -1;
    char extra; if (ss >> extra) return -1; return choice;
}

void trainArc() {
    int leftTurns = 48;
    while (leftTurns > 0) {
        printDivider();
        cout << "You have " << leftTurns << " training turns left." << endl;
        cout << "Choose a stat to train:" << endl;

        for (int i=0;i<5;++i) {
            cout << "[" << (i+1) << "] " << statList[i].name
                 << " (Current: " << fixed << setprecision(1) << statList[i].value
                 << ", Upgrades: " << statList[i].upgrades << ")" << endl;
        }
        cout << "[6] Self Discovery (Unlocked: " << nDiscovery << ")" << endl;
        cout << "Enter your choice: ";

        int choice = readChoiceInRange(1,6);
        if (choice == -1) { cout << "Invalid input. Please enter a number between 1-6." << endl; continue; }

        if (choice >= 1 && choice <= 5) {
            int idx = choice - 1;
            statList[idx].value += statList[idx].baseGrowth + statList[idx].perUpgradeGrowth * (statList[idx].upgrades - 1);
            statList[idx].upgrades++;
        } else if (choice == 6) {
            if (nDiscovery < 8) {
                cout << "You unlocked: " << selfDiscovery[nDiscovery] << "!" << endl;
                skillsUnlocked[nDiscovery] = true;
                switch (nDiscovery) {
                    case 0: unlock_GateOfOpening(); break;
                    case 1: unlock_ChainHandling(); break;
                    case 2: unlock_KeiganBarrage(); break;
                    case 3: unlock_FirstLotus(); break;
                    case 4: unlock_GateOfMastery(); break;
                    case 5: unlock_GateOfHealing(); break;
                    case 6: unlock_ChainBarrage(); break;
                    case 7: unlock_ReverseLotus(); break;
                    default: break;
                }
                nDiscovery++;
            } else {
                cout << "All self-discovery skills already unlocked!" << endl;
                continue;
            }
        }

        leftTurns--;
        months += 2;

        printUpdatedStatsBlock();
    }
}

int main() {
    initSkills();
    trainArc();

    cout << "Training complete! Final stats:" << endl;
    for (int i=0;i<5;++i) cout << statList[i].name << ": " << fixed << setprecision(1) << statList[i].value << " | ";
    cout << "\nSelf Discovery unlocked: " << nDiscovery << "/8" << endl;
    cout << months << " Months has passed." << endl;
    cout << "Skills Unlocked Status: ";
    for (int i=0;i<8;++i) cout << (skillsUnlocked[i] ? "/" : "X") << " ";
    cout << endl;

    return 0;
}
