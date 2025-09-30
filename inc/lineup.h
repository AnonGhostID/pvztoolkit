
#pragma once

#include <algorithm>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "utils.h"

namespace Pt
{

#define GRID (6 * 9)

class Lineup
{
  public:
    Lineup();
    Lineup(const std::string &);
    Lineup(const std::string &, const std::string &);
    ~Lineup();

    bool may_sleep[48] = {false, false, false, false, false, false, false, false, //
                          true, true, true, false, true, true, true, true,        //
                          false, false, false, false, false, false, false, false, //
                          true, false, false, false, false, false, false, true,   //
                          false, false, false, false, false, false, false, false, //
                          false, false, true, false, false, false, false, false}; //

    void Init(const std::string &); // Lineup string/code -> data
    bool OK();                      // Check if lineup is valid
    std::string Generate();         // Fill data -> lineup code

    std::string lineup_name;   // Lineup name
    std::string lineup_string; // Lineup string
    std::string lineup_code;   // Lineup code

    uint16_t plant[GRID];
    uint16_t plant_im[GRID];
    uint16_t plant_awake[GRID];
    uint16_t base[GRID];
    uint16_t base_im[GRID];
    uint16_t pumpkin[GRID];
    uint16_t pumpkin_im[GRID];
    uint16_t coffee[GRID];
    uint16_t coffee_im[GRID];
    uint16_t ladder[GRID];

    uint8_t rake_row;
    uint8_t scene;

    long long weight; // Weight for sorting

  private:
    void reset_data();

    bool ok;
    uint16_t items[GRID]; // Compressed

    std::vector<std::string> split(const std::string &, char);
    long hex2dec(const std::string &);

    bool lineup_string_to_data();  // Lineup string -> data
    bool lineup_code_to_data();    // Lineup code -> data
    void data_to_lineup_code();    // Data -> lineup code
    inline void compress_data();   // Compress array
    inline void decompress_data(); // Decompress array
};

} // namespace Pt
