//
// Entry point for VISE application
//
// Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
// Date: 12 Nov. 2019
//

#ifdef _WIN32
#include "vise_version.h"
#include "vise_util.h"
#include "http_server.h"

#include <boost/filesystem.hpp>
#include <Magick++.h>

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
std::map<std::string, std::string> vise_conf;
std::string vise_access_info_str;

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
    const boost::filesystem::path visehome = vise::vise_home();
    boost::filesystem::path vise_settings = visehome / "vise_settings.txt";

    if (!boost::filesystem::exists(vise_settings)) {
        // use default configuration for VISE
        boost::filesystem::path vise_store = visehome / "store";
        boost::filesystem::path www_store = visehome / "www";

        if (!boost::filesystem::exists(visehome)) {
            boost::filesystem::create_directories(visehome);
            boost::filesystem::create_directory(vise_store);
            boost::filesystem::create_directory(www_store);
        }

        ::vise_conf["vise_store"] = vise_store.string();
        ::vise_conf["www_store"] = www_store.string();
        ::vise_conf["address"] = "localhost";
        ::vise_conf["port"] = "9670";
        ::vise_conf["nthread"] = "4";
        vise::configuration_save(::vise_conf, vise_settings.string());
    }
    // load VISE configuration
    vise::configuration_load(vise_settings.string(), ::vise_conf);

    char exec_path[MAX_PATH];
    GetModuleFileName(hInstance, exec_path, MAX_PATH);
    boost::filesystem::path exec_dir(exec_path);
    //std::cout << "\nMagick::InitializeMagick = " << exec_dir.parent_path().string().c_str() << std::endl;
    Magick::InitializeMagick(exec_dir.parent_path().string().c_str());
    //cout << "\nImageMagick Magick++ quantum depth = " << MAGICKCORE_QUANTUM_DEPTH << flush;
    //Magick::InitializeMagick("");

    std::ostringstream ss;
    ss << VISE_FULLNAME << " (" << VISE_NAME << ") "
        << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH;
    vise_name_version_str = ss.str();

    ss.clear();
    ss.str("");
    ss << "To use the VISE application, visit http://" << ::vise_conf.at("address");
    ss << ":" << ::vise_conf.at("port") << "/ in a web browser.";
    vise_access_info_str = ss.str();

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

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T(VISE_FULLNAME),
            NULL);

        return 1;
    }

    // Store instance handle in our global variable
    hInst = hInstance;
    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
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
    vise::http_server server(::vise_conf);
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

    switch (message)
    {
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
        hFont = (HFONT)GetStockObject(ANSI_VAR_FONT);
        hOldFont = (HFONT)SelectObject(hdc, hFont);
        SIZE tdim;
        RECT trect;

        GetTextExtentPoint32(hdc, vise_name_version_str.c_str(), vise_name_version_str.size(), &tdim);
        trect.left = (win_size.right - win_size.left)/2 - tdim.cx/2;
        trect.right = (win_size.right - win_size.left) / 2 + tdim.cx / 2;
        trect.top = bitmap.bmHeight;
        trect.bottom = win_size.bottom;
        DrawText(hdc, vise_name_version_str.c_str(), vise_name_version_str.size(), &trect, 0);

        // Show URL for VISE web based interface
        GetTextExtentPoint32(hdc, vise_access_info_str.c_str(), vise_access_info_str.size(), &tdim);
        trect.left = (win_size.right - win_size.left) / 2 - tdim.cx / 2;
        trect.right = (win_size.right - win_size.left) / 2 + tdim.cx / 2;
        trect.top = 2*bitmap.bmHeight;
        trect.bottom = win_size.bottom;
        DrawText(hdc, vise_access_info_str.c_str(), vise_access_info_str.size(), &trect, 0);

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        DeleteObject(h_vise_logo);
        PostQuitMessage(0);
        break;
    case WM_CREATE:
    {
        //h_vise_logo = (HBITMAP) LoadImage(hInst, vise_logo_fn, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        h_vise_logo = (HBITMAP)LoadImage(NULL, vise_logo_fn, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if (h_vise_logo == NULL) {
            std::cout << "failed to load resource: " << vise_logo_fn << std::endl;
            MessageBox(hWnd, "Could not load vise_logo.", "Error", MB_OK | MB_ICONEXCLAMATION);
        }

        std::ostringstream ss;
        ss << "http://" << ::vise_conf.at("address") << ":" << ::vise_conf.at("port") << "/";
        std::string vise_url(ss.str());
        ShellExecute(hWnd, _T("open"), _T(vise_url.c_str()), 0, 0, SW_SHOW);

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
  const boost::filesystem::path visehome = vise::vise_home();

  boost::filesystem::path vise_settings_fn = visehome / "vise_settings.txt";
  std::map<std::string, std::string> vise_settings;
  std::cout << "Using VISE application configuration from: " << vise_settings_fn << std::endl;
  if(!boost::filesystem::exists(vise_settings_fn)) {
    // use default configuration for VISE
    boost::filesystem::path vise_store = visehome / "store";
    boost::filesystem::path www_store = visehome / "www";

    if(!boost::filesystem::exists(visehome)) {
      boost::filesystem::create_directories(visehome);
      boost::filesystem::create_directory(vise_store);
      boost::filesystem::create_directory(www_store);
    }

    vise_settings["vise_store"] = vise_store.string();
    vise_settings["www_store"] = www_store.string();
    vise_settings["address"] = "0.0.0.0";
    vise_settings["port"] = "9670";
    vise_settings["nthread"] = "0";
    // comments
    vise_settings["# www_store : "] = "HTML, Javascript, CSS, static images and other assets of VISE web application are stored in this path.";
    vise_settings["# vise_store : "] = "all files (images, index, configuration, etc) associated with a project files are stored in this path.";
    vise_settings["# nthread : "] = "0 will use all the available threads; nthread > 0 will use the specified number of threads; nthread < 0 will only use (MAX_THREADS-nthread) threads.";

    vise::configuration_save(vise_settings, vise_settings_fn.string());
  }

  if(argc == 1) { // no command line arguments -> run vise server
    // load VISE configuration
    vise::configuration_load(vise_settings_fn.string(), vise_settings);

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
        std::map<std::string, std::string> vise_settings;
        std::cout << "using VISE settings from "
                  << vise_settings_fn << std::endl;
        vise::configuration_load(vise_settings_fn.string(), vise_settings);

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
