#include "project_manager.h"
#include "project.h"
#include "vise_util.h"

#include <Magick++.h>
#include <boost/filesystem.hpp>
#include <vl/generic.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = boost::filesystem;

namespace {
void require(bool condition, const std::string &message) {
  if(!condition) throw std::runtime_error(message);
}

void copy_tree(const fs::path &source, const fs::path &target) {
  fs::create_directories(target);
  for(fs::directory_iterator it(source), end; it != end; ++it) {
    require(!fs::is_symlink(it->path()), "test fixture must not contain symlinks");
    const fs::path output = target / it->path().filename();
    if(fs::is_directory(it->path())) copy_tree(it->path(), output);
    else fs::copy_file(it->path(), output);
  }
}

void configure(const fs::path &project, bool index_only) {
  std::map<std::string, std::string> conf;
  require(vise::configuration_load((project / "data/conf.txt").string(), conf), "load fixture config");
  // A fixture's absolute directory overrides must never direct the test back
  // to its input tree. Each test project owns its copied data.
  for(const char *key : {"data_dir", "image_dir", "image_src_dir", "tmp_dir", "app_dir", "image_small_dir"})
    conf.erase(key);
  conf["project_name"] = project.filename().string();
  conf["nthread-indexing"] = "1";
  conf["nthread-search"] = "1";
  if(index_only) conf["index_only"] = "true";
  else conf.erase("index_only");
  require(vise::configuration_save(conf, (project / "data/conf.txt").string()), "save fixture config");
}

vise::http_response request(vise::project_manager &manager, const std::string &method,
                            const std::string &uri, const std::string &body = "") {
  vise::http_request req;
  std::ostringstream bytes;
  bytes << method << " " << uri << " HTTP/1.1\r\n";
  if(method == "POST" || method == "PUT") bytes << "Content-Length: " << body.size() << "\r\n";
  bytes << "\r\n" << body;
  req.parse(bytes.str());
  vise::http_response response;
  manager.process_http_request(req, response);
  return response;
}
}

