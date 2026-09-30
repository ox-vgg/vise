#include "proto_db_file.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <limits>
#include <stdexcept>

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

static void require_empty(const protoDbFile& database, uint32_t id) {
  std::vector<std::string> data(1, "stale result");
  database.getData(id, data);
  require(data.empty(), "Absent ID must clear an earlier result");
  require(!database.contains(id), "Absent ID must not be reported as present");
}

int main() {
  const boost::filesystem::path root = boost::filesystem::temp_directory_path() /
    boost::filesystem::unique_path("vise-protodb-bounds-%%%%-%%%%");
  boost::filesystem::create_directory(root);
  try {
    const std::string filename = (root / "entries.bin").string();
    {
      protoDbFileBuilder builder(filename);
      builder.addData(0, "first entry");
      // ID 1 has no records; ID 2 is the last valid ID.
      builder.addData(2, "last entry");
      builder.addData(2, "second part");
      builder.close();
    }
    {
      protoDbFile database(filename);
      require(database.numIDs() == 3, "Unexpected fixture ID count");
      std::vector<std::string> data;
      database.getData(0, data);
      require(data == std::vector<std::string>{"first entry"}, "ID 0 changed");
      require(database.contains(0), "ID 0 must be present");
      database.getData(database.numIDs() - 1, data);
      require(data == std::vector<std::string>{"last entry", "second part"}, "Last valid ID changed");
      require(database.contains(database.numIDs() - 1), "Last valid ID must be present");
      require_empty(database, 1);
      require_empty(database, database.numIDs());
      require_empty(database, database.numIDs() + 1);
      require_empty(database, std::numeric_limits<uint32_t>::max());
      database.getData(0, data);
      require(data == std::vector<std::string>{"first entry"}, "Invalid lookup disturbed subsequent reads");
    }
    const std::string empty_filename = (root / "empty.bin").string();
    {
      protoDbFileBuilder builder(empty_filename);
      builder.close();
    }
    {
      protoDbFile empty(empty_filename);
      require(empty.numIDs() == 0, "Empty database has an ID");
      require_empty(empty, 0);
      require_empty(empty, std::numeric_limits<uint32_t>::max());
    }
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    boost::filesystem::remove_all(root);
    return 1;
  }
  boost::filesystem::remove_all(root);
  return 0;
}
