#include "Server.h"

#include <iostream>

#include <boost/program_options.hpp>

int main(int argc, char **argv) {
  try
  {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::string config_file;
    desc.add_options()
      ("help", "produce help message")
      ("config", po::value<std::string>(&config_file), "load config file (.yaml)")
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << '\n';
      return 1;
    }

    resource::Config config;
    if (vm.count("config")) {
        std::cout << config_file << "\n";
        config.LoadFromFile(config_file);
    }
    else {
        config.LoadFromFile("default");
    }
    boost::asio::io_context io_context;
    server::ImageGeneratorServer server(io_context, config);
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
