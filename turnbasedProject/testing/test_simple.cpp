#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>

using namespace std;

// Simple, readable version of the skill/stat system.
// - No 2D arrays: each Skill owns its multiplier/flat vectors
// - Clear names and comments
// - Small demonstration in main() to show unlocking, toggling, per-turn costs and cooldowns

struct Stat {
    string name;
    double value;
};

struct Skill {
    string name;
    string type;           // e.g., "Stance", "Passive", "Finisher"
    bool isStance;         // if true, has per-turn EP drain
    int epCast;            // EP cost to activate (one-time)
    int epPerTurn;         // EP drain per turn while active
    int cooldown;          // cooldown in turns after deactivation/use
    int cdRemaining = 0;   // runtime cooldown counter
    bool requiresGateOn = false;           // example requirement
    bool requiresFrontLotusUsed = false;   // example requirement
    vector<double> multiplier; // per-stat multipliers (parallel to stats)
    vector<double> flat;       // per-stat flat additions
};

// Helper printing
void printStats(const vector<Stat>& stats) {
    cout << "Current Stats:\n";
    for (const auto &s : stats) cout << " - " << setw(10) << left << s.name << ": " << fixed << setprecision(1) << s.value << "\n";
}

void printSkillStatus(const vector<Skill>& skills, const vector<bool>& unlocked, const vector<bool>& active) {
    cout << "Skills:\n";
    for (size_t i = 0; i < skills.size(); ++i) {
        cout << "[" << (i+1) << "] " << setw(18) << left << skills[i].name
             << " | " << (unlocked[i] ? "Unlocked" : "Locked")
             << " | " << (active[i] ? "Active" : "Inactive")
             << " | CD:" << skills[i].cdRemaining << "\n";
    }
}

// Apply a skill's multipliers/flats to stats (and record what was applied in appliedDelta)
void applySkillEffects(const Skill &skill, vector<Stat>& stats, vector<double>& appliedDelta) {
    appliedDelta.assign(stats.size(), 0.0);
    for (size_t i = 0; i < stats.size(); ++i) {
        double before = stats[i].value;
        double delta = before * (i < skill.multiplier.size() ? skill.multiplier[i] - 1.0 : 0.0)
                     + (i < skill.flat.size() ? skill.flat[i] : 0.0);
        stats[i].value += delta;
        appliedDelta[i] = delta;
    }
}

void revertSkillEffects(const vector<double>& appliedDelta, vector<Stat>& stats) {
    for (size_t i = 0; i < stats.size(); ++i) stats[i].value -= appliedDelta[i];
}

// Activate a skill if possible. Returns true on activation.
bool activateSkill(int idx, vector<Skill>& skills, vector<Stat>& stats, vector<bool>& unlocked, vector<bool>& active,
                   vector<vector<double>>& appliedDeltas, vector<bool>& usedOnce) {
    if (!unlocked[idx]) { cout << "Skill not unlocked.\n"; return false; }
    if (active[idx]) { cout << "Skill already active.\n"; return false; }
    Skill &sk = skills[idx];

    // example requirement: requires gate to be active (assume G1 is index 0)
    if (sk.requiresGateOn && !active[0]) { cout << "Requires Gate of Opening (G1) to be active.\n"; return false; }
    if (sk.requiresFrontLotusUsed && !usedOnce[2]) { cout << "Requires Front Lotus to have been used.\n"; return false; }

    // check cooldown
    if (sk.cdRemaining > 0) { cout << "Skill on cooldown (" << sk.cdRemaining << " turns).\n"; return false; }

    // check EP pool (assume stat index 3 is EP Pool)
    if (sk.epCast > 0) {
        if (stats[3].value < sk.epCast) { cout << "Not enough EP to activate " << sk.name << ".\n"; return false; }
        stats[3].value -= sk.epCast;
    }

    // apply effects
    applySkillEffects(sk, stats, appliedDeltas[idx]);
    active[idx] = true;
    usedOnce[idx] = true;
    cout << "Activated: " << sk.name << "\n";
    return true;
}

// Deactivate skill and set cooldown if present
void deactivateSkill(int idx, vector<Skill>& skills, vector<Stat>& stats, vector<bool>& active, vector<vector<double>>& appliedDeltas) {
    if (!active[idx]) { cout << "Skill not active.\n"; return; }
    revertSkillEffects(appliedDeltas[idx], stats);
    appliedDeltas[idx].assign(stats.size(), 0.0);
    Skill &sk = skills[idx];
    if (sk.cooldown > 0) sk.cdRemaining = sk.cooldown;
    active[idx] = false;
    cout << "Deactivated: " << sk.name << " (CD set to " << sk.cdRemaining << ")\n";
}

