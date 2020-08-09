#ifndef COMMON_INCLUDE_CONFIG_HPP
#define COMMON_INCLUDE_CONFIG_HPP

#include <string>
#include <string_view>

namespace resource {


  class Config {
  public:

    void LoadFromFile(std::string_view config_file);
    void LoadFromData(const std::string& data);

    int getWidth() const;
    int getHeight() const;
    int getFramerate() const;
    int getPort() const;

    const std::string &data() const;

  private:
    void LoadDefaultConfig();

    int width_;
    int height_;
    int framerate_;
    int port_;

    std::string serdata_;
  };



}

#endif // !COMMON_INCLUDE_CONFIG_HPP
