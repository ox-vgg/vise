#include "proto_index.h"
#include "tfidf_v2.h"
#include "hamming.h"
#include "weighter_v2.h"
#include <boost/filesystem.hpp>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

// A backend with three word IDs, only word 1 observed. Bounds must be checked
// by protoIndex before any call into the backend, including parallel loading.
class testDb : public protoDb {
public:
  uint32_t numIDs() const { return 3; }
  void getData(uint32_t id, std::vector<std::string>& data) const {
    require(id < numIDs(), "Out-of-range backend lookup");
    data.clear();
    if (id == 1) {
      rr::indexEntry entry;
      entry.add_id(0);
      entry.mutable_qel_scale()->push_back(1);
      entry.set_data(std::string(4, '\0'));
      data.push_back(entry.SerializeAsString());
    }
  }
};

static rr::indexEntry query(bool absent) {
  rr::indexEntry result;
  if (absent) {
    // Two unmatched signatures precede the observed word; failure to consume
    // both would compare the wrong signature and lose an exact Hamming match.
    result.add_id(0); result.add_id(0);
    result.mutable_data()->append(8, char(0xff));
  }
  result.add_id(1);
  result.mutable_data()->append(4, '\0');
  if (absent) {
    result.add_id(3);
    result.add_id(std::numeric_limits<uint32_t>::max());
    result.mutable_data()->append(8, char(0xff));
  }
  result.set_qel_scale(std::string(result.id_size(), 1));
  return result;
}

static std::vector<double> weighted(protoIndex& index, bool absent, bool wgc) {
  rr::indexEntry representation = query(absent);
  const std::vector<double> idf = {1.0, 1.0, 1.0};
  tfidfV2::weightStatic(representation, NULL, &idf);
  uniqEntries entries;
  index.getUniqEntries(representation, entries);
  precompUEIterator iterator(entries);
  std::vector<double> scores;
  if (wgc) weighterV2::queryExecuteWGC(representation, &iterator, idf, {1.0}, scores, 128);
  else weighterV2::queryExecute(representation, &iterator, idf, {1.0}, scores);
  return scores;
}

int main() {
  const boost::filesystem::path root = boost::filesystem::temp_directory_path() /
    boost::filesystem::unique_path("vise-absent-word-%%%%-%%%%");
  boost::filesystem::create_directory(root);
  try {
    testDb database;
    protoIndex index(database, false);
    for (uint32_t id : {uint32_t(3), std::numeric_limits<uint32_t>::max()}) {
      std::vector<rr::indexEntry> entries(1);
      require(index.getEntries(id, entries) == 0 && entries.empty(), "Absent word must have no entries");
      require(!index.contains(id), "Absent word must not be present");
    }
    rr::indexEntry representation = query(true);
    const std::vector<double> idf = {1.0, 2.0, 3.0};
    tfidfV2::weightStatic(representation, NULL, &idf);
    require(representation.weight_size() == representation.id_size() &&
      representation.weight(2) == 2.0 && representation.weight(3) == 0.0 &&
      representation.weight(4) == 0.0, "IDF weighting changed observed or absent terms");
    for (bool wgc : {false, true}) {
      const auto normal = weighted(index, false, wgc);
      const auto missing = weighted(index, true, wgc);
      require(normal.size() == 1 && normal[0] > 0.0 && missing.size() == 1 && missing[0] > 0.0,
        "Absent-word TF-IDF/WGC query lost its observed match");
      rr::indexEntry only;
      only.add_id(3); only.add_id(std::numeric_limits<uint32_t>::max());
      only.set_qel_scale(std::string(2, 1));
      tfidfV2::weightStatic(only, NULL, &idf);
      uniqEntries empty;
      index.getUniqEntries(only, empty);
      precompUEIterator iterator(empty);
      std::vector<double> scores;
      if (wgc) weighterV2::queryExecuteWGC(only, &iterator, idf, {1.0}, scores, 128);
      else weighterV2::queryExecute(only, &iterator, idf, {1.0}, scores);
      require(scores.size() == 1 && scores[0] == 0.0, "Wholly absent query must have zero score");
    }
    const std::string weight_file = (root / "weights.bin").string();
    tfidfV2::save(weight_file, {1.0, 1.0, 1.0}, {1.0});
    tfidfV2 tfidf(&index, NULL, weight_file);
    const std::string embedding_file = (root / "hamming.bin").string();
    {
      rr::hammingData training;
      training.set_k(3); training.set_numdims(1); training.set_numbits(32);
      for (unsigned i = 0; i < 96; ++i) training.add_median(0.0);
      for (unsigned i = 0; i < 32; ++i) training.add_rotation(0.0);
      std::ofstream file(embedding_file, std::ios::binary);
      require(training.SerializeToOstream(&file), "Failed to save synthetic embedding");
    }
    hammingEmbedderFactory embedding(embedding_file, 32);
    hamming retriever(tfidf, &index, embedding);
    for (bool absent : {false, true}) {
      rr::indexEntry representation = query(absent);
      uniqEntries entries;
      index.getUniqEntries(representation, entries);
      precompUEIterator iterator(entries);
      std::vector<double> scores;
      retriever.queryExecute(representation, &iterator, scores);
      const double expected = absent ? 1.0 / 3.0 : 1.0;
      require(scores.size() == 1 && std::fabs(scores[0] - expected) < 1e-6,
        "Hamming signatures or query norm became misaligned at an absent word");
    }
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    boost::filesystem::remove_all(root);
    return 1;
  }
  boost::filesystem::remove_all(root);
  return 0;
}
