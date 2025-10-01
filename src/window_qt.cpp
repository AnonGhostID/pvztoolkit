#include "../inc/window_qt.h"
#include "../inc/utils.h"

#include <fstream>

#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QClipboard>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QScrollArea>

namespace Pt
{

// Plant and zombie names from original window.cpp
const char *plants[] = {
    "Peashooter", "Sunflower", "Cherry Bomb", "Wall-nut", "Potato Mine",
    "Snow Pea", "Chomper", "Repeater", "Puff-shroom", "Sun-shroom",
    "Fume-shroom", "Grave Buster", "Hypno-shroom", "Scaredy-shroom", "Ice-shroom",
    "Doom-shroom", "Lily Pad", "Squash", "Threepeater", "Tangle Kelp",
    "Jalapeno", "Spikeweed", "Torchwood", "Tall-nut", "Sea-shroom",
    "Plantern", "Cactus", "Blover", "Split Pea", "Starfruit",
    "Pumpkin", "Magnet-shroom", "Cabbage-pult", "Flower Pot", "Kernel-pult",
    "Coffee Bean", "Garlic", "Umbrella Leaf", "Marigold", "Melon-pult",
    "Gatling Pea", "Twin Sunflower", "Gloom-shroom", "Cattail", "Winter Melon",
    "Gold Magnet", "Spikerock", "Cob Cannon",
};

const char *zombies[] = {
    "Zombie", "Flag Zombie", "Conehead Zombie", "Pole Vaulting Zombie", "Buckethead Zombie",
    "Newspaper Zombie", "Screen Door Zombie", "Football Zombie", "Dancing Zombie", "Backup Dancer",
    "Ducky Tube Zombie", "Snorkel Zombie", "Zomboni", "Zombie Bobsled Team", "Dolphin Rider Zombie",
    "Jack-in-the-Box Zombie", "Balloon Zombie", "Digger Zombie", "Pogo Zombie", "Zombie Yeti",
    "Bungee Zombie", "Ladder Zombie", "Catapult Zombie", "Gargantuar", "Imp",
    "Dr. Zomboss", "Peashooter Zombie", "Wall-nut Zombie", "Jalapeno Zombie", "Gatling Pea Zombie",
    "Squash Zombie", "Tall-nut Zombie", "GigaGargantuar",
};

const char *modes[] = {
    "Adventure", "Survival: Day", "Survival: Night", "Survival: Pool", "Survival: Fog",
    "Survival: Roof", "Survival: Day (Hard)", "Survival: Night (Hard)", "Survival: Pool (Hard)",
    "Survival: Fog (Hard)", "Survival: Roof (Hard)", "Survival: Day (Endless)", "Survival: Night (Endless)",
    "Survival: Endless", "Survival: Fog (Endless)", "Survival: Roof (Endless)",
};

QtSpawnWindow::QtSpawnWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Spawn Details");
    resize(1000, 600);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    table_spawn = new QTableWidget(ROWS, COLS, this);
    table_spawn->setHorizontalHeaderLabels({"Z Type", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                                            "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "Total"});
    
    for (int i = 0; i < ROWS; i++) {
        table_spawn->setVerticalHeaderItem(i, new QTableWidgetItem(zombies[i]));
    }
    
    table_spawn->horizontalHeader()->setStretchLastSection(true);
    table_spawn->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    layout->addWidget(table_spawn);
    
    button_update_details = new QPushButton("Update Details", this);
    layout->addWidget(button_update_details);
}

QtSpawnWindow::~QtSpawnWindow()
{
}

void QtSpawnWindow::UpdateData(std::array<int, 1000> spawn_list)
{
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            data[i][j] = 0;
        }
    }
    
    total = 0;
    for (int wave = 0; wave < 20; wave++) {
        for (int idx = 0; idx < 50; idx++) {
            int zombie_type = spawn_list[wave * 50 + idx];
            if (zombie_type >= 0 && zombie_type < ROWS) {
                data[zombie_type][wave + 1]++;
                data[zombie_type][COLS - 1]++;
                total++;
            }
        }
    }
    
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            QString text = (j == 0) ? QString(zombies[i]) : QString::number(data[i][j]);
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            
            if (data[i][j] > 0 && j > 0) {
                item->setBackground(QColor(200, 230, 255));
            }
            
            table_spawn->setItem(i, j, item);
        }
    }
}

QtWindow::QtWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString("PvZ Toolkit %1 (Modern UI)").arg(VERSION_NAME));
    setMinimumSize(720, 420);
    resize(900, 500);
    
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    tabs = new QTabWidget(this);
    mainLayout->addWidget(tabs);
    
    createResourceTab();
    createBattleTab();
    createLineupTab();
    createSpawnTab();
    createOthersTab();
    
    game_status = new QLabel("Status: Searching for game...", this);
    game_status->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(game_status);
    
    applyModernTheme();
    ReadSettings();
}

QtWindow::~QtWindow()
{
    WriteSettings();
    if (window_spawn) {
        delete window_spawn;
    }
}

