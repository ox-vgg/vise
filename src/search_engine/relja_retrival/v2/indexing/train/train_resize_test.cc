#include "train_resize.h"

int main(int argc, char** argv) {
  boost::filesystem::path image_src_dir("/home/tlm/data/vise/debug/train_resize/image_src/");
  boost::filesystem::path image_dir("/home/tlm/data/vise/debug/train_resize/image/");
  boost::filesystem::path filelist_fn("/home/tlm/data/vise/debug/train_resize/filelist.txt");
  boost::filesystem::path filestat_fn("/home/tlm/data/vise/debug/train_resize/filestat.txt");
  std::string resize_dimension("50x50");
  std::ofstream logf("/home/tlm/data/vise/debug/train_resize/log.txt");

  buildIndex::computeTrainResize(image_src_dir, image_dir, filelist_fn, filestat_fn, resize_dimension, logf);

  logf.close();
}