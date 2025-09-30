#include "../inc/toolkit_qt.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QUrl>

#include <fstream>
#include <sstream>

namespace Pt
{

QtToolkit::QtToolkit(QWidget *parent)
    : QtWindow(parent)
{
    current_path = std::filesystem::current_path();
    
    // Initialize PvZ and PAK
    pvz = new PvZ();
    pak = new PAK();
    
    // Connect all signals to slots
    connectSignals();
}

QtToolkit::~QtToolkit()
{
    delete pvz;
    delete pak;
    if (window_spawn)
    {
        window_spawn->close();
    }
}

void QtToolkit::connectSignals()
{
    // Resource Tab
    connect(check_unlock_sun_limit, &QRadioButton::toggled, this, &QtToolkit::cb_unlock_sun_limit);
    connect(button_sun, &QPushButton::clicked, this, &QtToolkit::cb_set_sun);
    connect(button_money, &QPushButton::clicked, this, &QtToolkit::cb_set_money);
    connect(check_auto_collected, &QCheckBox::toggled, this, &QtToolkit::cb_auto_collected);
    connect(check_not_drop_loot, &QCheckBox::toggled, this, &QtToolkit::cb_not_drop_loot);
    connect(check_fertilizer, &QCheckBox::toggled, this, &QtToolkit::cb_fertilizer);
    connect(check_bug_spray, &QCheckBox::toggled, this, &QtToolkit::cb_bug_spray);
    connect(check_tree_food, &QCheckBox::toggled, this, &QtToolkit::cb_tree_food);
    connect(check_chocolate, &QCheckBox::toggled, this, &QtToolkit::cb_chocolate);
    connect(button_wisdom_tree, &QPushButton::clicked, this, &QtToolkit::cb_wisdom_tree);
    connect(check_free_planting, &QCheckBox::toggled, this, &QtToolkit::cb_free_planting);
    connect(check_placed_anywhere, &QCheckBox::toggled, this, &QtToolkit::cb_placed_anywhere);
    connect(check_fast_belt, &QCheckBox::toggled, this, &QtToolkit::cb_fast_belt);
    connect(check_lock_shovel, &QCheckBox::toggled, this, &QtToolkit::cb_lock_shovel);
    connect(button_mix, &QPushButton::clicked, this, &QtToolkit::cb_mix_mode);
    connect(button_level, &QPushButton::clicked, this, &QtToolkit::cb_endless_rounds);
    connect(button_unlock, &QPushButton::clicked, this, &QtToolkit::cb_unlock);
    connect(button_direct_win, &QPushButton::clicked, this, &QtToolkit::cb_direct_win);
    
    // Battle Tab
    connect(button_put_plant, &QPushButton::clicked, this, &QtToolkit::cb_put_plant);
    connect(button_put_zombie, &QPushButton::clicked, this, &QtToolkit::cb_put_zombie);
    connect(button_put_ladder, &QPushButton::clicked, this, &QtToolkit::cb_put_ladder);
    connect(button_put_grave, &QPushButton::clicked, this, &QtToolkit::cb_put_grave);
    connect(button_put_rake, &QPushButton::clicked, this, &QtToolkit::cb_put_rake);
    connect(button_lawn_mower_start, &QPushButton::clicked, this, &QtToolkit::cb_lawn_mower_start);
    connect(button_lawn_mower_delete, &QPushButton::clicked, this, &QtToolkit::cb_lawn_mower_delete);
    connect(button_lawn_mower_restore, &QPushButton::clicked, this, &QtToolkit::cb_lawn_mower_restore);
    connect(button_clear, &QPushButton::clicked, this, &QtToolkit::cb_clear);
    connect(check_plant_invincible, &QCheckBox::toggled, this, &QtToolkit::cb_plant_invincible);
    connect(check_plant_weak, &QCheckBox::toggled, this, &QtToolkit::cb_plant_weak);
    connect(check_zombie_invincible, &QCheckBox::toggled, this, &QtToolkit::cb_zombie_invincible);
    connect(check_zombie_weak, &QCheckBox::toggled, this, &QtToolkit::cb_zombie_weak);
    connect(check_reload_instantly, &QCheckBox::toggled, this, &QtToolkit::cb_reload_instantly);
    connect(check_mushroom_awake, &QCheckBox::toggled, this, &QtToolkit::cb_mushroom_awake);
    connect(check_stop_spawning, &QCheckBox::toggled, this, &QtToolkit::cb_stop_spawning);
    connect(check_stop_zombies, &QCheckBox::toggled, this, &QtToolkit::cb_stop_zombies);
    connect(check_lock_butter, &QCheckBox::toggled, this, &QtToolkit::cb_lock_butter);
    connect(check_no_crater, &QCheckBox::toggled, this, &QtToolkit::cb_no_crater);
    connect(check_no_ice_trail, &QCheckBox::toggled, this, &QtToolkit::cb_no_ice_trail);
    connect(check_zombie_not_explode, &QCheckBox::toggled, this, &QtToolkit::cb_zombie_not_explode);
    
    // Lineup Tab
    connect(button_seed, &QPushButton::clicked, this, &QtToolkit::cb_set_seed);
    connect(choice_slot, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtToolkit::cb_get_seed);
    connect(check_lineup_mode, &QCheckBox::toggled, this, &QtToolkit::cb_lineup_mode);
    connect(button_auto_ladder, &QPushButton::clicked, this, &QtToolkit::cb_auto_ladder);
    connect(button_put_lily_pad, &QPushButton::clicked, this, &QtToolkit::cb_put_lily_pad);
    connect(button_put_flower_pot, &QPushButton::clicked, this, &QtToolkit::cb_put_flower_pot);
    connect(button_reset, &QPushButton::clicked, this, &QtToolkit::cb_reset_scene);
    connect(button_get_lineup, &QPushButton::clicked, this, &QtToolkit::cb_get_lineup);
    connect(button_set_lineup, &QPushButton::clicked, this, &QtToolkit::cb_set_lineup);
    connect(button_capture, &QPushButton::clicked, this, &QtToolkit::cb_capture);
    connect(button_copy_lineup, &QPushButton::clicked, this, &QtToolkit::cb_copy_lineup);
    connect(button_paste_lineup, &QPushButton::clicked, this, &QtToolkit::cb_paste_lineup);
    
    // Spawn Tab
    connect(button_show_details, &QPushButton::clicked, this, &QtToolkit::cb_show_details);
    connect(button_set_spawn, &QPushButton::clicked, this, &QtToolkit::cb_set_spawn);
    
    // Connect lineup tab
    connect(button_load_lineup, &QPushButton::clicked, this, &QtToolkit::cb_load_lineup);
    connect(choice_scene, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtToolkit::cb_switch_lineup_scene);
    connect(choice_mode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtToolkit::cb_mode);
    
    // Others Tab
    connect(button_music, &QPushButton::clicked, this, &QtToolkit::cb_music);
    connect(button_userdata, &QPushButton::clicked, this, &QtToolkit::cb_userdata);
    connect(check_no_fog, &QCheckBox::toggled, this, &QtToolkit::cb_no_fog);
    connect(check_see_vase, &QCheckBox::toggled, this, &QtToolkit::cb_see_vase);
    connect(check_background, &QCheckBox::toggled, this, &QtToolkit::cb_background);
    connect(check_readonly, &QCheckBox::toggled, this, &QtToolkit::cb_readonly);
    connect(button_file, &QPushButton::clicked, this, &QtToolkit::cb_open_file);
    connect(button_unpack, &QPushButton::clicked, this, &QtToolkit::cb_unpack);
    connect(button_dir, &QPushButton::clicked, this, &QtToolkit::cb_open_dir);
    connect(button_pack, &QPushButton::clicked, this, &QtToolkit::cb_pack);
    connect(button_debug, &QPushButton::clicked, this, &QtToolkit::cb_debug_mode);
    connect(button_speed, &QPushButton::clicked, this, &QtToolkit::cb_speed);
    connect(check_limbo_page, &QCheckBox::toggled, this, &QtToolkit::cb_limbo_page);
    connect(choice_scheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtToolkit::cb_scheme);
    connect(button_document, &QPushButton::clicked, this, &QtToolkit::cb_document);
    connect(button_about, &QPushButton::clicked, this, &QtToolkit::cb_about);
}

void QtToolkit::close_all_sub_window()
{
    if (window_spawn && window_spawn->isVisible())
    {
        window_spawn->close();
    }
}

void QtToolkit::cb_find_result(int result)
{
    this->result = result;
    
    QString statusText;
    QString statusStyle;
    
    switch (result)
    {
    case PVZ_NOT_FOUND:
        statusText = "❌ Game not found";
        statusStyle = "background-color: #3d3d3d; color: #ff6b6b; padding: 8px; border-radius: 4px; font-weight: bold;";
        break;
    case PVZ_OPEN_ERROR:
        statusText = "⚠️ Failed to open game process";
        statusStyle = "background-color: #3d3d3d; color: #ff9f1c; padding: 8px; border-radius: 4px; font-weight: bold;";
        break;
    case PVZ_UNSUPPORTED:
        statusText = "⚠️ Unsupported game version";
        statusStyle = "background-color: #3d3d3d; color: #ff9f1c; padding: 8px; border-radius: 4px; font-weight: bold;";
        if (!bad_version_warned)
        {
            QMessageBox::warning(this, "Unsupported Version",
                "This version of PvZ is not supported.\n"
                "Please use an official version.");
            bad_version_warned = true;
        }
        break;
    default:
        statusText = "✅ Game connected";
        statusStyle = "background-color: #0d7377; color: white; padding: 8px; border-radius: 4px; font-weight: bold;";
        break;
    }
    
    game_status->setText(statusText);
    game_status->setStyleSheet(statusStyle);
}

// Resource Tab Callbacks

void QtToolkit::cb_unlock_sun_limit()
{
    pvz->UnlockSunLimit(check_unlock_sun_limit->isChecked());
}

void QtToolkit::cb_set_sun()
{
    pvz->SetSun(input_sun->value());
}

void QtToolkit::cb_set_money()
{
    pvz->SetMoney(input_money->value());
}

void QtToolkit::cb_auto_collected()
{
    pvz->AutoCollected(check_auto_collected->isChecked());
}

void QtToolkit::cb_not_drop_loot()
{
    pvz->NotDropLoot(check_not_drop_loot->isChecked());
}

void QtToolkit::cb_fertilizer()
{
    pvz->FertilizerUnlimited(check_fertilizer->isChecked());
}

void QtToolkit::cb_bug_spray()
{
    pvz->BugSprayUnlimited(check_bug_spray->isChecked());
}

void QtToolkit::cb_tree_food()
{
    pvz->TreeFoodUnlimited(check_tree_food->isChecked());
}

void QtToolkit::cb_chocolate()
{
    pvz->ChocolateUnlimited(check_chocolate->isChecked());
}

void QtToolkit::cb_wisdom_tree()
{
    pvz->SetTreeHeight(input_wisdom_tree->value());
}

void QtToolkit::cb_free_planting()
{
    pvz->FreePlanting(check_free_planting->isChecked());
}

void QtToolkit::cb_placed_anywhere()
{
    pvz->PlacedAnywhere(check_placed_anywhere->isChecked());
}

void QtToolkit::cb_fast_belt()
{
    pvz->FastBelt(check_fast_belt->isChecked());
}

void QtToolkit::cb_lock_shovel()
{
    pvz->LockShovel(check_lock_shovel->isChecked());
}

void QtToolkit::cb_mix_mode()
{
    int mode = choice_mode->currentIndex();
    int level = choice_adventure->currentIndex();
    pvz->MixMode(mode, level);
}

void QtToolkit::cb_endless_rounds()
{
    pvz->EndlessRounds(input_level->value());
}

void QtToolkit::cb_unlock()
{
    pvz->UnlockTrophy();
    QMessageBox::information(this, "Success", "All achievements unlocked!");
}

void QtToolkit::cb_direct_win()
{
    bool brightest = check_brightest_cob_cannon->isChecked();
    pvz->DirectWin(brightest);
}

void QtToolkit::cb_direct_win_thread()
{
    // Thread implementation if needed
}

// Battle Tab Callbacks

void QtToolkit::cb_put_plant()
{
    int row = choice_row->currentIndex();
    int col = choice_col->currentIndex();
    int plant_type = choice_plant->currentIndex();
    bool imitater = check_imitater->isChecked();
    
    pvz->PutPlant(row, col, plant_type, imitater);
}

void QtToolkit::cb_put_zombie()
{
    int row = choice_row->currentIndex();
    int col = choice_col->currentIndex();
    int zombie_type = choice_zombie->currentIndex();
    
    pvz->PutZombie(row, col, zombie_type);
}

void QtToolkit::cb_put_ladder()
{
    int row = choice_row->currentIndex();
    int col = choice_col->currentIndex();
    
    pvz->PutLadder(row, col);
}

void QtToolkit::cb_put_grave()
{
    int row = choice_row->currentIndex();
    int col = choice_col->currentIndex();
    
    pvz->PutGrave(row, col);
}

void QtToolkit::cb_put_rake()
{
    int row = choice_row->currentIndex();
    int col = choice_col->currentIndex();
    
    pvz->PutRake(row, col);
}

void QtToolkit::cb_lawn_mower_start()
{
    pvz->SetLawnMowers(0); // Start all
}

void QtToolkit::cb_lawn_mower_delete()
{
    pvz->SetLawnMowers(1); // Delete all
}

void QtToolkit::cb_lawn_mower_restore()
{
    pvz->SetLawnMowers(2); // Restore all
}

void QtToolkit::cb_clear()
{
    int item_type = choice_item->currentIndex();
    std::vector<int> items;
    
    switch (item_type)
    {
    case 0: // All plants
        pvz->ClearAllPlants();
        break;
    case 1: // All zombies
        pvz->KillAllZombies();
        break;
    case 2: // Craters
        items = {1};
        pvz->ClearGridItems(items);
        break;
    case 3: // Ladders
        items = {3};
        pvz->ClearGridItems(items);
        break;
    case 4: // Graves
        items = {2};
        pvz->ClearGridItems(items);
        break;
    }
}

void QtToolkit::cb_plant_invincible()
{
    pvz->PlantInvincible(check_plant_invincible->isChecked());
}

void QtToolkit::cb_plant_weak()
{
    pvz->PlantWeak(check_plant_weak->isChecked());
}

void QtToolkit::cb_zombie_invincible()
{
    pvz->ZombieInvincible(check_zombie_invincible->isChecked());
}

void QtToolkit::cb_zombie_weak()
{
    pvz->ZombieWeak(check_zombie_weak->isChecked());
}

void QtToolkit::cb_reload_instantly()
{
    pvz->ReloadInstantly(check_reload_instantly->isChecked());
}

void QtToolkit::cb_mushroom_awake()
{
    pvz->MushroomsAwake(check_mushroom_awake->isChecked());
}

void QtToolkit::cb_stop_spawning()
{
    pvz->StopSpawning(check_stop_spawning->isChecked());
}

void QtToolkit::cb_stop_zombies()
{
    pvz->StopZombies(check_stop_zombies->isChecked());
}

void QtToolkit::cb_lock_butter()
{
    pvz->LockButter(check_lock_butter->isChecked());
}

void QtToolkit::cb_no_crater()
{
    pvz->NoCrater(check_no_crater->isChecked());
}

void QtToolkit::cb_no_ice_trail()
{
    pvz->NoIceTrail(check_no_ice_trail->isChecked());
}

void QtToolkit::cb_zombie_not_explode()
{
    pvz->ZombieNotExplode(check_zombie_not_explode->isChecked());
}

// Lineup Tab Callbacks

void QtToolkit::cb_get_seed()
{
    int slot = choice_slot->currentIndex();
    int seed = pvz->GetSlotSeed(slot);
    
    if (seed >= 0 && seed < 48)
    {
        choice_seed->setCurrentIndex(seed % 48);
        check_seed_imitater->setChecked(seed >= 48);
    }
}

void QtToolkit::cb_set_seed()
{
    int slot = choice_slot->currentIndex();
    int seed = choice_seed->currentIndex();
    bool imitater = check_seed_imitater->isChecked();
    
    pvz->SetSlotSeed(slot, seed, imitater);
}

void QtToolkit::cb_lineup_mode()
{
    bool enabled = check_lineup_mode->isChecked();
    // Implementation for lineup mode
}

void QtToolkit::cb_auto_ladder()
{
    bool enabled = !pvz->GameOn();
    if (enabled)
    {
        QMessageBox::warning(this, "Not Available", "Please start the game first.");
        return;
    }
    
    pvz->AutoLadder(true);
    QMessageBox::information(this, "Success", "Auto ladder applied!");
}

void QtToolkit::cb_put_lily_pad()
{
    if (!pvz->GameOn())
    {
        QMessageBox::warning(this, "Error", "Please start the game first!");
        return;
    }
    
    int scene = choice_scene->currentIndex();
    if (scene != 2 && scene != 3)
    {
        QMessageBox::warning(this, "Error", "Lily pads can only be placed in Pool or Fog scenes!");
        return;
    }
    
    // Place lily pads in pool rows
    for (int row = 2; row < 4; row++)
    {
        for (int col = lily_pad_col_lower[row]; col <= lily_pad_col_upper[row]; col++)
        {
            pvz->PutPlant(row, col, 16, false); // 16 is lily pad
        }
    }
    
    QMessageBox::information(this, "Success", "Lily pads placed!");
}

void QtToolkit::cb_put_flower_pot()
{
    if (!pvz->GameOn())
    {
        QMessageBox::warning(this, "Error", "Please start the game first!");
        return;
    }
    
    int scene = choice_scene->currentIndex();
    if (scene != 4)
    {
        QMessageBox::warning(this, "Error", "Flower pots can only be placed in Roof scene!");
        return;
    }
    
    // Place flower pots on roof
    for (int row = 0; row < 5; row++)
    {
        for (int col = flower_pot_col_lower[row]; col <= flower_pot_col_upper[row]; col++)
        {
            pvz->PutPlant(row, col, 33, false); // 33 is flower pot
        }
    }
    
    QMessageBox::information(this, "Success", "Flower pots placed!");
}

void QtToolkit::cb_reset_scene()
{
    int scene = choice_scene->currentIndex();
    pvz->SetScene(scene, false);
}

void QtToolkit::cb_get_lineup()
{
    Lineup lineup = pvz->GetLineup();
    QString code = QString::fromStdString(lineup.lineup_code);
    editor_lineup_string->setPlainText(code);
}

void QtToolkit::cb_set_lineup()
{
    QString code = editor_lineup_string->toPlainText();
    Lineup lineup(code.toStdString());
    if (lineup.OK())
    {
        pvz->SetLineup(lineup);
        QMessageBox::information(this, "Success", "Lineup applied!");
    }
    else
    {
        QMessageBox::warning(this, "Error", "Invalid lineup code!");
    }
}

void QtToolkit::cb_capture()
{
    pvz->Screenshot();
    QMessageBox::information(this, "Success", "Screenshot saved!");
}

// Spawn Tab Callbacks

void QtToolkit::cb_show_details()
{
    if (!window_spawn)
    {
        window_spawn = new QtSpawnWindow(this);
        connect(window_spawn, &QDialog::finished, this, &QtToolkit::cb_on_hide_spawn_details);
        connect(window_spawn->button_update_details, &QPushButton::clicked, this, &QtToolkit::cb_update_details);
    }
    
    if (window_spawn->isVisible())
    {
        window_spawn->close();
    }
    else
    {
        cb_update_details();
        window_spawn->show();
        button_show_details->setText("📉 Hide Details");
    }
}

void QtToolkit::cb_update_details()
{
    if (!window_spawn) return;
    
    std::array<int, 1000> spawn_list = pvz->GetSpawnList();
    window_spawn->UpdateData(spawn_list);
}

void QtToolkit::cb_zombies_list()
{
    // Implementation for zombies list menu
}

void QtToolkit::cb_on_hide_spawn_details()
{
    button_show_details->setText("📊 Show Details");
}

void QtToolkit::cb_set_spawn()
{
    std::array<bool, 33> selected;
    selected.fill(false);
    
    for (int i = 0; i < 20; i++)
    {
        selected[i] = check_zombie[i]->isChecked();
    }
    
    int giga_weight = choice_giga_weight->currentIndex();
    bool giga_limit = check_giga_limit->isChecked();
    
    pvz->CustomizeSpawn(selected, giga_limit, true, giga_weight);
    QMessageBox::information(this, "Success", "Spawn list applied!");
}

// Others Tab Callbacks

void QtToolkit::cb_music()
{
    int music_id = choice_music->currentIndex();
    pvz->SetMusic(music_id);
}

void QtToolkit::cb_userdata()
{
    QString userdata_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDesktopServices::openUrl(QUrl::fromLocalFile(userdata_path));
}

void QtToolkit::cb_no_fog()
{
    pvz->NoFog(check_no_fog->isChecked());
}

void QtToolkit::cb_see_vase()
{
    pvz->SeeVase(check_see_vase->isChecked());
}

void QtToolkit::cb_background()
{
    pvz->BackgroundRunning(check_background->isChecked());
}

void QtToolkit::cb_readonly()
{
    pvz->UserdataReadonly(check_readonly->isChecked());
}

void QtToolkit::cb_unpack()
{
    QString file = input_file->text();
    QString dir = input_dir->text();
    
    if (file.isEmpty() || dir.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please select both file and directory!");
        return;
    }
    
    unpack_result = pak->Unpack(file.toStdString(), dir.toStdString());
    
    if (unpack_result == 0)
    {
        QMessageBox::information(this, "Success", "Unpacked successfully!");
    }
    else
    {
        QMessageBox::warning(this, "Error", QString("Unpack failed with code: %1").arg(unpack_result));
    }
}

void QtToolkit::cb_unpack_thread()
{
    // Thread implementation if needed
}

void QtToolkit::cb_pack()
{
    QString dir = input_dir->text();
    QString file = input_file->text();
    
    if (file.isEmpty() || dir.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please select both directory and file!");
        return;
    }
    
    pack_result = pak->Pack(dir.toStdString(), file.toStdString());
    
    if (pack_result == 0)
    {
        QMessageBox::information(this, "Success", "Packed successfully!");
    }
    else
    {
        QMessageBox::warning(this, "Error", QString("Pack failed with code: %1").arg(pack_result));
    }
}

void QtToolkit::cb_pack_thread()
{
    // Thread implementation if needed
}

void QtToolkit::cb_debug_mode()
{
    int debug_level = choice_debug->currentIndex();
    pvz->DebugMode(debug_level);
}

void QtToolkit::cb_speed()
{
    int speed_index = choice_speed->currentIndex();
    int frame_duration = 10; // Default 1x
    
    switch (speed_index)
    {
    case 0: frame_duration = 10; break; // 1x
    case 1: frame_duration = 5; break;  // 2x
    case 2: frame_duration = 2; break;  // 5x
    case 3: frame_duration = 1; break;  // 10x
    }
    
    pvz->SetFrameDuration(frame_duration);
}

void QtToolkit::cb_limbo_page()
{
    pvz->UnlockLimboPage(check_limbo_page->isChecked());
}

// Additional Window Callbacks

void QtToolkit::cb_scheme()
{
    // Theme switching is handled in applyModernTheme
    applyModernTheme();
}

void QtToolkit::cb_document()
{
    QDesktopServices::openUrl(QUrl("https://pvz.lmintlcx.com/toolkit/"));
}

void QtToolkit::cb_about()
{
    QMessageBox::about(this, "About PvZ Toolkit",
        QString("PvZ Toolkit %1 (Modern UI)\n\n"
                "A comprehensive modification tool for Plants vs. Zombies.\n\n"
                "Built with Qt6 for a modern user experience.\n\n"
                "Original FLTK version by lmintlcx\n"
                "Qt port with AI assistance\n\n"
                "License: GPL-3.0\n"
                "Website: https://pvz.lmintlcx.com/toolkit/").arg(VERSION_NAME));
}

} // namespace Pt