void QtWindow::createResourceTab()
{
    group_resource = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(group_resource);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    QGroupBox *sunMoneyGroup = new QGroupBox("Resources", group_resource);
    QGridLayout *sunMoneyLayout = new QGridLayout(sunMoneyGroup);
    sunMoneyLayout->setSpacing(8);
    
    check_unlock_sun_limit = new QRadioButton("Unlock Sun Limit", sunMoneyGroup);
    sunMoneyLayout->addWidget(check_unlock_sun_limit, 0, 0, 1, 3);
    
    sunMoneyLayout->addWidget(new QLabel("Sun:"), 1, 0);
    input_sun = new QSpinBox(sunMoneyGroup);
    input_sun->setRange(0, 999999);
    input_sun->setValue(9990);
    sunMoneyLayout->addWidget(input_sun, 1, 1);
    button_sun = new QPushButton("Set", sunMoneyGroup);
    button_sun->setMaximumWidth(80);
    sunMoneyLayout->addWidget(button_sun, 1, 2);
    
    box_money = new QLabel("Money:", sunMoneyGroup);
    sunMoneyLayout->addWidget(box_money, 2, 0);
    input_money = new QSpinBox(sunMoneyGroup);
    input_money->setRange(0, 99999);
    input_money->setSingleStep(100);
    input_money->setValue(8000);
    sunMoneyLayout->addWidget(input_money, 2, 1);
    button_money = new QPushButton("Set", sunMoneyGroup);
    button_money->setMaximumWidth(80);
    sunMoneyLayout->addWidget(button_money, 2, 2);
    
    mainLayout->addWidget(sunMoneyGroup);
    
    QGroupBox *itemsGroup = new QGroupBox("Garden Items", group_resource);
    QVBoxLayout *itemsLayout = new QVBoxLayout(itemsGroup);
    itemsLayout->setSpacing(6);
    
    check_auto_collected = new QCheckBox("Auto Collect Resources", itemsGroup);
    check_not_drop_loot = new QCheckBox("No Drop Loot", itemsGroup);
    check_fertilizer = new QCheckBox("Unlimited Fertilizer", itemsGroup);
    check_bug_spray = new QCheckBox("Unlimited Bug Spray", itemsGroup);
    check_tree_food = new QCheckBox("Unlimited Tree Food", itemsGroup);
    check_chocolate = new QCheckBox("Unlimited Chocolate", itemsGroup);
    
    itemsLayout->addWidget(check_auto_collected);
    itemsLayout->addWidget(check_not_drop_loot);
    itemsLayout->addWidget(check_fertilizer);
    itemsLayout->addWidget(check_bug_spray);
    itemsLayout->addWidget(check_tree_food);
    itemsLayout->addWidget(check_chocolate);
    
    QHBoxLayout *treeLayout = new QHBoxLayout();
    box_wisdom_tree = new QLabel("Wisdom Tree Height:");
    input_wisdom_tree = new QSpinBox();
    input_wisdom_tree->setRange(0, 999999);
    input_wisdom_tree->setValue(1000);
    button_wisdom_tree = new QPushButton("Set");
    button_wisdom_tree->setMaximumWidth(80);
    treeLayout->addWidget(box_wisdom_tree);
    treeLayout->addWidget(input_wisdom_tree);
    treeLayout->addWidget(button_wisdom_tree);
    itemsLayout->addLayout(treeLayout);
    
    mainLayout->addWidget(itemsGroup);
    
    QGroupBox *plantingGroup = new QGroupBox("Planting Options", group_resource);
    QVBoxLayout *plantingLayout = new QVBoxLayout(plantingGroup);
    plantingLayout->setSpacing(6);
    
    check_free_planting = new QCheckBox("Free Planting (No Cost)", plantingGroup);
    check_placed_anywhere = new QCheckBox("Place Anywhere", plantingGroup);
    check_fast_belt = new QCheckBox("Fast Conveyor Belt", plantingGroup);
    check_lock_shovel = new QCheckBox("Lock Shovel (Continuous Use)", plantingGroup);
    
    plantingLayout->addWidget(check_free_planting);
    plantingLayout->addWidget(check_placed_anywhere);
    plantingLayout->addWidget(check_fast_belt);
    plantingLayout->addWidget(check_lock_shovel);
    
    mainLayout->addWidget(plantingGroup);
    
    QGroupBox *levelGroup = new QGroupBox("Level Control", group_resource);
    QGridLayout *levelLayout = new QGridLayout(levelGroup);
    levelLayout->setSpacing(8);
    
    levelLayout->addWidget(new QLabel("Mode:"), 0, 0);
    choice_mode = new QComboBox();
    for (int i = 0; i < 16; i++) {
        choice_mode->addItem(modes[i]);
    }
    levelLayout->addWidget(choice_mode, 0, 1, 1, 2);
    
    levelLayout->addWidget(new QLabel("Adventure:"), 1, 0);
    choice_adventure = new QComboBox();
    for (int i = 1; i <= 50; i++) {
        choice_adventure->addItem(QString("Level %1").arg(i));
    }
    levelLayout->addWidget(choice_adventure, 1, 1, 1, 2);
    
    button_mix = new QPushButton("Set Mixed Mode");
    levelLayout->addWidget(button_mix, 2, 0, 1, 3);
    
    levelLayout->addWidget(new QLabel("Endless Rounds:"), 3, 0);
    input_level = new QSpinBox();
    input_level->setRange(0, 999999);
    input_level->setValue(1010);
    levelLayout->addWidget(input_level, 3, 1);
    button_level = new QPushButton("Set");
    button_level->setMaximumWidth(80);
    levelLayout->addWidget(button_level, 3, 2);
    
    button_unlock = new QPushButton("🏆 Unlock All");
    button_direct_win = new QPushButton("⚡ Direct Win");
    check_brightest_cob_cannon = new QCheckBox("Brightest Cob Cannon on Win");
    
    levelLayout->addWidget(button_unlock, 4, 0, 1, 3);
    levelLayout->addWidget(button_direct_win, 5, 0, 1, 3);
    levelLayout->addWidget(check_brightest_cob_cannon, 6, 0, 1, 3);
    
    mainLayout->addWidget(levelGroup);
    mainLayout->addStretch();

    QScrollArea *resourceScroll = new QScrollArea(this);
    resourceScroll->setWidget(group_resource);
    resourceScroll->setWidgetResizable(true);
    resourceScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tabs->addTab(resourceScroll, "⚡ Resource");
}

