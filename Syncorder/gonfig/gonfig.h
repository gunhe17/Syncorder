#pragma once

#include <iostream>
#include <string>


/**
 * @class
 */

class Config {
public:
    int record_duration = 5;
    std::string output_directory = "./output/";

    static Config parseArgs(int argc, char* argv[]);
};

// Global
extern Config gonfig;