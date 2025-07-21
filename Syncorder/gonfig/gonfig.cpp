#include "gonfig.h"

// Global
Config gonfig;

/**
 * @class
 */

Config Config::parseArgs(int argc, char* argv[]) {
    Config conf;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--output_directory" && i + 1 < argc) {
            conf.output_directory = argv[++i];
        }
    }
    
    return conf;
}