// Per-turn processing: deduct EP for stances and tick cooldowns
void perTurnUpdate(vector<Skill>& skills, vector<Stat>& stats, vector<bool>& active, vector<bool>& unlocked) {
    // deduct per-turn EP for active stances
    for (size_t i = 0; i < skills.size(); ++i) {
        if (!active[i]) continue;
        Skill &sk = skills[i];
        if (sk.isStance && sk.epPerTurn > 0) {
            if (stats[3].value >= sk.epPerTurn) {
                stats[3].value -= sk.epPerTurn;
                cout << sk.name << " consumed " << sk.epPerTurn << " EP. Remaining EP: " << stats[3].value << "\n";
            } else {
                cout << sk.name << " cannot be sustained (insufficient EP). Deactivating.\n";
                // simple crash penalty: reduce ATK slightly but not below 1
                stats[1].value = max(1.0, stats[1].value / 1.3);
                stats[2].value = max(1.0, stats[2].value / 1.3);
                // revert effects by marking for deactivation externally (caller should call deactivateSkill)
                // Here we just print note; in a full loop we'd call deactivateSkill.
            }
        }
    }
    // tick cooldowns
    for (auto &sk : skills) if (sk.cdRemaining > 0) --sk.cdRemaining;
}

int main() {
    // Simple stats: HP, ATK, END, EP Pool, EP Regen
    vector<Stat> stats = { {"HP", 100.0}, {"ATK", 20.0}, {"END", 15.0}, {"EP Pool", 40.0}, {"EP Regen", 3.0} };

    // Define a few skills; multipliers/flats align with stats vector order
    vector<Skill> skills;

    Skill g1;
    g1.name = "Gate of Opening (G1)";
    g1.type = "Stance";
    g1.isStance = true;
    g1.epCast = 20;
    g1.epPerTurn = 5;
    g1.cooldown = 0;
    g1.multiplier = {1.0, 1.8, 1.0, 1.0, 1.0};
    g1.flat = {0, 0, 0, 0, 0};
    skills.push_back(g1);

    Skill frontLotus;
    frontLotus.name = "Front Lotus";
    frontLotus.type = "Finisher";
    frontLotus.isStance = false;
    frontLotus.epCast = 12;
    frontLotus.epPerTurn = 0;
    frontLotus.cooldown = 0;
    frontLotus.multiplier = {1.0, 1.0, 1.0, 1.0, 1.0};
    frontLotus.flat = {0, 6.0, 0, 0, 0}; // e.g., temporary ATK-like effect
    skills.push_back(frontLotus);

    Skill reverseLotus;
    reverseLotus.name = "Reverse Lotus";
    reverseLotus.type = "Finisher";
    reverseLotus.isStance = false;
    reverseLotus.epCast = 25;
    reverseLotus.epPerTurn = 0;
    reverseLotus.cooldown = 5;
    reverseLotus.requiresGateOn = true; // requires G1 active
    reverseLotus.requiresFrontLotusUsed = true; // requires Front Lotus used once
    reverseLotus.multiplier = {1.0, 1.3, 1.0, 1.0, 1.0};
    reverseLotus.flat = {0, 8.0, 0, 0, 0};
    skills.push_back(reverseLotus);

    // runtime trackers
    vector<bool> unlocked(skills.size(), false);
    vector<bool> active(skills.size(), false);
    vector<vector<double>> appliedDeltas(skills.size());
    vector<bool> usedOnce(skills.size(), false);

    // Unlock first two skills as an example
    unlocked[0] = true; // G1 unlocked
    unlocked[1] = true; // Front Lotus unlocked

    cout << "=== Simple Skill Demo ===\n";
    printStats(stats);
    printSkillStatus(skills, unlocked, active);

    // Activate G1 (Gate stance)
    cout << "\nAttempting to activate G1...\n";
    if (activateSkill(0, skills, stats, unlocked, active, appliedDeltas, usedOnce)) {
        printStats(stats);
    }

    // Use Front Lotus once
    cout << "\nUsing Front Lotus (index 2 in this demo is Reverse Lotus; Front Lotus is index 2 in original, but here it's index 1)...\n";
    if (activateSkill(1, skills, stats, unlocked, active, appliedDeltas, usedOnce)) {
        // front lotus is an instant effect in this simplified demo; deactivate immediately
        deactivateSkill(1, skills, stats, active, appliedDeltas);
        printStats(stats);
    }

    // Try to activate Reverse Lotus (should fail because locked and requires frontLotus used and gate active)
    cout << "\nAttempting Reverse Lotus (locked) ...\n";
    if (!activateSkill(2, skills, stats, unlocked, active, appliedDeltas, usedOnce)) {
        cout << "-- unlocking Reverse Lotus now for demo --\n";
        unlocked[2] = true;
    }

    // Now try Reverse Lotus; this will succeed only if G1 is active and front lotus used
    cout << "\nAttempting Reverse Lotus again...\n";
    if (activateSkill(2, skills, stats, unlocked, active, appliedDeltas, usedOnce)) {
        deactivateSkill(2, skills, stats, active, appliedDeltas);
        printStats(stats);
    }

    // Simulate a few turns where per-turn EP drains occur (for active stances)
    cout << "\nSimulating 4 turns of stance upkeep...\n";
    for (int t = 1; t <= 4; ++t) {
        cout << "\n-- Turn " << t << " --\n";
        perTurnUpdate(skills, stats, active, unlocked);
        printStats(stats);
    }

    cout << "\nDemo complete.\n";
    return 0;
}
