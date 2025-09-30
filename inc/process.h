
#pragma once

#include <Windows.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

namespace Pt
{

// Output memory read/write data
#define _PZTK_MEMORY_OUTPUT

class Process
{
  public:
    Process();
    ~Process();

    // Open process by window class name and title
    bool OpenByWindow(const wchar_t *, const wchar_t *);
    bool OpenByProcess(const wchar_t *);

    // Process availability
    bool IsValid();

    // Read memory
    template <typename T>
    T ReadMemory(std::initializer_list<uintptr_t>);

    // Read memory字符串
    template <>
    std::string ReadMemory<std::string>(std::initializer_list<uintptr_t>);

    // Write memory
    template <typename T>
    void WriteMemory(T, std::initializer_list<uintptr_t>);

    // Read memory数组
    template <typename T, size_t size>
    std::array<T, size> ReadMemory(std::initializer_list<uintptr_t>);

    // Write memory数组
    template <typename T, size_t size>
    void WriteMemory(std::array<T, size>, std::initializer_list<uintptr_t>);

  protected:
    HWND hwnd;     // Window handle
    DWORD pid;     // Process ID
    HANDLE handle; // Process handle

    using remote_ptr_t = std::conditional_t<(INTPTR_MAX > INT32_MAX), uint32_t, uintptr_t>;

#if (defined _DEBUG) && (defined _PZTK_MEMORY_OUTPUT)
  private:
    std::string int_to_hex_string(unsigned int num)
    {
        std::stringstream sstream;
        sstream << "0x" << std::hex << num;
        return sstream.str();
    }

