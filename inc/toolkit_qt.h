#pragma once

#include <Windows.h>
#include <filesystem>
#include <vector>
#include <array>
#include <string>

#include "lineup.h"
#include "pak.h"
#include "pvz.h"
#include "window_qt.h"

namespace Pt
{

class QtToolkit : public QtWindow
{
    Q_OBJECT

public:
    QtToolkit(QWidget *parent = nullptr);
    ~QtToolkit();

    std::filesystem::path current_path;
    
private:
    void connectSignals();

    void close_all_sub_window();

public:
    PvZ *pvz;
    PAK *pak;

    int unpack_result = 0xFFFFFFFF;
    std::string unpack_text;

    int pack_result = 0xFFFFFFFF;
    std::string pack_text;

public slots:
    void cb_show_details();
    void cb_update_details();
    void cb_zombies_list();
    void cb_on_hide_spawn_details();

    void cb_unlock_sun_limit();
    void cb_set_sun();
    void cb_set_money();
    void cb_auto_collected();
    void cb_not_drop_loot();
    void cb_fertilizer();
    void cb_bug_spray();
    void cb_tree_food();
    void cb_chocolate();
    void cb_wisdom_tree();
    void cb_free_planting();
    void cb_placed_anywhere();
    void cb_fast_belt();
    void cb_lock_shovel();
    void cb_mix_mode();
    void cb_endless_rounds();
    void cb_unlock();
    void cb_direct_win();
    void cb_direct_win_thread();
    void cb_put_plant();
    void cb_put_zombie();
    void cb_put_ladder();
    void cb_put_grave();
    void cb_put_rake();
    void cb_lawn_mower_start();
    void cb_lawn_mower_delete();
    void cb_lawn_mower_restore();
    void cb_clear();
    void cb_plant_invincible();
    void cb_plant_weak();
    void cb_zombie_invincible();
    void cb_zombie_weak();
    void cb_reload_instantly();
    void cb_mushroom_awake();
    void cb_stop_spawning();
    void cb_stop_zombies();
    void cb_lock_butter();
    void cb_no_crater();
    void cb_no_ice_trail();
    void cb_zombie_not_explode();
    void cb_get_seed();
    void cb_set_seed();
    void cb_lineup_mode();
    void cb_auto_ladder();
    void cb_put_lily_pad();
    void cb_put_flower_pot();
    void cb_reset_scene();
    void cb_get_lineup();
    void cb_set_lineup();
    void cb_capture();
    void cb_set_spawn();
    void cb_music();
    void cb_userdata();
    void cb_no_fog();
    void cb_see_vase();
    void cb_background();
    void cb_readonly();
    void cb_unpack();
    void cb_unpack_thread();
    void cb_pack();
    void cb_pack_thread();
    void cb_debug_mode();
    void cb_speed();
    void cb_limbo_page();
    
protected:
    void cb_find_result(int) override;
};

} // namespace Pt
