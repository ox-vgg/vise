#include "proto_db_file.h"
#include "index_entry.pb.h"
#include "embedder.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace buildIndex {
  void mergeSortedFiles(std::vector<std::string> const&, std::string const,
                       uint32_t const, std::ofstream&, embedderFactory const*);
}

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

static void check_case(const boost::filesystem::path& root, uint32_t first) {
  rr::indexEntry sorted;
  const uint32_t words[] = {first, first, 48};
  for (unsigned int i=0; i<3; ++i) {
    sorted.add_id(words[i]);
    sorted.add_docid(i);
    sorted.add_qx(11+i);
    sorted.add_qy(21+i);
    sorted.mutable_qel_scale()->push_back(1);
    sorted.mutable_qel_ratio()->push_back(2);
    sorted.mutable_qel_angle()->push_back(3);
  }
  const std::string input = (root / (std::to_string(first)+"-sorted.bin")).string();
  const std::string output = (root / (std::to_string(first)+"-inverted.bin")).string();
  {
    protoDbFileBuilder writer(input, "index");
    writer.addData(0, sorted.SerializeAsString());
    writer.close();
  }
  std::ofstream log((root / "merge.log").string().c_str());
  noEmbedderFactory embedding;
  buildIndex::mergeSortedFiles({input}, output, 3, log, &embedding);
  protoDbFile database(output);
  std::vector<std::string> chunks;
  if (first!=0) {
    database.getData(0, chunks);
    require(chunks.empty(), "First nonzero visual word was incorrectly stored at word 0");
  }
  database.getData(first, chunks);
  require(chunks.size()==1, "First visual word missing from inverted index");
  rr::indexEntry decoded;
  require(decoded.ParseFromString(chunks[0]), "Cannot parse first visual word");
  require(decoded.qx_size()==2 && decoded.qx(0)==11 && decoded.qx(1)==12,
          "First word postings did not preserve feature coordinates");
  database.getData(48, chunks);
  require(chunks.size()==1 && decoded.ParseFromString(chunks[0]), "Last visual word missing");
  require(decoded.qx_size()==1 && decoded.qx(0)==13, "Last word posting changed");
}

int main() {
  const boost::filesystem::path root = boost::filesystem::temp_directory_path() /
                                      boost::filesystem::unique_path("vise-first-word-%%%%-%%%%");
  boost::filesystem::create_directory(root);
  try {
    check_case(root, 0);
    check_case(root, 26);
    boost::filesystem::remove_all(root);
    return 0;
  } catch(const std::exception& error) {
    std::cerr << error.what() << "\nFixture retained at " << root << std::endl;
    return 1;
  }
}
