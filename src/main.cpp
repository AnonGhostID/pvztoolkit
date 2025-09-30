
#include <Windows.h>

#include <shellapi.h>
#include <shlwapi.h>

#include <cassert>
#include <ctime>
#include <iostream>
#include <random>
#include <string>

#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/x.H>

#include "../inc/toolkit.h"
#include "../inc/utils.h"

#define IDI_ICON 1001

// Requires compiler to support C++17
static_assert(_MSC_VER >= 1916);

// Compiler target platform limited to x86
// Because the target application (Plants vs Zombies 1 PC version) is x86
// The trainer does not need x64 advantages, and would only increase size
// And Visual Studio does not support x64 inline assembly
static_assert(sizeof(void *) == 4);

void window_callback(Fl_Widget *w, void *)
{
    // Pressing Esc does not exit, but restores default window size
    if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
    {
        if (Fl::screen_scale(((Fl_Window *)w)->screen_num()) != ((Pt::Window *)w)->MinScale())
            Fl::screen_scale(((Fl_Window *)w)->screen_num(), ((Pt::Window *)w)->MinScale());
        return;
    }

    ((Pt::Toolkit *)w)->close_all_sub_window();
    ((Pt::Toolkit *)w)->hide();
}

void callback_pvz_check(void *w)
{
    // Periodically check game process status
    bool on = ((Pt::Toolkit *)w)->pvz->GameOn();
    double t = on ? 0.4 : 0.2;
    Fl::repeat_timeout(t, callback_pvz_check, w);

    if (IsDebuggerPresent())
        exit(-42);
}

/// main ///

Fl_Font ui_font = FL_FREE_FONT + 1; // UI Chinese
Fl_Font ms_font = FL_FREE_FONT + 2; // Monospace English