void QtWindow::createBattleTab()
{
    group_battle = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(group_battle);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    QGroupBox *placementGroup = new QGroupBox("Plant/Zombie Placement", group_battle);
    QGridLayout *placementLayout = new QGridLayout(placementGroup);
    placementLayout->setSpacing(8);
    
    placementLayout->addWidget(new QLabel("Row:"), 0, 0);
    choice_row = new CheckComboBox();
    choice_row->setPlaceholderText("Select Rows");
    choice_row->addAllItem("All Rows");
    for (int i = 1; i <= 6; i++) {
        choice_row->addCheckItem(QString("Row %1").arg(i), i - 1);
    }
    choice_row->setCheckedValues({0});
    placementLayout->addWidget(choice_row, 0, 1);
    
    placementLayout->addWidget(new QLabel("Col:"), 0, 2);
    choice_col = new CheckComboBox();
    choice_col->setPlaceholderText("Select Columns");
    choice_col->addAllItem("All Columns");
    for (int i = 1; i <= 9; i++) {
        choice_col->addCheckItem(QString("Column %1").arg(i), i - 1);
    }
    choice_col->setCheckedValues({0});
    placementLayout->addWidget(choice_col, 0, 3);
    
    placementLayout->addWidget(new QLabel("Plant:"), 1, 0);
    choice_plant = new QComboBox();
    for (int i = 0; i < 48; i++) {
        choice_plant->addItem(plants[i]);
    }
    placementLayout->addWidget(choice_plant, 1, 1, 1, 2);
    
    button_put_plant = new QPushButton("Put Plant");
    placementLayout->addWidget(button_put_plant, 1, 3);
    
    check_imitater = new QCheckBox("Imitater");
    placementLayout->addWidget(check_imitater, 2, 0, 1, 4);
    
    placementLayout->addWidget(new QLabel("Zombie:"), 3, 0);
    choice_zombie = new QComboBox();
    for (int i = 0; i < 33; i++) {
        choice_zombie->addItem(zombies[i]);
    }
    placementLayout->addWidget(choice_zombie, 3, 1, 1, 2);
    
    button_put_zombie = new QPushButton("Put Zombie");
    placementLayout->addWidget(button_put_zombie, 3, 3);
    
    button_put_ladder = new QPushButton("Put Ladder");
    button_put_grave = new QPushButton("Put Grave");
    button_put_rake = new QPushButton("Put Rake");
    
    placementLayout->addWidget(button_put_ladder, 4, 0, 1, 2);
    placementLayout->addWidget(button_put_grave, 4, 2, 1, 2);
    placementLayout->addWidget(button_put_rake, 5, 0, 1, 2);
    
    mainLayout->addWidget(placementGroup);
    
    QGroupBox *lawnMowerGroup = new QGroupBox("Lawn Mowers", group_battle);
    QHBoxLayout *lawnMowerLayout = new QHBoxLayout(lawnMowerGroup);
    
    button_lawn_mower_start = new QPushButton("Start All");
    button_lawn_mower_delete = new QPushButton("Delete All");
    button_lawn_mower_restore = new QPushButton("Restore All");
    
    lawnMowerLayout->addWidget(button_lawn_mower_start);
    lawnMowerLayout->addWidget(button_lawn_mower_delete);
    lawnMowerLayout->addWidget(button_lawn_mower_restore);
    
    mainLayout->addWidget(lawnMowerGroup);
    
    QGroupBox *clearGroup = new QGroupBox("Clear Operations", group_battle);
    QGridLayout *clearLayout = new QGridLayout(clearGroup);
    
    clearLayout->addWidget(new QLabel("Clear:"), 0, 0);
    choice_item = new QComboBox();
    choice_item->addItem("All Plants");
    choice_item->addItem("All Zombies");
    choice_item->addItem("Craters");
    choice_item->addItem("Ladders");
    choice_item->addItem("Graves");
    clearLayout->addWidget(choice_item, 0, 1);
    
    button_clear = new QPushButton("Clear Selected");
    clearLayout->addWidget(button_clear, 0, 2);
    
    mainLayout->addWidget(clearGroup);
    
    QGroupBox *modifiersGroup = new QGroupBox("Battle Modifiers", group_battle);
    QVBoxLayout *modifiersLayout = new QVBoxLayout(modifiersGroup);
    modifiersLayout->setSpacing(6);
    
    check_plant_invincible = new QCheckBox("Plant Invincible");
    check_plant_weak = new QCheckBox("Plant Weak");
    check_zombie_invincible = new QCheckBox("Zombie Invincible");
    check_zombie_weak = new QCheckBox("Zombie Weak");
    check_reload_instantly = new QCheckBox("Reload Instantly");
    check_mushroom_awake = new QCheckBox("Mushrooms Awake");
    check_stop_spawning = new QCheckBox("Stop Spawning");
    check_stop_zombies = new QCheckBox("Stop Zombies");
    check_lock_butter = new QCheckBox("Lock Butter (Always Stun)");
    check_no_crater = new QCheckBox("No Crater");
    check_no_ice_trail = new QCheckBox("No Ice Trail");
    check_zombie_not_explode = new QCheckBox("Jack/Jalapeno Not Explode");
    
    modifiersLayout->addWidget(check_plant_invincible);
    modifiersLayout->addWidget(check_plant_weak);
    modifiersLayout->addWidget(check_zombie_invincible);
    modifiersLayout->addWidget(check_zombie_weak);
    modifiersLayout->addWidget(check_reload_instantly);
    modifiersLayout->addWidget(check_mushroom_awake);
    modifiersLayout->addWidget(check_stop_spawning);
    modifiersLayout->addWidget(check_stop_zombies);
    modifiersLayout->addWidget(check_lock_butter);
    modifiersLayout->addWidget(check_no_crater);
    modifiersLayout->addWidget(check_no_ice_trail);
    modifiersLayout->addWidget(check_zombie_not_explode);
    
    mainLayout->addWidget(modifiersGroup);
    mainLayout->addStretch();

    QScrollArea *battleScroll = new QScrollArea(this);
    battleScroll->setWidget(group_battle);
    battleScroll->setWidgetResizable(true);
    battleScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tabs->addTab(battleScroll, "⚔️ Battle");
}

