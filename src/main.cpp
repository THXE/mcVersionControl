#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QDebug>
#include <windows.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")

#include "MainWindow.h"
#include "Utils/ThemeManager.h"

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif

typedef HRESULT (WINAPI *pDwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND*);

static bool enableBlur(HWND hwnd)
{
    HMODULE hDwm = LoadLibraryW(L"dwmapi.dll");
    if (!hDwm) return false;

    auto pfn = (pDwmEnableBlurBehindWindow)GetProcAddress(hDwm, "DwmEnableBlurBehindWindow");
    if (!pfn) { FreeLibrary(hDwm); return false; }

    DWM_BLURBEHIND bb = {};
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = NULL;
    bb.fTransitionOnMaximized = FALSE;

    HRESULT hr = pfn(hwnd, &bb);
    FreeLibrary(hDwm);
    return SUCCEEDED(hr);
}

void setWindowCornersRounded(HWND hwnd, bool rounded)
{
    DWORD cornerPref = rounded ? DWMWCP_ROUND : DWMWCP_DONOTROUND;
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
                          &cornerPref, sizeof(cornerPref));
}

static void setupWindowFrame(HWND hwnd)
{
    // 1. Windows 11 rounded corners (default)
    setWindowCornersRounded(hwnd, true);

    // 2. Acrylic blur behind
    enableBlur(hwnd);

    // 3. Add back WS_THICKFRAME, WS_MAXIMIZEBOX, and WS_CAPTION
    //    for resize + snap + system frame recognition.
    //    WM_NCCALCSIZE will still remove the NC area so the custom
    //    titlebar renders flush, but the styles let Windows know
    //    this is a standard resizable window for snap functionality.
    LONG style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);

    // 4. Extend client area into the frame (margins: 0,0,0,1 keeps
    //    a thin bottom edge for system resize recognition)
    MARGINS margins = {0, 0, 0, 1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // 5. Apply style change and recalc frame
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("MCVersionControl");
    app.setOrganizationName("MCVC");
    app.setApplicationVersion("1.0.0");

    app.setWindowIcon(QIcon(":/icon.png"));

    // Force-apply system theme (palette + QSS) on first launch
    ThemeManager::instance().forceApplyTheme(ThemeManager::systemTheme());

    MainWindow mainWindow;
    mainWindow.setWindowOpacity(0.0); // start transparent for fade-in
    mainWindow.show();

    // Windows 11 frame setup (must happen after show())
    HWND hwnd = reinterpret_cast<HWND>(mainWindow.winId());
    setupWindowFrame(hwnd);

    return app.exec();
}
