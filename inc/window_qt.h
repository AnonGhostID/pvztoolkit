#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QTextEdit>
#include <QTableWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QSettings>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>
#include <QDialog>
#include <QRadioButton>

#include "checkcombobox.h"
#include <Windows.h>
#include <filesystem>
#include <vector>
#include <array>

#include "lineup.h"
#include "pvz.h"
#include "version.h"

namespace Pt
{

class QtSpawnWindow : public QDialog
{
    Q_OBJECT

public:
    QtSpawnWindow(QWidget *parent = nullptr);
    ~QtSpawnWindow();
    
    void UpdateData(std::array<int, 1000> spawn_list);
    
    QTableWidget *table_spawn;
    QPushButton *button_update_details;
    QMenu *menu_zombies_list;

private:
    static const int ROWS = 33;
    static const int COLS = 21;
    int data[ROWS][COLS] = {{0}};
    int total = 0;
};

class QtWindow : public QMainWindow
{
    Q_OBJECT

public:
    QtWindow(QWidget *parent = nullptr);
    virtual ~QtWindow();
    
    void ReadSettings();
    void WriteSettings();
    
public:
    QTabWidget *tabs;
    
    QWidget *group_resource;
    QRadioButton *check_unlock_sun_limit;
    QSpinBox *input_sun;
    QPushButton *button_sun;
    QLabel *box_money;
    QSpinBox *input_money;
    QPushButton *button_money;
    QCheckBox *check_auto_collected;
    QCheckBox *check_not_drop_loot;
    QCheckBox *check_fertilizer;
    QCheckBox *check_bug_spray;
    QCheckBox *check_tree_food;
    QCheckBox *check_chocolate;
    QLabel *box_wisdom_tree;
    QSpinBox *input_wisdom_tree;
    QPushButton *button_wisdom_tree;
    QCheckBox *check_free_planting;
    QCheckBox *check_placed_anywhere;
    QCheckBox *check_fast_belt;
    QCheckBox *check_lock_shovel;
    QComboBox *choice_mode;
    QComboBox *choice_adventure;
    QPushButton *button_mix;
    QSpinBox *input_level;
    QPushButton *button_level;
    QPushButton *button_unlock;
    QPushButton *button_direct_win;
    QCheckBox *check_brightest_cob_cannon;
    
    QWidget *group_battle;
    CheckComboBox *choice_row;
    CheckComboBox *choice_col;
    QComboBox *choice_plant;
    QComboBox *choice_zombie;
    QPushButton *button_put_plant;
    QCheckBox *check_imitater;
    QPushButton *button_put_zombie;
    QPushButton *button_put_ladder;
    QPushButton *button_put_grave;
    QPushButton *button_put_rake;
    QPushButton *button_lawn_mower_start;
    QPushButton *button_lawn_mower_delete;
    QPushButton *button_lawn_mower_restore;
    QComboBox *choice_item;
    QPushButton *button_clear;
    QCheckBox *check_plant_invincible;
    QCheckBox *check_plant_weak;
    QCheckBox *check_zombie_invincible;
    QCheckBox *check_zombie_weak;
    QCheckBox *check_reload_instantly;
    QCheckBox *check_mushroom_awake;
    QCheckBox *check_stop_spawning;
    QCheckBox *check_stop_zombies;
    QCheckBox *check_lock_butter;
    QCheckBox *check_no_crater;
    QCheckBox *check_no_ice_trail;
    QCheckBox *check_zombie_not_explode;
    
    QWidget *group_lineup;
    QComboBox *choice_slot;
    QComboBox *choice_seed;
    QPushButton *button_seed;
    QCheckBox *check_seed_imitater;
    QCheckBox *check_lineup_mode;
    QPushButton *button_auto_ladder;
    unsigned int lily_pad_col_lower[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    unsigned int lily_pad_col_upper[8] = {2, 3, 4, 5, 6, 7, 8, 9};
    unsigned int flower_pot_col_lower[7] = {1, 4, 4, 4, 4, 4, 4};
    unsigned int flower_pot_col_upper[7] = {3, 4, 5, 6, 7, 8, 9};
    QMenu *menu_put_lily_pad;
    QMenu *menu_put_flower_pot;
    QPushButton *button_put_lily_pad;
    QPushButton *button_put_flower_pot;
    QPushButton *button_reset;
    QComboBox *choice_scene;
    std::vector<Lineup> lineups;
    unsigned int lineup_count[6] = {0};
    QPushButton *button_load_lineup;
    QComboBox *choice_lineup_name[6];
    QTextEdit *editor_lineup_string;
    QPushButton *button_get_lineup;
    QPushButton *button_copy_lineup;
    QPushButton *button_paste_lineup;
    QPushButton *button_set_lineup;
    QPushButton *button_capture;
    
    QWidget *group_spawn;
    int spawn_type[20];
    bool limit_species = true;
    QCheckBox *check_zombie[20];
    QPushButton *button_show_details;
    QComboBox *choice_giga_weight;
    QCheckBox *check_giga_limit;
    QPushButton *button_set_spawn;
    QMenu *menu_spawn_extra;
    QMenu *menu_spawn_mode;
    
    QWidget *group_others;
    QComboBox *choice_music;
    QPushButton *button_music;
    QPushButton *button_userdata;
    QCheckBox *check_no_fog;
    QCheckBox *check_see_vase;
    QCheckBox *check_background;
    QCheckBox *check_readonly;
    QPushButton *button_file;
    QLineEdit *input_file;
    QPushButton *button_unpack;
    QPushButton *button_dir;
    QLineEdit *input_dir;
    QPushButton *button_pack;
    QComboBox *choice_debug;
    QPushButton *button_debug;
    QComboBox *choice_speed;
    QPushButton *button_speed;
    QCheckBox *check_limbo_page;
    QComboBox *choice_scheme;
    QPushButton *button_document;
    QPushButton *button_about;
    
    int result = PVZ_NOT_FOUND;
    QLabel *game_status;
    bool bad_version_warned = false;
    
    QtSpawnWindow *window_spawn = nullptr;
    
protected slots:
    virtual void cb_find_result(int);
    void keep_selected_feature();
    void cb_mode();
    void cb_load_lineup();
    void import_lineup_list(bool);
    void import_lineup_list_file(std::wstring);
    void cb_switch_lineup_scene();
    void cb_show_lineup_string();
    void cb_copy_lineup();
    void cb_paste_lineup();
    void cb_spawn_mutex_0();
    void cb_spawn_mutex_3();
    void cb_spawn_count_check();
    void cb_clear_checked_zombies();
    void cb_disable_limit_species();
    void cb_switch_spawn_mode();
    void cb_open_file();
    void cb_open_dir();
    void cb_scheme();
    void cb_document();
    void cb_about();
    
private:
    void createResourceTab();
    void createBattleTab();
    void createLineupTab();
    void createSpawnTab();
    void createOthersTab();
    void applyModernTheme();
    void applyLightTheme();
};

} // namespace Pt