    // [[[0x6a9ec0] +0x768] +0x5560]
    std::string addr_list_to_string(std::initializer_list<uintptr_t> addr_list)
    {
        std::string str;
        for (auto it = addr_list.begin(); it != addr_list.end(); it++)
            if (it == addr_list.begin())
                str = "[" + int_to_hex_string(*it) + "]";
            else
                str = "[" + str + " +" + int_to_hex_string(*it) + "]";
        return str;
    }
#endif
};

template <typename T>
T Process::ReadMemory(std::initializer_list<uintptr_t> addr)
{
    T result = T();

    if (!IsValid())
        return result;

    uintptr_t offset = 0;
    for (auto it = addr.begin(); it != addr.end(); it++)
    {
        if (it != addr.end() - 1)
        {
            SIZE_T read_size = 0;
            remote_ptr_t next = 0;
            BOOL ret = ReadProcessMemory(this->handle, reinterpret_cast<const void *>(offset + *it), &next, sizeof(next), &read_size);
            if (ret == 0 || sizeof(next) != read_size)
                return T();
            offset = static_cast<uintptr_t>(next);
        }
        else
        {
            SIZE_T read_size = 0;
            if constexpr (std::is_integral_v<T> && sizeof(T) > sizeof(remote_ptr_t))
            {
                remote_ptr_t tmp = 0;
                BOOL ret = ReadProcessMemory(this->handle, reinterpret_cast<const void *>(offset + *it), &tmp, sizeof(tmp), &read_size);
                if (ret == 0 || sizeof(tmp) != read_size)
                    return T();
                result = static_cast<uintptr_t>(tmp);
            }
            else
            {
                BOOL ret = ReadProcessMemory(this->handle, reinterpret_cast<const void *>(offset + *it), &result, sizeof(result), &read_size);
                if (ret == 0 || sizeof(result) != read_size)
                    return T();
            }
        }
    }

#if (defined _DEBUG) && (defined _PZTK_MEMORY_OUTPUT)
    std::cout << addr_list_to_string(addr) << " --> " << std::dec << result << " / " << std::hex << result << std::endl;
#endif

    return result;
}

template <>
inline std::string Process::ReadMemory<std::string>(std::initializer_list<uintptr_t> addr)
{
    std::string result = std::string();

    if (!IsValid())
        return result;

    uintptr_t offset = 0;
    for (auto it = addr.begin(); it != addr.end(); it++)
    {
        if (it != addr.end() - 1)
        {
            SIZE_T read_size = 0;
            remote_ptr_t next = 0;
            BOOL ret = ReadProcessMemory(this->handle, reinterpret_cast<const void *>(offset + *it), &next, sizeof(next), &read_size);
            if (ret == 0 || sizeof(next) != read_size)
                return std::string();
            offset = static_cast<uintptr_t>(next);
        }
        else
        {
            SIZE_T read_size = 0;
            BOOL ret = 0;
            char ch = 0;

        read_letter:
            ret = ReadProcessMemory(this->handle, reinterpret_cast<const void *>(offset + *it), &ch, sizeof(ch), &read_size);
            while (read_size == sizeof(ch) && ret != 0 && ch != 0)
            {
                result += ch;
                offset += sizeof(ch);
                goto read_letter;
            }
        }
    }

#if (defined _DEBUG) && (defined _PZTK_MEMORY_OUTPUT)
    std::cout << addr_list_to_string(addr) << " --> " << result << std::endl;
#endif

    return result;
}

template <typename T>
void Process::WriteMemory(T value, std::initializer_list<uintptr_t> addr)
{
    if (!IsValid())
        return;

    uintptr_t offset = 0;
    for (auto it = addr.begin(); it != addr.end(); it++)
    {
        if (it != addr.end() - 1)
        {
            SIZE_T read_size = 0;
            remote_ptr_t next = 0;
            BOOL ret = ReadProcessMemory(this->handle, reinterpret_cast<const void *>(offset + *it), &next, sizeof(next), &read_size);
            if (ret == 0 || sizeof(next) != read_size)
                return;
            offset = static_cast<uintptr_t>(next);
        }
        else
        {
            SIZE_T write_size = 0;
            if constexpr (std::is_integral_v<T> && sizeof(T) > sizeof(remote_ptr_t))
            {
                remote_ptr_t tmp = static_cast<remote_ptr_t>(value);
                BOOL ret = WriteProcessMemory(this->handle, reinterpret_cast<void *>(offset + *it), &tmp, sizeof(tmp), &write_size);
                if (ret == 0 || sizeof(tmp) != write_size)
                    return;
            }
            else
            {
                BOOL ret = WriteProcessMemory(this->handle, reinterpret_cast<void *>(offset + *it), &value, sizeof(value), &write_size);
                if (ret == 0 || sizeof(value) != write_size)
                    return;
            }
        }
    }

#if (defined _DEBUG) && (defined _PZTK_MEMORY_OUTPUT)
    std::cout << addr_list_to_string(addr) << " <-- " << std::dec << value << " / " << std::hex << value << std::endl;
    // if (ReadMemory<T>(addr) != value)
    //     std::wcout << L"Memory write error!" << std::endl;
#endif
}

template <typename T, size_t size>
std::array<T, size> Process::ReadMemory(std::initializer_list<uintptr_t> addr)
{
    std::array<T, size> result = {T()};

    if (!IsValid())
        return result;

    T buff[size] = {0};
    uintptr_t offset = 0;
    for (auto it = addr.begin(); it != addr.end(); it++)
    {
        if (it != addr.end() - 1)
        {
            SIZE_T read_size = 0;
            remote_ptr_t next = 0;
            BOOL ret = ReadProcessMemory(this->handle, reinterpret_cast<const void *>(offset + *it), &next, sizeof(next), &read_size);
            if (ret == 0 || sizeof(next) != read_size)
                return std::array<T, size>{T()};
            offset = static_cast<uintptr_t>(next);
        }
        else
        {
            SIZE_T read_size = 0;
            BOOL ret = ReadProcessMemory(this->handle, reinterpret_cast<const void *>(offset + *it), &buff, sizeof(buff), &read_size);
            if (ret == 0 || sizeof(buff) != read_size)
                return std::array<T, size>{T()};
        }
    }
    for (size_t i = 0; i < size; i++)
        result[i] = buff[i];

#if (defined _DEBUG) && (defined _PZTK_MEMORY_OUTPUT)
    std::cout << addr_list_to_string(addr) << " --> ";
    for (size_t i = 0; i < size; i++)
        std::cout << std::hex << int(result[i]) << " ";
    std::cout << std::endl;
#endif

    return result;
}

template <typename T, size_t size>
void Process::WriteMemory(std::array<T, size> value, std::initializer_list<uintptr_t> addr)
{
    if (!IsValid())
        return;

    T buff[size] = {0};
    for (size_t i = 0; i < size; i++)
        buff[i] = value[i];
    uintptr_t offset = 0;
    for (auto it = addr.begin(); it != addr.end(); it++)
    {
        if (it != addr.end() - 1)
        {
            SIZE_T read_size = 0;
            remote_ptr_t next = 0;
            BOOL ret = ReadProcessMemory(this->handle, reinterpret_cast<const void *>(offset + *it), &next, sizeof(next), &read_size);
            if (ret == 0 || sizeof(next) != read_size)
                return;
            offset = static_cast<uintptr_t>(next);
        }
        else
        {
            SIZE_T write_size = 0;
            BOOL ret = WriteProcessMemory(this->handle, reinterpret_cast<void *>(offset + *it), &buff, sizeof(buff), &write_size);
            if (ret == 0 || sizeof(buff) != write_size)
                return;
        }
    }

#if (defined _DEBUG) && (defined _PZTK_MEMORY_OUTPUT)
    std::cout << addr_list_to_string(addr) << " <-- ";
    for (size_t i = 0; i < size; i++)
        std::cout << std::hex << int(value[i]) << " ";
    std::cout << std::endl;
    // if (ReadMemory<T, size>(addr) != value)
    //     std::wcout << L"Memory write error!" << std::endl;
#endif
}

} // namespace Pt
