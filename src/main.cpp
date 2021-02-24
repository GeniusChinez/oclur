#include <cstddef>

void quit(std::size_t, std::size_t);

#include "engine.cpp"
#include <iostream>

void quit(std::size_t number_of_errors, std::size_t number_of_warnings) {
    if (number_of_errors == 0 && number_of_warnings == 0) {
        std::cout << "-- successful\n";
        return;
    }

    std::printf(
        "errors: %zu; warnings: %zu\n\n", 
        number_of_errors, number_of_warnings
    );

    exit(number_of_errors);
}

// @todo: use clargs
// @todo: use memory-guard
int main(int argc, char* const argv[]) {
    oclur::Engine engine;
}
