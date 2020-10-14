//
// Entry point for VISE application
//
// Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
// Date: 12 Nov. 2019
//

#include "vise_version.h"
#include "vise_util.h"
#include "http_server.h"

#include <string>
#include <map>

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
std::map<std::string, std::string> vise_settings;
std::string vise_info_1;
std::string vise_info_2;
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
  std::cout << VISE_FULLNAME << " (" << VISE_NAME << ") "
			<< VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH
			<< std::endl;
  vise::init_vise_settings(::vise_settings);
  if(__argc == 1) { // no command line arguments -> run vise server
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
    ss << "VISE server is running now. To shutdown the VISE server, close this application window.";
    vise_info_1 = ss.str();

    ss.clear();
    ss.str("");
    ss << "http://" << ::vise_settings.at("address") << ":" << ::vise_settings.at("port") << "/";
    vise_ui_url = ss.str();

    ss.clear();
    ss.str("");
    ss << "Click here to visit VISE application interface web page available at " << vise_ui_url;
    vise_info_2 = ss.str();

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
    std::cout << "Initializing http_server ..." << std::endl;
    vise::project_manager manager(vise_settings);
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
  }
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
        //std::cout << "win: " << win_size.right << "," << win_size.left << std::endl;

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

        // VISE info 1
        GetTextExtentPoint32(hdc, vise_info_1.c_str(), vise_info_1.size(), &tdim);
        trect.left = (win_size.right - win_size.left) / 2 - tdim.cx / 2;
        trect.right = (win_size.right - win_size.left) / 2 + tdim.cx / 2;
        trect.top = 2*bitmap.bmHeight;
        trect.bottom = win_size.bottom;
        DrawText(hdc, vise_info_1.c_str(), vise_info_1.size(), &trect, 0);

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
          std::cout << "failed to load resource: " << vise_logo_fn << std::endl;
          MessageBox(hWnd, "Could not load vise_logo.", "Error", MB_OK | MB_ICONEXCLAMATION);
        }

        // add button to open VISE UI in web browser
        hOpenBrowserButton = CreateWindow( _T("BUTTON"), _T(vise_info_2.c_str()),
                                           WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                           80,
                                           400,
                                           640, 40,
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

#ifdef __linux__
//#ifdef _WIN32
#include "vise_version.h"
#include "vise_util.h"
#include "http_server.h"

#include <boost/filesystem.hpp>
#include <Magick++.h>

#include <iostream>
#include <memory>
#include <cstdlib>

int main(int argc, char **argv) {
  std::cout << VISE_FULLNAME << " (" << VISE_NAME << ") "
            << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH
            << std::endl;
  std::map<std::string, std::string> vise_settings;
  vise::init_vise_settings(vise_settings);

  if(argc == 1) { // no command line arguments -> run vise server
    boost::filesystem::path exec_dir(argv[0]);
    //std::cout << "\nMagick::InitializeMagick = " << exec_dir.parent_path().string().c_str() << std::endl;
    Magick::InitializeMagick(exec_dir.parent_path().string().c_str());
    std::cout << "\nImageMagick Magick++ quantum depth = " << MAGICKCORE_QUANTUM_DEPTH << std::endl;

    // start http server to serve contents in a web browser
    std::cout << "Initializing http_server ..." << std::endl;
    vise::project_manager manager(vise_settings);
    vise::http_server server(vise_settings, manager);
    server.start();
    return 0;
  }

  if(argc > 1) {
    std::string cmd(argv[1]);
    if(cmd == "create-project") {
      if(argc == 4) {
        std::string pname(argv[2]);
        boost::filesystem::path conf_fn(argv[3]);
        if( !boost::filesystem::exists(conf_fn) ) {
          std::cout << "project configuration file not found: "
                    << conf_fn << std::endl;
          return 1;
        }
        vise::project new_project(pname, conf_fn.string());
        bool success;
        std::string message;
        bool block_until_done = true;
        new_project.index_create(success, message, block_until_done);
        std::cout << message << std::endl;
      } else {
        std::cout << "Usage: " << argv[0]
                  << " " << argv[1] << " PROJECT_NAME CONFIG_FILENAME" << std::endl;
        return 1;
      }
      return 0;
    }

    if(cmd == "create-visual-vocabulary") {
      if(argc == 4) {
        std::string pname(argv[2]);
        boost::filesystem::path conf_fn(argv[3]);
        if( !boost::filesystem::exists(conf_fn) ) {
          std::cout << "project configuration file not found: "
                    << conf_fn << std::endl;
          return 1;
        }
        boost::filesystem::path data_dir = conf_fn.parent_path();
        std::string placeholder("dummy file to only perform traindesc, cluster, trainassign, trainhamm and avoid indexing stage");
        vise::file_save(data_dir / "index_dset.bin", placeholder);
        vise::file_save(data_dir / "index_fidx.bin", placeholder);
        vise::file_save(data_dir / "index_iidx.bin", placeholder);

        vise::project new_project(pname, conf_fn.string());
        bool success;
        std::string message;
        bool block_until_done = true;
        new_project.index_create(success, message, block_until_done);
        std::cout << message << std::endl;
      } else {
        std::cout << "Usage: " << argv[0]
                  << " " << argv[1] << " PROJECT_NAME CONFIG_FILENAME" << std::endl;
        return 1;
      }
      return 0;
    }

    if(cmd == "serve-project") {
      if(argc == 4) {
        std::string pname(argv[2]);
        boost::filesystem::path project_conf_fn(argv[3]);
        if( !boost::filesystem::exists(project_conf_fn) ) {
          std::cout << "project configuration file not found: "
                    << project_conf_fn << std::endl;
          return 1;
        }
        std::map<std::string, std::string> pname_pconf_fn_map;
        pname_pconf_fn_map[pname] = project_conf_fn.string();
        vise::project_manager manager(vise_settings);
        manager.serve_only(pname_pconf_fn_map);
        vise::http_server server(vise_settings, manager);
        server.start();
      } else {
        std::cout << "Usage: " << argv[0]
                  << " " << argv[1] << " PROJECT_NAME CONFIG_FILENAME" << std::endl;
        return 1;
      }
      return 0;
    }

    std::cout << "unknown command: " << argv[1] << std::endl;
    return 0;
  }
}
#endif // end of __linux__
