#include "project_manager.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <Magick++.h>

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

static void copy_fixture(const boost::filesystem::path& source, const boost::filesystem::path& target) {
  boost::filesystem::create_directories(target);
  for (boost::filesystem::recursive_directory_iterator it(source), end; it != end; ++it) {
    const boost::filesystem::path destination = target / boost::filesystem::relative(it->path(), source);
    if (boost::filesystem::is_directory(it->path())) boost::filesystem::create_directories(destination);
    else if (boost::filesystem::is_regular_file(it->path())) boost::filesystem::copy_file(it->path(), destination);
  }
}

int main(int argc, char** argv) {
  Magick::InitializeMagick(argv[0]);
  const boost::filesystem::path root = boost::filesystem::temp_directory_path() /
    boost::filesystem::unique_path("vise-post-%%%%-%%%%");
  boost::filesystem::create_directories(root / "unindexed" / "data");
  boost::filesystem::create_directories(root / "broken" / "data");
  // An existing, valid project with no index must produce 412, not dereference
  // an absent project or a null feature extractor.
  {
    std::ofstream conf((root / "unindexed" / "data" / "conf.txt").string());
    conf << "search_engine=relja_retrival\npreset_conf_id=preset_conf_manual\n";
  }
  {
    std::ofstream conf((root / "broken" / "data" / "conf.txt").string());
    conf << "invalid_configuration=true\n";
  }
  try {
    const std::map<std::string, std::string> settings = {
      {"vise-project-dir", root.string()}, {"vise-asset-dir", root.string()},
      {"http-namespace", "/"}
    };
    const std::vector<std::string> endpoints = {"_extract_image_features", "_search_using_features",
      "_get_feature_match_details", "_external_register", "_extract_image_features/"};
    for (const std::string& endpoint : endpoints) {
      vise::project_manager manager(settings);
      vise::http_request request;
      request.d_method = "POST";
      request.d_uri = "/unindexed/" + endpoint;
      vise::http_response response;
      manager.process_http_request(request, response);
      require(response.d_status_code == 412, "Unindexed POST must return 412");
      require(manager.project_is_loaded("unindexed"), "First POST must load the project");
      vise::http_response repeat;
      manager.process_http_request(request, repeat);
      require(repeat.d_status_code == 412, "Repeated unindexed POST must remain safe");
    }
    vise::project_manager manager(settings);
    for (const std::string& name : {std::string("missing"), std::string("broken")}) {
      for (unsigned attempt = 0; attempt < 2; ++attempt) {
        vise::http_request request;
        request.d_method = "POST";
        request.d_uri = "/" + name + "/_extract_image_features";
        vise::http_response response;
        manager.process_http_request(request, response);
        require(response.d_status_code == (name == "missing" ? 404 : 412), "Unexpected failure status");
        require(!manager.project_is_loaded(name), "Failed load must not publish a project");
      }
    }
    vise::http_request unknown;
    unknown.d_method = "POST";
    unknown.d_uri = "/unindexed/_unknown";
    vise::http_response response;
    manager.process_http_request(unknown, response);
    require(response.d_status_code == 400, "Unknown POST must remain 400");
    // Optionally use a small public, fully indexed project to exercise successful
    // extraction and search as the first request of two separate managers.
    // All files are copied to this test's temporary store before use.
    if (argc == 3) {
      copy_fixture(boost::filesystem::path(argv[1]), root / "ready");
      std::ifstream image(argv[2], std::ios::binary);
      const std::string bytes((std::istreambuf_iterator<char>(image)), std::istreambuf_iterator<char>());
      require(!bytes.empty(), "Query image is missing");
      std::string features;
      {
        vise::project_manager ready(settings);
        vise::http_request extract;
        extract.d_method = "POST";
        extract.d_uri = "/ready/_extract_image_features";
        extract.d_payload << bytes;
        vise::http_response result;
        ready.process_http_request(extract, result);
        require(result.d_status_code == 200 && !result.d_payload.empty(), "Cold extraction failed");
        features = result.d_payload;
      }
      {
        vise::project_manager ready(settings);
        vise::http_request search;
        search.d_method = "POST";
        search.d_uri = "/ready/_search_using_features";
        search.d_payload << features;
        vise::http_response result;
        ready.process_http_request(search, result);
        require(result.d_status_code == 200 && result.d_payload.find("\"PNAME\":\"ready\"") != std::string::npos,
          "Cold feature search failed");
        require(result.d_payload.find("\"RESULT\":[{") != std::string::npos, "Cold search returned no matches");
      }
    }
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    boost::filesystem::remove_all(root);
    return 1;
  }
  boost::filesystem::remove_all(root);
  return 0;
}
