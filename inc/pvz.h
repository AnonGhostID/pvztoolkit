
#pragma once

#include <Windows.h>

#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "code.h"
#include "data.h"
#include "lineup.h"
#include "process.h"

namespace Pt
{

typedef void (*cb_func)(void *, int);

class PvZ : public Process, public Code, public Data
{
  public:
    PvZ();
    ~PvZ();

    // Safely inject
    void asm_code_inject();

    // Apply hack
    template <typename T, size_t size>
    void enable_hack(HACK<T, size>, bool);

    // Apply hacks
    template <typename T, size_t size>
    void enable_hack(std::vector<HACK<T, size>>, bool);

#ifdef _DEBUG
    template <typename T, size_t size>
    void check_hack(HACK<T, size>);

    template <typename T, size_t size>
    void check_hack(std::vector<HACK<T, size>>);

    void check_all_hacks();
#endif

    // Set callback function for game search
    void callback(cb_func, void *);

    // Find Plants vs Zombies, return true if supported version found
    bool FindPvZ();

    // Check if game is running normally
    // Must check before each modification
    bool GameOn();

    // Game path
    std::string GamePath();

    // Game mode
    int GameMode();

    // Game UI
    int GameUI();

    // Game scene
    int GetScene();
    void SetScene(int, bool);

    // Scene row count
    int GetRowCount();

  protected:
    // Callback function pointer and window pointer
    cb_func cb_find_result;
    void *window;

  public:
    // The following are modification features

    // Unlock game
    void UnlockTrophy();

    // Direct win
    void DirectWin(bool);

    // Remove limit
    void UnlockSunLimit(bool);

    // Modify sun
    void SetSun(int);

    // Modify money
    void SetMoney(int);

    // Auto collect
    void AutoCollected(bool);

    // No loot drop
    void NotDropLoot(bool);

    // Unlimited fertilizer
    void FertilizerUnlimited(bool);

    // Unlimited bug spray
    void BugSprayUnlimited(bool);

    // Unlimited chocolate
    void ChocolateUnlimited(bool);

    // Unlimited tree food
    void TreeFoodUnlimited(bool);

    // Wisdom tree height
    void SetTreeHeight(int);

    // Free planting
    void FreePlanting(bool);

    // Place anywhere
    void PlacedAnywhere(bool);

    // Fast belt
    void FastBelt(bool);

    // Lock shovel
    void LockShovel(bool);

    // Mix mode
    void MixMode(int, int);

    // Endless rounds
    void EndlessRounds(int);

    // Generate plant
    void asm_put_plant(int, int, int, bool, bool);
    void PutPlant(int, int, int, bool);

    // Generate zombie
    void asm_put_zombie(int, int, int);
    void PutZombie(int, int, int);

    // Generate grave
    void asm_put_grave(int, int);
    void PutGrave(int, int);

    // Generate ladder and auto ladder
    void asm_put_ladder(int, int);
    void PutLadder(int, int);
    void AutoLadder(bool);

    // Generate rake
    void asm_put_rake(int, int);
    void PutRake(int, int);

    // Start/delete/restore lawn mowers
    void SetLawnMowers(int);

    // Clear all plants
    void ClearAllPlants();

    // Kill all zombies
    void KillAllZombies();

    // Clear grid items
    void ClearGridItems(std::vector<int>);

    // Plant invincible
    void PlantInvincible(bool);

    // Plant weak
    void PlantWeak(bool);

    // Zombie invincible
    void ZombieInvincible(bool);

    // Zombie weak
    void ZombieWeak(bool);

    // Reload instantly
    void ReloadInstantly(bool);

    // Mushrooms awake
    void MushroomsAwake(bool);

    // Stop spawning
    void StopSpawning(bool);

    // Stop zombies
    void StopZombies(bool);

    // Lock butter
    void LockButter(bool);

    // No crater
    void NoCrater(bool);

    // No ice trail
    void NoIceTrail(bool);

    // Zombie (pepper) not explode
    void ZombieNotExplode(bool);

    // Get seed in slot, imitater adds 48
    int GetSlotSeed(int);

    // Modify seed in slot
    void SetSlotSeed(int, int, bool);

    // Plant lily pad on pool
    void LilyPadOnPool(int, int);

    // Place flower pot on roof
    void FlowerPotOnRoof(int, int);

    // Window screenshot
    void Screenshot();

    // Get lineup
    Lineup GetLineup();

    // Set lineup
    void SetLineup(Lineup);

    // Generate spawn list based on zombie types
    void generate_spawn_list();

    // Update spawn preview in seed selection interface
    void update_spawn_preview();

    // Get spawn list
    std::array<int, 1000> GetSpawnList();

    // Modify spawn list
    void SetSpawnList(std::array<int, 1000>);

    // Internal function to generate spawn list
    void InternalSpawn(std::array<bool, 33>);

    // Custom fill spawn list
    void CustomizeSpawn(std::array<bool, 33>, bool, bool, int);

    // Modify background music
    void SetMusic(int);

    // Clear fog
    void NoFog(bool);

    // See vase
    void SeeVase(bool);

    // Background running
    void BackgroundRunning(bool);

    // Userdata readonly
    void UserdataReadonly(bool);

    // Built-in debug mode
    void DebugMode(int);

    // Frame duration
    int GetFrameDuration();
    void SetFrameDuration(int);

    // Show hidden levels
    void UnlockLimboPage(bool);

  private:
    std::chrono::steady_clock::time_point last_find_attempt = std::chrono::steady_clock::time_point::min();
};

template <typename T, size_t size>
void PvZ::enable_hack(HACK<T, size> hack, bool on)
{
    if (hack.mem_addr == 0x00000000 || hack.mem_addr == 0xffffffff)
        return;

    if (on)
        WriteMemory(std::array<T, size>(hack.hack_value), {hack.mem_addr});
    else
        WriteMemory(std::array<T, size>(hack.reset_value), {hack.mem_addr});
}

template <typename T, size_t size>
void PvZ::enable_hack(std::vector<HACK<T, size>> hacks, bool on)
{
    for (size_t i = 0; i < hacks.size(); i++)
    {
        if (hacks[i].mem_addr == 0x00000000 || hacks[i].mem_addr == 0xffffffff)
            continue;

        if (on)
            WriteMemory(std::array<T, size>(hacks[i].hack_value), {hacks[i].mem_addr});
        else
            WriteMemory(std::array<T, size>(hacks[i].reset_value), {hacks[i].mem_addr});
    }
}

#ifdef _DEBUG

template <typename T, size_t size>
void PvZ::check_hack(HACK<T, size> hack)
{
    bool ok = true;

    if (hack.mem_addr == 0x00000000 || hack.mem_addr == 0xffffffff)
        return;

    auto read_value = ReadMemory<T, size>({hack.mem_addr});
    if (read_value != hack.reset_value)
        ok = false;

    if (!ok)
        std::cout << "Hack Error: " << hack.mem_addr << std::endl;
}

template <typename T, size_t size>
void PvZ::check_hack(std::vector<HACK<T, size>> hacks)
{
    bool ok = true;

    for (size_t i = 0; i < hacks.size(); i++)
    {
        if (hacks[i].mem_addr == 0x00000000 || hacks[i].mem_addr == 0xffffffff)
            continue;

        auto read_value = ReadMemory<T, size>({hacks[i].mem_addr});
        if (read_value != hacks[i].reset_value)
            ok = false;

        if (!ok)
            std::cout << "Hack Error: " << hacks[i].mem_addr << std::endl;
    }
}

#endif

} // namespace Pt
