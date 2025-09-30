
#pragma once

#include <Windows.h>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "utils.h"

namespace Pt
{

#define UNPACK_SUCCESS 0           // Unpack successful
#define UNPACK_SRC_NOT_EXIST 1     // Failed to open source file
#define UNPACK_SRC_SIZE_ERROR 2    // Failed to get source file size
#define UNPACK_SRC_LOAD_ERROR 3    // Failed to read source file content
#define UNPACK_SRC_HEADER_ERROR 4  // File header format incorrect
#define UNPACK_SRC_DATA_ERROR 5    // File data corrupted
#define UNPACK_PATH_CREATE_ERROR 6 // Failed to create unpack path
#define UNPACK_FILE_CREATE_ERROR 7 // Failed to create unpack file
#define UNPACK_FILE_WRITE_ERROR 8  // Failed to write unpack file

#define PACK_SUCCESS 0           // Pack successful
#define PACK_SRC_NOT_EXIST 1     // Failed to open source folder
#define PACK_SRC_EMPTY_ERROR 2   // Source folder is empty
#define PACK_PATH_CREATE_ERROR 3 // Failed to create pack path
#define PACK_FILE_CREATE_ERROR 4 // Failed to create pack file
#define PACK_FILE_WRITE_ERROR 5  // Failed to write pack file
#define PACK_SRC_READ_ERROR 6    // Failed to read pack source file

class PAK
{
  public:
    PAK();
    ~PAK();

  private:
    // Recursively create folders
    bool create_path(const std::wstring &);

    // Recursively find all files
    void find_files(const std::wstring &,
                    std::vector<std::wstring> &, //
                    std::vector<int> &,          //
                    std::vector<FILETIME> &);

  public:
    // Unpack, parameters are source filename and unpack folder
    int Unpack(const std::wstring &, const std::wstring &);
    int Unpack(const std::string &, const std::string &);

    // Pack, parameters are source folder and pack filename
    int Pack(const std::wstring &, const std::wstring &);
    int Pack(const std::string &, const std::string &);
};

} // namespace Pt
