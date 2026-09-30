#include "project_manager.h"
#include <Magick++.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = boost::filesystem;
namespace {
void require(bool condition, const std::string& message) {
  if(!condition) throw std::runtime_error(message);
}
void write(const fs::path& file, const std::string& value) {
  fs::create_directories(file.parent_path());
  std::ofstream output(file.string().c_str(), std::ios::binary);
  output << value;
  require(bool(output), "create fixture file");
}
std::string read(const fs::path& file) {
  std::string value;
  require(vise::file_load(file, value), "read fixture file");
  return value;
}
unsigned mutate(vise::project_manager& manager, const std::string& method,
                const std::string& route, const std::string& value = "upload") {
  vise::http_request request;
  const std::string body = method == "PUT" ? value : "";
  request.parse(method + " /gallery-api/" + route + " HTTP/1.1\r\nContent-Length: " +
                std::to_string(body.size()) + "\r\n\r\n" + body);
  vise::http_response response;
  manager.process_http_request(request, response);
  return response.d_status_code;
}
void link(const fs::path& target, const fs::path& path, bool directory = false) {
  boost::system::error_code error;
  if(directory) fs::create_directory_symlink(target, path, error);
  else fs::create_symlink(target, path, error);
  require(!error, "symlink fixture requires native permission: " + error.message());
}
}

int main(int argc, char** argv) {
  const fs::path root = fs::temp_directory_path() / fs::unique_path("vise-mutation-%%%%-%%%%");
  try {
    Magick::InitializeMagick(argv[0]);
    const fs::path projects = root / "projects", gallery = projects / "gallery";
    fs::create_directories(gallery);
    fs::create_directories(projects / "unloaded");
    fs::create_directories(projects / "custom.project");
    write(root / "sentinel.txt", "outside sentinel");
    std::map<std::string, std::string> settings;
    vise::init_default_vise_settings(settings);
    settings["vise-project-dir"] = projects.string();
    settings["vise-asset-dir"] = (root / "assets").string();
    settings["http-namespace"] = "/gallery-api/"; // contains the project name
    {
      vise::project_manager manager(settings);
      for(const std::string method : {"PUT", "DELETE"}) {
        require(mutate(manager, method, "missing/file.jpg") == 404, "missing project must not throw");
        require(mutate(manager, method, "unloaded/file.jpg") == 412, "unloaded project must not throw");
      }
      require(manager.project_load("gallery"), "load disposable project");
      require(manager.project_load("custom.project"), "load existing manually named project");
      require(mutate(manager, "PUT", "custom.project/existing-name.jpg") == 200 &&
              fs::is_regular_file(projects / "custom.project/image_src/existing-name.jpg"),
              "existing project names must not acquire creation restrictions");
      fs::create_directories(gallery / "image_src/nested");
      fs::create_directories(gallery / "image/nested");
      const std::string binary("a\0b\r\nc", 6);
      require(mutate(manager, "PUT", "gallery/one.jpg", binary) == 200 &&
              read(gallery / "image_src/one.jpg") == binary, "normal binary upload");
      require(mutate(manager, "PUT", "gallery/nested/photo%20one.jpg?unused=ignored") == 200 &&
              read(gallery / "image_src/nested/photo one.jpg") == "upload", "nested upload excludes query and namespace");
      require(mutate(manager, "PUT", "gallery/nested%2fencoded.jpg") == 200 &&
              fs::is_regular_file(gallery / "image_src/nested/encoded.jpg"), "encoded nested separator");
      require(mutate(manager, "PUT", "gallery/one.jpg", "replacement") == 200 &&
              read(gallery / "image_src/one.jpg") == "replacement", "normal overwrite");
      write(gallery / "image/one.jpg", "image");
      write(gallery / "image/nested/photo one.jpg", "image");
      require(mutate(manager, "DELETE", "gallery/one.jpg") == 200 &&
              !fs::exists(gallery / "image/one.jpg"), "normal image delete");
      require(mutate(manager, "DELETE", "gallery/nested/photo%20one.jpg?unused=ignored") == 200 &&
              !fs::exists(gallery / "image/nested/photo one.jpg"), "nested image delete");
      for(const std::string method : {"PUT", "DELETE"}) {
        for(const std::string asset : {"", "../sentinel.txt", "%2e%2e%2fsentinel.txt", "..%5csentinel.txt",
          "%2fsentinel.txt", "C%3a%2fsentinel.txt", "one.jpg%3astream", "one.jpg%00tail", "%", "%0g",
          "nested//file.jpg", "nested/./file.jpg", "nested/../file.jpg", "nested/"})
          require(mutate(manager, method, "gallery/" + asset) == 400, "reject malformed/escaped name: " + asset);
        require(mutate(manager, method, "gallery/nested") == 412 &&
                fs::is_directory(gallery / "image/nested"), "directory must not be mutated");
        require(mutate(manager, method, "gallery/new-directory/file.jpg") == 412,
                "missing parents must not be created");
      }
      for(const char* store : {"image_src", "image"}) {
        link(root / "sentinel.txt", gallery / store / "outside.jpg");
        link(root, gallery / store / "escape", true);
        write(gallery / store / "inside.jpg", "inside");
        link(gallery / store / "inside.jpg", gallery / store / "inside-link.jpg");
        link(root / "missing.txt", gallery / store / "dangling.jpg");
      }
      for(const std::string method : {"PUT", "DELETE"}) {
        for(const std::string asset : {"outside.jpg", "escape/sentinel.txt", "inside-link.jpg", "dangling.jpg"})
          require(mutate(manager, method, "gallery/" + asset) == 412, "links must not be mutated");
      }
#ifdef _WIN32
      for(const std::string method : {"PUT", "DELETE"})
        for(const std::string asset : {"CON.jpg", "nul", "COM1.png", "LPT9.jpg", "nested/.. /sentinel.txt", "one.jpg."})
          require(mutate(manager, method, "gallery/" + asset) == 400, "reject Win32 filename alias");
#endif
      require(read(root / "sentinel.txt") == "outside sentinel" && !fs::exists(root / "missing.txt"),
              "outside files must remain untouched");
      require(read(gallery / "image/inside.jpg") == "inside" &&
              read(gallery / "image_src/inside.jpg") == "inside", "inside symlink targets remain untouched");
      require(!fs::exists(gallery / "image_src/new-directory"), "upload created an unrequested directory");
    }
    fs::remove_all(root);
    std::cout << "PASS: confined image writes/deletes, nested encoded names, missing projects, aliases and symlink escapes\n";
    return 0;
  } catch(const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << " (fixture preserved at " << root << ")\n";
    return 1;
  }
}