void QtWindow::createLineupTab()
{
    group_lineup = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(group_lineup);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    QGroupBox *seedGroup = new QGroupBox("Seed Modification", group_lineup);
    QGridLayout *seedLayout = new QGridLayout(seedGroup);
    
    seedLayout->addWidget(new QLabel("Slot:"), 0, 0);
    choice_slot = new QComboBox();
    for (int i = 1; i <= 10; i++) {
        choice_slot->addItem(QString::number(i));
    }
    seedLayout->addWidget(choice_slot, 0, 1);
    
    seedLayout->addWidget(new QLabel("Seed:"), 0, 2);
    choice_seed = new QComboBox();
    for (int i = 0; i < 48; i++) {
        choice_seed->addItem(plants[i]);
    }
    seedLayout->addWidget(choice_seed, 0, 3);
    
    button_seed = new QPushButton("Set Seed");
    seedLayout->addWidget(button_seed, 0, 4);
    
    check_seed_imitater = new QCheckBox("Imitater");
    seedLayout->addWidget(check_seed_imitater, 1, 0, 1, 2);
    
    check_lineup_mode = new QCheckBox("Quick Lineup Mode");
    seedLayout->addWidget(check_lineup_mode, 1, 2, 1, 3);
    
    mainLayout->addWidget(seedGroup);
    
    QGroupBox *setupGroup = new QGroupBox("Quick Setup", group_lineup);
    QGridLayout *setupLayout = new QGridLayout(setupGroup);
    
    button_auto_ladder = new QPushButton("Auto Ladder");
    setupLayout->addWidget(button_auto_ladder, 0, 0);
    
    button_put_lily_pad = new QPushButton("Put Lily Pads");
    setupLayout->addWidget(button_put_lily_pad, 0, 1);
    
    button_put_flower_pot = new QPushButton("Put Flower Pots");
    setupLayout->addWidget(button_put_flower_pot, 0, 2);
    
    button_reset = new QPushButton("Reset Scene");
    setupLayout->addWidget(button_reset, 1, 0);
    
    setupLayout->addWidget(new QLabel("Scene:"), 1, 1);
    choice_scene = new QComboBox();
    choice_scene->addItem("Day");
    choice_scene->addItem("Night");
    choice_scene->addItem("Pool");
    choice_scene->addItem("Fog");
    choice_scene->addItem("Roof");
    choice_scene->addItem("Moon");
    setupLayout->addWidget(choice_scene, 1, 2);
    
    mainLayout->addWidget(setupGroup);
    
    QGroupBox *lineupCodeGroup = new QGroupBox("Lineup Code", group_lineup);
    QVBoxLayout *lineupCodeLayout = new QVBoxLayout(lineupCodeGroup);
    
    button_load_lineup = new QPushButton("Load Lineup List");
    lineupCodeLayout->addWidget(button_load_lineup);
    
    for (int i = 0; i < 6; i++) {
        choice_lineup_name[i] = new QComboBox();
        lineupCodeLayout->addWidget(choice_lineup_name[i]);
    }
    
    editor_lineup_string = new QTextEdit();
    editor_lineup_string->setMaximumHeight(120);
    editor_lineup_string->setPlaceholderText("Enter lineup code here...");
    lineupCodeLayout->addWidget(editor_lineup_string);
    
    QHBoxLayout *lineupButtonsLayout = new QHBoxLayout();
    button_get_lineup = new QPushButton("Get Current");
    button_copy_lineup = new QPushButton("Copy");
    button_paste_lineup = new QPushButton("Paste");
    button_set_lineup = new QPushButton("Apply Lineup");
    button_capture = new QPushButton("📷 Screenshot");
    
    lineupButtonsLayout->addWidget(button_get_lineup);
    lineupButtonsLayout->addWidget(button_copy_lineup);
    lineupButtonsLayout->addWidget(button_paste_lineup);
    lineupButtonsLayout->addWidget(button_set_lineup);
    lineupButtonsLayout->addWidget(button_capture);
    
    lineupCodeLayout->addLayout(lineupButtonsLayout);
    
    mainLayout->addWidget(lineupCodeGroup);
    mainLayout->addStretch();

    QScrollArea *lineupScroll = new QScrollArea(this);
    lineupScroll->setWidget(group_lineup);
    lineupScroll->setWidgetResizable(true);
    lineupScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tabs->addTab(lineupScroll, "📋 Lineup");
}