int main(int argc, char **argv) {
  if(argc != 3) {
    std::cerr << "Usage: test_index_only COMPLETED_TEST_PROJECT QUERY_IMAGE\n";
    return 2;
  }
  const fs::path root = fs::temp_directory_path() / fs::unique_path("vise-index-only-%%%%-%%%%");
  try {
    Magick::InitializeMagick(argv[0]);
    fs::create_directories(root);
    const fs::path original = root / "gallery";
    const fs::path external = root / "external";
    copy_tree(fs::path(argv[1]) / "data", original / "data");
    copy_tree(fs::path(argv[1]) / "data", external / "data");
    configure(original, false);
    configure(external, true);
    std::string query;
    require(vise::file_load(fs::path(argv[2]), query) && !query.empty(), "load query image");

    std::map<std::string, std::string> conf;
    vise::init_default_vise_settings(conf);
    conf["vise-project-dir"] = root.string();
    conf["http-namespace"] = "/";
    std::string features, expected;
    std::string diagnostic_payload;
    unsigned int diagnostic_status = 0;
    {
      vl_rand_seed(vl_get_rand(), 1);
      vise::project_manager manager(conf);
      require(manager.project_load("gallery"), "load default-mode project");
      require(manager.project_index_is_loaded("gallery"), "default-mode index loaded");
      require(fs::is_directory(original / "image") && fs::is_directory(original / "image_src") &&
              fs::is_directory(original / "tmp"), "default mode still creates gallery directories");
      const auto diagnostic = request(manager, "GET", "/gallery/file_feature_status?file_id=0");
      diagnostic_status = diagnostic.d_status_code;
      diagnostic_payload = diagnostic.d_payload;
      // This PR does not implement that optional diagnostic. With the separate
      // API change present, mode handling must preserve its JSON response.
      require(diagnostic_status == 404 ||
              (diagnostic_status == 200 && diagnostic_payload.find("\"indexed_word_count\":") != std::string::npos),
              "diagnostic is either unsupported upstream or returns the optional JSON API");
      auto response = request(manager, "POST", "/gallery/_extract_image_features", query);
      require(response.d_status_code == 200 && !response.d_payload.empty(), "default query features");
      features = response.d_payload;
      response = request(manager, "POST", "/gallery/_search_using_features?response_format=json", features);
      require(response.d_status_code == 200 && response.d_payload.find("\"RESULT\":[") != std::string::npos,
              "fixture query must have at least one result");
      expected = response.d_payload;
    }
    for(int cold = 0; cold != 2; ++cold) {
      // VLFeat randomizes its vocabulary search forest at each load. Reset
      // that seed so the parity comparison measures gallery mode, not a new
      // approximate-search forest.
      vl_rand_seed(vl_get_rand(), 1);
      vise::project_manager manager(conf);
      // A new manager must cold-load the project directly from its first POST.
      auto response = request(manager, "POST", "/external/_extract_image_features", query);
      require(response.d_status_code == 200 && response.d_payload == features,
              "cold feature-byte parity (" + std::to_string(response.d_payload.size()) +
              " versus " + std::to_string(features.size()) + " bytes)");
      response = request(manager, "POST", "/external/_search_using_features?response_format=json", features);
      std::string expected_external = expected;
      const auto pos = expected_external.find("\"PNAME\":\"gallery\"");
      require(pos != std::string::npos, "baseline has project name");
      expected_external.replace(pos, std::string("\"PNAME\":\"gallery\"").size(), "\"PNAME\":\"external\"");
      require(response.d_status_code == 200 && response.d_payload == expected_external, "ranked search parity");
      require(request(manager, "GET", "/external/filelist?response_format=json").d_status_code == 200, "JSON filelist");
      require(request(manager, "GET", "/external/index_status?response_format=json").d_status_code == 200, "JSON status");
      const auto diagnostic = request(manager, "GET", "/external/file_feature_status?file_id=0");
      require(diagnostic.d_status_code == diagnostic_status && diagnostic.d_payload == diagnostic_payload,
              "optional file diagnostic preserves default handling and response body");
      require(request(manager, "GET", "/external/_image_src_count").d_payload == "0", "no local source images");
      for(const char *route : {"image/test.jpg", "image_src/test.jpg", "image_small/test.jpg", "_cover_image"})
        require(request(manager, "GET", std::string("/external/") + route).d_status_code == 404, "local image unavailable");
      for(const char *route : {"filelist", "external_search", "file?file_id=0", "register?file1_id=0&file2_id=0",
                               "app/test.js", "app/%2e%2e/image/test.jpg"})
        require(request(manager, "GET", std::string("/external/") + route).d_status_code == 412, "gallery UI unavailable");
      for(const char *route : {"_index_create", "_file_add", "_conf_save", "_external_register", "file_feature_status"})
        require(request(manager, "POST", std::string("/external/") + route).d_status_code == 412, "gallery mutation rejected");
      require(request(manager, "PUT", "/external/upload.jpg", "bytes").d_status_code == 412, "upload rejected");
      require(request(manager, "DELETE", "/external/test.jpg").d_status_code == 412, "image deletion rejected");
      require(request(manager, "GET", "/external/").d_payload.find("Index-only project") != std::string::npos, "mode explained");
      require(!fs::exists(external / "image") && !fs::exists(external / "image_src") && !fs::exists(external / "tmp"),
              "index-only serving must not recreate absent gallery directories");
    }
    const fs::path incomplete = root / "incomplete";
    fs::create_directories(incomplete / "data");
    fs::copy_file(external / "data/conf.txt", incomplete / "data/conf.txt");
    {
      vise::project project("incomplete", (incomplete / "data/conf.txt").string());
      require(project.state() == vise::project_state::INIT_FAILED, "index-only requires a completed index");
      bool success;
      std::string message;
      project.index_create(success, message, true);
      require(!success && message.find("index_only") != std::string::npos, "native indexing rejected");
    }
    {
      vise::project_manager manager(conf);
      require(request(manager, "POST", "/incomplete/_extract_image_features", query).d_status_code == 412,
              "incomplete index must not report successful empty features");
      std::map<std::string, std::string> bad_conf;
      require(vise::configuration_load((incomplete / "data/conf.txt").string(), bad_conf), "load incomplete config");
      bad_conf["index_only"] = "typo";
      require(vise::configuration_save(bad_conf, (incomplete / "data/conf.txt").string()), "save invalid mode");
      vise::project invalid("invalid", (incomplete / "data/conf.txt").string());
      require(invalid.state() == vise::project_state::INIT_FAILED, "invalid mode rejected");
      require(!fs::exists(incomplete / "image") && !fs::exists(incomplete / "image_src"), "invalid mode creates no gallery");
    }
    fs::remove_all(root);
    std::cout << "PASS: default directories, cold feature/rank parity, missing-image API, rejected mutations, incomplete index and invalid mode\n";
    return 0;
  } catch(const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << " (test files preserved at " << root.string() << ")\n";
    return 1;
  }
}
