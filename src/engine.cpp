#pragma once
#include "engine.h"

namespace oclur {    
    std::size_t Engine::get_number_of_errors() const {
        return issues.number_of_errors;
    }

    std::size_t Engine::get_number_of_warnings() const {
        return issues.number_of_warnings;
    }

    void Engine::increment_number_of_errors() {
        issues.number_of_errors++;
    }

    void Engine::increment_number_of_warnings() {
        issues.number_of_warnings++;
    }

    template <typename T>
    bool Engine::print_location_if_possible(const T& data) const {
        if constexpr(std::is_same_v<std::decay_t<T>, Location>) {
            std::cout << data.format();
        }
        else if constexpr(std::is_same_v<std::decay_t<T>, Location*>) {
            std::cout << data->format();
        }
        else {
            return false;
        }
        return true;
    }

    void Engine::print_issue_tail() const {
        std::cout << '\n';
    }

    template <typename ...Args>
    void Engine::print_issue_tail(Args&&... args) const {
        (std::cout << ... << args);
        std::cout << '\n';
    }

    template <typename T, typename ...Args> 
    void Engine::report_warning(const T& arg1, Args&&... args) {
        if (print_location_if_possible(arg1)) {
            std::cout << ": warning: ";
        }
        else {
            std::cout << "warning: " << arg1;
        }

        print_issue_tail(std::forward<Args>(args)...);
        increment_number_of_warnings();
    }

    template <typename T, typename ...Args> 
    void Engine::report_error(const T& arg1, Args&&... args) {
        if (print_location_if_possible(arg1)) {
            std::cout << ": error: ";
        }
        else {
            std::cout << "error: " << arg1;
        }

        print_issue_tail(std::forward<Args>(args)...);
        increment_number_of_errors();
    }

    template <typename T, typename ...Args> 
    void Engine::report_fatal_error(const T& arg1, Args&&... args) {
        report_error(arg1, std::forward<Args>(args)...);
        quit(get_number_of_errors(), get_number_of_warnings());
    }

    Engine::~Engine() {
        quit(get_number_of_errors(), get_number_of_warnings());
    }
}
