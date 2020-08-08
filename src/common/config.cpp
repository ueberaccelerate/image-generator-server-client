#include <resource/config.hpp>

#include <property/property.hpp>

namespace resource {

  class ServerConfig SERIALIZETHIS(ServerConfig) {
    SCALAR(width, int, "width of image");
    SCALAR(height, int, "height of image");
    SCALAR(framerate, int, "Frames send rate");
    SCALAR(port, int, "server port");

  public:
    CONSTRUCTORS(ServerConfig)
  };

  void resource::Config::LoadFromFile(std::string_view config_file) {
    ServerConfig config;
    try {
      if (config_file.empty())
        throw std::runtime_error("empty config_file");
      YAML::Emitter out;
      out << YAML::LoadFile(config_file.data());
      const std::string ser_data = out.c_str();
      config.deserialize(ser_data);

      height_ = config.height.get();
      width_ = config.width.get();
      framerate_ = config.framerate.get();
      port_ = config.port.get();

      std::cout 
        << "height: "    << height_    << "\n"
        << "width: "     << width_     << "\n"
        << "framerate: " << framerate_ << "\n"
        << "port: "      << port_      << "\n";

    } catch (std::exception &e) {
      std::cout << e.what() << '\n';
      LoadDefaultConfig();
    }
  }

  void resource::Config::LoadFromData(const std::string &data) {
    ServerConfig config;
    try {
      if (data.empty())
        throw std::runtime_error("empty data");
      config.deserialize(data);
    }
    catch (property::serialize_error& e) {
      std::cout << e.what();
      throw;
    }
  }

  int resource::Config::getWidth() const { return 0; }

  int resource::Config::getHeight() const { return 0; }

  int resource::Config::getFramerate() const { return 0; }

  int resource::Config::getPort() const { return 0; }

  void Config::LoadDefaultConfig() {
    std::cout << "** Load default server config\n";
    height_ = 256;
    width_ = 256;
    framerate_ = 1;
    port_ = 1234;
    ServerConfig config{
        "ImageGeneratorServer",
        "Generate random image with define width, height and framerate"};
    config.height.set(height_);
    config.width.set(width_);
    config.framerate.set(framerate_);
    config.port.set(port_);
    config.serialize([](const auto &serdata) {
      std::ofstream savedata("config.yaml");
      savedata << serdata;
    });
    std::cout
      << "height: "    << height_    << "\n"
      << "width: "     << width_     << "\n"
      << "framerate: " << framerate_ << "\n"
      << "port: "      << port_      << "\n";
  }
} // namespace resource