int main(int argc, char **argv)
{
#ifdef _DEBUG
    system("chcp 65001"); // Debug output Chinese
#endif

#ifdef _DEBUG
    for (int i = 0; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);
#endif

    if (argc == 0)
        return -0;

    if (argc == 4)
    {
        std::string m = argv[1];
        std::string file = argv[2];
        std::string dir = argv[3];

        Pt::PAK pak;
        if (m == "/U")
            return pak.Unpack(file, dir);
        else if (m == "/P")
            return pak.Pack(dir, file);
        else
            return 0xF7;
    }

    // UI background color
    Fl::background(243, 243, 243);

    // UI font
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
    Fl::set_font(ui_font, "Microsoft YaHei");
    Fl::set_font(ms_font, "Consolas");
#else
    TCHAR fontsPath[MAX_PATH];
    SHGetSpecialFolderPathW(NULL, fontsPath, CSIDL_FONTS, FALSE);
    std::wstring fonts_path = fontsPath;
    if (PathFileExistsW((fonts_path + L"\\msyh.ttc").c_str()) == TRUE || //
        PathFileExistsW((fonts_path + L"\\msyh.ttf").c_str()) == TRUE)
        Fl::set_font(ui_font, "Microsoft YaHei");
    else
        Fl::set_font(ui_font, "SimSun");
    if (PathFileExistsW((fonts_path + L"\\consola.ttf").c_str()) == TRUE)
        Fl::set_font(ms_font, "Consolas");
    else
        Fl::set_font(ms_font, "Courier New");
#endif

    // Set dialog properties
#ifdef _PTK_CHINESE_UI
    fl_message_font(ui_font, 13);
#else
    fl_message_font(ms_font, 13);
#endif
    fl_message_hotspot(1);

#ifdef _PTK_CHINESE_UI
    fl_cancel = "Cancel";
    fl_close = "Close";
    fl_no = "No";
    fl_ok = "OK";
    fl_yes = "Yes";
    Fl_Input::copy_menu_text = "Copy";
    Fl_Input::cut_menu_text = "Cut";
    Fl_Input::paste_menu_text = "Paste";
#endif

    // Set tooltip style
    Fl_Tooltip::delay(0.1f);
    Fl_Tooltip::hoverdelay(0.1f);
    Fl_Tooltip::hidedelay(5.0f);
    Fl_Tooltip::color(FL_WHITE);
    Fl_Tooltip::textcolor(FL_BLACK);
    Fl_Tooltip::font(ui_font);
    Fl_Tooltip::size(12);
    Fl_Tooltip::margin_width(5);
    Fl_Tooltip::margin_height(5);
    Fl_Tooltip::wrap_width(400);

    // Initialize random seed
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Enable thread lock mechanism on first call
    Fl::lock();

    // Splash screen

    bool show_splash = true;
    double dt = 1.2f; // Minimum display time

    Fl_Window splash(400 + 2, 225 + 2, "");
    splash.begin();
    Fl_Box box(1, 1, 400, 225, nullptr);
    splash.end();

    splash.color(FL_GREEN);
    splash.border(false);
    splash.set_non_modal();

    Fl_JPEG_Image img_jpeg("splash.jpg");
    Fl_PNG_Image img_png("splash.png");
    Fl_BMP_Image img_bmp("splash.bmp");

    if (img_jpeg.fail() == 0)
        box.image(img_jpeg);
    else if (img_png.fail() == 0)
    {
        box.image(img_png);
        if (Pt::VerifyFileHash(L"splash.png", "8dd46f92e0fa175513278ff208147fbf"))
            dt = 0.3f;
    }
    else if (img_bmp.fail() == 0)
        box.image(img_bmp);
    else
        show_splash = false;

    if (box.image())
    {
        int w = box.image()->w();
        int h = box.image()->h();
        box.size(w, h);
        splash.size(w + 2, h + 2);
    }

    if (show_splash)
    {
        splash.position((Fl::w() - splash.w()) / 2, (Fl::h() - splash.h()) / 2);
        splash.show();
        splash.wait_for_expose();
    }

    clock_t start = clock(); // Start timing

    // System requirements
    // Special modifications for Windows 2000 and XP compilation
    // On higher version systems, refuse to run to avoid potential problems
#if _WIN32_WINNT < _WIN32_WINNT_VISTA
    if (IsWindowsVistaOrGreater())
    {
        fl_message_title("PvZ Toolkit");
#ifdef _PTK_CHINESE_UI
        fl_alert("The operating system in use is not supported!\n"
                 "Requires Windows 2000 or XP to run。");
#else
        fl_alert("This operating system is not supported!\n"
                 "Windows 2000 or XP is required to run.");
#endif
        return -10;
    }
#endif

    // Anti-tamper Detection
    // This feature is designed for the original author lmintlcx.
    // If you want to make a customized version, delete this code.
#ifdef _PTK_SIGNATURE_CHECK
    wchar_t exePath[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    if (!Pt::VerifySignature(exePath, "\x21\x13\x67\x0f\x3b\x6c\x60\xaf\x42\x50\x7f\x07\xd3\x97\xbc\xd6"))
    {
#ifdef _PTK_CHINESE_UI
        fl_message_title("PvZ Toolkit Anti-tamper Detection");
        if (fl_choice("This program may be infected with a virus, please re-download from official channels!", //
#else
        fl_message_title("PvZ Toolkit Tamper-proof Detection");
        if (fl_choice("This program may have been infected with a virus, \n"
                      "please download it again from the official website!", //
#endif
                      "No", "Yes", 0) == 1)
            ShellExecuteW(nullptr, L"open", L"https://pvz.lmintlcx.com/toolkit/", //
                          nullptr, nullptr, SW_SHOWNORMAL);
        return -1;
    }
#endif

    // Run only single instance
    HANDLE m = CreateMutexW(nullptr, true, L"PvZ Toolkit");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return -2;

#ifdef _DEBUG
    std::wcout << L"Startup time (ms): " << (clock() - start) << std::endl;
#endif

    // Hide splash screen
    if (show_splash)
    {
        while ((clock() - start) / (double)CLOCKS_PER_SEC < dt)
            Fl::check();
        splash.hide();
        Fl::check();
    }

    // Show main window

    Pt::Toolkit pvztoolkit(0, 0, "");
    pvztoolkit.callback(window_callback);

    pvztoolkit.show(1, argv); // argc -> 1
    pvztoolkit.wait_for_expose();
    pvztoolkit.pvz->FindPvZ();
    SetForegroundWindow(fl_xid(&pvztoolkit));

    // Title bar icon
    HICON hIcon = LoadIconW(fl_display, MAKEINTRESOURCEW(IDI_ICON));
    SendMessageW(fl_xid(&pvztoolkit), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessageW(fl_xid(&pvztoolkit), WM_SETICON, ICON_BIG, (LPARAM)hIcon);

#ifdef _DEBUG
    // Avoid frequent output during debugging
#else
    Fl::add_timeout(0.01, callback_pvz_check, &pvztoolkit);
#endif

    int ret = Fl::run();

    // Release after main loop ends
    ReleaseMutex(m);
    CloseHandle(m);

    return ret;
}
