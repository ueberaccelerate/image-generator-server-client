#ifndef COMMON_INCLUDE_CONFIG_HPP
#define COMMON_INCLUDE_CONFIG_HPP

#include <string>
#include <string_view>

namespace resource {

  class Config {
  public:
    explicit Config(std::string_view config_file);
    std::string getName() const;

  };

}

#endif // !COMMON_INCLUDE_CONFIG_HPP
