//
// Entry point for VISE application
//
// Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
// Date: 12 Nov. 2019
//

#include "vise_version.h"
#include "vise_util.h"
#include "http_server.h"
#include "project_manager.h"

#include <string>
#include <map>
#include <iostream>

#include <boost/filesystem.hpp>
#include <Magick++.h>

#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <shellapi.h>

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T(VISE_NAME);

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T(VISE_FULLNAME);

HINSTANCE hInst;

static TCHAR vise_logo_fn[] = _T("vise_logo.bmp");
static TCHAR vise_icon_fn[] = _T("vise_icon.ico");

HBITMAP h_vise_logo = NULL;
HICON h_vise_icon = NULL;

std::string vise_name_version_str;
std::string vise_software_page_str;
std::map<std::string, std::string> vise_settings;
std::string vise_info_1;
std::string vise_info_2;
std::string vise_info_3a;
std::string vise_info_3b;
std::string vise_ui_url;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
  /*
  // @todo: remove for release
  // for debugging Windows 10 builds
  FILE* fp;
  AttachConsole(ATTACH_PARENT_PROCESS);
  AttachConsole(GetCurrentProcessId());
  freopen_s(&fp, "CONIN$", "r", stdin);
  freopen_s(&fp, "CONOUT$", "w", stdout);
  freopen_s(&fp, "CONOUT$", "w", stderr);
  */

  vise::init_vise_settings(::vise_settings);
  char exec_path[MAX_PATH];
  GetModuleFileName(hInstance, exec_path, MAX_PATH);
  boost::filesystem::path exec_dir(exec_path);
  Magick::InitializeMagick(exec_dir.parent_path().string().c_str());

  std::ostringstream ss;
  ss << VISE_FULLNAME << " (" << VISE_NAME << ") "
     << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH;
  vise_name_version_str = ss.str();

  ss.clear();
  ss.str("");
  ss << "http://" << ::vise_settings.at("http-address") << ":" << ::vise_settings.at("http-port") << "/";
  vise_ui_url = ss.str();

  ss.clear();
  ss.str("");
  ss << "VISE server is running now. Close this application window to shutdown the VISE server.";
  vise_info_1 = ss.str();

  ss.clear();
  ss.str("");
  ss << "Open Visual Search Engine";
  vise_info_2 = ss.str();

  ss.clear();
  ss.str("");
  ss << "VISE is listening for requests on " << vise_ui_url;
  vise_info_3a = ss.str();
  ss.clear();
  ss.str("");
  ss << "VISE Projects are stored in " << ::vise_settings.at("vise-project-dir") << " ";
  vise_info_3b = ss.str();

  // VISE software page URL
  ss.clear();
  ss.str("");
  ss << "https://www.robots.ox.ac.uk/~vgg/software/vise/";
  vise_software_page_str = ss.str();

  h_vise_icon = (HICON)LoadImage(hInstance, vise_icon_fn, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  //wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
  wcex.hIcon = (HICON)LoadImage(NULL, "vise_icon_32x32.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = szWindowClass;
  //wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
  wcex.hIconSm = (HICON)LoadImage(NULL, "vise_icon_16x16.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);

  if (!RegisterClassEx(&wcex)) {
    MessageBox(NULL,
               _T("Call to RegisterClassEx failed!"),
               _T(VISE_FULLNAME),
               NULL);

    return 1;
  }

  // Store instance handle in our global variable
  hInst = hInstance;
  HWND hWnd = CreateWindow(szWindowClass,
                           szTitle,
                           WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           800, 600,
                           NULL,
                           NULL,
                           hInstance,
                           NULL
                           );

  if (!hWnd) {
    MessageBox(NULL,
               _T("Call to CreateWindow failed!"),
               _T(VISE_FULLNAME),
               NULL);
    return 1;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  // start http server to serve contents in a web browser
  vise::project_manager manager(::vise_settings);
  vise::http_server server(::vise_settings, manager);
  std::thread server_thread(&vise::http_server::start, &server);

  // Main message loop:
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

  return (int)msg.wParam;
  return 0;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    static HWND hOpenBrowserButton;

    switch (message)
    {
    case WM_CLOSE:
      PostQuitMessage(0);
      break;
    case WM_COMMAND:
      {
        ShellExecute(hWnd, _T("open"), _T(vise_ui_url.c_str()), 0, 0, SW_SHOW);
        break;
      }
    case WM_PAINT:
      {
        BITMAP          bitmap;
        HDC             hdcMem;
        HGDIOBJ         oldBitmap;
        int             dest_x;

        hdc = BeginPaint(hWnd, &ps);

        hdcMem = CreateCompatibleDC(hdc);
        oldBitmap = SelectObject(hdcMem, h_vise_logo);

        RECT win_size;
        GetWindowRect(hWnd, &win_size);
        GetObject(h_vise_logo, sizeof(bitmap), &bitmap);
        dest_x = (win_size.right - win_size.left) / 2 - bitmap.bmWidth / 2;

        BitBlt(hdc, dest_x, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);


        // show VISE name and version text
        HFONT hFont, hOldFont;
        hFont = CreateFont(16, 0, 0, 0, FW_DONTCARE,
                           false, false, false, ANSI_CHARSET,
                           OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH,
                           TEXT("ANSI_VAR_FON"));
        //hFont = (HFONT)GetStockObject(SYSTEM_FONT);
        hOldFont = (HFONT)SelectObject(hdc, hFont);
        SIZE tdim;
        RECT trect;

        GetTextExtentPoint32(hdc, vise_name_version_str.c_str(), vise_name_version_str.size(), &tdim);
        trect.left = (win_size.right - win_size.left)/2 - tdim.cx/2;
        trect.right = (win_size.right - win_size.left) / 2 + tdim.cx / 2;
        trect.top = bitmap.bmHeight;
        trect.bottom = win_size.bottom;
        DrawText(hdc, vise_name_version_str.c_str(), vise_name_version_str.size(), &trect, 0);

        // VISE software page
        GetTextExtentPoint32(hdc, vise_software_page_str.c_str(), vise_software_page_str.size(), &tdim);
        trect.left = (win_size.right - win_size.left)/2 - tdim.cx/2;
        trect.right = (win_size.right - win_size.left) / 2 + tdim.cx / 2;
        trect.top = 1.15*bitmap.bmHeight;
        trect.bottom = win_size.bottom;
        DrawText(hdc, vise_software_page_str.c_str(), vise_software_page_str.size(), &trect, 0);

          // VISE info 1
        GetTextExtentPoint32(hdc, vise_info_1.c_str(), vise_info_1.size(), &tdim);
        trect.left = (win_size.right - win_size.left) / 2 - tdim.cx / 2;
        trect.right = (win_size.right - win_size.left) / 2 + tdim.cx / 2;
        trect.top = 2*bitmap.bmHeight;
        trect.bottom = win_size.bottom;
        DrawText(hdc, vise_info_1.c_str(), vise_info_1.size(), &trect, 0);

        // VISE info 3
        GetTextExtentPoint32(hdc, vise_info_3a.c_str(), vise_info_3a.size(), &tdim);
        trect.left = (win_size.right - win_size.left) / 2 - tdim.cx / 2;
        trect.right = (win_size.right - win_size.left) / 2 + tdim.cx / 2;
        trect.top = 2.15*bitmap.bmHeight;
        trect.bottom = win_size.bottom;
        DrawText(hdc, vise_info_3a.c_str(), vise_info_3a.size(), &trect, 0);

        GetTextExtentPoint32(hdc, vise_info_3b.c_str(), vise_info_3b.size(), &tdim);
        trect.left = (win_size.right - win_size.left) / 2 - tdim.cx / 2;
        trect.right = (win_size.right - win_size.left) / 2 + tdim.cx / 2;
        trect.top = 2.3*bitmap.bmHeight;
        trect.bottom = win_size.bottom;
        DrawText(hdc, vise_info_3b.c_str(), vise_info_3b.size(), &trect, 0);

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hWnd, &ps);
        break;
      }
    case WM_DESTROY:
      DeleteObject(h_vise_logo);
      PostQuitMessage(0);
      break;
    case WM_CREATE:
      {
        h_vise_logo = (HBITMAP)LoadImage(NULL, vise_logo_fn, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if (h_vise_logo == NULL) {
          MessageBox(hWnd, "Could not load vise_logo.", "Error", MB_OK | MB_ICONEXCLAMATION);
        }

        // add button to open VISE UI in web browser
        hOpenBrowserButton = CreateWindow( _T("BUTTON"), _T(vise_info_2.c_str()),
                                           WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                           200,
                                           450,
                                           400, 40,
                                           hWnd,
                                           NULL,
                                           (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                                           NULL);
        break;
      }
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
    }

    return 0;
}
#endif
