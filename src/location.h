#pragma once

#include <string>
#include <cstddef>

namespace oclur {
    struct Location {
        const std::string& file;
        std::size_t line {1};
        std::size_t column {0};

        Location& operator++(){
            column++;
            return *this;
        }

        std::string format() const {
            return std::string("in file '")  + file + "': " + 
                std::string("line ") + std::to_string(line) + "." + 
                std::to_string(column);
        }
    };
}
