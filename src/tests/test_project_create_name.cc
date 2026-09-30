#include "project_manager.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <stdexcept>

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int main() {
  const boost::filesystem::path root = boost::filesystem::temp_directory_path() /
    boost::filesystem::unique_path("vise-project-name-%%%%-%%%%");
  boost::filesystem::create_directories(root / "projects");
  try {
    const std::map<std::string, std::string> settings = {
      {"vise-project-dir", (root / "projects").string()},
      {"vise-asset-dir", root.string()}, {"http-namespace", "/"}
    };
    vise::project_manager manager(settings);
    // Include encoded separators and traversal; validation must see the decoded name.
    const std::vector<std::string> invalid = {"", "../escaped", "..%2Fescaped",
      "..%5Cescaped", "two%2Flevels", "bad.name", "bad%25name", "bad&name", "bad$name"};
    for (const std::string& name : invalid) {
      vise::http_response response;
      manager.vise_project_create({{"pname", name}, {"response_format", "json"}}, response);
      require(response.d_payload.find("\"STATUS\":\"error\"") != std::string::npos,
        "Invalid project name must return an error");
      require(boost::filesystem::is_empty(root / "projects"), "Invalid name created project data");
      require(!boost::filesystem::exists(root / "escaped"), "Traversal escaped the project store");
    }
    vise::http_response missing;
    manager.vise_project_create({{"response_format", "json"}}, missing);
    require(missing.d_payload.find("project name is missing") != std::string::npos,
      "Missing name must retain its error");
    vise::http_response valid;
    manager.vise_project_create({{"pname", "GoodProject_12"}, {"response_format", "json"}}, valid);
    require(valid.d_payload.find("\"STATUS\":\"ok\"") != std::string::npos,
      "Valid name must still create a project");
    require(boost::filesystem::exists(root / "projects" / "GoodProject_12" / "data" / "conf.txt"),
      "Valid project configuration is missing");
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    boost::filesystem::remove_all(root);
    return 1;
  }
  boost::filesystem::remove_all(root);
  return 0;
}
