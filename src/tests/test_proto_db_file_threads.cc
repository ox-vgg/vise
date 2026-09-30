#include "proto_db_file.h"
#include <boost/filesystem.hpp>
#include <atomic>
#include <iostream>
#include <random>
#include <thread>

int main() {
  const boost::filesystem::path filename = boost::filesystem::temp_directory_path() /
    boost::filesystem::unique_path("vise-protodb-%%%%-%%%%.bin");
  const uint32_t count = 256;
  std::vector<std::vector<std::string> > expected(count);
  try {
    protoDbFileBuilder builder(filename.string());
    for (uint32_t id = 0; id < count; ++id) {
      // Different record sizes expose cursor interference in both size and payload reads.
      for (uint32_t part = 0; part < 1 + id % 3; ++part) {
        std::string data(32 + (id * 47 + part * 19) % 2048, char(id % 251));
        data.replace(0, std::to_string(id).size(), std::to_string(id));
        expected[id].push_back(data);
        builder.addData(id, data);
      }
    }
    builder.close();
    std::atomic<bool> failed(false);
    // Reopen several times to cover the file-backed cold path, without caches.
    for (unsigned pass = 0; pass < 3; ++pass) {
      protoDbFile db(filename.string());
      std::atomic<bool> start(false);
      std::vector<std::thread> readers;
      for (unsigned worker = 0; worker < 8; ++worker) {
        readers.emplace_back([&, worker]() {
          std::mt19937 random(1009 * pass + worker);
          while (!start.load()) std::this_thread::yield();
          try {
            for (unsigned read = 0; read < 2000 && !failed.load(); ++read) {
              const uint32_t id = random() % count;
              std::vector<std::string> actual;
              db.getData(id, actual);
              if (actual != expected[id]) failed.store(true);
            }
          } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
            failed.store(true);
          }
        });
      }
      start.store(true);
      for (std::thread& reader : readers) reader.join();
    }
    boost::filesystem::remove(filename);
    if (failed.load()) {
      std::cerr << "Concurrent file-backed reads returned incorrect records" << std::endl;
      return 1;
    }
    return 0;
  } catch (const std::exception& error) {
    boost::filesystem::remove(filename);
    std::cerr << error.what() << std::endl;
    return 1;
  }
}
