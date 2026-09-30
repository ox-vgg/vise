#include "relja_retrival.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

static void initialize(const boost::filesystem::path& project, const std::string& log) {
  const boost::filesystem::path config = project / "data" / "conf.txt";
  {
    std::ofstream stream(config.string());
    stream << "search_engine=relja_retrival\n";
    if (!log.empty()) stream << "index_log_file=" << log << "\n";
  }
  vise::relja_retrival engine(config, project);
  require(!engine.index_is_loaded(), "New project unexpectedly has a loaded index");
}

static void check_log(const boost::filesystem::path& filename) {
  std::ifstream stream(filename.string());
  const std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
  require(content.find("///// LOG START:") != std::string::npos, "Start entry missing");
  require(content.find("~~~~~ LOG END:") != std::string::npos, "End entry missing");
}

int main() {
  const boost::filesystem::path root = boost::filesystem::temp_directory_path() /
    boost::filesystem::unique_path("vise-log-%%%%-%%%%");
  for (const char* folder : {"data", "image", "image_src", "tmp", "logs"})
    boost::filesystem::create_directories(root / folder);
  try {
    initialize(root, "");
    check_log(root / "data" / "index.log");
    boost::filesystem::remove(root / "data" / "index.log");
    initialize(root, "logs/relative.log");
    check_log(root / "logs" / "relative.log");
    require(!boost::filesystem::exists(root / "data" / "index.log"), "Configured log used data directory");
    initialize(root, (root / "logs" / "absolute.log").string());
    check_log(root / "logs" / "absolute.log");
    // A directory used as the log filename reliably fails on all supported OSes,
    // including accounts allowed to bypass ordinary file permission restrictions.
    boost::filesystem::create_directory(root / "data" / "index.log");
    initialize(root, "");
    initialize(root, "missing-parent/unwritable.log");
    require(!boost::filesystem::exists(root / "missing-parent"), "Logging created unexpected directories");
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    boost::filesystem::remove_all(root);
    return 1;
  }
  boost::filesystem::remove_all(root);
  return 0;
}