void QtWindow::createSpawnTab()
{
    group_spawn = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(group_spawn);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    QGroupBox *zombieSelectionGroup = new QGroupBox("Select Zombies to Spawn", group_spawn);
    QGridLayout *zombieLayout = new QGridLayout(zombieSelectionGroup);
    zombieLayout->setSpacing(8);
    
    for (int i = 0; i < 20; i++) {
        check_zombie[i] = new QCheckBox(zombies[i]);
        zombieLayout->addWidget(check_zombie[i], i / 4, i % 4);
    }
    
    mainLayout->addWidget(zombieSelectionGroup);
    
    QGroupBox *spawnOptionsGroup = new QGroupBox("Spawn Options", group_spawn);
    QGridLayout *spawnOptionsLayout = new QGridLayout(spawnOptionsGroup);
    
    button_show_details = new QPushButton("📊 Show Details");
    spawnOptionsLayout->addWidget(button_show_details, 0, 0, 1, 2);
    
    spawnOptionsLayout->addWidget(new QLabel("Giga Weight:"), 1, 0);
    choice_giga_weight = new QComboBox();
    choice_giga_weight->addItem("Normal");
    choice_giga_weight->addItem("High");
    choice_giga_weight->addItem("Very High");
    spawnOptionsLayout->addWidget(choice_giga_weight, 1, 1);
    
    check_giga_limit = new QCheckBox("Limit Giga Count");
    spawnOptionsLayout->addWidget(check_giga_limit, 2, 0, 1, 2);
    
    button_set_spawn = new QPushButton("⚡ Apply Spawn List");
    button_set_spawn->setMinimumHeight(40);
    spawnOptionsLayout->addWidget(button_set_spawn, 3, 0, 1, 2);
    
    mainLayout->addWidget(spawnOptionsGroup);
    
    QGroupBox *presetsGroup = new QGroupBox("Spawn Presets", group_spawn);
    QVBoxLayout *presetsLayout = new QVBoxLayout(presetsGroup);
    
    QPushButton *btnNatural = new QPushButton("Natural Mode");
    QPushButton *btnExtreme = new QPushButton("Extreme Mode");
    QPushButton *btnCustom = new QPushButton("Custom Mode");
    QPushButton *btnClear = new QPushButton("Clear Selection");
    
    connect(btnNatural, &QPushButton::clicked, this, &QtWindow::cb_spawn_mutex_0);
    connect(btnExtreme, &QPushButton::clicked, this, &QtWindow::cb_spawn_mutex_3);
    connect(btnCustom, &QPushButton::clicked, this, &QtWindow::cb_switch_spawn_mode);
    connect(btnClear, &QPushButton::clicked, this, &QtWindow::cb_clear_checked_zombies);
    
    presetsLayout->addWidget(btnNatural);
    presetsLayout->addWidget(btnExtreme);
    presetsLayout->addWidget(btnCustom);
    presetsLayout->addWidget(btnClear);
    
    mainLayout->addWidget(presetsGroup);
    mainLayout->addStretch();

    QScrollArea *spawnScroll = new QScrollArea(this);
    spawnScroll->setWidget(group_spawn);
    spawnScroll->setWidgetResizable(true);
    spawnScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tabs->addTab(spawnScroll, "🧟 Spawn");
}

