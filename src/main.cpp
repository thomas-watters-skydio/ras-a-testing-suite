
#include <mavsdk/mavsdk.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <thread>
#include <memory>
#include <vector>
#include <chrono>
#include "environment.hpp"

#include "gtest/gtest.h"



int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    
    if (argc < 3) {
        std::cout << "Usage: ras_a_test_suite CONNECTION_URL [yaml-config file] [gtest arguments]" << std::endl;
        return 1;
    }

    const std::string yaml_path = argv[2];
    const std::string connection_url{argv[1]};

    try {
        RASATestingSuite::Environment::create(connection_url, yaml_path);
    } catch(YAML::BadFile &e) {
        std::cerr << "YAML file load \""<< yaml_path << "\" failed: " << e.msg << std::endl;
        exit(1);
    }

    ::testing::AddGlobalTestEnvironment(RASATestingSuite::Environment::getInstance());
    return RUN_ALL_TESTS();
}
