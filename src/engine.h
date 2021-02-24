#pragma once
#include "location.h"

#include <cstdint>

#include <type_traits>
#include <vector>
#include <string>

#include <iostream>

namespace oclur {
    class Engine {
    public:
        Engine() = default;
        Engine(const Engine&) = delete;
        Engine(Engine&&) = default;
        Engine& operator=(const Engine&) = delete;
        Engine& operator=(Engine&&) = default;

        std::size_t get_number_of_errors() const;
        std::size_t get_number_of_warnings() const;

        ~Engine();

        template <typename T, typename ...Args> 
        void report_warning(const T&, Args&&...);

        template <typename T, typename ...Args> 
        void report_error(const T&, Args&&...);

        template <typename T, typename ...Args> 
        void report_fatal_error(const T&, Args&&...);

    private:
        template <typename T>
        [[nodiscard]] 
        bool print_location_if_possible(const T&) const;

        template <typename ...Args>
        void print_issue_tail(Args&&...) const;
        void print_issue_tail() const;

        void increment_number_of_errors();
        void increment_number_of_warnings();

        struct {
            std::size_t number_of_errors {0};
            std::size_t number_of_warnings {0};
        } issues;
    };
}
