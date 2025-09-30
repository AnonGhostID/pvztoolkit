#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QIcon>
#include <QPixmap>
#include <QSplashScreen>
#include <QFile>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSettings>

#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>

#include <cassert>
#include <ctime>
#include <iostream>
#include <random>
#include <string>

#include "../inc/toolkit_qt.h"
#include "../inc/utils.h"

#define IDI_ICON 1001

// Compile-time checks
static_assert(_MSC_VER >= 1916);
static_assert(sizeof(void *) == 4);

// UI Mode Selector Dialog
class UIModeSelector : public QDialog
{
    Q_OBJECT

public:
    UIModeSelector(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("PvZ Toolkit - Select UI Mode");
        setFixedSize(400, 250);
        setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
        
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setSpacing(20);
        layout->setContentsMargins(30, 30, 30, 30);
        
        QLabel *titleLabel = new QLabel("Choose Your Interface");
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        layout->addWidget(titleLabel);
        
        QLabel *descLabel = new QLabel("Select which UI you want to use:");
        descLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(descLabel);
        
        QPushButton *modernBtn = new QPushButton("🎨 Modern UI (Qt6)");
        modernBtn->setMinimumHeight(50);
        modernBtn->setStyleSheet("font-size: 14pt; font-weight: bold; background-color: #0d7377; color: white;");
        connect(modernBtn, &QPushButton::clicked, this, [this]() {
            selectedMode = 1;
            accept();
        });
        layout->addWidget(modernBtn);
        
        QPushButton *classicBtn = new QPushButton("📦 Classic UI (FLTK)");
        classicBtn->setMinimumHeight(50);
        classicBtn->setStyleSheet("font-size: 14pt; font-weight: bold; background-color: #555555; color: white;");
        classicBtn->setEnabled(false);
        classicBtn->setToolTip("Classic UI is only available in FLTK build");
        layout->addWidget(classicBtn);
        
        QLabel *noteLabel = new QLabel("Note: This build only supports Modern UI");
        noteLabel->setAlignment(Qt::AlignCenter);
        noteLabel->setStyleSheet("color: gray; font-size: 9pt;");
        layout->addWidget(noteLabel);
    }
    
    int selectedMode = 0; // 0 = none, 1 = modern, 2 = classic
};

void callback_pvz_check(void *w)
{
    Pt::QtToolkit *toolkit = static_cast<Pt::QtToolkit *>(w);
    bool on = toolkit->pvz->GameOn();
    
    // Schedule next check
    QTimer::singleShot(on ? 400 : 200, [w]() {
        callback_pvz_check(w);
    });
    
    if (IsDebuggerPresent())
        exit(-42);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    
    app.setApplicationName("PvZ Toolkit");
    app.setOrganizationName("PvZ Toolkit");
    app.setApplicationVersion(VERSION_NAME);
    
#ifdef _DEBUG
    system("chcp 65001");
    for (int i = 0; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);
#endif
    
    if (argc == 0)
        return -0;
    
    // Command-line PAK operations
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
    
    // Initialize random seed
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Show splash screen
    QPixmap splashPixmap;
    QSplashScreen *splash = nullptr;
    double dt = 1.2;
    
    if (QFile::exists("splash.png"))
    {
        splashPixmap.load("splash.png");
        splash = new QSplashScreen(splashPixmap);
        splash->show();
        app.processEvents();
        
        if (Pt::VerifyFileHash(L"splash.png", "8dd46f92e0fa175513278ff208147fbf"))
            dt = 0.3;
    }
    else if (QFile::exists("splash.jpg"))
    {
        splashPixmap.load("splash.jpg");
        splash = new QSplashScreen(splashPixmap);
        splash->show();
        app.processEvents();
    }
    else if (QFile::exists("splash.bmp"))
    {
        splashPixmap.load("splash.bmp");
        splash = new QSplashScreen(splashPixmap);
        splash->show();
        app.processEvents();
    }
    
    clock_t start = clock();
    
    // Single instance check
    HANDLE mutex = CreateMutexW(nullptr, true, L"PvZ Toolkit Qt");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        QMessageBox::warning(nullptr, "PvZ Toolkit", 
            "Another instance is already running!");
        return -2;
    }
    
    // Anti-tamper check (optional)
#ifdef _PTK_SIGNATURE_CHECK
    wchar_t exePath[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    if (!Pt::VerifySignature(exePath, "\x21\x13\x67\x0f\x3b\x6c\x60\xaf\x42\x50\x7f\x07\xd3\x97\xbc\xd6"))
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            nullptr, "PvZ Toolkit Tamper-proof Detection",
            "This program may have been infected with a virus,\n"
            "please download it again from the official website!\n\n"
            "Open website?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes)
            ShellExecuteW(nullptr, L"open", L"https://pvz.lmintlcx.com/toolkit/",
                         nullptr, nullptr, SW_SHOWNORMAL);
        return -1;
    }
#endif
    
#ifdef _DEBUG
    std::wcout << L"启动用时(毫秒): " << (clock() - start) << std::endl;
#endif
    
    // Wait for minimum splash screen time
    if (splash)
    {
        while ((clock() - start) / (double)CLOCKS_PER_SEC < dt)
            app.processEvents();
        splash->finish(nullptr);
        delete splash;
    }
    
    // Show UI mode selector (in this Qt-only build, auto-select Modern)
    // In a dual-build, this would show both options
    QSettings settings("PvZToolkit", "UI");
    int uiMode = settings.value("uiMode", 0).toInt();
    
    if (uiMode == 0)
    {
        UIModeSelector selector;
        if (selector.exec() == QDialog::Accepted)
        {
            uiMode = selector.selectedMode;
            settings.setValue("uiMode", uiMode);
        }
        else
        {
            return 0; // User cancelled
        }
    }
    
    // Create main window
    Pt::QtToolkit toolkit;
    toolkit.show();
    
    // Find PvZ game
    QTimer::singleShot(100, [&toolkit]() {
        toolkit.pvz->FindPvZ();
    });
    
    // Set window icon
    HICON hIcon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_ICON));
    if (hIcon)
    {
        toolkit.setWindowIcon(QIcon(QPixmap::fromWinHICON(hIcon)));
        DestroyIcon(hIcon);
    }
    
    // Start PvZ check timer
    QTimer::singleShot(10, [&toolkit]() {
        callback_pvz_check(&toolkit);
    });
    
    int ret = app.exec();
    
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    
    return ret;
}