void QtWindow::createOthersTab()
{
    group_others = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(group_others);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    QGroupBox *audioGroup = new QGroupBox("Audio", group_others);
    QHBoxLayout *audioLayout = new QHBoxLayout(audioGroup);
    
    audioLayout->addWidget(new QLabel("Music:"));
    choice_music = new QComboBox();
    choice_music->addItem("Default");
    choice_music->addItem("Grasswalk");
    choice_music->addItem("Moongrains");
    audioLayout->addWidget(choice_music);
    
    button_music = new QPushButton("Set Music");
    audioLayout->addWidget(button_music);
    
    mainLayout->addWidget(audioGroup);
    
    QGroupBox *visualGroup = new QGroupBox("Visual Effects", group_others);
    QVBoxLayout *visualLayout = new QVBoxLayout(visualGroup);
    
    check_no_fog = new QCheckBox("Remove Fog");
    check_see_vase = new QCheckBox("See Through Vases");
    check_background = new QCheckBox("Background Running");
    
    visualLayout->addWidget(check_no_fog);
    visualLayout->addWidget(check_see_vase);
    visualLayout->addWidget(check_background);
    
    mainLayout->addWidget(visualGroup);
    
    QGroupBox *savesGroup = new QGroupBox("Saves Management", group_others);
    QVBoxLayout *savesLayout = new QVBoxLayout(savesGroup);
    
    button_userdata = new QPushButton("📁 Open Userdata Folder");
    check_readonly = new QCheckBox("Userdata Read-Only");
    
    savesLayout->addWidget(button_userdata);
    savesLayout->addWidget(check_readonly);
    
    mainLayout->addWidget(savesGroup);
    
    QGroupBox *pakGroup = new QGroupBox("PAK File Tools", group_others);
    QGridLayout *pakLayout = new QGridLayout(pakGroup);
    
    pakLayout->addWidget(new QLabel("File:"), 0, 0);
    input_file = new QLineEdit();
    pakLayout->addWidget(input_file, 0, 1);
    button_file = new QPushButton("Browse");
    pakLayout->addWidget(button_file, 0, 2);
    
    button_unpack = new QPushButton("Unpack");
    pakLayout->addWidget(button_unpack, 1, 0);
    
    pakLayout->addWidget(new QLabel("Dir:"), 2, 0);
    input_dir = new QLineEdit();
    pakLayout->addWidget(input_dir, 2, 1);
    button_dir = new QPushButton("Browse");
    pakLayout->addWidget(button_dir, 2, 2);
    
    button_pack = new QPushButton("Pack");
    pakLayout->addWidget(button_pack, 3, 0);
    
    mainLayout->addWidget(pakGroup);
    
    QGroupBox *advancedGroup = new QGroupBox("Advanced", group_others);
    QGridLayout *advancedLayout = new QGridLayout(advancedGroup);
    
    advancedLayout->addWidget(new QLabel("Debug Mode:"), 0, 0);
    choice_debug = new QComboBox();
    choice_debug->addItem("Off");
    choice_debug->addItem("Level 1");
    choice_debug->addItem("Level 2");
    advancedLayout->addWidget(choice_debug, 0, 1);
    button_debug = new QPushButton("Set");
    advancedLayout->addWidget(button_debug, 0, 2);
    
    advancedLayout->addWidget(new QLabel("Game Speed:"), 1, 0);
    choice_speed = new QComboBox();
    choice_speed->addItem("1x");
    choice_speed->addItem("2x");
    choice_speed->addItem("5x");
    choice_speed->addItem("10x");
    advancedLayout->addWidget(choice_speed, 1, 1);
    button_speed = new QPushButton("Set");
    advancedLayout->addWidget(button_speed, 1, 2);
    
    check_limbo_page = new QCheckBox("Unlock Limbo Page");
    advancedLayout->addWidget(check_limbo_page, 2, 0, 1, 3);
    
    mainLayout->addWidget(advancedGroup);
    
    QGroupBox *themeGroup = new QGroupBox("UI Theme", group_others);
    QHBoxLayout *themeLayout = new QHBoxLayout(themeGroup);
    
    themeLayout->addWidget(new QLabel("Color Scheme:"));
    choice_scheme = new QComboBox();
    choice_scheme->addItem("Dark");
    choice_scheme->addItem("Light");
    themeLayout->addWidget(choice_scheme);
    
    mainLayout->addWidget(themeGroup);
    
    QHBoxLayout *infoButtonsLayout = new QHBoxLayout();
    button_document = new QPushButton("📖 Help");
    button_about = new QPushButton("ℹ️ About");
    infoButtonsLayout->addWidget(button_document);
    infoButtonsLayout->addWidget(button_about);
    mainLayout->addLayout(infoButtonsLayout);
    
    mainLayout->addStretch();

    QScrollArea *othersScroll = new QScrollArea(this);
    othersScroll->setWidget(group_others);
    othersScroll->setWidgetResizable(true);
    othersScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tabs->addTab(othersScroll, "⚙️ Others");
}

