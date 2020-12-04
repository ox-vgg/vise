#include "search_engine.h"

vise::search_engine::search_engine(std::string se_name)
  : d_se_name(se_name)
{
  std::cout << "search_engine(): [" << d_se_name << "]"
            << std::endl;
}

void vise::search_engine::register_image(uint32_t file1_id, uint32_t file2_id,
                                         uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                                         std::array<double, 9> &H) const {
  std::cerr << "register_image() not implemented for "
            << "search engine " << d_se_name << std::endl;
}

void vise::search_engine::register_external_image(const std::string &image_data,
                                                  const uint32_t file2_id,
                                                  std::array<double, 9> &H) const {
  std::cerr << "register_external_image() not implemented for "
            << "search engine " << d_se_name << std::endl;
}

vise::search_engine::~search_engine() {
  std::cout << "~search_engine(): [" << d_se_name << "]"
            << std::endl;
}
