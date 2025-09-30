
#include "../inc/process.h"

#include <TlHelp32.h>
#include <cwchar>

namespace Pt
{

Process::Process()
{
    this->hwnd = nullptr;
    this->pid = 0;
    this->handle = nullptr;
}

Process::~Process()
{
    if (IsValid())
        CloseHandle(this->handle);
}

bool Process::OpenByWindow(const wchar_t *class_name, const wchar_t *window_name)
{
    if (IsValid())
        CloseHandle(this->handle);

    this->hwnd = nullptr;
    this->pid = 0;
    this->handle = nullptr;

    this->hwnd = FindWindowW(class_name, window_name);
    if (this->hwnd != nullptr)
    {
        GetWindowThreadProcessId(this->hwnd, &(this->pid));
        if (this->pid != 0)
        {
            this->handle = OpenProcess(PROCESS_ALL_ACCESS, false, this->pid);
            if (this->handle != nullptr)
            {
                //
            }
        }
    }

    // assert(PROCESS_ALL_ACCESS == 0x001FFFFF);

#ifdef _DEBUG
    std::wcout << L"查找窗口: " << (class_name == nullptr ? L"nullptr" : class_name)               //
               << L" " << (window_name == nullptr ? L"nullptr" : window_name) << std::endl         //
               << L" -> " << this->hwnd << L" " << this->pid << L" " << this->handle << std::endl; //
#endif

    // 返回的是窗口有没有找到而不是进程有没有打开
    return this->hwnd != nullptr;
}

namespace
{
    struct WindowSearchData
    {
        DWORD pid;
        HWND hwnd;
    };

    BOOL CALLBACK FindWindowForPid(HWND hwnd, LPARAM lParam)
    {
        auto *data = reinterpret_cast<WindowSearchData *>(lParam);
        DWORD window_pid = 0;
        GetWindowThreadProcessId(hwnd, &window_pid);
        if (window_pid == data->pid && GetWindow(hwnd, GW_OWNER) == nullptr)
        {
            wchar_t class_name[64] = {};
            if (GetClassNameW(hwnd, class_name, static_cast<int>(sizeof(class_name) / sizeof(class_name[0]))) && _wcsicmp(class_name, L"MainWindow") == 0)
            {
                data->hwnd = hwnd;
                return FALSE;
            }
        }
        return TRUE;
    }

    BOOL CALLBACK FindAnyTopLevelForPid(HWND hwnd, LPARAM lParam)
    {
        auto *data = reinterpret_cast<WindowSearchData *>(lParam);
        DWORD window_pid = 0;
        GetWindowThreadProcessId(hwnd, &window_pid);
        if (window_pid == data->pid && GetWindow(hwnd, GW_OWNER) == nullptr)
        {
            data->hwnd = hwnd;
            return FALSE;
        }
        return TRUE;
    }
}

bool Process::OpenByProcess(const wchar_t *process_name)
{
    if (IsValid())
        CloseHandle(this->handle);

    this->hwnd = nullptr;
    this->pid = 0;
    this->handle = nullptr;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(PROCESSENTRY32W);

    bool found_process = false;
    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            if (_wcsicmp(entry.szExeFile, process_name) == 0)
            {
                this->pid = entry.th32ProcessID;
                this->handle = OpenProcess(PROCESS_ALL_ACCESS, false, this->pid);
                found_process = (this->handle != nullptr);
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);

    if (!found_process)
        return false;

    WindowSearchData data{this->pid, nullptr};
    EnumWindows(FindWindowForPid, reinterpret_cast<LPARAM>(&data));
    if (!data.hwnd)
        EnumWindows(FindAnyTopLevelForPid, reinterpret_cast<LPARAM>(&data));

    if (!data.hwnd)
    {
        if (this->handle)
        {
            CloseHandle(this->handle);
            this->handle = nullptr;
        }
        this->pid = 0;
        return false;
    }

    this->hwnd = data.hwnd;
    return true;
}

bool Process::IsValid()
{
    if (this->handle == nullptr)
        return false;

    DWORD exit_code;
    BOOL ret = GetExitCodeProcess(this->handle, &exit_code);
    bool valid = (ret != 0 && exit_code == STILL_ACTIVE);

#ifdef _DEBUG
    if (!valid)
        std::wcout << L"当前进程不可用." << std::endl;
#endif

    return valid;
}

} // namespace Pt