void QtWindow::applyModernTheme()
{
    QString darkTheme = R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
        QWidget {
            background-color: #1e1e1e;
            color: #e0e0e0;
            font-family: 'Segoe UI', sans-serif;
            font-size: 10pt;
        }
        QTabWidget::pane {
            border: 1px solid #3d3d3d;
            background-color: #252525;
            border-radius: 4px;
        }
        QTabBar::tab {
            background-color: #2d2d2d;
            color: #b0b0b0;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #0d7377;
            color: #ffffff;
        }
        QTabBar::tab:hover {
            background-color: #3d3d3d;
        }
        QGroupBox {
            background-color: #2d2d2d;
            border: 1px solid #3d3d3d;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 12px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            color: #0d7377;
        }
        QPushButton {
            background-color: #0d7377;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #14a5aa;
        }
        QPushButton:pressed {
            background-color: #0a5c5f;
        }
        QPushButton:disabled {
            background-color: #3d3d3d;
            color: #707070;
        }
        QCheckBox, QRadioButton {
            spacing: 8px;
            color: #e0e0e0;
        }
        QCheckBox::indicator, QRadioButton::indicator {
            width: 18px;
            height: 18px;
            border-radius: 3px;
            border: 2px solid #3d3d3d;
            background-color: #2d2d2d;
        }
        QCheckBox::indicator:checked, QRadioButton::indicator:checked {
            background-color: #0d7377;
            border-color: #0d7377;
        }
        QCheckBox::indicator:hover, QRadioButton::indicator:hover {
            border-color: #0d7377;
        }
        QComboBox, QSpinBox, QLineEdit, QTextEdit {
            background-color: #2d2d2d;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            padding: 6px;
            color: #e0e0e0;
        }
        QComboBox:hover, QSpinBox:hover, QLineEdit:hover, QTextEdit:hover {
            border-color: #0d7377;
        }
        QComboBox:focus, QSpinBox:focus, QLineEdit:focus, QTextEdit:focus {
            border-color: #14a5aa;
            border-width: 2px;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #e0e0e0;
            margin-right: 5px;
        }
        QLabel {
            background-color: transparent;
            color: #e0e0e0;
        }
        QTableWidget {
            background-color: #2d2d2d;
            alternate-background-color: #252525;
            gridline-color: #3d3d3d;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #1e1e1e;
            color: #0d7377;
            padding: 6px;
            border: none;
            font-weight: bold;
        }
        QScrollBar:vertical {
            background-color: #2d2d2d;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: #0d7377;
            border-radius: 6px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #14a5aa;
        }
        QScrollBar:horizontal {
            background-color: #2d2d2d;
            height: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:horizontal {
            background-color: #0d7377;
            border-radius: 6px;
            min-width: 20px;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            background: none;
            border: none;
        }
        QStatusBar {
            background-color: #1e1e1e;
            color: #e0e0e0;
        }
    )";
    
    setStyleSheet(darkTheme);
}

void QtWindow::applyLightTheme()
{
    QString lightTheme = R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QWidget {
            background-color: #f5f5f5;
            color: #2d2d2d;
            font-family: 'Segoe UI', sans-serif;
            font-size: 10pt;
        }
        QTabWidget::pane {
            border: 1px solid #d0d0d0;
            background-color: #ffffff;
            border-radius: 4px;
        }
        QTabBar::tab {
            background-color: #e0e0e0;
            color: #505050;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #0d7377;
            color: #ffffff;
        }
        QTabBar::tab:hover {
            background-color: #d0d0d0;
        }
        QGroupBox {
            background-color: #ffffff;
            border: 1px solid #d0d0d0;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 12px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            color: #0d7377;
        }
        QPushButton {
            background-color: #0d7377;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #14a5aa;
        }
        QPushButton:pressed {
            background-color: #0a5c5f;
        }
        QPushButton:disabled {
            background-color: #d0d0d0;
            color: #909090;
        }
        QCheckBox, QRadioButton {
            spacing: 8px;
            color: #2d2d2d;
        }
        QCheckBox::indicator, QRadioButton::indicator {
            width: 18px;
            height: 18px;
            border-radius: 3px;
            border: 2px solid #b0b0b0;
            background-color: #ffffff;
        }
        QCheckBox::indicator:checked, QRadioButton::indicator:checked {
            background-color: #0d7377;
            border-color: #0d7377;
        }
        QCheckBox::indicator:hover, QRadioButton::indicator:hover {
            border-color: #0d7377;
        }
        QComboBox, QSpinBox, QLineEdit, QTextEdit {
            background-color: #ffffff;
            border: 1px solid #b0b0b0;
            border-radius: 4px;
            padding: 6px;
            color: #2d2d2d;
        }
        QComboBox:hover, QSpinBox:hover, QLineEdit:hover, QTextEdit:hover {
            border-color: #0d7377;
        }
        QComboBox:focus, QSpinBox:focus, QLineEdit:focus, QTextEdit:focus {
            border-color: #14a5aa;
            border-width: 2px;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #2d2d2d;
            margin-right: 5px;
        }
        QLabel {
            background-color: transparent;
            color: #2d2d2d;
        }
        QTableWidget {
            background-color: #ffffff;
            alternate-background-color: #f9f9f9;
            gridline-color: #d0d0d0;
            border: 1px solid #d0d0d0;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #e0e0e0;
            color: #0d7377;
            padding: 6px;
            border: none;
            font-weight: bold;
        }
        QScrollBar:vertical {
            background-color: #e0e0e0;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: #0d7377;
            border-radius: 6px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #14a5aa;
        }
        QScrollBar:horizontal {
            background-color: #e0e0e0;
            height: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:horizontal {
            background-color: #0d7377;
            border-radius: 6px;
            min-width: 20px;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            background: none;
            border: none;
        }
        QStatusBar {
            background-color: #e0e0e0;
            color: #2d2d2d;
        }
    )";
    
    setStyleSheet(lightTheme);
}

void QtWindow::ReadSettings()
{
    QSettings settings("PvZToolkit", "QtModernUI");
    restoreGeometry(settings.value("geometry").toByteArray());
    tabs->setCurrentIndex(settings.value("currentTab", 0).toInt());
}

void QtWindow::WriteSettings()
{
    QSettings settings("PvZToolkit", "QtModernUI");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("currentTab", tabs->currentIndex());
}

