#include "search_engine.h"

vise::search_engine::search_engine(std::string se_name)
  : d_se_name(se_name)
{
  std::cout << "search_engine(): [" << d_se_name << "]"
            << std::endl;
}

vise::search_engine::~search_engine() {
  std::cout << "~search_engine(): [" << d_se_name << "]"
            << std::endl;
}
