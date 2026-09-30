#include "project.h"
#include "project_manager.h"
#include "vise_util.h"

#include <Magick++.h>
#include <boost/filesystem.hpp>
#include <atomic>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace fs = boost::filesystem;
namespace {
void require(bool condition, const std::string &message) {
  if(!condition) throw std::runtime_error(message);
}
void copy_tree(const fs::path &source, const fs::path &target) {
  fs::create_directories(target);
  for(fs::directory_iterator it(source), end; it != end; ++it) {
    require(!fs::is_symlink(it->path()), "test fixture must contain no symlinks");
    const fs::path output = target / it->path().filename();
    if(fs::is_directory(it->path())) copy_tree(it->path(), output);
    else fs::copy_file(it->path(), output);
  }
}
vise::http_response get(vise::project_manager &manager, const std::string &uri) {
  vise::http_request request;
  request.parse("GET " + uri + " HTTP/1.1\r\n\r\n");
  vise::http_response response;
  manager.process_http_request(request, response);
  return response;
}
}

int main(int argc, char **argv) {
  if(argc != 3) {
    std::cerr << "Usage: test_file_feature_status COMPLETED_TEST_PROJECT FEATURELESS_FILENAME\n";
    return 2;
  }
  const fs::path root = fs::temp_directory_path() / fs::unique_path("vise-feature-status-%%%%-%%%%");
  try {
    Magick::InitializeMagick(argv[0]);
    const fs::path project_dir = root / "gallery";
    copy_tree(fs::path(argv[1]) / "data", project_dir / "data");
    std::map<std::string, std::string> pconf;
    require(vise::configuration_load((project_dir / "data/conf.txt").string(), pconf), "load config");
    for(const char *key : {"data_dir", "image_dir", "image_src_dir", "tmp_dir", "app_dir", "image_small_dir"})
      pconf.erase(key);
    pconf["project_name"] = "gallery";
    pconf["nthread-indexing"] = "1";
    require(vise::configuration_save(pconf, (project_dir / "data/conf.txt").string()), "save test config");

    uint32_t empty_id = 0, present_id = 0, count = 0;
    bool empty_found = false, present_found = false;
    {
      vise::project project("gallery", (project_dir / "data/conf.txt").string());
      require(project.index_is_loaded(), "fixture must have a loaded index");
      count = project.fid_count();
      for(uint32_t id = 0; id < count; ++id) {
        const auto status = project.index_file_feature_status(id);
        if(status.filename == argv[2]) {
          require(status.indexed_word_count == 0, "featureless fixture must have no forward-index words");
          empty_id = id;
          empty_found = true;
        } else if(status.indexed_word_count > 0) {
          present_id = id;
          present_found = true;
        }
      }
      require(empty_found && present_found, "fixture needs both featureless and nonempty documents");
      bool threw = false;
      try { project.index_file_feature_status(count); } catch(const std::out_of_range &) { threw = true; }
      require(threw, "native API must reject an out-of-range ID");
      std::atomic<bool> started(false), invalid_snapshot(false);
      std::thread reader([&] {
        for(unsigned int iteration = 0; iteration < 5000; ++iteration) {
          try {
            const auto status = project.index_file_feature_status(empty_id);
            if(status.filename != argv[2] || status.indexed_word_count != 0) invalid_snapshot = true;
          } catch(const std::runtime_error &) {
            // A concurrent unload may finish before this read acquires its locks.
          } catch(...) { invalid_snapshot = true; }
          started = true;
        }
      });
      while(!started) std::this_thread::yield();
      bool success;
      std::string message;
      project.index_unload(success, message);
      reader.join();
      require(success, "unload fixture index");
      require(!invalid_snapshot, "diagnostics must remain valid during concurrent unload");
      threw = false;
      try { project.index_file_feature_status(empty_id); } catch(const std::runtime_error &) { threw = true; }
      require(threw, "native API must reject an unloaded index");
    }
    {
      std::map<std::string, std::string> conf;
      vise::init_default_vise_settings(conf);
      conf["vise-project-dir"] = root.string();
      conf["http-namespace"] = "/";
      vise::project_manager manager(conf);
      const std::string endpoint = "/gallery/file_feature_status?file_id=";
      auto response = get(manager, endpoint + std::to_string(empty_id));
      require(response.d_status_code == 200 && response.d_fields.at("Content-Type") == "application/json" &&
              response.d_payload.find("\"indexed_word_count\":0,\"has_indexed_features\":false") != std::string::npos,
              "featureless file remains present and reports empty features");
      response = get(manager, endpoint + std::to_string(present_id));
      require(response.d_status_code == 200 &&
              response.d_payload.find("\"has_indexed_features\":true") != std::string::npos, "indexed file reports words");
      for(const char *value : {"", "-1", "+1", "1.5", "1x", "4294967296", "999999999999999999999999"})
        require(get(manager, endpoint + value).d_status_code == 400, "malformed ID rejected");
      require(get(manager, "/gallery/file_feature_status").d_status_code == 400, "missing ID rejected");
      require(get(manager, endpoint + std::to_string(count)).d_status_code == 404, "unknown ID rejected");
      const auto files = get(manager, "/gallery/filelist?response_format=json");
      require(files.d_status_code == 200 && files.d_payload.find(argv[2]) != std::string::npos,
              "featureless file stays in filelist");
      vise::http_response unloaded;
      manager.project_index_unload("gallery", unloaded);
      require(get(manager, endpoint + std::to_string(empty_id)).d_status_code == 412, "HTTP unloaded index rejected");
    }
    fs::remove_all(root);
    std::cout << "PASS: native forward-word status, featureless filelist member, malformed/unknown IDs and unloaded indexes\n";
    return 0;
  } catch(const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << " (test files preserved at " << root.string() << ")\n";
    return 1;
  }
}
