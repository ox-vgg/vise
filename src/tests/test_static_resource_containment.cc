#include "project_manager.h"
#include "vise_util.h"
#include <Magick++.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = boost::filesystem;
namespace {
const std::string sentinel = "PRIVATE-OUTSIDE-STATIC-STORE";
void require(bool condition, const std::string &message) {
  if(!condition) throw std::runtime_error(message);
}
void write(const fs::path &path, const std::string &contents) {
  fs::create_directories(path.parent_path());
  std::ofstream stream(path.string().c_str(), std::ios::binary);
  stream << contents;
  require(bool(stream), "create test resource");
}
std::string encoded(const std::string &value) {
  const char *hex = "0123456789abcdef";
  std::string result;
  for(unsigned char byte : value) {
    result += '%'; result += hex[byte >> 4]; result += hex[byte & 15];
  }
  return result;
}
vise::http_response get(vise::project_manager &manager, const std::string &path,
                        const std::string &name_space = "/api/") {
  vise::http_request request;
  request.parse("GET " + name_space + path + " HTTP/1.1\r\n\r\n");
  vise::http_response response;
  manager.process_http_request(request, response);
  return response;
}
vise::http_response project_request(vise::project_manager &manager, const std::string &method,
                                   const std::string &path) {
  vise::http_request request;
  request.parse(method + " /api/" + path + " HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
  vise::http_response response;
  manager.process_http_request(request, response);
  return response;
}
void deny(vise::project_manager &manager, const std::string &path) {
  const auto response = get(manager, path);
  require(response.d_status_code == 404 && response.d_payload.find(sentinel) == std::string::npos,
          "escaped resource must be unavailable: " + path + " (status=" +
          std::to_string(response.d_status_code) + ", outside sentinel=" +
          (response.d_payload.find(sentinel) == std::string::npos ? "absent" : "present") + ")");
}
void allow(vise::project_manager &manager, const std::string &path, const std::string &payload) {
  const auto response = get(manager, path);
  require(response.d_status_code == 200 && response.d_payload == payload, "normal resource changed: " + path);
}
void symlink(const fs::path &target, const fs::path &link, bool directory = false) {
  boost::system::error_code error;
  if(directory) fs::create_directory_symlink(target, link, error);
  else fs::create_symlink(target, link, error);
  require(!error, "symlink fixture requires native symlink permission: " + error.message());
}
}

int main(int argc, char **argv) {
  const fs::path root = fs::temp_directory_path() / fs::unique_path("vise-static-path-%%%%-%%%%");
  try {
    Magick::InitializeMagick(argv[0]);
    const fs::path www = root / "www", projects = root / "projects", gallery = projects / "gallery";
    write(root / "private.txt", sentinel);
    write(root / "www-sibling/private.txt", sentinel);
    write(www / "asset.js", "global asset");
    write(www / "nested/asset space.js", "nested global asset");
    write(root / "app/private.js", sentinel);
    write(projects / "app/private.js", sentinel);
    write(gallery / "private.txt", sentinel);
    for(const char *store : {"app", "image", "image_src", "image_small"}) {
      write(gallery / store / "asset.js", std::string("public ") + store);
      symlink(gallery / "private.txt", gallery / store / "escape.js");
      symlink(gallery / store / "asset.js", gallery / store / "inside.js");
      symlink(root / "www-sibling", gallery / store / "escape-directory", true);
    }
    symlink(root / "private.txt", www / "escape.js");
    symlink(www / "asset.js", www / "inside.js");
    symlink(root / "www-sibling", www / "escape-directory", true);
    fs::create_directories(projects / "fallback");
    fs::create_directories(projects / "coverless/image");
    symlink(root / "private.txt", projects / "coverless/image/escape.jpg");
    write(projects / "custom.project/app/asset.js", "dotted project asset");
#ifdef _WIN32
    const std::string unicode_name = fs::path(L"\u6d4b\u8bd5").string();
#else
    const std::string unicode_name = "\xe6\xb5\x8b\xe8\xaf\x95";
#endif
    write(projects / unicode_name / "app/asset.js", "Unicode project asset");

    std::map<std::string, std::string> conf;
    vise::init_default_vise_settings(conf);
    conf["http-namespace"] = "/api/";
    conf["http-www-dir"] = www.string();
    conf["vise-project-dir"] = projects.string();
    conf["vise-asset-dir"] = (root / "assets").string();
    {
      vise::project_manager manager(conf);
      // Per-asset containment must not first select the project-store parent.
      deny(manager, "../app/private.js");
      deny(manager, "./app/private.js");
      for(const char *project : {"", "%2e", "%2e%2e", "%2f", "%5c", "bad%2fname",
                                 "bad%5cname", "C%3a", "bad%00name", "bad%1fname", "bad%7fname", "%2g"}) {
        for(const char *method : {"GET", "POST", "PUT", "DELETE"}) {
          const auto response = project_request(manager, method, std::string(project) + "/app/private.js");
          require(response.d_status_code == 404 && response.d_payload.find(sentinel) == std::string::npos,
                  "unsafe project selector must be rejected before routing");
        }
      }
#ifdef _WIN32
      for(const char *project : {"CON", "NUL.jpg", "COM1", "LPT9.ext", "bad.", "bad%20", "bad|name"})
        deny(manager, std::string(project) + "/app/private.js");
#endif
      require(!fs::exists(root / "data") && !fs::exists(root / "image") &&
              !fs::exists(root / "image_src") && !fs::exists(root / "tmp") &&
              !fs::exists(projects / "data") && !fs::exists(projects / "image"),
              "invalid project names must not create parent project directories");
      require(!manager.project_is_loaded("..") && !manager.project_is_loaded("."),
              "invalid project names must not be loaded");
      allow(manager, "custom.project/app/asset.js", "dotted project asset");
      allow(manager, unicode_name + "/app/asset.js", "Unicode project asset");
      // Stock VISE reads this sentinel through the app store's parent path.
      deny(manager, "gallery/app/%2e%2e%2fprivate.txt");
      allow(manager, "asset.js", "global asset");
      allow(manager, "nested%2fasset%20space.js", "nested global asset");
      allow(manager, "inside.js", "global asset");
      deny(manager, "escape.js");
      deny(manager, "escape-directory%2fprivate.txt");
      deny(manager, "%2e%2e%2fprivate.txt");
      deny(manager, "%2e%2e%2fwww-sibling%2fprivate.txt");
      deny(manager, encoded((root / "private.txt").generic_string()));
      deny(manager, "asset.js%00private.txt");
      require(get(manager, "asset.js%").d_status_code == 400, "malformed URL rejected");
      for(const char *escape : {"%2g", "%g2", "%0z"})
        require(get(manager, std::string("asset.js") + escape).d_status_code == 400,
                "partially valid hexadecimal escapes rejected");
      for(const char *store : {"app", "image", "image_src", "image_small"}) {
        const std::string prefix = std::string("gallery/") + store + "/";
        allow(manager, prefix + "asset.js", std::string("public ") + store);
        allow(manager, prefix + "asset.js?cache=1", std::string("public ") + store);
        allow(manager, prefix + "inside.js", std::string("public ") + store);
        deny(manager, prefix + "escape.js");
        deny(manager, prefix + "escape-directory/private.txt");
        deny(manager, prefix + "../private.txt");
        deny(manager, prefix + "%2e%2e%2fprivate.txt");
        deny(manager, prefix + encoded((root / "private.txt").generic_string()));
        deny(manager, prefix + "asset.js%00private.txt");
        deny(manager, prefix + "..%5cprivate.txt");
        require(get(manager, prefix + "asset.js%2g").d_status_code == 400,
                "project asset uses strict percent decoding");
      }
      allow(manager, "fallback/app/asset.js", "global asset");
      deny(manager, "fallback/app/escape.js");
      deny(manager, "coverless/_cover_image");
      const auto cover = get(manager, "gallery/_cover_image");
      require(cover.d_status_code == 200 && cover.d_payload.find(sentinel) == std::string::npos,
              "cover image must select an in-store file");
    }
    {
      conf["http-namespace"] = "/gallery/";
      vise::project_manager manager(conf);
      const auto response = get(manager, "gallery/app/asset.js", "/gallery/");
      require(response.d_status_code == 200 && response.d_payload == "public app",
              "namespace containing the project name must not change the resource path");
    }
    fs::remove_all(root);
    std::cout << "PASS: global/project static resources, canonical containment, encoded/absolute/NUL paths, symlink escapes and cover images\n";
    return 0;
  } catch(const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << " (test files preserved at " << root << ")\n";
    return 1;
  }
}