// Stub implementations for slots (to be implemented in toolkit_qt.cpp or overridden)
void QtWindow::cb_find_result(int) {}
void QtWindow::keep_selected_feature()
{
    // Keep checkboxes state when switching tabs
}

void QtWindow::cb_mode()
{
    int mode = choice_mode->currentIndex();
    if (mode == 0)
    {
        choice_adventure->setEnabled(true);
    }
    else
    {
        choice_adventure->setEnabled(false);
    }
}
void QtWindow::cb_load_lineup()
{
    QString file = QFileDialog::getOpenFileName(this, "Load Lineup List", 
                                                QString(), "Text Files (*.txt);;All Files (*)");
    if (!file.isEmpty())
    {
        import_lineup_list_file(file.toStdWString());
    }
}
void QtWindow::import_lineup_list(bool reload)
{
    if (reload)
    {
        lineups.clear();
        for (int i = 0; i < 6; i++)
        {
            lineup_count[i] = 0;
            choice_lineup_name[i]->clear();
        }
    }
    
    for (auto &lineup : lineups)
    {
        if (lineup.scene >= 0 && lineup.scene < 6)
        {
            choice_lineup_name[lineup.scene]->addItem(QString::fromStdString(lineup.lineup_name));
            lineup_count[lineup.scene]++;
        }
    }
}
void QtWindow::import_lineup_list_file(std::wstring filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        QMessageBox::warning(this, "Error", "Failed to open lineup file!");
        return;
    }
    
    lineups.clear();
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
            
        Lineup lineup(line);
        if (lineup.OK())
        {
            lineups.push_back(lineup);
        }
    }
    
    file.close();
    import_lineup_list(true);
    
    QMessageBox::information(this, "Success", 
                             QString("Loaded %1 lineups").arg(lineups.size()));
}
void QtWindow::cb_switch_lineup_scene()
{
    int scene = choice_scene->currentIndex();
    // Update UI based on selected scene
    if (scene >= 0 && scene < 6)
    {
        // Populate lineup choices for this scene
        choice_lineup_name[scene]->setEnabled(lineup_count[scene] > 0);
    }
}

void QtWindow::cb_show_lineup_string()
{
    // Show lineup string in editor
    int scene = choice_scene->currentIndex();
    if (scene >= 0 && scene < 6)
    {
        int idx = choice_lineup_name[scene]->currentIndex();
        if (idx >= 0 && idx < lineups.size())
        {
            QString code = QString::fromStdString(lineups[idx].lineup_code);
            editor_lineup_string->setPlainText(code);
        }
    }
}
void QtWindow::cb_copy_lineup()
{
    QString code = editor_lineup_string->toPlainText();
    QApplication::clipboard()->setText(code);
}

void QtWindow::cb_paste_lineup()
{
    QString code = QApplication::clipboard()->text();
    editor_lineup_string->setPlainText(code);
}
void QtWindow::cb_spawn_mutex_0()
{
    // Natural spawn mode
    for (int i = 0; i < 20; i++)
    {
        check_zombie[i]->setChecked(false);
    }
    // Select natural zombies
    int natural[] = {0, 2, 4, 5, 6, 7, 11, 12, 15, 17, 22, 23};
    for (int idx : natural)
    {
        if (idx < 20)
            check_zombie[idx]->setChecked(true);
    }
}

void QtWindow::cb_spawn_mutex_3()
{
    // Extreme spawn mode
    for (int i = 0; i < 20; i++)
    {
        check_zombie[i]->setChecked(false);
    }
    // Select extreme zombies
    int extreme[] = {3, 4, 7, 8, 12, 15, 16, 17, 18, 22, 23};
    for (int idx : extreme)
    {
        if (idx < 20)
            check_zombie[idx]->setChecked(true);
    }
}

void QtWindow::cb_spawn_count_check()
{
    int count = 0;
    for (int i = 0; i < 20; i++)
    {
        if (check_zombie[i]->isChecked())
            count++;
    }
    
    if (count == 0)
    {
        QMessageBox::warning(this, "Warning", "No zombies selected!");
    }
}

void QtWindow::cb_clear_checked_zombies()
{
    for (int i = 0; i < 20; i++)
    {
        check_zombie[i]->setChecked(false);
    }
}

void QtWindow::cb_disable_limit_species()
{
    limit_species = !limit_species;
    check_giga_limit->setChecked(limit_species);
}

void QtWindow::cb_switch_spawn_mode()
{
    // Toggle spawn mode
    limit_species = !limit_species;
}
void QtWindow::cb_open_file()
{
    QString file = QFileDialog::getOpenFileName(this, "Select PAK File", QString(), "PAK Files (*.pak);;All Files (*)");
    if (!file.isEmpty())
    {
        input_file->setText(file);
    }
}

void QtWindow::cb_open_dir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory");
    if (!dir.isEmpty())
    {
        input_dir->setText(dir);
    }
}
void QtWindow::cb_scheme()
{
    int scheme = choice_scheme->currentIndex();
    if (scheme == 0) {
        applyModernTheme();
    } else {
        applyLightTheme();
    }
}
void QtWindow::cb_document() {}
void QtWindow::cb_about() {}

} // namespace Pt
