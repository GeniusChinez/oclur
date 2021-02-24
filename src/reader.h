#pragma once

#include <iostream>
#include <fstream>

namespace oclur {
    std::pair<bool, std::string> read_file(std::string_view filepath) {
        std::ifstream is(filepath.data(), std::ios::binary);

        if (!is.is_open()) {
            return {false, ""};
        }

        std::string filedata(
            (std::istreambuf_iterator<char>(is)), 
            (std::istreambuf_iterator<char>())
        );

        is.close();
        return {true, std::move(filedata)};
    }
}
